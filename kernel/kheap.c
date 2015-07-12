#include <kernel/kheap.h>
#include <kernel/paging.h>
#include <kernel/memory.h>
#include <arch/memory.h>

#include <kernel/log.h>

static v_addr_t kheap_start;
static v_addr_t kheap_end;

size_t kheap_init(v_addr_t start, size_t initial_size)
{
	v_addr_t addr;

	kheap_start = start;

	for (addr = kheap_start;
			addr < kheap_start + initial_size;
			addr += PAGE_SIZE) {
		p_addr_t page_frame = memory_page_frame_alloc();

		if (!page_frame)
			return 0;

		if (paging_map(page_frame, addr, VM_OPT_WRITE) != 0) {
			memory_unref_page_frame(page_frame);
			break;
		}
	}

	kheap_end = addr;

	return (addr - start);
}

/*
 * Recursive function that allocates a physical page and maps it.
 * If one of the calls "below" fails, unmap and free the physical
 * pages previously allocated
 */
static int _kheap_sbrk(unsigned int page_number, v_addr_t addr)
{
	if (page_number == 0)
		return 0;

	p_addr_t page_frame = memory_page_frame_alloc();

	if (!page_frame || paging_map(page_frame, addr, VM_OPT_WRITE) != 0)
		return -1;

	if (_kheap_sbrk(page_number - 1, addr + PAGE_SIZE) != 0) {
		paging_unmap(addr);
		memory_unref_page_frame(page_frame);
		return -1;
	}

	return 0;
}

v_addr_t kheap_sbrk(size_t increment)
{
	if (kheap_get_end() + increment >= KHEAP_LIMIT)
		return (v_addr_t)NULL;

	unsigned int nb_pages = increment >> PAGE_SIZE_SHIFT; // increment / PAGE_SIZE

	if (_kheap_sbrk(nb_pages, kheap_end) == 0) {
		kheap_end += nb_pages << PAGE_SIZE_SHIFT; // nb_pages * PAGE_SIZE
		return 0;
	}

	return (v_addr_t)NULL;
}

v_addr_t kheap_get_start(void)
{
	return kheap_start;
}

v_addr_t kheap_get_end(void)
{
	return kheap_end;
}
