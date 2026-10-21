"""End-to-end panel test of DUB BASE v1.1 (simplified layout) in the emulator."""
import struct
from armemu import *

import os
B = os.environ.get('BUILD', '../build_dub')
ELF, BIN = B + '/plaits.elf', B + '/plaits.bin'
KEY = int(os.environ.get('KEY', '0'))
SCALE_PCS = tuple((KEY + x) % 12 for x in (0, 2, 3, 5, 7, 8, 10))
S = 0x2000C000
ALLOC, FRAMES = S, S + 0x200
GPIOB_IDR = 0x48000410
OFF_LEDS, OFF_POTS, OFF_SEQ = 68, 52, 8528
POT = dict(FREQ=0, HARM=1, TIMBRE=2, MORPH=3, T_ATT=4, FM_ATT=5, M_ATT=6)
COL = {0: '.', 0xff0000: 'R', 0x00ff00: 'G', 0xffff00: 'Y'}
BYTES = ['density', 'length', 'speed', 'octave', 'chord_model', 'transpose']

class Panel:
    def __init__(self, flash_image=None):
        fw = self.fw = Firmware(ELF, BIN, False)
        fw.uc.mem_write(0x08004000, flash_image if flash_image else b'\xff' * 0x3000)
        sym = fw.sym
        self.voice, self.ui, self.patch, self.mods = sym['voice'], sym['ui'], sym['patch'], sym['modulations']
        self.settings, self.dub = sym['settings'], sym['dub_settings']
        fw.w32(ALLOC, sym['shared_buffer']); fw.w32(ALLOC + 4, sym['shared_buffer']); fw.w32(ALLOC + 8, 16384); fw.w32(ALLOC + 12, 16384)
        fw.call('_ZN6plaits5Voice4InitEPN6stmlib15BufferAllocatorE', self.voice, ALLOC)
        fw.call(fw.find('_ZN6plaits8Settings4InitEv'), self.settings)
        fw.call(fw.find('_ZN6plaits3dub15DefaultSettings'), self.dub)
        self.buttons = [False, False]
        self.set_gpio()
        for drv in ('_ZN6plaits5CvAdc4InitEv', '_ZN6plaits7PotsAdc4InitEv'):
            fw.skip(drv)
        fw.call(fw.find('_ZN6plaits2Ui4InitE'), self.ui, self.patch, self.mods, self.settings, self.dub, self.voice)
        self.poll = fw.find('_ZN6plaits2Ui4PollEv')
        self.render = fw.find('_ZN6plaits5Voice6RenderE')
        self.pots = [0.5] * 7
        self.clock = False
        self.voct = 0.0
        self.block_no = 0
    def set_gpio(self):
        v = 0xffff
        if self.buttons[0]: v &= ~(1 << 7)
        if self.buttons[1]: v &= ~(1 << 6)
        self.fw.w32(GPIOB_IDR, v)
    def run_ms(self, ms, period=500):
        for _ in range(ms * 4):
            self.set_gpio()
            for i, p in enumerate(self.pots):
                self.fw.uc.mem_write(self.ui + OFF_POTS + 2 * i, struct.pack('<H', int(max(0, min(1, p)) * 65535)))
            self.fw.call(self.poll, self.ui)
            trig = 1.0 if self.clock and (self.block_no % period) < 8 else 0.0
            for i in range(8): self.fw.wf(self.mods + 4 * i, 0.0)
            self.fw.wf(self.mods + 4 * 6, trig)
            self.fw.wf(self.mods + 4 * 1, self.voct)
            self.fw.uc.mem_write(self.mods + 32, bytes([0, 0, 0, 1 if self.clock else 0, 0]))
            self.fw.call(self.render, self.voice, self.patch, self.mods, self.dub, FRAMES, 12)
            self.block_no += 1
    def press(self, b, ms):
        self.buttons[b] = True; self.run_ms(ms); self.buttons[b] = False; self.run_ms(30)
    def leds(self):
        return ''.join(COL.get(struct.unpack('<I', bytes(self.fw.uc.mem_read(self.ui + OFF_LEDS + 4 * i, 4)))[0], '?') for i in range(8))
    def leds_over(self, ms):
        seen = set()
        for _ in range(ms):
            self.run_ms(1); seen.add(self.leds())
        return seen
    def get(self, name):
        if name == 'envelope': return round(self.fw.rf(self.dub), 3)
        if name == 'resonance': return round(self.fw.rf(self.dub + 4), 3)
        v = self.fw.uc.mem_read(self.dub + 8 + BYTES.index(name), 1)[0]
        return v - 256 if name == 'transpose' and v > 127 else v
    def patch_f(self, i): return round(self.fw.rf(self.patch + 4 * i), 3)
    def pattern(self): return bytes(self.fw.uc.mem_read(self.voice + OFF_SEQ, 64))
    def mode(self): return self.fw.uc.mem_read(self.ui, 1)[0]

