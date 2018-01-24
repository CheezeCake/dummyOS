include make.config

LDFLAGS:=-lgcc

ARCHDIR=arch/$(TARGET_ARCH)/
LIBKDIR=libk/
KERNELDIR=kernel/
FSDIR=fs/
SCRIPTSDIR=scripts/
ISODIR=isodir/

include $(ARCHDIR)make.config
include $(LIBKDIR)make.config
include $(KERNELDIR)make.config
include $(FSDIR)make.config
RAMFS=
OBJECTS=$(ARCH_OBJS) $(LIBK_OBJS) $(KERNEL_OBJS) $(FS_OBJS) $(RAMFS)
KERNEL_LINKER_SCRIPT=$(ARCHDIR)kernel.ld

KERNEL_BIN=dummy_os.bin
KERNEL_ISO=dummy_os.iso

.PHONY: all arch libk kernel fs clean mrproper rebuild bin iso run debug


all: bin

$(KERNEL_BIN): $(OBJECTS) $(KERNEL_LINKER_SCRIPT)
	$(CC) -T $(KERNEL_LINKER_SCRIPT) -o $@ -ffreestanding -nostdlib $(OBJECTS) $(LDFLAGS)

$(KERNEL_ISO): $(KERNEL_BIN)
	ISODIR=$(ISODIR) $(SCRIPTSDIR)iso.sh

arch:
	@$(MAKE) -C $(ARCHDIR)

libk:
	@$(MAKE) -C $(LIBKDIR)

kernel: arch libk
	@$(MAKE) -C $(KERNELDIR)

fs:
	@$(MAKE) -C $(FSDIR)

bin: arch libk kernel fs $(KERNEL_BIN)

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
	@$(MAKE) clean -C $(FSDIR)

mrproper: clean
	@rm -f $(KERNEL_BIN) $(KERNEL_ISO)
	@rm -rf $(ISODIR)

rebuild: mrproper all
