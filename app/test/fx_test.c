/* Renders 24 s of the firmware + app FX: levels, NaN check, echo tail. */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "../src/fw.h"
#include "../src/fx.h"
int app_storage_load(void *p, int np, void *s, int ns) { return 0; }
void app_storage_save(const void *p, int np, const void *s, int ns) {}
static float buf[2 * 480];
int main(int argc, char **argv) {
    fw_init(5); fw_set_clock(1, 120); fw_set_mix(1.8f, 0.f);
    hw_pots[4] = 0.35f;
    FILE *w = argc > 1 ? fopen(argv[1], "wb") : 0;
    float peak = 0, tailpk = 0; int nan = 0;
    for (int blk = 0; blk < 2400; blk++) {        /* 24 s in 10 ms blocks */
        if (blk == 1800) fw_set_clock(1, 120), hw_pots[4] = 0.0f;   /* density 1: almost silent -> hear tails */
        fw_render(buf, 480, 48000.f);
        fx_process(buf, 480, 120.f);
        for (int i = 0; i < 960; i++) { float a = fabsf(buf[i]); if (a != a) nan++; if (a > peak) peak = a; if (blk > 1810 && a > tailpk) tailpk = a; }
        if (w) { short s16[960]; for (int i = 0; i < 960; i++) s16[i] = (short)(buf[i] * 32767); fwrite(s16, 2, 960, w); }
    }
    if (w) fclose(w);
    printf("peak %.2f, NaN %d, tail peak %.2f -> %s\n", peak, nan, tailpk, (nan == 0 && peak <= 1.0f && peak > 0.2f) ? "PASS" : "FAIL");
    return 0;
}
