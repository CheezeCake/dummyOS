#include <arch/broadcom/bcm2835/bcm2835.h>
#include <arch/broadcom/bcm2836/bcm2836.h>
#include <arch/machine.h>

#include <config.h>

const struct machine_ops* machine_get_ops(void)
{
	const struct machine_ops* ops =
#ifdef RPI_MODEL
# if RPI_MODEL <= 1
	bcm2835_machine_ops();
# elif RPI_MODEL == 2
	bcm2836_machine_ops();
# else
# error unsupported raspberry pi model
# endif
#endif
	return ops;
}
