#include <fs/file.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <libk/libk.h>

int vfs_file_init(struct vfs_file* file, struct vfs_cache_node* node,
				  int flags)
{
	memset(file, 0, sizeof(struct vfs_file));

	file->node = node;
	file->flags = flags;

	return 0;
}

int vfs_file_create(struct vfs_cache_node* node, int flags,
					struct vfs_file** result)
{
	struct vfs_file* file;
	int err;

	file = kmalloc(sizeof(struct vfs_file));
	if (!file)
		return -ENOMEM;

	err = vfs_file_init(file, node, flags);
	if (err) {
		kfree(file);
		file = NULL;
	}

	*result = file;

	return err;
}

void vfs_file_destroy(struct vfs_file* file)
{
	kfree(file);
}
