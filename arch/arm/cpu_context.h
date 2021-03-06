#ifndef _CPU_CONTEXT_H_
#define _CPU_CONTEXT_H_

#include <kernel/types.h>

struct cpu_context
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r12;

	uint32_t sp;
	uint32_t lr;

	uint32_t alignment;

	uint32_t pc;
	uint32_t cpsr;
};

#endif
