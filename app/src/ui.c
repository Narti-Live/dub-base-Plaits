/* DUB STAB PLAITS app UI: a Plaits panel (positions taken from the Plaits panel
   artwork, plaits_v50.ai) driving the firmware, and under it a live view of
   how the sequence is generated and played. */
#include "plat.h"
#include "gfx.h"
#include "fw.h"
#include "ui.h"
#include "fx.h"

/* ---- panel geometry: Plaits artwork units (355 x 780) -> canvas */
#define SX (720.f / 355.f)
#define SY 1.85f
#define Y0 32.f
#define PX(x) ((x) * SX)
#define PY(y) (((y) - Y0) * SY)
#define PR(r) ((r) * SX)
#define PANEL_H ((int)PY(722))
#define STRIP_H 330
#define VIZ_Y (PANEL_H + STRIP_H)

#define C_PANEL HEX(0xf0f0ec)
#define C_INK HEX(0x1d1d1d)
#define C_TEAL HEX(0x16a39d)
#define C_PINK HEX(0xe2275e)
#define C_GRAY HEX(0x8b8b86)
#define C_BOX HEX(0x58595b)
#define C_BG HEX(0x131416)
#define C_CARD HEX(0x1d1f22)
#define C_LINE HEX(0x34373c)
#define C_TXT HEX(0xe8e8e4)
#define C_DIM HEX(0x8c9096)
#define C_YEL HEX(0xffd23a)
#define C_GRN HEX(0x48e070)
#define C_RED HEX(0xff4a3a)

UiPersist ui_persist;

typedef struct { float x, y, r; float range; } Knob;
static const Knob K[7] = {
    {70, 175, 30, 380}, {285, 175, 30, 380}, {70, 330, 22, 340}, {285, 330, 22, 340},
    {65, 478, 17, 300}, {178, 472, 17, 300}, {291, 478, 17, 300}};
static const float BTN[2][2] = {{152, 101}, {205, 101}};
#define BTN_R 16
#define LED_X 178
#define LED_Y(i) (145 + (i) * 31.7f)
enum { J_MODEL, J_TIMBRE, J_FM, J_MORPH, J_HARMO, J_TRIG, J_LEVEL, J_VOCT, J_OUT, J_AUX, J_N };
static const float JK[J_N][2] = {
    {42, 572}, {110, 572}, {178, 572}, {246, 572}, {313, 572},
    {42, 655}, {110, 655}, {178, 655}, {246, 655}, {313, 655}};
#define JACK_R 17

static const char *kModel[8] = {"DUB SAW", "VA", "WAVESHP", "FM", "PHASE D", "CHIPTUNE", "CHORDS", "STRINGS"};
static const char *kHTM[8][3] = {
    {"detune", "saw -> pulse", "noise chuff"}, {"osc 2 interval", "pulse width", "pulse <-> saw"},
    {"asymmetry", "fold", "triangle -> saw"}, {"ratio", "index", "feedback"},
    {"resonance", "distortion", "PD <-> resonant"}, {"duty cycle", "bit crush", "square <-> triangle"},
    {"brightness", "inversion", "waveform"}, {"brightness", "filter / ensemble", "registration"}};
static const char *kSpeed[8] = {"1/2", "1/4", "1/8", "1/8T", "1/16", "1/16T", "1/32", "1/32T"};
static const char *kRoman[7] = {"i", "ii", "III", "iv", "v", "VI", "VII"};
static const char *kNote[12] = {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};
static const int kMinorPc[7] = {0, 2, 3, 5, 7, 8, 10};
static const char *kVoicingName[8] = {"DUB voicing", "triad", "triad, 1st inversion", "triad, 2nd inversion",
                                      "7th chord", "7th, 1st inversion", "7th, 2nd inversion", "7th, 3rd inversion"};
/* the firmware variant this app was built with */
static int g_key, g_minor_only, g_voicings, g_variant_read;
static void read_variant(void) { if (!g_variant_read) { fw_variant(&g_key, &g_minor_only, &g_voicings); g_variant_read = 1; } }
static int key_pc(void) { return ((g_key % 12) + 12) % 12; }
static const char *key_name(void) { return key_pc() == 7 ? "G MINOR" : (key_pc() == 0 ? "C MINOR" : "MINOR"); }
static const char *key_word(void) { return key_pc() == 7 ? "G minor" : "C minor"; }
static int in_key(int pc) { static const int m[12] = {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0}; return m[((pc - key_pc()) % 12 + 12) % 12]; }
static int degree_root_pc(int d) { return (key_pc() + kMinorPc[((d % 7) + 7) % 7]) % 12; }
/* Names a chord from its notes: root, quality, 7th, and the bass if inverted. */
static void chord_label(const int8_t *notes, int n, int root_pc, char *out, int size) {
    int has[12] = {0}, bass = -1;
    for (int i = 0; i < n; i++) if (notes[i] >= 0) { has[((notes[i] - root_pc) % 12 + 12) % 12] = 1; if (bass < 0) bass = notes[i] % 12; }
    const char *q;
    if (has[4]) q = has[11] ? "maj7" : (has[10] ? "7" : "");
    else if (has[6] && !has[7]) q = has[10] ? "m7b5" : "dim";
    else q = has[10] ? "m7" : "m";
    if (bass >= 0 && bass != root_pc) fmt(out, size, "%s%s/%s", kNote[root_pc], q, kNote[bass]);
    else fmt(out, size, "%s%s", kNote[root_pc], q);
}
static const uint32_t kDegCol[7] = {HEX(0x2fb7ad), HEX(0x8a7fd0), HEX(0x4f8fe0), HEX(0xe0a63a),
                                    HEX(0xe2457a), HEX(0x73c25a), HEX(0xe0703a)};
static const int kStrongOffbeat[16] = {0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0};

/* ---- touch state */
enum { TG_NONE, TG_KNOB, TG_BUTTON, TG_JACK, TG_SCROLL, TG_STRIP, TG_FX };
typedef struct { int active, target, index, x0, y0, x, y, moved; float v0; } Ptr;
static Ptr ptrs[10];
static float scroll, scroll_v, last_scroll;
static int view_h = 1600;
static int press_frames[2], edited_in_hold[2];
static int knob_touch[7];

/* ---- live data */
static FwSnapshot S;
static int have_snap;
static uint32_t last_step_counter;
static float step_flash[16];
static int8_t prev_degree[16];
static uint8_t prev_rank[16];
static float gen_anim = 2.f;
static float lfo_hist[4][180];
static int lfo_pos;
static uint32_t frame_no;
static float scope[512];

static uint32_t *panel_bg;

int ui_content_height(void) { return VIZ_Y + 2970; }

void ui_defaults(void) {
    for (int i = 0; i < 7; i++) ui_persist.pots[i] = 0.5f;
    ui_persist.pots[0] = 0.45f;
    ui_persist.pots[4] = 0.4f;
    ui_persist.clock_mode = 0;
    ui_persist.bpm = 120.f;
    ui_persist.listen = 0;
    ui_persist.fx = fx_params;
}
void ui_apply(void) {
    for (int i = 0; i < 7; i++) hw_pots[i] = ui_persist.pots[i];
    fw_set_clock(ui_persist.clock_mode, ui_persist.bpm);
    fx_params = ui_persist.fx;
    fw_set_mix(1.8f, 0.f);  /* AUX is a pitch CV: only OUT goes to the speaker (+5 dB for phone speakers; the firmware is unchanged) */
}

/* =============================================================== panel art */
static void nut(float x, float y, float r) {
    disc(x, y + 3, r + 2, HEX(0x000000), 0.10f);
    shaded_disc(x, y, r, HEX(0xa7a7a3), 1.f, 1.f);
    ring(x, y, r - 2.5f, r - 1.f, HEX(0x7d7d79), 0.6f);
    disc(x, y, r * 0.56f, HEX(0x2a2a2a), 1.f);
    disc(x, y, r * 0.42f, HEX(0x070707), 1.f);
}

