include ../make.config
include make.config

ARCH_INCLUDE_DIR=../arch/$(TARGET_ARCH)/include/
KERNEL_INCLUDE_DIR=../include/

CPPFLAGS+=-I$(KERNEL_INCLUDE_DIR) -I$(ARCH_INCLUDE_DIR)

all: $(LIBK_OBJS)

memcmp.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \

memcpy.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \

memset.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \

snprintf.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \

strcat.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \

strcmp.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \

strcpy.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \

strlcpy.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \

strlen.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \

strncat.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \

strncmp.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \

strncpy.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \

strnlen.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \

vsnprintf.o: \
	$(KERNEL_INCLUDE_DIR)kernel/libk.h \


%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(LIBK_OBJS)
