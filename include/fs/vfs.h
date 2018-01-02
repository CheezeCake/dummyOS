#ifndef _FS_VFS_H_
#define _FS_VFS_H_

#include <stdbool.h>

#include <fs/cache_node.h>
#include <fs/path.h>

/**
 * @brief Initialiazes the VFS subsystem
 *
 * @return 0 on success
 */
int vfs_init(void);

/**
 * @brief Mounts a File system
 */
int vfs_mount(struct vfs_cache_node* device, struct vfs_cache_node* mountpoint,
			  void* data, const char* filesystem);

/**
 * @brief Unmounts a File system
 *
 * @param mountpoint the node on which the file system is mounted
 * @return 0 on success \n
 *		-EINVAL if no file system was mounted on mountpoint
 */
int vfs_umount(struct vfs_cache_node* mountpoint);

/**
 * @brief Find a node in the VFS with a vfs_path
 */
int vfs_lookup(const vfs_path_t* path, struct vfs_cache_node* root,
			   struct vfs_cache_node* cwd, struct vfs_cache_node** result);

/**
 * @brief Find a node in a file system
 */
int vfs_lookup_in_fs(const vfs_path_t* path, struct vfs_superblock* sb,
					 struct vfs_cache_node** result);

#endif
