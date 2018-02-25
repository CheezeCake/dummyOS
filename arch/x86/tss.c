#include <libk/libk.h>
#include "gdt.h"
#include "segment.h"
#include "tss.h"

static struct tss tss;

void tss_update(uint32_t esp)
{
	tss.esp0 = esp;
}

void tss_init(void)
{
	memset(&tss, 0, sizeof(struct tss));
	tss.ss0 = make_segment_selector(PRIVILEGE_KERNEL, KDATA);

	gdt_init_system_segment(TSS, (uint32_t)&tss, sizeof(tss), PRIVILEGE_USER,
							TSS_32BIT);

	// load TSS
	__asm__ volatile ("ltr %w0"
					  :
					  : "r" (make_segment_selector(PRIVILEGE_USER, TSS))
					  : "memory");
}
