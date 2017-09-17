include make.config

LDFLAGS:=-lgcc

ARCHDIR=arch/$(TARGET_ARCH)/
LIBKDIR=libk/
KERNELDIR=kernel/
SCRIPTSDIR=scripts/
ISODIR=isodir/

include $(ARCHDIR)make.config
include $(LIBKDIR)make.config
include $(KERNELDIR)make.config
OBJECTS=$(ARCH_OBJS) $(LIBK_OBJS) $(KERNEL_OBJS)

KERNEL_BIN=dummy_os.bin
KERNEL_ISO=dummy_os.iso

.PHONY: all arch libk kernel clean mrproper rebuild bin iso run debug


all: bin

$(KERNEL_BIN): $(OBJECTS)
	$(CC) -T $(ARCHDIR)kernel.ld -o $@ -ffreestanding -nostdlib $(OBJECTS) $(LDFLAGS)

$(KERNEL_ISO): $(KERNEL_BIN)
	GRUB_MKRESCUE=$(GRUB_MKRESCUE) ISODIR=$(ISODIR) $(SCRIPTSDIR)iso.sh

arch:
	@$(MAKE) -C $(ARCHDIR)

libk:
	@$(MAKE) -C $(LIBKDIR)

kernel: arch libk
	@$(MAKE) -C $(KERNELDIR)

bin: arch libk kernel $(KERNEL_BIN)

iso: bin $(KERNEL_ISO)

run: bin
	$(SCRIPTSDIR)qemu.sh $(KERNEL_BIN)

run-iso: iso
	$(SCRIPTSDIR)qemu.sh $(KERNEL_ISO)

debug: bin
	QEMU_EXTRA_FLAGS='-s -S' $(SCRIPTSDIR)qemu.sh $(KERNEL_BIN)

clean:
	@$(MAKE) clean -C $(ARCHDIR)
	@$(MAKE) clean -C $(LIBKDIR)
	@$(MAKE) clean -C $(KERNELDIR)

mrproper: clean
	@rm -f $(KERNEL_BIN) $(KERNEL_ISO)
	@rm -rf $(ISODIR)

rebuild: mrproper all
