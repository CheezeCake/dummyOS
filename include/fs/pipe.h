#ifndef _FS_PIPE_H_
#define _FS_PIPE_H_

struct vfs_file;
struct vfs_file_operations;

int pipe_copy(struct vfs_file* file, struct vfs_file* copy);

int fifo_copy(struct vfs_file* file, struct vfs_file* copy);

struct vfs_file_operations* fifo_get_fops(void);

#endif
