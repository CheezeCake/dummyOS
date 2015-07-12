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
	unsigned int refs;

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


// these symbols are defined in the linker script kernel.ld
extern uint8_t __begin_kernel;
extern uint8_t __end_kernel;

inline p_addr_t page_frame_align_inf(p_addr_t addr)
{
	return (addr & ~(PAGE_SIZE - 1));
}

inline p_addr_t page_frame_align_sup(p_addr_t addr)
{
	return (addr + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
}

inline p_addr_t get_kernel_base_page_frame(void)
{
	return page_frame_align_inf((p_addr_t)&__begin_kernel);
}

inline p_addr_t get_kernel_top_page_frame(size_t page_frames_in_ram)
{
	// __end_kernel + page_frame descriptors
	return page_frame_align_sup((p_addr_t)&__end_kernel +
			(page_frames_in_ram * sizeof(struct page_frame)));
}


void memory_init(size_t ram_size_bytes);
p_addr_t memory_page_frame_alloc(void);
int memory_ref_page_frame(p_addr_t addr);
int memory_unref_page_frame(p_addr_t addr);
void memory_statistics(unsigned int* nb_used_page_frames,
		unsigned int* nb_free_page_frames);

#endif
