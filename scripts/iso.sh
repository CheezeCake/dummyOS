#!/bin/sh

BIN="$1"
ISO="$2"
INITRD="$3"

usage() {
	echo "$0 bin iso initrd"
	exit 1
}

if [ $# -ne 3 ]
then
	usage
fi

ISODIR="isodir"

mkdir -p "${ISODIR}/boot/grub"

cp "${BIN}" "${ISODIR}/boot"
cp "${INITRD}" "${ISODIR}/boot/initrd"

cat > "${ISODIR}/boot/grub/grub.cfg" << EOF
menuentry "dummyOS" {
	multiboot /boot/dummy_os.bin
	module /boot/initrd
}
EOF

grub-mkrescue -o "${ISO}" "${ISODIR}"
