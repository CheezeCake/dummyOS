#ifndef _FS_VFS_CACHE_NODE_H_
#define _FS_VFS_CACHE_NODE_H_

#include <fs/file.h>
#include <fs/inode.h>
#include <fs/path.h>
#include <kernel/locking/mutex.h>
#include <libk/list.h>
#include <libk/refcount.h>

struct vfs_file;

/**
 * @brief Cached file system node
 */
struct vfs_cache_node
{
	vfs_path_t name;
	struct vfs_inode* inode;

	struct vfs_cache_node* parent;
	list_t children;

	struct vfs_cache_node* mountpoint; // where this node is mounted
	struct vfs_cache_node* mounted; // the root of the fs mounted on this node

	/** Chained in vfs_cache_node::children */
	list_node_t cn_children_list;
	/** Chained in vfs_inode::cnodes */
	list_node_t i_cnodes;
	/** Chained in vfs_file::readdir_cache */
	list_node_t f_readdir;

	mutex_t lock;

	refcount_t refcnt;
};

/*
 * @brief Initialiazes the VFS cache subsystem
 *
 * @return 0 on success
 */
int vfs_cache_init(void);

/**
 * @brief Creates a vfs_cache_node object
 *
 * @param inode the underlying inode
 * @param name the node name
 * @param result where to store the created vfs_cache_node
 * @return 0 on success
 */
int vfs_cache_node_create(struct vfs_inode* inode, const vfs_path_t* name,
						  struct vfs_cache_node** result);

/**
 * @brief Initialiazes a vfs_cache_node object
 *
 * @param inode the underlying inode
 * @param name the node name
 * @return 0 on success
 */
int vfs_cache_node_init(struct vfs_cache_node* node, struct vfs_inode* inode,
						const vfs_path_t* name);

/**
 * @brief Add child node
 */
int vfs_cache_node_insert_child(struct vfs_cache_node* parent,
								struct vfs_inode* child_inode,
								const vfs_path_t* name,
								struct vfs_cache_node** inserted_child);

/*
 * @brief Returns the child node that matches a name if it exists
 *
 * @return the child node if it was found, NULL otherwise
 */
struct vfs_cache_node*
vfs_cache_node_lookup_child(struct vfs_cache_node* parent,
							const vfs_path_t* name);

/*
 * @brief Returns the root node of the vfs_cache subsystem
 */
struct vfs_cache_node* vfs_cache_node_get_root(void);

/*
 * @brief Returns the parent node of a vfs_cache_node object
 */
struct vfs_cache_node*
vfs_cache_node_get_parent(const struct vfs_cache_node* node);

/**
 * @brief Returns the root node of the last file system mounted
 *
 * @return the root node of the mounted file system, or mountpoint if no
 * file system was mounted
 */
struct vfs_cache_node*
vfs_cache_node_resolve_mounted_fs(struct vfs_cache_node* mountpoint);

/**
 * @brief Increments the reference counter by one.
 */
void vfs_cache_node_ref(struct vfs_cache_node* node);

/**
 * @brief Decrements the reference counter by one.
 */
void vfs_cache_node_unref(struct vfs_cache_node* node);

/**
 * @brief Returns the reference counter value.
 */
int vfs_cache_node_get_ref(const struct vfs_cache_node* node);

int vfs_cache_node_open(struct vfs_cache_node* cnode, int flags,
						struct vfs_file* file);

void dump_cache_tree(void);
#endif
