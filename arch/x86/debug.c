#include <kernel/debug.h>
#include <kernel/log.h>

#define print_register(reg, value) \
	log_e_printf("%s = 0x%x\t", reg, value)

static void debug_stacktrace(int frame, uint32_t* ebp)
{
	for (int i = 0; i < frame; i++) {
		uint32_t* args = ebp + 2;
		uint32_t eip = ebp[1];
		if (eip == 0)
			return;

		log_e_printf("#%d: [0x%x] (0x%x, 0x%x, 0x%x)\n", i, (unsigned int)eip,
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
	log_e_putchar('\n'); \
	X(esi, output) \
	X(edi, output) \
	X(ebp, output) \
	X(esp, output) \
	log_e_putchar('\n');

	log_e_putchar('\n');

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
	log_e_putchar('\n');

	uint32_t* ebp;
	__asm__ ("movl %%ebp, %0" : "=r" (ebp));
	ebp = (uint32_t*)ebp[0];

	log_e_puts("\n=== STACKTRACE ===\n");
	debug_stacktrace(DEBUG_MAX_FRAMES, ebp);
}
