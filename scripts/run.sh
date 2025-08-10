#!/usr/bin/sh

mkdir -p logs/

date=$(date '+%d-%m-%Y--%H.%M.%S')
logfile="logs/qemu-logs-$date.log"
touch "$logfile"

# Absolute path
path_to="$(pwd)/$logfile"

echo "$path_to"
#QEMU_ARGS="-debugcon file:${path_to} -m 32"
QEMU_ARGS="-debugcon stdio -m 32"

if [ "$#" -le 1 ]; then
    echo "Usage: ./run.sh <image_type> <image>"
    exit 1
fi

case "$1" in
    "floppy")   QEMU_ARGS="${QEMU_ARGS} -fda $2"
    ;;
    "disk")     QEMU_ARGS="${QEMU_ARGS} -hda $2"
    ;;
    *)          echo "Unknown image type $1."
                exit 2
esac

qemu-system-i386 $QEMU_ARGS