ok = True
def check(cond, msg):
    global ok
    print(('  PASS ' if cond else '  FAIL ') + msg)
    ok = ok and cond

p = Panel()
p.run_ms(1500)
check(p.get('chord_model') == 0 and p.leds()[0] == 'Y', 'boot: chord model 1 on a yellow LED (%s)' % p.leds())
p.press(1, 100); check(p.get('chord_model') == 1, 'tap RIGHT -> next model')
p.press(0, 100); p.press(0, 100); check(p.get('chord_model') == 7, 'tap LEFT twice -> wraps to model 8')
p.press(1, 100); check(p.get('chord_model') == 0, 'tap RIGHT -> model 1')

# FREQUENCY = envelope
p.pots[POT['FREQ']] = 0.1; p.run_ms(500); e1 = p.get('envelope')
check(abs(e1 - 0.1) < 0.02, 'FREQUENCY sets the envelope (%.2f)' % e1)
check('Y' in p.leds(), 'envelope shown as a yellow bar (%s)' % p.leds())
p.pots[POT['FREQ']] = 0.8; p.run_ms(500)
check(abs(p.get('envelope') - 0.8) < 0.02, 'envelope follows the knob (%.2f)' % p.get('envelope'))

# LEFT + FREQUENCY = octave
p.buttons[0] = True; p.run_ms(100)
p.pots[POT['HARM']] = 0.9; p.run_ms(300)
check(abs(p.patch_f(1) - 0.9) < 0.03, 'HARMONICS still works while LEFT is held (%.2f)' % p.patch_f(1))
p.pots[POT['FREQ']] = 1.0; p.run_ms(300)
check(p.get('octave') == 5, 'LEFT + FREQUENCY -> octave 5 (%d)' % p.get('octave'))
check(p.leds()[4] == 'Y', 'octave 5 shown on LED 5 (%s)' % p.leds())
check(abs(p.get('envelope') - 0.8) < 0.02, 'envelope held while editing octave (%.2f)' % p.get('envelope'))
p.pots[POT['FREQ']] = 0.5; p.run_ms(300)
check(p.get('octave') == 3, 'octave 3 at noon (%d)' % p.get('octave'))
p.buttons[0] = False; p.run_ms(60)
check(p.get('chord_model') == 0, 'releasing after an edit does not navigate')

# RIGHT + FREQUENCY = resonance
p.buttons[1] = True; p.run_ms(100)
p.pots[POT['FREQ']] = 1.0; p.run_ms(300)
check(p.get('resonance') > 0.97, 'RIGHT + FREQUENCY -> resonance up (%.2f)' % p.get('resonance'))
p.pots[POT['FREQ']] = 0.4; p.run_ms(300)
p.buttons[1] = False; p.run_ms(60)
check(abs(p.get('resonance') - 0.4) < 0.03 and p.get('chord_model') == 0, 'resonance kept, no navigation (%.2f)' % p.get('resonance'))

