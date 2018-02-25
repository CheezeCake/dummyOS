#ifndef _ARCH_MEMORY_H_
#define _ARCH_MEMORY_H_

#include <kernel/types.h>
#include <kernel/page_frame_status.h>

#define PAGE_SIZE 0x1000 // 4kB
#define PAGE_SIZE_SHIFT 12

#define X86_MEMORY_HARDWARE_MAP_BEGIN 0xa0000
#define X86_MEMORY_HARDWARE_MAP_END 0x100000

static inline enum page_frame_status
get_page_frame_status(p_addr_t pf, p_addr_t mem_base, p_addr_t mem_top,
					  p_addr_t kbase, p_addr_t ktop)
{
	if (pf < mem_base)
		return PAGE_FRAME_RESERVED;
	else if (pf < X86_MEMORY_HARDWARE_MAP_BEGIN)
		return PAGE_FRAME_FREE;
	else if (pf < X86_MEMORY_HARDWARE_MAP_END)
		return PAGE_FRAME_HW_MAP;
	else if (pf < kbase)
		return PAGE_FRAME_FREE;
	else if (pf < ktop)
		return PAGE_FRAME_KERNEL;
	else
		return PAGE_FRAME_FREE;
}

#endif
