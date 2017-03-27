include ../make.config
include make.config

CPPFLAGS+=-I../include -I../arch/$(TARGET_ARCH)/include

all: $(LIBK_OBJS)

memcmp.o: ../include/kernel/libk.h

memcpy.o: ../include/kernel/libk.h

memset.o: ../include/kernel/libk.h

snprintf.o: ../include/kernel/libk.h

strcat.o: ../include/kernel/libk.h

strcmp.o: ../include/kernel/libk.h

strcpy.o: ../include/kernel/libk.h

strlcpy.o: ../include/kernel/libk.h

strlen.o: ../include/kernel/libk.h

strncat.o: ../include/kernel/libk.h

strncmp.o: ../include/kernel/libk.h

strncpy.o: ../include/kernel/libk.h

strnlen.o: ../include/kernel/libk.h

vsnprintf.o: ../include/kernel/libk.h


%.o: %.c ../include/kernel/libk.h
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(LIBK_OBJS)
