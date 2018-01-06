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

int vfs_file_create(struct vfs_cache_node* node, int flags,
					struct vfs_file** result);

void vfs_file_destroy(struct vfs_file* file);
#endif
