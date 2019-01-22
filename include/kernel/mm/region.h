#ifndef _KERNEL_MM_REGION_H_
#define _KERNEL_MM_REGION_H_

#include <kernel/types.h>
#include <libk/refcount.h>

typedef struct region region_t;
struct region
{
	int prot;

	p_addr_t* frames;
	size_t nr_frames;

	refcount_t refcnt;
};

int __region_init(region_t* region, p_addr_t* frames, size_t nr_frames,
		  int prot);

int region_create_from_range(p_addr_t start, size_t size, int prot,
			     region_t** result);

int region_create(size_t nr_frames, int prot, region_t** result);

void region_unref(region_t* region);

void region_ref(region_t* region);

int region_get_ref(const region_t* region);

#endif
