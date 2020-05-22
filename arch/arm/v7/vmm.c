#include <dummyos/errno.h>
#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <kernel/types.h>
#include <kernel/mm/vmm.h>
#include <libk/libk.h>
#include "paging.h"

struct armv7_vmm
{
	uint64_t* ttbr0_;
	uint64_t* ttbr0;
	p_addr_t ttbr0_phys;

	struct vmm vmm;
};

static inline struct armv7_vmm* get_armv7_vmm(struct vmm* vmm)
{
	return container_of(vmm, struct armv7_vmm, vmm);
}

static int create(struct vmm** result)
{
#define LV1_TABLE_SIZE 32
#define LV1_TABLE_ALIGNMENT 32
	struct armv7_vmm* v7vmm;

	v7vmm = kmalloc(sizeof(struct armv7_vmm));
	if (!v7vmm)
		return -ENOMEM;

	v7vmm->ttbr0_ = kmalloc(LV1_TABLE_SIZE + LV1_TABLE_ALIGNMENT - 1);
	if (!v7vmm->ttbr0_)
		goto fail;

	v7vmm->ttbr0 = (uint64_t*)align_up((v_addr_t)v7vmm->ttbr0_,
					   LV1_TABLE_ALIGNMENT);
	v7vmm->ttbr0_phys = paging_virt_to_phys((v_addr_t)v7vmm->ttbr0);

	memset(v7vmm->ttbr0, 0, LV1_TABLE_SIZE);

	*result = &v7vmm->vmm;

	return 0;

fail:
	kfree(v7vmm);

	return -ENOMEM;
}

static void armv7_vmm_reset(struct armv7_vmm* v7vmm)
{
	paging_clear_userspace(v7vmm->ttbr0); // FIXME: handle error

	kfree(v7vmm->ttbr0_);
	v7vmm->ttbr0_ = NULL;
	v7vmm->ttbr0 = NULL;
	v7vmm->ttbr0_phys = 0;
}

static void destroy(struct vmm* vmm)
{
	struct armv7_vmm* v7vmm = get_armv7_vmm(vmm);

	armv7_vmm_reset(v7vmm);
	kfree(v7vmm);
}

static void switch_to(struct vmm* vmm)
{
	const struct armv7_vmm* v7vmm = get_armv7_vmm(vmm);

	paging_switch_ttbr0(v7vmm->ttbr0_phys);
}

static int clone_current(struct vmm* clone)
{
	const struct armv7_vmm* v7clone = get_armv7_vmm(clone);
	struct vmm* vmm = vmm_get_current_vmm();
	const struct armv7_vmm* v7vmm = get_armv7_vmm(vmm);

	return paging_clone_current_cow(v7clone->ttbr0, v7vmm->ttbr0);
}

static int sync_kernel_space(struct vmm* vmm, void* data)
{
	return 0;
}

static int update_user_prot(v_addr_t virt, int prot)
{
	struct vmm* vmm = vmm_get_current_vmm();
	const struct armv7_vmm* v7vmm = get_armv7_vmm(vmm);

	return paging_update_user_prot(virt, prot, v7vmm->ttbr0);
}

static int map_user_page(p_addr_t phys, v_addr_t virt, int prot)
{
	struct vmm* vmm = vmm_get_current_vmm();
	const struct armv7_vmm* v7vmm = get_armv7_vmm(vmm);

	return paging_user_map(phys, virt, prot, v7vmm->ttbr0);
}

static int unmap_user_page(v_addr_t virt)
{
	struct vmm* vmm = vmm_get_current_vmm();
	const struct armv7_vmm* v7vmm = get_armv7_vmm(vmm);

	return paging_user_unmap(virt, v7vmm->ttbr0);
}

static const struct vmm_interface impl = {
	.create			= create,
	.destroy		= destroy,
	.switch_to		= switch_to,
	.clone_current		= clone_current,
	.sync_kernel_space	= sync_kernel_space,

	.copy_page		= paging_copy_page,
	.update_kernel_page_prot	= paging_update_kernel_prot,
	.map_kernel_page		= paging_kernel_map,
	.unmap_kernel_page		= paging_kernel_unmap,
	.update_user_page_prot		= update_user_prot,
	.map_user_page			= map_user_page,
	.unmap_user_page		= unmap_user_page,
};

int arm_vmm_register(void)
{
	return vmm_interface_register(&impl);
}
