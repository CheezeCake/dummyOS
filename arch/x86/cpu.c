#include <kernel/cpu.h>
#include <kernel/log.h>

static char x86_cpu_vendor_str[12] = { '\0' };
static struct cpu_info x86_cpu_info = { .cpu_vendor = x86_cpu_vendor_str };

struct cpu_info* cpu_info(void)
{
	uint32_t* ebx = (uint32_t*)x86_cpu_vendor_str;
	uint32_t* ecx = (uint32_t*)(x86_cpu_vendor_str + 4);
	uint32_t* edx = (uint32_t*)(x86_cpu_vendor_str + 8);

	__asm__ ("xorl %%eax, %%eax\n"
			"cpuid\n"
			: "=b" (*ebx), "=c" (*edx), "=d" (*ecx));

	return &x86_cpu_info;
}
