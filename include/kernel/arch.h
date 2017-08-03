#ifndef _KERNEL_ARCH_H_
#define _KERNEL_ARCH_H_

#include <stddef.h>

int arch_init(void);
void arch_memory_management_init(size_t ram_size_bytes);

#endif
