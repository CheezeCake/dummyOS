#include <libk/libk.h>
#include "gdt.h"
#include "segment.h"

static struct gdt_segment_descriptor gdt[GDT_SIZE] = { {0} };

static inline void gdt_init_segment(struct gdt_segment_descriptor* segment_descr,
				    uint32_t base, uint32_t limit,
				    enum privilege_level dpl)
{
	segment_descr->limit_15_0 = limit & 0xffff;

	segment_descr->base_address_15_0 = base & 0xffff;

	segment_descr->base_23_16 = (base >> 16) & 0xff;
	segment_descr->dpl = dpl;
	segment_descr->present = 1;

	segment_descr->limit_19_16 = (limit >> 16) & 0xf;
	segment_descr->avl = 0;
	segment_descr->zero = 0;
	segment_descr->operation_size = 1; // 32bit
	segment_descr->granularity = 1; // 4k pages

	segment_descr->base_31_24 = base >> 24;
}

static inline void gdt_init_code_data_segment(struct gdt_segment_descriptor* segment_descr,
					      enum privilege_level dpl,
					      enum code_data_segment_types type)
{
	gdt_init_segment(segment_descr, 0, 0xffffffff, dpl);

	segment_descr->type = type;
	segment_descr->descriptor_type = CODE_DATA_SEGMENT;
}

void gdt_init_system_segment(unsigned int segment_index,
			     uint32_t base, uint32_t limit,
			     enum privilege_level dpl,
			     enum system_segment_types type)
{
	struct gdt_segment_descriptor* const segment_descr = &gdt[segment_index];

	gdt_init_segment(segment_descr, base, limit, dpl);

	segment_descr->type = type;
	segment_descr->descriptor_type = SYSTEM_SEGMENT;
}

void gdt_init(void)
{
	struct gdtr gdt_register;

	memset(gdt, 0, sizeof(struct gdt_segment_descriptor)); // NULL descriptor
	gdt_init_code_data_segment(&gdt[KCODE], PRIVILEGE_KERNEL, CODE_SEGMENT);
	gdt_init_code_data_segment(&gdt[KDATA], PRIVILEGE_KERNEL, DATA_SEGMENT);
	gdt_init_code_data_segment(&gdt[UCODE], PRIVILEGE_USER, CODE_SEGMENT);
	gdt_init_code_data_segment(&gdt[UDATA], PRIVILEGE_USER, DATA_SEGMENT);

	gdt_register.base_address = (uint32_t)gdt;
	gdt_register.limit = sizeof(gdt);

	// load the gdtr register and update the segment registers
	__asm__ volatile ("lgdt %0\n"
			  "movw %1, %%ax\n"
			  "movw %%ax, %%ss\n"
			  "movw %%ax, %%ds\n"
			  "movw %%ax, %%es\n"
			  "movw %%ax, %%fs\n"
			  "movw %%ax, %%gs\n"
			  "ljmp %2, $next\n"
			  "next:"
			  :
			  : "m" (gdt_register),
			  "i" (make_segment_selector(PRIVILEGE_KERNEL, KDATA)),
			  "i" (make_segment_selector(PRIVILEGE_KERNEL, KCODE))
			  : "eax", "memory");
}
