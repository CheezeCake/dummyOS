include ../make.config
include make.config

CPPFLAGS+=-I../include -I../arch/$(TARGET_ARCH)/include

all: $(LIBK_OBJS)

%.o: %.c ../include/kernel/libk.h
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(LIBK_OBJS)
