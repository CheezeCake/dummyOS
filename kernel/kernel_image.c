#include <kernel/kassert.h>
#include <kernel/kernel_image.h>
#include <kernel/mm/memory.h>

// these symbols are defined in the linker script kernel.ld
extern const uint8_t __kernel_v_start;
extern const uint8_t __kernel_p_start;
extern const uint8_t __kernel_v_end;
extern const uint8_t __kernel_p_end;

size_t kernel_end_shift = 0;

void kernel_image_shift_kernel_end(size_t bytes)
{
	kernel_end_shift += bytes;
	kassert(kernel_end_shift >= bytes);
}

/*
 * physical
 */
p_addr_t kernel_image_get_phys_begin(void)
{
	return (p_addr_t)&__kernel_p_start;
}

p_addr_t kernel_image_get_phys_end(void)
{
	return ((p_addr_t)&__kernel_p_end + kernel_end_shift);
}

p_addr_t kernel_image_get_base_page_frame(void)
{
	return page_align_down((p_addr_t)&__kernel_p_start);
}

p_addr_t kernel_image_get_top_page_frame(void)
{
	return page_align_up(kernel_image_get_phys_end());
}

/*
 * virtual
 */
v_addr_t kernel_image_get_virt_begin(void)
{
	return (v_addr_t)&__kernel_v_start;
}

v_addr_t kernel_image_get_virt_end(void)
{
	return ((v_addr_t)&__kernel_v_end + kernel_end_shift);
}

v_addr_t kernel_image_get_base_page(void)
{
	return page_align_down((v_addr_t)&__kernel_v_start);
}

v_addr_t kernel_image_get_top_page(void)
{
	return page_align_up(kernel_image_get_virt_end());
}
