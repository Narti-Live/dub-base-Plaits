/* 24 s of model 8 (ACID): dry, then with the app's tape echo + reverb. */
#include <stdio.h>
#include <string.h>
#include "../src/fw.h"
#include "../src/fx.h"
int app_storage_load(void *p, int np, void *s, int ns) { return 0; }
void app_storage_save(const void *p, int np, const void *s, int ns) {}
static float buf[2 * 480];
static short pcm[2 * 480];
int main(int argc, char **argv) {
    FILE *f = fopen(argv[1], "wb");
    fw_init(21); fw_set_mix(1.8f, 0.f); fw_set_clock(1, 126);
    hw_pots[0] = 0.25f; hw_pots[1] = 0.32f; hw_pots[2] = 0.75f; hw_pots[3] = 0.70f;   /* env, cutoff, accent, reso */
    hw_pots[4] = 0.85f; hw_pots[5] = 1.0f; hw_pots[6] = 0.5f;                         /* density, length, speed 1/16 */
    for (int b = 0; b < 30; b++) fw_render(buf, 480, 48000.f);
    for (int k = 0; k < 7; k++) { hw_buttons[1] = 1; for (int b = 0; b < 10; b++) fw_render(buf, 480, 48000.f); hw_buttons[1] = 0; for (int b = 0; b < 4; b++) fw_render(buf, 480, 48000.f); }
    fx_params.echo = 0.4f; fx_params.time = 2; fx_params.repeats = 0.5f; fx_params.reverb = 0.35f; fx_params.wow = 0.35f;
    for (int b = 0; b < 2400; b++) {
        if (b == 1100) hw_pots[1] = 0.55f;                   /* open the cutoff half way through */
        fw_render(buf, 480, 48000.f);
        if (b >= 1200) fx_process(buf, 480, 126.f);           /* second half: with the app's echo + reverb */
        for (int i = 0; i < 960; i++) { float v = buf[i]; v = v > 1 ? 1 : (v < -1 ? -1 : v); pcm[i] = (short)(v * 32767); }
        fwrite(pcm, 2, 960, f);
    }
    fclose(f); return 0;
}
