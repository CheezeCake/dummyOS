#include <dummyos/compiler.h>
#include <dummyos/stat.h>
#include <fs/device.h>
#include <fs/file.h>
#include <fs/inode.h>
#include <fs/vfs.h>
#include <kernel/kmalloc.h>
#include <kernel/mm/uaccess.h>
#include <kernel/sched/sched.h>
#include <libk/libk.h>

static inline mode_t vfs_node_type2stat_mode(enum vfs_node_type type)
{
	switch (type) {
		case REGULAR:
			return  S_IFREG;
		case DIRECTORY:
			 return S_IFDIR;
		case SYMLINK:
			 return S_IFLNK;
		case CHARDEV:
			 return S_IFCHR;
		case BLOCKDEV:
			 return S_IFBLK;
		case FIFO:
			 return S_IFIFO;
		default:
			 return S_IFMT;
	}
}

static int stat(const struct vfs_file* file, struct stat* sb)
{
	struct vfs_inode* inode = vfs_file_get_inode(file);

	memset(sb, 0, sizeof(struct stat));
	if (inode) {
		sb->st_dev = device_makedev(&inode->dev);
		sb->st_mode = vfs_node_type2stat_mode(inode->type);
		sb->st_nlink = inode->linkcnt;
		sb->st_blocks = inode->size / 512;
	}
	else {
		sb->st_mode = vfs_node_type2stat_mode(FIFO);
	}
	sb->st_ino = (ino_t)inode;
	sb->st_blksize = 512;

	return 0;
}

int sys_fstat(int fd, struct stat* __user sb)
{
	const struct vfs_file* file = NULL;
	struct stat kstat;
	int err;

	file = process_get_file(sched_get_current_process(), fd);
	if (!file)
		return -EBADF;

	err = stat(file, &kstat);
	if (!err)
		err = copy_to_user(sb, &kstat, sizeof(struct stat));

	return err;
}

int sys_stat(const char* __user path, struct stat* __user sb)
{
	char* kpath = NULL;
	struct vfs_path vfspath;
	struct vfs_file* file = NULL;
	struct stat kstat;
	int err;

	err = strndup_from_user(path, VFS_PATH_MAX_LEN, &kpath);
	if (err)
		return err;

	err = vfs_path_init(&vfspath, kpath, strlen(kpath));
	if (err)
		goto fail_vfs_path;

	err = vfs_open(&vfspath, 0, &file);
	if (err)
		goto fail_open;

	err = stat(file, &kstat);
	if (!err)
		err = copy_to_user(sb, &kstat, sizeof(struct stat));

	vfs_close(file);
fail_open:
	vfs_path_reset(&vfspath);
fail_vfs_path:
	kfree(kpath);

	return err;
}
