#ifndef _KMALLOC_H_
#define _KMALLOC_H_

#include <kernel/kernel_image.h>

p_addr_t kmalloc_early(size_t size);
void kmalloc_init(p_addr_t kernel_top);
void* kmalloc(size_t size);
void kfree(void* ptr);

#endif
