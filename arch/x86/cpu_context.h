#ifndef _CPU_CONTEXT_H_
#define _CPU_CONTEXT_H_

#include <dummyos/compiler.h>
#include <kernel/types.h>

struct cpu_context
{
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;

	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
	uint32_t eax;

	uint16_t ds;
	uint16_t es;
	uint16_t fs;
	uint16_t gs;
	uint16_t ss;
	uint16_t alignment;

	// pushed by the CPU
	uint32_t error_code;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	struct
	{
		uint32_t esp;
		uint32_t ss;
	} user;
} __attribute__((packed));
static_assert(sizeof(struct cpu_context) == 64, "sizeof cpu_context != 64");

#endif
