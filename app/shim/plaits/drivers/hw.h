// DUB BASE app: the "hardware" the firmware sees, filled in by the Android app.
#ifndef PLAITS_APP_HW_H_
#define PLAITS_APP_HW_H_
#include <stdint.h>
extern "C" {
extern volatile float hw_pots[7];        // knob positions 0..1
extern volatile int hw_buttons[2];       // 1 = held
extern uint32_t hw_leds[8];              // written by Leds::Write()
}
#endif
