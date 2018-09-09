#include <dummyos/errno.h>
#include <dummyos/fcntl.h>
#include <fs/file.h>
#include <fs/inode.h>
#include <fs/pipe.h>
#include <kernel/kmalloc.h>
#include <libk/libk.h>

static inline void vfs_file_init_data_fields(struct vfs_file* file, off_t cur,
											 struct vfs_file_operations* op)
{
	file->cur = cur;
	file->op = op;
}

static int vfs_file_init(struct vfs_file* file, struct vfs_cache_node* cnode,
						 int flags)
{
	memset(file, 0, sizeof(struct vfs_file));

	file->cnode = cnode;
	if (cnode)
		vfs_cache_node_ref(cnode);

	file->flags = flags;

	list_init(&file->readdir_cache);
	mutex_init(&file->lock);

	vfs_file_init_data_fields(file, 0, NULL);

	return 0;
}

int vfs_file_create(struct vfs_cache_node* cnode, int flags,
					struct vfs_file** result)
{
	struct vfs_file* file;
	int err;

	file = kmalloc(sizeof(struct vfs_file));
	if (!file)
		return -ENOMEM;

	err = vfs_file_init(file, cnode, flags);
	if (err) {
		kfree(file);
		file = NULL;
	}

	*result = file;

	return err;
}

static int vfs_file_copy_create(struct vfs_file* file, struct vfs_file** copy)
{
	int err;

	err = vfs_file_create(file->cnode, file->flags, copy);
	if (!err)
		vfs_file_init_data_fields(*copy, file->cur, file->op);

	return err;
}

static void clear_readdir_cache(struct vfs_file* file)
{
	list_node_t* it;
	list_node_t* next;
	struct vfs_cache_node* cnode;

	list_foreach_safe(&file->readdir_cache, it, next) {
		cnode = list_entry(it, struct vfs_cache_node, f_readdir);
		vfs_cache_node_unref(cnode);
	}

	list_clear(&file->readdir_cache);
}

static void vfs_file_reset(struct vfs_file* file)
{
	if (file->cnode)
		vfs_cache_node_unref(file->cnode);
	clear_readdir_cache(file);
	mutex_destroy(&file->lock);

	memset(file, 0, sizeof(struct vfs_file));
}

void vfs_file_destroy(struct vfs_file* file)
{
	vfs_file_reset(file);
	kfree(file);
}

int vfs_file_dup(struct vfs_file* file, struct vfs_file** dup)
{
	int err;

	err = vfs_file_copy_create(file, dup);
	if (err)
		return err;

	if (file->cnode && file->cnode->inode) {
		if (file->cnode->inode->type == FIFO) {
			err = fifo_dup(file, *dup);
			if (err)
				goto fail;
		}
	}
	else {
		err = pipe_dup(file, *dup);
		if (err)
			goto fail;
	}

	return 0;

fail:
	vfs_file_destroy(*dup);
	return err;
}

struct vfs_cache_node* vfs_file_get_cache_node(const struct vfs_file* file)
{
	return file->cnode;
}

struct vfs_inode* vfs_file_get_inode(const struct vfs_file* file)
{
	return vfs_file_get_cache_node(file)->inode;
}

void vfs_file_add_readdir_entry(struct vfs_file* file,
								struct vfs_cache_node* entry)
{
	mutex_lock(&file->lock);

	list_push_back(&file->readdir_cache, &entry->f_readdir);
	vfs_cache_node_ref(entry);

	mutex_unlock(&file->lock);
}

bool vfs_file_flags_read(int flags)
{
	return ((flags + 1) & (O_RDONLY + 1));
}

bool vfs_file_flags_write(int flags)
{
	return ((flags + 1) & (O_WRONLY + 1));
}
