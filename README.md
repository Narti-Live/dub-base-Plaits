# DUB STAB PLAITS — generative dub techno stabs for Plaits

**Dub techno chords from a Mutable Instruments Plaits.** Clocked from TRIG, always in C minor,
one knob for the envelope, and a 1 V/oct root-note CV on AUX. Plus **Dub Stab Plaits**, an Android
app that runs the same firmware code so you can try it on your phone.

**Website:** https://narti-live.github.io/dub-base-Plaits/  
**By Narti Live** — [Instagram @narti_live](https://www.instagram.com/narti_live) · [YouTube @narti_live](https://www.youtube.com/@narti_live)

> **Unofficial firmware — use at your own risk.** DUB STAB PLAITS is not made, tested or supported by Mutable Instruments.
> It has been tested in an emulator and on a Plaits clone, not on every hardware revision. It is provided as is,
> without warranty (see [LICENSE](LICENSE)). You can always go back by installing the official Plaits firmware WAV.

| Download | |
|---|---|
| [`DubStab-Plaits-v1.5.wav`](downloads/DubStab-Plaits-v1.5.wav) | Firmware update for the module (play it into Plaits) |
| [`DubStab-Plaits-v1.5.apk`](downloads/DubStab-Plaits-v1.5.apk) | Android app (Android 8+), with tape echo + reverb |
| [`DubStab-Plaits-v1.5-Cm-DEEP.wav`](downloads/DubStab-Plaits-v1.5-Cm-DEEP.wav) · [`.apk`](downloads/DubStab-Plaits-v1.5-Cm-DEEP.apk) | **C minor DEEP**: open dub techno chords (m7, m9, stacked 4ths, sus, fifths), no major chords |
| [`DubStab-M4L-v1.0.zip`](downloads/DubStab-M4L-v1.0.zip) | Max for Live devices: Dub Stab MIDI, Dub Stab Synth, Dub Stab Plaits (root, four minor scales, minor chords only) |

Also: the [visual manual](https://narti-live.github.io/dub-base-Plaits/docs/manual/), [`MANUAL.md`](docs/MANUAL.md) and the raw `.bin` image for SWD / ST-Link in [`downloads/`](downloads/) (app at `0x08008000`). Beta versions for testers are on the website (password protected).

## Install in one minute

1. Power off, unplug all cables from Plaits, patch your computer's headphone output into **MODEL**.
2. Set **FREQUENCY** to 12 o'clock, hold the **LEFT** button and power on.
3. Play `DubStab-Plaits-v1.5.wav` at about 80 % volume (no MP3, no notification sounds), or use the flasher on the website.
   The level meter (LEDs 1–4) should show both green LEDs lit, yellow at most flickering, red off. When it ends, the module restarts into DUB STAB PLAITS.

All LEDs blinking red: press LEFT, nudge FREQUENCY and play the file again.
Going back: install the official Plaits WAV the same way.

## Panel at a glance

| Control | Function |
|---|---|
| FREQUENCY | Envelope: attack + decay + filter envelope |
| LEFT + FREQUENCY / RIGHT + FREQUENCY | Octave 1–5 / resonance |
| HARMONICS · TIMBRE · MORPH | Sound of the chord model |
| Attenuverters | Density · length · speed (with triplets) |
| Tap LEFT / RIGHT | Previous / next chord model |
| Hold LEFT 1 s | NEW pattern |
| Hold RIGHT 1 s | Chords / single notes for the playing model (LED yellow / red) |
| Both buttons 2 s | CV TUNE for the AUX pitch CV |

## Repository

```
index.html, assets/     the website (GitHub Pages)
downloads/              firmware WAV + BIN, Android APK
docs/MANUAL.md          manual
firmware/patches/       the changes against pichenettes/eurorack (git format-patch)
firmware/files/         the same changes as plain files (plaits/…)
firmware/build_fw.sh    clang / lld build (no arm-none-eabi-gcc needed)
firmware/emu/           Cortex-M4 emulator tests (Unicorn): panel, flash, CV, CPU
app/                    Dub Stab Plaits for Android: firmware core + touch panel, no NDK / SDK needed
```

### Build the firmware

The firmware source here is an early development version (the patch series in `firmware/patches`). The full v1.0 source (ACID model, attenuated CV) will be published together with the current beta.


```sh
git clone https://github.com/pichenettes/eurorack && cd eurorack
git checkout f4cfa76 && git submodule update --init
git am ../dub-base-Plaits/firmware/patches/*.patch
cd .. && ./dub-base-Plaits/firmware/build_fw.sh eurorack build_dub      # needs clang, lld, llvm compiler-rt sources in ./llvm
python3 dub-base-Plaits/firmware/tools/qpsk_encoder3.py -t stm32f3 -k -s 48000 -b 12000 -c 6000 -p 256 \
    build_dub/plaits.bin -o DubStab-Plaits.wav
```

### Build the app

```sh
FW=/path/to/patched/eurorack app/tools/build_lib.sh            # clang + lld, arm64-v8a and x86_64
APK_LIBDIR=lib-c python3 app/tools/mkapk.py app app/build/DubPlaits.apk   # makes its own signing key in app/keys/
```

## Credits

Based on the open-source [Plaits](https://github.com/pichenettes/eurorack) firmware by Émilie Gillet (MIT).
Not affiliated with or endorsed by Mutable Instruments. Tested in a Cortex-M4 emulator and on a Plaits clone.
