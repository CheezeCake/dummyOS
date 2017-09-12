#include <libk/libk.h>
#include "gdt.h"
#include "segment.h"
#include "tss.h"

static struct gdt_segment_descriptor* const gdt =
	(struct gdt_segment_descriptor*)GDT_ADDRESS;
static struct tss tss;

static inline void tss_init_segment()
{
	const uint32_t base = (uint32_t)&tss;
	const uint32_t limit = sizeof(struct tss);
	struct gdt_segment_descriptor* tss_descr = &gdt[TSS];

	tss_descr->limit_15_0 = limit & 0xffff;

	tss_descr->base_address_15_0 = base & 0xffff;

	tss_descr->base_23_16 = (base >> 16) & 0xff;
	tss_descr->type = TSS_32BIT;
	tss_descr->descriptor_type = 0; // 0 = system segment descriptor
	tss_descr->dpl = PRIVILEGE_USER;
	tss_descr->present = 1;

	tss_descr->limit_19_16 = (limit >> 16) & 0xf;
	tss_descr->avl = 0;
	tss_descr->zero = 0;
	tss_descr->operation_size = 1;
	tss_descr->granularity = 1;

	tss_descr->base_31_24 = base >> 24;
}

void tss_update(uint32_t esp)
{
	tss.esp0 = esp;
}

void tss_init(void)
{
	memset(&tss, 0, sizeof(struct tss));
	tss.ss0 = make_segment_selector(PRIVILEGE_KERNEL, KDATA);

	tss_init_segment();

	// load TSS
	__asm__ __volatile__ (
			"ltr %w0"
			:
			: "r" (make_segment_selector(PRIVILEGE_USER, TSS))
			: "memory");
}
