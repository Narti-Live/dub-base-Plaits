#!/bin/bash
# Compiles the DUB STAB PLAITS firmware core (unchanged ui.cc / voice.cc / dub.cc /
# Plaits engines + the app shims) into a static archive for one target.
# usage: build_core.sh <target-triple|host> <out-dir>
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
FW=${FW:-$ROOT/../eurorack}
T=$1; OUT=$2; mkdir -p $OUT/obj
case $T in
  host) ARCH="";;
  aarch64*) ARCH="--target=$T -mno-outline-atomics -fPIC";;
  *) ARCH="--target=$T -fPIC";;
esac
FLAGS="$ARCH -O2 -DTEST -ffreestanding -fno-builtin-printf -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit \
 -fno-math-errno -fno-strict-aliasing -fno-stack-protector -fvisibility=hidden -fshort-enums \
 -nostdlibinc -isystem $ROOT/rt -isystem $(clang -print-resource-dir)/include \
 -I$ROOT/shim -I$FW $EXTRA_DEFS -include cstdio -std=gnu++11 -w"
SRCS="$FW/plaits/ui.cc $FW/plaits/resources.cc $(ls $FW/plaits/dsp/*.cc $FW/plaits/dsp/*/*.cc $FW/plaits/dub/*.cc) \
 $FW/stmlib/dsp/atan.cc $FW/stmlib/dsp/units.cc $FW/stmlib/utils/random.cc $ROOT/src/settings_app.cc $ROOT/src/fw.cc"
objs=""; pids=()
for s in $SRCS; do
  o=$OUT/obj/$(echo ${s#$FW/} | tr / _).o; objs="$objs $o"
  if [ ! -f $o ] || [ $s -nt $o ] || [ -n "$FORCE" ] || [ $ROOT/src/fw.h -nt $o ]; then
    clang++ $FLAGS -c $s -o $o & pids+=($!)
  fi
done
fail=0; for p in "${pids[@]}"; do wait $p || fail=1; done
[ $fail = 0 ] || { echo "compile failed"; exit 1; }
rm -f $OUT/libfwcore.a
llvm-ar rcs $OUT/libfwcore.a $objs 2>/dev/null || ar rcs $OUT/libfwcore.a $objs
echo "built $OUT/libfwcore.a"
