#include <fs/file.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <libk/libk.h>

int vfs_file_create(struct vfs_cache_node* node, int flags,
					struct vfs_file** result)
{
	struct vfs_file* file = kmalloc(sizeof(struct vfs_file));
	if (!file)
		return -ENOMEM;

	memset(file, 0, sizeof(struct vfs_file));

	file->node = node;
	file->flags = flags;

	*result = file;

	return 0;
}

void vfs_file_destroy(struct vfs_file* file)
{
	kfree(file);
}
