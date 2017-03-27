include ../../../make.config
include make.config

ARCH_INCLUDE_DIR=../include/
KERNEL_INCLUDE_DIR=../../../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)

all: $(ARCH_DRIVERS_OBJS)

terminal.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \
	$(KERNEL_INCLUDE_DIR)kernel/terminal.h \
	$(KERNEL_INCLUDE_DIR)kernel/vga.h \

ioport_0xe9.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \
	ioport_0xe9.h \

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(ARCH_DRIVERS_OBJS)
