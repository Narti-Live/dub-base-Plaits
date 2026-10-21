/* DUB STAB PLAITS for Plaits (app) — NativeActivity glue: window, touch, AAudio, saved state */
#include "android_min.h"
#include "plat.h"
#include "fw.h"
#include "ui.h"
#include "fx.h"

static ANativeActivity *g_act;
static ANativeWindow *g_win;
static AInputQueue *g_queue;
static ALooper *g_looper;
static int g_timer = -1;
static int g_winW = 1, g_winH = 1, g_bufH = 1;
static uint32_t *g_canvas;
static AAudioStream *g_stream;
static volatile int g_audioRestart;
static volatile float g_sr = 48000.f;

/* ---------------------------------------------------------------- audio */
static int32_t audio_cb(AAudioStream *s, void *u, void *data, int32_t n) {
    (void)s; (void)u;
#if defined(__aarch64__)
    uint64_t fpcr; __asm__ volatile("mrs %0, fpcr" : "=r"(fpcr)); __asm__ volatile("msr fpcr, %0" ::"r"(fpcr | (1u << 24)));
#elif defined(__x86_64__)
    unsigned int csr; __asm__ volatile("stmxcsr %0" : "=m"(csr)); csr |= 0x8040; __asm__ volatile("ldmxcsr %0" ::"m"(csr));
#endif
    fw_render((float *)data, n, g_sr);
    fx_process((float *)data, n, ui_persist.clock_mode ? ui_persist.bpm : 120.f);
    return 0;
}
static void audio_err(AAudioStream *s, void *u, aaudio_result_t e) { (void)s; (void)u; (void)e; g_audioRestart = 1; }
static void audio_stop(void) {
    if (g_stream) { AAudioStream_requestStop(g_stream); AAudioStream_close(g_stream); g_stream = 0; }
}
static void audio_start(void) {
    AAudioStreamBuilder *b;
    if (AAudio_createStreamBuilder(&b) != 0) return;
    AAudioStreamBuilder_setDirection(b, AAUDIO_DIRECTION_OUTPUT);
    AAudioStreamBuilder_setFormat(b, AAUDIO_FORMAT_PCM_FLOAT);
    AAudioStreamBuilder_setChannelCount(b, 2);
    AAudioStreamBuilder_setSampleRate(b, 48000);   /* the firmware's rate: 4000 blocks of 12 per second */
    AAudioStreamBuilder_setSharingMode(b, AAUDIO_SHARING_MODE_SHARED);
    AAudioStreamBuilder_setPerformanceMode(b, AAUDIO_PERFORMANCE_MODE_LOW_LATENCY);
    AAudioStreamBuilder_setDataCallback(b, audio_cb, 0);
    AAudioStreamBuilder_setErrorCallback(b, audio_err, 0);
    AAudioStream *s = 0;
    if (AAudioStreamBuilder_openStream(b, &s) == 0 && s) {
        int sr = AAudioStream_getSampleRate(s);
        g_sr = sr >= 8000 ? (float)sr : 48000.f;
        int burst = AAudioStream_getFramesPerBurst(s);
        if (burst > 0) AAudioStream_setBufferSizeInFrames(s, burst * 4);
        g_stream = s;
        AAudioStream_requestStart(s);
    }
    AAudioStreamBuilder_delete(b);
}

/* ---------------------------------------------------------------- saved state
   One file: the app's knob positions etc. + the firmware's two flash chunks
   (calibration + DUB STAB PLAITS state), exactly as Settings would store them. */
#define MAGIC 0x31505544u  /* 'DUP1' */
#define BLOB 256
static uint8_t g_pers[BLOB], g_state[BLOB];
static int g_pers_n, g_state_n;
/* queued by the audio thread (no file I/O there) */
static uint8_t q_state[BLOB], q_pers[BLOB];
static volatile int q_state_n, q_pers_n;
static volatile uint32_t q_seq;
static volatile int g_dirty;

