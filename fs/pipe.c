#include <dummyos/fcntl.h>
#include <fs/pipe.h>
#include <kernel/kmalloc.h>
#include <kernel/locking/mutex.h>
#include <kernel/mm/uaccess.h>
#include <kernel/panic.h>
#include <kernel/sched/sched.h>
#include <kernel/sched/wait.h>
#include <libk/circ_buf.h>
#include <libk/libk.h>
#include <libk/refcount.h>

#define PIPE_BUFFER_SIZE 128

static struct vfs_file_operations pipe_fops;
static struct vfs_file_operations fifo_fops;

struct pipe
{
	CIRC_BUF_DEFINE1(buffer, PIPE_BUFFER_SIZE);

	mutex_t lock;
	wait_queue_t wq;

	refcount_t readers;
	refcount_t writers;
};

static void pipe_init(struct pipe* p)
{
	circ_buf_init(&p->buffer, p->circ_buf_buffer(buffer), PIPE_BUFFER_SIZE);

	mutex_init(&p->lock);
	wait_init(&p->wq);

	refcount_init_zero(&p->readers);
	refcount_init_zero(&p->writers);
}

static int pipe_create(struct pipe** result)
{
	struct pipe* p;

	p = kmalloc(sizeof(struct pipe));
	if (!p)
		return -ENOMEM;

	pipe_init(p);

	*result = p;

	return 0;
}

static void pipe_reset(struct pipe* p)
{
	mutex_destroy(&p->lock);
	wait_reset(&p->wq);

	memset(p, 0, sizeof(struct pipe));
}

static void pipe_destroy(struct pipe* p)
{
	pipe_reset(p);
	kfree(p);
}

#define pipe_add(f, type) refcount_inc(&(f)->type)

static inline void pipe_add_reader(struct pipe* p)
{
	pipe_add(p, readers);
}

static inline void pipe_add_writer(struct pipe* p)
{
	pipe_add(p, writers);
}

/**
 * @return 0 if the pipe was destroyed
 */
static inline int pipe_remove(struct pipe* p, refcount_t* type,
							   const refcount_t* other_type)
{
	int type_rc = refcount_dec(type);

	if (type_rc == 0) {
		if (refcount_get(other_type) == 0) {
			pipe_destroy(p);
			return 0;
		}
		else {
			// no more readers/writers
			// wake so the others are notified
			wait_wake_all(&p->wq);
		}
	}

	return type_rc;
}

static int pipe_remove_reader(struct pipe* p)
{
	return pipe_remove(p, &p->readers, &p->writers);
}

static int pipe_remove_writer(struct pipe* p)
{
	return pipe_remove(p, &p->writers, &p->readers);
}

static void generate_sigpipe(void)
{
	/* const struct process* proc = sched_get_current_process(); */
	/* process_kill(proc->pid, SIGPIPE); */
}

/*
 * file operations
 */
static int __pipe_acquire(struct pipe* p, int flags, bool block)
{
	bool r = vfs_file_flags_read(flags);
	bool w = vfs_file_flags_write(flags);
	refcount_t* other = NULL;

	if (r && w) {
		pipe_add_reader(p);
		pipe_add_writer(p);
		wait_wake_all(&p->wq);
		return 0;
	}
	else if (r) {
		pipe_add_reader(p);
		other = &p->writers;
	}
	else if (w) {
		pipe_add_writer(p);
		other = &p->readers;
	}
	else {
		return -EPERM;
	}

	if (block) {
		while (refcount_get(other) == 0) {
			wait_wake_all(&p->wq);
			wait_wait(&p->wq);
		}

		wait_wake_all(&p->wq);
	}

	return 0;
}

static int pipe_open(struct vfs_file* this, int flags)
{
	PANIC("pipe_open");
	return -EINVAL;
}

static int fifo_open(struct vfs_file* this, int flags)
{
	struct vfs_inode* inode = vfs_file_get_inode(this);
	int err;

	if (!inode->private_data) {
		struct pipe* p;
		err = pipe_create(&p);
		if (err)
			return err;

		inode->private_data = p;
	}

	err = __pipe_acquire(inode->private_data, flags, true);
	if (!err)
		this->op = &fifo_fops;

	return err;
}

static int __pipe_close(struct pipe* p, struct vfs_file* file)
{
	bool r = vfs_file_perm_read(file);
	bool w = vfs_file_perm_write(file);
	int ret;

	if (r && w) {
		pipe_remove_reader(p);
		ret = pipe_remove_writer(p);
	}
	else if (r) {
		ret = pipe_remove_reader(p);
	}
	else if (w) {
		ret = pipe_remove_writer(p);
	}
	else {
		ret = -EINVAL;
	}

	return ret;
}

