#include <stdbool.h>

#include <fs/file.h>
#include <kernel/elf.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <libk/libk.h>
#include <libk/utils.h>

static inline bool check_magic(const struct elf_header* e_hdr)
{
	const uint8_t magic[] = { 0x7f, 0x45, 0x4c, 0x46 };
	return (memcmp(e_hdr->e_ident, magic, sizeof(magic)) == 0);
}

static bool check_header(const struct elf_header* e_hdr)
{
	if (!check_magic(e_hdr))
		return false;
	// only 32bit elf is supported
	if (e_hdr->e_ident[EI_CLASS] != ELFCLASS32)
		return false;
	// only little-endian is supported
	if (e_hdr->e_ident[EI_DATA] != ELFDATA2LSB)
		return false;
	// version, osabi, abiversion

	// only executables
	if (le2h16(e_hdr->e_type) != ET_EXEC)
		return false;
	if (!elf_check_machine(le2h16(e_hdr->e_machine)))
		return false;

	return true;
}

int elf_load_binary(struct vfs_file* binfile)
{
	struct elf_header e_hdr;
	struct elf_program_header* e_phdr;
	v_addr_t entry_point;
	off_t phdr_offset, shdr_offset;
	uint16_t phdr_size, phdr_num;
	uint16_t shdr_size, shdr_num;
	uint16_t e_shstrndx;
	int err = 0;

	if (binfile->op->read(binfile, &e_hdr, sizeof(e_hdr)) != sizeof(e_hdr))
		return -ENOEXEC;
	if (check_header(&e_hdr))
		return -ENOEXEC;

	entry_point = le2h32(e_hdr.e_entry);

	phdr_offset = le2h32(e_hdr.e_phoff); // program header offset
	shdr_offset = le2h32(e_hdr.e_shoff); // section header offset

	// program header table info
	phdr_size = le2h16(e_hdr.e_phentsize);
	phdr_num = le2h16(e_hdr.e_phnum);

	// section header table info
	shdr_size = le2h16(e_hdr.e_shentsize);
	shdr_num = le2h16(e_hdr.e_shnum);

	// consistency check
	if (phdr_size != sizeof(struct elf_program_header) ||
		shdr_size != sizeof(struct elf_section_header))
		return -ENOEXEC;

	e_shstrndx = le2h16(e_hdr.e_shstrndx);

	size_t size = phdr_size * phdr_num;
	if (!(e_phdr = kmalloc(sizeof(size))))
		return -ENOMEM;

	off_t seek_offset = binfile->op->lseek(binfile, phdr_offset, SEEK_SET);
	if (seek_offset != phdr_offset) {
		err = seek_offset;
		goto end;
	}
	ssize_t read = binfile->op->read(binfile, e_phdr, size);
	if (read != size) {
		err = read;
		goto end;
	}

	for (int i = 0; i < phdr_num; ++i) {
		off_t offset = le2h32(e_phdr[i].p_offset);
		v_addr_t vaddr = le2h32(e_phdr[i].p_vaddr);
		uint32_t filesz = le2h32(e_phdr[i].p_filesz);
		uint32_t memsz = le2h32(e_phdr[i].p_memsz);
	}

end:
	kfree(e_phdr);

	return err;
}
