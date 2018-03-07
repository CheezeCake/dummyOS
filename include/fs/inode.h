#ifndef _FS_VFS_INODE_H_
#define _FS_VFS_INODE_H_

#include <fs/superblock.h>
#include <fs/path.h>
#include <libk/refcount.h>

struct vfs_cache_node;
struct vfs_inode_operations;

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

/*
 * @brief open() flags
 */
#define O_RDONLY	0
#define O_WRONLY	1
#define O_RDWR		2
#define O_APPEND	3
#define O_CREAT		4
#define O_TRUNC		5

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
	/**
	 * @brief Searches for child node
	 *
	 * @node Only applies to type == DIRECTORY
	 */
	int (*lookup)(struct vfs_inode* this, const vfs_path_t* name,
								struct vfs_inode** result);
	/**
	 * @brief Extracts the target path of a symlink
	 *
	 * @note Only applies to type == SYMLINK
	 */
	int (*readlink)(struct vfs_inode* this, vfs_path_t** target_path);

	/**
	 * @brief Opens this inode as a file
	 *
	 * @note This should fill the vfs_file::op field
	 */
	int (*open)(struct vfs_inode* this, struct vfs_cache_node* node,
				int flags, struct vfs_file* file);

	/**
	 * @brief Closes and file  associated with and inode
	 */
	int (*close)(struct vfs_inode* this, struct vfs_file* file);

	/**
	 * @brief Destroys a vfs_inode object
	 */
	void (*destroy)(struct vfs_inode* this);
};

/**
 * @brief Initialiazes a vfs_inode object
 */
void vfs_inode_init(struct vfs_inode* inode, enum vfs_node_type type,
					struct vfs_superblock* sb, struct vfs_inode_operations* op);

/**
 * @brief Increments the reference counter by one.
 */
void vfs_inode_ref(struct vfs_inode* inode);

/**
 * @brief Decrements the reference counter by one.
 */
void vfs_inode_unref(struct vfs_inode* inode);

/**
 * @brief Decrements the reference counter by one, without destroying the object
 * if the count drops to zero.
 */
void vfs_inode_release(struct vfs_inode* inode);

#endif
