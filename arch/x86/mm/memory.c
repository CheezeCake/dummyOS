#include <kernel/mm/memory.h>
#include "memory.h"

enum
{
	IOMAP,
};

static mem_area_t mem_layout[] = {
	[IOMAP] = MEMORY_AREA(X86_MEMORY_HARDWARE_MAP_START,
						  X86_MEMORY_HARDWARE_MAP_END,
						  "mapped I/O"),
};

void x86_memory_init(void)
{
	memory_layout_init(mem_layout, sizeof(mem_layout) / sizeof(mem_layout[0]));
}