static void draw_panel_bg(void) {
    fill_rect(0, 0, CW, PANEL_H, C_PANEL, 1.f);
    /* subtle brushed texture */
    uint32_t seed = 12345;
    for (int y = 0; y < PANEL_H; y++) {
        seed = seed * 1664525u + 1013904223u;
        float a = ((seed >> 24) & 15) / 15.f * 0.035f;
        fill_rect(0, y, CW, 1, HEX(0x9a9a94), a);
    }
    /* top band */
    float b0 = PY(44), b1 = PY(66);
    fill_rect(0, b0, CW, 2.5f, HEX(0xc3c3be), 1.f);
    fill_rect(0, b1 - 2.5f, CW, 2.5f, HEX(0xc3c3be), 1.f);
    for (float x = -60; x < CW + 60; x += 16) line(x, b1 - 3, x + (b1 - b0 - 6), b0 + 3, 2.5f, HEX(0xcfcfca), 1.f);
    text(F_TINY, 18, (b0 + b1) * 0.5f, "DUB STAB PLAITS", HEX(0x9b9b96), 1.f, AL_LEFT);
    read_variant();
    text(F_TINY, CW - 18, (b0 + b1) * 0.5f, g_minor_only ? (key_pc() == 7 ? "G MINOR - MINOR CHORDS" : "MINOR CHORDS") : key_name(), HEX(0x9b9b96), 1.f, AL_RIGHT);

    /* button hints and dotted links */
    for (int b = 0; b < 2; b++) {
        float x = PX(BTN[b][0]), y = PY(BTN[b][1]);
        disc(x, y + 4, PR(BTN_R) + 6, 0, 0.10f);
        ring(x, y, PR(BTN_R) + 1, PR(BTN_R) + 5, HEX(0xb9b9b4), 1.f);
        dotted(x, y + PR(BTN_R) + 12, x, PY(132), 7, 2.2f, C_INK);
    }
    text(F_SMALL, PX(124), PY(93), "tap  < prev", C_GRAY, 1.f, AL_RIGHT);
    text(F_SMALL, PX(124), PY(108), "hold  NEW", C_INK, 1.f, AL_RIGHT);
    text(F_SMALL, PX(233), PY(93), "next >  tap", C_GRAY, 1.f, AL_LEFT);
    text(F_SMALL, PX(233), PY(108), "RANDOM  hold", C_INK, 1.f, AL_LEFT);

    /* LED legends: chord models (teal) and speeds (pink) */
    for (int i = 0; i < 8; i++) {
        float y = PY(LED_Y(i));
        disc(PX(153), y, PR(8.5f), C_TEAL, 1.f);
        char n[4] = {(char)('1' + i), 0};
        text(F_TINY, PX(153), y, n, HEX(0xffffff), 1.f, AL_CENTER);
        text(F_TINY, PX(142), y, kModel[i], C_INK, 1.f, AL_RIGHT);
        text(F_TINY, PX(196), y, kSpeed[i], C_PINK, 1.f, AL_LEFT);
        ring(PX(LED_X), y, PR(10) + 1, PR(10) + 4, HEX(0xc9c7c0), 1.f);
    }

    /* knob shadows, labels */
    for (int k = 0; k < 7; k++) {
        float x = PX(K[k].x), y = PY(K[k].y), r = PR(K[k].r);
        glow(x + 2, y + 7, r * 1.25f, 0, 0.30f);
    }
    text(F_LABEL, PX(72), PY(248), "FREQUENCY", C_INK, 1.f, AL_CENTER);
    text(F_TINY, PX(66), PY(262), "ENVELOPE", C_TEAL, 1.f, AL_CENTER);
    text(F_TINY, PX(66), PY(272), "L: OCT   R: RESO", C_GRAY, 1.f, AL_CENTER);
    text(F_LABEL, PX(285), PY(248), "HARMONICS", C_INK, 1.f, AL_CENTER);
    /* TIMBRE / MORPH with the little Plaits pictograms */
    disc(PX(26), PY(398), PR(8), C_INK, 1.f);
    ring(PX(26), PY(398), PR(3.2f), PR(5.2f), C_PANEL, 1.f);
    text(F_LABEL, PX(40), PY(398), "TIMBRE", C_INK, 1.f, AL_LEFT);
    disc(PX(327), PY(398), PR(8), C_INK, 1.f);
    line(PX(323), PY(404), PX(331), PY(392), 3, C_PANEL, 1.f);
    text(F_LABEL, PX(314), PY(398), "MORPH", C_INK, 1.f, AL_RIGHT);
    text(F_LABEL, PX(178), PY(428), "FM", C_INK, 1.f, AL_CENTER);
    /* attenuverter marks */
    const char *att_fn[3] = {"DENSITY", "LENGTH", "SPEED"};
    for (int k = 4; k < 7; k++) {
        float x = PX(K[k].x), y = PY(K[k].y), r = PR(K[k].r);
        line(x, y - r - 24, x, y - r - 6, 4.5f, C_INK, 1.f);
        line(x - r - 8, y - r - 15, x - r + 4, y - r - 15, 3, C_INK, 1.f);
        line(x + r - 4, y - r - 15, x + r + 8, y - r - 15, 3, C_INK, 1.f);
        line(x + r + 2, y - r - 21, x + r + 2, y - r - 9, 3, C_INK, 1.f);
        text(F_TINY, x, y + r + 16, att_fn[k - 4], C_TEAL, 1.f, AL_CENTER);
    }
    /* dotted cables from attenuverters to their CV jacks (as on the panel) */
    dotted_curve(PX(65), PY(478) + PR(17) + 30, PX(100), PY(505), PX(110), PY(572) - PR(JACK_R) - 6, 9, 2.4f, C_INK);
    dotted(PX(178), PY(472) + PR(17) + 30, PX(178), PY(572) - PR(JACK_R) - 6, 9, 2.4f, C_INK);
    dotted_curve(PX(291), PY(478) + PR(17) + 30, PX(256), PY(505), PX(246), PY(572) - PR(JACK_R) - 6, 9, 2.4f, C_INK);
    dotted(PX(42) + PR(JACK_R) + 6, PY(655), PX(110) - PR(JACK_R) - 6, PY(655), 9, 2.4f, C_INK);

    /* jacks */
    text(F_LABEL, PX(45), PY(531), "MODEL", C_INK, 1.f, AL_CENTER);
    text(F_LABEL, PX(313), PY(531), "HARMO", C_INK, 1.f, AL_CENTER);
    round_rect(PX(217), PY(604), PX(343) - PX(217), PY(691) - PY(604), 10, C_BOX, 1.f);
    text(F_LABEL, PX(42), PY(619), "TRIG", C_INK, 1.f, AL_CENTER);
    text(F_LABEL, PX(110), PY(619), "LEVEL", C_INK, 1.f, AL_CENTER);
    text(F_LABEL, PX(178), PY(619), "V/OCT", C_INK, 1.f, AL_CENTER);
    text(F_LABEL, PX(246), PY(619), "OUT", HEX(0xffffff), 1.f, AL_CENTER);
    text(F_LABEL, PX(313), PY(619), "AUX", HEX(0xffffff), 1.f, AL_CENTER);
    for (int j = 0; j < J_N; j++) nut(PX(JK[j][0]), PY(JK[j][1]), PR(JACK_R));
    text(F_TINY, PX(42), PY(684), "CLOCK IN", C_TEAL, 1.f, AL_CENTER);
    fill_rect(PX(15), PY(704), PX(340) - PX(15), 4, HEX(0xc3c3be), 1.f);
    text(F_TINY, PX(130), PY(714), "tap TRIG to patch the clock", C_GRAY, 1.f, AL_CENTER);
}

/* =============================================================== dynamic panel */
static float knob_angle(float v) { return (-150.f + 300.f * v) * (PI_F / 180.f); }