# attenuverters
p.pots[POT['T_ATT']] = 0.0; p.run_ms(300); d0 = p.get('density')
p.pots[POT['T_ATT']] = 1.0; p.run_ms(300); d1 = p.get('density')
check(d0 == 1 and d1 == 16, 'TIMBRE attenuverter -> density 1..16 (%d, %d)' % (d0, d1))
check(p.leds() == 'GGGGGGGG', 'density 16 shown as a full green bar (%s)' % p.leds())
p.pots[POT['FM_ATT']] = 0.0; p.run_ms(300); l0 = p.get('length')
p.pots[POT['FM_ATT']] = 1.0; p.run_ms(300); l1 = p.get('length')
check(l0 == 1 and l1 == 16, 'FM attenuverter -> length 1..16 (%d, %d)' % (l0, l1))
sp = []
for v in (0.0, 0.5, 0.36, 0.64, 1.0, 0.5):
    p.pots[POT['M_ATT']] = v; p.run_ms(300); sp.append((p.get('speed'), p.leds()))
check(sp[0][0] == 0 and sp[1][0] == 4 and sp[4][0] == 7 and sp[5][0] == 4, 'MORPH attenuverter -> speed 1/2 .. 1/16 at noon .. 1/32T (%s)' % [s[0] for s in sp])
check(any(s[0] in (3, 5, 7) and 'R' in s[1] for s in sp), 'triplet speeds shown in red (%s)' % sp)
p.pots[POT['M_ATT']] = 0.5; p.run_ms(300)
p.pots[POT['T_ATT']] = 0.3; p.run_ms(300)

# NEW and RANDOM
p.clock = True
p.run_ms(200)
pat0 = p.pattern()
p.buttons[0] = True; p.run_ms(1000); armed = ''.join(p.leds_over(150)); p.buttons[0] = False
seen = p.leds_over(300)
check(p.pattern() != pat0, 'hold LEFT 1 s -> NEW pattern')
check('G' in armed and any(s.count('G') >= 3 for s in seen), 'armed display green, then a green sweep (%s)' % armed)
check(p.get('chord_model') == 0, 'NEW did not navigate')
h0 = p.patch_f(1)
p.buttons[1] = True; p.run_ms(1000); armed = ''.join(p.leds_over(150)); p.buttons[1] = False; p.run_ms(100)
check(p.patch_f(1) != h0, 'hold RIGHT 1 s -> RANDOM sound (HARMONICS %.2f -> %.2f)' % (h0, p.patch_f(1)))
check('Y' in armed and p.get('chord_model') == 0, 'armed display yellow, no navigation (%s)' % armed)

# both buttons briefly: nothing happens
pat1 = p.pattern(); m1 = p.get('chord_model')
p.buttons = [True, True]; p.run_ms(300); p.buttons = [False, False]; p.run_ms(100)
check(p.pattern() == pat1 and p.get('chord_model') == m1, 'tapping both buttons does nothing')

# V/OCT moves the chord by scale degrees
p.voct = 2.0; p.run_ms(50)
check(p.get('transpose') == 1, 'V/OCT +2 semitones -> one scale degree up (%d)' % p.get('transpose'))
p.voct = 0.0; p.run_ms(50)

# audio
peak = 0
for _ in range(400):
    p.run_ms(1)
    fr = p.fw.r16s(FRAMES, 24); peak = max(peak, max(abs(x) for x in fr[0::2]))
check(peak > 1000, 'OUT carries audio while clocked (peak %d)' % peak)
p.clock = False

