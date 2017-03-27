include ../make.config
include make.config

CPPFLAGS+=-I../include -I../arch/$(TARGET_ARCH)/include

all: $(KERNEL_OBJS)

memory.o: ../include/kernel/memory.h \
	../include/kernel/page_frame_status.h \
	../include/kernel/libk.h \
	../include/kernel/list.h \
	../include/kernel/kassert.h \
	../include/kernel/kmalloc.h \
	../include/kernel/kernel_image.h \
	../arch/$(TARGET_ARCH)/include/arch/memory.h \
	../include/kernel/log.h

kernel.o: ../include/kernel/terminal.h \
	../include/kernel/log.h \
	../include/kernel/kernel.h \
	../include/kernel/arch.h \
	../include/kernel/cpu.h \
	../include/kernel/interrupt.h \
	../include/kernel/kassert.h \
	../include/kernel/multiboot.h \
	../include/kernel/sched.h \
	../include/kernel/thread.h \
	../include/kernel/time.h \

kernel_image.o: ../include/kernel/kernel_image.h \
	../include/kernel/memory.h \
	../include/kernel/kassert.h

kmalloc.o: ../include/kernel/kmalloc.h \
	../include/kernel/kheap.h \
	../include/kernel/panic.h \
	../include/kernel/log.h \

kheap.o: ../include/kernel/kheap.h \
	../include/kernel/memory.h \
	../arch/$(TARGET_ARCH)/include/arch/memory.h \
	../include/kernel/log.h \
	../include/kernel/paging.h \

time.o: ../include/kernel/time.h \
	../include/kernel/kassert.h \
	../include/kernel/log.h \
	../include/kernel/sched.h \

thread.o: ../include/kernel/thread.h \
	../include/kernel/kmalloc.h \
	../include/kernel/libk.h \
	../include/kernel/cpu_context.h \
	../include/kernel/sched.h \

sched.o: ../include/kernel/sched.h \
	../include/kernel/time.h \
	../include/kernel/kassert.h \
	../include/kernel/cpu_context.h \
	../include/kernel/libk.h \
	../include/kernel/log.h


%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(KERNEL_OBJS)
