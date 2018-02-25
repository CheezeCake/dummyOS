#ifndef _KERNEL_ARCH_H_
#define _KERNEL_ARCH_H_

#include <kernel/types.h>

int arch_init(void);

int arch_mm_init(size_t ram_size_bytes);

#endif
