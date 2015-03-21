include make.config

LDFLAGS:=-lgcc

ARCHDIR=arch/$(TARGET_ARCH)/
LIBKDIR=libk/
KERNELDIR=kernel/
SUPPORTDIR=support/
ISODIR=isodir/

include $(ARCHDIR)make.config
include $(LIBKDIR)make.config
include $(KERNELDIR)make.config
OBJECTS=$(ARCH_OBJS) $(LIBK_OBJS) $(KERNEL_OBJS)

.PHONY: all arch libk kernel clean mrproper rebuild iso run


all: dummy_os.bin

dummy_os.bin: arch libk kernel
	$(CC) -T $(ARCHDIR)kernel.ld -o dummy_os.bin -ffreestanding -nostdlib $(OBJECTS) $(LDFLAGS)

arch:
	@$(MAKE) -C $(ARCHDIR)

libk:
	@$(MAKE) -C $(LIBKDIR)

kernel: arch libk
	@$(MAKE) -C $(KERNELDIR)

iso: dummy_os.bin
	GRUB_MKRESCUE=$(GRUB_MKRESCUE) ISODIR=$(ISODIR) $(SUPPORTDIR)iso.sh

run: iso
	$(SUPPORTDIR)qemu.sh

clean:
	@$(MAKE) clean -C $(ARCHDIR)
	@$(MAKE) clean -C $(LIBKDIR)
	@$(MAKE) clean -C $(KERNELDIR)

mrproper: clean
	@rm -f dummy_os.bin dummy_os.iso
	@rm -rf $(ISODIR)

rebuild: mrproper all
