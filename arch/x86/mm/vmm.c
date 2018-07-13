#include <arch/vm.h>
#include <dummyos/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/types.h>
#include <kernel/mm/vmm.h>
#include "paging.h"

#include <kernel/log.h>

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

static void x86_vmm_reset(struct x86_vmm* x86vmm)
{
	paging_clear_userspace(x86vmm->cr3);
	memory_page_frame_free(x86vmm->cr3);
	x86vmm->cr3 = 0;
}

static void destroy(struct vmm* vmm)
{
	struct x86_vmm* x86vmm = get_x86_vmm(vmm);

	x86_vmm_reset(x86vmm);
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

	for (size_t i = 0; i < nr_pages; ++i, addr += PAGE_SIZE) {
		int err = paging_map(mapping->region->frames[i], addr,
						 mapping->region->prot);
		if (err)
			return __destroy_mapping(mapping->start, i);
	}

	return 0;
}

static int destroy_mapping(const mapping_t* mapping)
{
	return __destroy_mapping(mapping->start, mapping_size_in_pages(mapping));
}

static int copy_mapping_pages(const mapping_t* mapping, region_t* copy)
{
	v_addr_t addr = mapping->start;
	size_t nr_pages = mapping_size_in_pages(mapping);

	if (nr_pages < copy->nr_frames)
		return -EINVAL;

	for (size_t i = 0; i < copy->nr_frames; ++i, addr += PAGE_SIZE) {
		int err = paging_copy_page(addr, copy->frames[i]);
		if (err)
			return __destroy_mapping(mapping->start, i);
	}

	return 0;
}

static int update_mapping_prot(const mapping_t* mapping)
{
	v_addr_t addr = mapping->start;
	size_t nr_pages = mapping_size_in_pages(mapping);

	for (size_t i = 0; i < nr_pages; ++i, addr += PAGE_SIZE) {
		int err = paging_update_prot(addr, mapping->region->prot);
		if (err)
			return err;
	}

	return 0;
}

static const struct vmm_interface impl = {
	.create = create,
	.destroy = destroy,
	.switch_to = switch_to,
	.clone_current = clone_current,
	.sync_kernel_space = sync_kernel_space,

	.is_userspace_address = is_userspace_address,

	.create_mapping = create_mapping,
	.destroy_mapping = destroy_mapping,
	.copy_mapping_pages = copy_mapping_pages,
	.update_mapping_prot = update_mapping_prot,
};

int vmm_register(void)
{
	return vmm_interface_register(&impl);
}
