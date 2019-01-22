#include <dummyos/errno.h>
#include <fs/chardev.h>
#include <kernel/kmalloc.h>
#include <libk/list.h>

struct chardev
{
	enum device_major major;

	struct vfs_file_operations* fops;

	void* (*get_device)(enum device_minor);
};

struct chardev* devs[CHARDEV_MAJOR_NR] = { NULL };

static inline struct chardev* find_chardev(enum device_major major)
{
	return (major < CHARDEV_MAJOR_NR) ? devs[major] : NULL;
}

static int chardev_create(enum device_major major,
			  struct vfs_file_operations* fops,
			  void* (*get_device)(enum device_minor),
			  struct chardev** result)
{
	struct chardev* cd = kmalloc(sizeof(struct chardev));

	if (!cd)
		return -ENOMEM;

	cd->major = major;
	cd->fops = fops;
	cd->get_device = get_device;

	*result = cd;

	return 0;
}

static inline void chardev_destroy(struct chardev* cd)
{
	kfree(cd);
}

int chardev_register(enum device_major major,
		     struct vfs_file_operations* fops,
		     void* (*get_device)(enum device_minor))
{
	struct chardev* cd = NULL;
	int err;

	if (find_chardev(major))
		return -EEXIST;

	err = chardev_create(major, fops, get_device, &cd);
	if (!err)
		devs[major] = cd;

	return err;
}

int chardev_unregister(enum device_major major)
{
	struct chardev* cd = find_chardev(major);

	if (!cd)
		return -ENOENT;

	devs[major] = NULL;
	chardev_destroy(cd);

	return 0;
}

struct vfs_file_operations* chardev_get_fops(const struct device* dev)
{
	struct chardev* cd = find_chardev(dev->major);

	return (cd) ? cd->fops : NULL;
}

void* chardev_get_device(const struct device* dev)
{
	struct chardev* cd = find_chardev(dev->major);

	return (cd && cd->get_device) ? cd->get_device(dev->minor) : NULL;

}
