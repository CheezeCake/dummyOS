#ifndef _KERNEL_KMALLOC_H_
#define _KERNEL_KMALLOC_H_

#include <stddef.h>

#include <kernel/types.h>

p_addr_t kmalloc_early(size_t size);
void kmalloc_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif
