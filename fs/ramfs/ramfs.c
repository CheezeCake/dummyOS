/**
 * @brief ramfs file system
 *
 * Simple read-only in-memory file system that uses the "ar" format with
 * SystemV/GNU extensions.
 * https://en.wikipedia.org/wiki/Ar_(Unix)
 */

#include <fs/file.h>
#include <fs/filesystem.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/types.h>
#include <libk/libk.h>
#include <libk/list.h>
#include <libk/utils.h>

#include <kernel/log.h>

#define AR_MAGIC		"!<arch>\n"
#define AR_MAGIC_SIZE	8

#define AR_MAX_REGULAR_FILENAME_LENGTH 15

/**
 * @brief AR header
 *
 * 60 bytes long file header
 */
struct ar_header
{
	char name[16];
	char date[12];
	char uid[6];
	char gid[6];
	char mode[8];
	char size[10];
	char end[2];
};

/**
 * @brief Ramfs superblock data
 */
struct ramfs_superblock_data
{
	/** start of ar archive */
	void* start;

	void* long_filenames_data;
	size_t long_filenames_size;

	struct ar_header* first_file_header;
};

/**
 * @brief File system dependent inode
 */
struct ramfs_inode_info
{
	void* header_start;

	void* data_start;
	size_t data_size;

	struct vfs_inode inode; /**< enclosed vfs_inode */
};

/** Accessor to enclosing ramfs_inode_info */
#define get_ramfs_inode(vfs_inode) \
	container_of(vfs_inode, struct ramfs_inode_info, inode)


// objects declaration
static struct vfs_filesystem ramfs;
static struct vfs_superblock_operations ramfs_superblock_op;
static struct vfs_inode_operations ramfs_inode_op;
static struct vfs_file_operations ramfs_file_op;

static int ramfs_node_info_create(struct ar_header* header,
								  struct vfs_superblock* sb,
								  struct ramfs_inode_info** result)
{
	struct ramfs_inode_info* ramfs_inode =
		kmalloc(sizeof(struct ramfs_inode_info));
	if (!ramfs_inode)
		return -ENOMEM;

	memset(ramfs_inode, 0, sizeof(struct ramfs_inode_info));

	ramfs_inode->header_start = header;
	if (header) {
		ramfs_inode->data_start = header + 1;
		ramfs_inode->data_size = strntol(header->size, sizeof(header->size),
										 NULL, 10);
	}

	// create inode
	vfs_inode_init(&ramfs_inode->inode, DIRECTORY, sb, &ramfs_inode_op);

	*result = ramfs_inode;

	return 0;
}

static void ramfs_node_info_destroy(struct ramfs_inode_info* inode)
{
	kfree(inode);
}

static int create_root_node(struct vfs_superblock* sb,
							struct vfs_cache_node** result)
{
	int err;
	struct ramfs_inode_info* ramfs_inode = NULL;

	// create inode
	err = ramfs_node_info_create(NULL, sb, &ramfs_inode);
	if (!err) {
		// grabs the inode
		err = vfs_cache_node_create(&ramfs_inode->inode, NULL, result);
		vfs_inode_unref(&ramfs_inode->inode); // drop inode
	}

	return err;
}

static size_t ar_filename_len(const char* filename)
{
	const char* str = filename;
	while (*str != '/')
		++str;

	return (str - filename);
}

static inline struct ar_header*
ar_get_next_header(struct ar_header* header, void* archive_start)
{
	const size_t file_size = strntol(header->size, sizeof(header->size),
									 NULL, 10);
	void* next_header = (void*)(header + 1) + file_size;
	const ptrdiff_t offset = next_header - archive_start;

	// "Each archive file member begins on an even byte boundary;
	// a newline is inserted between files if necessary"
	if (offset % 2 == 1)
		++next_header;

	return next_header;
}

static const char*
ar_get_long_filename(const struct ar_header* header,
					 const struct ramfs_superblock_data* sb_data)
{
	if (!sb_data->long_filenames_data)
		return NULL;

	const size_t offset = strntol(header->name + 1, sizeof(header->name) - 1,
								  NULL, 10);
	return (offset > sb_data->long_filenames_size)
		? NULL : (sb_data->long_filenames_data + offset);
}

static inline bool ar_header_valid(const struct ar_header* header)
{
	return (header->end[0] == '`' && header->end[1] == '\n');
}

static inline bool ar_check_magic(const void* start)
{
	return (strncmp(start, AR_MAGIC, AR_MAGIC_SIZE) == 0);
}

static inline bool ar_long_filename_header(const struct ar_header* header)
{
	return (header->name[0] == '/' && header->name[1] == '/');
}

static void superblock_data_init(struct ramfs_superblock_data* sb_data,
								 void* start)
{
	struct ar_header* first_header = start + AR_MAGIC_SIZE;

	memset(sb_data, 0, sizeof(struct ramfs_superblock_data));

	sb_data->start = start;

	if (ar_long_filename_header(first_header)) {
		sb_data->long_filenames_data = first_header + 1;
		sb_data->long_filenames_size = strntol(first_header->size,
											   sizeof(first_header->size),
											   NULL, 10);
		sb_data->first_file_header =
			sb_data->long_filenames_data + sb_data->long_filenames_size;
	}
	else {
		sb_data->first_file_header = first_header;
	}
}

/*
 * vfs_filesystem
 */
