include ../make.config
include make.config

ARCH_INCLUDE_DIR=../arch/$(TARGET_ARCH)/include/
KERNEL_INCLUDE_DIR=../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)

all: $(KERNEL_OBJS)

memory.o: \
	$(ARCH_INCLUDE_DIR)arch/memory.h \
	$(KERNEL_INCLUDE_DIR)kernel/kassert.h \
	$(KERNEL_INCLUDE_DIR)kernel/kernel_image.h \
	$(KERNEL_INCLUDE_DIR)kernel/kmalloc.h \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \
	$(KERNEL_INCLUDE_DIR)kernel/log.h \
	$(KERNEL_INCLUDE_DIR)kernel/memory.h \
	$(KERNEL_INCLUDE_DIR)kernel/page_frame_status.h \
	$(KERNEL_INCLUDE_DIR)kernel/sz_list.h \

kernel.o: \
	$(KERNEL_INCLUDE_DIR)kernel/arch.h \
	$(KERNEL_INCLUDE_DIR)kernel/cpu.h \
	$(KERNEL_INCLUDE_DIR)kernel/interrupt.h \
	$(KERNEL_INCLUDE_DIR)kernel/kassert.h \
	$(KERNEL_INCLUDE_DIR)kernel/kernel.h \
	$(KERNEL_INCLUDE_DIR)kernel/log.h \
	$(KERNEL_INCLUDE_DIR)kernel/multiboot.h \
	$(KERNEL_INCLUDE_DIR)kernel/sched.h \
	$(KERNEL_INCLUDE_DIR)kernel/terminal.h \
	$(KERNEL_INCLUDE_DIR)kernel/thread.h \
	$(KERNEL_INCLUDE_DIR)kernel/time.h \

kernel_image.o: \
	$(KERNEL_INCLUDE_DIR)kernel/kassert.h \
	$(KERNEL_INCLUDE_DIR)kernel/kernel_image.h \
	$(KERNEL_INCLUDE_DIR)kernel/memory.h \

kmalloc.o: \
	$(KERNEL_INCLUDE_DIR)kernel/kheap.h \
	$(KERNEL_INCLUDE_DIR)kernel/kmalloc.h \
	$(KERNEL_INCLUDE_DIR)kernel/locking/spinlock.h \
	$(KERNEL_INCLUDE_DIR)kernel/log.h \
	$(KERNEL_INCLUDE_DIR)kernel/panic.h \

kheap.o: \
	$(ARCH_INCLUDE_DIR)arch/memory.h \
	$(KERNEL_INCLUDE_DIR)kernel/kheap.h \
	$(KERNEL_INCLUDE_DIR)kernel/log.h \
	$(KERNEL_INCLUDE_DIR)kernel/memory.h \
	$(KERNEL_INCLUDE_DIR)kernel/paging.h \

time.o: \
	$(KERNEL_INCLUDE_DIR)kernel/kassert.h \
	$(KERNEL_INCLUDE_DIR)kernel/log.h \
	$(KERNEL_INCLUDE_DIR)kernel/sched.h \
	$(KERNEL_INCLUDE_DIR)kernel/time.h \

thread.o: \
	$(KERNEL_INCLUDE_DIR)kernel/cpu_context.h \
	$(KERNEL_INCLUDE_DIR)kernel/kmalloc.h \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \
	$(KERNEL_INCLUDE_DIR)kernel/sched.h \
	$(KERNEL_INCLUDE_DIR)kernel/thread.h \

sched.o: \
	$(KERNEL_INCLUDE_DIR)kernel/cpu_context.h \
	$(KERNEL_INCLUDE_DIR)kernel/kassert.h \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \
	$(KERNEL_INCLUDE_DIR)kernel/log.h \
	$(KERNEL_INCLUDE_DIR)kernel/sched.h \
	$(KERNEL_INCLUDE_DIR)kernel/time.h \

semaphore.o: \
	$(KERNEL_INCLUDE_DIR)kernel/locking/semaphore.h \


%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(KERNEL_OBJS)
