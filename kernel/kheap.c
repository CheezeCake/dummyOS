#include <kernel/kassert.h>
#include <kernel/kheap.h>
#include <kernel/kmalloc.h>
#include <kernel/log.h>
#include <kernel/memory.h>
#include <kernel/paging.h>

#define KHEAP_LIMIT KERNEL_VADDR_SPACE_LIMIT

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
	kassert(start + initial_size < KHEAP_LIMIT);

	v_addr_t addr;

	kheap_start = start;

	for (addr = kheap_start;
			addr < kheap_start + initial_size;
			addr += PAGE_SIZE) {
		p_addr_t page_frame = memory_page_frame_alloc();

		if (!page_frame)
			return 0;

		if (paging_map(page_frame, addr, VM_OPT_WRITE) != 0) {
			memory_page_frame_free(page_frame);
			break;
		}
	}

	kheap_end = addr;

	// initialiaze the kmalloc subsystem
	kmalloc_init(kheap_start, kheap_end - kheap_start);

#ifndef NDEBUG
	dump();
#endif

	return (kheap_end - kheap_start);
}

size_t kheap_extend_pages(size_t nr_pages)
{
	v_addr_t map_to = kheap_end;
	size_t mapped_pages;

	for (mapped_pages = 0;
		 mapped_pages < nr_pages && map_to < KHEAP_LIMIT;
		 ++mapped_pages, map_to += PAGE_SIZE)
	{
		p_addr_t frame;

		frame = memory_page_frame_alloc();

		if (!frame) {
			log_i_printf("Failed to allocate page frame %lu "
						 "(%lu pages requested).\n", mapped_pages, nr_pages);
			break;
		}
		if (paging_map(frame, map_to, VM_OPT_WRITE) != 0) {
			log_i_printf("Failed to map page frame %lu to address %p "
						 "(%lu pages requested).\n", mapped_pages, (void*)map_to,
						 nr_pages);
			memory_page_frame_free(frame);
			break;
		}
	}

	kheap_end = map_to;

	dump();

	return mapped_pages;
}

v_addr_t kheap_get_start(void)
{
	return kheap_start;
}

v_addr_t kheap_get_end(void)
{
	return kheap_end;
}
