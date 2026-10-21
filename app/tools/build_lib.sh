#!/bin/bash
# Builds libdubplaits.so (arm64-v8a, x86_64): firmware core (C++) + app (C), stub NDK libs.
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
V=${VARIANT:-c}; OUT=$ROOT/build/lib-$V; STUB=$ROOT/build/stubs
declare -A LIBS
LIBS[android]="ANativeActivity_setWindowFlags ANativeWindow_setBuffersGeometry ANativeWindow_getWidth ANativeWindow_getHeight ANativeWindow_lock ANativeWindow_unlockAndPost ALooper_forThread ALooper_addFd ALooper_removeFd AInputQueue_attachLooper AInputQueue_detachLooper AInputQueue_getEvent AInputQueue_preDispatchEvent AInputQueue_finishEvent AInputEvent_getType AMotionEvent_getAction AMotionEvent_getPointerCount AMotionEvent_getPointerId AMotionEvent_getX AMotionEvent_getY AKeyEvent_getKeyCode AKeyEvent_getAction"
LIBS[aaudio]="AAudio_createStreamBuilder AAudioStreamBuilder_setDirection AAudioStreamBuilder_setFormat AAudioStreamBuilder_setChannelCount AAudioStreamBuilder_setSampleRate AAudioStreamBuilder_setSharingMode AAudioStreamBuilder_setPerformanceMode AAudioStreamBuilder_setDataCallback AAudioStreamBuilder_setErrorCallback AAudioStreamBuilder_openStream AAudioStreamBuilder_delete AAudioStream_close AAudioStream_requestStart AAudioStream_requestStop AAudioStream_getSampleRate AAudioStream_getFramesPerBurst AAudioStream_setBufferSizeInFrames"
LIBS[m]="sinf cosf tanf expf powf logf log2f exp2f atanf tanhf sqrtf floorf ceilf roundf fmodf"
LIBS[c]="memset memcpy memmove memcmp strlen malloc free abort timerfd_create timerfd_settime read write open close"
for ABI in arm64-v8a x86_64; do
  case $ABI in arm64-v8a) T=aarch64-linux-android26; X="-mno-outline-atomics";; x86_64) T=x86_64-linux-android26; X="";; esac
  mkdir -p $STUB/$ABI $OUT/$ABI
  for L in android aaudio m c; do
    : > $STUB/$ABI/lib$L.c
    for f in ${LIBS[$L]}; do echo "void $f(void){}" >> $STUB/$ABI/lib$L.c; done
    clang --target=$T -fPIC -shared -nostdlib -fuse-ld=lld -Wl,-soname,lib$L.so -fno-builtin -w -o $STUB/$ABI/lib$L.so $STUB/$ABI/lib$L.c
  done
  $ROOT/tools/build_core.sh $T $ROOT/build/core-$V-$ABI > /dev/null
  clang --target=$T $X -O2 -fPIC -ffreestanding -fno-stack-protector -fno-math-errno -fvisibility=hidden \
    -Wall -Wno-unused-function -nostdlib -shared -fuse-ld=lld \
    -Wl,-soname,libdubplaits.so -Wl,--no-undefined -Wl,-z,max-page-size=16384 -Wl,-z,noexecstack -Wl,--hash-style=both -Wl,--build-id=sha1 -Wl,--gc-sections \
    -o $OUT/$ABI/libdubplaits.so $ROOT/src/main.c $ROOT/src/ui.c $ROOT/src/gfx.c $ROOT/src/fx.c \
    $ROOT/build/core-$V-$ABI/libfwcore.a -L$STUB/$ABI -landroid -laaudio -lm -lc
  llvm-strip --strip-unneeded $OUT/$ABI/libdubplaits.so 2>/dev/null || true
done
ls -la $OUT/*/