int app_storage_load(void *p, int np, void *s, int ns) {
    int ok = 0;
    if (g_pers_n == np && np <= BLOB) { memcpy(p, g_pers, np); ok = 1; }
    if (g_state_n == ns && ns <= BLOB) { memcpy(s, g_state, ns); ok = 1; }
    return ok;
}
void app_storage_save(const void *p, int np, const void *s, int ns) {
    q_seq = q_seq + 1;
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    if (p && np <= BLOB) { memcpy(q_pers, p, np); q_pers_n = np; }
    if (s && ns <= BLOB) { memcpy(q_state, s, ns); q_state_n = ns; }
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    q_seq = q_seq + 1;
    g_dirty = 1;
}
static void take_queue(void) {
    for (int t = 0; t < 4; t++) {
        uint32_t a = q_seq;
        if (a & 1) continue;
        __atomic_thread_fence(__ATOMIC_SEQ_CST);
        uint8_t ps[BLOB], ss[BLOB]; int pn = q_pers_n, sn = q_state_n;
        memcpy(ps, q_pers, BLOB); memcpy(ss, q_state, BLOB);
        __atomic_thread_fence(__ATOMIC_SEQ_CST);
        if (q_seq != a) continue;
        if (pn) { memcpy(g_pers, ps, pn); g_pers_n = pn; }
        if (sn) { memcpy(g_state, ss, sn); g_state_n = sn; }
        return;
    }
}
static void state_path(char *out) {
    const char *d = g_act && g_act->internalDataPath ? g_act->internalDataPath : "/data/local/tmp";
    int i = 0; while (d[i] && i < 400) { out[i] = d[i]; i++; }
    const char *f = "/dubplaits.state"; int j = 0; while (f[j]) out[i++] = f[j++];
    out[i] = 0;
}
static void state_save(void) {
    if (g_dirty) { g_dirty = 0; take_queue(); }
    char p[512]; state_path(p);
    int fd = open(p, O_WRONLY_ | O_CREAT_ | O_TRUNC_, 0600);
    if (fd < 0) return;
    uint32_t hdr[4] = {MAGIC, (uint32_t)sizeof(UiPersist), (uint32_t)g_pers_n, (uint32_t)g_state_n};
    write(fd, hdr, sizeof hdr);
    write(fd, &ui_persist, sizeof ui_persist);
    if (g_pers_n) write(fd, g_pers, g_pers_n);
    if (g_state_n) write(fd, g_state, g_state_n);
    close(fd);
}
static void state_load(void) {
    char p[512]; state_path(p);
    int fd = open(p, O_RDONLY_);
    if (fd < 0) return;
    uint32_t hdr[4];
    if (read(fd, hdr, sizeof hdr) == sizeof hdr && hdr[0] == MAGIC && hdr[1] == sizeof(UiPersist) && hdr[2] <= BLOB && hdr[3] <= BLOB) {
        UiPersist u;
        if (read(fd, &u, sizeof u) == sizeof u) {
            int ok = u.bpm >= 30.f && u.bpm <= 240.f && u.listen >= 0 && u.listen <= 2;
            for (int i = 0; i < 7; i++) if (!(u.pots[i] >= 0.f && u.pots[i] <= 1.f)) ok = 0;
            if (!(u.fx.echo >= 0.f && u.fx.echo <= 1.f && u.fx.repeats >= 0.f && u.fx.repeats <= 1.f &&
                  u.fx.reverb >= 0.f && u.fx.reverb <= 1.f && u.fx.wow >= 0.f && u.fx.wow <= 1.f && u.fx.time >= 0 && u.fx.time < 6)) ok = 0;
            if (ok) ui_persist = u;
            if (hdr[2] && read(fd, g_pers, hdr[2]) == (int)hdr[2]) g_pers_n = hdr[2];
            if (hdr[3] && read(fd, g_state, hdr[3]) == (int)hdr[3]) g_state_n = hdr[3];
        }
    }
    close(fd);
}
static uint32_t random_seed(void) {
    uint32_t s = 0x2545f491u;
    int fd = open("/dev/urandom", O_RDONLY_);
    if (fd >= 0) { read(fd, &s, sizeof s); close(fd); }
    return s;
}

