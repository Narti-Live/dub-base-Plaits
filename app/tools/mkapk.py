#!/usr/bin/env python3
"""Builds and v2-signs the Dub Base APK without the Android SDK.
usage: mkapk.py <root> <out.apk>"""
import struct, sys, os, zipfile, zlib, hashlib, datetime
from cryptography import x509
from cryptography.x509.oid import NameOID
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import rsa, padding

ROOT, OUT = sys.argv[1], sys.argv[2]
PKG = os.environ.get("APK_PKG", "com.nartilive.dubplaits")
LABEL = os.environ.get("APK_LABEL", "Dub Plaits")
LIB = "dubplaits"
LIBDIR = os.environ.get("APK_LIBDIR", "lib")
ICON = os.environ.get("APK_ICON", "icon.png")
VCODE = int(os.environ.get("APK_VCODE", "14"))
VNAME = os.environ.get("APK_VNAME", "1.4")

# ---------------------------------------------------------------- binary XML
ANDROID_NS = "http://schemas.android.com/apk/res/android"
ATTR_IDS = {"theme": 0x01010000, "label": 0x01010001, "icon": 0x01010002, "name": 0x01010003,
            "hasCode": 0x0101000c, "exported": 0x01010010, "screenOrientation": 0x0101001e,
            "configChanges": 0x0101001f, "value": 0x01010024, "minSdkVersion": 0x0101020c,
            "versionCode": 0x0101021b, "versionName": 0x0101021c, "targetSdkVersion": 0x01010270,
            "required": 0x0101028e, "extractNativeLibs": 0x010104ea}
T_REF, T_STR, T_DEC, T_HEX, T_BOOL = 0x01, 0x03, 0x10, 0x11, 0x12

# (tag, [(ns?, name, type, value)], children)
def A(name, t, v, ns=True): return (ns, name, t, v)
manifest = ("manifest", [A("versionCode", T_DEC, VCODE), A("versionName", T_STR, VNAME), A("package", T_STR, PKG, ns=False)], [
    ("uses-sdk", [A("minSdkVersion", T_DEC, 26), A("targetSdkVersion", T_DEC, 34)], []),
    ("uses-feature", [A("name", T_STR, "android.hardware.touchscreen"), A("required", T_BOOL, False)], []),
    ("application", [A("theme", T_REF, 0x0103000a), A("label", T_STR, LABEL), A("icon", T_REF, 0x7f010000),
                     A("hasCode", T_BOOL, False), A("extractNativeLibs", T_BOOL, True)], [
        ("activity", [A("theme", T_REF, 0x0103000a), A("label", T_STR, LABEL), A("name", T_STR, "android.app.NativeActivity"),
                      A("exported", T_BOOL, True), A("screenOrientation", T_DEC, 1), A("configChanges", T_HEX, 0xfb0)], [
            ("meta-data", [A("name", T_STR, "android.app.lib_name"), A("value", T_STR, LIB)], []),
            ("intent-filter", [], [
                ("action", [A("name", T_STR, "android.intent.action.MAIN")], []),
                ("category", [A("name", T_STR, "android.intent.category.LAUNCHER")], []),
            ]),
        ]),
    ]),
])

