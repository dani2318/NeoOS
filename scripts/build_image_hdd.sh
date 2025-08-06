#!/usr/bin/sh


TARGET=$1
SIZE=$2

S1_S2_LOC_OFF=480
DISK_P1_BEGIN=2048
DISK_SECTOR_COUNT=$((($SIZE + 511) / 512))
DISK_P1_END=$(($DISK_SECTOR_COUNT - 1))

echo "Generating disk image ${TARGET} (${DISK_SECTOR_COUNT} sectors)..."
dd if=/dev/zero of=$TARGET bs=512 count=${DISK_SECTOR_COUNT} >/dev/null

chown "$(whoami)" "$BUILD_DIR"

# Create partition table
DISK_SECTOR_END=$(($DISK_SECTOR_COUNT - 1))
parted -s $TARGET mklabel msdos
parted -s $TARGET mkpart primary 2048s ${DISK_SECTOR_END}s
parted -s $TARGET set 1 boot on

STAGE2_SIZE=$(stat -c %s ${BUILD_DIR}/stage2.bin)
STAGE2_SECTORS=$((( (${STAGE2_SIZE} + 511 ) / 512 )))

if [ ${STAGE2_SECTORS} \> ${DISK_PART1_BEGIN} ]; then
    echo "Stage2 too big!!!"
    exit 2
fi
dd if=${BUILD_DIR}/stage2.bin of=$TARGET conv=notrunc bs=512 seek=1 >/dev/null

# Attach loopback
DEVICE=$(losetup -fP --show $TARGET)
TARGET_PARTITON="${DEVICE}p1"

mkfs.fat -n "NBOS" $TARGET_PARTITON >/dev/null

dd if=${BUILD_DIR}/stage1.bin of=$TARGET_PARTITON conv=notrunc bs=1 count=3 >/dev/null
dd if=${BUILD_DIR}/stage1.bin of=$TARGET_PARTITON conv=notrunc bs=1 seek=90 skip=90 >/dev/null

echo "01 00 00 00 " | xxd -r -p | dd of=$TARGET_PARTITON conv=notrunc bs=1 seek=$S1_S2_LOC_OFF
printf "%x" $STAGE2_SECTORS | xxd -r -p | dd of=$TARGET_PARTITON conv=notrunc bs=1 seek=$(($S1_S2_LOC_OFF + 4))

mcopy -i $TARGET_PARTITON ${BUILD_DIR}/kernel.bin "::kernel.bin"
mcopy -i $TARGET_PARTITON test.txt "::test.txt"
mmd -i $TARGET_PARTITON "::mydirs"
mcopy -i $TARGET_PARTITON test.txt "::mydirs/test.txt"

# Detach loopback
losetup -d $DEVICE


