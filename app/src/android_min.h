/* Minimal hand-written declarations of the Android NDK C APIs used by Dub Stab Plaits.
   Layouts match the NDK headers (android/native_activity.h, native_window.h,
   input.h, looper.h, aaudio/AAudio.h). */
#ifndef ANDROID_MIN_H
#define ANDROID_MIN_H
#include <stdint.h>
#include <stddef.h>

typedef struct ANativeWindow ANativeWindow;
typedef struct AInputQueue AInputQueue;
typedef struct AInputEvent AInputEvent;
typedef struct ALooper ALooper;
typedef struct AAssetManager AAssetManager;
typedef struct ARect { int32_t left, top, right, bottom; } ARect;

struct ANativeActivity;
typedef struct ANativeActivityCallbacks {
    void (*onStart)(struct ANativeActivity *);
    void (*onResume)(struct ANativeActivity *);
    void *(*onSaveInstanceState)(struct ANativeActivity *, size_t *outSize);
    void (*onPause)(struct ANativeActivity *);
    void (*onStop)(struct ANativeActivity *);
    void (*onDestroy)(struct ANativeActivity *);
    void (*onWindowFocusChanged)(struct ANativeActivity *, int hasFocus);
    void (*onNativeWindowCreated)(struct ANativeActivity *, ANativeWindow *);
    void (*onNativeWindowResized)(struct ANativeActivity *, ANativeWindow *);
    void (*onNativeWindowRedrawNeeded)(struct ANativeActivity *, ANativeWindow *);
    void (*onNativeWindowDestroyed)(struct ANativeActivity *, ANativeWindow *);
    void (*onInputQueueCreated)(struct ANativeActivity *, AInputQueue *);
    void (*onInputQueueDestroyed)(struct ANativeActivity *, AInputQueue *);
    void (*onContentRectChanged)(struct ANativeActivity *, const ARect *);
    void (*onConfigurationChanged)(struct ANativeActivity *);
    void (*onLowMemory)(struct ANativeActivity *);
} ANativeActivityCallbacks;

typedef struct ANativeActivity {
    ANativeActivityCallbacks *callbacks;
    void *vm;
    void *env;
    void *clazz;
    const char *internalDataPath;
    const char *externalDataPath;
    int32_t sdkVersion;
    void *instance;
    AAssetManager *assetManager;
    const char *obbPath;
} ANativeActivity;

void ANativeActivity_setWindowFlags(ANativeActivity *, uint32_t addFlags, uint32_t removeFlags);
#define AWINDOW_FLAG_KEEP_SCREEN_ON 0x00000080
#define AWINDOW_FLAG_FULLSCREEN 0x00000400

typedef struct ANativeWindow_Buffer {
    int32_t width, height, stride, format;
    void *bits;
    uint32_t reserved[6];
} ANativeWindow_Buffer;
#define WINDOW_FORMAT_RGBX_8888 2
int32_t ANativeWindow_setBuffersGeometry(ANativeWindow *, int32_t w, int32_t h, int32_t format);
int32_t ANativeWindow_getWidth(ANativeWindow *);
int32_t ANativeWindow_getHeight(ANativeWindow *);
int32_t ANativeWindow_lock(ANativeWindow *, ANativeWindow_Buffer *outBuffer, ARect *inOutDirtyBounds);
int32_t ANativeWindow_unlockAndPost(ANativeWindow *);

typedef int (*ALooper_callbackFunc)(int fd, int events, void *data);
ALooper *ALooper_forThread(void);
int ALooper_addFd(ALooper *, int fd, int ident, int events, ALooper_callbackFunc cb, void *data);
int ALooper_removeFd(ALooper *, int fd);
#define ALOOPER_EVENT_INPUT 1

