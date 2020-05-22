#ifndef _KERNEL_ARCH_H_
#define _KERNEL_ARCH_H_

#include <kernel/types.h>

int arch_init(void);

int arch_console_init(void);

size_t arch_get_mem_size(void);

#endif
