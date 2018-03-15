#include <arch/vm.h>
#include <fs/file.h>
#include <kernel/elf.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/memory.h>
#include <kernel/paging.h>
#include <kernel/types.h>
#include <libk/libk.h>
#include <libk/endian.h>

static inline bool check_magic(const struct elf_header* e_hdr)
{
	const uint8_t magic[] = { 0x7f, 'E', 'L', 'F' };
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

int elf_load_binary(struct vfs_file* binfile, v_addr_t* entry_point)
{
	struct elf_header e_hdr;
	struct elf_program_header* e_phdr;
	off_t phdr_offset;
	uint16_t phdr_size, phdr_num;
	int err = 0;

	if (binfile->op->read(binfile, &e_hdr, sizeof(e_hdr)) != sizeof(e_hdr))
		return -ENOEXEC;
	if (!check_header(&e_hdr))
		return -ENOEXEC;

	*entry_point = le2h32(e_hdr.e_entry);

	phdr_offset = le2h32(e_hdr.e_phoff); // program header offset

	// program header table info
	phdr_size = le2h16(e_hdr.e_phentsize);
	phdr_num = le2h16(e_hdr.e_phnum);

	// consistency check
	if (phdr_size != sizeof(struct elf_program_header))
		return -ENOEXEC;

	size_t size = phdr_size * phdr_num;
	e_phdr = kmalloc(size);
	if (!e_phdr)
		return -ENOMEM;

	off_t seek_offset = binfile->op->lseek(binfile, phdr_offset, SEEK_SET);
	if (seek_offset != phdr_offset) {
		err = -EIO;
		goto end;
	}
	ssize_t read = binfile->op->read(binfile, e_phdr, size);
	if (read != size) {
		err = -EIO;
		goto end;
	}

	for (int i = 0; i < phdr_num; ++i) {
		if (le2h32(e_phdr[i].p_type) != PT_LOAD)
			continue;

		off_t offset = le2h32(e_phdr[i].p_offset);
		v_addr_t vaddr = le2h32(e_phdr[i].p_vaddr);
		uint32_t filesz = le2h32(e_phdr[i].p_filesz);
		uint32_t memsz = le2h32(e_phdr[i].p_memsz);
		uint32_t flags = le2h32(e_phdr[i].p_flags);
		uint32_t align = le2h32(e_phdr[i].p_align);

		if (memsz == 0)
			continue;

		if (!virtual_memory_is_userspace_address(vaddr) ||
			!virtual_memory_is_userspace_address(vaddr + memsz)) {
			err = -ENOEXEC;
			goto end;
		}
		if (offset % align != vaddr % align) {
			err = -ENOEXEC;
			goto end;
		}

		v_addr_t map_addr = vaddr;
		int nb_pages = memsz / PAGE_SIZE;
		if (memsz % PAGE_SIZE > 0)
			++nb_pages;
		for (int p = 0; p < nb_pages; ++p) {
			p_addr_t page = memory_page_frame_alloc();
			if (!page) {
				err = -ENOMEM;
				goto end;
			}

			err = paging_map(page, map_addr, flags | VM_FLAG_USER);
			if (err)
				goto end;

			map_addr += PAGE_SIZE;
		}


		// read segment into memory
		if (binfile->op->lseek(binfile, offset, SEEK_SET) != offset) {
			err = -EIO;
			goto end;
		}
		if (binfile->op->read(binfile, (void*)vaddr, filesz) != filesz) {
			err = -EIO;
			goto end;
		}

		// zero fill
		if (filesz < memsz)
			memset((void*)vaddr + filesz, 0, memsz - filesz);
	}

end:
	kfree(e_phdr);

	return err;
}
