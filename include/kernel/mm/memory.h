#ifndef _KERNEL_MM_MEMORY_H_
#define _KERNEL_MM_MEMORY_H_

#include <kernel/page_frame_status.h>
#include <kernel/types.h>
#include <libk/utils.h>

static inline enum page_frame_status
get_page_frame_status(p_addr_t pf, p_addr_t mem_base, p_addr_t mem_top,
					  p_addr_t kbase, p_addr_t ktop);

/*
 * Include platform specific implementation.
 */
#include <arch/memory.h>

#ifndef PAGE_SIZE
# error "arch/memory.h must define PAGE_SIZE"
#endif
#ifndef PAGE_SIZE_SHIFT
# error "arch/memory.h must define PAGE_SIZE_SHIFT"
#endif

static inline p_addr_t page_align_down(p_addr_t addr)
{
	return align_down(addr, PAGE_SIZE);
}

static inline p_addr_t page_align_up(p_addr_t addr)
{
	return align_up(addr, PAGE_SIZE);
}

void memory_init(size_t ram_size_bytes);

p_addr_t memory_page_frame_alloc(void);

int memory_page_frame_free(p_addr_t addr);

void memory_statistics(unsigned int* nb_used_page_frames,
					   unsigned int* nb_free_page_frames);

#endif
