#!/usr/bin/sh

TARGET=$1

S1_S2_LOC_OFF=480

dd if=/dev/zero of=$TARGET bs=512 count=2880 >/dev/null

STAGE2_SIZE=$(stat -c %s ${BUILD_DIR}/stage2.bin)
STAGE2_SECTORS=$((( (${STAGE2_SIZE} + 511 ) / 512 )))
RESERVED_SECTORS=$(( $STAGE2_SECTORS + 1))

mkfs.fat -F 12 -R $RESERVED_SECTORS -n "NBOS" $TARGET >/dev/null

dd if=${BUILD_DIR}/stage1.bin of=$TARGET conv=notrunc bs=1 count=3 >/dev/null
dd if=${BUILD_DIR}/stage1.bin of=$TARGET conv=notrunc bs=1 seek=62 skip=62 >/dev/null
dd if=${BUILD_DIR}/stage2.bin of=$TARGET conv=notrunc bs=512 seek=1 >/dev/null

echo "01 00 00 00 " | xxd -r -p | dd of=$TARGET conv=notrunc bs=1 seek=$S1_S2_LOC_OFF
printf "%x" $STAGE2_SECTORS | xxd -r -p | dd of=$TARGET conv=notrunc bs=1 seek=$(($S1_S2_LOC_OFF + 4))

mcopy -i $TARGET ${BUILD_DIR}/kernel.bin "::kernel.bin"
mcopy -i $TARGET test.txt "::test.txt"
mmd -i $TARGET "::mydirs"
mcopy -i $TARGET test.txt "::mydirs/test.txt"