include ../../make.config
include make.config

ARCH_INCLUDE_DIR=include/
KERNEL_INCLUDE_DIR=../../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)

ASM_SOURCES_DEPS=cpu_context_switch.d syscall.d
TMP_ARCH_ROOT_DEPS:=$(subst .o,.d,$(ARCH_ROOT_OBJS))
ARCH_ROOT_DEPS=$(filter-out $(ASM_SOURCES_DEPS), $(TMP_ARCH_ROOT_DEPS))

all: $(ARCH_ROOT_OBJS)
	@$(MAKE) -C boot
	@$(MAKE) -C drivers

%.d: %.[cS]
	@$(CC) -MM -MF $@ $< $(CFLAGS) $(CPPFLAGS)

include $(ARCH_ROOT_DEPS)

.PHONY: all clean

%.o: %.s
	$(AS) $^ -o $@

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	@rm -f $(ARCH_ROOT_OBJS)
	@rm -f $(ARCH_ROOT_DEPS)
	@$(MAKE) clean -C boot
	@$(MAKE) clean -C drivers
