#ifndef _FS_VFS_FILE_H_
#define _FS_VFS_FILE_H_

#include <kernel/types.h>

struct vfs_cache_node;
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
	struct vfs_cache_node* node;

	off_t cur;
	int flags;

	struct vfs_file_operations* op;
};

/**
 * @brief File operations interface
 */
struct vfs_file_operations
{
	off_t (*lseek)(struct vfs_file* this, off_t offset, int whence);
	ssize_t (*read)(struct vfs_file* this, void* buf, size_t count);
	ssize_t (*write)(struct vfs_file* this, void* buf, size_t count);
};

/**
 * @brief Initializes a vfs_file object
 */
int vfs_file_init(struct vfs_file* file, struct vfs_cache_node* node,
				  int flags);

/**
 * @brief Creates a vfs_file object
 *
 * @param result where to store the created vfs_file
 */
int vfs_file_create(struct vfs_cache_node* node, int flags,
					struct vfs_file** result);

/**
 * @brief Destroys a vfs_file object
 */
void vfs_file_destroy(struct vfs_file* file);
#endif
