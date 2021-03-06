#include <dummyos/errno.h>
#include <fs/inode.h>
#include <fs/pipe.h>
#include <kernel/kmalloc.h>
#include <libk/libk.h>

void vfs_inode_init(struct vfs_inode* inode, enum vfs_node_type type,
		    struct vfs_superblock* sb, struct vfs_inode_operations* op)
{
	memset(inode, 0, sizeof(struct vfs_inode));

	inode->type = type;
	inode->sb = sb;
	inode->op = op;

	list_init(&inode->cnodes);
	refcount_init(&inode->refcnt);
}

void vfs_inode_init_dev(struct vfs_inode* inode, enum device_major major,
			enum device_minor minor)
{
	inode->dev.major = major;
	inode->dev.minor = minor;
}

int vfs_inode_open_fops(struct vfs_inode* inode,
			struct vfs_file_operations** result)
{
	struct vfs_file_operations* fops = NULL;

	switch (inode->type) {
		case CHARDEV:
			fops = chardev_get_fops(&inode->dev);
			if (!fops)
				return -ENXIO;
			inode->private_data = chardev_get_device(&inode->dev);
			break;
		case FIFO:
			fops = fifo_get_fops();
			break;
		default:
			return 0;
	}

	*result = fops;

	return 0;
}

void vfs_inode_ref(struct vfs_inode* inode)
{
	refcount_inc(&inode->refcnt);
}

void vfs_inode_unref(struct vfs_inode* inode)
{
	if (refcount_dec(&inode->refcnt) == 0)
		inode->op->destroy(inode);
}

void vfs_inode_release(struct vfs_inode* inode)
{
	refcount_dec(&inode->refcnt);
}

void vfs_inode_add_cnode(struct vfs_inode* inode, struct vfs_cache_node* cnode)
{
	list_push_back(&inode->cnodes, &cnode->i_cnodes);
}

void vfs_inode_remove_cnode(struct vfs_inode* inode,
			    struct vfs_cache_node* cnode)
{
	list_erase(&cnode->i_cnodes);
}