static void draw_knob(int k, float v, float eff, int touched) {
    float x = PX(K[k].x), y = PY(K[k].y), r = PR(K[k].r);
    int att = k >= 4;
    float a = knob_angle(v), s = sinf(a), c = -cosf(a);
    if (!att) {
        shaded_disc(x, y, r, HEX(0xcfcfcb), 1.f, 1.f);              /* skirt */
        for (int i = 0; i < 36; i++) {                                /* knurling */
            float q = i * (2 * PI_F / 36);
            line(x + sinf(q) * r * 0.86f, y - cosf(q) * r * 0.86f, x + sinf(q) * r * 0.97f, y - cosf(q) * r * 0.97f,
                 1.6f, HEX(0xb3b3ae), 0.7f);
        }
        shaded_disc(x, y, r * 0.80f, HEX(0xeeeeea), 0.8f, 1.f);      /* cap */
        line(x + s * r * 0.18f, y + c * r * 0.18f, x + s * r * 0.97f, y + c * r * 0.97f, r * 0.085f + 1.5f, C_INK, 1.f);
    } else {
        shaded_disc(x, y, r, HEX(0xf6f6f3), 0.7f, 1.f);
        ring(x, y, r - 2, r, HEX(0xb9b9b4), 1.f);
        line(x - s * r * 0.15f, y - c * r * 0.15f, x + s * r * 0.92f, y + c * r * 0.92f, 4.f, C_INK, 1.f);
    }
    if (eff >= 0.f) {   /* where the parameter really is (catch-up after RANDOM, LFO drift) */
        float e = knob_angle(eff);
        disc(x + sinf(e) * (r + 9), y - cosf(e) * (r + 9), 4.5f, C_PINK, 0.95f);
    }
    if (touched) ring(x, y, r + 14, r + 17, C_TEAL, 0.9f);
}

static void draw_button(int b) {
    float x = PX(BTN[b][0]), y = PY(BTN[b][1]), r = PR(BTN_R);
    int p = hw_buttons[b];
    shaded_disc(x, y + (p ? 2 : 0), r, p ? HEX(0x4a4c50) : HEX(0x66686c), 1.f, 1.f);
    ring(x, y + (p ? 2 : 0), r * 0.72f, r * 0.76f, HEX(0x000000), 0.15f);
    if (p && !edited_in_hold[b]) {       /* hold progress towards NEW / RANDOM (1 s) */
        float t = press_frames[b] / 30.f;
        if (t > 0.2f) {
            int n = (int)(clampf(t, 0.f, 1.f) * 48);
            for (int i = 0; i < n; i++) {
                float q = i * (2 * PI_F / 48);
                disc(x + sinf(q) * (r + 12), y - cosf(q) * (r + 12), 3.f, t >= 1.f ? (b ? C_YEL : C_GRN) : C_TEAL, 1.f);
            }
        }
    }
}

static void draw_led(int i) {
    float x = PX(LED_X), y = PY(LED_Y(i)), r = PR(10);
    float rr = sqrt_(S.led[i][0]), gg = sqrt_(S.led[i][1]);   /* the eye is not linear */
    float br = rr > gg ? rr : gg;
    uint32_t col = RGB((int)clampf(255 * rr + 40 * gg, 0, 255), (int)clampf(60 * rr + 235 * gg, 0, 255), (int)(40 * gg + 30 * rr));
    if (rr > 0.15f && gg > 0.15f) col = mix(col, C_YEL, 0.5f);
    if (br > 0.02f) glow(x, y, r * 2.3f, col, 0.5f * br);
    shaded_disc(x, y, r, mix(HEX(0xdcdad2), col, clampf(br * 1.4f, 0, 1)), 0.6f, 1.f);
    if (br > 0.05f) disc(x - r * 0.25f, y - r * 0.3f, r * 0.35f, HEX(0xffffff), 0.45f * br);
}

static void draw_plug(int j, uint32_t cable) {
    float x = PX(JK[j][0]), y = PY(JK[j][1]), r = PR(JACK_R);
    /* cable hangs down out of the panel */
    float ex = x + (j == J_TRIG ? -30 : (j == J_AUX ? 34 : 6)), ey = PANEL_H + 40;
    float px = x, py = y;
    for (int i = 1; i <= 24; i++) {
        float t = i / 24.f, u = 1 - t;
        float cx = x, cy = y + 160;
        float qx = u * u * x + 2 * u * t * cx + t * t * ex, qy = u * u * y + 2 * u * t * cy + t * t * ey;
        line(px, py, qx, qy, r * 0.62f, scale_rgb(cable, 0.75f), 1.f);
        line(px - 2, py, qx - 2, qy, r * 0.22f, scale_rgb(cable, 1.25f), 0.5f);
        px = qx; py = qy;
    }
    disc(x + 2, y + 5, r * 1.05f, 0, 0.25f);
    shaded_disc(x, y, r * 0.98f, HEX(0x2b2d31), 1.f, 1.f);
    ring(x, y, r * 0.55f, r * 0.68f, cable, 1.f);
    disc(x - r * 0.3f, y - r * 0.35f, r * 0.2f, HEX(0xffffff), 0.25f);
}

static void popup(float x, float y, const char *l1, const char *l2) {
    int w1 = text_width(F_MED, l1), w2 = l2 ? text_width(F_SMALL, l2) : 0;
    int w = (w1 > w2 ? w1 : w2) + 32, h = l2 ? 74 : 46;
    float bx = clampf(x - w * 0.5f, 8, CW - 8 - w), by = y - h;
    round_rect(bx + 2, by + 4, w, h, 12, 0, 0.25f);
    round_rect(bx, by, w, h, 12, HEX(0x202226), 0.96f);
    text(F_MED, bx + w * 0.5f, by + 23, l1, HEX(0xffffff), 1.f, AL_CENTER);
    if (l2) text(F_SMALL, bx + w * 0.5f, by + 52, l2, HEX(0xb8bcc2), 1.f, AL_CENTER);
}

static void knob_popup(int k) {
    char a[96], b[128];
    float x = PX(K[k].x), y = PY(K[k].y) - PR(K[k].r) - 22;
    int m = S.model & 7;
    switch (k) {
    case 0:
        if (S.cv_tune) { fmt(a, 96, "CV SCALE  %.2f %%", (S.cv_scale / 6553.6f - 1.f) * 100.f); fmt(b, 128, "until C5 on your synth is exactly 2 octaves above C3"); }
        else if (hw_buttons[0]) { fmt(a, 96, "OCTAVE  %d", S.octave); fmt(b, 128, "LEFT + FREQUENCY (noon = 3)"); }
        else if (hw_buttons[1]) { fmt(a, 96, "RESONANCE  %.2f", S.resonance); fmt(b, 128, "RIGHT + FREQUENCY  (Q %.1f, never self-oscillates)", S.q); }
        else {
            fmt(a, 96, "ENVELOPE  %.2f", S.envelope);
            fmt(b, 128, "attack %d ms   decay %.2f s   filter +%.1f oct", (int)(S.attack_s * 1000 + 0.5f), S.decay_s, S.env_oct);
        }
        break;
    case 1: case 2: case 3: {
        float v = k == 1 ? S.harmonics : (k == 2 ? S.timbre : S.morph);
        fmt(a, 96, "%s  %.2f", k == 1 ? "HARMONICS" : (k == 2 ? "TIMBRE" : "MORPH"), v);
        fmt(b, 128, "%s: %s", kModel[m], kHTM[m][k - 1]);
        break;
    }
    case 4: fmt(a, 96, "DENSITY  %d / 16", S.density); fmt(b, 128, "steps with rank < %d play", S.density); break;
    case 5: fmt(a, 96, "LENGTH  %d", S.length); fmt(b, 128, "the pattern loops after %d steps", S.length); break;
    default: fmt(a, 96, "SPEED  %s", kSpeed[S.speed & 7]); fmt(b, 128, S.speed == 3 || S.speed == 5 || S.speed == 7 ? "triplet (red LED)" : "steps per TRIG pulse (16ths)"); break;
    }
    popup(x, y, a, b);
}

