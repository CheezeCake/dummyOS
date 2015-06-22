include ../../make.config

CPPFLAGS+=-I../../include

all: $(ARCH_ROOT_OBJS)
	@$(MAKE) -C boot
	@$(MAKE) -C drivers

.PHONY: all clean

gdt.o: gdt.h

idt.o: idt.h

exception.o: exception.h idt.h irq.h interrupt.h

interrupt.o: exception.h irq.h

i8259.o: i8259.h io_ports.h

irq.o: irq.h i8259.h idt.h interrupt.h

cpu.o: ../../include/kernel/cpu.h

arch.o: ../../include/kernel/arch.h

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	@$(MAKE) clean -C boot
	@$(MAKE) clean -C drivers
	@rm -f $(ARCH_ROOT_OBJS)
