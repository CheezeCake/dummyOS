#ifndef _FS_VFS_FILE_H_
#define _FS_VFS_FILE_H_

#include <dummyos/const.h>
#include <dummyos/dirent.h>
#include <fs/inode.h>
#include <kernel/locking/mutex.h>
#include <kernel/types.h>

struct vfs_cache_node;
struct vfs_inode;
struct vfs_file_operations;

/*
 * lseek() whence values
 */
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

/**
 * @brief File system file interface
 *
 * A vfs_file represents an opened file
 */
struct vfs_file
{
	struct vfs_cache_node* cnode;

	off_t cur;
	int flags;

	list_t readdir_cache;
	mutex_t lock;

	struct vfs_file_operations* op;
};

/**
 * @brief File operations interface
 */
struct vfs_file_operations
{
	int (*open)(struct vfs_inode* inode, int flags, struct vfs_file* file);
	int (*close)(struct vfs_file* this);

	int (*readdir)(struct vfs_file* this,
				   void (*add_entry)(struct vfs_file*, struct vfs_cache_node*));

	off_t (*lseek)(struct vfs_file* this, off_t offset, int whence);
	ssize_t (*read)(struct vfs_file* this, void* buf, size_t count);
	ssize_t (*write)(struct vfs_file* this, void* buf, size_t count);
	int (*ioctl)(struct vfs_file* this, int request, intptr_t arg);
};


/**
 * @brief Creates a vfs_file object
 */
int vfs_file_create(struct vfs_file_operations* fops,
					struct vfs_cache_node* cnode,
					int flags, struct vfs_file** result);

/**
 * @brief Initializes a vfs_file object
 */
int vfs_file_init(struct vfs_file* file, struct vfs_file_operations* fops,
				  struct vfs_cache_node* cnode,
				  int flags);

/**
 * @brief Destroys a vfs_file object
 */
void vfs_file_destroy(struct vfs_file* file);

/**
 * @brief Resets a vfs_file object
 */
void vfs_file_reset(struct vfs_file* file);

int vfs_file_dup(struct vfs_file* file, struct vfs_file** dup);

struct vfs_cache_node* vfs_file_get_cache_node(struct vfs_file* file);

struct vfs_inode* vfs_file_get_inode(struct vfs_file* file);

void vfs_file_add_readdir_entry(struct vfs_file* file,
								struct vfs_cache_node* entry);

#endif
