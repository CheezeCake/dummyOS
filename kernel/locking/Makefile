include ../../make.config
include make.config

ARCH_INCLUDE_DIR=../../arch/$(TARGET_ARCH)/include/
KERNEL_INCLUDE_DIR=../../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)

KERNEL_LOCKING_DEPS=$(subst .o,.d,$(KERNEL_LOCKING_OBJS))

all: $(KERNEL_LOCKING_OBJS)

%.d: %.[csS]
	$(CC) -MM -MF $@ $< $(CFLAGS) $(CPPFLAGS)

include $(KERNEL_LOCKING_DEPS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(KERNEL_LOCKING_OBJS)
	@rm -f $(KERNEL_LOCKING_DEPS)
