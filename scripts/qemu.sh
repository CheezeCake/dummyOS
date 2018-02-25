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

${QEMU} ${KERNEL_FLAG} ${QEMU_FLAGS} ${QEMU_EXTRA_FLAGS}