def build_axml(root):
    # collect strings: resource-mapped attribute names first
    attr_names, others = [], []
    def walk(n):
        tag, attrs, kids = n
        for ns, name, t, v in attrs:
            if ns and name not in attr_names: attr_names.append(name)
        for k in kids: walk(k)
    walk(root)
    attr_names.sort(key=lambda a: ATTR_IDS[a])
    strings = list(attr_names)
    def S(s):
        if s not in strings: strings.append(s)
        return strings.index(s)
    S("android"); S(ANDROID_NS)
    def walk2(n):
        tag, attrs, kids = n
        S(tag)
        for ns, name, t, v in attrs:
            S(name)
            if t == T_STR: S(v)
        for k in kids: walk2(k)
    walk2(root)
    # string pool (UTF-16)
    offs, data = [], b""
    for s in strings:
        offs.append(len(data))
        e = s.encode("utf-16le")
        data += struct.pack("<H", len(s)) + e + b"\0\0"
    while len(data) % 4: data += b"\0"
    hdr = 28
    sp_body = b"".join(struct.pack("<I", o) for o in offs) + data
    strings_start = hdr + 4 * len(strings)
    sp = struct.pack("<HHIIIIII", 0x0001, hdr, hdr + len(sp_body), len(strings), 0, 0, strings_start, 0) + sp_body
    rm_body = b"".join(struct.pack("<I", ATTR_IDS[a]) for a in attr_names)
    rm = struct.pack("<HHI", 0x0180, 8, 8 + len(rm_body)) + rm_body
    nsi, urii = strings.index("android"), strings.index(ANDROID_NS)
    body = struct.pack("<HHIIIII", 0x0100, 16, 24, 1, 0xffffffff, nsi, urii)
    line = [1]
    def elem(n):
        nonlocal body
        tag, attrs, kids = n
        line[0] += 1
        enc = []
        for ns, name, t, v in sorted(attrs, key=lambda a: ATTR_IDS.get(a[1], 0xffffffff) if a[0] else 0xfffffffe):
            raw = strings.index(v) if t == T_STR else 0xffffffff
            if t == T_STR: d = strings.index(v)
            elif t == T_BOOL: d = 0xffffffff if v else 0
            else: d = v
            enc.append(struct.pack("<IIIHBBI", urii if ns else 0xffffffff, strings.index(name), raw, 8, 0, t, d))
        ab = b"".join(enc)
        body += struct.pack("<HHIII", 0x0102, 16, 36 + len(ab), line[0], 0xffffffff)
        body += struct.pack("<IIHHHHHH", 0xffffffff, strings.index(tag), 20, 20, len(attrs), 0, 0, 0) + ab
        for k in kids: elem(k)
        line[0] += 1
        body += struct.pack("<HHIIIII", 0x0103, 16, 24, line[0], 0xffffffff, 0xffffffff, strings.index(tag))
    elem(root)
    body += struct.pack("<HHIIIII", 0x0101, 16, 24, line[0] + 1, 0xffffffff, nsi, urii)
    payload = sp + rm + body
    return struct.pack("<HHI", 0x0003, 8, 8 + len(payload)) + payload

# ---------------------------------------------------------------- resources.arsc
def build_arsc():
    """Minimal resource table: one package 0x7f with drawable/icon -> res/drawable/icon.png"""
    def spool(strs):
        offs, data = [], b""
        for s in strs:
            offs.append(len(data)); data += struct.pack("<H", len(s)) + s.encode("utf-16le") + b"\0\0"
        while len(data) % 4: data += b"\0"
        body = b"".join(struct.pack("<I", o) for o in offs) + data
        return struct.pack("<HHIIIIII", 1, 28, 28 + len(body), len(strs), 0, 0, 28 + 4 * len(strs), 0) + body
    gpool = spool(["res/drawable/icon.png"])
    tpool = spool(["drawable"])
    kpool = spool(["icon"])
    # typeSpec
    tspec = struct.pack("<HHIBBHI", 0x0202, 16, 16 + 4, 1, 0, 0, 1) + struct.pack("<I", 0)
    # type chunk with default config (size 64 config struct)
    config = struct.pack("<I", 64) + b"\0" * 60
    entries_start = 20 + len(config) + 4
    entry = struct.pack("<HHI", 8, 0, 0) + struct.pack("<HBBI", 8, 0, 0x03, 0)
    ttype = struct.pack("<HHIBBHII", 0x0201, 20 + len(config), entries_start + len(entry), 1, 0, 0, 1, entries_start)
    ttype += config + struct.pack("<I", 0) + entry
    name = PKG.encode("utf-16le").ljust(256, b"\0")
    pkg_hdr_size = 288
    type_off = pkg_hdr_size
    key_off = type_off + len(tpool)
    pbody = tpool + kpool + tspec + ttype
    pkg = struct.pack("<HHII", 0x0200, pkg_hdr_size, pkg_hdr_size + len(pbody), 0x7f) + name
    pkg += struct.pack("<IIIII", type_off, 1, key_off, 1, 0)
    pkg += pbody
    body = gpool + pkg
    return struct.pack("<HHII", 0x0002, 12, 12 + len(body), 1) + body

# ---------------------------------------------------------------- zip (aligned)
def build_zip(entries):
    """entries: list of (name, bytes, stored, align)"""
    out, cd = bytearray(), bytearray()
    dt = (0, 0x21)  # 1980-01-01 00:00
    for name, data, stored, align in entries:
        nb = name.encode()
        crc = zlib.crc32(data) & 0xffffffff
        if stored: comp, method = data, 0
        else:
            c = zlib.compressobj(9, zlib.DEFLATED, -15); comp = c.compress(data) + c.flush(); method = 8
        off = len(out)
        extra = b""
        if align:
            pad = (-(off + 30 + len(nb))) % align
            extra = b"\0" * pad
        out += struct.pack("<IHHHHHIIIHH", 0x04034b50, 20, 0, method, dt[0], dt[1], crc, len(comp), len(data), len(nb), len(extra)) + nb + extra + comp
        cd += struct.pack("<IHHHHHHIIIHHHHHII", 0x02014b50, 20, 20, 0, method, dt[0], dt[1], crc, len(comp), len(data), len(nb), 0, 0, 0, 0, 0, off) + nb
    cd_off = len(out)
    eocd = struct.pack("<IHHHHIIH", 0x06054b50, 0, 0, len(entries), len(entries), len(cd), cd_off, 0)
    return bytes(out), bytes(cd), eocd

