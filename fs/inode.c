#include <fs/inode.h>
#include <kernel/kmalloc.h>
#include <libk/libk.h>

void vfs_inode_init(struct vfs_inode* inode, enum vfs_node_type type,
					struct vfs_superblock* sb, struct vfs_inode_operations* op)
{
	memset(inode, 0, sizeof(struct vfs_inode));

	inode->type = type;
	inode->sb = sb;
	inode->op = op;

	refcount_init(&inode->refcnt);
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