static void draw_panel(void) {
    /* blit the static artwork (only visible rows) */
    int y0 = (int)T.oy, y1 = y0 + T.h;
    for (int y = y0 < 0 ? 0 : y0; y < y1 && y < PANEL_H; y++)
        memcpy(T.px + (size_t)(y - y0) * T.stride, panel_bg + (size_t)y * CW, CW * 4);
    if (y0 > PANEL_H) return;
    int m = S.model & 7;
    (void)m;
    float eff[7] = {-1, -1, -1, -1, -1, -1, -1};
    if (have_snap) {
        eff[0] = S.envelope;
        eff[1] = clampf(S.harmonics + S.lfo[0] * 0.075f, 0, 1);
        eff[2] = clampf(S.timbre + S.lfo[1] * 0.075f, 0, 1);
        eff[3] = clampf(S.morph + S.lfo[2] * 0.075f, 0, 1);
    }
    for (int k = 0; k < 7; k++) draw_knob(k, ui_persist.pots[k], eff[k], knob_touch[k]);
    for (int b = 0; b < 2; b++) draw_button(b);
    for (int i = 0; i < 8; i++) draw_led(i);
    if (ui_persist.clock_mode) draw_plug(J_TRIG, HEX(0x2f86a8));
    draw_plug(J_OUT, HEX(0xc23c3c));
    {   /* the pitch CV on AUX, as a voltage tag */
        char t[32]; fmt(t, 32, "%c%.2f V", S.cv_volts < 0 ? '-' : '+', S.cv_volts < 0 ? -S.cv_volts : S.cv_volts);
        round_rect(PX(313) - 56, PY(655) + PR(JACK_R) + 8, 112, 30, 8, HEX(0xd6a62a), 1.f);
        text(F_SMALL, PX(313), PY(655) + PR(JACK_R) + 23, t, HEX(0x1d1d1d), 1.f, AL_CENTER);
    }
    if (S.cv_tune) {
        char a[64], b[96];
        fmt(a, 64, "CV TUNE   AUX = %s", S.cv_tune == 1 ? "C3  (0 V)" : "C5  (+2 V)");
        fmt(b, 96, "scale %.1f %%   turn FREQUENCY, tap a button to save", (S.cv_scale / 6553.6f - 1.f) * 100.f);
        popup(CW * 0.5f, PY(545), a, b);
    }
    if (ui_persist.clock_mode) {
        char t[32]; fmt(t, 32, "%d BPM", (int)(ui_persist.bpm + 0.5f));
        round_rect(PX(42) - 52, PY(655) - PR(JACK_R) - 46, 104, 32, 8, HEX(0x2f86a8), 1.f);
        text(F_SMALL, PX(42), PY(655) - PR(JACK_R) - 30, t, HEX(0xffffff), 1.f, AL_CENTER);
    }
    for (int k = 0; k < 7; k++) if (knob_touch[k]) knob_popup(k);
}

/* =============================================================== strip */
typedef struct { float x, y, w, h; } Box;
static void draw_fx(void);
static Box strip_box(int i) {   /* 0 int, 1 ext, 2 -, 3 bpm, 4 +, 5 out, 6 aux, 7 both */
    float y = PANEL_H;
    switch (i) {
    case 0: return (Box){150, y + 22, 170, 64};
    case 1: return (Box){326, y + 22, 120, 64};
    case 2: return (Box){462, y + 22, 64, 64};
    case 3: return (Box){530, y + 22, 112, 64};
    case 4: return (Box){646, y + 22, 64, 64};
    case 5: return (Box){150, y + 108, 150, 64};
    case 6: return (Box){306, y + 108, 150, 64};
    default: return (Box){462, y + 108, 248, 64};
    }
}
static void seg(Box b, const char *l1, const char *l2, int on) {
    round_rect(b.x, b.y, b.w, b.h, 10, on ? C_TEAL : HEX(0x2c2f34), 1.f);
    if (l2) {
        text(F_MED, b.x + b.w * 0.5f, b.y + 23, l1, on ? HEX(0xffffff) : C_TXT, 1.f, AL_CENTER);
        text(F_TINY, b.x + b.w * 0.5f, b.y + 47, l2, on ? HEX(0xe0fffb) : C_DIM, 1.f, AL_CENTER);
    } else text(F_MED, b.x + b.w * 0.5f, b.y + b.h * 0.5f, l1, on ? HEX(0xffffff) : C_TXT, 1.f, AL_CENTER);
}
static void draw_strip(void) {
    float y = PANEL_H;
    fill_rect(0, y, CW, STRIP_H, HEX(0x222427), 1.f);
    fill_rect(0, y, CW, 3, HEX(0x0c0c0d), 1.f);
    text(F_SMALL, 24, y + 44, "TRIG", C_TXT, 1.f, AL_LEFT);
    text(F_TINY, 24, y + 66, "clock in", C_DIM, 1.f, AL_LEFT);
    seg(strip_box(0), "UNPATCHED", "internal 120 BPM", !ui_persist.clock_mode);
    seg(strip_box(1), "CLOCK", "16ths", ui_persist.clock_mode);
    seg(strip_box(2), "-", 0, 0);
    Box b = strip_box(3);
    char t[16]; fmt(t, 16, "%d", (int)(ui_persist.bpm + 0.5f));
    round_rect(b.x, b.y, b.w, b.h, 10, HEX(0x16181b), 1.f);
    text(F_BIG, b.x + b.w * 0.5f, b.y + 26, t, ui_persist.clock_mode ? HEX(0xffffff) : C_DIM, 1.f, AL_CENTER);
    text(F_TINY, b.x + b.w * 0.5f, b.y + 52, "BPM  < drag >", C_DIM, 1.f, AL_CENTER);
    seg(strip_box(4), "+", 0, 0);
    text(F_SMALL, 24, y + 130, "AUX", C_TXT, 1.f, AL_LEFT);
    text(F_TINY, 24, y + 152, "pitch CV out", C_DIM, 1.f, AL_LEFT);
    {
        Box c = {150, y + 108, CW - 160, 64};
        round_rect(c.x, c.y, c.w, c.h, 10, HEX(0x16181b), 1.f);
        char t[64], nn[8];
        float v = S.cv_volts;
        int note = (int)__builtin_floorf(48 + v * 12 + 0.5f);
        fmt(nn, 8, "%s%d", kNote[((note % 12) + 12) % 12], note / 12 - 1);
        fmt(t, 64, "%c%.2f V", v < 0 ? '-' : '+', v < 0 ? -v : v);
        text(F_BIG, c.x + 16, c.y + 32, t, HEX(0xd6a62a), 1.f, AL_LEFT);
        text(F_MED, c.x + 190, c.y + 22, nn, C_TXT, 1.f, AL_LEFT);
        text(F_TINY, c.x + 190, c.y + 46, "chord root, 1 V/oct, C3 = 0 V", C_DIM, 1.f, AL_LEFT);
        text(F_TINY, c.x + c.w - 14, c.y + 22, "tune: hold", C_DIM, 1.f, AL_RIGHT);
        text(F_TINY, c.x + c.w - 14, c.y + 44, "both 2 s", C_DIM, 1.f, AL_RIGHT);
    }
    draw_fx();
}

/* ---- FX row (app only): tape echo + reverb */
static Box fx_box(int i) { float w = (CW - 160 - 4 * 6) / 5.f; return (Box){150 + i * (w + 6), PANEL_H + 196, w, 110}; }
static float *fx_val(int i) {
    switch (i) { case 0: return &ui_persist.fx.echo; case 2: return &ui_persist.fx.repeats; case 3: return &ui_persist.fx.wow; case 4: return &ui_persist.fx.reverb; }
    return 0;
}
static void draw_fx(void) {
    float y = PANEL_H + 196;
    text(F_SMALL, 24, y + 22, "FX", C_TXT, 1.f, AL_LEFT);
    text(F_TINY, 24, y + 44, "tape echo", C_DIM, 1.f, AL_LEFT);
    text(F_TINY, 24, y + 64, "+ reverb", C_DIM, 1.f, AL_LEFT);
    text(F_TINY, 24, y + 92, "app only", HEX(0x6a6f78), 1.f, AL_LEFT);
    static const char *nm[5] = {"ECHO", "TIME", "REPEATS", "WOW", "REVERB"};
    for (int i = 0; i < 5; i++) {
        Box b = fx_box(i);
        round_rect(b.x, b.y, b.w, b.h, 10, HEX(0x16181b), 1.f);
        text(F_TINY, b.x + b.w * 0.5f, b.y + 18, nm[i], C_DIM, 1.f, AL_CENTER);
        char t[24];
        if (i == 1) {
            text(F_MED, b.x + b.w * 0.5f, b.y + 56, fx_time_names[ui_persist.fx.time % 6], HEX(0xffffff), 1.f, AL_CENTER);
            text(F_TINY, b.x + b.w * 0.5f, b.y + 86, "tap / drag", C_DIM, 1.f, AL_CENTER);
        } else {
            float v = *fx_val(i);
            fmt(t, 24, "%d", (int)(v * 100 + 0.5f));
            text(F_BIG, b.x + b.w * 0.5f, b.y + 56, t, v > 0.01f ? HEX(0xffffff) : C_DIM, 1.f, AL_CENTER);
            fill_rect(b.x + 10, b.y + b.h - 18, b.w - 20, 5, HEX(0x2c2f34), 1.f);
            fill_rect(b.x + 10, b.y + b.h - 18, (b.w - 20) * v, 5, i == 4 ? HEX(0x7f95b8) : C_TEAL, 1.f);
        }
    }
}

