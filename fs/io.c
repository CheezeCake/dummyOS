#include <dummyos/const.h>
#include <dummyos/dirent.h>
#include <dummyos/errno.h>
#include <dummyos/fcntl.h>
#include <fs/vfs.h>
#include <kernel/kmalloc.h>
#include <kernel/mm/uaccess.h>
#include <kernel/process.h>
#include <kernel/sched/sched.h>
#include <libk/libk.h>
#include <libk/list.h>

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

	err = vfs_file_create(NULL, NULL, flags, &file);
	if (err)
		goto fail_file;

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

	vfs_path_reset(&vfspath);
	kfree(kpath);

	return fd;

fail_add:
	vfs_close(file);
	vfs_file_destroy(file);
fail_open:
	vfs_file_destroy(file);
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
	vfs_file_destroy(file);

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

static ssize_t _dirent_init(struct dirent* __user dirp, long d_ino,
							int d_type, size_t d_namlen, const char d_name[])
{
#define copy_to_user_member(member)									\
	do {															\
		err = copy_to_user(&dirp->member, &member, sizeof(member)); \
		if (err)													\
			return err;												\
	} while (0)

	const int16_t d_reclen = align_up(sizeof(struct dirent) + d_namlen + 1,
									  _Alignof(struct dirent));
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

static ssize_t getdents(struct vfs_file* file, struct dirent* __user dirp,
						size_t nbytes)
{
	list_node_t* it = list_get(&file->readdir_cache, file->cur);
	size_t offset = 0;
	int err = 0;

	for (; !err && it != list_end(&file->readdir_cache);
		 it = list_it_next(it), ++file->cur)
	{
		struct vfs_cache_node* cnode =
			list_entry(it, struct vfs_cache_node, f_readdir);
		struct vfs_inode* inode = cnode->inode;
		ssize_t n = _dirent_init((struct dirent*)((int8_t*)dirp + offset),
								 (long)inode,
								 inode->type,
								 vfs_path_get_size(&cnode->name),
								 vfs_path_get_str(&cnode->name));
		if (n < 0)
			err = n;
		else
			offset += n;

		if (offset >= nbytes)
			err = -ENOMEM;
	}

	return (err) ? err : offset;
}

ssize_t sys_getdents(int fd, struct dirent* __user dirp, size_t nbytes)
{
	struct vfs_file* file;
	ssize_t n;
	int err;

	file = process_get_file(sched_get_current_process(), fd);
	if (!file)
		return -EBADF;

	if (file->cnode->inode->type != DIRECTORY)
		return -ENOTDIR;
	if (!file->op->readdir)
		return -ENODEV;
	if (!vmm_is_valid_userspace_address((v_addr_t)dirp))
		return -EFAULT;

	if (list_empty(&file->readdir_cache)) {
		err = file->op->readdir(file, vfs_file_add_readdir_entry);
		file->cur = 0;
		if (err)
			return err;
	}

	mutex_lock(&file->lock);
	n = getdents(file, dirp, nbytes);
	mutex_unlock(&file->lock);

	return n;
}
