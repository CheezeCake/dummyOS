#include <dummyos/fcntl.h>
#include <fs/file.h>
#include <kernel/kmalloc.h>
#include <kernel/locking/mutex.h>
#include <kernel/sched/wait.h>
#include <libk/circ_buf.h>
#include <libk/libk.h>
#include <libk/refcount.h>

#define PIPE_BUFFER_SIZE 128

static struct vfs_file_operations fifo_r_fops;
static struct vfs_file_operations fifo_w_fops;

struct fifo
{
	CIRC_BUF_DEFINE1(buffer, PIPE_BUFFER_SIZE);

	mutex_t lock;
	wait_queue_t wq;

	refcount_t readers;
	refcount_t writers;
};

static void fifo_init(struct fifo* f)
{
	circ_buf_init(&f->buffer, f->circ_buf_buffer(buffer), PIPE_BUFFER_SIZE);

	mutex_init(&f->lock);
	wait_init(&f->wq);

	refcount_init(&f->readers);
	refcount_init(&f->writers);
}

static int fifo_create(struct fifo** result)
{
	struct fifo* f;

	f = kmalloc(sizeof(struct fifo));
	if (!f)
		return -ENOMEM;

	fifo_init(f);

	*result = f;

	return 0;
}

static inline void fifo_reset(struct fifo* f)
{
	mutex_destroy(&f->lock);
	wait_reset(&f->wq);

	memset(f, 0, sizeof(struct fifo));
}

static void fifo_destroy(struct fifo* f)
{
	fifo_reset(f);
}

#define fifo_add(f, type) refcount_inc(&(f)->type)

#define _fifo_add_reader(f) fifo_add(f, readers)
#define _fifo_add_writer(f) fifo_add(f, writers)

static inline void fifo_remove(void** fptr, refcount_t* type,
							   const refcount_t* other_type)
{
	if (refcount_dec(type) == 0) {
		if (refcount_get(other_type) == 0) {
			fifo_destroy(*fptr);
			*fptr = NULL;
		}
		else {
			// no more readers/writers
			// wake so the others are notified
			wait_wake_all(&((struct fifo*)*fptr)->wq);
		}
	}
}

static void fifo_add_reader(struct vfs_inode* fifo)
{
	struct fifo* f = fifo->private_data;

	if (f)
		_fifo_add_reader(f);
}

static void fifo_remove_reader(struct vfs_inode* fifo)
{
	struct fifo* f = fifo->private_data;

	if (f)
		fifo_remove(&fifo->private_data, &f->readers, &f->writers);
}

static void fifo_add_writer(struct vfs_inode* fifo)
{
	struct fifo* f = fifo->private_data;

	if (f)
		_fifo_add_writer(f);
}

static void fifo_remove_writer(struct vfs_inode* fifo)
{
	struct fifo* f = fifo->private_data;

	if (f)
		fifo_remove(&fifo->private_data, &f->writers, &f->readers);
}


// TODO: implement
static int generate_sigpipe(void)
{
	return 0;
}


/*
 * file operations
 */
static int fifo_open(struct vfs_inode* inode, int flags, struct vfs_file* file)
{
	if (!inode->private_data)
		return -EINVAL;

	if (flags & O_RDWR)
		return -EPERM;

	if (flags & O_RDONLY) {
		inode->fops = &fifo_r_fops;
		fifo_add_reader(inode);
	}
	else if (flags & O_WRONLY) {
		inode->fops = &fifo_w_fops;
		fifo_add_writer(inode);
	}
	else {
		return -EINVAL; /* */
	}

	return 0;
}

static int fifo_close(struct vfs_file* this)
{
	struct vfs_inode* inode = vfs_file_get_inode(this);

	if (!inode->private_data)
		return -EINVAL;

	if (inode->fops == &fifo_r_fops)
		fifo_remove_reader(inode);
	else if (inode->fops == &fifo_w_fops)
		fifo_remove_writer(inode);
	else
		return -EINVAL;

	return 0;
}

static ssize_t fifo_read(struct vfs_file* this, void* buf, size_t count)
{
	struct fifo* f = vfs_file_get_inode(this)->private_data;
	uint8_t* buffer = buf;
	size_t n = 0;
	uint8_t c;
	int err;

	if (!f)
		return -EIO;

	while (n < count) {
		mutex_lock(&f->lock);

		if (circ_buf_empty(&f->buffer)) {
			mutex_unlock(&f->lock);

			if (refcount_get(&f->writers) == 0) // no writers
				return n;

			wait_wait(&f->wq);
		}
		else {
			err = circ_buf_pop(&f->buffer, &c);
			mutex_unlock(&f->lock);

			if (err)
				return n;

			buffer[n++] = c;
		}
	}

	return n;
}

static ssize_t fifo_write(struct vfs_file* this, const void* buf, size_t count)
{
	struct fifo* f = vfs_file_get_inode(this)->private_data;
	const uint8_t* buffer = buf;
	size_t n = 0;
	int err;

	if (!f)
		return -EIO;

	while (n < count) {
		mutex_lock(&f->lock);

		if (circ_buf_full(&f->buffer)) {
			mutex_unlock(&f->lock);

			if (refcount_get(&f->readers) == 0) // no readers
				return generate_sigpipe();
		}
		else {
			err = circ_buf_push(&f->buffer, buffer[n]);
			mutex_unlock(&f->lock);

			if (err)
				return n;

			++n;
		}
	}

	return n;
}

static struct vfs_file_operations fifo_r_fops = {
	.open	= fifo_open,
	.close	= fifo_close,
	.lseek	= NULL,
	.read	= fifo_read,
	.write	= NULL,
	.ioctl	= NULL,
};

static struct vfs_file_operations fifo_w_fops = {
	.open	= fifo_open,
	.close	= fifo_close,
	.lseek	= NULL,
	.read	= NULL,
	.write	= fifo_write,
	.ioctl	= NULL,
};

int pipe_create(struct vfs_file** r, struct vfs_file** w)
{
	struct fifo* f;
	int err;

	err = fifo_create(&f);
	if (err)
		return err;

	err = vfs_file_create(&fifo_r_fops, NULL, 0, r);
	if (err)
		return err;

	err = vfs_file_create(&fifo_w_fops, NULL, 0, w);
	if (err) {
		vfs_file_destroy(*r);
		return err;
	}

	return err;
}

int fifo_dup(struct vfs_file* fifo)
{
	struct vfs_inode* inode = vfs_file_get_inode(fifo);
	int err;

	if (!inode->private_data)
		return -EIO;

	err = fifo_open(inode, fifo->flags, fifo);

	return err;
}

struct vfs_file_operations* fifo_get_fops(void)
{
	return &fifo_r_fops;
}
