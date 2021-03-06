#include <kernel/debug.h>
#include <kernel/log.h>
#include <kernel/mm/vmm.h>

#define print_register(reg, value) \
	log_e_printf("%s = 0x%x\t", reg, value)

static void debug_stacktrace(int frame, uint32_t* ebp)
{
	for (int i = 0; i < frame; i++) {
		const uint32_t eip = ebp[1];
		ebp = (uint32_t*)ebp[0];
		const uint32_t* args = ebp + 2;

		if (!eip)
			return;

		log_e_printf("#%d: [0x%x] ", i, (unsigned int)eip);

		if (vmm_is_userspace_address(eip)) {
			log_e_print("<-- userspace\n");
			return;
		}

		if (ebp) {
			log_e_printf("(0x%x, 0x%x, 0x%x)\n", (unsigned int)args[0],
				     (unsigned int)args[1], (unsigned int)args[2]);
		}
		else {
			log_e_putchar('\n');
			return;
		}
	}
}

static void debug_print_eflags(uint32_t eflags)
{
	const char* const flag_abbr[][2] = {
		[0] = { "cf", "CF" },
		[2] = { "pf", "PF" },
		[4] = { "af", "AF" },
		[6] = { "zf", "ZF" },
		[7] = { "sf", "SF" },
		[8] = { "tf", "TF" },
		[9] = { "if", "IF" },
		[10] = { "df", "DF" },
		[11] = { "of", "OF" },
		[14] = { "nt", "NT" },
		[16] = { "rf", "RF" },
		[17] = { "vm", "VM" },
		[18] = { "ac", "AC" },
		[19] = { "vif", "VIF" },
		[20] = { "vip", "VIP" },
		[21] = { "id",  "ID" }
	};
	const uint32_t bitmask = 0x3f4fd5;

	log_e_printf("eflags: IOPL=%d ", (unsigned int)((eflags >> 12) & 0x3));

	for (int i = 21; i >= 0; --i) {
		if((bitmask >> i) & 1) {
			const int set = (eflags >> i) & 1;
			log_e_puts(flag_abbr[i][set]);
			log_e_putchar(' ');
		}
	}

	log_e_printf("(0x%x)\n", (unsigned int)eflags);
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
	X(esp, output) \
	X(ebp, output) \
	X(esi, output) \
	X(edi, output) \
	log_e_putchar('\n'); \
	X(cs, output) \
	X(ss, output) \
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
		 "popl %0\n"
		 : "=r" (reg));
	debug_print_eflags(reg);

	uint32_t* ebp;
	__asm__ ("movl %%ebp, %0" : "=r" (ebp));

	log_e_puts("\n=== STACKTRACE ===\n");
	debug_stacktrace(DEBUG_MAX_FRAMES, ebp);
	}
