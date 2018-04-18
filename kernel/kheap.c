#include <kernel/kassert.h>
#include <kernel/kheap.h>
#include <kernel/kmalloc.h>
#include <kernel/log.h>
#include <kernel/mm/memory.h>
#include <kernel/mm/vmm.h>

#define KHEAP_LIMIT KERNEL_SPACE_LIMIT

static v_addr_t kheap_start;
static v_addr_t kheap_end;

static void dump(void)
{
	log_i_printf("start = %p, end = %p, size = %p, initial_size = 0x%x"
				 " | KHEAP_LIMIT = 0x%x\n",
				 (void*)kheap_start, (void*)kheap_end,
				 (void*)(kheap_end - kheap_start),
				 KHEAP_INITIAL_SIZE, KHEAP_LIMIT);
}

size_t kheap_init(v_addr_t start, size_t initial_size)
{
	int err;

	initial_size = page_align_up(initial_size);
	kassert(start + initial_size < KHEAP_LIMIT);

	kheap_start = start;
	kheap_end = start + initial_size;

	err = vmm_create_kernel_mapping(kheap_start, initial_size, VMM_PROT_WRITE);
	if (err)
		return 0;


	// initialiaze the kmalloc subsystem
	kmalloc_init(kheap_start, kheap_end - kheap_start);

#ifndef NDEBUG
	dump();
#endif

	return (kheap_end - kheap_start);
}

size_t kheap_sbrk(size_t increment)
{
	int err;

	increment = page_align_up(increment);

	err = vmm_create_kernel_mapping(kheap_end, increment, VMM_PROT_WRITE);
	if (err)
		return 0;

	kheap_end += increment;

	return increment;
}

v_addr_t kheap_get_start(void)
{
	return kheap_start;
}

v_addr_t kheap_get_end(void)
{
	return kheap_end;
}
