#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <kernel/types.h>

#define X86_MEMORY_HARDWARE_MAP_START	0xa0000
#define X86_MEMORY_HARDWARE_MAP_END	0x100000

void x86_memory_init(void);

#endif
