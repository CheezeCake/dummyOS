#ifndef _KERNEL_IMAGE_H_
#define _KERNEL_IMAGE_H_

#include <stddef.h>
#include <kernel/types.h>

void kernel_image_shift_kernel_end(size_t bytes);

p_addr_t kernel_image_get_end(void);
p_addr_t kernel_image_get_begin(void);

p_addr_t kernel_image_get_base_page_frame(void);
p_addr_t kernel_image_get_top_page_frame(void);

#endif
