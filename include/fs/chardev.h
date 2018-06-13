#ifndef _FS_VFS_CHARDEV_H_
#define _FS_VFS_CHARDEV_H_

#include <fs/device.h>

#define CHARDEV_MAJOR_NR DEVICE_CHAR_MAJOR_SENTINEL

struct vfs_file_operations;

/**
 * Register a character device with its major number
 */
int chardev_register(enum device_major major,
					 struct vfs_file_operations* fops,
					 void* (*get_device)(enum device_minor));

/**
 * Unregister a character device
 */
int chardev_unregister(enum device_major major);

/**
 * Returns the vfs_file_operations associated with dev->major
 */
struct vfs_file_operations* chardev_get_fops(const struct device* dev);

void* chardev_get_device(const struct device* dev);

#endif
