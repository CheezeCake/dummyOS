#include <fs/inode.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <libk/libk.h>

void vfs_inode_init(struct vfs_inode* inode, enum vfs_node_type type,
					struct vfs_superblock* sb, struct vfs_inode_operations* op,
					struct vfs_file_operations* fops)
{
	memset(inode, 0, sizeof(struct vfs_inode));

	inode->type = type;
	inode->sb = sb;
	inode->op = op;
	inode->fops = fops;

	refcount_init(&inode->refcnt);
}

void vfs_inode_init_dev(struct vfs_inode* inode, enum device_major major,
						enum device_minor minor)
{
	inode->dev.major = major;
	inode->dev.minor = minor;
}

int vfs_inode_open(struct vfs_inode* inode, int flags, struct vfs_file* file)
{
	struct vfs_file_operations* fops = inode->fops;
	int err;

	if (!fops)
		return -ENXIO;

	if (inode->type == CHARDEV) {
		fops = chardev_get_fops(&inode->dev);
		if (!fops)
			return -ENXIO;

		inode->private_data = chardev_get_device(&inode->dev);
	}

	err = vfs_file_init(file, fops, inode, flags);
	if (err)
		goto fail;

	err = file->op->open(inode, flags, file);
	if (err)
		goto fail;

	return 0;

fail:
	vfs_file_reset(file);

	return err;
}

void vfs_inode_ref(struct vfs_inode* inode)
{
	refcount_inc(&inode->refcnt);
}

void vfs_inode_unref(struct vfs_inode* inode)
{
	vfs_inode_release(inode);
	if (refcount_get(&inode->refcnt) == 0)
		inode->op->destroy(inode);
}

void vfs_inode_release(struct vfs_inode* inode)
{
	refcount_dec(&inode->refcnt);
}
