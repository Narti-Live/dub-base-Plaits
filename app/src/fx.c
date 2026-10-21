/* App-only effects after the firmware's OUT: a tape echo in the spirit of the
   old dub echo machines (wow + flutter on the tape speed, each repeat a bit
   darker, thinner and more saturated, tempo-synced) and a dark plate-like
   reverb (8-line feedback delay network). The module itself stays dry. */
#include "plat.h"
#include "fx.h"

FxParams fx_params = {0.35f, 2, 0.55f, 0.30f, 0.35f};   /* dotted 1/8: the classic dub echo */

#define SR 48000
#define ECHO_LEN (SR * 2)                 /* up to 2 s of "tape" */
static float tape[ECHO_LEN];
static int wpos;
static float time_lp;                     /* smoothed delay time: tape-speed glide when it changes */
static float lp1, lp2, hp_s, fb_sat;
static float wow_ph, flut_ph, drift, drift_target;
static uint32_t rng = 0x1234567u;
static inline float frand(void) { rng = rng * 1664525u + 1013904223u; return (float)(rng >> 8) * (1.0f / 16777216.0f); }

/* reverb: 8 delay lines (mutually prime lengths) with a Householder mix */
static const int kLen[8] = {1557, 1617, 1491, 1422, 1277, 1356, 1188, 1116};
static float rbuf[8][2400];
static int rpos[8];
static float rlp[8];
static float pre[4800];
static int prepos;
static float ap_buf[2][600];
static int ap_pos[2];

static const float kDiv[6] = {0.5f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f};   /* in 8th notes */
const char *fx_time_names[6] = {"1/16", "1/8", "1/8 dotted", "1/4", "1/4 dotted", "1/2"};

static inline float sat(float x) {          /* soft tape saturation */
    if (x > 2.5f) return 1.0f;
    if (x < -2.5f) return -1.0f;
    return x * (27.f + x * x) / (27.f + 9.f * x * x) * 0.98f;
}

/* clean up to 0.7, then a smooth knee: the dry signal is not coloured */
static inline float knee(float x) {
    float a = x < 0 ? -x : x;
    if (a <= 0.7f) return x;
    float y = 0.7f + 0.3f * sat((a - 0.7f) / 0.3f);
    return x < 0 ? -y : y;
}

static float allpass(int k, float x, int len, float g) {
    float y = ap_buf[k][ap_pos[k]];
    float v = x + g * y;
    ap_buf[k][ap_pos[k]] = v;
    ap_pos[k] = (ap_pos[k] + 1) % len;
    return y - g * v;
}

void fx_reset(void) {
    memset(tape, 0, sizeof tape);
    memset(rbuf, 0, sizeof rbuf);
    memset(pre, 0, sizeof pre);
    memset(ap_buf, 0, sizeof ap_buf);
    lp1 = lp2 = hp_s = fb_sat = 0.f;
    time_lp = 0.f;
}

void fx_process(float *stereo, int frames, float bpm) {
    const FxParams p = fx_params;
    if (bpm < 30.f) bpm = 120.f;
    int div = p.time < 0 ? 0 : (p.time > 5 ? 5 : p.time);
    float eighth = 60.f / bpm * 0.5f * SR;
    float target = eighth * kDiv[div];
    if (target > ECHO_LEN - 2400) target = ECHO_LEN - 2400;
    if (time_lp <= 0.f) time_lp = target;
    const float echo = p.echo, fb = 0.2f + p.repeats * 0.78f;   /* up to near self-oscillation, tamed by the saturation */
    const float wow = p.wow;
    const float rev = p.reverb;
    for (int i = 0; i < frames; i++) {
        float dry = stereo[2 * i];
        /* tape speed: slow wow, faster flutter, random drift */
        wow_ph += 0.55f / SR; if (wow_ph >= 1.f) wow_ph -= 1.f;
        flut_ph += 6.3f / SR; if (flut_ph >= 1.f) flut_ph -= 1.f;
        if ((i & 255) == 0) drift_target = (frand() - 0.5f) * 2.f;
        drift += (drift_target - drift) * 0.00002f;
        time_lp += (target - time_lp) * 0.00004f;                /* glides like a tape machine */
        float mod = wow * (sinf(wow_ph * 6.2831853f) * 0.0045f + sinf(flut_ph * 6.2831853f) * 0.0009f + drift * 0.002f);
        float d = time_lp * (1.f + mod);
        float rp = (float)wpos - d;
        while (rp < 0) rp += ECHO_LEN;
        int i0 = (int)rp; float fr = rp - i0;
        int i1 = i0 + 1 >= ECHO_LEN ? 0 : i0 + 1;
        float wet = tape[i0] + (tape[i1] - tape[i0]) * fr;
        /* the repeat path: darker and thinner each time, saturated */
        lp1 += 0.32f * (wet - lp1);
        lp2 += 0.32f * (lp1 - lp2);
        hp_s += 0.012f * (lp2 - hp_s);
        float shaped = lp2 - hp_s;
        float w = sat((dry * 0.9f + shaped * fb) * 1.15f) * 0.87f;
        tape[wpos] = w;
        wpos = wpos + 1 >= ECHO_LEN ? 0 : wpos + 1;
        float eL = shaped, eR;
        { /* a second read head a little later for width */
            float rp2 = rp - 0.013f * SR; while (rp2 < 0) rp2 += ECHO_LEN;
            int j0 = (int)rp2; float f2 = rp2 - j0; int j1 = j0 + 1 >= ECHO_LEN ? 0 : j0 + 1;
            eR = (tape[j0] + (tape[j1] - tape[j0]) * f2) * 0.85f;
        }
        /* reverb: predelay, diffusion, FDN */
        float rin = (dry * 0.6f + shaped * echo) * 0.5f;
        pre[prepos] = rin; prepos = (prepos + 1) % 4800;
        float x = pre[(prepos + 4800 - 1440) % 4800];             /* 30 ms predelay */
        x = allpass(0, x, 556, 0.6f);
        x = allpass(1, x, 441, 0.6f);
        float o[8], sum = 0.f;
        for (int k = 0; k < 8; k++) { o[k] = rbuf[k][rpos[k]]; sum += o[k]; }
        float h = sum * 0.25f;
        const float decay = 0.80f + rev * 0.17f;
        float rl = 0.f, rr = 0.f;
        for (int k = 0; k < 8; k++) {
            float v = (o[k] - h) * decay + x;
            rlp[k] += 0.45f * (v - rlp[k]);                      /* damping: dark, dubby */
            rbuf[k][rpos[k]] = rlp[k];
            rpos[k] = rpos[k] + 1 >= kLen[k] ? 0 : rpos[k] + 1;
            if (k & 1) rr += o[k]; else rl += o[k];
        }
        float rg = rev * 0.55f;
        float L = dry + eL * echo + rl * rg;
        float R = dry + eR * echo + rr * rg;
        stereo[2 * i] = knee(L * 0.85f);
        stereo[2 * i + 1] = knee(R * 0.85f);
    }
}
