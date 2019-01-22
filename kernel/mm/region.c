#include <dummyos/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/mm/memory.h>
#include <kernel/mm/region.h>
#include <libk/libk.h>

static void region_reset(region_t* region)
{
	for (size_t i = 0; i < region->nr_frames; ++i) {
		if (region->frames[i])
			memory_page_frame_free(region->frames[i]);
	}

	kfree(region->frames);

	memset(region, 0, sizeof(region_t));
}

static void region_destroy(region_t* region)
{
	region_reset(region);
	kfree(region);
}

void region_unref(region_t* region)
{
	if (refcount_dec(&region->refcnt) == 0)
		region_destroy(region);
}

void region_ref(region_t* region)
{
	refcount_inc(&region->refcnt);
}

int region_get_ref(const region_t* region)
{
	return refcount_get(&region->refcnt);
}

static void region_init_from_frames(region_t* region, p_addr_t* frames,
				    size_t nr_frames, int prot)
{
	region->frames = frames;
	region->nr_frames = nr_frames;
	region->prot = prot;
	refcount_init(&region->refcnt);
}

static int region_init_from_range(region_t* region, p_addr_t start, size_t size,
				  int prot)
{
	p_addr_t* frames;
	size_t nr_frames = page_align_up(size) / PAGE_SIZE;

	frames = kcalloc(nr_frames, sizeof(p_addr_t));
	if (!frames)
		return -ENOMEM;

	p_addr_t addr = start;
	for (size_t i = 0; i < nr_frames; ++i, addr += PAGE_SIZE)
		frames[i] = addr;

	region_init_from_frames(region, frames, nr_frames, prot);

	return 0;
}

int region_create_from_range(p_addr_t start, size_t size, int prot,
			     region_t** result)
{
	region_t* region;
	int err;

	region = kmalloc(sizeof(region_t));
	if (!region)
		return -ENOMEM;

	err = region_init_from_range(region, start, size, prot);
	if (err) {
		kfree(region);
		region = NULL;
	}

	*result = region;

	return err;
}

int __region_init(region_t* region, p_addr_t* frames, size_t nr_frames,
		  int prot)
{
	bool frames_ok = true;

	for (size_t i = 0; frames_ok && i < nr_frames; ++i) {
		frames[i] = memory_page_frame_alloc();
		if (!frames[i])
			frames_ok = false;
	}

	region_init_from_frames(region, frames, nr_frames, prot);

	if (!frames_ok) {
		region_reset(region);
		return -ENOMEM;
	}

	return 0;
}

static int region_init(region_t* region, size_t nr_frames, int prot)
{
	p_addr_t* frames;

	frames = kcalloc(nr_frames, sizeof(p_addr_t));
	if (!frames)
		return -ENOMEM;

	return __region_init(region, frames, nr_frames, prot);
}

int region_create(size_t nr_frames, int prot, region_t** result)
{
	region_t* region;
	int err;

	region = kmalloc(sizeof(region_t));
	if (!region)
		return -ENOMEM;

	err = region_init(region, nr_frames, prot);
	if (err) {
		kfree(region);
		region = NULL;
	}

	*result = region;

	return err;
}
