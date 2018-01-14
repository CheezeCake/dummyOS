#ifndef _KERNEL_ELF_H_
#define _KERNEL_ELF_H_

#include <arch/elf.h>
#include <kernel/types.h>

// http://www.sco.com/developers/gabi/2012-12-31/contents.html
// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
// http://wiki.osdev.org/ELF
// http://refspecs.linuxbase.org/elf/elf.pdf

/** elf_header::e_ident size */
#define EI_NIDENT		16

// elf_header::e_indent indexes
#define EI_CLASS		4
#define EI_DATA			5
#define EI_VERSION		6
#define EI_OSABI		7
#define EI_ABIVERSION	8

// elf_header::e_indent[EI_CLASS]
#define ELFCLASS32		1

// elf_header::e_indent[EI_DATA]
#define ELFDATA2LSB		1
#define ELFDATA2MSB		2

// elf_header::e_type
#define ET_EXEC			2

// elf_program_header::p_type
#define PT_LOAD			1

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

struct elf_program_header
{
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
};

struct elf_section_header
{
	uint32_t sh_name;
	uint32_t sh_type;
	uint32_t sh_flags;
	uint32_t sh_addr;
	uint32_t sh_offset;
	uint32_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint32_t sh_addralign;
	uint32_t sh_entsize;
};

#endif
