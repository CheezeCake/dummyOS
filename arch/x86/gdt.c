#include "gdt.h"
#include "segment.h"

static struct gdt_segment_descriptor gdt[GDT_SIZE] = {{0, }};

static inline void gdt_init_segment(struct gdt_segment_descriptor* segment_descr,
		uint8_t dpl, uint8_t type)
{
	segment_descr->limit_15_0 = 0xffff;

	segment_descr->base_address_15_0 = 0;

	segment_descr->base_23_16 = 0;
	segment_descr->type = type;
	segment_descr->descriptor_type = 1;
	segment_descr->dpl = dpl & 0x3;
	segment_descr->present = 1;

	segment_descr->limit_19_16 = 0xf;
	segment_descr->avl = 0;
	segment_descr->operation_size = 1;
	segment_descr->granularity = 1;

	segment_descr->base_31_24 = 0;
}

void gdt_init(void)
{
	struct gdtr gdt_register;

	gdt_init_segment(&gdt[KCODE], 0, CODE_SEGMENT);
	gdt_init_segment(&gdt[KDATA], 0, DATA_SEGMENT);
	gdt_init_segment(&gdt[UCODE], 3, CODE_SEGMENT);
	gdt_init_segment(&gdt[UDATA], 3, DATA_SEGMENT);

	gdt_register.base_address = (uint32_t)gdt;
	gdt_register.limit = sizeof(gdt) - 1;

	// load the gdtr resgister and update the segement registers
	__asm__ __volatile__ (
			"lgdt %0\n"
			"movw %1, %%ax\n"
			"movw %%ax, %%ss\n"
			"movw %%ax, %%ds\n"
			"movw %%ax, %%es\n"
			"movw %%ax, %%fs\n"
			"movw %%ax, %%gs\n"
			"ljmp %2, $next\n"
			"next:"
			:
			: "m" (gdt_register), "i" (make_segment_register(0, false, KDATA)),
			  "i" (make_segment_register(0, false, KCODE))
			: "eax", "memory");
}
