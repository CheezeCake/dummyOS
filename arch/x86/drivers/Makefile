include ../../../make.config
include make.config

ARCH_INCLUDE_DIR=../include/
KERNEL_INCLUDE_DIR=../../../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)

ARCH_DRIVERS_DEPS=$(subst .o,.d,$(ARCH_DRIVERS_OBJS))

all: $(ARCH_DRIVERS_OBJS)

%.d: %.[csS]
	$(CC) -MM -MF $@ $< $(CFLAGS) $(CPPFLAGS)

include $(ARCH_DRIVERS_DEPS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(ARCH_DRIVERS_OBJS)
	@rm -f $(ARCH_DRIVERS_DEPS)
