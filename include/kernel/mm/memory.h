#ifndef _KERNEL_MM_MEMORY_H_
#define _KERNEL_MM_MEMORY_H_

#include <kernel/types.h>
#include <libk/list.h>
#include <libk/utils.h>

typedef struct memory_area
{
	p_addr_t start;
	size_t size;

#ifndef NDEBUG
	char* label;
#endif
	v_addr_t vaddr;
} mem_area_t;

#ifdef NDEBUG
# define MEMORY_AREA(s, sz, va, l) { .start = (s), .size = (sz), .vaddr = (va)}
#else
# define MEMORY_AREA(s, sz, va, l) { .start = (s), .size = (sz), .vaddr = (va), .label = l }
#endif

static inline void mem_area_init(mem_area_t* m, p_addr_t start, size_t size,
				 char* label)
{
	m->start = start;
	m->size = size;
#ifndef NDEBUG
	m->label = label;
#endif
}

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

static inline bool page_is_aligned(p_addr_t addr)
{
	return is_aligned(addr, PAGE_SIZE);
}

void __memory_early_init(void);

void memory_init(size_t ram_size_bytes, const mem_area_t* mem_layout,
		 size_t layout_len);

p_addr_t memory_page_frame_alloc(void);

int memory_page_frame_free(p_addr_t addr);

void memory_statistics(unsigned int* nb_used_page_frames,
		       unsigned int* nb_free_page_frames);

void memory_layout_init(mem_area_t* layout, size_t n);

p_addr_t memory_get_memory_base(void);

p_addr_t memory_get_memory_top(void);

#endif
