#include <dummyos/const.h>
#include <fs/vfs.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/mm/uaccess.h>
#include <kernel/process.h>
#include <kernel/sched/sched.h>
#include <libk/libk.h>
#include <usr/fcntl.h>
#include <usr/dirent.h>

int sys_open(const char* __user path, int flags)
{
	struct vfs_file* file;
	char* kpath;
	vfs_path_t vfspath;
	int err;
	int fd;

	err = strndup_from_user(path, VFS_PATH_MAX_LEN, &kpath);
	if (err)
		return err;

	err = vfs_path_init(&vfspath, kpath, strlen(kpath));
	if (err)
		goto fail_path;

	file = kmalloc(sizeof(struct vfs_file));
	if (!file) {
		err = -ENOMEM;
		goto fail_file;
	}

	err = vfs_open(&vfspath, flags, file);
	if (err) {
		if (err != ENOENT || !(flags & O_CREAT))
			goto fail_open;

		// TODO: create the file
	}

	fd = process_add_file(sched_get_current_process(), file);
	if (fd < 0) {
		err = fd;
		goto fail_add;
	}

	return fd;

fail_add:
	vfs_close(file);
fail_open:
	kfree(file);
fail_file:
	vfs_path_reset(&vfspath);
fail_path:
	kfree(kpath);

	return err;
}

int sys_close(int fd)
{
	struct vfs_file* file;
	int err;

	file = process_remove_file(sched_get_current_process(), fd);
	if (!file)
		return -EBADF;

	err = vfs_close(file);

	return err;
}

off_t sys_lseek(int fd, off_t offset, int whence)
{
	return -1;
}

ssize_t sys_read(int fd, void* __user buf, size_t count)
{
	char* kbuf;
	struct vfs_file* file;
	ssize_t ret;
	int err;

	file = process_get_file(sched_get_current_process(), fd);
	if (!file)
		return -EBADF;

	if (!file->op || !file->op->read)
		return -EIO;

	kbuf = kmalloc(count);
	if (!kbuf)
		return -ENOMEM;

	ret = file->op->read(file, kbuf, count);
	if (ret > 0) {
		err = copy_to_user(buf, kbuf, ret);
		if (err)
			ret = err;
	}

	kfree(kbuf);

	return ret;
}

ssize_t sys_write(int fd, const void* __user buf, size_t count)
{
	char* kbuf;
	struct vfs_file* file;
	ssize_t ret;
	int err;

	file = process_get_file(sched_get_current_process(), fd);
	if (!file)
		return -EBADF;

	if (!file->op || !file->op->write)
		return -EIO;

	kbuf = kmalloc(count);
	if (!kbuf)
		return -ENOMEM;

	err = copy_from_user(kbuf, buf, count);
	if (err)
		ret = err;
	else
		ret = file->op->write(file, kbuf, count);

	kfree(kbuf);

	return ret;
}

int sys_ioctl(int fd, int request, intptr_t arg)
{
	struct vfs_file* file;

	file = process_get_file(sched_get_current_process(), fd);
	if (!file)
		return -EBADF;

	if (!file->op)
		return -ENODEV;
	if (!file->op->ioctl)
		return -ENOTTY;

	return file->op->ioctl(file, request, arg);
}

ssize_t sys_getdents(int fd, struct dirent* __user dirp, size_t nbytes)
{
	struct vfs_file* file;

	file = process_get_file(sched_get_current_process(), fd);
	if (!file)
		return -EBADF;

	if (file->cnode->inode->type != DIRECTORY)
		return -ENOTDIR;
	if (!file->op->getdents)
		return -ENODEV;
	if (!vmm_is_valid_userspace_address((v_addr_t)dirp))
		return -EFAULT;

	return file->op->getdents(file, dirp, nbytes);
}

ssize_t _dirent_init(struct dirent* __user dirp, long d_ino,
					 int d_type, size_t d_namlen, const char d_name[])
{
#define copy_to_user_member(member)									\
	do {															\
		err = copy_to_user(&dirp->member, &member, sizeof(member)); \
		if (err)													\
			return err;												\
	} while (0)

	const int16_t d_reclen = __dirent_size() + d_namlen + 1;
	ssize_t n;
	int err;

	copy_to_user_member(d_ino);
	copy_to_user_member(d_reclen);
	copy_to_user_member(d_type);
	copy_to_user_member(d_namlen);

	n = strlcpy_to_user((char*)&dirp->d_name, d_name, d_namlen + 1);
	if (n < 0)
		return n;

	return d_reclen;
}
