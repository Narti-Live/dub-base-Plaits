"""Cortex-M4 emulation harness for Plaits firmware DSP code (unicorn).
Loads the application .bin at 0x08008000, initialises .data/.bss, runs the
C++ constructors, and lets tests call firmware functions by symbol name."""
import os, sys, struct, subprocess
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, '..', 'unicorn', 'bindings', 'python'))
os.environ.setdefault('LIBUNICORN_PATH', os.path.join(HERE, '..', 'unicorn', 'build'))
from unicorn import *
from unicorn.arm_const import *

FLASH, RAM = 0x08000000, 0x20000000
RET_MAGIC = 0x0807FF00

class Firmware:
    def __init__(self, elf, binf, count=True):
        self.sym = {}
        out = subprocess.run(['llvm-nm', '--defined-only', elf], capture_output=True, text=True).stdout
        for line in out.splitlines():
            p = line.split()
            if len(p) == 3: self.sym[p[2]] = int(p[0], 16)
        self.uc = Uc(UC_ARCH_ARM, UC_MODE_THUMB | UC_MODE_MCLASS)
        uc = self.uc
        uc.ctl_set_cpu_model(UC_CPU_ARM_CORTEX_M4)
        uc.mem_map(FLASH, 0x80000)
        uc.mem_map(RAM, 0x10000)                       # 32K + slack to catch overflows
        uc.mem_map(0x40000000, 0x20000000)             # peripherals (read as zero)
        uc.mem_write(0x08008000, open(binf, 'rb').read())
        # FPU on: CPACR full access for CP10/CP11
        try: uc.reg_write(UC_ARM_REG_CPACR, 0xF << 20)
        except Exception as e: print('cpacr', e)
        # .data / .bss
        sid, sd, ed, sb, eb = (self.sym[k] for k in ('_sidata', '_sdata', '_edata', '_sbss', '_ebss'))
        uc.mem_write(sd, bytes(uc.mem_read(sid, ed - sd)))
        uc.mem_write(sb, b'\0' * (eb - sb))
        self.stack_top = 0x20008000
        self.icount = 0
        self.counting = False
        self.counting = False
        if count: uc.hook_add(UC_HOOK_CODE, self._code)
        self.call('__libc_init_array')

    def _code(self, uc, addr, size, ud):
        if self.counting: self.icount += 1

    def find(self, prefix):
        c = [k for k in self.sym if k.startswith(prefix)]
        if len(c) != 1: raise KeyError((prefix, c[:5]))
        return self.sym[c[0]]

    def call(self, fn, *args, count=False, fargs=None):
        addr = self.sym[fn] if isinstance(fn, str) and fn in self.sym else (self.find(fn) if isinstance(fn, str) else fn)
        uc = self.uc
        regs = [UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3]
        for r, a in zip(regs, args): uc.reg_write(r, a & 0xffffffff)
        sp = self.stack_top - 64
        for i, a in enumerate(args[4:]): self.w32(sp + 4 * i, a)
        if fargs:
            for i, f in enumerate(fargs):
                uc.reg_write(UC_ARM_REG_S0 + i, struct.unpack('<I', struct.pack('<f', f))[0])
        uc.reg_write(UC_ARM_REG_SP, sp)
        uc.reg_write(UC_ARM_REG_LR, RET_MAGIC | 1)
        self.icount = 0
        self.counting = count
        uc.emu_start(addr | 1, RET_MAGIC, count=200_000_000)
        self.counting = False
        if uc.reg_read(UC_ARM_REG_PC) & ~1 != RET_MAGIC & ~1:
            raise RuntimeError('call to %s did not return (pc=%#x)' % (fn, uc.reg_read(UC_ARM_REG_PC)))
        return uc.reg_read(UC_ARM_REG_R0)


    def skip(self, prefix):
        # make a function return immediately (hardware driver inits that poll status flags)
        addr = self.find(prefix) & ~1
        def cb(uc, a, size, ud):
            uc.reg_write(UC_ARM_REG_PC, uc.reg_read(UC_ARM_REG_LR))
        self.uc.hook_add(UC_HOOK_CODE, cb, begin=addr, end=addr)

    def w32(self, a, v): self.uc.mem_write(a, struct.pack('<I', v & 0xffffffff))
    def wf(self, a, v): self.uc.mem_write(a, struct.pack('<f', v))
    def r16s(self, a, n): return struct.unpack('<%dh' % n, bytes(self.uc.mem_read(a, 2 * n)))
    def rf(self, a): return struct.unpack('<f', bytes(self.uc.mem_read(a, 4)))[0]
