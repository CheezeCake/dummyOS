#include <kernel/errno.h>
#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <kernel/log.h>
#include <kernel/mm/memory.h>
#include <kernel/sched/sched.h>
#include <kernel/mm/vmm.h>
#include <libk/libk.h>


// TODO: store kernel mappings

static struct vmm_interface* interface = NULL;

static LIST_DEFINE(vmm_list);


int vmm_interface_register(struct vmm_interface* vmm_interface)
{
	if (interface)
		return -EEXIST;

	interface = vmm_interface;

	return 0;
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

static int __vmm_destroy_user_mapping(mapping_t* mapping)
{
	int err;

	err = interface->destroy_mapping(mapping->start, mapping->size);
	mapping_destroy(mapping);

	return err;
}

static void vmm_destroy_mappings(struct vmm* vmm)
{
	list_node_t* it;
	list_node_t* next;

	list_foreach_safe(&vmm->mappings, it, next) {
		mapping_t* mapping = list_entry(it, mapping_t, m_list);
		list_erase(&vmm->mappings, it);

		__vmm_destroy_user_mapping(mapping);
	}

}

static void vmm_destroy(struct vmm* vmm)
{
	vmm_destroy_mappings(vmm);
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

int vmm_create_kernel_mapping(v_addr_t start, size_t size, int prot)
{
	int err = 0;

	if (interface->is_userspace_address(start) || !is_aligned(start, PAGE_SIZE))
		return -EINVAL;

	if (start + size < start)
		return -EOVERFLOW;

	for (v_addr_t addr = start; addr < start + size; addr += PAGE_SIZE) {
		p_addr_t frame = memory_page_frame_alloc();

		if (!frame)
			err = -ENOMEM;
		else
			err = interface->map_page(frame, addr, prot & ~VMM_PROT_USER);

		if (err) {
			kassert(vmm_destroy_kernel_mapping(start, addr - start) == 0);
			return err;
		}
	}

	return 0;
}

int vmm_destroy_kernel_mapping(v_addr_t start, size_t size)
{
	// TODO: free page frames
	return interface->destroy_mapping(start, size);
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

static mapping_t* find_mapping(struct vmm* vmm, v_addr_t addr)
{
	list_node_t* it;

	list_foreach(&vmm->mappings, it) {
		mapping_t* m = list_entry(it, mapping_t, m_list);

		if (mapping_contains_addr(m, addr))
			return m;
	}

	return NULL;
}

static void add_mapping(struct vmm* vmm, mapping_t* mapping)
{
	list_node_t* it;

	list_foreach(&vmm->mappings, it) {
		mapping_t* m = list_entry(it, mapping_t, m_list);

		if (m->start > mapping->start) {
			list_insert_after(it, &mapping->m_list);
			return;
		}
	}

	list_push_back(&vmm->mappings, &mapping->m_list);
}

int vmm_create_user_mapping(v_addr_t addr, size_t size, int prot, int flags)
{
	struct vmm* current = get_current_vmm();
	mapping_t* mapping;
	int err;

	size = page_align_up(size);
	if (!range_in_userspace(addr, size) ||
		!is_aligned(addr, PAGE_SIZE))
		return -EINVAL;

	err = mapping_create(addr, size, prot | VMM_PROT_USER, flags, &mapping);
	if (err)
		return err;

	err = interface->create_mapping(mapping);
	if (err)
		mapping_destroy(mapping);
	else
		add_mapping(current, mapping);

	return err;
}

int vmm_destroy_user_mapping(v_addr_t addr)
{
	mapping_t* mapping = NULL;
	int err;

	if (!vmm_is_userspace_address(addr))
		return -EINVAL;

	mapping = find_mapping(get_current_vmm(), addr);
	if (mapping)
		err = __vmm_destroy_user_mapping(mapping);
	else
		err = -EINVAL;

	return err;
}

int vmm_extend_user_mapping(v_addr_t addr, size_t increment)
{
	mapping_t* mapping = NULL;
	v_addr_t ext_start;
	int err;

	if (!vmm_is_userspace_address(addr))
		return -EINVAL;

	mapping = find_mapping(get_current_vmm(), addr);
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
