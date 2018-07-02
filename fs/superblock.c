#include <fs/superblock.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <libk/libk.h>

int vfs_superblock_init(struct vfs_superblock* sb,
						struct vfs_cache_node* device,
						struct vfs_filesystem* fs,
						struct vfs_cache_node* root,
						void* data,
						struct vfs_superblock_operations* op)
{
	memset(sb, 0, sizeof(struct vfs_superblock));

	sb->device = device;
	sb->fs = fs;
	sb->root = root;
	sb->data = data;
	sb->op = op;

	return 0;
}

int vfs_superblock_create(struct vfs_cache_node* device,
						  struct vfs_filesystem* fs,
						  struct vfs_cache_node* root,
						  void* data,
						  struct vfs_superblock_operations* op,
						  struct vfs_superblock** result)
{
	struct vfs_superblock* sb;
	int err;

	sb = kmalloc(sizeof(struct vfs_superblock));
	if (!sb)
		return -ENOMEM;

	err = vfs_superblock_init(sb, device, fs, root, data, op);
	if (err) {
		kfree(sb);
		sb = NULL;
	}

	*result = sb;

	return err;
}

void vfs_superblock_reset(struct vfs_superblock* sb)
{
	if (sb->device)
		vfs_cache_node_unref(sb->device);
	if (sb->root)
		vfs_cache_node_unref(sb->root);

	memset(sb, 0, sizeof(struct vfs_superblock));
}

void vfs_superblock_destroy(struct vfs_superblock* sb)
{
	vfs_superblock_reset(sb);
	kfree(sb);
}
