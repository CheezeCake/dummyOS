#ifndef _FS_VFS_FILESYSTEM_H_
#define _FS_VFS_FILESYSTEM_H_

#include <libk/list.h>

/** Max length for a file system name */
#define VFS_FILESYSTEM_NAME_MAX_LENGTH 16

struct vfs_inode;
struct vfs_cache_node;
struct vfs_superblock;

/**
 * @brief File system interface
 */
struct vfs_filesystem
{
	char name[VFS_FILESYSTEM_NAME_MAX_LENGTH];

	int (*superblock_create)(struct vfs_filesystem* this,
				 struct vfs_cache_node* device,
				 int flags, void* data,
				 struct vfs_superblock** result);

	int (*superblock_destroy)(struct vfs_filesystem* this,
				  struct vfs_superblock* sb);

	/** Chained in filesystem::@ref ::filesystem_list */
	list_node_t fs_list;
};

/**
 * @brief Register a filesystem
 */
int vfs_filesystem_register(struct vfs_filesystem* fs);

/**
 * @brief Retrieve a vfs_filesystem with its name
 */
struct vfs_filesystem* vfs_filesystem_get_fs(const char* fs_name);

#endif
