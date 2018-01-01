#ifndef _FS_VFS_INODE_H_
#define _FS_VFS_INODE_H_

#include <fs/superblock.h>
#include <fs/path.h>
#include <libk/refcount.h>

/**
 * @brief File system node types
 */
enum vfs_node_type
{
	REGULAR,
	DIRECTORY,
	SYMLINK,
	DEVCHAR,
	DEVBLOCK
};

/**
 * @brief File system inode interface
 */
struct vfs_inode
{
	enum vfs_node_type type;
	struct vfs_superblock* sb;

	unsigned int linkcnt; // hard links

	struct vfs_inode_operations* op;

	refcount_t refcnt;
};

/**
 * @brief File system inode operations interface
 */
struct vfs_inode_operations
{
	// directory
	int (*lookup)(struct vfs_inode* this, const vfs_path_t* name,
								struct vfs_inode** result);
	// symlink
	int (*readlink)(struct vfs_inode* this, vfs_path_t** target_path);
	// all
	/* struct file* (*open)(struct vfs_inode* this, int mode); */
	/* int (*close)(struct vfs_inode* this, struct file* file); */

	void (*destroy)(struct vfs_inode* this);
};

/**
 * @brief Initialiazes a vfs_inode object
 */
void vfs_inode_init(struct vfs_inode* inode, enum vfs_node_type type,
					struct vfs_superblock* sb, struct vfs_inode_operations* op);

/**
 * @brief Grabs the object. Increments the reference counter by one.
 */
void vfs_inode_grab_ref(struct vfs_inode* inode);

/**
 * @brief Drops the object. Decrements the reference counter by one.
 */
void vfs_inode_drop_ref(struct vfs_inode* inode);

/**
 * @brief Decrements the reference counter by one, without destroying the object
 * if the count drops to zero.
 */
void vfs_inode_release_ref(struct vfs_inode* inode);

#endif
