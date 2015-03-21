include ../make.config
include make.config

CPPFLAGS=-I../include

all: $(KERNEL_OBJS)

%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(KERNEL_OBJS)