# save + power cycle
p.press(1, 100); p.press(1, 100)   # model 3
p.run_ms(2400)
flash = bytes(p.fw.uc.mem_read(0x08004000, 0x3000))
check(struct.pack('<I', 0x32425544) in flash, 'state chunk (DUB2) written to flash')
saved = (p.get('chord_model'), p.get('octave'), p.get('resonance'), p.pattern()[:48])
q = Panel(flash); q.run_ms(300)
restored = (q.get('chord_model'), q.get('octave'), q.get('resonance'), q.pattern()[:48])
check(saved[0] == restored[0] and saved[1] == restored[1] and abs(saved[2] - restored[2]) < 0.01 and saved[3][:16] == restored[3][:16],
      'model %d, octave %d, resonance %.2f and the pattern restored after power cycle' % restored[:3])

# ---- v1.3: AUX = root note pitch CV (1 V/oct, C3 = 0 V)
SCALE = 6553.6
def aux(pp): return pp.fw.r16s(FRAMES, 24)[1::2]
def out(pp): return pp.fw.r16s(FRAMES, 24)[0::2]
q.clock = True
notes = set(); steady = True
for _ in range(1500):
    q.run_ms(1)
    a = aux(q)
    if len(set(a)) != 1: steady = False
    notes.add(round(-a[0] / SCALE * 12 + 48, 3))
q.clock = False
inscale = all(abs(n - round(n)) < 0.01 and (round(n) % 12) in SCALE_PCS for n in notes)
check(steady and inscale and len(notes) >= 1, 'AUX holds a steady 1 V/oct root note, always in the key (%s)' % sorted(notes))

q.buttons = [True, True]; q.run_ms(2200); q.buttons = [False, False]; q.run_ms(50)
check(q.mode() == 6, 'holding both buttons 2 s enters CV TUNE (mode %d)' % q.mode())
q.run_ms(200); a0 = aux(q)[0]; o = max(abs(x) for x in out(q))
q.run_ms(2000); a1 = aux(q)[0]
check(a0 == 0 and abs(a1 + 2 * SCALE) < 1, 'CV TUNE: AUX alternates 0 V / +2 V (codes %d, %d)' % (a0, a1))
peak = 0
for _ in range(50): q.run_ms(1); peak = max(peak, max(abs(x) for x in out(q)))
check(peak > 5000, 'CV TUNE: OUT plays a reference tone (peak %d)' % peak)
env0 = q.get('envelope')
q.pots[POT['FREQ']] = 0.6; q.run_ms(600)
q.run_ms(2000 - (q.block_no // 4) % 2000 + 100)
codes = sorted(set([aux(q)[0]] + [ (q.run_ms(1000), aux(q)[0])[1] for _ in range(4)]))
check(min(codes) < -2 * SCALE * 1.05, 'FREQUENCY trims the CV scale (+2 V code %d)' % min(codes))
check(abs(q.get('envelope') - env0) < 0.01, 'envelope untouched while tuning')
q.press(0, 100); q.run_ms(100)
q.clock = True; q.run_ms(1500); q.clock = False
check(q.mode() == 0 and -aux(q)[0] / SCALE * 12 + 48 not in (48.0 + 24,) and q.fw.uc.mem_read(q.dub + 14, 1)[0] == 0,
      'tap saves and leaves CV TUNE, AUX back to the chord root')
flash = bytes(q.fw.uc.mem_read(0x08004000, 0x3000))
check(struct.pack('<I', 0x43534356) in flash, 'CV scale saved with the calibration data')
r = Panel(flash); r.run_ms(300)
r.buttons = [True, True]; r.run_ms(2200); r.buttons = [False, False]; r.run_ms(2300)
check(aux(r)[0] < -2 * SCALE * 1.05, 'CV scale restored after power cycle (+2 V code %d)' % aux(r)[0])
r.press(1, 100); r.run_ms(50)
r.buttons = [True, True]; r.run_ms(4400); r.buttons = [False, False]; r.run_ms(50)
check(r.mode() == 2, 'holding both buttons 4 s enters the CV input calibration (mode %d)' % r.mode())
print('ALL PASS' if ok else 'SOME CHECKS FAILED')
