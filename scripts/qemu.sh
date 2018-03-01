#!/bin/sh

usage() {
	echo "$0 qemu-system-suffix (kernel_bin | kernel_iso) qemu_flags..."
	exit 1
}

if [ $# -lt 2 ]
then
	usage
fi

QEMU="$1"
KERNEL_FLAG=
case $2 in
	*.iso)
		KERNEL_FLAG="-cdrom $2"
		;;
	*.bin)
		KERNEL_FLAG="-kernel $2"
		;;
	*)
		usage
esac
shift 2
QEMU_FLAGS=$*

cd "${MESON_BUILD_ROOT}"
"qemu-system-${QEMU}" ${KERNEL_FLAG} ${QEMU_FLAGS}