static int pipe_close(struct vfs_file* this)
{
	__pipe_close(this->private_data, this);
	this->private_data = NULL;

	return 0;
}

static int fifo_close(struct vfs_file* this)
{
	struct vfs_inode* inode = vfs_file_get_inode(this);
	int ret;

	ret = __pipe_close(inode->private_data, this);
	if (ret == 0)
		inode->private_data = NULL;

	return 0;
}

static ssize_t __pipe_read(struct pipe* p, void* buf, size_t count)
{
	uint8_t* buffer = buf;
	size_t n = 0;
	uint8_t c;
	int err;

	if (!p)
		return -EIO;

	mutex_lock(&p->lock);
	while (circ_buf_empty(&p->buffer)) {
		mutex_unlock(&p->lock);

		wait_wait(&p->wq);

		mutex_lock(&p->lock);
	}

	do {
		err = circ_buf_pop(&p->buffer, &c);
		if (!err)
			buffer[n++] = c;
	} while (!err && n < count && !circ_buf_empty(&p->buffer));

	mutex_unlock(&p->lock);

	return n;
}

static ssize_t pipe_read(struct vfs_file* this, void* buf, size_t count)
{
	return __pipe_read(this->private_data, buf, count);
}

static ssize_t fifo_read(struct vfs_file* this, void* buf, size_t count)
{
	return __pipe_read(vfs_file_get_inode(this)->private_data, buf, count);
}

static ssize_t __pipe_write(struct pipe* p, const void* buf, size_t count)
{
	const uint8_t* buffer = buf;
	size_t n = 0;
	int err = 0;

	if (!p)
		return -EIO;

	if (refcount_get(&p->readers) == 0) {
		generate_sigpipe();
		return -EPIPE;
	}

	mutex_lock(&p->lock);

	while (!err && n < count && !circ_buf_full(&p->buffer)) {
		err = circ_buf_push(&p->buffer, buffer[n]);
		if (!err)
			++n;
	}

	mutex_unlock(&p->lock);

	return n;
}

static ssize_t pipe_write(struct vfs_file* this, const void* buf, size_t count)
{
	return __pipe_write(this->private_data, buf, count);
}

static ssize_t fifo_write(struct vfs_file* this, const void* buf, size_t count)
{
	return __pipe_write(vfs_file_get_inode(this)->private_data, buf, count);
}

static struct vfs_file_operations pipe_fops = {
	.open	= pipe_open,
	.close	= pipe_close,
	.read	= pipe_read,
	.write	= pipe_write,
};

static struct vfs_file_operations fifo_fops = {
	.open	= fifo_open,
	.close	= fifo_close,
	.read	= fifo_read,
	.write	= fifo_write,
};


int pipe_copy(struct vfs_file* file, struct vfs_file* copy)
{
	int err;

	err = __pipe_acquire(file->private_data, file->flags, false);
	copy->private_data = file->private_data;

	return err;
}

int fifo_copy(struct vfs_file* file, struct vfs_file* copy)
{
	struct vfs_inode* inode = vfs_file_get_inode(file);
	int err;

	err = __pipe_acquire(inode->private_data, file->flags, true);

	return err;
}

struct vfs_file_operations* fifo_get_fops(void)
{
	return &fifo_fops;
}

static inline int create_pipe_file(struct pipe* p, int flags,
								   struct vfs_file** result)
{
	struct vfs_file* file;
	int err;

	err = vfs_file_create(NULL, flags, &file);
	if (!err) {
		file->private_data = p;
		__pipe_acquire(p, flags, false);

		file->op = &pipe_fops;

		*result = file;
	}

	return err;
}

int sys_pipe(int __user fds[2])
{
	struct process* proc = sched_get_current_process();
	struct vfs_file* r;
	struct vfs_file* w;
	struct pipe* p;
	int kfds[2];
	int err;

	err = pipe_create(&p);
	if (err)
		return err;

	err = create_pipe_file(p, O_RDONLY, &r);
	if (err)
		goto fail_reader;

	err = create_pipe_file(p, O_WRONLY, &w);
	if (err)
		goto fail_writer;

	kfds[0] = process_add_file(proc, r);
	if (kfds[0] < 0) {
		err = kfds[0];
		goto fail_add_reader;
	}

	kfds[1] = process_add_file(proc, w);
	if (kfds[1] < 0) {
		err = kfds[1];
		goto fail_add_writer;
	}

	err = copy_to_user(fds, kfds, sizeof(kfds));
	if (err)
		goto fail_fd_copy;

	return 0;

fail_fd_copy:
	process_remove_file(proc, kfds[1]);
fail_add_writer:
	process_remove_file(proc, kfds[0]);
fail_add_reader:
	vfs_file_destroy(w);
fail_writer:
	vfs_file_destroy(r);
fail_reader:
	pipe_destroy(p);

	return err;
}
