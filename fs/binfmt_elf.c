#include <stdbool.h>

#include <fs/file.h>
#include <kernel/elf.h>
#include <kernel/errno.h>
#include <libk/libk.h>

static inline bool check_magic(const struct elf_header* e_hdr)
{
	const uint8_t magic[] = { 0x7f, 0x45, 0x4c, 0x46 };
	return (memcmp(e_hdr->e_ident, magic, sizeof(magic)) == 0);
}

int elf_load_binary(struct vfs_file* binfile)
{
	struct elf_header e_hdr;

	if (binfile->op->read(binfile, &e_hdr, sizeof(e_hdr)) != sizeof(e_hdr))
		return -ENOEXEC;

	if (!check_magic(&e_hdr))
		return -ENOEXEC;
	// only 32bit elf is supported
	if (e_hdr.e_ident[EI_CLASS] != ELFCLASS32)
		return -ENOEXEC;
	// only little-endian is supported
	if (e_hdr.e_ident[EI_DATA] != ELFDATA2LSB)
		return -ENOEXEC;
	// version, osabi, abiversion

	if (!elf_check_machine(e_hdr.e_machine))
		return -ENOEXEC;

	return 0;
}
