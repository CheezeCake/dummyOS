#ifndef _KERNEL_KERNEL_IMAGE_H_
#define _KERNEL_KERNEL_IMAGE_H_

#include <stddef.h>

#include <kernel/types.h>

void kernel_image_shift_kernel_end(size_t bytes);

/*
 * physical
 */
p_addr_t kernel_image_get_phys_end(void);
p_addr_t kernel_image_get_phys_begin(void);

p_addr_t kernel_image_get_base_page_frame(void);
p_addr_t kernel_image_get_top_page_frame(void);

/*
 * virtual
 */
v_addr_t kernel_image_get_virt_begin(void);
v_addr_t kernel_image_get_virt_end(void);

v_addr_t kernel_image_get_base_page(void);
v_addr_t kernel_image_get_top_page(void);

#endif
