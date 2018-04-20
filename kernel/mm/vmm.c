#include <kernel/errno.h>
#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <kernel/log.h>
#include <kernel/mm/memory.h>
#include <kernel/sched/sched.h>
#include <kernel/mm/vmm.h>
#include <libk/libk.h>


/** kernel mappings list */
list_t kernel_mappings;

static struct vmm_interface* interface = NULL;

static LIST_DEFINE(vmm_list);


int vmm_interface_register(struct vmm_interface* vmm_interface)
{
	if (interface)
		return -EEXIST;

	interface = vmm_interface;

	return 0;
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

	err = interface->create(result);
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

	err = interface->destroy_mapping(mapping->start, mapping->size);
	// XXX: allways destroy the mapping?
	mapping_destroy(mapping);

	return err;
}

static void vmm_destroy_mappings(list_t* mappings)
{
	list_node_t* it;
	list_node_t* next;

	list_foreach_safe(mappings, it, next) {
		mapping_t* mapping = list_entry(it, mapping_t, m_list);
		list_erase(mappings, it);

		vmm_destroy_mapping(mapping);
	}

}

static void vmm_destroy(struct vmm* vmm)
{
	vmm_destroy_mappings(&vmm->mappings);
	interface->destroy(vmm);

	list_erase(&vmm_list, &vmm->vmm_list_node);
}

void vmm_ref(struct vmm* vmm)
{
	refcount_inc(&vmm->refcnt);
	log_printf("%s, ref %p : %d\n", __func__, (void*)vmm,
			   refcount_get(&vmm->refcnt));
}

void vmm_unref(struct vmm* vmm)
{
	log_printf("%s, ref %p : %d\n", __func__, (void*)vmm,
			   refcount_get(&vmm->refcnt) - 1);
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
			int err = interface->sync_kernel_space(vmm, data);
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

	err = interface->create_mapping(mapping);
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
		list_erase(mappings, &mapping->m_list);

	return err;
}

int vmm_setup_initial_kheap_mapping(mapping_t* initial_kheap_mapping)
{
	return interface->create_mapping(initial_kheap_mapping);
}

int vmm_create_kernel_mapping(v_addr_t start, size_t size, int prot)
{
	mapping_t* mapping;
	int err;

	if (interface->is_userspace_address(start))
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

	err = interface->clone_current(clone);
	if (err) {
	}

	return err;
}

bool vmm_is_userspace_address(v_addr_t addr)
{
	return interface->is_userspace_address(addr);
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

	if (!vmm_range_free(ext_start, ext_start + increment))
		return -EINVAL;

	err = vmm_create_user_mapping(ext_start, increment, mapping->region->prot,
								  mapping->flags);

	return err;
}

bool vmm_range_free(v_addr_t addr, size_t size)
{
	return false;
}

void vmm_switch_to(struct vmm* vmm)
{
	log_printf("%s(): switching to %p\n", __func__, (void*)vmm);
	interface->switch_to(vmm);
}
