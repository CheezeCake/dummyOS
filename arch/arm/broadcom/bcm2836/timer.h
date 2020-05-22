#ifndef _TIMER_H_
#define _TIMER_H_

#include <kernel/types.h>

int bcm2836_timer_init(uint32_t usec);
bool bcm2836_timer_irq(void);

#endif