/* =============================================================== visualisation */
static void card(float y, float h, const char *title, const char *sub) {
    round_rect(14, y, CW - 28, h, 16, C_CARD, 1.f);
    text(F_MED, 34, y + 30, title, C_TXT, 1.f, AL_LEFT);
    if (sub) text(F_TINY, CW - 34, y + 31, sub, C_DIM, 1.f, AL_RIGHT);
}
static float seconds_since_step(void) { return (float)(S.blocks - S.last.block) / 4000.f; }
static float env_at(float t) {
    if (t < 0) return 0;
    if (t < S.attack_s) return t / S.attack_s;
    return expf(-6.9f * (t - S.attack_s) / S.decay_s);
}

#define GX 30.f
#define GW ((CW - 60.f) / 16.f)
static float draw_grid(float y) {
    float h = 500;
    char t[128];
    card(y, h, "1  THE PATTERN", "hold LEFT 1 s = NEW");
    fmt(t, 128, "step %d / %d     density %d     speed %s     %s", S.step + 1, S.length, S.density, kSpeed[S.speed & 7],
        S.clocked ? "TRIG clock" : "internal 120 BPM");
    text(F_SMALL, 34, y + 62, t, C_DIM, 1.f, AL_LEFT);
    float anim = gen_anim;
    /* chord row: runs of the same degree */
    float ry = y + 84, rh = 58;
    int i = 0;
    while (i < 16) {
        int j = i;
        while (j + 1 < 16 && S.degree[j + 1] == S.degree[i] && S.voicing[j + 1] == S.voicing[i]) j++;
        int d = S.degree[i] % 7;
        char cl[24]; chord_label(S.chord[i], 5, degree_root_pc(d), cl, 24);
        float vis = clampf((anim * 16.f / 0.35f) - i, 0.f, 1.f);
        if (vis > 0.f) {
            float x0 = GX + i * GW + 2, w = (j - i + 1) * GW - 4;
            round_rect(x0, ry, w, rh, 9, kDegCol[d], 0.22f * vis);
            round_rect(x0, ry, w, 5, 2, kDegCol[d], vis);
            text(F_MED, x0 + w * 0.5f, ry + 22, kRoman[d], kDegCol[d], vis, AL_CENTER);
            if (w > text_width(F_TINY, cl) + 6)
                text(F_TINY, x0 + w * 0.5f, ry + 44, cl, C_TXT, vis * 0.85f, AL_CENTER);
        }
        i = j + 1;
    }
    /* rank bars */
    float by = y + 360, bh = 196;
    int knob_d = knob_touch[4], knob_l = knob_touch[5];
    for (i = 0; i < 16; i++) {
        float x0 = GX + i * GW;
        int r = S.rank[i];
        float vis = clampf(((anim - 0.35f) / 0.5f) * 16.f - r, 0.f, 1.f);
        float hh = (16 - r) / 16.f * bh * vis;
        int plays = r < S.density, inloop = i < S.length;
        uint32_t col = plays ? kDegCol[S.degree[i] % 7] : HEX(0x3a3d43);
        float fl = step_flash[i];
        if (i == S.step) round_rect(x0 + 1, by - bh - 4, GW - 2, bh + 8, 8, HEX(0xffffff), 0.07f + 0.25f * fl);
        round_rect(x0 + 5, by - hh, GW - 10, hh, 6, col, inloop ? (plays ? 0.75f + 0.25f * fl : 0.9f) : 0.3f);
        if (fl > 0.01f) glow(x0 + GW * 0.5f, by - hh, GW * 1.3f, col, 0.5f * fl);
        if (vis > 0.5f) {
            fmt(t, 128, "%d", r + 1);
            text(F_TINY, x0 + GW * 0.5f, by - hh + 13, t, plays ? HEX(0x101113) : C_DIM, inloop ? 1.f : 0.5f, AL_CENTER);
        }
        if (kStrongOffbeat[i]) {
            line(x0 + GW * 0.5f - 6, by + 14, x0 + GW * 0.5f, by + 7, 2.5f, C_PINK, 0.9f);
            line(x0 + GW * 0.5f + 6, by + 14, x0 + GW * 0.5f, by + 7, 2.5f, C_PINK, 0.9f);
        }
        if (S.accent[i] && vis > 0.9f) disc(x0 + GW * 0.5f, by + 30, 6, C_YEL, inloop ? 1.f : 0.4f);
        else ring(x0 + GW * 0.5f, by + 30, 4, 5.5f, HEX(0x4a4d53), 1.f);
        fmt(t, 128, "%d", i + 1);
        text(F_TINY, x0 + GW * 0.5f, by + 54, t, (i & 3) == 0 ? C_TXT : C_DIM, 1.f, AL_CENTER);
        if (!inloop) {
            for (float q = 0; q < bh; q += 14) line(x0 + 4, by - q, x0 + GW - 4, by - q - 10, 1.5f, HEX(0x2a2c30), 1.f);
        }
    }
    /* density "water line" */
    float ly = by - (16.5f - S.density) / 16.f * bh;
    line(GX, ly, CW - GX, ly, knob_d ? 4.f : 2.5f, C_GRN, 0.9f);
    fmt(t, 128, "DENSITY %d", S.density);
    round_rect(CW - GX - 132, ly - 32, 132, 26, 6, C_GRN, knob_d ? 1.f : 0.85f);
    text(F_TINY, CW - GX - 66, ly - 19, t, HEX(0x0d1f12), 1.f, AL_CENTER);
    /* loop bracket */
    float lx1 = GX + S.length * GW - 3, lyb = by + 74;
    line(GX + 3, lyb, lx1, lyb, knob_l ? 4.f : 3.f, C_YEL, 1.f);
    line(GX + 3, lyb - 8, GX + 3, lyb, 3, C_YEL, 1.f);
    line(lx1, lyb - 8, lx1, lyb, 3, C_YEL, 1.f);
    fmt(t, 128, "LENGTH %d", S.length);
    text(F_TINY, clampf(lx1 - 4, GX + 90, CW - 40), lyb + 17, t, C_YEL, 1.f, AL_RIGHT);
    /* legend */
    float gy = y + h - 22;
    line(40, gy - 5, 46, gy - 12, 2.5f, C_PINK, 1.f); line(52, gy - 5, 46, gy - 12, 2.5f, C_PINK, 1.f);
    text(F_TINY, 60, gy - 7, "off-beat priority", C_DIM, 1.f, AL_LEFT);
    disc(246, gy - 8, 6, C_YEL, 1.f);
    text(F_TINY, 258, gy - 7, "accent", C_DIM, 1.f, AL_LEFT);
    text(F_TINY, 340, gy - 7, "bar height = rank (1 = first to play)", C_DIM, 1.f, AL_LEFT);
    if (anim < 1.2f) {
        const char *ph = anim < 0.35f ? "NEW: picking chords..." : (anim < 0.85f ? "NEW: ranking the steps..." : "NEW: accents");
        round_rect(CW * 0.5f - 160, y + 8, 320, 40, 10, C_GRN, 0.95f);
        text(F_SMALL, CW * 0.5f, y + 28, ph, HEX(0x0d1f12), 1.f, AL_CENTER);
    }
    return h;
}

static void note_name(int n, char *o) { fmt(o, 8, "%s%d", kNote[n % 12], n / 12 - 1); }

