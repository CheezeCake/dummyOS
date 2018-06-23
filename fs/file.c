#include <fs/file.h>
#include <fs/inode.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <libk/libk.h>

int vfs_file_init(struct vfs_file* file, struct vfs_file_operations* fops,
				  struct vfs_cache_node* cnode,
				  int flags)
{
	memset(file, 0, sizeof(struct vfs_file));

	file->cnode = cnode;
	vfs_cache_node_ref(cnode);

	file->flags = flags;
	file->op = fops;

	return 0;
}

void vfs_file_reset(struct vfs_file* file)
{
	vfs_cache_node_unref(file->cnode);
}

int vfs_file_dup(struct vfs_file* file, struct vfs_file* dup)
{
	memcpy(dup, file, sizeof(struct vfs_file));
	vfs_cache_node_ref(dup->cnode);

	return 0;
}

struct vfs_cache_node* vfs_file_get_cache_node(struct vfs_file* file)
{
	return file->cnode;
}

struct vfs_inode* vfs_file_get_inode(struct vfs_file* file)
{
	return vfs_file_get_cache_node(file)->inode;
}
