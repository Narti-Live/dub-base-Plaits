#!/bin/bash
# Builds the Plaits application firmware with clang/lld (no arm-none-eabi-gcc needed).
# usage: build_fw.sh <source-root> <out-dir>
set -e
SRC=$(cd "$1" && pwd); OUT=$2; mkdir -p $OUT/obj $OUT/crt
RT=$(cd "$(dirname "$0")/rt" && pwd)
CRT=$(cd "$(dirname "$0")/llvm/compiler-rt/lib/builtins" && pwd)
ARCH="--target=thumbv7em-none-eabihf -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb"
DEFS="-DGCC_ARMCM4 -DSTM32F37X -DARM_MATH_CM4 -D__FPU_PRESENT=1 -DAPPLICATION -DF_CPU=72000000L -DF_CRYSTAL=8000000L -DHSE_VALUE=8000000L -DUSE_STDPERIPH_DRIVER -DSYSCLK_FREQ_72MHz=72000000L $EXTRA_DEFS"
STM=$SRC/stmlib/third_party/STM
INC="-I$SRC -I$STM -I$STM/CMSIS/CM3_f37x -I$STM/CMSIS/CM3_f37x/startup/gcc -I$STM/STM32F37x_StdPeriph_Driver/inc"
COMMON="$ARCH -g0 -fdata-sections -ffunction-sections -fshort-enums -fno-unwind-tables -fno-asynchronous-unwind-tables -fno-math-errno -fno-strict-aliasing -nostdlibinc -isystem $RT/include -isystem $(clang -print-resource-dir)/include $DEFS $INC -Wno-unused-command-line-argument"
CXXFLAGS="$COMMON -std=gnu++11 -fno-exceptions -fno-rtti -fno-threadsafe-statics -fno-use-cxa-atexit -Wall -Wno-unused-private-field -Wno-unused-local-typedef -Wno-deprecated-register -Wno-unused-const-variable -Wno-unused-function -Wno-unused-variable -Wno-missing-braces -Wno-mismatched-tags -Wno-unused-but-set-variable"
CFLAGS="$COMMON -std=gnu99 -w"

# compiler-rt builtins (built once)
if [ ! -f $OUT/crt/libbuiltins.a ]; then
  objs=""
  for f in $CRT/*.c $CRT/arm/*.S; do
    b=$(basename $f); case $b in *apple*|*darwin*|atomic*|emutls*|enable_execute_stack*|clear_cache*|gcc_personality*|os_version*|cpu_model*|trampoline*|*tf*|*xf*|*ti*|*vfp*|*sync*|chkstk*) continue;; esac
    pre=""; case $f in */arm/*) pre="arm_";; esac
    clang $ARCH -O2 -w -ffreestanding -nostdlibinc -isystem $(clang -print-resource-dir)/include -I$CRT -c $f -o $OUT/crt/$pre${b%.*}.o 2>/dev/null && objs="$objs $OUT/crt/$pre${b%.*}.o" || true
  done
  llvm-ar rcs $OUT/crt/libbuiltins.a $objs 2>/dev/null || ar rcs $OUT/crt/libbuiltins.a $objs
fi

PKGS="plaits plaits/drivers plaits/dsp plaits/dsp/fm plaits/dsp/fx plaits/dsp/chords plaits/dsp/drum_modelling plaits/dsp/engine plaits/dsp/engine2 plaits/dsp/physical_modelling plaits/dsp/speech plaits/dub stm_audio_bootloader/fsk stmlib/dsp stmlib/utils stmlib/system"
objs=""
pids=()
# newest header: a changed header rebuilds everything (struct layouts must match)
NEWEST_H=$(find $SRC/plaits $SRC/stmlib -name '*.h' -printf '%T@ %p\n' | sort -n | tail -1 | cut -d' ' -f2)
compile() { # src obj flags...
  local s=$1 o=$2; shift 2
  if [ ! -f $o ] || [ $s -nt $o ] || [ $NEWEST_H -nt $o ] || [ -n "$FORCE" ]; then "$@" -c $s -o $o || { echo "FAILED: $s"; return 1; }; fi
}
# Hot DSP code gets -O2 (with loop unrolling, like the original gcc build);
# everything else is optimised for size so the image fits in 224K.
optfor() { case $1 in plaits/dsp*|stmlib/dsp*|plaits/dub*) echo "${OPT_DSP:--O2 -fno-unroll-loops}";; *) echo "${OPT_OTHER:--Os}";; esac; }
# Engines whose inner loops lose a lot without unrolling (measured in the emulator).
UNROLL="additive_engine.cc modal_engine.cc modal_voice.cc resonator.cc string_engine.cc string_voice.cc string.cc chord_engine.cc string_machine_engine.cc wavetable_engine.cc"
optfile() { local b=$(basename $1); for u in $UNROLL; do [ "$b" = "$u" ] && { echo "-O2"; return; }; done; optfor $2; }
for p in $PKGS; do
  [ -d $SRC/$p ] || continue
  for f in $SRC/$p/*.cc; do [ -f "$f" ] || continue; O=$(optfile $f $p); o=$OUT/obj/$(echo ${f#$SRC/} | tr / _).o; objs="$objs $o"; compile $f $o clang++ $O $CXXFLAGS & pids+=($!); done
  for f in $SRC/$p/*.c; do [ -f "$f" ] || continue; O=$(optfor $p); o=$OUT/obj/$(echo ${f#$SRC/} | tr / _).o; objs="$objs $o"; compile $f $o clang $O $CFLAGS & pids+=($!); done
done
for f in $SRC/plaits/resources.cc; do :; done
for f in $STM/CMSIS/CM3_f37x/system_stm32f37x.c $STM/STM32F37x_StdPeriph_Driver/src/*.c $RT/src/rt.c; do
  o=$OUT/obj/$(basename $f).o; objs="$objs $o"; compile $f $o clang -Os $CFLAGS & pids+=($!)
done
o=$OUT/obj/startup.o; objs="$objs $o"; compile $STM/CMSIS/CM3_f37x/startup/gcc/startup_stm32f37x.s $o clang $ARCH -x assembler-with-cpp & pids+=($!)
fail=0; for p in "${pids[@]}"; do wait $p || fail=1; done
[ $fail = 0 ] || { echo "compile failed"; exit 1; }
# drop duplicates
objs=$(echo $objs | tr ' ' '\n' | awk '!s[$0]++' | tr '\n' ' ')
sed -e '/DISCARD/,/}/d' $SRC/stmlib/linker_scripts/stm32f373x_flash_application_large.ld > $OUT/link.ld
ld.lld -T $OUT/link.ld --gc-sections -Map=$OUT/plaits.map -o $OUT/plaits.elf $objs $OUT/crt/libbuiltins.a
llvm-objcopy -O binary -R ._user_heap_stack -R .bss $OUT/plaits.elf $OUT/plaits.bin
llvm-size $OUT/plaits.elf 2>/dev/null || size $OUT/plaits.elf
ls -la $OUT/plaits.bin