/* ---------------------------------------------------------------- drawing */
static void draw(void) {
    if (!g_win || !g_canvas) return;
    ui_render(g_canvas, CW, g_bufH);
    ANativeWindow_Buffer buf;
    if (ANativeWindow_lock(g_win, &buf, 0) != 0) return;
    int h = buf.height < g_bufH ? buf.height : g_bufH;
    if (buf.width >= CW)
        for (int y = 0; y < h; y++) memcpy((uint32_t *)buf.bits + (size_t)y * buf.stride, g_canvas + (size_t)y * CW, CW * 4);
    ANativeWindow_unlockAndPost(g_win);
}
static void setup_window(ANativeWindow *w) {
    g_win = w;
    int ww = ANativeWindow_getWidth(w), wh = ANativeWindow_getHeight(w);
    if (ww > 0 && wh > 0 && !(ww == CW && wh == g_bufH)) { g_winW = ww; g_winH = wh; }
    g_bufH = (int)((float)g_winH * CW / g_winW + 0.5f);
    if (g_bufH < 800) g_bufH = 800;
    if (g_bufH > 2400) g_bufH = 2400;
    if (g_canvas) free(g_canvas);
    g_canvas = (uint32_t *)malloc((size_t)CW * g_bufH * 4);
    ANativeWindow_setBuffersGeometry(w, CW, g_bufH, WINDOW_FORMAT_RGBX_8888);
    draw();
}

/* ---------------------------------------------------------------- input */
static int on_input(int fd, int events, void *data) {
    (void)fd; (void)events; (void)data;
    AInputEvent *ev = 0;
    while (g_queue && AInputQueue_getEvent(g_queue, &ev) >= 0) {
        if (AInputQueue_preDispatchEvent(g_queue, ev)) continue;
        int handled = 0;
        if (AInputEvent_getType(ev) == AINPUT_EVENT_TYPE_MOTION) {
            int32_t a = AMotionEvent_getAction(ev);
            int act = a & AMOTION_EVENT_ACTION_MASK;
            int idx = (a >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT) & 0xff;
            float sx = (float)CW / g_winW, sy = (float)g_bufH / g_winH;
            #define PX_(i) ((int)(AMotionEvent_getX(ev, i) * sx))
            #define PY_(i) ((int)(AMotionEvent_getY(ev, i) * sy))
            if (act == AMOTION_EVENT_ACTION_DOWN || act == AMOTION_EVENT_ACTION_POINTER_DOWN)
                ui_touch(UI_DOWN, AMotionEvent_getPointerId(ev, idx), PX_(idx), PY_(idx));
            else if (act == AMOTION_EVENT_ACTION_MOVE) {
                size_t n = AMotionEvent_getPointerCount(ev);
                for (size_t i = 0; i < n; i++) ui_touch(UI_MOVE, AMotionEvent_getPointerId(ev, i), PX_(i), PY_(i));
            } else if (act == AMOTION_EVENT_ACTION_UP || act == AMOTION_EVENT_ACTION_POINTER_UP)
                ui_touch(UI_UP, AMotionEvent_getPointerId(ev, idx), PX_(idx), PY_(idx));
            else if (act == AMOTION_EVENT_ACTION_CANCEL) ui_cancel_all();
            handled = 1;
        }
        AInputQueue_finishEvent(g_queue, ev, handled);
    }
    return 1;
}

static int on_timer(int fd, int events, void *data) {
    (void)events; (void)data;
    uint64_t x; read(fd, &x, sizeof x);
    static int ticks;
    if (g_audioRestart) { g_audioRestart = 0; audio_stop(); audio_start(); }
    if (g_dirty && ++ticks > 30) { ticks = 0; state_save(); }
    draw();
    return 1;
}

