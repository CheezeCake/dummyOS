include ../../../make.config
include make.config

CPPFLAGS:=-I../../../include

all: $(ARCH_DRIVERS_OBJS)

terminal.o: ../../../include/kernel/terminal.h

ioport_0xe9.o: ../../../include/kernel/libk.h ioport_0xe9.h

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(ARCH_DRIVERS_OBJS)
