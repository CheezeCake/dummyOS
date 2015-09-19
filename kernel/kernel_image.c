#include <kernel/kernel_image.h>
#include <kernel/memory.h>
#include <kernel/kassert.h>

// these symbols are defined in the linker script kernel.ld
extern const uint8_t __begin_kernel;
extern const uint8_t __end_kernel;

size_t kernel_end_shift = 0;

void kernel_image_shift_kernel_end(size_t bytes)
{
	kernel_end_shift += bytes;
	kassert(kernel_end_shift >= bytes);
}

p_addr_t kernel_image_get_begin(void)
{
	return (p_addr_t)&__begin_kernel;
}

p_addr_t kernel_image_get_end(void)
{
	return ((p_addr_t)&__end_kernel + kernel_end_shift);
}

p_addr_t kernel_image_get_base_page_frame(void)
{
	return page_frame_align_inf((p_addr_t)&__begin_kernel);
}

p_addr_t kernel_image_get_top_page_frame(void)
{
	return page_frame_align_sup((p_addr_t)&__end_kernel + kernel_end_shift);
}
