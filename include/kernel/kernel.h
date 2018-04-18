#ifndef _KERNEL_KERNEL_H_
#define _KERNEL_KERNEL_H_

#include <kernel/interrupt.h>

void clock_tick(int nr, struct cpu_context* interrupted_ctx);

#endif