static float draw_chord(float y) {
    float h = 330;
    char t[128];
    card(y, h, "2  THE CHORD NOW", S.last.num_notes ? kVoicingName[S.last.voicing & 7] : "");
    float age = seconds_since_step();
    float e = env_at(age);
    int d = S.last.num_notes ? S.last.degree % 7 : 0;
    if (S.last.num_notes) chord_label(S.last.notes, S.last.num_notes, ((S.last.root % 12) + 12) % 12, t, 128);
    else fmt(t, 128, "-");
    text(F_BIG, 34, y + 84, t, S.last.num_notes ? kDegCol[d] : C_DIM, 1.f, AL_LEFT);
    int cw = text_width(F_BIG, t);
    fmt(t, 128, "degree %s of %s   step %d%s", kRoman[d], key_word(), S.last.step + 1, S.last.accent ? "   ACCENT" : "");
    text(F_SMALL, 34 + cw + 18, y + 80, t, C_DIM, 1.f, AL_LEFT);
    char notes[80] = {0}; int p = 0;
    for (int k = 0; k < S.last.num_notes && k < 5; k++) {
        char nn[8]; note_name(S.last.notes[k], nn);
        for (char *q = nn; *q; q++) notes[p++] = *q;
        notes[p++] = ' '; notes[p++] = ' ';
    }
    notes[p] = 0;
    text(F_MED, 34, y + 124, notes, C_TXT, 1.f, AL_LEFT);
    /* keyboard: 4 octaves around the register */
    int lo = 12 * (S.octave + 1) - 12;
    int whites = 29;
    float kx = 30, kw = (CW - 60.f) / whites, ky = y + 152, kh = 150;
    int lit[128] = {0};
    for (int k = 0; k < S.last.num_notes && k < 5; k++) { int n = S.last.notes[k]; if (n >= 0 && n < 128) lit[n] = 1; }
    int wi = 0;
    for (int n = lo; n <= lo + 48; n++) {
        int pc = n % 12;
        if (pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10) continue;
        float x = kx + wi * kw;
        uint32_t c = lit[n] ? mix(HEX(0xd9d9d4), C_YEL, 0.35f + 0.65f * e) : HEX(0xd9d9d4);
        round_rect(x + 1, ky, kw - 2, kh, 4, c, 1.f);
        if (in_key(pc)) disc(x + kw * 0.5f, ky + kh - 14, 4, C_TEAL, 1.f);
        if (pc == 0) { char nn[8]; note_name(n, nn); text(F_TINY, x + kw * 0.5f, ky + kh - 34, nn, HEX(0x6b6b66), 1.f, AL_CENTER); }
        wi++;
    }
    wi = 0;
    for (int n = lo; n <= lo + 48; n++) {
        int pc = n % 12;
        if (!(pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10)) { wi++; continue; }
        float x = kx + wi * kw - kw * 0.32f;
        uint32_t c = lit[n] ? mix(HEX(0x26282b), C_YEL, 0.35f + 0.65f * e) : HEX(0x26282b);
        round_rect(x, ky, kw * 0.64f, kh * 0.6f, 3, c, 1.f);
        if (in_key(pc)) disc(x + kw * 0.32f, ky + kh * 0.6f - 12, 3.5f, C_TEAL, 1.f);
    }
    if (S.last.num_notes && e > 0.01f)
        for (int n = lo; n <= lo + 48; n++) if (lit[n]) { (void)n; }
    fmt(t, 128, "AUX  %c%.2f V", S.cv_volts < 0 ? '-' : '+', S.cv_volts < 0 ? -S.cv_volts : S.cv_volts);
    text(F_MED, CW - 34, y + 124, t, HEX(0xd6a62a), 1.f, AL_RIGHT);
    return h;
}

static float draw_how(float y) {
    char t[700];
    float h = 620;
    card(y, h, "3  HOW NEW MAKES A PATTERN", "same code as the module");
    /* describe this pattern's chord runs */
    char prog[200]; int p = 0;
    int i = 0, runs = 0, passing = 0, last_d = -1;
    while (i < 16) {
        int j = i; while (j + 1 < 16 && S.degree[j + 1] == S.degree[i]) j++;
        int d = S.degree[i] % 7;
        if (j == i && i > 0 && i < 15 && S.degree[i - 1] == S.degree[i + 1]) passing++;
        else if (d != last_d) {
            if (runs && p < 180) { prog[p++] = ' '; prog[p++] = '>'; prog[p++] = ' '; }
            char cl[24]; chord_label(S.chord[i], 5, degree_root_pc(d), cl, 24);
            for (const char *q = cl; *q && p < 190; q++) prog[p++] = *q;
            runs++;
            last_d = d;
        }
        i = j + 1;
    }
    prog[p] = 0;
    int playing = 0, acc = 0;
    for (i = 0; i < S.length; i++) { if (S.rank[i] < S.density) playing++; if (S.accent[i] && S.rank[i] < S.density) acc++; }
    float yy = y + 62;
    struct { const char *n; const char *s; } st[6] = {
        {"1", g_minor_only ? "Chords: 1, 2 or 4 per pattern (35 / 40 / 25 %). The first is the tonic i 70 % of the time; the next ones are drawn with weights i 4, iv 2.5, v 2, ii 0.7, never the same twice in a row. No major chords."
                           : "Chords: 1, 2 or 4 per pattern (35 / 40 / 25 %). The first is the tonic i 70 % of the time; the next ones are drawn with weights i 4, iv 2.5, v 2, VI 2, III 1.5, VII 1, ii 0.7, never the same twice in a row."},
        {"2", g_voicings ? "Each section gets a voicing: triad 40 %, 7th chord 40 %, DUB 20 %, in any inversion. Each step takes its section's chord; 6 % get a passing chord."
                         : "Each step takes the chord of its section of the bar. 6 % of steps get a passing chord."},
        {"3", "Every step gets a rank = off-beat bonus + random. The 3rd 16th of each beat gets the biggest bonus, so it plays first: the classic dub off-beat stab."},
        {"4", "DENSITY plays the steps whose rank is below it. Raise it and steps join in rank order, so the groove grows without changing."},
        {"5", "Accents: 35 % on the strong off-beats, 15 % elsewhere. An accent opens the filter further."},
        {"6", "LENGTH loops the first N steps. SPEED turns each TRIG pulse (a 16th) into 1/2 ... 1/32T steps."}};
    for (int k = 0; k < 6; k++) {
        disc(44, yy + 13, 13, C_TEAL, 1.f);
        text(F_TINY, 44, yy + 13, st[k].n, HEX(0xffffff), 1.f, AL_CENTER);
        yy += text_wrap(F_SMALL, 70, yy, CW - 110, 26, st[k].s, C_TXT, 0.92f) + 12;
    }
    round_rect(30, yy, CW - 60, y + h - yy - 18, 10, HEX(0x16181b), 1.f);
    fmt(t, 700, "This pattern: %s%s", prog, passing ? " (+ passing chords)" : "");
    float th = text_wrap(F_SMALL, 46, yy + 10, CW - 92, 26, t, C_YEL, 1.f);
    fmt(t, 700, "%d of %d steps play, %d accented, every note in %s.", playing, S.length, acc, key_word());
    text_wrap(F_SMALL, 46, yy + 14 + th, CW - 92, 26, t, C_DIM, 1.f);
    return h;
}

