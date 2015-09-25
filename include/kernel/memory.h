#ifndef _KMEMORY_H_
#define _KMEMORY_H_

#include <stddef.h>
#include <kernel/page_frame_status.h>
#include <kernel/types.h>
#include <arch/memory.h>

/*
 * describes a physical page
 */
struct page_frame
{
	p_addr_t addr;

	struct page_frame* prev;
	struct page_frame* next;
};


/*
 * list
 */
struct page_frame_list
{
	struct page_frame* root;
	size_t size;
};


static inline p_addr_t page_frame_align_inf(p_addr_t addr)
{
	return (addr & ~(PAGE_SIZE - 1));
}

static inline p_addr_t page_frame_align_sup(p_addr_t addr)
{
	return (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

void memory_init(size_t ram_size_bytes);
p_addr_t memory_page_frame_alloc(void);
int memory_page_frame_free(p_addr_t addr);
void memory_statistics(unsigned int* nb_used_page_frames,
		unsigned int* nb_free_page_frames);

#endif
