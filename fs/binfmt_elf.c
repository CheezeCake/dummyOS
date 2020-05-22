#include <dummyos/errno.h>
#include <fs/file.h>
#include <kernel/elf.h>
#include <kernel/kmalloc.h>
#include <kernel/mm/memory.h>
#include <kernel/mm/uaccess.h>
#include <kernel/mm/vmm.h>
#include <kernel/types.h>
#include <libk/endian.h>
#include <libk/libk.h>

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

int elf_load_binary(struct vfs_file* binfile, struct process_image* img)
{
	struct elf_header e_hdr;
	struct elf_program_header* e_phdr;
	off_t phdr_offset;
	uint16_t phdr_size, phdr_num;
	v_addr_t _end = 0;
	int err = 0;

	if (binfile->op->read(binfile, &e_hdr, sizeof(e_hdr)) != sizeof(e_hdr))
		return -ENOEXEC;
	if (!check_header(&e_hdr))
		return -ENOEXEC;

	img->entry_point = le2h32(e_hdr.e_entry);

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
		v_addr_t start = vaddr;
		v_addr_t end = vaddr + memsz;

		if (memsz == 0)
			continue;

		if (end > _end)
			_end = end;

		if (!vmm_is_userspace_address(vaddr) ||
		    !vmm_is_userspace_address(vaddr + memsz)) {
			err = -ENOEXEC;
			goto end;
		}
		if (align > 1) {
			if (offset % align != vaddr % align) {
				err = -ENOEXEC;
				goto end;
			}
		}
		start = align_down(vaddr, PAGE_SIZE);

		uint8_t vmm_flags = VMM_PROT_USER | VMM_PROT_WRITE;
		if (flags & PF_X) vmm_flags |= VMM_PROT_EXEC;
		err = vmm_create_user_mapping(start, vaddr - start + memsz, vmm_flags, 0);
		if (err)
			goto end;


		// read segment into memory
		if (binfile->op->lseek(binfile, offset, SEEK_SET) != offset) {
			err = -EIO;
			goto end;
		}
		void* buffer = kmalloc(filesz);
		if (!buffer) {
			err = -ENOMEM;
			goto end;
		}
		if (binfile->op->read(binfile, buffer, filesz) != filesz) {
			err = -EIO;
			kfree(buffer);
			goto end;
		}

		err = copy_to_user((void*)vaddr, buffer, filesz);
		if (err) {
			kfree(buffer);
			goto end;
		}

		// zero fill
		if (filesz < memsz)
			memset_user((int8_t*)vaddr + filesz, 0, memsz - filesz);

		kfree(buffer);

		if (!(flags & PF_W)) {
			err = vmm_update_user_mapping_prot(start, vmm_flags & ~VMM_PROT_WRITE);
			if (err)
				goto end;
		}
	}

	img->brk = align_up(_end, PAGE_SIZE);

end:
	kfree(e_phdr);

	return err;
}
