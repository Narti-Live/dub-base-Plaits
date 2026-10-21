#ifndef PLAITS_DRIVERS_LEDS_H_
#define PLAITS_DRIVERS_LEDS_H_
#include "stmlib/stmlib.h"
#include "plaits/drivers/hw.h"
namespace plaits {
const int kNumLEDs = 8;
enum LedColor {
  LED_COLOR_OFF = 0, LED_COLOR_RED = 0xff0000,
  LED_COLOR_GREEN = 0x00ff00, LED_COLOR_YELLOW = 0xffff00,
};
// The 8 bicolour LEDs, drawn on screen by the app.
class Leds {
 public:
  Leds() { }
  ~Leds() { }
  void Init() { Clear(); Write(); }
  void Write() { for (int i = 0; i < kNumLEDs; ++i) hw_leds[i] = colors_[i]; }
  void Clear() { for (int i = 0; i < kNumLEDs; ++i) colors_[i] = 0; }
  void set(int index, uint32_t color) { colors_[index] = color; }
  void mask(int index, uint32_t color) { colors_[index] |= color; }
 private:
  uint32_t colors_[kNumLEDs];
  DISALLOW_COPY_AND_ASSIGN(Leds);
};
}  // namespace plaits
#endif
