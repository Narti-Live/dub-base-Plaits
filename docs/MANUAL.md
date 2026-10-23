# DUB STAB PLAITS — v1.3

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
| Hold **RIGHT** 1 s | **Chords / single notes** for the model that is playing (each model remembers it). Single notes = one note per step, a melody in the scale that follows the chord progression. ACID starts on its mono line; hold RIGHT for **acid chords**. The model LED is **yellow** for chords, **red** for single notes. |

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
| **TRIG** | Clock in, counted as 16ths. Starting the clock starts the pattern from step 1: when the clock stops (no pulse for 4 clock periods, 0.5–4.5 s) the sequencer goes back to the first step and waits. Plugging the cable in does the same. Unpatched: internal 120 BPM. |
| **V/OCT** | Picks the chord's scale degree (D = II, Eb = III …). It is snapped to C minor, so chords stay in the scale. |
| **MODEL** | Selects the chord model |
| **HARMONICS / TIMBRE / MORPH** | CV for those parameters, attenuated: a full ±5 V swing moves them ±15 % around the knob position (1 V = 3 %) |
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
| 6 | **VHIKK** (Vhikk X-inspired) | WARP: left phase modulation, noon clean, right ring modulation | carrier feedback: sine → saw-like | SPACE: tuned dark comb, past noon frequency-shifted spiralling repeats |
| 7 | Plaits **STRING MACHINE** | brightness | filter / ensemble | registration |
| 8 | **ACID** (mono bass) | cutoff | accent | resonance |

**ACID (model 8)** plays a mono acid bass line generated with the pattern: chord roots, octaves,
fifths and thirds, one octave above the chords, with slides and the pattern's accents. Every note
is in the key. On ACID, **FREQUENCY sets the filter envelope amount**; decay is fixed, the wave is
a saw and the character warm. Accents are louder and snappier, and repeated accents build up a
"sweep". Resonance goes up to a strong squelch but never self-oscillates, and the level matches
the chord models. (RIGHT + FREQUENCY resonance applies to the chord models only.)

NEW writes 1, 2 or 4 chords per pattern, mostly held, on C minor degrees (mostly I, IV, V,
VI, III). Off-beats get priority, so low densities give classic off-beat stabs. Some steps are accented.
Chord model, octave, resonance and the pattern are saved 2 s after a change.

## Installing

Unplug all cables. Patch your audio output into **MODEL** and set FREQUENCY to 12 o'clock.
Power on with the **left** button held, then play `DubStab-Plaits-v1.3.wav` (or `DubStab-Plaits-v1.3-Cm-DEEP.wav`) to the end (2 min 37 s).
LEDs 1–4 are a level meter: both green LEDs lit is right, yellow may flicker, red should stay off (computer volume around 80 %). If all LEDs blink red, press the left button,
adjust FREQUENCY and play the file again. To go back, install the official Plaits WAV
the same way; your calibration is kept.

*Tested in a Cortex-M4 emulator (boot, panel, flash save/restore, audio, CPU) and running on a Plaits clone.*

## Version

**v1.3** — model 6 is VHIKK (Vhikk X-inspired pair + tuned, frequency-shifting space) instead of CHIPTUNE.

**v1.2** — starting the TRIG clock starts the sequencer from step 1 (both versions).

**v1.1** — hold RIGHT: chords / single notes per model, acid chords.
  Also as **C minor DEEP** (`DubStab-Plaits-v1.1-Cm-DEEP.wav`): open dub techno shapes in root position — DUB m7, open m9, stacked 4ths (m11), 7sus4, sus2, fifths, open minor, m(add9). One shape per pattern most of the time; roots mostly i, iv, v, with VII and VI; on III, VI and VII the shapes become sus chords, so no major chord sounds.

**v1.0** — first release.

The **Dub Stab Plaits** Android app runs this same code, so you can try the module on your phone first. It adds an app-only tape echo (tempo-synced, wow and flutter, darkening repeats) and a reverb; the module itself stays dry.
