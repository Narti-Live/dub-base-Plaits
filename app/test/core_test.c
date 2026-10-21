/* Panel test of the firmware core as built for the app: same checks as the
   emulator test (emu/uitest2.py), driven through the app's touch "hardware". */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../src/fw.h"

extern uint32_t hw_leds[8];
static unsigned char store_p[512], store_s[512];
static int have_p, have_s, saves;
int app_storage_load(void *p, int np, void *s, int ns) {
    if (have_p) memcpy(p, store_p, np);
    if (have_s) memcpy(s, store_s, ns);
    return have_p || have_s;
}
void app_storage_save(const void *p, int np, const void *s, int ns) {
    if (p) { memcpy(store_p, p, np); have_p = 1; }
    if (s) { memcpy(store_s, s, ns); have_s = 1; }
    saves++;
}

static float buf[24 * 2];
static float peak;
static void run_ms(int ms) {
    for (int i = 0; i < ms * 4; i++) {
        fw_render(buf, 12, 48000.f);
        for (int k = 0; k < 24; k++) if (fabsf(buf[k]) > peak) peak = fabsf(buf[k]);
    }
}
static void leds(char *o) {
    for (int i = 0; i < 8; i++) {
        uint32_t c = hw_leds[i];
        o[i] = c == 0 ? '.' : c == 0xff0000 ? 'R' : c == 0xff00 ? 'G' : c == 0xffff00 ? 'Y' : '?';
    }
    o[8] = 0;
}
static FwSnapshot S(void) { FwSnapshot s; run_ms(5); fw_snapshot(&s); return s; }
static int ok = 1;
static void check(int c, const char *m) { printf("  %s %s\n", c ? "PASS" : "FAIL", m); if (!c) ok = 0; }
static void press(int b, int ms) { hw_buttons[b] = 1; run_ms(ms); hw_buttons[b] = 0; run_ms(30); }

