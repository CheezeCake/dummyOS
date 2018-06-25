#ifndef _FS_VFS_INODE_H_
#define _FS_VFS_INODE_H_

#include <fs/chardev.h>
#include <fs/path.h>
#include <fs/superblock.h>
#include <libk/list.h>
#include <libk/refcount.h>
#include <usr/dirent.h>

struct vfs_cache_node;
struct vfs_inode_operations;
struct vfs_file;

/**
 * @brief File system node types
 */
enum vfs_node_type
{
	REGULAR		= DT_REG,
	DIRECTORY	= DT_DIR,
	SYMLINK		= DT_LNK,
	CHARDEV		= DT_CHR,
	BLOCKDEV	= DT_BLK
};

/**
 * @brief File system inode interface
 */
struct vfs_inode
{
	enum vfs_node_type type;
	struct vfs_superblock* sb;

	struct device dev;

	unsigned int linkcnt; // hard links

	struct vfs_inode_operations* op;
	struct vfs_file_operations* fops;

	void* private_data;

	list_t cnodes;
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
	 * @node Only applies to type == DIRECTORY
	 */
	int (*link)(struct vfs_inode* this, const vfs_path_t* name,
				struct vfs_inode*);

	/**
	 * @node Only applies to type == DIRECTORY
	 */
	int (*unlink)(struct vfs_inode* this, const vfs_path_t* name);

	/**
	 * @brief Extracts the target path of a symlink
	 *
	 * @note Only applies to type == SYMLINK
	 */
	int (*readlink)(struct vfs_inode* this, vfs_path_t** result);

	/**
	 * @brief Destroys a vfs_inode object
	 */
	void (*destroy)(struct vfs_inode* this);
};

/**
 * @brief Initialiazes a vfs_inode object
 */
void vfs_inode_init(struct vfs_inode* inode, enum vfs_node_type type,
					struct vfs_superblock* sb, struct vfs_inode_operations* op,
					struct vfs_file_operations* fops);

/**
 * @brief Initialiazes the "device" field in a vfs_inode object
 */
void vfs_inode_init_dev(struct vfs_inode* inode, enum device_major major,
						enum device_minor minor);

int vfs_inode_open_fops(struct vfs_inode* inode,
						struct vfs_file_operations** result_fops);

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

void vfs_inode_add_cnode(struct vfs_inode* inode, struct vfs_cache_node* cnode);

void vfs_inode_remove_cnode(struct vfs_inode* inode,
							struct vfs_cache_node* cnode);

#endif