# ---------------------------------------------------------------- v2 signature
def lp(b): return struct.pack("<I", len(b)) + b
def chunked_digest(sections):
    chunks = []
    for s in sections:
        for i in range(0, len(s), 1 << 20):
            c = s[i:i + (1 << 20)]
            chunks.append(hashlib.sha256(b"\xa5" + struct.pack("<I", len(c)) + c).digest())
    return hashlib.sha256(b"\x5a" + struct.pack("<I", len(chunks)) + b"".join(chunks)).digest()

def load_key(keydir):
    os.makedirs(keydir, exist_ok=True)
    kp, cp = os.path.join(keydir, "key.pem"), os.path.join(keydir, "cert.der")
    if not os.path.exists(kp):
        key = rsa.generate_private_key(public_exponent=65537, key_size=2048)
        nm = x509.Name([x509.NameAttribute(NameOID.COMMON_NAME, "Dub Base"), x509.NameAttribute(NameOID.ORGANIZATION_NAME, "Narti Live")])
        now = datetime.datetime(2026, 1, 1)
        cert = (x509.CertificateBuilder().subject_name(nm).issuer_name(nm).public_key(key.public_key())
                .serial_number(x509.random_serial_number()).not_valid_before(now)
                .not_valid_after(now + datetime.timedelta(days=365 * 40)).sign(key, hashes.SHA256()))
        open(kp, "wb").write(key.private_bytes(serialization.Encoding.PEM, serialization.PrivateFormat.PKCS8, serialization.NoEncryption()))
        open(cp, "wb").write(cert.public_bytes(serialization.Encoding.DER))
    key = serialization.load_pem_private_key(open(kp, "rb").read(), None)
    return key, open(cp, "rb").read()

def sign_v2(entries_bytes, cd, eocd, key, cert_der):
    digest = chunked_digest([entries_bytes, cd, eocd])
    ALG = 0x0103  # RSASSA-PKCS1-v1_5 with SHA2-256
    signed_data = lp(lp(struct.pack("<I", ALG) + lp(digest))) + lp(lp(cert_der)) + lp(b"")
    sig = key.sign(signed_data, padding.PKCS1v15(), hashes.SHA256())
    pub = key.public_key().public_bytes(serialization.Encoding.DER, serialization.PublicFormat.SubjectPublicKeyInfo)
    signer = lp(signed_data) + lp(lp(struct.pack("<I", ALG) + lp(sig))) + lp(pub)
    value = lp(lp(signer))
    pairs = struct.pack("<QI", 4 + len(value), 0x7109871a) + value
    size = len(pairs) + 8 + 16
    block = struct.pack("<Q", size) + pairs + struct.pack("<Q", size) + b"APK Sig Block 42"
    new_eocd = eocd[:16] + struct.pack("<I", len(entries_bytes) + len(block)) + eocd[20:]
    return entries_bytes + block + cd + new_eocd

if __name__ == "__main__":
    B = os.path.join(ROOT, "build")
    files = [
        ("AndroidManifest.xml", build_axml(manifest), False, 0),
        ("resources.arsc", build_arsc(), True, 4),
        ("res/drawable/icon.png", open(os.path.join(B, ICON), "rb").read(), True, 4),
        ("lib/arm64-v8a/libdubplaits.so", open(os.path.join(B, LIBDIR, "arm64-v8a/libdubplaits.so"), "rb").read(), False, 0),
        ("lib/x86_64/libdubplaits.so", open(os.path.join(B, LIBDIR, "x86_64/libdubplaits.so"), "rb").read(), False, 0),
    ]
    eb, cd, eocd = build_zip(files)
    key, cert = load_key(os.path.join(ROOT, "keys"))
    apk = sign_v2(eb, cd, eocd, key, cert)
    open(OUT, "wb").write(apk)
    print("wrote", OUT, len(apk), "bytes")
