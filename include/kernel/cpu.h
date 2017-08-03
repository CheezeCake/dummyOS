#ifndef _KERNEL_CPU_H_
#define _KERNEL_CPU_H_

struct cpu_info
{
	const char* cpu_vendor;
};

struct cpu_info* cpu_info(void);

#endif
