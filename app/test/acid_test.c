/* Loudness of every model (same settings), and the ACID model over a grid of
   FREQUENCY / HARMONICS / TIMBRE / MORPH: peak, RMS, clipped samples. */
#include <stdio.h>
#include <math.h>
#include "../src/fw.h"
int app_storage_load(void *p, int np, void *s, int ns) { return 0; }
void app_storage_save(const void *p, int np, const void *s, int ns) {}
static float buf[2 * 480];
static void run(int ms, float *pk, double *rms, long *n, long *clip) {
    for (int b = 0; b < ms / 10; b++) {
        fw_render(buf, 480, 48000.f);
        for (int i = 0; i < 960; i += 2) { float a = fabsf(buf[i]); if (a > *pk) *pk = a; *rms += a * a; (*n)++; if (a > 0.985f) (*clip)++; }
    }
}
static void to_model(int m) { /* boot on model 1, tap RIGHT m times */
    for (int k = 0; k < m; k++) { hw_buttons[1] = 1; float p; double r; long n, c; p = 0; r = 0; n = c = 0; run(100, &p, &r, &n, &c); hw_buttons[1] = 0; run(40, &p, &r, &n, &c); }
}
int main(void) {
    printf("model loudness (density 10, env 0.5, H/T/M 0.5), 4 s each:\n");
    for (int m = 0; m < 8; m++) {
        fw_init(9); fw_set_mix(1.f, 0.f); fw_set_clock(1, 124);
        for (int i = 0; i < 7; i++) hw_pots[i] = 0.5f; hw_pots[4] = 0.62f;
        float p = 0; double r = 0; long n = 0, c = 0; run(300, &p, &r, &n, &c);
        to_model(m); p = 0; r = 0; n = c = 0; run(4000, &p, &r, &n, &c);
        FwSnapshot s; fw_snapshot(&s);
        printf("  model %d: peak %.2f  rms %.3f (%.1f dBFS)  clipped %ld\n", s.model + 1, p, sqrt(r / n), 20 * log10(sqrt(r / n) + 1e-9), c);
    }
    printf("ACID grid (worst case of H x T x M for each FREQUENCY):\n");
    float V[3] = {0.f, 0.5f, 1.f};
    for (int e = 0; e < 3; e++) {
        float wp = 0, minr = 9, maxr = 0; long wc = 0;
        for (int h = 0; h < 3; h++) for (int t = 0; t < 3; t++) for (int mo = 0; mo < 3; mo++) {
            fw_init(9); fw_set_mix(1.f, 0.f); fw_set_clock(1, 124);
            hw_pots[0] = V[e]; hw_pots[1] = V[h]; hw_pots[2] = V[t]; hw_pots[3] = V[mo]; hw_pots[4] = 0.8f; hw_pots[5] = 1; hw_pots[6] = 0.5f;
            float p = 0; double r = 0; long n = 0, c = 0; run(300, &p, &r, &n, &c);
            to_model(7); p = 0; r = 0; n = c = 0; run(3000, &p, &r, &n, &c);
            float rr = sqrt(r / n); if (p > wp) wp = p; if (rr < minr) minr = rr; if (rr > maxr) maxr = rr; wc += c;
        }
        printf("  FREQ %.1f: peak %.2f  rms %.3f..%.3f  clipped %ld\n", V[e], wp, minr, maxr, wc);
    }
    return 0;
}
