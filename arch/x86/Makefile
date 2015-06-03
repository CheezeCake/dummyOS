include ../../make.config

CPPFLAGS+=-I../../include

all: $(ARCH_ROOT_OBJS)
	@$(MAKE) -C boot
	@$(MAKE) -C drivers

.PHONY: all clean

%.o: %.c %.h
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	@$(MAKE) clean -C boot
	@$(MAKE) clean -C drivers
	@rm -f $(ARCH_ROOT_OBJS)
