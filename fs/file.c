#include <fs/file.h>
#include <fs/inode.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <libk/libk.h>

int vfs_file_init(struct vfs_file* file, struct vfs_file_operations* fops,
				  struct vfs_inode* inode,
				  int flags)
{
	memset(file, 0, sizeof(struct vfs_file));

	file->inode = inode;
	vfs_inode_ref(inode);

	file->flags = flags;
	file->op = fops;

	return 0;
}

void vfs_file_reset(struct vfs_file* file)
{
	vfs_inode_unref(file->inode);
}
