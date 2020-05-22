#ifndef _GPIO_H_
#define _GPIO_H_

#include <kernel/types.h>

enum gpio_fselect
{
	GPIO_INPUT,
	GPIO_OUTPUT,
	GPIO_FN5,
	GPIO_FN4,
	GPIO_FN0,
	GPIO_FN1,
	GPIO_FN2,
	GPIO_FN3
};

enum gpio_pud // pull up/down
{
	GPIO_PUD_OFF,
	GPIO_PUD_DOWN,
	GPIO_PUD_UP
};

void gpio_configure(uint32_t pin, enum gpio_fselect fn, enum gpio_pud pud);

void gpio_set(uint32_t pin, bool state);

bool gpio_get(uint32_t pin);

#endif
