include ../../make.config

all:
	@$(MAKE) -C boot
	@$(MAKE) -C drivers

.PHONY: all clean

clean:
	@$(MAKE) clean -C boot
	@$(MAKE) clean -C drivers
