#ifndef PLAITS_DRIVERS_SWITCHES_H_
#define PLAITS_DRIVERS_SWITCHES_H_
#include "stmlib/stmlib.h"
#include "plaits/drivers/hw.h"
namespace plaits {
enum Switch { SWITCH_ROW_1, SWITCH_ROW_2, SWITCH_LAST };
// Same 8-sample debouncer as the module, fed by the touch buttons.
class Switches {
 public:
  Switches() { }
  ~Switches() { }
  void Init() { for (int i = 0; i < SWITCH_LAST; ++i) switch_state_[i] = 0xff; }
  void Debounce() {
    for (int i = 0; i < SWITCH_LAST; ++i)
      switch_state_[i] = (switch_state_[i] << 1) | (hw_buttons[i] ? 0 : 1);
  }
  inline bool released(Switch s) const { return switch_state_[s] == 0x7f; }
  inline bool just_pressed(Switch s) const { return switch_state_[s] == 0x80; }
  inline bool pressed(Switch s) const { return switch_state_[s] == 0x00; }
  inline bool pressed_immediate(Switch s) const { return hw_buttons[s] != 0; }
 private:
  uint8_t switch_state_[SWITCH_LAST];
  DISALLOW_COPY_AND_ASSIGN(Switches);
};
}  // namespace plaits
#endif
