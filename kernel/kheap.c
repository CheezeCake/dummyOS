#include <kernel/kheap.h>
#include <kernel/paging.h>
#include <kernel/memory.h>
#include <arch/memory.h>
#include <kernel/kassert.h>

static v_addr_t kheap_start;
static v_addr_t kheap_end;

#define KHEAP_SBRK_MAX_SIZE_IN_PAGES 16
static struct
{
	p_addr_t page_frame;
	v_addr_t mapping_address;
} mapped_pages[KHEAP_SBRK_MAX_SIZE_IN_PAGES] = { {0, 0} };

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

	return (addr - start);
}

static void sbrk_cleanup(unsigned int nb_pages)
{
	for (int i = 0; i < nb_pages; ++i) {
		paging_unmap(mapped_pages[i].mapping_address);
		memory_page_frame_free(mapped_pages[i].page_frame);

		mapped_pages[i].page_frame = 0;
		mapped_pages[i].mapping_address = 0;
	}
}

v_addr_t kheap_sbrk(size_t increment)
{
	if (kheap_end + increment >= KHEAP_LIMIT)
		return -1;

	const v_addr_t previous_kheap_end = kheap_end;
	const unsigned int nb_pages = increment >> PAGE_SIZE_SHIFT; // increment / PAGE_SIZE

	if (nb_pages > KHEAP_SBRK_MAX_SIZE_IN_PAGES)
		return -1;

	v_addr_t mapping_address = kheap_end;
	for (int i = 0; i < nb_pages; ++i) {
		p_addr_t page_frame = memory_page_frame_alloc();

		if (!page_frame) {
			memory_page_frame_free(page_frame);
			sbrk_cleanup(i);
			return -1;
		}

		if (paging_map(page_frame, mapping_address, VM_OPT_WRITE) != 0) {
			sbrk_cleanup(i + 1);
			return -1;
		}

		mapped_pages[i].page_frame = page_frame;
		mapped_pages[i].mapping_address = mapping_address;

		mapping_address += PAGE_SIZE;
	}

	kheap_end += nb_pages << PAGE_SIZE_SHIFT; // nb_pages * PAGE_SIZE

	return previous_kheap_end;
}

v_addr_t kheap_get_start(void)
{
	return kheap_start;
}

v_addr_t kheap_get_end(void)
{
	return kheap_end;
}
