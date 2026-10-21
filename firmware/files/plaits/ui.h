// Copyright 2016 Emilie Gillet. DUB BASE changes 2026 Narti Live.
// MIT license, see the original Plaits sources.
//
// User interface: two hidden knob layers (hold LEFT = sound, hold RIGHT =
// sequence), button gestures for NEW / MUTATE / BACK / RANDOM / EVOLVE.

#ifndef PLAITS_UI_H_
#define PLAITS_UI_H_

#include "stmlib/stmlib.h"

#include "stmlib/dsp/hysteresis_quantizer.h"

#include "plaits/drivers/cv_adc.h"
#include "plaits/drivers/leds.h"
#include "plaits/drivers/normalization_probe.h"
#include "plaits/drivers/pots_adc.h"
#include "plaits/drivers/switches.h"
#include "plaits/dsp/voice.h"
#include "plaits/dub/dub.h"
#include "plaits/settings.h"

namespace plaits {

const int kNumNormalizedChannels = 5;
const int kProbeSequenceDuration = 32;

enum UiMode {
  UI_MODE_NORMAL,
  UI_MODE_DISPLAY_DATA_TRANSFER_PROGRESS,
  UI_MODE_CALIBRATION_C1,
  UI_MODE_CALIBRATION_C3,
  UI_MODE_TEST,
  UI_MODE_ERROR,
  UI_MODE_CV_TUNE
};

enum FactoryTestingCommand {
  FACTORY_TESTING_READ_POT,
  FACTORY_TESTING_READ_CV,
  FACTORY_TESTING_READ_GATE,
  FACTORY_TESTING_GENERATE_TEST_SIGNAL,
  FACTORY_TESTING_CALIBRATE,
  FACTORY_TESTING_READ_NORMALIZATION,
};

// What the LEDs are showing for a moment after a control moves.
enum DubShow {
  SHOW_NONE,
  SHOW_ENVELOPE,
  SHOW_OCTAVE,
  SHOW_RESONANCE,
  SHOW_DENSITY,
  SHOW_LENGTH,
  SHOW_SPEED,
};

enum DubFlash {
  FLASH_NONE,
  FLASH_NEW,
  FLASH_RANDOM,
};

// A knob with one main and two hidden parameters (one per button).
class DubPot {
 public:
  void Init(float lp) {
    lp_ = lp;
    value_ = previous_ = stored_ = 0.0f;
    locked_ = false;
    editing_ = false;
    catching_up_ = false;
    main_ = 0.0f;
    hidden_[0] = hidden_[1] = 0.0f;
    edited_layer_ = -1;
    primed_ = false;
  }
  inline void ProcessControlRate(float adc) {
    if (!primed_) { value_ = previous_ = adc; primed_ = true; }
    value_ += lp_ * (adc - value_);
    if (!locked_ && !catching_up_) main_ = value_;
  }
  // layer: -1 none held, 0 or 1 = hidden layer of the held button
  inline bool ProcessUiRate(int layer) {
    bool changed = false;
    if (layer >= 0) {
      if (!locked_) {
        locked_ = true;
        editing_ = false;
        stored_ = main_;
        previous_ = value_;
      }
      if (!editing_ && fabsf(value_ - previous_) > 0.03f) {
        editing_ = true;
        edited_layer_ = layer;
      }
      if (editing_) {
        hidden_[layer] = value_;
        changed = true;
      }
    } else {
      if (locked_) {
        locked_ = false;
        if (editing_) {
          catching_up_ = true;
          main_ = stored_;
        }
        editing_ = false;
        previous_ = value_;
      }
      if (catching_up_) {
        if (fabsf(value_ - previous_) > 0.005f) {
          float delta = value_ - previous_;
          float skew = delta > 0.0f
              ? (1.001f - stored_) / (1.001f - previous_)
              : (0.001f + stored_) / (0.001f + previous_);
          CONSTRAIN(skew, 0.1f, 10.0f);
          stored_ += skew * delta;
          CONSTRAIN(stored_, 0.0f, 1.0f);
          if (fabsf(stored_ - value_) < 0.005f) catching_up_ = false;
          previous_ = value_;
          main_ = stored_;
        }
      } else {
        previous_ = value_;
      }
    }
    return changed;
  }
  // Jump the main parameter (RANDOM); the knob catches up when moved.
  inline void Override(float v) {
    stored_ = main_ = v;
    previous_ = value_;
    catching_up_ = true;
  }
  inline void Realign() { catching_up_ = false; locked_ = false; editing_ = false; }
  inline float main() const { return main_; }
  inline float hidden(int layer) const { return hidden_[layer]; }
  inline float value() const { return value_; }
  inline bool editing() const { return editing_; }
  inline int edited_layer() const { return edited_layer_; }
  inline void set_hidden(int layer, float v) { hidden_[layer] = v; }
 private:
  float lp_, value_, previous_, stored_, main_;
  float hidden_[2];
  bool locked_, editing_, catching_up_, primed_;
  int edited_layer_;
};

class Ui {
 public:
  Ui() { }
  ~Ui() { }

