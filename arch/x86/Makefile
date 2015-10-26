include ../../make.config

CPPFLAGS+=-I../../include -Iinclude

all: $(ARCH_ROOT_OBJS)
	@$(MAKE) -C boot
	@$(MAKE) -C drivers

.PHONY: all clean

gdt.o: gdt.h idt.h

idt.o: idt.h

exception.o: exception.h idt.h irq.h ../../include/kernel/interrupt.h

interrupt.o: exception.h irq.h

i8259.o: i8259.h io_ports.h

irq.o: irq.h i8259.h idt.h ../../include/kernel/interrupt.h

cpu.o: ../../include/kernel/cpu.h

arch.o: ../../include/kernel/arch.h \
	../../include/kernel/panic.h \
	../../include/kernel/kernel.h \
	../../include/kernel/time.h

paging.o: ../../include/kernel/paging.h

i8254.o: i8254.h io_ports.h

cpu_context.o: ../../include/kernel/cpu_context.h \
	../../include/kernel/libk.h


%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	@$(MAKE) clean -C boot
	@$(MAKE) clean -C drivers
	@rm -f $(ARCH_ROOT_OBJS)
