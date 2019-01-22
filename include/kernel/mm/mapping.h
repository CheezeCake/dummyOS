#ifndef _KERNEL_MM_MAPPING_H_
#define _KERNEL_MM_MAPPING_H_

#include <kernel/mm/memory.h>
#include <kernel/mm/region.h>
#include <libk/list.h>

typedef struct mapping
{
	int flags;

	v_addr_t start;
	size_t size;

	region_t* region;

	list_node_t m_list;
} mapping_t;

static inline size_t mapping_size_in_pages(const mapping_t* mapping)
{
	return (page_align_up(mapping->size) / PAGE_SIZE);
}

static inline bool mapping_contains_addr(const mapping_t* mapping, v_addr_t addr)
{
	return (addr >= mapping->start && addr < mapping->start + mapping->size);
}


static inline v_addr_t mapping_get_end(const mapping_t* mapping)
{
	return (mapping->start + mapping->size - 1);
}

int __mapping_init(mapping_t* mapping, region_t* region, v_addr_t start,
		   size_t size, int flags);

int mapping_create_from_range(v_addr_t mstart, p_addr_t pstart, size_t size,
			      int prot, int flags, mapping_t** result);

int mapping_create(v_addr_t start, size_t size, int prot, int flags,
		   mapping_t** result);

int mapping_copy_create(const mapping_t* src, mapping_t** result);

void mapping_destroy(mapping_t* mapping);

void mapping_reset(mapping_t* mapping);

#endif
