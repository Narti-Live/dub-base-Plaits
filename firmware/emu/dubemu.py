"""Drive the DUB BASE firmware DSP (Voice + sequencer) inside the Cortex-M4 emulator."""
import sys, struct
sys.path.insert(0, '.')
from armemu import *

S = 0x2000C000
ALLOC, PATCH, MODS, FRAMES, DUBS = S, S + 0x40, S + 0x100, S + 0x200, S + 0x300

class DubFw:
    def __init__(self, elf, binf, count=True):
        fw = self.fw = Firmware(elf, binf, count)
        fw.w32(ALLOC, fw.sym['shared_buffer']); fw.w32(ALLOC + 4, fw.sym['shared_buffer'])
        fw.w32(ALLOC + 8, 16384); fw.w32(ALLOC + 12, 16384)
        self.voice = fw.sym['voice']
        fw.call('_ZN6plaits5Voice4InitEPN6stmlib15BufferAllocatorE', self.voice, ALLOC)
        fw.call(fw.find('_ZN6plaits3dub15DefaultSettings'), DUBS)
        self.render = fw.find('_ZN6plaits5Voice6RenderE')
        self.patch(0.5, 0.5, 0.5)
        self.mods(trigger=0.0, trigger_patched=False)
    def patch(self, h, t, m, engine=8, amounts=(0.0, 0.0, 0.0)):
        fw = self.fw
        for i, v in enumerate([0.0, h, t, m, amounts[0], amounts[1], amounts[2]]): fw.wf(PATCH + 4 * i, v)
        fw.w32(PATCH + 28, engine); fw.wf(PATCH + 32, 0.5); fw.wf(PATCH + 36, 0.5)
    def mods(self, trigger=0.0, trigger_patched=False, note=0.0, level=None, engine_cv=0.0):
        fw = self.fw
        vals = [engine_cv, note, 0.0, 0.0, 0.0, 0.0, trigger, level if level is not None else 0.0]
        for i, v in enumerate(vals): fw.wf(MODS + 4 * i, v)
        fw.uc.mem_write(MODS + 32, bytes([0, 0, 0, 1 if trigger_patched else 0, 1 if level is not None else 0]))
    def set(self, **kw):
        bytes_ = ['density', 'length', 'speed', 'octave', 'chord_model', 'transpose']
        for k, v in kw.items():
            if k == 'envelope': self.fw.wf(DUBS, v)
            elif k == 'resonance': self.fw.wf(DUBS + 4, v)
            else: self.fw.uc.mem_write(DUBS + 8 + bytes_.index(k), bytes([v & 255]))
    def block(self, count=False):
        self.fw.call(self.render, self.voice, PATCH, MODS, DUBS, FRAMES, 12, count=count)
        fr = self.fw.r16s(FRAMES, 24)
        return fr[0::2], fr[1::2], self.fw.icount
