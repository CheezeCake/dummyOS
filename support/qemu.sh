#!/bin/sh

qemu-system-i386 ${QEMU_EXTRA_FLAGS} -cdrom dummy_os.iso -debugcon stdio
