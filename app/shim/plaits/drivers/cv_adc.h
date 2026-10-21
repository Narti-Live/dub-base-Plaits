#ifndef PLAITS_DRIVERS_CV_ADC_H_
#define PLAITS_DRIVERS_CV_ADC_H_
#include "stmlib/stmlib.h"
namespace plaits {
enum CvAdcChannel {
  CV_ADC_CHANNEL_MODEL, CV_ADC_CHANNEL_V_OCT, CV_ADC_CHANNEL_FM,
  CV_ADC_CHANNEL_HARMONICS, CV_ADC_CHANNEL_TIMBRE, CV_ADC_CHANNEL_MORPH,
  CV_ADC_CHANNEL_TRIGGER, CV_ADC_CHANNEL_LEVEL, CV_ADC_CHANNEL_LAST
};
// No CV jacks on a phone: every input reads 0 (the app writes the
// modulations it needs, TRIG clock and patch flags, after Ui::Poll()).
class CvAdc {
 public:
  CvAdc() { }
  ~CvAdc() { }
  void Init() { for (int i = 0; i < CV_ADC_CHANNEL_LAST; ++i) values_[i] = 0; }
  void DeInit() { }
  void Convert() { }
  inline int16_t value(CvAdcChannel channel) const { return values_[channel]; }
  inline float float_value(CvAdcChannel channel) const { return static_cast<float>(value(channel)) / 32768.0f; }
 private:
  int16_t values_[CV_ADC_CHANNEL_LAST];
  DISALLOW_COPY_AND_ASSIGN(CvAdc);
};
}  // namespace plaits
#endif
