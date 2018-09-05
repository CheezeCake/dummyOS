#ifndef _KERNEL_MM_MEMORY_H_
#define _KERNEL_MM_MEMORY_H_

#include <kernel/types.h>
#include <libk/list.h>
#include <libk/utils.h>

typedef struct memory_area
{
	v_addr_t start;
	v_addr_t end;

#ifndef NDEBUG
	char* label;
#endif

	list_node_t m_layout;
} mem_area_t;

#ifdef NDEBUG
# define MEMORY_AREA(s, e, l) { .start = (s), .end = (e) - 1, }
#else
# define MEMORY_AREA(s, e, l) { .start = (s), .end = (e) - 1, .label = l }
#endif

static inline void mem_area_init(mem_area_t* m, v_addr_t start, v_addr_t end, char* label)
{
	m->start = start;
	m->end = end - 1;
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

void memory_init(size_t ram_size_bytes);

p_addr_t memory_page_frame_alloc(void);

int memory_page_frame_free(p_addr_t addr);

void memory_statistics(unsigned int* nb_used_page_frames,
					   unsigned int* nb_free_page_frames);

void memory_layout_init(mem_area_t* layout, size_t n);

p_addr_t memory_get_memory_base(void);

p_addr_t memory_get_memory_top(void);

#endif
