include ../../make.config
include make.config

ARCH_INCLUDE_DIR=../../arch/$(TARGET_ARCH)/include/
KERNEL_INCLUDE_DIR=../../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)

KERNEL_TIME_DEPS=$(subst .o,.d,$(KERNEL_TIME_OBJS))

all: $(KERNEL_TIME_OBJS)

%.d: %.[cS]
	@$(CC) -MM -MF $@ $< $(CFLAGS) $(CPPFLAGS)

include $(KERNEL_TIME_DEPS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(KERNEL_TIME_OBJS)
	@rm -f $(KERNEL_TIME_DEPS)
