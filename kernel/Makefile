include ../make.config
include make.config

ARCH_INCLUDE_DIR=../arch/$(TARGET_ARCH)/include/
KERNEL_INCLUDE_DIR=../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)
KERNEL_ROOT_DEPS=$(subst .o,.d,$(KERNEL_ROOT_OBJS))

all: $(KERNEL_ROOT_OBJS)
	@$(MAKE) -C locking
	@$(MAKE) -C time

%.d: %.[cS]
	@$(CC) -MM -MF $@ $< $(CFLAGS) $(CPPFLAGS)

include $(KERNEL_ROOT_DEPS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(KERNEL_ROOT_OBJS)
	@rm -f $(KERNEL_ROOT_DEPS)
	@$(MAKE) clean -C locking
	@$(MAKE) clean -C time
