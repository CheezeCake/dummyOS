include ../../make.config
include make.config

ARCH_INCLUDE_DIR=include/
KERNEL_INCLUDE_DIR=../../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)

ARCH_ROOT_DEPS=$(subst .o,.d,$(ARCH_ROOT_OBJS))

all: $(ARCH_ROOT_OBJS)
	@$(MAKE) -C boot
	@$(MAKE) -C drivers

%.d: %.[csS]
	$(CC) -MM -MF $@ $< $(CFLAGS) $(CPPFLAGS)

include $(ARCH_ROOT_DEPS)

.PHONY: all clean

%.o: %.s
	@touch $(subst .s,.d,$<)
	$(AS) $^ -o $@

%.o: %.c
	@echo onche
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	@rm -f $(ARCH_ROOT_OBJS)
	@rm -f $(ARCH_ROOT_DEPS)
	@$(MAKE) clean -C boot
	@$(MAKE) clean -C drivers