void AInputQueue_attachLooper(AInputQueue *, ALooper *, int ident, ALooper_callbackFunc cb, void *data);
void AInputQueue_detachLooper(AInputQueue *);
int32_t AInputQueue_getEvent(AInputQueue *, AInputEvent **outEvent);
int32_t AInputQueue_preDispatchEvent(AInputQueue *, AInputEvent *);
void AInputQueue_finishEvent(AInputQueue *, AInputEvent *, int handled);
int32_t AInputEvent_getType(const AInputEvent *);
int32_t AMotionEvent_getAction(const AInputEvent *);
size_t AMotionEvent_getPointerCount(const AInputEvent *);
int32_t AMotionEvent_getPointerId(const AInputEvent *, size_t idx);
float AMotionEvent_getX(const AInputEvent *, size_t idx);
float AMotionEvent_getY(const AInputEvent *, size_t idx);
int32_t AKeyEvent_getKeyCode(const AInputEvent *);
int32_t AKeyEvent_getAction(const AInputEvent *);
#define AINPUT_EVENT_TYPE_KEY 1
#define AINPUT_EVENT_TYPE_MOTION 2
#define AMOTION_EVENT_ACTION_MASK 0xff
#define AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT 8
#define AMOTION_EVENT_ACTION_DOWN 0
#define AMOTION_EVENT_ACTION_UP 1
#define AMOTION_EVENT_ACTION_MOVE 2
#define AMOTION_EVENT_ACTION_CANCEL 3
#define AMOTION_EVENT_ACTION_POINTER_DOWN 5
#define AMOTION_EVENT_ACTION_POINTER_UP 6

/* AAudio */
typedef struct AAudioStreamStruct AAudioStream;
typedef struct AAudioStreamBuilderStruct AAudioStreamBuilder;
typedef int32_t aaudio_result_t;
typedef int32_t (*AAudioStream_dataCallback)(AAudioStream *, void *user, void *audioData, int32_t numFrames);
typedef void (*AAudioStream_errorCallback)(AAudioStream *, void *user, aaudio_result_t error);
aaudio_result_t AAudio_createStreamBuilder(AAudioStreamBuilder **);
void AAudioStreamBuilder_setDirection(AAudioStreamBuilder *, int32_t);
void AAudioStreamBuilder_setFormat(AAudioStreamBuilder *, int32_t);
void AAudioStreamBuilder_setChannelCount(AAudioStreamBuilder *, int32_t);
void AAudioStreamBuilder_setSampleRate(AAudioStreamBuilder *, int32_t);
void AAudioStreamBuilder_setSharingMode(AAudioStreamBuilder *, int32_t);
void AAudioStreamBuilder_setPerformanceMode(AAudioStreamBuilder *, int32_t);
void AAudioStreamBuilder_setDataCallback(AAudioStreamBuilder *, AAudioStream_dataCallback, void *);
void AAudioStreamBuilder_setErrorCallback(AAudioStreamBuilder *, AAudioStream_errorCallback, void *);
aaudio_result_t AAudioStreamBuilder_openStream(AAudioStreamBuilder *, AAudioStream **);
aaudio_result_t AAudioStreamBuilder_delete(AAudioStreamBuilder *);
aaudio_result_t AAudioStream_close(AAudioStream *);
aaudio_result_t AAudioStream_requestStart(AAudioStream *);
aaudio_result_t AAudioStream_requestStop(AAudioStream *);
int32_t AAudioStream_getSampleRate(AAudioStream *);
int32_t AAudioStream_getFramesPerBurst(AAudioStream *);
aaudio_result_t AAudioStream_setBufferSizeInFrames(AAudioStream *, int32_t);
#define AAUDIO_DIRECTION_OUTPUT 0
#define AAUDIO_FORMAT_PCM_FLOAT 2
#define AAUDIO_SHARING_MODE_SHARED 1
#define AAUDIO_PERFORMANCE_MODE_LOW_LATENCY 12

/* libc */
struct timespec_ { long tv_sec, tv_nsec; };
struct itimerspec_ { struct timespec_ it_interval, it_value; };
int timerfd_create(int clockid, int flags);
int timerfd_settime(int fd, int flags, const struct itimerspec_ *nv, struct itimerspec_ *old);
long read(int fd, void *buf, size_t n);
long write(int fd, const void *buf, size_t n);
int open(const char *path, int flags, ...);
int close(int fd);
#define CLOCK_MONOTONIC_ 1
#define TFD_NONBLOCK_ 04000
#define TFD_CLOEXEC_ 02000000
#define O_RDONLY_ 0
#define O_WRONLY_ 1
#define O_CREAT_ 0100
#define O_TRUNC_ 01000

#endif
