include ../../make.config

CPPFLAGS+=-I../../include

all: $(ARCH_ROOT_OBJS)
	@$(MAKE) -C boot
	@$(MAKE) -C drivers

.PHONY: all clean

gdt.o: gdt.h

idt.o: idt.h

exception.o: exception.h

interrupt.o: exception.h

i8259.o: i8259.h io_ports.h

cpu.o: ../../include/kernel/cpu.h

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	@$(MAKE) clean -C boot
	@$(MAKE) clean -C drivers
	@rm -f $(ARCH_ROOT_OBJS)
