#ifndef _KERNEL_ELF_H_
#define _KERNEL_ELF_H_

#include <arch/elf.h>
#include <kernel/types.h>

// http://www.sco.com/developers/gabi/2012-12-31/contents.html
// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
// http://wiki.osdev.org/ELF
// http://refspecs.linuxbase.org/elf/elf.pdf

#define EI_NIDENT		16

#define EI_CLASS		4
#define EI_DATA			5
#define EI_VERSION		6
#define EI_OSABI		7
#define EI_ABIVERSION	8

#define ELFCLASS32		1

#define ELFDATA2LSB		1
#define ELFDATA2MSB		2

#define ET_EXEC			2

struct elf_header
{
	uint8_t e_ident[EI_NIDENT];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;

	uint16_t e_phentsize;
	uint16_t e_phnum;

	uint16_t e_shentsize;
	uint16_t e_shnum;

	uint16_t e_shstrndx;
};

#endif