static float draw_env(float y) {
    float h = 300;
    char t[128];
    card(y, h, "4  FREQUENCY = ENVELOPE", "attack + decay + filter on one knob");
    float px = 34, pw = CW - 68, py = y + 74, ph = 150;
    float span = S.attack_s + S.decay_s * 0.85f;
    if (span < 0.3f) span = 0.3f;
    round_rect(px, py, pw, ph, 8, HEX(0x16181b), 1.f);
    for (int g = 1; g < 4; g++) line(px, py + ph * g / 4, px + pw, py + ph * g / 4, 1, C_LINE, 1.f);
    float lx = px, la = py + ph, lf = py + ph;
    for (int i = 1; i <= 160; i++) {
        float tt = span * i / 160.f, x = px + pw * i / 160.f;
        float e = env_at(tt);
        float a = py + ph - e * (ph - 8);
        float fo = S.env_oct * e;                       /* filter octaves above base */
        float f = py + ph - fo / 6.f * (ph - 8);
        line(lx, lf, x, f, 2.5f, C_TEAL, 0.9f);
        line(lx, la, x, a, 3.f, C_YEL, 1.f);
        lx = x; la = a; lf = f;
    }
    float age = seconds_since_step();
    if (age < span && S.last.num_notes) {
        float x = px + pw * age / span, e = env_at(age);
        line(x, py, x, py + ph, 1.5f, HEX(0xffffff), 0.35f);
        disc(x, py + ph - e * (ph - 8), 7, C_YEL, 1.f);
        glow(x, py + ph - e * (ph - 8), 26, C_YEL, 0.5f);
    }
    fmt(t, 128, "attack %d ms", (int)(S.attack_s * 1000 + 0.5f));
    text(F_SMALL, 34, y + 250, t, C_YEL, 1.f, AL_LEFT);
    fmt(t, 128, "decay %.2f s", S.decay_s);
    text(F_SMALL, 200, y + 250, t, C_YEL, 1.f, AL_LEFT);
    fmt(t, 128, "filter env +%.1f oct", S.env_oct);
    text(F_SMALL, 350, y + 250, t, C_TEAL, 1.f, AL_LEFT);
    fmt(t, 128, "resonance %.2f", S.resonance);
    text(F_SMALL, CW - 34, y + 250, t, C_RED, 1.f, AL_RIGHT);
    fmt(t, 128, "window %.1f s   yellow = level, teal = filter cutoff, dot = now", span);
    text(F_TINY, 34, y + 278, t, C_DIM, 1.f, AL_LEFT);
    return h;
}

static float draw_lfo(float y) {
    float h = 320;
    char t[96];
    card(y, h, "5  RANDOM LFO", "always on: speed 20, depth 15");
    const char *nm[4] = {"HARMONICS", "TIMBRE", "MORPH", "CUTOFF"};
    float base[4] = {S.harmonics, S.timbre, S.morph, -1};
    for (int c = 0; c < 4; c++) {
        float ry = y + 66 + c * 56, rx = 200, rw = CW - 34 - rx, rh = 44;
        text(F_SMALL, 34, ry + 14, nm[c], C_TXT, 1.f, AL_LEFT);
        if (base[c] >= 0) fmt(t, 96, "%.2f %c %.2f", base[c], S.lfo[c] >= 0 ? '+' : '-', (S.lfo[c] < 0 ? -S.lfo[c] : S.lfo[c]) * 0.075f);
        else fmt(t, 96, "%c%.1f semitones", S.lfo[3] >= 0 ? '+' : '-', (S.lfo[3] < 0 ? -S.lfo[3] : S.lfo[3]) * 0.075f * 24.f);
        text(F_TINY, 34, ry + 36, t, C_DIM, 1.f, AL_LEFT);
        round_rect(rx, ry, rw, rh, 6, HEX(0x16181b), 1.f);
        line(rx, ry + rh * 0.5f, rx + rw, ry + rh * 0.5f, 1, C_LINE, 1.f);
        float lx = rx, ly = ry + rh * 0.5f;
        for (int i = 0; i < 180; i++) {
            float v = lfo_hist[c][(lfo_pos + i) % 180];
            float x = rx + rw * i / 179.f, yv = ry + rh * 0.5f - v * rh * 0.45f;
            if (i) line(lx, ly, x, yv, 2.5f, c == 3 ? C_TEAL : C_PINK, 0.9f);
            lx = x; ly = yv;
        }
        disc(lx, ly, 5, c == 3 ? C_TEAL : C_PINK, 1.f);
    }
    text(F_TINY, 34, y + h - 18, "12 s of history. The pink dots on the knobs show where the parameter really is.", C_DIM, 1.f, AL_LEFT);
    return h;
}

static float draw_scope(float y) {
    float h = 230;
    char t[64];
    card(y, h, "6  OUT", 0);
    fmt(t, 64, "level %d %%", (int)(S.meter * 100));
    text(F_TINY, CW - 34, y + 31, t, C_DIM, 1.f, AL_RIGHT);
    float px = 34, pw = CW - 68, py = y + 56, ph = 150;
    round_rect(px, py, pw, ph, 8, HEX(0x16181b), 1.f);
    line(px, py + ph * 0.5f, px + pw, py + ph * 0.5f, 1, C_LINE, 1.f);
    /* trigger on a rising zero crossing to keep the wave still */
    int st = 0;
    for (int i = 1; i < 256; i++) if (scope[i - 1] < 0 && scope[i] >= 0) { st = i; break; }
    float lx = px, ly = py + ph * 0.5f;
    for (int i = 0; i < 256; i++) {
        float v = clampf(scope[st + i] * 1.4f, -1, 1);
        float x = px + pw * i / 255.f, yy = py + ph * 0.5f - v * ph * 0.46f;
        if (i) line(lx, ly, x, yy, 2.2f, C_YEL, 0.95f);
        lx = x; ly = yy;
    }
    fill_rect(px, py + ph + 10, pw * clampf(S.meter, 0, 1), 6, S.meter > 0.95f ? C_RED : C_GRN, 1.f);
    return h;
}

static float draw_leds_help(float y) {
    float h = 420;
    card(y, h, "7  WHAT THE LEDS SAY", 0);
    for (int i = 0; i < 8; i++) {
        float x = 50 + i * 40, yy = y + 74;
        float rr = sqrt_(S.led[i][0]), gg = sqrt_(S.led[i][1]), br = rr > gg ? rr : gg;
        uint32_t col = RGB((int)clampf(255 * rr + 40 * gg, 0, 255), (int)clampf(60 * rr + 235 * gg, 0, 255), 40);
        if (rr > 0.15f && gg > 0.15f) col = mix(col, C_YEL, 0.5f);
        if (br > 0.02f) glow(x, yy, 30, col, 0.5f * br);
        disc(x, yy, 12, mix(HEX(0x3a3c40), col, clampf(br * 1.4f, 0, 1)), 1.f);
    }
    text(F_TINY, 380, y + 74, "live mirror of the 8 LEDs", C_DIM, 1.f, AL_LEFT);
    const char *lines =
        "Model: one yellow LED (others dim green). It blinks off on every step that plays.\n"
        "Turning a control shows its value for a second: envelope = yellow bar, resonance = red bar, "
        "density = green bar, length = yellow bar, octave = one yellow LED, speed = one LED (red = triplet).\n"
        "Holding LEFT: green pulse, then a green sweep = NEW. Holding RIGHT: yellow pulse = RANDOM.\n"
        "Both buttons 2 s = CV TUNE: LEDs 1-4 green while AUX sends C3 (0 V), LEDs 5-8 while it sends C5 (+2 V); "
        "the red LED shows the scale trim. Tap a button to save.";
    text_wrap(F_SMALL, 34, y + 106, CW - 68, 27, lines, C_TXT, 0.92f);
    return h;
}

static void draw_viz(void) {
    float y = VIZ_Y;
    if (y - T.oy > T.h) return;
    fill_rect(0, y, CW, ui_content_height() - y, C_BG, 1.f);
    y += 14;
    y += draw_grid(y) + 14;
    y += draw_chord(y) + 14;
    y += draw_how(y) + 14;
    y += draw_env(y) + 14;
    y += draw_lfo(y) + 14;
    y += draw_scope(y) + 14;
    y += draw_leds_help(y) + 14;
    text_wrap(F_TINY, 34, y, CW - 68, 22,
              g_voicings ? "This app runs the DUB STAB PLAITS v1.5 firmware code itself (ui.cc, voice.cc, dub.cc and the Plaits "
              "engines), polled 4000 times a second like the module. Only the knobs, buttons, LEDs and flash memory "
              "are replaced by the screen. The tape echo and reverb are app-only. Plaits by Emilie Gillet, MIT license." :
              "This app runs the DUB STAB PLAITS v1.4 firmware code itself (ui.cc, voice.cc, dub.cc and the Plaits "
              "engines), polled 4000 times a second like the module. Only the knobs, buttons, LEDs and flash memory "
              "are replaced by the screen. The tape echo and reverb are app-only. Plaits by Emilie Gillet, MIT license.", C_DIM, 1.f);
}

