#ifndef _FS_PIPE_H_
#define _FS_PIPE_H_

struct vfs_file;
struct vfs_file_operations;

int pipe_dup(struct vfs_file* file, struct vfs_file* dup);

int fifo_dup(struct vfs_file* file, struct vfs_file* dup);

struct vfs_file_operations* fifo_get_fops(void);

#endif
