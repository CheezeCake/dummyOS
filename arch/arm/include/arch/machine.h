#ifndef _ARCH_MACHINE_H_
#define _ARCH_MACHINE_H_

#include <kernel/mm/memory.h>
#include <kernel/types.h>
#include <libk/utils.h>

struct machine_ops
{
	int (*init)(void);
	int (*timer_init)(uint32_t tick_usec);
	void (*irq_handler)(void);
	size_t (*physical_mem)(void);
	struct {
		const mem_area_t* layout;
		size_t size;
	} mem_layout;
	const struct log_ops *log_ops;
};

#define MACHINE_MEMORY_LAYOUT(arr) { .layout = arr, .size = ARRAY_SIZE(arr) }

const struct machine_ops* machine_get_ops(void);

#endif
