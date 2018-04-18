#ifndef _KERNEL_KMALLOC_H_
#define _KERNEL_KMALLOC_H_

#include <kernel/types.h>

v_addr_t kmalloc_early(size_t size);

void kmalloc_init(v_addr_t kheap_start, size_t kheap_size);

void* kmalloc(size_t size);

void* kcalloc(size_t count, size_t size);

void kfree(void* ptr);

#endif
