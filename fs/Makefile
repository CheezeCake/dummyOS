include ../make.config
include make.config

ARCH_INCLUDE_DIR=../arch/$(TARGET_ARCH)/include/
KERNEL_INCLUDE_DIR=../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)

FSROOT_DEPS=$(subst .o,.d,$(FSROOT_OBJS))

all: $(FSROOT_OBJS)
	@$(MAKE) -C ramfs

%.d: %.[cS]
	@$(CC) -MM -MF $@ $< $(CFLAGS) $(CPPFLAGS)

include $(FS_DEPS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(FSROOT_OBJS)
	@rm -f $(FSROOT_DEPS)
	@$(MAKE) clean -C ramfs