int main(void) {
    char l[9], m[200];
    fw_init(1);
    run_ms(1500);
    leds(l);
    check(S().model == 0 && l[0] == 'Y', "boot: model 1 on a yellow LED");
    press(1, 100); check(S().model == 1, "tap RIGHT -> next model");
    press(0, 100); press(0, 100); check(S().model == 7, "tap LEFT twice -> wraps to model 8");
    press(1, 100); check(S().model == 0, "tap RIGHT -> model 1");

    hw_pots[0] = 0.1f; run_ms(500);
    leds(l); sprintf(m, "FREQUENCY -> envelope %.2f, yellow bar %s", S().envelope, l);
    check(fabsf(S().envelope - 0.1f) < 0.02f && strchr(l, 'Y'), m);

    hw_buttons[0] = 1; run_ms(100);
    hw_pots[0] = 1.0f; run_ms(300); leds(l);
    sprintf(m, "LEFT + FREQUENCY -> octave %d, LED %s", S().octave, l);
    check(S().octave == 5 && l[4] == 'Y', m);
    hw_pots[0] = 0.5f; run_ms(300); check(S().octave == 3, "octave 3 at noon");
    hw_buttons[0] = 0; run_ms(60);
    check(S().model == 0 && fabsf(S().envelope - 0.1f) < 0.02f, "envelope held, no navigation after edit");

    hw_buttons[1] = 1; run_ms(100);
    hw_pots[0] = 1.0f; run_ms(300); float r1 = S().resonance;
    hw_pots[0] = 0.4f; run_ms(300);
    hw_buttons[1] = 0; run_ms(60);
    sprintf(m, "RIGHT + FREQUENCY -> resonance %.2f then %.2f", r1, S().resonance);
    check(r1 > 0.97f && fabsf(S().resonance - 0.4f) < 0.03f && S().model == 0, m);

    hw_pots[4] = 0.f; run_ms(300); int d0 = S().density;
    hw_pots[4] = 1.f; run_ms(300); int d1 = S().density; leds(l);
    sprintf(m, "TIMBRE att -> density %d..%d (%s)", d0, d1, l);
    check(d0 == 1 && d1 == 16 && !strcmp(l, "GGGGGGGG"), m);
    hw_pots[5] = 0.f; run_ms(300); int l0 = S().length;
    hw_pots[5] = 1.f; run_ms(300); int l1 = S().length;
    sprintf(m, "FM att -> length %d..%d", l0, l1); check(l0 == 1 && l1 == 16, m);
    int sp[6]; float sv[6] = {0, .5f, .36f, .64f, 1, .5f};
    for (int i = 0; i < 6; i++) { hw_pots[6] = sv[i]; run_ms(300); sp[i] = S().speed; }
    sprintf(m, "MORPH att -> speed %d %d %d %d %d %d", sp[0], sp[1], sp[2], sp[3], sp[4], sp[5]);
    check(sp[0] == 0 && sp[1] == 4 && sp[4] == 7 && sp[5] == 4, m);
    hw_pots[4] = 0.3f; run_ms(300);

    /* NEW */
    fw_set_clock(1, 120);
    run_ms(200);
    FwSnapshot a = S();
    hw_buttons[0] = 1; run_ms(1100); hw_buttons[0] = 0; run_ms(300);
    FwSnapshot b = S();
    check(memcmp(a.rank, b.rank, 16) || memcmp(a.degree, b.degree, 16), "hold LEFT 1 s -> NEW pattern");
    check(b.model == 0, "NEW did not navigate");
    float h0 = S().harmonics;
    hw_buttons[1] = 1; run_ms(1100); hw_buttons[1] = 0; run_ms(100);
    sprintf(m, "hold RIGHT 1 s -> RANDOM (HARMONICS %.2f -> %.2f)", h0, S().harmonics);
    check(fabsf(S().harmonics - h0) > 0.001f && S().model == 0, m);

    /* clock: steps at 120 BPM 16ths = 8 per second (density decides which sound) */
    hw_pots[4] = 1.f; run_ms(300);
    uint32_t c0 = S().step_counter; run_ms(2000); uint32_t c1 = S().step_counter;
    sprintf(m, "app clock 120 BPM, density 16: %u steps in 2 s", c1 - c0);
    check(c1 - c0 >= 15 && c1 - c0 <= 17, m);
    fw_set_clock(1, 90); run_ms(500);
    c0 = S().step_counter; run_ms(4000); c1 = S().step_counter;
    sprintf(m, "app clock 90 BPM: %u steps in 4 s (expect 24)", c1 - c0);
    check(c1 - c0 >= 23 && c1 - c0 <= 25, m);

    /* all chord notes in C minor */
    int bad = 0, notes = 0;
    static const int inscale[12] = {1,0,1,1,0,1,0,1,1,0,1,0};
    for (int k = 0; k < 40; k++) {
        hw_buttons[0] = 1; run_ms(1100); hw_buttons[0] = 0; run_ms(50);
        FwSnapshot s = S();
        for (int i = 0; i < 16; i++) for (int j = 0; j < 5; j++) if (s.chord[i][j] >= 0) { notes++; if (!inscale[s.chord[i][j] % 12]) bad++; }
    }
    sprintf(m, "%d chord notes from 40 NEW patterns, %d outside C minor", notes, bad);
    check(bad == 0 && notes > 1000, m);

    /* AUX pitch CV: chord roots, always C minor notes */
    { int seen[128] = {0}, badcv = 0, kinds = 0;
      for (int k = 0; k < 12; k++) {
        hw_buttons[0] = 1; run_ms(1100); hw_buttons[0] = 0;
        for (int t = 0; t < 40; t++) { run_ms(50); FwSnapshot s = S(); float n = 48 + s.cv_volts * 12;
          int ni = (int)(n + 100.5f) - 100; if (fabsf(n - ni) > 0.01f || !inscale[((ni % 12) + 12) % 12]) badcv++; else if (!seen[ni]++) kinds++; } }
      sprintf(m, "AUX CV follows chord roots: %d different notes, %d off-scale", kinds, badcv);
      check(kinds >= 3 && badcv == 0, m); }
    hw_buttons[0] = hw_buttons[1] = 1; run_ms(2200); hw_buttons[0] = hw_buttons[1] = 0; run_ms(100);
    { FwSnapshot s = S(); sprintf(m, "both buttons 2 s -> CV TUNE (state %d, %.2f V)", s.cv_tune, s.cv_volts);
      check(s.cv_tune == 1 && fabsf(s.cv_volts) < 0.001f, m);
      run_ms(2000); s = S(); sprintf(m, "CV TUNE high note +2 V (%.2f V)", s.cv_volts); check(s.cv_tune == 2 && fabsf(s.cv_volts - 2) < 0.001f, m);
      press(1, 100); s = S(); check(s.cv_tune == 0, "tap leaves CV TUNE"); }

    peak = 0; run_ms(2000);
    sprintf(m, "audio while clocked (peak %.2f)", peak); check(peak > 0.05f && peak <= 1.0f, m);
    for (int mdl = 0; mdl < 8; mdl++) {
        peak = 0; run_ms(1500);
        sprintf(m, "model %d makes sound (peak %.2f)", S().model + 1, peak);
        check(peak > 0.03f, m);
        press(1, 100);
    }

    /* save + "power cycle" */
    press(1, 100); press(1, 100); run_ms(2500);
    FwSnapshot before = S();
    check(saves > 0 && have_s, "state saved after a change");
    fw_init(7); run_ms(300);
    FwSnapshot after = S();
    sprintf(m, "restored: model %d octave %d resonance %.2f pattern %s", after.model + 1, after.octave, after.resonance,
            memcmp(before.degree, after.degree, 16) || memcmp(before.rank, after.rank, 16) ? "DIFFERENT" : "same");
    check(after.model == before.model && after.octave == before.octave && fabsf(after.resonance - before.resonance) < 0.01f &&
          !memcmp(before.degree, after.degree, 16) && !memcmp(before.rank, after.rank, 16), m);
    printf(ok ? "ALL PASS\n" : "SOME CHECKS FAILED\n");
    return !ok;
}
