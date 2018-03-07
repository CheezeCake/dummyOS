#!/bin/sh

usage() {
	echo "$0 arch qemu_flags..."
	exit 1
}

if [ $# -lt 1 ]
then
	usage
fi

QEMU_SYSTEM="$1"
shift
if [ ${QEMU_SYSTEM} = 'x86' ]
then
	QEMU_SYSTEM='i386'
fi

cd "${MESON_BUILD_ROOT}"
"qemu-system-${QEMU_SYSTEM}" -debugcon stdio $*
