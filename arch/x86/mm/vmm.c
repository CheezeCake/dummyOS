#include <arch/vm.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/types.h>
#include <kernel/mm/vmm.h>
#include "paging.h"

struct x86_vmm
{
	p_addr_t cr3;

	struct vmm vmm;
};


static inline struct x86_vmm* get_x86_vmm(struct vmm* vmm)
{
	return container_of(vmm, struct x86_vmm, vmm);
}

static int create_pd_and_init_kernel(struct x86_vmm* x86vmm)
{
	p_addr_t pd;
	int err;

	pd = memory_page_frame_alloc();
	if (!pd)
		return -ENOMEM;

	err = paging_init_pd(pd);
	if (err)
		memory_page_frame_free(pd);
	else
		x86vmm->cr3 = pd;

	return err;
}

static int create(struct vmm** result)
{
	struct x86_vmm* x86vmm;
	int err;

	x86vmm = kmalloc(sizeof(struct x86_vmm));
	if (!x86vmm)
		return -ENOMEM;

	err = create_pd_and_init_kernel(x86vmm);
	if (err)
		kfree(x86vmm);
	else
		*result = &x86vmm->vmm;

	return err;
}

static void destroy(struct vmm* vmm)
{
	struct x86_vmm* x86vmm = get_x86_vmm(vmm);

	memory_page_frame_free(x86vmm->cr3);
	kfree(x86vmm);
}

static void switch_to(struct vmm* vmm)
{
	const struct x86_vmm* x86vmm = get_x86_vmm(vmm);

	paging_switch_cr3(x86vmm->cr3);
}

static int clone_current(struct vmm* clone)
{
	const struct x86_vmm* x86clone = get_x86_vmm(clone);

	paging_clone_current_cow(x86clone->cr3);

	return 0;
}

static int sync_kernel_space(struct vmm* vmm, void* data)
{
	const struct x86_vmm* x86vmm = get_x86_vmm(vmm);

	return paging_sync_kernel_space(x86vmm->cr3, data);
}

static bool is_userspace_address(v_addr_t addr)
{
	return (addr < KERNEL_SPACE_START);
}

static int map_page(p_addr_t frame, v_addr_t page, int prot)
{
	return paging_map(frame, page, prot);
}

/*
 * on current vmm
 */
static int __destroy_mapping(v_addr_t addr, size_t nr_pages)
{
	int err = 0;

	for (size_t i = 0; !err && i < nr_pages; ++i, addr += PAGE_SIZE)
		err = paging_unmap(addr);

	return err;
}

static int create_mapping(const mapping_t* mapping)
{
	v_addr_t addr = mapping->start;
	size_t nr_pages = mapping_size_in_pages(mapping);
	int err = 0;

	for (size_t i = 0;
		 !err && i < nr_pages;
		 ++i, addr += PAGE_SIZE)
	{
		err = paging_map(mapping->region->frames[i], addr,
						 mapping->region->prot);
		if (err)
			__destroy_mapping(mapping->start, i);
	}

	return err;
}

static int destroy_mapping(v_addr_t start, size_t size)
{
	return __destroy_mapping(start, size);
}

static struct vmm_interface impl = {
	.create = create,
	.destroy = destroy,
	.switch_to = switch_to,
	.clone_current = clone_current,
	.sync_kernel_space = sync_kernel_space,

	.is_userspace_address = is_userspace_address,

	.map_page = map_page,
	.create_mapping = create_mapping,
	.destroy_mapping = destroy_mapping,
};

int vmm_register(void)
{
	return vmm_interface_register(&impl);
}
