#!/usr/bin/env python3
"""Independent APK Signature Scheme v2 verifier (spec-based). usage: verify_v2.py file.apk"""
import struct, sys, hashlib
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import padding, rsa, ec
from cryptography import x509

d = open(sys.argv[1], "rb").read()
e = d.rfind(b"PK\x05\x06")
cd_size, cd_off = struct.unpack_from("<II", d, e + 12)
assert d[cd_off - 16:cd_off] == b"APK Sig Block 42", "no signing block"
sz2 = struct.unpack_from("<Q", d, cd_off - 24)[0]
bstart = cd_off - sz2 - 8
assert struct.unpack_from("<Q", d, bstart)[0] == sz2
p = bstart + 8
pairs = {}
while p < cd_off - 24:
    ln, pid = struct.unpack_from("<QI", d, p)
    pairs[pid] = d[p + 12:p + 8 + ln]
    p += 8 + ln
print("pairs:", [hex(k) for k in pairs])
v2 = pairs[0x7109871a]
def rd(b, o):
    n = struct.unpack_from("<I", b, o)[0]; return b[o + 4:o + 4 + n], o + 4 + n
def seq(b):
    o, out = 0, []
    while o < len(b):
        x, o = rd(b, o); out.append(x)
    return out
signers, _ = rd(v2, 0)
for s in seq(signers):
    sd, o = rd(s, 0); sigs, o = rd(s, o); pub, o = rd(s, o)
    digs, o2 = rd(sd, 0); certs, o2 = rd(sd, o2)
    key = serialization.load_der_public_key(pub)
    cert = x509.load_der_x509_certificate(seq(certs)[0])
    assert cert.public_key().public_bytes(serialization.Encoding.DER, serialization.PublicFormat.SubjectPublicKeyInfo) == pub
    print("cert:", cert.subject.rfc4514_string())
    for sg in seq(sigs):
        alg = struct.unpack_from("<I", sg, 0)[0]; sig, _ = rd(sg, 4)
        h = hashes.SHA256() if alg in (0x0103, 0x0201) else hashes.SHA512()
        if isinstance(key, rsa.RSAPublicKey): key.verify(sig, sd, padding.PKCS1v15(), h)
        else: key.verify(sig, sd, ec.ECDSA(h))
        print("signature alg %#x OK" % alg)
    eocd = bytearray(d[e:]); struct.pack_into("<I", eocd, 16, bstart)
    secs = [d[:bstart], d[cd_off:cd_off + cd_size], bytes(eocd)]
    for dg in seq(digs):
        alg = struct.unpack_from("<I", dg, 0)[0]; want, _ = rd(dg, 4)
        H = hashlib.sha256 if alg in (0x0103, 0x0201) else hashlib.sha512
        ch = []
        for sec in secs:
            for i in range(0, len(sec), 1 << 20):
                c = sec[i:i + (1 << 20)]; ch.append(H(b"\xa5" + struct.pack("<I", len(c)) + c).digest())
        got = H(b"\x5a" + struct.pack("<I", len(ch)) + b"".join(ch)).digest()
        print("content digest %#x" % alg, "OK" if got == want else "MISMATCH")
        assert got == want
print("VERIFIED")