  void Init(Patch* patch, Modulations* modulations, Settings* settings,
            dub::Settings* dub_settings, Voice* voice);

  void Poll();

  void DisplayDataTransferProgress(float progress) {
    mode_ = UI_MODE_DISPLAY_DATA_TRANSFER_PROGRESS;
    data_transfer_progress_ = progress;
    pwm_counter_ = progress == 1.0f || progress < 0.0f ? 1500 : 0;
  }

  inline bool test_mode() const {
    return mode_ == UI_MODE_TEST;
  }

  uint8_t HandleFactoryTestingRequest(uint8_t command);

 private:
  void UpdateLEDs();
  void ReadSwitches();
  void ProcessPots();
  void LoadState();
  void SaveState();
  void DetectNormalization();
  void Navigate(int direction);
  void Flash(DubFlash f) { flash_ = f; flash_time_ = 0; }
  void Show(DubShow s) { shown_ = s; shown_time_ = 0; }
  void DrawValue(int pwm);

  void StartCvTune();
  void EndCvTune();
  void StartCalibration();
  void CalibrateC1();
  void CalibrateC3();

  UiMode mode_;

  CvAdc cv_adc_;
  PotsAdc pots_adc_;
  Leds leds_;
  Switches switches_;

  int ui_task_;

  float data_transfer_progress_;
  Patch* patch_;
  Modulations* modulations_;
  dub::Settings* dub_;
  Voice* voice_;
  NormalizationProbe normalization_probe_;
  DubPot pots_[POTS_ADC_CHANNEL_LAST];
  float pitch_lp_;
  float pitch_lp_calibration_;

  Settings* settings_;

  int normalization_detection_count_;
  int normalization_detection_mismatches_[kNumNormalizedChannels];
  uint32_t normalization_probe_state_;

  int pwm_counter_;
  int press_time_[SWITCH_LAST];
  bool ignore_release_[SWITCH_LAST];
  bool edited_while_pressed_[SWITCH_LAST];
  int both_pressed_time_;

  DubFlash flash_;
  int flash_time_;
  DubShow shown_;
  int shown_time_;
  int step_flash_;
  int dirty_time_;
  float last_envelope_;
  stmlib::HysteresisQuantizer2 density_quantizer_;
  stmlib::HysteresisQuantizer2 length_quantizer_;

  float cv_c1_;  // For calibration
  int tune_time_;
  float tune_knob_, tune_scale_, tune_envelope_;
  bool tune_moved_;

  static const CvAdcChannel normalized_channels_[kNumNormalizedChannels];

  DISALLOW_COPY_AND_ASSIGN(Ui);
};

}  // namespace plaits

#endif  // PLAITS_UI_H_
