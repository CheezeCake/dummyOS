include ../../make.config
include make.config

ARCH_INCLUDE_DIR=../../arch/$(TARGET_ARCH)/include/
KERNEL_INCLUDE_DIR=../../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)

all: $(KERNEL_LOCKING_OBJS)

# dependencies

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(KERNEL_LOCKING_OBJS)
