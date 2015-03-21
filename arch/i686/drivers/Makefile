include ../../../make.config
include make.config

CPPFLAGS:=-I../../../include

all: $(ARCH_DRIVERS_OBJS)

%.o: %.c
	$(CC) -c $^ -o $@ $(CFLAGS) $(CPPFLAGS)

.PHONY: all clean

clean:
	@rm -f $(ARCH_DRIVERS_OBJS)
