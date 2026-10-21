// Copyright 2016 Emilie Gillet.
//
// Author: Emilie Gillet (emilie.o.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Main synthesis voice.

#include "plaits/dsp/voice.h"
#include "plaits/dsp/oscillator/sine_oscillator.h"
#include "plaits/user_data.h"

namespace plaits {

using namespace std;
using namespace stmlib;

void Voice::Init(BufferAllocator* allocator) {
  engines_.Init();

  engines_.RegisterInstance(&virtual_analog_vcf_engine_, false, 1.0f, 1.0f);
  engines_.RegisterInstance(&phase_distortion_engine_, false, 0.7f, 0.7f);
  engines_.RegisterInstance(&six_op_engine_, true, 1.0f, 1.0f);
  engines_.RegisterInstance(&six_op_engine_, true, 1.0f, 1.0f);
  engines_.RegisterInstance(&six_op_engine_, true, 1.0f, 1.0f);
  engines_.RegisterInstance(&wave_terrain_engine_, false, 0.7f, 0.7f);
  engines_.RegisterInstance(&string_machine_engine_, false, 0.8f, 0.8f);
  engines_.RegisterInstance(&chiptune_engine_, false, 0.5f, 0.5f);
  
  engines_.RegisterInstance(&virtual_analog_engine_, false, 0.8f, 0.8f);
  engines_.RegisterInstance(&waveshaping_engine_, false, 0.7f, 0.6f);
  engines_.RegisterInstance(&fm_engine_, false, 0.6f, 0.6f);
  engines_.RegisterInstance(&grain_engine_, false, 0.7f, 0.6f);
  engines_.RegisterInstance(&additive_engine_, false, 0.8f, 0.8f);
  engines_.RegisterInstance(&wavetable_engine_, false, 0.6f, 0.6f);
  engines_.RegisterInstance(&chord_engine_, false, 0.8f, 0.8f);
  engines_.RegisterInstance(&speech_engine_, false, -0.7f, 0.8f);

  engines_.RegisterInstance(&swarm_engine_, false, -3.0f, 1.0f);
  engines_.RegisterInstance(&noise_engine_, false, -1.0f, -1.0f);
  engines_.RegisterInstance(&particle_engine_, false, -2.0f, 1.0f);
  engines_.RegisterInstance(&string_engine_, true, -1.0f, 0.8f);
  engines_.RegisterInstance(&modal_engine_, true, -1.0f, 0.8f);
  engines_.RegisterInstance(&bass_drum_engine_, true, 0.8f, 0.8f);
  engines_.RegisterInstance(&snare_drum_engine_, true, 0.8f, 0.8f);
  engines_.RegisterInstance(&hi_hat_engine_, true, 0.8f, 0.8f);
  
  for (int i = 0; i < engines_.size(); ++i) {
    // All engines will share the same RAM space.
    allocator->Free();
    engines_.get(i)->Init(allocator);
  }
  
  engine_quantizer_.Init(engines_.size(), 0.05f, true);
  previous_engine_index_ = -1;
  reload_user_data_ = false;
  engine_cv_ = 0.0f;
  
  out_post_processor_.Init();
  aux_post_processor_.Init();

  decay_envelope_.Init();
  lpg_envelope_.Init();
  
  trigger_state_ = false;
  previous_note_ = 0.0f;
  
  trigger_delay_.Init(trigger_delay_line_);

  chord_model_quantizer_.Init(dub::kNumChordModels, 0.05f, true);
  sequencer_.Init();
  poly_synth_.Init();
  post_shaper_.Init();
  mono_note_ = 48.0f;
  chord_note_ = 48.0f;
  cv_note_ = dub::kCvZeroNote;
  tune_phase_ = 0.0f;
  gate_blocks_ = 0;
  active_display_ = 0;
  step_played_ = false;
}

// AUX is a 1 V/oct pitch CV of the chord root (C3 = 0 V), held between chords.
void Voice::WriteCv(const dub::Settings& ds, float note, Frame* frames, size_t size) {
  float volts = (note - dub::kCvZeroNote) * (1.0f / 12.0f);
  // Negative codes give positive volts (the output stage inverts, see
  // ChannelPostProcessor's -32767 gain).
  int32_t code = static_cast<int32_t>(-volts * ds.cv_scale + (volts > 0.0f ? -0.5f : 0.5f));
  short s = static_cast<short>(stmlib::Clip16(code));
  for (size_t i = 0; i < size; ++i) frames[i].aux = s;
}

// Registry indices of the Plaits engines that can play sequencer chords.
static const int kChordEngineIndex = 14;
static const int kStringMachineIndex = 6;

void Voice::Render(
    const Patch& patch,
    const Modulations& modulations,
    dub::Settings& ds,
    Frame* frames,
    size_t size) {
  // V/OCT picks the scale degree (C minor), so chords never leave the scale.
  float semitones = (modulations.note + previous_note_) * 0.5f;
  previous_note_ = modulations.note;
  ds.transpose = dub::SemitonesToDegrees(semitones);

  // ---- Sequencer: TRIG is the clock input.
  sequencer_.ProcessLfo();
  dub::Event event;
  const bool fired = sequencer_.Process(
      ds, modulations.trigger, modulations.trigger_patched, &event);
  step_played_ = fired;
  if (fired) cv_note_ = event.root;

  // CV TUNE: AUX alternates 0 V / +2 V, OUT plays the same notes as a sine.
  if (ds.cv_tune) {
    const float note = ds.cv_tune == 1 ? dub::kCvZeroNote : dub::kCvZeroNote + 24.0f;
    const float f = NoteToFrequency(note);
    for (size_t i = 0; i < size; ++i) {
      tune_phase_ += f;
      if (tune_phase_ >= 1.0f) tune_phase_ -= 1.0f;
      frames[i].out = static_cast<short>(stmlib::Clip16(static_cast<int32_t>(
          -SineNoWrap(tune_phase_) * 12000.0f)));
    }
    WriteCv(ds, note, frames, size);
    return;
  }

  dub::Shape shape;
  dub::ComputeShape(ds, &shape);

  float compressed_level = 1.3f * modulations.level / (0.3f + fabsf(modulations.level));
  CONSTRAIN(compressed_level, 0.0f, 1.0f);
  const float level = modulations.level_patched ? compressed_level : 1.0f;

  // Random LFO, always on (depth 15) on HARMONICS, TIMBRE, MORPH and cutoff.
  const float depth = 0.15f * 0.5f;
  float harmonics = patch.harmonics + modulations.harmonics + sequencer_.lfo(0) * depth;
  CONSTRAIN(harmonics, 0.0f, 1.0f);
  float timbre = patch.timbre + (modulations.timbre_patched ? modulations.timbre : 0.0f) + sequencer_.lfo(1) * depth;
  CONSTRAIN(timbre, 0.0f, 1.0f);
  float morph = patch.morph + (modulations.morph_patched ? modulations.morph : 0.0f) + sequencer_.lfo(2) * depth;
  CONSTRAIN(morph, 0.0f, 1.0f);
  shape.cutoff_hz *= SemitonesToRatio(sequencer_.lfo(3) * depth * 24.0f);

  // ---- Chord model selection (buttons + MODEL CV).
  const int chord_model = chord_model_quantizer_.Process(ds.chord_model, modulations.engine);
  active_display_ = 100 + chord_model;
  const int engine_index = chord_model == 6 ? kChordEngineIndex
      : (chord_model == 7 ? kStringMachineIndex : -1);

  if (engine_index < 0) {
    // ---- Light polyphonic dub chord models.
    if (previous_engine_index_ != -1 - chord_model) {
      out_post_processor_.Reset();
      previous_engine_index_ = -1 - chord_model;
    }
    if (fired) {
      poly_synth_.NoteOn(event, shape);
    }
    poly_synth_.Render(
        chord_model, harmonics, timbre, morph, shape, level,
        out_buffer_, aux_buffer_, size);
    out_post_processor_.Process(1.0f, true, 1.0f, 1.0f, 0.0f,
        out_buffer_, &frames->out, size, 2);
    WriteCv(ds, cv_note_, frames, size);
    return;
  }

  // ---- Plaits CHORDS / STRING MACHINE playing the sequencer's chord.
  Engine* e = engines_.get(engine_index);
  if (engine_index != previous_engine_index_) {
    e->LoadUserData(NULL);
    e->Reset();
    out_post_processor_.Reset();
    aux_post_processor_.Reset();
    previous_engine_index_ = engine_index;
  }
  ChordBank* chords = engine_index == kChordEngineIndex
      ? chord_engine_.mutable_chords()
      : string_machine_engine_.mutable_chords();
  if (fired) {
    post_shaper_.Trigger(shape, event.accent);
    float ratios[dub::kMaxChordNotes];
    chord_note_ = event.notes[0];
    for (int i = 0; i < event.num_notes; ++i) {
      ratios[i] = SemitonesToRatio(event.notes[i] - event.notes[0]);
    }
    chords->set_custom_chord(ratios, event.num_notes);
  }

  EngineParameters p;
  p.trigger = fired ? (TRIGGER_RISING_EDGE | TRIGGER_HIGH) : TRIGGER_LOW;
  p.accent = fired && event.accent ? 1.0f : 0.8f;
  p.harmonics = harmonics;
  p.timbre = timbre;
  p.morph = morph;
  // The chord engine renders two octaves down at TIMBRE = 0 (its inversion).
  p.note = chord_note_ + (engine_index == kChordEngineIndex ? 24.0f : 0.0f);

  bool already_enveloped = e->post_processing_settings.already_enveloped;
  e->Render(p, out_buffer_, aux_buffer_, size, &already_enveloped);

  // HARMONICS has no chord to choose here: it opens the filter. On the
  // CHORDS engine, TIMBRE (inversion) moves the chord up by up to 5 octaves,
  // so the filter follows it (otherwise high inversions were filtered away).
  float cutoff_semitones = (harmonics - 0.5f) * 36.0f;
  if (engine_index == kChordEngineIndex) cutoff_semitones += timbre * 48.0f;
  post_shaper_.Process(shape, level, SemitonesToRatio(cutoff_semitones),
      out_buffer_, aux_buffer_, size);

  const PostProcessingSettings& pp_s = e->post_processing_settings;
  out_post_processor_.Process(pp_s.out_gain, true, 1.0f, 1.0f, 0.0f,
      out_buffer_, &frames->out, size, 2);
  (void)pp_s;
  WriteCv(ds, cv_note_, frames, size);
}

}  // namespace plaits
