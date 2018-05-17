#include <kernel/errno.h>
#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <kernel/log.h>
#include <kernel/mm/memory.h>
#include <kernel/sched/sched.h>
#include <kernel/mm/vmm.h>
#include <libk/libk.h>


/** kernel mappings list */
static LIST_DEFINE(kernel_mappings);

/** arch specific implementation of the vmm interface */
static struct vmm_interface* vmm_impl = NULL;

/** created vmm list */
static LIST_DEFINE(vmm_list);

/** the vmm context we are currently running on */
static struct vmm* current_vmm = NULL;

int vmm_interface_register(struct vmm_interface* impl)
{
	if (vmm_impl)
		return -EEXIST;

	vmm_impl = impl;

	return 0;
}

struct vmm* vmm_get_current_vmm(void)
{
	return current_vmm;
}

void vmm_switch_to(struct vmm* vmm)
{
	if (vmm != current_vmm) {
		log_printf("%s(): switching from %p to %p\n", __func__,
				   (void*)current_vmm, (void*)vmm);

		if (current_vmm)
			vmm_unref(current_vmm);

		current_vmm = vmm;
		vmm_ref(vmm);

		vmm_impl->switch_to(vmm);
	}
}


static mapping_t* find_mapping(list_t* mappings, v_addr_t addr)
{
	list_node_t* it;

	list_foreach(mappings, it) {
		mapping_t* m = list_entry(it, mapping_t, m_list);

		if (mapping_contains_addr(m, addr))
			return m;
	}

	return NULL;
}

static void add_mapping(list_t* mappings, mapping_t* mapping)
{
	list_node_t* it;

	list_foreach(mappings, it) {
		mapping_t* m = list_entry(it, mapping_t, m_list);

		if (m->start > mapping->start) {
			list_insert_after(it, &mapping->m_list);
			return;
		}
	}

	list_push_back(mappings, &mapping->m_list);
}

int vmm_create(struct vmm** result)
{
	int err;
	struct vmm* vmm;

	err = vmm_impl->create(result);
	if (err)
		return err;

	vmm = *result;

	list_init(&vmm->mappings);
	refcount_init(&vmm->refcnt);

	list_push_back(&vmm_list, &vmm->vmm_list_node);

	return 0;
}

static int vmm_destroy_mapping(mapping_t* mapping)
{
	int err;

	err = vmm_impl->destroy_mapping(mapping);
	if (!err)
		mapping_destroy(mapping);

	return err;
}

static void vmm_destroy_mappings(list_t* mappings)
{
	list_node_t* it;
	list_node_t* next;

	list_foreach_safe(mappings, it, next) {
		mapping_t* mapping = list_entry(it, mapping_t, m_list);
		list_erase(it);

		vmm_destroy_mapping(mapping);
	}

}

static void vmm_destroy(struct vmm* vmm)
{
	log_e_printf("VMM_DESTROY: %p\n", (void*)vmm);
	vmm_destroy_mappings(&vmm->mappings);
	vmm_impl->destroy(vmm);

	list_erase(&vmm->vmm_list_node);
}

void vmm_ref(struct vmm* vmm)
{
	refcount_inc(&vmm->refcnt);
}

void vmm_unref(struct vmm* vmm)
{
	if (refcount_dec(&vmm->refcnt) == 0)
		vmm_destroy(vmm);
}

static inline struct vmm* get_current_vmm(void)
{
	const struct thread* thr = sched_get_current_thread();
	return (thr) ? thr->process->vmm : NULL;
}

int vmm_sync_kernel_space(void* data)
{
	struct vmm* current = get_current_vmm();
	list_node_t* it;

	if (!current)
		return 0;

	list_foreach(&vmm_list, it) {
		struct vmm* vmm = list_entry(it, struct vmm, vmm_list_node);
		if (current != vmm) {
			int err = vmm_impl->sync_kernel_space(vmm, data);
			if (err) {
				log_e_printf("%s: failed to sync kernel mapping (data=%p),"
							 "vmm %p\n", __func__, (void*)data, (void*)vmm);
				PANIC("sync kernel space");
			}
		}
	}

	return 0;
}

static int vmm_create_mapping(v_addr_t start, size_t size, int prot, int flags,
							  mapping_t** result)
{
	mapping_t* mapping;
	int err;

	if (!is_aligned(start, PAGE_SIZE))
		return -EINVAL;

	err = mapping_create(start, size, prot, flags, &mapping);
	if (err)
		return err;

	err = vmm_impl->create_mapping(mapping);
	if (err) {
		mapping_destroy(mapping);
		mapping = NULL;
	}

	*result = mapping;

	return err;
}

static int vmm_find_and_destroy_mapping(list_t* mappings, v_addr_t addr)
{
	mapping_t* mapping;
	int err;

	mapping = find_mapping(mappings, addr);
	if (!mapping)
		return -EINVAL;

	err = vmm_destroy_mapping(mapping);
	if (!err)
		list_erase(&mapping->m_list);

	return err;
}

