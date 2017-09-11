include ../../make.config
include make.config

ARCH_INCLUDE_DIR=../../arch/$(TARGET_ARCH)/include/
KERNEL_INCLUDE_DIR=../../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)

KERNEL_SCHED_DEPS=$(subst .o,.d,$(KERNEL_SCHED_OBJS))

all: $(KERNEL_SCHED_OBJS)

%.d: %.[cS]
	@$(CC) -MM -MF $@ $< $(CFLAGS) $(CPPFLAGS)

include $(KERNEL_SCHED_DEPS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(KERNEL_SCHED_OBJS)
	@rm -f $(KERNEL_SCHED_DEPS)
