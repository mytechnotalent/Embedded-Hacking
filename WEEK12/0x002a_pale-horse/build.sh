#!/usr/bin/env bash
#
# build.sh - assemble, link and stitch the PALE HORSE flash image.
#
# Produces build/pale-horse.bin and build/pale-horse.uf2:
#   0x10000000  stage-2 loader (IMAGE_DEF + vectors + loader)
#   0x10001000  stage-3 application (padded to APP_SIZE)
#   0x1003e000  encrypted ghost blob
#   0x1003f000  manifest sector (word 0 = app CRC32, then the manifest)
#
set -euo pipefail

TC="${PICO_TOOLCHAIN:-$HOME/.pico-sdk/toolchain/14_2_Rel1}/bin"
AS="$TC/arm-none-eabi-as"
LD="$TC/arm-none-eabi-ld"
OBC="$TC/arm-none-eabi-objcopy"
AFLAGS="-mcpu=cortex-m33 -mthumb -I lib"

OUT=build
mkdir -p "$OUT"

echo "[1/5] stage-2 loader"
$AS $AFLAGS stage2/vector_table.s -o "$OUT/stage2_vt.o"
$AS $AFLAGS stage2/image_def.s    -o "$OUT/stage2_id.o"
$AS $AFLAGS stage2/verify.s       -o "$OUT/stage2_verify.o"
$AS $AFLAGS stage2/decrypt.s      -o "$OUT/stage2_decrypt.o"
$AS $AFLAGS stage2/reset.s        -o "$OUT/stage2_reset.o"
$AS $AFLAGS lib/stack.s           -o "$OUT/lib_stack.o"
$AS $AFLAGS lib/coprocessor.s     -o "$OUT/lib_coproc.o"
$LD -T stage2/linker.ld -o "$OUT/stage2.elf" \
  "$OUT/stage2_id.o" "$OUT/stage2_vt.o" "$OUT/stage2_verify.o" \
  "$OUT/stage2_decrypt.o" "$OUT/stage2_reset.o" "$OUT/lib_stack.o" "$OUT/lib_coproc.o"
$OBC -O binary "$OUT/stage2.elf" "$OUT/stage2.bin"

echo "[2/5] stage-3 application"
for m in uart gpio reset stack coprocessor xosc pwm i2c lcd dht11 delay; do
  $AS $AFLAGS "lib/$m.s" -o "$OUT/lib_$m.o"
done
$AS $AFLAGS app/vector_table.s -o "$OUT/app_vt.o"
$AS $AFLAGS app/reset.s        -o "$OUT/app_reset.o"
$AS $AFLAGS app/console.s      -o "$OUT/app_console.o"
$AS $AFLAGS app/commands.s     -o "$OUT/app_commands.o"
$LD -T app/linker.ld -o "$OUT/app.elf" \
  "$OUT/app_vt.o" "$OUT/app_reset.o" "$OUT/app_console.o" "$OUT/app_commands.o" \
  "$OUT/lib_uart.o" "$OUT/lib_gpio.o" "$OUT/lib_reset.o" "$OUT/lib_stack.o" \
  "$OUT/lib_coprocessor.o" "$OUT/lib_xosc.o" "$OUT/lib_pwm.o" "$OUT/lib_i2c.o" "$OUT/lib_lcd.o" \
  "$OUT/lib_dht11.o" "$OUT/lib_delay.o"
$OBC -O binary "$OUT/app.elf" "$OUT/app.bin"

echo "[3/5] ghost stage"
$AS $AFLAGS ghost/auth.s -o "$OUT/ghost.o"
$LD -T ghost/linker.ld -o "$OUT/ghost.elf" "$OUT/ghost.o"
$OBC -O binary "$OUT/ghost.elf" "$OUT/ghost.bin"
python3 scripts/encrypt_ghost.py "$OUT/ghost.bin" "$OUT/ghost_blob.bin"

echo "[4/5] assemble combined image"
python3 scripts/stitch_image.py "$OUT"

echo "[5/5] UF2"
python3 uf2conv.py "$OUT/pale-horse.bin" --base 0x10000000 \
  --family 0xe48bff59 --output "$OUT/pale-horse.uf2"

echo "done: $OUT/pale-horse.bin $OUT/pale-horse.uf2"
