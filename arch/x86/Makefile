include ../../make.config
include make.config

ARCH_INCLUDE_DIR=include/
KERNEL_INCLUDE_DIR=../../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)

all: $(ARCH_ROOT_OBJS)
	@$(MAKE) -C boot
	@$(MAKE) -C drivers

.PHONY: all clean

gdt.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \
	gdt.h \
	segment.h \

idt.o: \
	$(KERNEL_INCLUDE_DIR)kernel/interrupt.h \
	idt.h \
	segment.h \

exception.o: \
	$(KERNEL_INCLUDE_DIR)kernel/interrupt.h \
	$(KERNEL_INCLUDE_DIR)kernel/panic.h \
	$(KERNEL_INCLUDE_DIR)kernel/terminal.h \
	exception.h \
	idt.h \
	irq.h \

interrupt.o: \
	exception.h \
	irq.h \

i8259.o: \
	i8259.h \
	io_ports.h \

irq.o: \
	$(KERNEL_INCLUDE_DIR)kernel/interrupt.h \
	i8259.h \
	idt.h \
	irq.h \

cpu.o: \
	$(KERNEL_INCLUDE_DIR)kernel/cpu.h \
	$(KERNEL_INCLUDE_DIR)kernel/log.h \

arch.o: \
	$(ARCH_INCLUDE_DIR)arch/irq.h \
	$(KERNEL_INCLUDE_DIR)kernel/arch.h \
	$(KERNEL_INCLUDE_DIR)kernel/kernel.h \
	$(KERNEL_INCLUDE_DIR)kernel/kernel_image.h \
	$(KERNEL_INCLUDE_DIR)kernel/kmalloc.h \
	$(KERNEL_INCLUDE_DIR)kernel/log.h \
	$(KERNEL_INCLUDE_DIR)kernel/memory.h \
	$(KERNEL_INCLUDE_DIR)kernel/paging.h \
	$(KERNEL_INCLUDE_DIR)kernel/panic.h \
	$(KERNEL_INCLUDE_DIR)kernel/time.h \
	exception.h \
	gdt.h \
	i8254.h \
	idt.h \
	irq.h \

paging.o: paging.c \
	$(ARCH_INCLUDE_DIR)arch/memory.h \
	$(ARCH_INCLUDE_DIR)arch/virtual_memory.h \
	$(KERNEL_INCLUDE_DIR)kernel/kassert.h \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \
	$(KERNEL_INCLUDE_DIR)kernel/memory.h \
	$(KERNEL_INCLUDE_DIR)kernel/paging.h \
	$(KERNEL_INCLUDE_DIR)kernel/types.h \

i8254.o: \
	i8254.h \
	io_ports.h \

cpu_context.o: \
	$(KERNEL_INCLUDE_DIR)kernel/cpu_context.h \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \
	gdt.h \
	segment.h \

debug.o: \
	$(KERNEL_INCLUDE_DIR)kernel/debug.h \
	$(KERNEL_INCLUDE_DIR)kernel/log.h \

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	@$(MAKE) clean -C boot
	@$(MAKE) clean -C drivers
	@rm -f $(ARCH_ROOT_OBJS)