int vmm_setup_kernel_mapping(mapping_t* mapping)
{
	int err;

	err =  vmm_impl->create_mapping(mapping);
	if (!err)
		add_mapping(&kernel_mappings, mapping);

	return err;
}

int vmm_create_kernel_mapping(v_addr_t start, size_t size, int prot)
{
	mapping_t* mapping;
	int err;

	if (vmm_impl->is_userspace_address(start))
		return -EINVAL;

	if (start + size < start)
		return -EOVERFLOW;

	err = vmm_create_mapping(start, size, prot & ~VMM_PROT_USER, 0, &mapping);
	if (!err)
		add_mapping(&kernel_mappings, mapping);

	return err;
}

int vmm_destroy_kernel_mapping(v_addr_t addr)
{
	if (vmm_is_userspace_address(addr))
		return -EINVAL;

	return vmm_find_and_destroy_mapping(&kernel_mappings, addr);
}

int vmm_clone_current(struct vmm* clone)
{
	struct vmm* current = get_current_vmm();
	list_node_t* it;
	int err;

	kassert(list_empty(&clone->mappings));

	list_foreach(&current->mappings, it) {
		const mapping_t* src = list_entry(it, mapping_t, m_list);
		mapping_t* cpy;
		err = mapping_copy_create(src, &cpy);
		if (err) // FIXME
			return err;

		list_push_back(&clone->mappings, &cpy->m_list);
	}

	err = vmm_impl->clone_current(clone);
	if (err) {
	}

	return err;
}

bool vmm_is_userspace_address(v_addr_t addr)
{
	return vmm_impl->is_userspace_address(addr);
}

static inline bool range_in_userspace(v_addr_t start, size_t size)
{
	return (vmm_is_userspace_address(start) &&
			vmm_is_userspace_address(start + size - 1));
}

int vmm_create_user_mapping(v_addr_t addr, size_t size, int prot, int flags)
{
	struct vmm* current = get_current_vmm();
	mapping_t* mapping;
	int err;

	size = page_align_up(size);
	if (!range_in_userspace(addr, size))
		return -EINVAL;

	err = vmm_create_mapping(addr, size, prot | VMM_PROT_USER, flags, &mapping);
	if (!err)
		add_mapping(&current->mappings, mapping);

	return err;
}

int vmm_destroy_user_mapping(v_addr_t addr)
{
	struct vmm* current = get_current_vmm();

	if (!vmm_is_userspace_address(addr))
		return -EINVAL;

	return vmm_find_and_destroy_mapping(&current->mappings, addr);
}

int vmm_extend_user_mapping(v_addr_t addr, size_t increment)
{
	struct vmm* current = get_current_vmm();
	mapping_t* mapping = NULL;
	v_addr_t ext_start;
	int err;

	if (!vmm_is_userspace_address(addr))
		return -EINVAL;

	mapping = find_mapping(&current->mappings, addr);
	if (!mapping)
		return -EINVAL;

	increment = page_align_up(increment);

	if (mapping->flags & VMM_MAP_GROWSDOW) {
		ext_start = mapping->start - increment;
	}
	else {
		ext_start = mapping->start + mapping->size;
	}

	if (!vmm_range_is_free(ext_start, ext_start + increment))
		return -EINVAL;

	err = vmm_create_user_mapping(ext_start, increment, mapping->region->prot,
								  mapping->flags);

	return err;
}

bool vmm_range_is_free(v_addr_t addr, size_t size)
{
	// TODO: implement
	return false;
}

static int handle_cow_fault(mapping_t* mapping, int flags)
{
	region_t* copy;
	int err;

	if (region_get_ref(mapping->region) == 1) {
		return vmm_impl->update_mapping_prot(mapping);
	}

	err = region_create(mapping_size_in_pages(mapping),
						mapping->region->prot, &copy);
	if (err)
		return err;

	err = vmm_impl->copy_mapping_pages(mapping, copy);
	if (err) {
		region_unref(copy);
		return err;
	}

	err = vmm_impl->destroy_mapping(mapping);
	if (err) {
		region_unref(copy);
		return err;
	}

	region_unref(mapping->region);
	mapping->region = copy;

	err = vmm_impl->create_mapping(mapping);
	if (err) {
		list_erase(&mapping->m_list);
		mapping_destroy(mapping);
	}

	return err;
}

void vmm_handle_page_fault(v_addr_t fault_addr, int flags)
{
	log_e_printf("#PF: %p (flags=%p)\n", (void*)fault_addr,
				 (void*)(v_addr_t)flags);

	struct vmm* current = get_current_vmm();
	mapping_t* mapping = find_mapping(&current->mappings, fault_addr);

	if (!mapping) {
		// sigsegv
		PANIC("page fault");
	}

	if (flags & VMM_FAULT_NOT_PRESENT) {
		if (mapping)
			PANIC("fault not present for present mapping");

		//sigsegv
		PANIC("page fault1");
	}

	if (flags & VMM_FAULT_WRITE) {
		if (mapping->region->prot & VMM_PROT_WRITE) {
			handle_cow_fault(mapping, flags);
		}
		else {
			// sigsegv
			PANIC("page fault2");
		}
	}

	/* PANIC("default"); */

}
