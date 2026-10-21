#ifndef PLAITS_DRIVERS_POTS_ADC_H_
#define PLAITS_DRIVERS_POTS_ADC_H_
#include "stmlib/stmlib.h"
#include "plaits/drivers/hw.h"
namespace plaits {
enum PotsAdcChannel {
  POTS_ADC_CHANNEL_FREQUENCY_POT, POTS_ADC_CHANNEL_HARMONICS_POT,
  POTS_ADC_CHANNEL_TIMBRE_POT, POTS_ADC_CHANNEL_MORPH_POT,
  POTS_ADC_CHANNEL_TIMBRE_ATTENUVERTER, POTS_ADC_CHANNEL_FM_ATTENUVERTER,
  POTS_ADC_CHANNEL_MORPH_ATTENUVERTER, POTS_ADC_CHANNEL_LAST
};
// Touch knobs instead of the ADC: same 16-bit values as the module.
class PotsAdc {
 public:
  PotsAdc() { }
  ~PotsAdc() { }
  void Init() { Convert(); }
  void Convert() {
    for (int i = 0; i < POTS_ADC_CHANNEL_LAST; ++i) {
      float v = hw_pots[i];
      v = v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
      values_[i] = static_cast<uint16_t>(v * 65535.0f);
    }
  }
  inline int32_t value(PotsAdcChannel channel) const { return static_cast<int32_t>(values_[channel]); }
  inline float float_value(PotsAdcChannel channel) const { return static_cast<float>(value(channel)) / 65536.0f; }
 private:
  uint16_t values_[POTS_ADC_CHANNEL_LAST];
  DISALLOW_COPY_AND_ASSIGN(PotsAdc);
};
}  // namespace plaits
#endif
