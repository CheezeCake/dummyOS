#ifndef _ARCH_ELF_H_
#define _ARCH_ELF_H_

#include <kernel/types.h>

#define ELF_HEADER_MACHINE_X86 0x3

static inline bool elf_check_machine(uint16_t e_machine)
{
	return (e_machine == ELF_HEADER_MACHINE_X86);
}

#endif
