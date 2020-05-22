#include <dummyos/errno.h>
#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <kernel/log.h>
#include <kernel/mm/memory.h>
#include <kernel/mm/vmm.h>
#include <kernel/sched/sched.h>
#include <kernel/sched/sched.h>
#include <libk/libk.h>


/** arch specific implementation of the vmm interface */
static const struct vmm_interface* vmm_impl = NULL;

/** created vmm list */
static LIST_DEFINE(vmm_list);

/** the vmm context we are currently running on */
static struct vmm* current_vmm = NULL;

v_addr_t __fixup_addr = 0;

int vmm_interface_register(const struct vmm_interface* impl)
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


static mapping_t* find_mapping(const list_t* mappings, v_addr_t addr)
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

static void vmm_destroy_mappings(list_t* mappings)
{
	list_node_t* it;
	list_node_t* next;

	list_foreach_safe(mappings, it, next) {
		mapping_t* mapping = list_entry(it, mapping_t, m_list);
		list_erase(it);

		mapping_destroy(mapping);
	}

}

static void vmm_destroy(struct vmm* vmm)
{
	log_i_printf("VMM_DESTROY: %p\n", (void*)vmm);
	vmm_destroy_mappings(&vmm->mappings);
	list_erase(&vmm->vmm_list_node);
	vmm_impl->destroy(vmm);

	memset(vmm, 0, sizeof(struct vmm));
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

void vmm_uaccess_setup(void)
{
	const struct thread* thread = sched_get_current_thread();
	if (thread->type == UTHREAD)
		vmm_switch_to(thread->process->vmm);
}

int vmm_sync_kernel_space(void* data)
{
	list_node_t* it;

	list_foreach(&vmm_list, it) {
		struct vmm* vmm = list_entry(it, struct vmm, vmm_list_node);
		if (current_vmm != vmm) {
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

static int __unmap_mapping(v_addr_t addr, size_t nr_pages)
{
	int err = 0;

	for (size_t i = 0; !err && i < nr_pages; ++i, addr += PAGE_SIZE) {
		if (vmm_is_userspace_address(addr))
			err = vmm_impl->unmap_user_page(addr);
		else
			err = vmm_impl->unmap_kernel_page(addr);
	}

	return err;
}

static int unmap_mapping(const mapping_t* mapping)
{
	return __unmap_mapping(mapping->start, mapping_size_in_pages(mapping));
}

static int vmm_destroy_mapping(mapping_t* mapping)
{
	int err;

	err = unmap_mapping(mapping);
	if (!err) // FIXME: allways?
		mapping_destroy(mapping);

	return err;
}

static int map_mapping(const mapping_t* mapping,
		       int (* const map_page)(p_addr_t, v_addr_t, int))
{
	v_addr_t addr = mapping->start;
	size_t nr_pages = mapping_size_in_pages(mapping);

	for (size_t i = 0; i < nr_pages; ++i, addr += PAGE_SIZE) {
		int err = map_page(mapping->region->frames[i], addr,
				   mapping->region->prot);
		if (err) {
			__unmap_mapping(mapping->start, i);
			return err;
		}
	}

	return 0;
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

static inline bool is_within_physical_memory_bounds(p_addr_t start,
						    p_addr_t end)
{
	return (start >= memory_get_memory_base() &&
		end <= memory_get_memory_top());
}

int vmm_map_kernel_page(p_addr_t phys, v_addr_t virt, int prot)
{
	if (!page_is_aligned(phys) || !page_is_aligned(virt) ||
	    vmm_is_userspace_address(virt))
		return -EINVAL;

	return vmm_impl->map_kernel_page(phys, virt, prot);
}

int vmm_map_kernel_range(p_addr_t phys, v_addr_t virt, size_t size, int prot)
{
	size_t i = 0;
	size_t nr_pages = 0;
	int err = 0;

	size = page_align_up(size);
	nr_pages = size / PAGE_SIZE;

	while (i < nr_pages) {
		err = vmm_map_kernel_page(phys, virt, prot);
		if (err)
			break;
		i += 1;
		phys += PAGE_SIZE;
		virt += PAGE_SIZE;
	}

	if (err) {
		while (i--) {
			virt -= PAGE_SIZE;
			vmm_impl->unmap_kernel_page(virt); // FIXME: handle error
		}
	}

	return err;
}

int vmm_clone(struct vmm* vmm, struct vmm* clone)
{
	list_node_t* it;
	int err;

	kassert(list_empty(&clone->mappings));

	vmm_switch_to(vmm);

	list_foreach(&vmm->mappings, it) {
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
	return (addr < KERNEL_SPACE_START);
}

bool vmm_is_valid_userspace_address(v_addr_t addr)
{
	return (vmm_is_userspace_address(addr) &&
		find_mapping(&current_vmm->mappings, addr));
}

static inline bool range_in_userspace(v_addr_t start, size_t size)
{
	return (vmm_is_userspace_address(start) &&
		vmm_is_userspace_address(start + size - 1));
}

int vmm_create_user_mapping(v_addr_t addr, size_t size, int prot, int flags)
{
	mapping_t* mapping;
	int err;

	vmm_uaccess_setup();

	size = page_align_up(size);
	if (!range_in_userspace(addr, size) && !is_aligned(addr, PAGE_SIZE))
		return -EINVAL;

	err = mapping_create(addr, size, prot | VMM_PROT_USER, flags, &mapping);
	if (err)
		return err;

	err = map_mapping(mapping, vmm_impl->map_user_page);
	if (err)
		goto destroy_mapping;

	add_mapping(&current_vmm->mappings, mapping);

	return 0;

destroy_mapping:
	mapping_destroy(mapping);
	mapping = NULL;

	return err;
}

int vmm_destroy_user_mapping(v_addr_t addr)
{
	if (!vmm_is_userspace_address(addr))
		return -EINVAL;

	vmm_uaccess_setup();

	return vmm_find_and_destroy_mapping(&current_vmm->mappings, addr);
}

static int vmm_extend_user_mapping(v_addr_t addr, size_t increment)
{
	mapping_t* mapping = NULL;
	v_addr_t ext_start;
	ssize_t diff;
	int err;

	if (!vmm_is_userspace_address(addr))
		return -EINVAL;

	vmm_uaccess_setup();

	mapping = find_mapping(&current_vmm->mappings, addr);
	if (!mapping)
		return -EINVAL;

	if (mapping_contains_addr(mapping, addr + increment - 1))
		return 0;

	diff = addr + increment - mapping_get_end(mapping);
	increment = page_align_up(diff);

	if (mapping->flags & VMM_MAP_GROWSDOW) {
		ext_start = mapping->start - increment;
	}
	else {
		ext_start = mapping->start + mapping->size;
	}

	if (!vmm_range_is_free(ext_start, ext_start + increment - 1))
		return -EINVAL;

	err = vmm_create_user_mapping(ext_start, increment, mapping->region->prot,
				      mapping->flags);

	return err;
}

void* sys_sbrk(intptr_t increment)
{
	struct process* current = sched_get_current_process();
	v_addr_t brk = current->img.brk;
	const mapping_t* mapping;
	int err;

	vmm_uaccess_setup();

	mapping = find_mapping(&current_vmm->mappings, brk);
	if (!mapping) {
		err = vmm_create_user_mapping(brk, increment,
					      VMM_PROT_USER | VMM_PROT_WRITE, 0);
	}
	else {
		err = vmm_extend_user_mapping(brk, increment);
	}

	if (!err)
		current->img.brk = brk + increment;

	return (err) ? (void*)-1 : (void*)brk;
}

bool vmm_range_is_free(v_addr_t start, v_addr_t end)
{
	v_addr_t addr = start;

	while (addr < end) {
		mapping_t* mapping = find_mapping(&current_vmm->mappings, addr);
		if (mapping)
			return false;

		addr += PAGE_SIZE;
	}

	return true;
}

static int update_user_mapping_prot(const mapping_t* mapping, int prot)
{
	v_addr_t addr = mapping->start;
	size_t nr_pages = mapping_size_in_pages(mapping);

	for (size_t i = 0; i < nr_pages; ++i, addr += PAGE_SIZE) {
		int err = vmm_impl->update_user_page_prot(addr, prot);
		if (err)
			return err;
	}

	mapping->region->prot = prot;

	return 0;
}

int vmm_update_user_mapping_prot(v_addr_t addr, int prot)
{
	const mapping_t* mapping = find_mapping(&current_vmm->mappings, addr);
	if (!mapping)
		return -ENOENT;

	return update_user_mapping_prot(mapping, prot);
}

static int copy_mapping_pages(const mapping_t* mapping, region_t* copy)
{
	v_addr_t addr = mapping->start;
	size_t nr_pages = mapping_size_in_pages(mapping);

	if (nr_pages < copy->nr_frames)
		return -EINVAL;

	for (size_t i = 0; i < copy->nr_frames; ++i, addr += PAGE_SIZE) {
		int err = vmm_impl->copy_page(addr, copy->frames[i]);
		if (err) {
			__unmap_mapping(mapping->start, i);
			return err;
		}
	}

	return 0;
}

static int reset_user_mapping_prot(const mapping_t* mapping)
{
	return update_user_mapping_prot(mapping, mapping->region->prot);
}

static int handle_cow_fault(mapping_t* mapping, int flags)
{
	region_t* copy;
	int err;

	if (region_get_ref(mapping->region) == 1) {
		return reset_user_mapping_prot(mapping);
	}

	err = region_create(mapping_size_in_pages(mapping),
			    mapping->region->prot, &copy);
	if (err)
		return err;

	err = copy_mapping_pages(mapping, copy);
	if (err) {
		region_unref(copy);
		return err;
	}

	err = unmap_mapping(mapping);
	if (err) {
		region_unref(copy);
		return err;
	}

	region_unref(mapping->region);
	mapping->region = copy;

	err = map_mapping(mapping, vmm_impl->map_user_page);
	if (err) {
		list_erase(&mapping->m_list);
		mapping_destroy(mapping);
	}

	return err;
}

v_addr_t vmm_handle_page_fault(v_addr_t fault_addr, int flags)
{
	log_w_printf("#PF: %p (flags=%p)\n", (void*)fault_addr,
		     (void*)(v_addr_t)flags);

	if (flags & VMM_FAULT_USER) {
		mapping_t* mapping = find_mapping(&current_vmm->mappings, fault_addr);

		if (mapping &&
		    (flags & VMM_FAULT_WRITE) &&
		    (mapping->region->prot & VMM_PROT_WRITE) &&
		    !(flags & VMM_FAULT_ALIGNMENT))
		{
			kassert(handle_cow_fault(mapping, flags) == 0);
		}
		else {
			process_kill(sched_get_current_process()->pid, SIGSEGV);
			sched_yield();
			return fault_addr;
		}
	}
	else {
		if (__fixup_addr) {
			v_addr_t ret = __fixup_addr;
			__fixup_addr = 0;
			return ret;
		}

		PANIC("kernel page fault");
	}

	return 0;
}