static int superblock_create(struct vfs_filesystem* this,
							 struct vfs_cache_node* device,
							 int flags, void* data,
							 struct vfs_superblock** result)
{
	int err = -ENOMEM;
	struct vfs_superblock* sb = NULL;
	struct ramfs_superblock_data* sb_data = NULL;

	// data = ptr to fs in memory
	if (!ar_check_magic(data))
		return -EINVAL;

	sb = kmalloc(sizeof(struct vfs_superblock));
	if (!sb)
		goto fail;

	sb_data = kmalloc(sizeof(struct ramfs_superblock_data));
	if (!sb_data)
		goto fail;

	memset(sb, 0, sizeof(struct vfs_superblock));
	superblock_data_init(sb_data, data);

	err = create_root_node(sb, &sb->root);
	if (err)
		goto fail;
	sb->device = device;
	sb->fs = &ramfs;
	sb->data = sb_data;
	sb->op = &ramfs_superblock_op;

	*result = sb;

	return 0;

fail:
	kfree(sb);
	kfree(sb_data);

	return err;
}

static int superblock_destroy(struct vfs_filesystem* this,
							  struct vfs_superblock* sb)
{
	if (vfs_cache_node_get_ref(sb->root) > 1)
		return -EBUSY;

	vfs_cache_node_unref(sb->root);
	kfree(sb->data);
	kfree(sb);

	return 0;
}

/*
 * superblock operations
 */
static int alloc_inode(struct vfs_superblock* this, struct vfs_inode** result)
{
	return -EROFS;
}

static int free_inode(struct vfs_superblock* this, struct vfs_inode* inode)
{
	return -EROFS;
}

static int read_inode(struct vfs_superblock* this, struct vfs_inode* inode)
{
	vfs_inode_init(inode, REGULAR, this, &ramfs_inode_op);
	return 0;
}

/*
 * inode operations
 */
static int lookup(struct vfs_inode* this, const vfs_path_t* name,
								struct vfs_inode** result)
{
	if (this->type != DIRECTORY)
		return -ENOTDIR;

	const struct ramfs_superblock_data* sb_data = this->sb->data;
	struct ar_header* fh = sb_data->first_file_header;
	bool found = false;

	while (!found && ar_header_valid(fh)) {
		const char* filename = fh->name;
		if (filename[0] == '/')
			filename = ar_get_long_filename(fh, sb_data);

		if (filename && vfs_path_name_str_equals(name, filename,
												 ar_filename_len(filename))) {
			found = true;
		}
		else {
			struct ar_header* next = ar_get_next_header(fh, sb_data->start);
			if (next == fh)
				break; // avoid infinite loop
			fh = next;
		}
	}

	if (found) {
		struct ramfs_inode_info* rfs_node;
		int err = ramfs_node_info_create(fh, this->sb, &rfs_node);
		if (err)
			return err;

		*result = &rfs_node->inode;

		return 0;
	}

	return -ENOENT;
}

static int readlink(struct vfs_inode* this, vfs_path_t** result)
{
	return -EINVAL; // symlinks are not supported
}

static int open(struct vfs_inode* this, struct vfs_cache_node* node,
			int flags, struct vfs_file* file)
{
	file->op = &ramfs_file_op;
	return 0;
}

static int close(struct vfs_inode* this, struct vfs_file* file)
{
	return 0;
}

static void destroy(struct vfs_inode* this)
{
	ramfs_node_info_destroy(get_ramfs_inode(this));
}

/*
 * file operations
 */
off_t lseek(struct vfs_file* this, off_t offset, int whence)
{
	off_t new;
	struct ramfs_inode_info* ramfs_inode = get_ramfs_inode(this->node->inode);

	switch (whence) {
		case SEEK_CUR:
			new = this->cur + offset;
			if (new < this->cur) // negative result
				return -EINVAL;
			break;
		case SEEK_SET:
			new = offset;
			break;
		case SEEK_END:
			new = ramfs_inode->data_size - offset - 1;
			if (new > ramfs_inode->data_size) // overflow
				return -EOVERFLOW;
			break;
		default:
			return -EINVAL;
	}

	this->cur = new;

	return new;
}

ssize_t read(struct vfs_file* this, void* buf, size_t count)
{
	if (this->node->inode->type == DIRECTORY)
		return -EISDIR;

	const struct ramfs_inode_info* ramfs_inode =
		get_ramfs_inode(this->node->inode);
	size_t left = ramfs_inode->data_size - this->cur;

	if (count > left)
		count = left;
	if (count > SSIZE_MAX)
		count = SSIZE_MAX;

	memcpy(buf, ramfs_inode->data_start + this->cur, count);

	return count;
}

ssize_t write(struct vfs_file* this, void* buf, size_t count)
{
	return -EROFS;
}

/*
 * objects initialization
 */
static struct vfs_filesystem ramfs = {
	.name = "ramfs",
	.superblock_create = superblock_create,
	.superblock_destroy = superblock_destroy,
	.fs_list = LIST_NODE_NULL
};

static struct vfs_superblock_operations ramfs_superblock_op = {
	.alloc_inode = alloc_inode,
	.free_inode = free_inode,
	.read_inode = read_inode
};

static struct vfs_inode_operations ramfs_inode_op = {
	.lookup = lookup,
	.readlink = readlink,
	.open = open,
	.close = close,
	.destroy = destroy
};

static struct vfs_file_operations ramfs_file_op = {
	.lseek = lseek,
	.read = read,
	.write = write,
};

int ramfs_init_and_register(void)
{
	return vfs_filesystem_register(&ramfs);
}
