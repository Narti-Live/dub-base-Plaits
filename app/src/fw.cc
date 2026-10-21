// DUB BASE app: runs the DUB BASE v1.1 Plaits firmware (ui.cc, voice.cc,
// dub.cc and the Plaits engines, unchanged) on a phone. This file plays the
// part of plaits.cc: it owns the same global objects and calls Ui::Poll()
// and Voice::Render() once per 12-sample block, like FillBuffer() does.

#include "plaits/dsp/voice.h"
#include "plaits/settings.h"
#include "plaits/ui.h"
#include "stmlib/utils/buffer_allocator.h"
#include "stmlib/utils/random.h"

#include "fw.h"

#ifndef DUB_KEY_OFFSET
#define DUB_KEY_OFFSET 0
#endif
#ifndef DUB_MINOR_ONLY
#define DUB_MINOR_ONLY 0
#endif
#ifndef DUB_VOICINGS
#define DUB_VOICINGS 0
#endif
extern "C" void fw_variant(int* key_offset, int* minor_only, int* voicings) {
  *key_offset = DUB_KEY_OFFSET; *minor_only = DUB_MINOR_ONLY; *voicings = DUB_VOICINGS;
}

using namespace plaits;
using namespace stmlib;

extern "C" {
volatile float hw_pots[7] = { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
volatile int hw_buttons[2];
uint32_t hw_leds[8];
}

static Modulations modulations;
static Patch patch;
static Settings settings;
static Ui ui;
static Voice voice;
static dub::Settings dub_settings;
static char shared_buffer[16384];

static volatile int clock_mode;     // 0 = TRIG unpatched (internal 120), 1 = app clock into TRIG
static volatile float clock_bpm = 120.0f;
static double clock_phase;          // in blocks
static int trig_high;               // blocks left in the current trigger pulse
static float block_rate = 4000.0f;
static int initialised;

static float led_avg[8][2];          // PWM-averaged LED brightness (red, green)
static FwSnapshot snap;
static volatile uint32_t snap_seq;
static uint32_t block_counter;
static uint32_t step_counter;
static FwEvent last_event;
static float scope[FW_SCOPE];
static volatile uint32_t scope_pos;
static float meter;

extern "C" void fw_init(uint32_t seed) {
  BufferAllocator allocator(shared_buffer, 16384);
  voice.Init(&allocator);
  Random::Seed(seed);
  settings.Init();
  dub::DefaultSettings(&dub_settings);
  ui.Init(&patch, &modulations, &settings, &dub_settings, &voice);
  initialised = 1;
}

extern "C" void fw_set_clock(int mode, float bpm) {
  clock_mode = mode;
  if (bpm < 30.0f) bpm = 30.0f;
  if (bpm > 240.0f) bpm = 240.0f;
  clock_bpm = bpm;
}

static void TakeSnapshot() {
  snap_seq = snap_seq + 1;
  __atomic_thread_fence(__ATOMIC_SEQ_CST);
  const dub::Pattern& p = voice.mutable_sequencer()->pattern();
  for (int i = 0; i < 16; ++i) {
    snap.degree[i] = p.degree[i];
    snap.rank[i] = p.rank[i];
    snap.accent[i] = p.accent[i];
    float root;
    float notes[dub::kMaxChordNotes];
    snap.voicing[i] = DUB_VOICINGS ? p.note[i] : 0;
    int n = dub::BuildChord(dub_settings, p.degree[i] + dub_settings.transpose, snap.voicing[i], notes, &root);
    for (int k = 0; k < 5; ++k) snap.chord[i][k] = k < n ? static_cast<int8_t>(notes[k]) : -1;
  }
  snap.step = voice.mutable_sequencer()->step();
  snap.density = dub_settings.density;
  snap.length = dub_settings.length;
  snap.speed = dub_settings.speed;
  snap.octave = dub_settings.octave;
  snap.model = dub_settings.chord_model;
  snap.transpose = dub_settings.transpose;
  snap.envelope = dub_settings.envelope;
  snap.resonance = dub_settings.resonance;
  dub::Shape shape;
  dub::ComputeShape(dub_settings, &shape);
  snap.attack_s = shape.attack_s;
  snap.decay_s = shape.decay_s;
  snap.env_oct = shape.env_oct;
  snap.q = shape.q;
  for (int i = 0; i < 4; ++i) snap.lfo[i] = voice.mutable_sequencer()->lfo(i);
  snap.harmonics = patch.harmonics;
  snap.timbre = patch.timbre;
  snap.morph = patch.morph;
  for (int i = 0; i < 8; ++i) { snap.led[i][0] = led_avg[i][0]; snap.led[i][1] = led_avg[i][1]; }
  snap.step_counter = step_counter;
  snap.last = last_event;
  snap.meter = meter;
  snap.cv_tune = dub_settings.cv_tune;
  snap.cv_scale = dub_settings.cv_scale;
  {
    float n = dub_settings.cv_tune ? (dub_settings.cv_tune == 1 ? dub::kCvZeroNote : dub::kCvZeroNote + 24.0f) : voice.cv_note();
    snap.cv_volts = (n - dub::kCvZeroNote) / 12.0f;
  }
  snap.clocked = clock_mode;
  snap.blocks = block_counter;
  __atomic_thread_fence(__ATOMIC_SEQ_CST);
  snap_seq = snap_seq + 1;
}

extern "C" int fw_snapshot(FwSnapshot* out) {
  for (int tries = 0; tries < 8; ++tries) {
    uint32_t a = snap_seq;
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    if (a & 1) continue;
    *out = snap;
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    if (snap_seq == a) return 1;
  }
  return 0;
}

extern "C" void fw_scope(float* out, int n) {
  uint32_t pos = scope_pos;
  for (int i = 0; i < n; ++i) out[i] = scope[(pos + FW_SCOPE - n + i) % FW_SCOPE];
}

static void RunBlock(Voice::Frame* frames) {
  ui.Poll();

  // The TRIG jack: the app clock sends 16th-note pulses (2 ms high).
  const bool patched = clock_mode != 0;
  if (patched) {
    double period = block_rate * 60.0 / (clock_bpm * 4.0);
    clock_phase += 1.0;
    if (clock_phase >= period) {
      clock_phase -= period;
      if (clock_phase > period) clock_phase = 0.0;
      trig_high = 8;
    }
  } else {
    clock_phase = 0.0;
    trig_high = 0;
  }
  // No CV cables on a phone: only TRIG may be "patched".
  modulations.engine = 0.0f;
  modulations.note = 0.0f;
  modulations.frequency = 0.0f;
  modulations.harmonics = 0.0f;
  modulations.timbre = 0.0f;
  modulations.morph = 0.0f;
  modulations.trigger = trig_high > 0 ? 1.0f : 0.0f;
  modulations.level = 1.0f;
  modulations.frequency_patched = false;
  modulations.timbre_patched = false;
  modulations.morph_patched = false;
  modulations.trigger_patched = patched;
  modulations.level_patched = false;
  if (trig_high > 0) --trig_high;

  voice.Render(patch, modulations, dub_settings, frames, kBlockSize);

  if (voice.step_played()) {
    ++step_counter;
    const dub::Sequencer* s = voice.mutable_sequencer();
    int st = s->step();
    last_event.step = st;
    last_event.accent = s->pattern().accent[st];
    last_event.block = block_counter;
    float root;
    float notes[dub::kMaxChordNotes];
    last_event.voicing = DUB_VOICINGS ? s->pattern().note[st] : 0;
    int n = dub::BuildChord(dub_settings, s->pattern().degree[st] + dub_settings.transpose, last_event.voicing, notes, &root);
    last_event.root = static_cast<int>(root);
    last_event.num_notes = n;
    for (int k = 0; k < 5; ++k) last_event.notes[k] = k < n ? static_cast<int8_t>(notes[k]) : -1;
    last_event.degree = (s->pattern().degree[st] + dub_settings.transpose) % 7;
    if (last_event.degree < 0) last_event.degree += 7;
  }

  // The LEDs are driven with PWM by the firmware: average them like an eye would.
  for (int i = 0; i < 8; ++i) {
    float r = (hw_leds[i] & 0xff0000) ? 1.0f : 0.0f;
    float g = (hw_leds[i] & 0x00ff00) ? 1.0f : 0.0f;
    led_avg[i][0] += 0.02f * (r - led_avg[i][0]);
    led_avg[i][1] += 0.02f * (g - led_avg[i][1]);
  }
  ++block_counter;
  if ((block_counter & 15) == 0) TakeSnapshot();
}

// Interleaved stereo float. OUT and AUX are mixed by the gains set by the app.
static Voice::Frame pending[kBlockSize];
static int pending_pos = kBlockSize;
static volatile float out_gain = 1.0f, aux_gain = 0.0f;

extern "C" void fw_set_mix(float out, float aux) { out_gain = out; aux_gain = aux; }

extern "C" void fw_render(float* stereo, int frames, float sample_rate) {
  if (!initialised) {
    for (int i = 0; i < frames * 2; ++i) stereo[i] = 0.0f;
    return;
  }
  block_rate = sample_rate / static_cast<float>(kBlockSize);
  const float go = out_gain * (1.0f / 32768.0f), ga = aux_gain * (1.0f / 32768.0f);
  uint32_t sp = scope_pos;
  for (int i = 0; i < frames; ++i) {
    if (pending_pos >= static_cast<int>(kBlockSize)) {
      RunBlock(pending);
      pending_pos = 0;
    }
    // The module's outputs are inverted by the output stage: flip them back.
    float o = -static_cast<float>(pending[pending_pos].out);
    float a = -static_cast<float>(pending[pending_pos].aux);
    ++pending_pos;
    (void)a; (void)ga;   // AUX = pitch CV (DC), not audio
    float s = o * go;
    if (s > 1.0f) s = 1.0f;
    if (s < -1.0f) s = -1.0f;
    stereo[2 * i] = s;
    stereo[2 * i + 1] = s;
    if (i & 1) { scope[sp] = o * (1.0f / 32768.0f); sp = (sp + 1) % FW_SCOPE; }
    float m = s < 0.0f ? -s : s;
    meter = m > meter ? m : meter * 0.9995f;
  }
  scope_pos = sp;
}

// No C++ runtime on this build.
extern "C" {
void __cxa_pure_virtual() { while (1) { } }
void _ZdlPv(void* p) { (void)p; }
void _ZdlPvm(void* p, unsigned long n) { (void)p; (void)n; }
int atexit(void (*f)(void)) { (void)f; return 0; }
}
