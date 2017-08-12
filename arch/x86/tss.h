#ifndef _TSS_H_
#define _TSS_H_

#include <stdint.h>

// Intel Architecture Software Developer’s Manual Volume 3, section 6.2.1
struct tss
{
	uint16_t previous_task_link;
	uint16_t reserved1;
	uint32_t esp0;
	uint16_t ss0;
	uint16_t reserved2;
	uint32_t esp1;
	uint16_t ss1;
	uint16_t reserved3;
	uint32_t esp2;
	uint16_t ss2;
	uint16_t reserved4;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint16_t es;
	uint16_t reserved5;
	uint16_t cs;
	uint16_t reserved6;
	uint16_t ss;
	uint16_t reserved7;
	uint16_t ds;
	uint16_t reserved8;
	uint16_t fs;
	uint16_t reserved9;
	uint16_t gs;
	uint16_t reserved10;
	uint16_t ldt;
	uint16_t reserved11;
	uint16_t trap;
	uint16_t iomap_base;
} __attribute__((packed));

void tss_init(void);
void tss_update(uint32_t esp);

#endif
