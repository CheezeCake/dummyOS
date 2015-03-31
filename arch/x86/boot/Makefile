include ../../../make.config
include make.config

all: $(ARCH_BOOT_OBJS)

%.o: %.s
	$(AS) $^ -o $@

.PHONY: all clean

clean:
	@rm -f $(ARCH_BOOT_OBJS)
