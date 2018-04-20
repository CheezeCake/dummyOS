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

	region_t* parent;
	refcount_t refcnt;
};

int __region_init(region_t* region, p_addr_t* frames, size_t nr_frames,
				  int prot);

int region_create(size_t nr_frames, int prot, region_t** result);

void region_unref(region_t* region);

void region_ref(region_t* region);

#endif