/* ---------------------------------------------------------------- lifecycle */
static void onStart(ANativeActivity *a) { (void)a; if (!g_stream) audio_start(); }
static void onResume(ANativeActivity *a) { (void)a; if (!g_stream) audio_start(); }
static void *onSave(ANativeActivity *a, size_t *n) { (void)a; *n = 0; return 0; }
static void onPause(ANativeActivity *a) { (void)a; ui_cancel_all(); state_save(); }
static void onStop(ANativeActivity *a) { (void)a; audio_stop(); }
static void onDestroy(ANativeActivity *a) {
    (void)a; state_save(); audio_stop();
    if (g_timer >= 0 && g_looper) { ALooper_removeFd(g_looper, g_timer); close(g_timer); g_timer = -1; }
}
static void onFocus(ANativeActivity *a, int f) { (void)a; if (!f) ui_cancel_all(); }
static void onWinCreated(ANativeActivity *a, ANativeWindow *w) { (void)a; setup_window(w); }
static void onWinResized(ANativeActivity *a, ANativeWindow *w) { (void)a; setup_window(w); }
static void onWinRedraw(ANativeActivity *a, ANativeWindow *w) { (void)a; (void)w; draw(); }
static void onWinDestroyed(ANativeActivity *a, ANativeWindow *w) { (void)a; (void)w; g_win = 0; }
static void onQueueCreated(ANativeActivity *a, AInputQueue *q) {
    (void)a; g_queue = q;
    AInputQueue_attachLooper(q, g_looper, 1, on_input, 0);
}
static void onQueueDestroyed(ANativeActivity *a, AInputQueue *q) { (void)a; AInputQueue_detachLooper(q); g_queue = 0; }
static void onContentRect(ANativeActivity *a, const ARect *r) {
    (void)a;
    int w = r->right - r->left, h = r->bottom - r->top;
    if (w > 0 && h > 0 && (w != g_winW || h != g_winH)) { g_winW = w; g_winH = h; }
}
static void onConfig(ANativeActivity *a) { (void)a; }
static void onLowMem(ANativeActivity *a) { (void)a; }

__attribute__((visibility("default")))
void ANativeActivity_onCreate(ANativeActivity *a, void *savedState, size_t savedSize) {
    (void)savedState; (void)savedSize;
    g_act = a;
    ANativeActivityCallbacks *c = a->callbacks;
    c->onStart = onStart; c->onResume = onResume; c->onSaveInstanceState = onSave;
    c->onPause = onPause; c->onStop = onStop; c->onDestroy = onDestroy;
    c->onWindowFocusChanged = onFocus;
    c->onNativeWindowCreated = onWinCreated; c->onNativeWindowResized = onWinResized;
    c->onNativeWindowRedrawNeeded = onWinRedraw; c->onNativeWindowDestroyed = onWinDestroyed;
    c->onInputQueueCreated = onQueueCreated; c->onInputQueueDestroyed = onQueueDestroyed;
    c->onContentRectChanged = onContentRect; c->onConfigurationChanged = onConfig; c->onLowMemory = onLowMem;
    ANativeActivity_setWindowFlags(a, AWINDOW_FLAG_KEEP_SCREEN_ON | AWINDOW_FLAG_FULLSCREEN, 0);

    static int inited;
    if (!inited) {
        ui_defaults();
        state_load();
        ui_apply();                  /* knob positions are in place before the firmware boots */
        fw_init(random_seed());
        inited = 1;
    }
    g_looper = ALooper_forThread();
    if (g_timer < 0) {
        g_timer = timerfd_create(CLOCK_MONOTONIC_, TFD_NONBLOCK_ | TFD_CLOEXEC_);
        struct itimerspec_ ts = {{0, 1000000000L / 30}, {0, 1000000000L / 30}};
        timerfd_settime(g_timer, 0, &ts, 0);
        ALooper_addFd(g_looper, g_timer, 2, ALOOPER_EVENT_INPUT, on_timer, 0);
    }
    if (!g_stream) audio_start();
}
