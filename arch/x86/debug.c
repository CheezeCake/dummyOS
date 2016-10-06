#include <kernel/debug.h>
#include <kernel/terminal.h>

#define print_register(reg, value) \
	terminal_printf("%s:\t0x%x\n", reg, value)

static void debug_stacktrace(int frame, uint32_t* ebp)
{
	for (int i = 0; i < frame; i++) {
		uint32_t* args = ebp + 2;
		uint32_t eip = ebp[1];
		if (eip == 0)
			return;

		terminal_printf("#%d: [0x%x] (0x%x, 0x%x, 0x%x)\n", i, (unsigned int)eip,
				(unsigned int)args[0], (unsigned int)args[1],
				(unsigned int)args[2]);

		ebp = (uint32_t*)ebp[0];
	}
}

void debug_dump()
{
	uint32_t reg;

#define REGISTERS(output) \
	X(eax, output) \
	X(ebx, output) \
	X(ecx, output) \
	X(edx, output) \
	X(esi, output) \
	X(edi, output) \
	X(ebp, output) \
	X(esp, output)

	terminal_puts("\n");

#define X(reg, output) { \
	__asm__ ("movl %%"#reg", %0" : "=r" (output)); \
	print_register(#reg, (unsigned int)output); \
}
	REGISTERS(reg)
#undef X


	__asm__ (
			"pushfl\n"
			"movl (%%esp), %0\n"
			"addl $4, %%esp"
			: "=r" (reg));
	print_register("eflags", (unsigned int)reg);

	uint32_t* ebp;
	__asm__ ("movl %%ebp, %0" : "=r" (ebp));
	ebp = (uint32_t*)ebp[0];

	terminal_puts("\n=== STACKTRACE ===\n");
	debug_stacktrace(DEBUG_MAX_FRAMES, ebp);
}
