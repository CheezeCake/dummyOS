#ifndef _KERNEL_MM_VMM_H_
#define _KERNEL_MM_VMM_H_

#include <kernel/mm/mapping.h>
#include <kernel/mm/memory.h>
#include <kernel/mm/region.h>
#include <kernel/mm/vm.h>
#include <libk/bits.h>
#include <libk/list.h>
#include <libk/refcount.h>

/*
 * region protection flags
 */
#define VMM_PROT_WRITE		BIT(0)
#define VMM_PROT_EXEC		BIT(1)
#define VMM_PROT_USER		BIT(2)
#define VMM_PROT_NOCACHE	BIT(3)

/*
 * mapping flags
 */
#define VMM_MAP_GROWSDOW	BIT(1)

/*
 * fault flags
 */
#define VMM_FAULT_USER		BIT(0)
#define VMM_FAULT_WRITE		BIT(1)
#define VMM_FAULT_NOT_PRESENT	BIT(2)
#define VMM_FAULT_ALIGNMENT	BIT(3)

/**
 * @brief Virtual Memory Manager
 */
struct vmm
{
	list_t mappings;

	refcount_t refcnt;

	list_node_t vmm_list_node;
};

struct vmm_interface
{
	int (*create)(struct vmm** result);
	void (*destroy)(struct vmm* vmm);
	void (*switch_to)(struct vmm* vmm);
	int (*clone_current)(struct vmm* clone);
	int (*sync_kernel_space)(struct vmm* vmm, void* data);


	int (*copy_page)(v_addr_t src, p_addr_t dst);
	int (*update_kernel_page_prot)(v_addr_t addr, int prot);
	int (*map_kernel_page)(p_addr_t phys, v_addr_t virt, int prot);
	int (*unmap_kernel_page)(v_addr_t virt);
	int (*update_user_page_prot)(v_addr_t addr, int prot);
	int (*map_user_page)(p_addr_t phys, v_addr_t virt, int prot);
	int (*unmap_user_page)(v_addr_t virt);
};

int vmm_interface_register(const struct vmm_interface* vmm_interface);


/**
 * @brief Creates a vmm object
 */
int vmm_create(struct vmm** result);

/**
 * @brief Increments the reference counter by one.
 */
void vmm_ref(struct vmm* vmm);

/**
 * @brief Decrements the reference counter by one.
 */
void vmm_unref(struct vmm* vmm);

int vmm_clone(struct vmm* vmm, struct vmm* clone);

bool vmm_is_userspace_address(v_addr_t addr);

bool vmm_is_valid_userspace_address(v_addr_t addr);

int vmm_map_kernel_page(p_addr_t phys, v_addr_t virt, int prot);

int vmm_map_kernel_range(p_addr_t phys, v_addr_t virt, size_t size, int prot);

int vmm_sync_kernel_space(void* data);

/**
 * addr must be PAGE_SIZE aligned
 */
int vmm_create_user_mapping(v_addr_t start, size_t size, int prot, int flags);

int vmm_destroy_user_mapping(v_addr_t addr);

int vmm_update_user_mapping_prot(v_addr_t addr, int prot);

bool vmm_range_is_free(v_addr_t start, v_addr_t end);

struct vmm* vmm_get_current_vmm(void);

void vmm_switch_to(struct vmm* vmm);

void vmm_uaccess_setup(void);

v_addr_t vmm_handle_page_fault(v_addr_t fault_addr, int flags);

#endif
