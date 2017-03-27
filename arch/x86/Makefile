include ../../make.config

CPPFLAGS+=-I../../include -Iinclude

all: $(ARCH_ROOT_OBJS)
	@$(MAKE) -C boot
	@$(MAKE) -C drivers

.PHONY: all clean

gdt.o: gdt.h \
	segment.h \
	../../include/kernel/libk.h

idt.o: idt.h \
	../../include/kernel/interrupt.h \
	segment.h

exception.o: exception.h idt.h \
	irq.h \
	../../include/kernel/interrupt.h \
	../../include/kernel/panic.h \
	../../include/kernel/terminal.h

interrupt.o: exception.h irq.h

i8259.o: i8259.h io_ports.h

irq.o: irq.h i8259.h idt.h ../../include/kernel/interrupt.h

cpu.o: ../../include/kernel/cpu.h \
	../../include/kernel/log.h

arch.o: ../../include/kernel/arch.h \
	../../include/kernel/panic.h \
	../../include/kernel/kernel.h \
	../../include/kernel/time.h \
	../../include/kernel/kernel_image.h \
	../../include/kernel/kmalloc.h \
	../../include/kernel/log.h \
	../../include/kernel/memory.h \
	../../include/kernel/paging.h \
	exception.h \
	gdt.h \
	i8254.h \
	idt.h \
	irq.h

paging.o: ../../include/kernel/paging.h \
	include/arch/memory.h \
	include/arch/virtual_memory.h \
	../../include/kernel/kassert.h \
	../../include/kernel/libk.h \
	../../include/kernel/memory.h \
	../../include/kernel/types.h

i8254.o: i8254.h io_ports.h

cpu_context.o: ../../include/kernel/cpu_context.h \
	../../include/kernel/libk.h \
	gdt.h \
	segment.h

debug.o: ../../include/kernel/debug.h \
	../../include/kernel/log.h

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	@$(MAKE) clean -C boot
	@$(MAKE) clean -C drivers
	@rm -f $(ARCH_ROOT_OBJS)
