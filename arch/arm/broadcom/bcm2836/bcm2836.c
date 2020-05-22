#include <arch/broadcom/bcm2835/bcm2835.h>
#include <arch/broadcom/bcm2835/irq.h>
#include <arch/broadcom/bcm2835/log.h>
#include <arch/broadcom/bcm2835/memory.h>
#include <arch/machine.h>

#include "peripherals.h"
#include "timer.h"

static void bcm2836_irq_handler(void)
{
	if (bcm2836_timer_irq())
		return;

	bcm2835_irq_handler();
}

int bcm2835_timer_init(uint32_t usec);

static const mem_area_t mem_layout[] = {
	BCM2835_PERIPHERALS_MEMORY_AREA,
	MEMORY_AREA(BCM2836_ARM_PERIPHERALS_BASE_PHYS,
		    BCM2836_ARM_PERIPHERALS_SIZE,
		    BCM2836_ARM_PERIPHERALS_BASE,
		    "ARM MMIO"),
};

static const struct machine_ops bcm2836_ops = {
	.init		= bcm2835_machine_init_common,
	.timer_init	= bcm2836_timer_init,
	/* .timer_init	= bcm2835_timer_init, */
	.irq_handler	= bcm2836_irq_handler,
	.physical_mem	= bcm2835_physical_mem,
	.mem_layout	= MACHINE_MEMORY_LAYOUT(mem_layout),
	.log_ops	= &bcm2835_log_ops,
};

const struct machine_ops* bcm2836_machine_ops(void)
{
	return &bcm2836_ops;
}
