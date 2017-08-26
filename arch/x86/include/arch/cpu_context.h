#ifndef _ARCH_CPU_CONTEXT_H_
#define _ARCH_CPU_CONTEXT_H_

#include <stdint.h>

struct cpu_context
{
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;

	uint32_t edi;
	uint32_t esi;
	uint32_t esp;
	uint32_t ebp;

	uint16_t cs;
	uint16_t ds;
	uint16_t es;
	uint16_t fs;
	uint16_t gs;
	uint16_t ss;

	uint32_t eip;

	uint32_t eflags;
}; // __attribute__((packed));

extern void cpu_context_switch(struct cpu_context* from, const struct cpu_context* to);

#endif
