#ifndef _FS_FIFO_H_
#define _FS_FIFO_H_

struct vfs_file;
struct vfs_file_operations;

int fifo_dup(struct vfs_file* fifo);

struct vfs_file_operations* fifo_get_fops(void);

#endif
