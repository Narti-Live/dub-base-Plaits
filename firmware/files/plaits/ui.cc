// Copyright 2016 Emilie Gillet. DUB BASE changes 2026 Narti Live.
// MIT license, see the original Plaits sources.
//
// DUB BASE panel:
//   FREQUENCY            envelope (attack + decay + filter envelope amount)
//   LEFT  + FREQUENCY    octave
//   RIGHT + FREQUENCY    resonance
//   HARMONICS / TIMBRE / MORPH   chord model parameters
//   TIMBRE / FM / MORPH attenuverters   density / length / speed
//   tap LEFT / RIGHT     previous / next chord model
//   hold LEFT 1 s        NEW pattern
//   hold RIGHT 1 s       RANDOM sound
//   hold both 2 s        CV calibration (original Plaits procedure)

#include "plaits/ui.h"

#include <algorithm>

#include "stmlib/dsp/dsp.h"
#include "stmlib/dsp/units.h"
#include "stmlib/system/system_clock.h"
#include "stmlib/utils/random.h"

namespace plaits {

using namespace std;
using namespace stmlib;

static const int kTapTime = 600;          // ms: shorter press = previous / next
static const int kHoldTime = 1000;        // ms: longer press = NEW / RANDOM
static const int kCalibrationTime = 2000; // ms: both held = CV TUNE (again: input calibration)
static const int kTunePhase = 2000;       // ms per reference note in CV TUNE
static const int kSaveDelay = 2000;       // ms after the last change
static const int kFlashTime = 450;
static const int kShowTime = 1000;

void Ui::Init(Patch* patch, Modulations* modulations, Settings* settings,
              dub::Settings* dub_settings, Voice* voice) {
  patch_ = patch;
  modulations_ = modulations;
  settings_ = settings;
  dub_ = dub_settings;
  voice_ = voice;

  cv_adc_.Init();
  pots_adc_.Init();
  leds_.Init();
  switches_.Init();

  ui_task_ = 0;
  mode_ = UI_MODE_NORMAL;

  for (int i = 0; i < POTS_ADC_CHANNEL_LAST; ++i) {
    pots_[i].Init(i <= POTS_ADC_CHANNEL_HARMONICS_POT ? 0.005f : 0.01f);
  }
  density_quantizer_.Init(16, 0.25f, true);
  length_quantizer_.Init(16, 0.25f, true);

  LoadState();
  float scale;
  if (settings_->cv_scale(&scale)) dub_->cv_scale = scale;
  tune_time_ = 0;
  tune_moved_ = false;

  if (switches_.pressed_immediate(SWITCH_ROW_2)) {
    State* state = settings_->mutable_state();
    state->color_blind = state->color_blind == 1 ? 0 : 1;
    settings_->SaveState();
    ignore_release_[0] = ignore_release_[1] = true;
  }

  normalization_detection_count_ = 0;
  normalization_probe_state_ = 0;
  normalization_probe_.Init();
  fill(&normalization_detection_mismatches_[0],
       &normalization_detection_mismatches_[kNumNormalizedChannels], 0);

  pwm_counter_ = 0;
  fill(&press_time_[0], &press_time_[SWITCH_LAST], 0);
  fill(&ignore_release_[0], &ignore_release_[SWITCH_LAST], false);
  fill(&edited_while_pressed_[0], &edited_while_pressed_[SWITCH_LAST], false);
  both_pressed_time_ = 0;
  flash_ = FLASH_NONE;
  flash_time_ = 0;
  shown_ = SHOW_NONE;
  shown_time_ = kShowTime;
  step_flash_ = 0;
  dirty_time_ = 0;
  last_envelope_ = -1.0f;
  cv_c1_ = 0.0f;
  pitch_lp_ = 0.0f;
  pitch_lp_calibration_ = 0.0f;
  data_transfer_progress_ = 0.0f;
}

static inline float U8(uint8_t v) { return static_cast<float>(v) / 255.0f; }
static inline uint8_t ToU8(float v) { CONSTRAIN(v, 0.0f, 1.0f); return static_cast<uint8_t>(v * 255.0f + 0.5f); }

void Ui::LoadState() {
  const State& st = settings_->state();
  dub::DefaultSettings(dub_);
  if (st.octave >= 1 && st.octave <= 5) {   // a saved DUB BASE state exists
    dub_->chord_model = st.chord_model % dub::kNumChordModels;
    dub_->octave = st.octave;
    dub_->resonance = U8(st.resonance);
    if (st.has_pattern) {
      dub::Pattern p = voice_->mutable_sequencer()->pattern();
      for (int i = 0; i < dub::kNumSteps; ++i) {
        uint16_t w = st.steps[i];
        p.degree[i] = (w & 7) % 7;
        p.rank[i] = (w >> 7) & 15;
        p.accent[i] = (w >> 11) & 1;
      }
      voice_->mutable_sequencer()->set_pattern(p);
    }
  }
  pots_[POTS_ADC_CHANNEL_FREQUENCY_POT].set_hidden(0, (dub_->octave - 1) / 4.0f);
  pots_[POTS_ADC_CHANNEL_FREQUENCY_POT].set_hidden(1, dub_->resonance);
}

void Ui::SaveState() {
  State* st = settings_->mutable_state();
  st->chord_model = dub_->chord_model;
  st->octave = dub_->octave;
  st->resonance = ToU8(dub_->resonance);
  const dub::Pattern& p = voice_->mutable_sequencer()->pattern();
  for (int i = 0; i < dub::kNumSteps; ++i) {
    st->steps[i] = (p.degree[i] & 7) | ((p.rank[i] & 15) << 7) | ((p.accent[i] & 1) << 11);
  }
  st->has_pattern = 1;
  settings_->SaveState();
}

// ------------------------------------------------------------------ knobs

void Ui::ProcessPots() {
  const bool left = switches_.pressed(SWITCH_ROW_1);
  const bool right = switches_.pressed(SWITCH_ROW_2);
  int layer = -1;
  if (left && !right) layer = 0;
  if (right && !left) layer = 1;
  // Only FREQUENCY has a second function (with a button held).
  DubPot& f = pots_[POTS_ADC_CHANNEL_FREQUENCY_POT];
  if (f.ProcessUiRate(layer)) {
    if (layer == 0) {
      dub_->octave = 1 + static_cast<uint8_t>(f.hidden(0) * 4.999f);
      Show(SHOW_OCTAVE);
    } else {
      dub_->resonance = f.hidden(1);
      Show(SHOW_RESONANCE);
    }
    edited_while_pressed_[layer] = true;
    dirty_time_ = kSaveDelay;
  }
  for (int i = POTS_ADC_CHANNEL_HARMONICS_POT; i < POTS_ADC_CHANNEL_LAST; ++i) {
    pots_[i].ProcessUiRate(-1);
  }
}

// ------------------------------------------------------------------ buttons

void Ui::Navigate(int direction) {
  dub_->chord_model = (dub_->chord_model + dub::kNumChordModels + direction) % dub::kNumChordModels;
  dirty_time_ = kSaveDelay;
}

void Ui::ReadSwitches() {
  switches_.Debounce();

  switch (mode_) {
    case UI_MODE_NORMAL:
      {
        for (int i = 0; i < SWITCH_LAST; ++i) {
          if (switches_.just_pressed(Switch(i))) {
            press_time_[i] = 0;
            ignore_release_[i] = false;
            edited_while_pressed_[i] = false;
            if (switches_.pressed_immediate(Switch(1 - i))) {
              // Both buttons: only the 2 s calibration gesture exists.
              ignore_release_[0] = ignore_release_[1] = true;
            }
          }
          if (switches_.pressed(Switch(i))) ++press_time_[i];
        }
        const bool both = switches_.pressed_immediate(SWITCH_ROW_1) &&
            switches_.pressed_immediate(SWITCH_ROW_2);
        if (both) {
          ignore_release_[0] = ignore_release_[1] = true;
          if (++both_pressed_time_ >= kCalibrationTime) {
            both_pressed_time_ = 0;
            StartCvTune();
            break;
          }
        } else {
          both_pressed_time_ = 0;
        }
        for (int i = 0; i < SWITCH_LAST; ++i) {
          if (!switches_.released(Switch(i))) continue;
          if (ignore_release_[i] || edited_while_pressed_[i]) continue;
          const int held = press_time_[i];
          if (held < kTapTime) {
            Navigate(i == 0 ? -1 : +1);
          } else if (held >= kHoldTime) {
            if (i == 0) {
              voice_->mutable_sequencer()->New();
              Flash(FLASH_NEW);
              dirty_time_ = kSaveDelay;
            } else {
              pots_[POTS_ADC_CHANNEL_HARMONICS_POT].Override(Random::GetFloat());
              pots_[POTS_ADC_CHANNEL_TIMBRE_POT].Override(Random::GetFloat());
              pots_[POTS_ADC_CHANNEL_MORPH_POT].Override(Random::GetFloat());
              Flash(FLASH_RANDOM);
            }
          }
        }
      }
      break;

    case UI_MODE_DISPLAY_DATA_TRANSFER_PROGRESS:
      break;

    case UI_MODE_CV_TUNE:
      {
        // Both held 2 s more: the original CV input calibration.
        const bool both = switches_.pressed_immediate(SWITCH_ROW_1) &&
            switches_.pressed_immediate(SWITCH_ROW_2);
        if (both) {
          if (++both_pressed_time_ >= kCalibrationTime) {
            both_pressed_time_ = 0;
            EndCvTune();
            StartCalibration();
            break;
          }
        } else {
          both_pressed_time_ = 0;
        }
        for (int i = 0; i < SWITCH_LAST; ++i) {
          if (switches_.just_pressed(Switch(i))) {
            ignore_release_[i] = false;
            press_time_[i] = 0;
          }
          if (switches_.pressed(Switch(i))) ++press_time_[i];
          if (switches_.released(Switch(i)) && !ignore_release_[i] && !both) {
            // A tap saves the scale and goes back to playing.
            EndCvTune();
            settings_->set_cv_scale(dub_->cv_scale);
            settings_->SavePersistentData();
            ignore_release_[0] = ignore_release_[1] = true;
            break;
          }
        }
        if (mode_ != UI_MODE_CV_TUNE) break;   // just saved and left
        // Reference notes alternate: AUX 0 V (C3) and +2 V (C5).
        tune_time_ = (tune_time_ + 1) % (2 * kTunePhase);
        dub_->cv_tune = tune_time_ < kTunePhase ? 1 : 2;
      }
      break;

    case UI_MODE_CALIBRATION_C1:
      for (int i = 0; i < SWITCH_LAST; ++i) {
        if (switches_.just_pressed(Switch(i))) {
          press_time_[i] = 0;
          ignore_release_[i] = true;
          CalibrateC1();
          break;
        }
      }
      break;

    case UI_MODE_CALIBRATION_C3:
      for (int i = 0; i < SWITCH_LAST; ++i) {
        if (switches_.just_pressed(Switch(i))) {
          press_time_[i] = 0;
          ignore_release_[i] = true;
          CalibrateC3();
          break;
        }
      }
      break;

    case UI_MODE_TEST:
    case UI_MODE_ERROR:
      for (int i = 0; i < SWITCH_LAST; ++i) {
        if (switches_.just_pressed(Switch(i))) {
          press_time_[i] = 0;
          ignore_release_[i] = true;
          mode_ = UI_MODE_NORMAL;
        }
      }
      break;
  }
}

// ------------------------------------------------------------------ LEDs

static void Bar(Leds* leds, float v, int pwm, uint32_t color) {
  v = v * 8.0f - 0.001f;
  for (int i = 0; i < kNumLEDs; ++i) {
    float a = v - static_cast<float>(i);
    leds->set(i, a >= 1.0f || a * 16.0f > pwm ? color : LED_COLOR_OFF);
  }
}

static void Bar16(Leds* leds, int n, int pwm, uint32_t color) {
  for (int i = 0; i < kNumLEDs; ++i) {
    int lit = n - 2 * i;   // 2 = full, 1 = half
    leds->set(i, lit >= 2 || (lit == 1 && pwm < 4) ? color : LED_COLOR_OFF);
  }
}

void Ui::DrawValue(int pwm) {
  const dub::Settings& d = *dub_;
  switch (shown_) {
    case SHOW_ENVELOPE: Bar(&leds_, d.envelope, pwm, LED_COLOR_YELLOW); break;
    case SHOW_OCTAVE: leds_.set(d.octave - 1, LED_COLOR_YELLOW); break;
    case SHOW_RESONANCE: Bar(&leds_, d.resonance, pwm, LED_COLOR_RED); break;
    case SHOW_DENSITY: Bar16(&leds_, d.density, pwm, LED_COLOR_GREEN); break;
    case SHOW_LENGTH: Bar16(&leds_, d.length, pwm, LED_COLOR_YELLOW); break;
    case SHOW_SPEED:
      // straight values green, triplets red
      leds_.set(d.speed, (d.speed == 3 || d.speed == 5 || d.speed == 7) ? LED_COLOR_RED : LED_COLOR_GREEN);
      break;
    default: break;
  }
}

void Ui::UpdateLEDs() {
  leds_.Clear();
  ++pwm_counter_;

  int pwm_counter = pwm_counter_ & 15;
  int triangle = (pwm_counter_ >> 4) & 31;
  triangle = triangle < 16 ? triangle : 31 - triangle;

  if (dirty_time_ > 0 && --dirty_time_ == 0) {
    SaveState();
  }
  if (step_flash_ > 0) --step_flash_;
  if (shown_time_ < kShowTime) ++shown_time_;

  switch (mode_) {
    case UI_MODE_NORMAL:
      {
        const bool left = switches_.pressed(SWITCH_ROW_1);
        const bool right = switches_.pressed(SWITCH_ROW_2);
        if (flash_ != FLASH_NONE && flash_time_ < kFlashTime) {
          ++flash_time_;
          int pos = flash_time_ * 8 / kFlashTime;
          for (int i = 0; i < kNumLEDs; ++i) {
            uint32_t c = flash_ == FLASH_NEW
                ? (i <= pos ? LED_COLOR_GREEN : LED_COLOR_OFF)
                : (((i * 3 + pos) & 1) ? LED_COLOR_YELLOW : LED_COLOR_OFF);
            leds_.set(i, c);
          }
          break;
        }
        flash_ = FLASH_NONE;
        if (shown_ != SHOW_NONE && shown_time_ < kShowTime) {
          DrawValue(pwm_counter);
          break;
        }
        shown_ = SHOW_NONE;
        // Armed: one button held long enough for NEW (green) / RANDOM (yellow).
        if (left != right) {
          int b = left ? 0 : 1;
          if (press_time_[b] >= kHoldTime && !edited_while_pressed_[b] && !ignore_release_[b]) {
            uint32_t c = b == 0 ? LED_COLOR_GREEN : LED_COLOR_YELLOW;
            for (int i = 0; i < kNumLEDs; ++i) {
              leds_.set(i, pwm_counter < triangle ? c : LED_COLOR_OFF);
            }
            break;
          }
        }
        // The chord model on a yellow LED, the rest glowing green; the model
        // LED blinks off on every step that plays.
        const int model = voice_->active_engine() - 100;
        if (voice_->step_played()) step_flash_ = 35;
        const bool color_blind = settings_->state().color_blind == 1;
        for (int i = 0; i < kNumLEDs; ++i) {
          leds_.set(i, !color_blind && pwm_counter == 0 ? LED_COLOR_GREEN : LED_COLOR_OFF);
        }
        if (model >= 0 && model < kNumLEDs) {
          leds_.set(model, step_flash_ ? LED_COLOR_OFF : LED_COLOR_YELLOW);
        }
      }
      break;

    case UI_MODE_DISPLAY_DATA_TRANSFER_PROGRESS:
      {
        if (data_transfer_progress_ == 1.0f) {
          for (int i = 0; i < 8; ++i) {
            leds_.set(i, i == (triangle >> 1) ? LED_COLOR_OFF : LED_COLOR_GREEN);
          }
        } else if (data_transfer_progress_ < 0.0f) {
          for (int i = 0; i < 8; ++i) {
            leds_.set(i, pwm_counter < triangle ? LED_COLOR_RED : LED_COLOR_OFF);
          }
        } else {
          float value = data_transfer_progress_ - 0.001f;
          for (int i = 0; i < 8; ++i) {
            leds_.set(i, value * 128.0f > pwm_counter ? LED_COLOR_GREEN : LED_COLOR_OFF);
            value -= 0.125f;
          }
        }
      }
      if (pwm_counter_ > 3000) {
        mode_ = UI_MODE_NORMAL;
      }
      break;

    case UI_MODE_CV_TUNE:
      {
        // LEDs 1-4 = low note (0 V), LEDs 5-8 = high note (+2 V); the
        // red LED shows how far the scale is trimmed from the default.
        const int first = dub_->cv_tune == 1 ? 0 : 4;
        for (int i = 0; i < 4; ++i) leds_.set(first + i, LED_COLOR_GREEN);
        float trim = dub_->cv_scale / dub::kDefaultCvScale;
        int pos = static_cast<int>(3.5f + (trim - 1.0f) * 10.0f + 0.5f);
        CONSTRAIN(pos, 0, 7);
        if (pwm_counter < 8) leds_.set(pos, LED_COLOR_RED);
      }
      break;

    case UI_MODE_CALIBRATION_C1:
      if (pwm_counter < triangle) {
        leds_.set(0, LED_COLOR_GREEN);
      }
      break;

    case UI_MODE_CALIBRATION_C3:
      if (pwm_counter < triangle) {
        leds_.set(0, LED_COLOR_YELLOW);
      }
      break;

    case UI_MODE_ERROR:
      if (pwm_counter < triangle) {
        for (int i = 0; i < kNumLEDs; ++i) {
          leds_.set(i, LED_COLOR_RED);
        }
      }
      break;

    case UI_MODE_TEST:
      {
        int color = (pwm_counter_ >> 10) % 3;
        for (int i = 0; i < kNumLEDs; ++i) {
          leds_.set(
              i, pwm_counter > ((triangle + (i * 2)) & 15)
                  ? (color == 0
                     ? LED_COLOR_GREEN
                      : (color == 1 ? LED_COLOR_YELLOW : LED_COLOR_RED))
                  : LED_COLOR_OFF);
        }
      }
      break;
  }
  leds_.Write();
}

// ------------------------------------------------------------------ CV

/* static */
const CvAdcChannel Ui::normalized_channels_[] = {
  CV_ADC_CHANNEL_FM,
  CV_ADC_CHANNEL_TIMBRE,
  CV_ADC_CHANNEL_MORPH,
  CV_ADC_CHANNEL_TRIGGER,
  CV_ADC_CHANNEL_LEVEL,
};

void Ui::DetectNormalization() {
  bool expected_value = normalization_probe_state_ >> 31;
  for (int i = 0; i < kNumNormalizedChannels; ++i) {
    CvAdcChannel channel = normalized_channels_[i];
    bool read_value = cv_adc_.value(channel) < \
        settings_->calibration_data(channel).normalization_detection_threshold;
    if (expected_value != read_value) {
      ++normalization_detection_mismatches_[i];
    }
  }

  ++normalization_detection_count_;
  if (normalization_detection_count_ == kProbeSequenceDuration) {
    normalization_detection_count_ = 0;
    bool* destination = &modulations_->frequency_patched;
    for (int i = 0; i < kNumNormalizedChannels; ++i) {
      destination[i] = normalization_detection_mismatches_[i] >= 2;
      normalization_detection_mismatches_[i] = 0;
    }
  }

  normalization_probe_state_ = 1103515245 * normalization_probe_state_ + 12345;
  normalization_probe_.Write(normalization_probe_state_ >> 31);
}

void Ui::Poll() {
  for (int i = 0; i < POTS_ADC_CHANNEL_LAST; ++i) {
    pots_[i].ProcessControlRate(pots_adc_.float_value(PotsAdcChannel(i)));
  }

  float* destination = &modulations_->engine;
  for (int i = 0; i < CV_ADC_CHANNEL_LAST; ++i) {
    destination[i] = settings_->calibration_data(i).Transform(
        cv_adc_.float_value(CvAdcChannel(i)));
  }

  ONE_POLE(pitch_lp_, modulations_->note, 0.7f);
  modulations_->note = pitch_lp_;

  ONE_POLE(
      pitch_lp_calibration_, cv_adc_.float_value(CV_ADC_CHANNEL_V_OCT), 0.1f);

  ui_task_ = (ui_task_ + 1) % 4;
  switch (ui_task_) {
    case 0:
      UpdateLEDs();
      break;
    case 1:
      ReadSwitches();
      break;
    case 2:
      ProcessPots();
      break;
    case 3:
      DetectNormalization();
      break;
  }

  cv_adc_.Convert();
  pots_adc_.Convert();

  // Main knob layer.
  patch_->harmonics = pots_[POTS_ADC_CHANNEL_HARMONICS_POT].main();
  patch_->timbre = pots_[POTS_ADC_CHANNEL_TIMBRE_POT].main();
  patch_->morph = pots_[POTS_ADC_CHANNEL_MORPH_POT].main();
  patch_->note = 0.0f;

  if (mode_ == UI_MODE_CV_TUNE) {
    // FREQUENCY trims the CV scale (relative to where the knob was).
    float v = pots_[POTS_ADC_CHANNEL_FREQUENCY_POT].value();
    if (!tune_moved_ && fabsf(v - tune_knob_) > 0.02f) tune_moved_ = true;
    if (tune_moved_) {
      dub_->cv_scale = tune_scale_ * stmlib::SemitonesToRatio((v - tune_knob_) * 18.0f);
    }
    return;
  }
  float env = pots_[POTS_ADC_CHANNEL_FREQUENCY_POT].main();
  dub_->envelope = env;
  if (last_envelope_ < 0.0f) last_envelope_ = env;
  if (fabsf(env - last_envelope_) > 0.02f) {
    last_envelope_ = env;
    Show(SHOW_ENVELOPE);
  }

  // The three attenuverters run the sequencer.
  int density = 1 + density_quantizer_.Process(pots_[POTS_ADC_CHANNEL_TIMBRE_ATTENUVERTER].main());
  int length = 1 + length_quantizer_.Process(pots_[POTS_ADC_CHANNEL_FM_ATTENUVERTER].main());
  // Speed: 1/16 owns a wide zone around the centre detent.
  static const float kSpeedEdges[dub::kNumSpeeds - 1] = { 0.1f, 0.2f, 0.3f, 0.42f, 0.58f, 0.7f, 0.82f };
  float sv = pots_[POTS_ADC_CHANNEL_MORPH_ATTENUVERTER].main();
  int speed = dub_->speed;
  for (int k = 0; k < dub::kNumSpeeds; ++k) {
    float lo = k == 0 ? -1.0f : kSpeedEdges[k - 1] - (k <= speed ? 0.015f : -0.015f);
    float hi = k == dub::kNumSpeeds - 1 ? 2.0f : kSpeedEdges[k] + (k >= speed ? 0.015f : -0.015f);
    if (sv >= lo && sv < hi) { speed = k; break; }
  }
  if (density != dub_->density) { if (pwm_counter_ > 500) Show(SHOW_DENSITY); dub_->density = density; }
  if (length != dub_->length) { if (pwm_counter_ > 500) Show(SHOW_LENGTH); dub_->length = length; }
  if (speed != dub_->speed) { if (pwm_counter_ > 500) Show(SHOW_SPEED); dub_->speed = speed; }
}

// ------------------------------------------------------------------ calibration

void Ui::StartCvTune() {
  mode_ = UI_MODE_CV_TUNE;
  tune_time_ = 0;
  tune_moved_ = false;
  tune_knob_ = pots_[POTS_ADC_CHANNEL_FREQUENCY_POT].value();
  tune_scale_ = dub_->cv_scale;
  tune_envelope_ = dub_->envelope;
  dub_->cv_tune = 1;
  ignore_release_[0] = ignore_release_[1] = true;
}

void Ui::EndCvTune() {
  mode_ = UI_MODE_NORMAL;
  dub_->cv_tune = 0;
  // The envelope stays where it was; the knob picks it up when turned.
  pots_[POTS_ADC_CHANNEL_FREQUENCY_POT].Override(tune_envelope_);
}

void Ui::StartCalibration() {
  mode_ = UI_MODE_CALIBRATION_C1;
  normalization_probe_.Disable();
}

void Ui::CalibrateC1() {
  for (int i = 0; i < CV_ADC_CHANNEL_LAST; ++i) {
    if (i != CV_ADC_CHANNEL_V_OCT) {
      ChannelCalibrationData* c = settings_->mutable_calibration_data(i);
      c->offset = -cv_adc_.float_value(CvAdcChannel(i)) * c->scale;
    }
  }
  cv_c1_ = pitch_lp_calibration_;
  mode_ = UI_MODE_CALIBRATION_C3;
}

void Ui::CalibrateC3() {
  float c1 = cv_c1_;
  float c3 = pitch_lp_calibration_;
  float delta = c3 - c1;

  if (delta > -0.6f && delta < -0.2f) {
    ChannelCalibrationData* c = settings_->mutable_calibration_data(
        CV_ADC_CHANNEL_V_OCT);
    c->scale = 24.0f / delta;
    c->offset = 12.0f - c->scale * c1;
    settings_->SavePersistentData();
    mode_ = UI_MODE_NORMAL;
  } else {
    mode_ = UI_MODE_ERROR;
  }
  normalization_probe_.Init();
}

uint8_t Ui::HandleFactoryTestingRequest(uint8_t command) {
  uint8_t argument = command & 0x1f;
  command = command >> 5;
  uint8_t reply = 0;
  switch (command) {
    case FACTORY_TESTING_READ_POT:
      reply = pots_adc_.value(PotsAdcChannel(argument)) >> 8;
      break;
    case FACTORY_TESTING_READ_CV:
      reply = (cv_adc_.value(CvAdcChannel(argument)) + 32768) >> 8;
      break;
    case FACTORY_TESTING_READ_NORMALIZATION:
      reply = (&modulations_->frequency_patched)[argument] ? 0 : 255;
      break;
    case FACTORY_TESTING_READ_GATE:
      reply = switches_.pressed(Switch(argument));
      break;
    case FACTORY_TESTING_GENERATE_TEST_SIGNAL:
      mode_ = argument ? UI_MODE_TEST : UI_MODE_NORMAL;
      break;
    case FACTORY_TESTING_CALIBRATE:
      switch (argument) {
        case 0:
          StartCalibration();
          break;
        case 1:
          CalibrateC1();
          break;
        case 2:
          CalibrateC3();
          SaveState();
          break;
      }
      break;
  }
  return reply;
}

}  // namespace plaits
