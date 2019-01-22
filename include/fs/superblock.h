#ifndef _FS_VFS_SUPERBLOCK_H_
#define _FS_VFS_SUPERBLOCK_H_

#include <fs/cache_node.h>
#include <fs/filesystem.h>
#include <fs/inode.h>
#include <libk/list.h>

struct vfs_superblock_operations;

/**
 * @brief File system superblock interface
 *
 * A superblock represents an "instance" of a given vfs_filesystem
 */
struct vfs_superblock
{
	struct vfs_cache_node* device;

	struct vfs_filesystem* fs;
	struct vfs_cache_node* root;

	void* data; /**< superblock private data */

	struct vfs_superblock_operations *op;

	list_node_t mounted_list; /**< Chained in vfs::@ref ::mounted_list */
};

/**
 * @brief File system superblock operations interface
 */
struct vfs_superblock_operations
{
	/**
	 * @brief Allocate an inode on the device
	 */
	int (*alloc_inode)(struct vfs_superblock* this, struct vfs_inode** result);

	/**
	 * @brief Free an inode on the device
	 */
	int (*free_inode)(struct vfs_superblock* this, struct vfs_inode* inode);
	// statfs

	/**
	 * @brief Read inode from the device
	 *
	 * @note This "fills" the vfs_inode struct.
	 * The on-disk inode is located with the data stored in the enclosing
	 * file-system dependent inode_info struct.
	 */
	int (*read_inode)(struct vfs_superblock* this, struct vfs_inode* inode);
};

/**
 * @brief Initialiazes a vfs_superblock object
 */
int vfs_superblock_init(struct vfs_superblock* sb,
			struct vfs_cache_node* device,
			struct vfs_filesystem* fs,
			struct vfs_cache_node* root,
			void* data,
			struct vfs_superblock_operations* op);

/**
 * @brief Creates a vfs_superblock object
 *
 * Allocates the memory for the result.
 */
int vfs_superblock_create(struct vfs_cache_node* device,
			  struct vfs_filesystem* fs,
			  struct vfs_cache_node* root,
			  void* data,
			  struct vfs_superblock_operations* op,
			  struct vfs_superblock** result);

/**
 * @brief Resets a vfs_superblock object
 */
void vfs_superblock_reset(struct vfs_superblock* sb);

/**
 * @brief Destroys a vfs_superblock object
 *
 * Calls kfree on the object.
 */
void vfs_superblock_destroy(struct vfs_superblock* sb);

#endif
