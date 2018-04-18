#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/mm/mapping.h>
#include <libk/libk.h>

static void mapping_reset(mapping_t* mapping)
{
	region_unref(mapping->region);

	memset(mapping, 0, sizeof(mapping_t));
}

void mapping_destroy(mapping_t* mapping)
{
	mapping_reset(mapping);
	kfree(mapping);
}

static int mapping_init(mapping_t* mapping, v_addr_t start, size_t size,
						int prot)
{
	region_t* region;
	v_addr_t end = start + size;
	int err;

	if (end < start)
		return -EOVERFLOW;

	err = region_create((end - start) / PAGE_SIZE, prot, &region);
	if (!err) {
		mapping->start = start;
		mapping->size = size;
		mapping->region = region;
		list_node_init(&mapping->m_list);
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

	err = mapping_init(mapping, start, size, prot);
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
