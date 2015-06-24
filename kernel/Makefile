include ../make.config
include make.config

CPPFLAGS+=-I../include -I../arch/$(TARGET_ARCH)/include

all: $(KERNEL_OBJS)

kernel.o: ../include/kernel/terminal.h \
	../include/kernel/log.h \

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(KERNEL_OBJS)
