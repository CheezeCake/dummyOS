include ../make.config
include make.config

ARCH_INCLUDE_DIR=../arch/$(TARGET_ARCH)/include/
KERNEL_INCLUDE_DIR=../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)

LIBK_DEPS=$(subst .o,.d,$(LIBK_OBJS))

all: $(LIBK_OBJS)

%.d: %.[cS]
	@$(CC) -MM -MF $@ $< $(CFLAGS) $(CPPFLAGS)

include $(LIBK_DEPS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(LIBK_OBJS)
	@rm -f $(LIBK_DEPS)
