#ifndef UI_H
#define UI_H
#include <stdint.h>
#include "fx.h"

#define CW 720

enum { UI_DOWN, UI_MOVE, UI_UP };

/* what the app remembers besides the firmware's own saved state */
typedef struct {
    float pots[7];
    int clock_mode;     /* 0 = TRIG unpatched (firmware's internal 120 BPM), 1 = app clock patched into TRIG */
    float bpm;
    int listen;         /* unused since v1.3 (AUX is a CV) */
    FxParams fx;        /* app-only tape echo + reverb */
} UiPersist;
extern UiPersist ui_persist;

void ui_defaults(void);
void ui_apply(void);                       /* push ui_persist to the firmware */
void ui_render(uint32_t *canvas, int w, int h);
void ui_touch(int type, int id, int x, int y);
void ui_cancel_all(void);
int ui_content_height(void);
void ui_scroll_to(float y);
#endif
