#include <arch/broadcom/bcm2835/bcm2835.h>
#include <arch/broadcom/bcm2836/bcm2836.h>
#include <arch/machine.h>
#include <kernel/arch.h>
#include <kernel/kassert.h>
#include <kernel/kernel.h>
#include <kernel/kernel_image.h>
#include <kernel/mm/memory.h>
#include <kernel/mm/vmm.h>
#include <kernel/time/time.h>
#include <kernel/kheap.h>

#include "exception.h"

#define TICK_INTERVAL_IN_US (1 * 1000 * 1000)

int arm_vmm_register(void);
int paging_init(void);

static size_t machine_get_physical_mem(void)
{
	const struct machine_ops* ops = machine_get_ops();
	kassert(ops->physical_mem);

	return ops->physical_mem();
}

void __kernel_main(void)
{
	extern char _binary_archive_start;
	// TODO: initrd
	kernel_main(&_binary_archive_start);
}

void (*arch_irq_handler)(void) = NULL;

static int machine_init(void)
{
	const struct machine_ops* ops = machine_get_ops();
	int err;

	kassert(ops->irq_handler);
	arch_irq_handler = ops->irq_handler;

	if ((err = ops->init()) ||
	    (err = ops->timer_init(TICK_INTERVAL_IN_US)))
		return err;

	if (ops->log_ops)
		log_register(ops->log_ops);

	return 0;
}

size_t arch_get_mem_size(void)
{
	return machine_get_physical_mem();
}

int arch_console_init(void)
{
	return 0;
}

static int map_mmio(const mem_area_t* mem_layout, size_t mem_layout_size)
{
	int err = 0;

	for (size_t i = 0; i < mem_layout_size && !err; ++i) {
		err = vmm_map_kernel_range(mem_layout[i].start,
					   mem_layout[i].vaddr,
					   mem_layout[i].size,
					   VMM_PROT_WRITE | VMM_PROT_NOCACHE);
	}

	return err;
}

static int arch_mm_init(void)
{
	const struct machine_ops* ops = machine_get_ops();
	const mem_area_t* layout = NULL;
	size_t layout_size = 0;

	__memory_early_init();

	arm_vmm_register();

	if (ops->mem_layout.layout) {
		layout = ops->mem_layout.layout;
		layout_size = ops->mem_layout.size;

		map_mmio(ops->mem_layout.layout, ops->mem_layout.size);
	}

	if (kheap_init(kernel_image_get_top_page()) < KHEAP_INITIAL_SIZE)
		PANIC("Not enough memory for kernel heap!");

	memory_init(machine_get_physical_mem(), layout, layout_size);

	return 0;
}

int arch_init(void)
{
	paging_init();

	arch_mm_init();

	exception_init();

	machine_init();

	struct timespec tick;
	timespec_init(&tick, TICK_INTERVAL_IN_US / 1000);
	time_init(tick);

	return 0;
}