/* =============================================================== frame */
static void update(void) {
    FwSnapshot s;
    if (fw_snapshot(&s)) {
        if (have_snap && (memcmp(s.rank, prev_rank, 16) || memcmp(s.degree, prev_degree, 16))) gen_anim = 0.f;
        memcpy(prev_rank, s.rank, 16); memcpy(prev_degree, s.degree, 16);
        if (have_snap && s.step_counter != last_step_counter) step_flash[s.last.step & 15] = 1.f;
        last_step_counter = s.step_counter;
        S = s; have_snap = 1;
    }
    for (int i = 0; i < 16; i++) step_flash[i] *= 0.86f;
    if (gen_anim < 2.f) gen_anim += 1.f / 45.f;
    if ((frame_no & 1) == 0) {
        for (int c = 0; c < 4; c++) lfo_hist[c][lfo_pos] = S.lfo[c];
        lfo_pos = (lfo_pos + 1) % 180;
    }
    fw_scope(scope, 512);
    for (int b = 0; b < 2; b++) press_frames[b] = hw_buttons[b] ? press_frames[b] + 1 : 0;
    /* scrolling with a little inertia */
    int dragging = 0;
    for (int i = 0; i < 10; i++) if (ptrs[i].active && ptrs[i].target == TG_SCROLL) dragging = 1;
    if (dragging) { scroll_v = scroll - last_scroll; }
    else if (scroll_v != 0.f) {
        scroll += scroll_v; scroll_v *= 0.93f;
        if (scroll_v < 0.3f && scroll_v > -0.3f) scroll_v = 0.f;
    }
    float maxs = ui_content_height() - view_h;
    if (maxs < 0) maxs = 0;
    if (scroll < 0) { scroll = 0; scroll_v = 0; }
    if (scroll > maxs) { scroll = maxs; scroll_v = 0; }
    last_scroll = scroll;
    frame_no++;
}

void ui_render(uint32_t *canvas, int w, int h) {
    (void)w;
    if (!panel_bg) {
        panel_bg = (uint32_t *)malloc((size_t)CW * PANEL_H * 4);
        T.px = panel_bg; T.w = CW; T.h = PANEL_H; T.stride = CW; T.oy = 0;
        draw_panel_bg();
    }
    view_h = h;
    update();
    T.px = canvas; T.w = CW; T.h = h; T.stride = CW; T.oy = (float)(int)scroll;
    draw_panel();
    if (T.oy + h > PANEL_H) { draw_strip(); draw_viz(); }
    /* the scroll position, as a thin bar */
    float frac = scroll / (ui_content_height() - h > 1 ? ui_content_height() - h : 1);
    float bh = h * (float)h / ui_content_height();
    fill_rect(CW - 6, T.oy + frac * (h - bh), 4, bh, HEX(0x888888), 0.35f);
}

/* =============================================================== touch */
static float dist2(float x, float y, float cx, float cy) { return (x - cx) * (x - cx) + (y - cy) * (y - cy); }
static int in_box(Box b, float x, float y) { return x >= b.x && x < b.x + b.w && y >= b.y && y < b.y + b.h; }

void ui_touch(int type, int id, int x, int y_screen) {
    if (id < 0 || id >= 10) return;
    Ptr *p = &ptrs[id];
    float y = y_screen + scroll;
    if (type == UI_DOWN) {
        p->active = 1; p->x0 = p->x = x; p->y0 = p->y = y_screen; p->moved = 0;
        p->target = TG_SCROLL; p->index = -1;
        scroll_v = 0;
        for (int b = 0; b < 2; b++) {
            float r = PR(BTN_R) * 1.7f;
            if (dist2(x, y, PX(BTN[b][0]), PY(BTN[b][1])) < r * r) {
                p->target = TG_BUTTON; p->index = b; hw_buttons[b] = 1; edited_in_hold[b] = 0; return;
            }
        }
        for (int k = 0; k < 7; k++) {
            float r = PR(K[k].r) * (k >= 4 ? 1.6f : 1.3f);
            if (dist2(x, y, PX(K[k].x), PY(K[k].y)) < r * r) {
                p->target = TG_KNOB; p->index = k; p->v0 = ui_persist.pots[k]; knob_touch[k]++; return;
            }
        }
        int jacks[1] = {J_TRIG};
        for (int i = 0; i < 1; i++) {
            float r = PR(JACK_R) * 1.8f;
            if (dist2(x, y, PX(JK[jacks[i]][0]), PY(JK[jacks[i]][1])) < r * r) { p->target = TG_JACK; p->index = jacks[i]; return; }
        }
        for (int i = 0; i < 5; i++) if (in_box(strip_box(i), x, y)) { p->target = TG_STRIP; p->index = i; p->v0 = ui_persist.bpm; return; }
        for (int i = 0; i < 5; i++) if (in_box(fx_box(i), x, y)) {
            p->target = TG_FX; p->index = i; p->v0 = i == 1 ? (float)ui_persist.fx.time : *fx_val(i); return;
        }
        return;
    }
    if (!p->active) return;
    int dx = x - p->x0, dy = y_screen - p->y0;
    if (dx * dx + dy * dy > 18 * 18) p->moved = 1;
    if (type == UI_MOVE) {
        if (p->target == TG_KNOB) {
            int k = p->index;
            float v = clampf(p->v0 + (dx - dy) / K[k].range, 0.f, 1.f);
            ui_persist.pots[k] = v; hw_pots[k] = v;
            if (k == 0) for (int b = 0; b < 2; b++) if (hw_buttons[b]) edited_in_hold[b] = 1;
        } else if (p->target == TG_FX && p->moved) {
            if (p->index == 1) {
                int t = (int)(p->v0 + (dx - dy) / 70.f + 600.5f) % 6;
                ui_persist.fx.time = t;
            } else *fx_val(p->index) = clampf(p->v0 + (dx - dy) / 280.f, 0.f, 1.f);
            ui_apply();
        } else if (p->target == TG_SCROLL) {
            scroll -= y_screen - p->y;
        } else if (p->target == TG_STRIP && p->index == 3 && p->moved) {
            ui_persist.bpm = clampf(p->v0 + (dx - dy) / 4.f, 30.f, 240.f);
            ui_persist.bpm = (float)(int)(ui_persist.bpm + 0.5f);
            ui_apply();
        } else if (p->target == TG_STRIP && p->moved && p->index != 3) {
            p->target = TG_SCROLL; scroll -= y_screen - p->y0; /* turn it into a scroll */
        } else if (p->target == TG_JACK && p->moved) {
            p->target = TG_SCROLL; scroll -= y_screen - p->y0;
        }
        p->x = x; p->y = y_screen;
        return;
    }
    /* UP */
    if (p->target == TG_BUTTON) hw_buttons[p->index] = 0;
    else if (p->target == TG_KNOB) { if (knob_touch[p->index] > 0) knob_touch[p->index]--; }
    else if (p->target == TG_JACK && !p->moved) {
        if (p->index == J_TRIG) ui_persist.clock_mode = !ui_persist.clock_mode;
        ui_apply();
    } else if (p->target == TG_FX && !p->moved && p->index == 1) {
        ui_persist.fx.time = (ui_persist.fx.time + 1) % 6;
        ui_apply();
    } else if (p->target == TG_STRIP && !p->moved) {
        switch (p->index) {
        case 0: ui_persist.clock_mode = 0; break;
        case 1: ui_persist.clock_mode = 1; break;
        case 2: ui_persist.bpm = clampf(ui_persist.bpm - 1, 30, 240); ui_persist.clock_mode = 1; break;
        case 4: ui_persist.bpm = clampf(ui_persist.bpm + 1, 30, 240); ui_persist.clock_mode = 1; break;
        }
        ui_apply();
    }
    p->active = 0;
}

void ui_cancel_all(void) {
    for (int i = 0; i < 10; i++) {
        if (ptrs[i].active && ptrs[i].target == TG_KNOB && knob_touch[ptrs[i].index] > 0) knob_touch[ptrs[i].index]--;
        ptrs[i].active = 0;
    }
    hw_buttons[0] = hw_buttons[1] = 0;
    for (int k = 0; k < 7; k++) knob_touch[k] = 0;
}

void ui_scroll_to(float y) { scroll = y; scroll_v = 0; last_scroll = y; }
