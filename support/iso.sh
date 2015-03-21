#!/bin/sh

mkdir -p ${ISODIR}
mkdir -p ${ISODIR}/boot
mkdir -p ${ISODIR}/boot/grub

cp dummy_os.bin ${ISODIR}/boot/dummy_os.bin

cat > ${ISODIR}/boot/grub/grub.cfg << EOF
menuentry "Dummy OS" {
	multiboot /boot/dummy_os.bin
}
EOF

${GRUB_MKRESCUE} -o dummy_os.iso ${ISODIR}
