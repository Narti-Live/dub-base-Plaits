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
// Settings storage.

#include "plaits/settings.h"

#include <algorithm>
#include <cstring>


extern "C" {
// Implemented by the app (no file I/O on the audio thread: it only queues a copy).
int app_storage_load(void* persistent, int persistent_size, void* state, int state_size);
void app_storage_save(const void* persistent, int persistent_size, const void* state, int state_size);
}

namespace plaits {

using namespace std;

bool Settings::Init() {
  ChannelCalibrationData* c = persistent_data_.channel_calibration_data;
  c[CV_ADC_CHANNEL_MODEL].offset = 0.025f;
  c[CV_ADC_CHANNEL_MODEL].scale = -1.03f;
  c[CV_ADC_CHANNEL_MODEL].normalization_detection_threshold = 0;
  
  c[CV_ADC_CHANNEL_V_OCT].offset = 25.71;
  c[CV_ADC_CHANNEL_V_OCT].scale = -60.0f;
  c[CV_ADC_CHANNEL_V_OCT].normalization_detection_threshold = 0;

  c[CV_ADC_CHANNEL_FM].offset = 0.0f;
  c[CV_ADC_CHANNEL_FM].scale = -60.0f;
  c[CV_ADC_CHANNEL_FM].normalization_detection_threshold = -2945;

  c[CV_ADC_CHANNEL_HARMONICS].offset = 0.0f;
  c[CV_ADC_CHANNEL_HARMONICS].scale = -1.0f;
  c[CV_ADC_CHANNEL_HARMONICS].normalization_detection_threshold = 0;

  c[CV_ADC_CHANNEL_TIMBRE].offset = 0.0f;
  c[CV_ADC_CHANNEL_TIMBRE].scale = -1.6f;
  c[CV_ADC_CHANNEL_TIMBRE].normalization_detection_threshold = -2945;

  c[CV_ADC_CHANNEL_MORPH].offset = 0.0f;
  c[CV_ADC_CHANNEL_MORPH].scale = -1.6f;
  c[CV_ADC_CHANNEL_MORPH].normalization_detection_threshold = -2945;

  c[CV_ADC_CHANNEL_TRIGGER].offset = 0.4f;
  c[CV_ADC_CHANNEL_TRIGGER].scale = -0.6f;
  c[CV_ADC_CHANNEL_TRIGGER].normalization_detection_threshold = 13663;

  c[CV_ADC_CHANNEL_LEVEL].offset = 0.49f;
  c[CV_ADC_CHANNEL_LEVEL].scale = -0.6f;
  c[CV_ADC_CHANNEL_LEVEL].normalization_detection_threshold = 21403;
  
  memset(&state_, 0, sizeof(state_));
  
  bool success = app_storage_load(&persistent_data_, sizeof(persistent_data_), &state_, sizeof(state_)) != 0;
  

  return success;
}

void Settings::SavePersistentData() {
  app_storage_save(&persistent_data_, sizeof(persistent_data_), 0, 0);
}

void Settings::SaveState() {
  app_storage_save(0, 0, &state_, sizeof(state_));
}

}  // namespace plaits
