#include <dummyos/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/types.h>
#include <kernel/mm/vmm.h>
#include "paging.h"
#include "vm.h"

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

	return paging_clone_current_cow(x86clone->cr3);
}

static int sync_kernel_space(struct vmm* vmm, void* data)
{
	const struct x86_vmm* x86vmm = get_x86_vmm(vmm);

	return paging_sync_kernel_space(x86vmm->cr3, data);
}

static const struct vmm_interface impl = {
	.create				= create,
	.destroy			= destroy,
	.switch_to			= switch_to,
	.clone_current			= clone_current,
	.sync_kernel_space		= sync_kernel_space,

	.copy_page			= paging_copy_page,
	.update_kernel_page_prot	= paging_update_prot,
	.map_kernel_page		= paging_map,
	.unmap_kernel_page		= paging_unmap,
	.update_user_page_prot		= paging_update_prot,
	.map_user_page			= paging_map,
	.unmap_user_page		= paging_unmap,
};

int x86_vmm_register(void)
{
	return vmm_interface_register(&impl);
}
