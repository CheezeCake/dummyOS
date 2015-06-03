include ../make.config
include make.config

CPPFLAGS+=-I../include

all: $(KERNEL_OBJS)

kernel.o: ../include/kernel/terminal.h \
	../include/kernel/log.h \
	../arch/x86/log.h

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(KERNEL_OBJS)
