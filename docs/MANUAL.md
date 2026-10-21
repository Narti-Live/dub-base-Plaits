# DUB BASE for Plaits — v1.4

By **Narti Live** · [Instagram @narti_live](https://www.instagram.com/narti_live) · [YouTube @narti_live](https://www.youtube.com/@narti_live)

> **Unofficial firmware — use at your own risk.** Not made, tested or supported by Mutable Instruments. Provided as is, without warranty. You can always reinstall the official Plaits firmware.

A dub-techno chord sequencer inside Mutable Instruments Plaits.
**TRIG is the clock.** Every chord is a **C minor DUB chord** (root, 5th, 7th, then the 3rd and
5th an octave up), so no note ever leaves C minor. Always in chord mode. No effects; patch
your own delay and reverb after it. Based on the open-source Plaits firmware by Émilie Gillet (MIT).

## Panel

| Control | Function |
|---|---|
| **FREQUENCY** | **Envelope**: attack, decay and filter envelope on one knob. Left = short dark pluck, noon = dub stab, right = long swell with a big filter sweep. |
| **LEFT held + FREQUENCY** | **Octave** 1–5 (noon = 3) |
| **RIGHT held + FREQUENCY** | **Resonance** (limited: it never self-oscillates) |
| **HARMONICS / TIMBRE / MORPH** | Sound of the chord model (table below) |
| **TIMBRE attenuverter** | **Density**: how many of the 16 steps play (1–16) |
| **FM attenuverter** | **Length** of the pattern (1–16 steps) |
| **MORPH attenuverter** | **Speed**: 1/2, 1/4, 1/8, **1/8T**, 1/16 (centre), **1/16T**, 1/32, **1/32T** |
| Tap **LEFT / RIGHT** | Previous / next chord model |
| Hold **LEFT** 1 s | **NEW** pattern (LEDs pulse green, then sweep) |
| Hold **RIGHT** 1 s | **RANDOM** sound: new HARMONICS / TIMBRE / MORPH (the knobs pick up again when you turn them) |

A slow random LFO is always on (speed 20, depth 15). It gently drifts HARMONICS, TIMBRE,
MORPH and the filter.

Hold **both buttons 2 s** for **CV TUNE** (see below). Keep holding for 2 s more for the original
Plaits CV input calibration. The colour-blind LED mode is still there (hold RIGHT at power-on).

## AUX pitch CV and CV TUNE

AUX sends the **root note** of every chord that plays (1 V/oct, C3 = 0 V, about −2 V to +3 V), and holds it
until the next chord: patch it to another synth's V/OCT and clock that synth's envelope from the
same clock you send to TRIG. Every root is a C minor note.

The output level differs a little from module to module, so tune the CV once:

1. Patch AUX to your synth's V/OCT. Hold **both buttons for 2 s**.
2. AUX now alternates every 2 s: **C3 (0 V)** with LEDs 1–4 green, and **C5 (+2 V)** with LEDs 5–8 green.
   OUT plays the same two notes as a sine, if you want to tune by ear.
3. Tune your synth so the low note is a C, then turn **FREQUENCY** until the high note is exactly two octaves
   above it. The red LED shows how far the scale is trimmed (the envelope is not changed).
4. **Tap a button** to save and go back to playing. The scale is kept with the calibration data.

Note: this assumes the AUX output passes steady voltages (DC). If the CV slowly drifts back to 0 V
while a note is held, the output stage is AC-coupled and the CV cannot work on that module.

## Jacks

| Jack | Function |
|---|---|
| **TRIG** | Clock in, counted as 16ths. Unpatched: internal 120 BPM. |
| **V/OCT** | Picks the chord's scale degree (D = II, Eb = III …). It is snapped to C minor, so chords stay in the scale. |
| **MODEL** | Selects the chord model |
| **HARMONICS / TIMBRE / MORPH** | CV for those parameters |
| **LEVEL** | VCA |
| **FM** | not used |
| **OUT** | Chords |
| **AUX** | **Pitch CV out**: the root note of each chord, 1 V/oct, C3 = 0 V, held until the next chord. Tune it once with CV TUNE. |

## LEDs

The chord model is shown on a yellow LED (the rest glow dim green); it blinks off on every
step that plays. When you move a control, the LEDs show its value for a second:
envelope = yellow bar, resonance = red bar, density = green bar, length = yellow bar,
octave = one yellow LED, speed = one LED (straight green, triplets red).

## Chord models

| # | Model | HARMONICS | TIMBRE | MORPH |
|---|---|---|---|---|
| 1 | **DUB SAW** (saw > pulse) | detune | saw → pulse | noise "chuff" on each stab |
| 2 | VA | 2nd oscillator interval | pulse width | pulse ↔ saw |
| 3 | WAVESHAPE | asymmetry | fold | triangle → saw |
| 4 | FM | ratio | index | feedback |
| 5 | PHASE DIST | resonance | distortion | PD ↔ resonant |
| 6 | CHIPTUNE | duty cycle | bit crush | square ↔ triangle |
| 7 | Plaits **CHORDS** engine | brightness | inversion | waveform |
| 8 | Plaits **STRING MACHINE** | brightness | filter / ensemble | registration |

NEW writes 1, 2 or 4 chords per pattern, mostly held, on C minor degrees (mostly I, IV, V,
VI, III). Off-beats get priority, so low densities give classic off-beat stabs. Some steps are accented.
Chord model, octave, resonance and the pattern are saved 2 s after a change.

## Installing

Unplug all cables. Patch your audio output into **MODEL** and set FREQUENCY to 12 o'clock.
Power on with the **left** button held, then play `DubBase-Plaits-v1.4.wav` to the end (2 min 36 s).
LEDs 1–4 are a level meter: both green LEDs lit is right, yellow may flicker, red should stay off (computer volume around 80 %). If all LEDs blink red, press the left button,
adjust FREQUENCY and play the file again. To go back, install the official Plaits WAV
the same way; your calibration is kept.

*Tested in a Cortex-M4 emulator (boot, panel, flash save/restore, audio, CPU) and running on a Plaits clone.*

## Changes

**v1.4**
- Cleaner, less hot output. A leveler with 6 dB of headroom keeps stacked chords and high resonance out of the clipper (distortion was up to 28 %, now under 0.5 %). OUT peaks around 10 Vpp, like other Eurorack oscillators.

**v1.3**
- AUX is now a 1 V/oct pitch CV of the chord root (the sub-bass and the engines' AUX signal are gone), with CV TUNE.
- Both buttons 2 s = CV TUNE; 4 s = CV input calibration.

**v1.2**
- Model 7 (CHORDS): the filter now follows TIMBRE (inversion), so high inversions are no longer filtered almost silent.
- The random LFO starts moving right after power-on instead of after about 8 seconds.

**v1.1**
- First public version: fixed C minor, DUB chords, one-knob envelope, sequencer on the attenuverters.

The **Dub Plaits** Android app runs this same code, so you can try the module on your phone first. It adds an app-only tape echo (tempo-synced, wow and flutter, darkening repeats) and a reverb; the module itself stays dry.
