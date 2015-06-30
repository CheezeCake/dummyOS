include ../make.config
include make.config

CPPFLAGS+=-I../include -I../arch/$(TARGET_ARCH)/include

all: $(KERNEL_OBJS)

memory.o: ../include/kernel/memory.h \
	../include/kernel/page_frame_status.h \
	../include/kernel/libk.h \
	../include/kernel/list.h \
	../include/kernel/kassert.h \
	../arch/$(TARGET_ARCH)/include/arch/memory.h \

kernel.o: ../include/kernel/terminal.h \
	../include/kernel/log.h \

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(KERNEL_OBJS)
