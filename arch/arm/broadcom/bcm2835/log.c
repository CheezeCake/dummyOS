#include <arch/broadcom/bcm2835/log.h>
#include "uart.h"

const struct log_ops bcm2835_log_ops = {
	.puts		= uart1_puts,
	.putchar	= uart1_putchar,
};
