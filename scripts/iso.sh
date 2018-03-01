#!/bin/sh

BIN="$1"
ISO="$2"

usage() {
	echo "$0 bin iso"
	exit 1
}

if [ $# -ne 2 ]
then
	usage
fi

ISODIR="isodir"

mkdir -p "${ISODIR}/boot/grub"

cp "${BIN}" "${ISODIR}/boot"

cat > "${ISODIR}/boot/grub/grub.cfg" << EOF
menuentry "dummyOS" {
	multiboot /boot/dummy_os.bin
}
EOF

grub-mkrescue -o "${ISO}" "${ISODIR}"
