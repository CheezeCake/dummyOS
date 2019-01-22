#include <dummyos/errno.h>
#include <dummyos/fcntl.h>
#include <fs/file.h>
#include <fs/inode.h>
#include <fs/pipe.h>
#include <fs/vfs.h>
#include <kernel/kmalloc.h>
#include <kernel/process.h>
#include <kernel/sched/sched.h>
#include <libk/libk.h>

static void vfs_file_destroy(struct vfs_file* file);

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

	refcount_init(&file->refcnt);

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

void vfs_file_ref(struct vfs_file* file)
{
	refcount_inc(&file->refcnt);
}

void vfs_file_unref(struct vfs_file* file)
{
	if (refcount_dec(&file->refcnt) == 0)
		vfs_file_destroy(file);
}

void vfs_file_release(struct vfs_file* file)
{
	refcount_dec(&file->refcnt);
}

int vfs_file_get_ref(const struct vfs_file* file)
{
	return refcount_get(&file->refcnt);
}

static int __vfs_file_copy_create(struct vfs_file* file, struct vfs_file** copy)
{
	int err;

	err = vfs_file_create(file->cnode, file->flags, copy);
	if (!err)
		vfs_file_init_data_fields(*copy, file->cur, file->op);

	return err;
}

int vfs_file_copy_create(struct vfs_file* file, struct vfs_file** copy)
{
	int err;

	err = __vfs_file_copy_create(file, copy);
	if (err)
		return err;

	if (file->cnode && file->cnode->inode) {
		if (file->cnode->inode->type == FIFO) {
			err = fifo_copy(file, *copy);
			if (err)
				goto fail;
		}
	}
	else {
		err = pipe_copy(file, *copy);
		if (err)
			goto fail;
	}

	return 0;

fail:
	vfs_file_unref(*copy);
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

static void vfs_file_destroy(struct vfs_file* file)
{
	vfs_file_reset(file);
	kfree(file);
}

struct vfs_cache_node* vfs_file_get_cache_node(const struct vfs_file* file)
{
	return file->cnode;
}

struct vfs_inode* vfs_file_get_inode(const struct vfs_file* file)
{
	const struct vfs_cache_node* cnode = vfs_file_get_cache_node(file);
	return (cnode) ? cnode->inode : NULL;
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

int sys_dup(int oldfd)
{
	struct process* proc = sched_get_current_process();
	struct vfs_file* old = process_get_file(proc, oldfd);
	int newfd;

	if (!old)
		return -EBADF;

	newfd = process_add_file(proc, old);

	return newfd;
}

int sys_dup2(int oldfd, int newfd)
{
	struct process* proc = sched_get_current_process();
	struct vfs_file* old = process_get_file(proc, oldfd);
	struct vfs_file* new = NULL;
	int err;

	if (!old)
		return -EBADF;

	if (oldfd == newfd)
		return newfd;

	new = process_get_file(proc, newfd);
	if (new)
		vfs_file_ref(new);

	if (new) {
		vfs_file_ref(new);
		process_remove_file(proc, newfd, NULL);
		if (vfs_file_get_ref(new) == 1)
			vfs_close(new);
		vfs_file_unref(new);
	}

	err = process_add_file_at(proc, old, newfd);
	if (err)
		return err;

	return newfd;
}
