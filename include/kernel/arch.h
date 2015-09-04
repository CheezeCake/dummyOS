#ifndef _KARCH_H_
#define _KARCH_H_

#include <stddef.h>

int arch_init(void);
void arch_memory_management_init(size_t ram_size_bytes);

#endif
