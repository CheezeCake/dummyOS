include ../../make.config
include make.config

ARCH_INCLUDE_DIR=../../arch/$(TARGET_ARCH)/include/
KERNEL_INCLUDE_DIR=../../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)

FS_RAMFS_DEPS=$(subst .o,.d,$(FS_RAMFS_OBJS))

all: $(FS_RAMFS_OBJS)

%.d: %.[cS]
	@$(CC) -MM -MF $@ $< $(CFLAGS) $(CPPFLAGS)

include $(FS_RAMFS_DEPS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(FS_RAMFS_OBJS)
	@rm -f $(FS_RAMFS_DEPS)
