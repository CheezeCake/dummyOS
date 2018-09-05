#include <dummyos/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/mm/mapping.h>
#include <libk/libk.h>

void mapping_reset(mapping_t* mapping)
{
	region_unref(mapping->region);

	memset(mapping, 0, sizeof(mapping_t));
}

void mapping_destroy(mapping_t* mapping)
{
	mapping_reset(mapping);
	kfree(mapping);
}

static int mapping_init_from_range(mapping_t* mapping, v_addr_t mstart,
								   p_addr_t pstart, size_t size, int prot,
								   int flags)
{
	region_t* region;
	int err;

	err = region_create_from_range(pstart, size, prot, &region);
	if (err)
		return err;

	err = __mapping_init(mapping, region, mstart, size, flags);

	region_unref(region);

	return err;
}

int mapping_create_from_range(v_addr_t mstart, p_addr_t pstart, size_t size,
							  int prot, int flags, mapping_t** result)
{
	mapping_t* mapping;
	int err;

	mapping = kmalloc(sizeof(mapping_t));
	if (!mapping)
		return -ENOMEM;

	err = mapping_init_from_range(mapping, mstart, pstart, size, prot, flags);
	if (err) {
		kfree(mapping);
		mapping = NULL;
	}

	*result = mapping;

	return err;
}

int __mapping_init(mapping_t* mapping, region_t* region, v_addr_t start,
				   size_t size, int flags)
{
	if (start + size < start)
		return -EOVERFLOW;

	mapping->flags = flags;
	mapping->start = start;
	mapping->size = size;
	mapping->region = region;
	region_ref(region);
	list_node_init(&mapping->m_list);

	return 0;
}

static int mapping_init(mapping_t* mapping, v_addr_t start, size_t size,
						int prot, int flags)
{
	region_t* region;
	v_addr_t end = start + size;
	int err;

	if (end < start)
		return -EOVERFLOW;

	err = region_create((end - start) / PAGE_SIZE, prot, &region);
	if (!err) {
		err = __mapping_init(mapping, region, start, size, flags);
		region_unref(region);
	}

	return err;
}

int mapping_create(v_addr_t start, size_t size, int prot, int flags,
				   mapping_t** result)
{
	mapping_t* mapping;
	int err;

	mapping = kmalloc(sizeof(mapping_t));
	if (!mapping)
		return -ENOMEM;

	err = mapping_init(mapping, start, size, prot, flags);
	if (err) {
		kfree(mapping);
		mapping = NULL;
	}

	*result = mapping;

	return err;
}

int mapping_copy_create(const mapping_t* src, mapping_t** result)
{
	mapping_t* mapping;

	mapping = kmalloc(sizeof(mapping_t));
	if (!mapping)
		return -ENOMEM;

	memcpy(mapping, src, sizeof(mapping_t));

	region_ref(mapping->region);
	list_node_init(&mapping->m_list);

	*result = mapping;

	return 0;
}
