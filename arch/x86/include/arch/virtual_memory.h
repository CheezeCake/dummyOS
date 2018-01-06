#ifndef _ARCH_VIRTUAL_MEMORY_H_
#define _ARCH_VIRTUAL_MEMORY_H_

#include <stdbool.h>

#include <arch/memory.h>

/*
 * +----------------------+ 4GB
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |  user address space  |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * |                      |
 * +---+------------------+ 1GB (0x40000000)
 * |   |                  |
 * |   |  mirroring zone  |
 * |   |                  |
 * |   +------------------+ 1GB - 4MB (0x3fc00000)
 * |   |  reserved page   |
 * |   +------------------+ 1GB - 4MB - 4kB (0x3fbff000)
 * |                      |
 * | kernel address space |
 * |                      |
 * |                      |
 * +----------------------+
 */


/*
 * address space sizes
 */
#define VADDR_SPACE_SIZE 0xffffffff // 4GB
#define KERNEL_VADDR_SPACE_SIZE 0x40000000 // 1GB
#define USER_VADDR_SPACE_SIZE 0xC0000000 // 3GB

/*
 * limits
 */
#define KERNEL_VADDR_SPACE_TOP 0x40000000 // 1GB
#define MIRRORING_VADDR_BEGIN (KERNEL_VADDR_SPACE_TOP - 0x400000) // 1GB - 4MB
#define KERNEL_VADDR_SPACE_LIMIT (MIRRORING_VADDR_BEGIN - PAGE_SIZE) // 1GB - 4MB - 4kB
#define KERNEL_VADDR_SPACE_RESERVED KERNEL_VADDR_SPACE_LIMIT

#define USER_VADDR_SPACE_START	0x40000000 // 1GB
#define USER_VADDR_SPACE_END	0xffffffff // 4GB

/*
 * page directory entry count
 */
#define VM_COVERED_PER_PD_ENTRY 0x400000 // 4MB
#define KERNEL_VADDR_SPACE_PAGE_DIRECTORY_ENTRIES \
	(KERNEL_VADDR_SPACE_SIZE / VM_COVERED_PER_PD_ENTRY) // 256
#define USER_VADDR_SPACE_PAGE_DIRECTORY_ENTRIES \
	(USER_VADDR_SPACE_SIZE / VM_COVERED_PER_PD_ENTRY) // 768

static inline bool virtual_memory_is_userspace_address(v_addr_t address)
{
	return (address >= KERNEL_VADDR_SPACE_TOP);
}

#endif
