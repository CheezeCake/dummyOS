#include <dummyos/compiler.h>
#include <dummyos/errno.h>
#include <kernel/kassert.h>
#include <kernel/kheap.h>
#include <kernel/kmalloc.h>
#include <kernel/log.h>
#include <kernel/mm/memory.h>
#include <kernel/mm/vm.h>
#include <kernel/mm/vmm.h>

#define KHEAP_MAX_SIZE (256 * 1024 * 1024) // 256MB

static_assert(KHEAP_INITIAL_SIZE < KHEAP_MAX_SIZE,
	      "KHEAP_INITIAL_SIZE >= KHEAP_MAX_SIZE");

static v_addr_t kheap_start;
static v_addr_t kheap_end;

static void dump(void)
{
	log_i_printf("start = %p, end = %p, size = %p, initial_size = 0x%x"
		     " | KHEAP_LIMIT = %p\n",
		     (void*)kheap_start, (void*)kheap_end,
		     (void*)(kheap_end - kheap_start),
		     KHEAP_INITIAL_SIZE, (void*)(kheap_start + KHEAP_MAX_SIZE));
}

size_t kheap_init(v_addr_t start)
{
	kheap_start = start;
	kheap_end = start;

	if (kheap_sbrk(KHEAP_INITIAL_SIZE) < KHEAP_INITIAL_SIZE)
		return -ENOMEM;

	// initialiaze the kmalloc subsystem
	kmalloc_init(kheap_start, kheap_end - kheap_start);

#ifndef NDEBUG
	dump();
#endif

	return (kheap_end - kheap_start);
}

size_t kheap_sbrk(size_t increment)
{
	size_t pages = page_align_up(increment) / PAGE_SIZE;
	size_t i;
	int err = 0;

	for (i = 0; i < pages && !err; ++i) {
		p_addr_t frame = memory_page_frame_alloc();
		if (!frame) {
			err = -ENOMEM;
		}
		else {
			err = vmm_map_kernel_page(frame, kheap_end,
						  VMM_PROT_WRITE);
			if (!err)
				kheap_end += PAGE_SIZE;
		}
	}

	if (i != pages)
		log_i_puts("could not fully satisfy sbrk request\n");

	return (i * PAGE_SIZE);
}

v_addr_t kheap_get_start(void)
{
	return kheap_start;
}

v_addr_t kheap_get_end(void)
{
	return kheap_end;
}
