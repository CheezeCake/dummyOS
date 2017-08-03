#ifndef _KERNEL_KHEAP_H_
#define _KERNEL_KHEAP_H_

#include <stddef.h>
#include <kernel/types.h>
#include <arch/virtual_memory.h>

#define KHEAP_INITIAL_SIZE 0x80000 // 512kB
#define KHEAP_LIMIT MIRRORING_VADDR_BEGIN

/*
 * kheap_init:
 * returns the size of the initialized kernel heap
 * return value < initial_size => error
 */
size_t kheap_init(v_addr_t start, size_t initial_size);

v_addr_t kheap_sbrk(size_t increment);
v_addr_t kheap_get_start(void);
v_addr_t kheap_get_end(void);

#endif
