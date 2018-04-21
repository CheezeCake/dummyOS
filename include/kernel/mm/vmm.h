#ifndef _KERNEL_MM_VMM_H_
#define _KERNEL_MM_VMM_H_

#include <kernel/mm/mapping.h>
#include <kernel/mm/memory.h>
#include <kernel/mm/region.h>
#include <libk/list.h>
#include <libk/refcount.h>

/*
 * region protection flags
 */
#define VMM_PROT_READ		(1 << 0)
#define VMM_PROT_WRITE		(1 << 1)
#define VMM_PROT_EXEC		(1 << 2)
#define VMM_PROT_USER		(1 << 3)

/*
 * mapping flags
 */
#define VMM_MAP_GROWSDOW	(1 << 1)

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

	bool (*is_userspace_address)(v_addr_t addr);

	int (*create_mapping)(const mapping_t* mapping);
	int (*destroy_mapping)(v_addr_t start, size_t size);
};

int vmm_interface_register(struct vmm_interface* vmm_interface);


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

int vmm_clone_current(struct vmm* clone);

bool vmm_is_userspace_address(v_addr_t addr);

int vmm_setup_kernel_mapping(mapping_t* mapping);

/**
 * addr must be PAGE_SIZE aligned
 */
int vmm_create_kernel_mapping(v_addr_t start, size_t size, int prot);

int vmm_destroy_kernel_mapping(v_addr_t start);

int vmm_sync_kernel_space(void* data);

/**
 * addr must be PAGE_SIZE aligned
 */
int vmm_create_user_mapping(v_addr_t start, size_t size, int prot, int flags);

int vmm_destroy_user_mapping(v_addr_t addr);

int vmm_extend_user_mapping(v_addr_t addr, size_t increment);

bool vmm_range_free(v_addr_t start, size_t size);

void vmm_switch_to(struct vmm* vmm);

#endif
