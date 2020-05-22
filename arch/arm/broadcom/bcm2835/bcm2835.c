#include <arch/broadcom/bcm2835/bcm2835.h>
#include <arch/broadcom/bcm2835/irq.h>
#include <arch/broadcom/bcm2835/log.h>
#include <arch/broadcom/bcm2835/mailbox/property.h>
#include <arch/broadcom/bcm2835/memory.h>
#include <arch/machine.h>
#include <fs/tty.h>
#include <kernel/console.h>
#include <kernel/cpu.h>
#include <kernel/kassert.h>
#include <kernel/page_frame_status.h>
#include <kernel/types.h>
#include <libk/libk.h>

#include "timer.h"
#include "uart.h"

static int uart_console_init(void)
{
	int err;
	struct tty* console_tty;

	err = console_init(uart0_putchar, &console_tty);
	uart0_set_tty(console_tty);

	return err;
}

int bcm2835_machine_init_common(void)
{
	bcm2835_irq_init();

	// log
	uart1_init();

	return uart_console_init();
}

size_t bcm2835_physical_mem(void)
{
	size_t bytes = 0;

	kassert(bcm2835_mailbox_property_get_arm_mem(&bytes) ==
		BCM2835_MAILBOX_PROPERTY_REQUEST_SUCCESS);

	return bytes;
}

static const mem_area_t mem_layout[] = {
	BCM2835_PERIPHERALS_MEMORY_AREA,
};

static const struct machine_ops bcm2835_ops = {
	.init		= bcm2835_machine_init_common,
	.timer_init	= bcm2835_timer_init,
	.irq_handler	= bcm2835_irq_handler,
	.physical_mem	= bcm2835_physical_mem,
	.mem_layout	= MACHINE_MEMORY_LAYOUT(mem_layout),
	.log_ops	= &bcm2835_log_ops,
};

const struct machine_ops* bcm2835_machine_ops(void)
{
	return &bcm2835_ops;
}


int terminal_printf(const char* format, ...)
{
	const int buffer_size = 256;
	char buffer[buffer_size];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, buffer_size, format, ap);
	va_end(ap);

	return uart0_puts(buffer);
}

void terminal_putchar(char c)
{
	uart0_putchar(c);
}

int terminal_puts(const char* str)
{
	return uart0_puts(str);
}

void terminal_write(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_clear(uint8_t color)
{
}

void terminal_init(void)
{
	uart0_init();
}

void debug_dump(void)
{
}
