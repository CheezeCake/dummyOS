#!/bin/sh

KERNEL_FLAG=

case $1 in
	*.iso)
		KERNEL_FLAG="-cdrom $1"
		;;
	*.bin)
		KERNEL_FLAG="-kernel $1"
		;;
	*)
		exit 1
esac

qemu-system-i386 ${QEMU_EXTRA_FLAGS} ${KERNEL_FLAG} -debugcon stdio
