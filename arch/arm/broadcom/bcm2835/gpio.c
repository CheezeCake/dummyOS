#include <arch/broadcom/bcm2835/peripherals.h>
#include <kernel/interrupt.h>
#include "gpio.h"

enum gpio_register
{
	// GPIO Function Select
	GPFSEL0		= (0x00	>> 2),
	GPFSEL1		= (0x04	>> 2),
	GPFSEL2		= (0x08	>> 2),
	GPFSEL3		= (0x0c	>> 2),
	GPFSEL4		= (0x10 >> 2),
	GPFSEL5		= (0x14 >> 2),

	// GPIO Pin Output Set
	GPSET0		= (0x1c >> 2),
	GPSET1		= (0x20 >> 2),

	// GPIO Pin Output Clear
	GPCLR0		= (0x28 >> 2),
	GPCLR1		= (0x2c >> 2),

	// GPIO Pin Level
	GPLEV0		= (0x34 >> 2),
	GPLEV1		= (0x38 >> 2),

	// GPIO Pin Pull-up/down Enable
	GPPUD		= (0x94 >> 2),

	// GPIO Pin Pull-up/down Enable Clock
	GPPUDCLK0	= (0x98 >> 2),
	GPPUDCLK1	= (0x9c >> 2),
};

static volatile uint32_t* const gpio_base = (volatile uint32_t*)GPIO_BASE;

static __attribute__((always_inline)) void delay(int32_t count)
{
	__asm__ volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
					 : "=r"(count): [count]"0"(count) : "cc");
}

static void gpio_select_function(uint32_t pin, enum gpio_fselect fn)
{
	volatile uint32_t* gpfsel = &gpio_base[GPFSEL0];
	int n = pin / 10;
	int shift = (pin % 10) * 3;

	gpfsel[n] &= ~(7 << shift);
	gpfsel[n] |= fn << shift;
}

void gpio_configure(uint32_t pin, enum gpio_fselect fn, enum gpio_pud pud)
{
	volatile uint32_t* gppudclk = &gpio_base[GPPUDCLK0];
	int clk_n = pin / 32;

	irq_disable();

	gpio_select_function(pin, fn);

	// 1. write control signal
	gpio_base[GPPUD] = pud;
	// 2. wait 150 cycles
	delay(150);

	// 3. clock the control signal
	gppudclk[clk_n] = (1 << (pin % 32));
	// 4. wait 150 cycles
	delay(150);

	// 5. remove the control signal
	gpio_base[GPPUD] = GPIO_PUD_OFF;

	// 6. remove the clock
	gppudclk[clk_n] = 0;

	irq_enable();
}

void gpio_set(uint32_t pin, bool state)
{
	int reg_idx = (state) ? GPSET0 : GPCLR0;
	volatile uint32_t* reg = &gpio_base[reg_idx];

	reg[pin / 32] |= (1 << (pin % 32));
}

bool gpio_get(uint32_t pin)
{
	volatile uint32_t* reg = &gpio_base[GPLEV0];

	return ((reg[pin / 32] >> (pin % 32)) & 1);
}
