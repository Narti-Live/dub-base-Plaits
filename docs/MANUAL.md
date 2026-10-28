# DUB STAB PLAITS · v1.5

**A dub techno chord sequencer for Mutable Instruments Plaits.**
By **Narti Live** · [Instagram](https://www.instagram.com/narti_live) · [YouTube](https://www.youtube.com/@narti_live)

> 👉 **The visual manual is easier to follow:** [narti-live.github.io/dub-base-Plaits/docs/manual](https://narti-live.github.io/dub-base-Plaits/docs/manual/). It has the numbered panel, sound demos and every control.

> ⚠️ **Unofficial firmware, use at your own risk.** Not made, tested or supported by Mutable Instruments. You can always reinstall the official Plaits firmware.

Send Plaits a clock and it plays generative dub techno stabs, **always in C minor**. One knob is the envelope, three knobs shape the sound, three small knobs shape the groove. The module is dry: patch your own delay and reverb after it.

---

## 1 · Quick start

1. **Flash it.** See [Flash](#7--flash-and-restore).
2. **Patch OUT** to your mixer, through a delay and a reverb.
3. **Clock TRIG** with 16th notes. Nothing patched = 120 BPM.
4. **FREQUENCY at noon** = a dub stab. Turn the small TIMBRE knob (**DENSITY**) to choose how many steps play.
5. **Play.** Tap LEFT / RIGHT for the model. **Hold LEFT** for a new pattern. Twist HARMONICS, TIMBRE, MORPH.

> Start the clock → the pattern starts at step 1. Stop it for 4 clock periods (0.5–4.5 s) → it goes back to step 1 and waits.

---

## 2 · Every control

### Buttons, LEDs, big knobs

| # | Control | Function |
|---|---|---|
| 1 | **LEFT button** | Tap: previous model · Hold 1 s: **NEW pattern** |
| 2 | **RIGHT button** | Tap: next model · Hold 1 s: **chords ↔ single notes** |
| 3 | **8 LEDs** | Playing model (yellow = chords, red = notes), blinks on each step · shows values while you turn |
| 4 | **FREQUENCY** | **Envelope**: left pluck · noon dub stab · right long swell |
| 5 | **HARMONICS** | Model sound 1 (see models) |
| 6 | **TIMBRE** | Model sound 2 |
| 7 | **MORPH** | Model sound 3 |

### Sequencer (small knobs)

| # | Control | Function |
|---|---|---|
| 8 | TIMBRE small knob | **DENSITY**: how many of the 16 steps play |
| 9 | FM small knob | **LENGTH**: 1–16 steps |
| 10 | MORPH small knob | **SPEED**: 1/2 · 1/4 · 1/8 · 1/8T · **1/16** (centre) · 1/16T · 1/32 · 1/32T |

### Jacks

| # | Jack | Function |
|---|---|---|
| 11 | MODEL | Model CV |
| 12 | TIMBRE | Timbre CV (±5 V = ±15 %) |
| 13 | FM | not used |
| 14 | MORPH | Morph CV (±5 V = ±15 %) |
| 15 | HARMO | Harmonics CV (±5 V = ±15 %) |
| 16 | **TRIG** | **Clock in** (16ths) |
| 17 | LEVEL | VCA |
| 18 | V/OCT | Transpose the progression by scale degrees, snapped to the key |
| 19 | **OUT** | Chords (dry) |
| 20 | **AUX** | **Root CV out**, 1 V/oct, C3 = 0 V |

### Combos

| Do this | You get |
|---|---|
| Hold LEFT + turn FREQUENCY | Octave 1–5 (noon = 3) |
| Hold RIGHT + turn FREQUENCY | Resonance (limited, never screams) |
| Hold both buttons 2 s | CV TUNE (4 s: original Plaits calibration) |
| Hold RIGHT at power-on | Colour-blind LED mode |

A slow random LFO is always on and gently drifts HARMONICS, TIMBRE, MORPH and the filter.

---

## 3 · The 8 chord models

| # | Model | HARMONICS | TIMBRE | MORPH |
|---|---|---|---|---|
| 1 | **DUB SAW** · the classic | detune | saw → pulse | noise “chuff” |
| 2 | VA | 2nd osc interval | pulse width | pulse ↔ saw |
| 3 | WAVESHAPE | asymmetry | fold | triangle → saw |
| 4 | FM | ratio | index | feedback |
| 5 | PHASE DIST | resonance | distortion | PD ↔ resonant |
| 6 | **VHIKK** · Vhikk X-inspired | WARP: phase mod ← noon → ring mod | feedback: sine → saw-like | SPACE: tuned echo, past noon spiralling shifter |
| 7 | **STRINGS** · Plaits string machine | brightness | filter / ensemble | registration |
| 8 | **ACID** · mono bass | cutoff | accent | resonance |

- **Hold RIGHT** on any model: chords ↔ single notes (a melody that follows the chords). On ACID: mono line ↔ acid chords.
- On ACID, **FREQUENCY** sets the filter envelope amount.

---

## 4 · How a pattern works

- **NEW** (hold LEFT) writes 1, 2 or 4 chords, mostly i, iv, v, VI and III, ranks every step (off-beats first) and adds accents.
- **DENSITY** plays the steps ranked below it, so the groove grows without reshuffling.
- **LENGTH** loops the first steps; **SPEED** sets steps per clock pulse.
- Model, octave, resonance and pattern are **saved 2 s after a change**.

---

## 5 · CV and AUX

**AUX** sends the root of every chord (1 V/oct, C3 = 0 V, about −2 V to +3 V), held until the next chord. Patch it to a second synth.

**CV TUNE (do it once):**

1. Patch AUX to your synth's V/OCT. Hold **both buttons 2 s**.
2. AUX alternates every 2 s: **C3** (LEDs 1–4 green) and **C5** (LEDs 5–8 green).
3. Tune your synth to C on the low note, then turn **FREQUENCY** until the high note is exactly 2 octaves up.
4. **Tap a button** to save.

If a held note slowly sinks back to 0 V, that module's AUX output is AC-coupled and the CV can't work on it.

---

## 6 · What the LEDs say

| You see | It means |
|---|---|
| One yellow LED | Playing model, chords (blinks off on each step) |
| One red LED | Playing model, single notes |
| Yellow bar | Envelope or length |
| Red bar | Resonance |
| Green bar | Density |
| One LED | Octave (yellow) · speed (green, red = triplets) |
| Green pulse, then a sweep | NEW pattern |

---

## 7 · Flash and restore

Two versions: **C minor** (all chords, DUB voicing) and **Cm DEEP** (open m7, m9, stacked 4ths, sus, fifths, no major chords).

1. Unplug all cables. Audio from your phone or computer into **MODEL**. FREQUENCY at **12 o'clock**.
2. Power on with **LEFT held**.
3. Play `DubStab-Plaits-v1.5.wav` (or `DubStab-Plaits-v1.5-Cm-DEEP.wav`) to the end (2 min 42 s), about 80 % volume, no other sounds.
4. LEDs 1–4 = level meter: both green on, yellow may flicker, red off.
5. The module restarts by itself.

**All LEDs blink red?** Press LEFT, adjust the volume, play again. **Back to stock?** Flash the official Plaits `.wav` the same way; calibration is kept.

The **Dub Stab Plaits** Android app runs the same code (plus an app-only echo and reverb), so you can try it on your phone first.

---

## 8 · Versions

| Version | What changed |
|---|---|
| **v1.5** | 8–17 % less CPU on the busiest models. Same sound. |
| v1.4 | No soft clipping with long envelopes. |
| v1.3 | Model 6 is VHIKK instead of CHIPTUNE. |
| v1.2 | Starting the clock starts the pattern at step 1. |
| v1.1 | Hold RIGHT: chords / single notes, acid chords. Cm DEEP version. |
| v1.0 | First release. |

*Based on the open-source Plaits firmware by Émilie Gillet (MIT). VHIKK is original code, inspired by the Forge TME Vhikk X and not affiliated with Forge TME. Tested in a Cortex-M4 emulator and on a Plaits clone.*
