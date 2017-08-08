#!/bin/sh

qemu_extra_flags=

if [ $1 = 'debug' ]
then
	qemu_extra_flags='-s'
fi

qemu-system-i386 $qemu_extra_flags -cdrom dummy_os.iso -debugcon stdio
