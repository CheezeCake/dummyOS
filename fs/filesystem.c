#include <stddef.h>

#include <fs/filesystem.h>
#include <kernel/errno.h>
#include <libk/libk.h>

/**
 * Registered @ref vfs_filesystem list
 */
static list_t filesystem_list = LIST_NULL;

int vfs_filesystem_register(struct vfs_filesystem* fs)
{
	if (vfs_filesystem_get_fs(fs->name))
		return -EEXIST;

	list_push_back(&filesystem_list, &fs->fs_list);

	return 0;
}

struct vfs_filesystem* vfs_filesystem_get_fs(const char* fs_name)
{
	struct list_node* it;

	list_foreach(&filesystem_list, it) {
		struct vfs_filesystem* registered =
			list_entry(it, struct vfs_filesystem, fs_list);
		if (strncmp(fs_name, registered->name, VFS_FILESYSTEM_NAME_MAX_LENGTH) == 0)
			return registered;
	}

	return NULL;
}
