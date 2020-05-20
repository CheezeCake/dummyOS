/**
 * @brief ramfs file system
 *
 * Simple read-only in-memory file system that uses the "UStar" format.
 * https://en.wikipedia.org/wiki/Tar_(computing)#UStar_format
 */

#include <dummyos/compiler.h>
#include <dummyos/errno.h>
#include <fs/device.h>
#include <fs/file.h>
#include <fs/filesystem.h>
#include <fs/inode.h>
#include <fs/superblock.h>
#include <fs/vfs.h>
#include <kernel/kmalloc.h>
#include <kernel/types.h>
#include <libk/libk.h>
#include <libk/list.h>
#include <libk/utils.h>

#include <kernel/log.h>

#define TMAGIC		"ustar\0"
#define BLOCK_SIZE	512

/**
 * @brief UStar header
 *
 * 512 bytes long file header
 */
struct ustar_header
{
	int8_t name[100];
	int8_t mode[8];
	int8_t uid[8];
	int8_t gid[8];
	int8_t size[12];
	int8_t mtime[12];
	int8_t cksum[8];
	int8_t typeflag;
	int8_t linkname[100];
	int8_t magic[6];
	int8_t version[2];
	int8_t uname[32];
	int8_t gname[32];
	int8_t devmajor[8];
	int8_t devminor[8];
	int8_t prefix[155];

	int8_t padding[12];
};
static_assert(sizeof(struct ustar_header) == BLOCK_SIZE,
	      "sizeof(struct ustar_header)");

/*
 * ustar_header::typeflag values
 */
#define REGTYPE 	'0' 	/**< Regular file. */
#define AREGTYPE 	'\0' 	/**< Regular file. */
#define LNKTYPE 	'1' 	/**< Link. */
#define SYMTYPE 	'2' 	/**< Reserved. */
#define CHRTYPE 	'3' 	/**< Character special. */
#define BLKTYPE 	'4' 	/**< Block special. */
#define DIRTYPE 	'5' 	/**< Directory. */
#define FIFOTYPE 	'6' 	/**< FIFO special. */
#define CONTTYPE 	'7' 	/**< Reserved. */

static inline enum vfs_node_type ustar2vfs_type(uint8_t type)
{
	switch (type) {
		case REGTYPE:
		case AREGTYPE:
			return REGULAR;
			/* case LNKTYPE: */
		case SYMTYPE:
			return SYMLINK;
		case CHRTYPE:
			return CHARDEV;
		case BLKTYPE:
			return BLOCKDEV;
		case DIRTYPE:
			return DIRECTORY;
		case FIFOTYPE:
			return FIFO;
		default:
			return -1;
	}
}

/*
 * ustar_header
 */
static inline size_t name_field_len(const char* name, size_t field_size)
{
	return (name[field_size - 1] == '\0') ? strlen(name) : field_size;
}

static inline size_t ustar_header_prefix_len(const struct ustar_header* h)
{
	return name_field_len((const char*)h->prefix, sizeof(h->prefix));
}

static inline size_t ustar_header_name_len(const struct ustar_header* h)
{
	return name_field_len((const char*)h->name, sizeof(h->name));
}

static inline size_t ustar_header_linkname_len(const struct ustar_header* h)
{
	return name_field_len((const char*)h->linkname, sizeof(h->linkname));
}

	static inline struct ustar_header*
ustar_header_get_next_header(const struct ustar_header* header)
{
	size_t file_size = strntol((const char*)header->size, sizeof(header->size),
				   NULL, 8);
	return (struct ustar_header*)((int8_t*)(header + 1) +
				      align_up(file_size, BLOCK_SIZE));
}

static inline bool ustar_header_valid(const struct ustar_header* header)
{
	// TODO: checksum
	return (memcmp(header->magic, TMAGIC, sizeof(header->magic)) == 0);
}

#define USTAR_FULLNAME_MAX_LEN (member_size(struct ustar_header, prefix) + \
				member_size(struct ustar_header, name))

static inline void ustar_header_fullname_len(const struct ustar_header* h,
					     size_t* prefix_len,
					     size_t* name_len)
{
	if (prefix_len)
		*prefix_len = (h) ? ustar_header_prefix_len(h) : 0;
	if (name_len)
		*name_len = (h) ? ustar_header_name_len(h) : 0;
}

/**
 * Concatenates prefix, name and an optional path.
 *
 * @param fullname result string
 * @param fullname_len result string length
 * @param append optional path to append to result
 */
static int ustar_header_fullname_init(char* fullname, size_t fullname_len,
				      const struct ustar_header* h,
				      const vfs_path_t* append)
{
	size_t prefix_len;
	size_t name_len;
	size_t append_len = 0;
	size_t full_len;

	ustar_header_fullname_len(h, &prefix_len, &name_len);
	if (append)
		append_len = vfs_path_get_size(append);
	full_len = prefix_len + name_len + append_len + 2;

	memset(fullname, 0, fullname_len);

	if (fullname_len >= full_len) {
		if (h) {
			strncpy(fullname, (const char*)h->prefix, prefix_len);
			strncat(fullname, (const char*)h->name, name_len);
		}
		if (append) {
			strncat(fullname, "/", 1);
			strncat(fullname, vfs_path_get_str(append), append_len);
		}

		return 0;
	}

	return -ERANGE;
}

/**
 * Concatenates prefix, name and an optional path.
 *
 * @param fullname result string
 * @param append optional path to append to result
 */
static int ustar_header_fullname_create(const struct ustar_header* h,
					const vfs_path_t* append,
					char** fullname)
{
	size_t prefix_len;
	size_t name_len;
	size_t append_len = 0;
	size_t full_len;
	int err = -ENOMEM;

	ustar_header_fullname_len(h, &prefix_len, &name_len);
	if (append)
		append_len = vfs_path_get_size(append) + 1;
	full_len = prefix_len + name_len + append_len + 1;

	*fullname = kmalloc(full_len);

	if (*fullname) {
		err = ustar_header_fullname_init(*fullname, full_len, h, append);
		if (err)
			kfree(*fullname);
	}

	return err;
}

static int ustar_header_fullname_init_path_init(vfs_path_t* fullname_path,
						char* fullname,
						size_t fullname_len,
						const struct ustar_header* h,
						const vfs_path_t* append)
{
	int err;

	err = ustar_header_fullname_init(fullname, fullname_len, h, append);
	if (!err)
		err = vfs_path_init(fullname_path, fullname, strlen(fullname));

	return err;
}

static inline bool ustar_header_is_device(const struct ustar_header* header)
{
	return (header->typeflag == CHRTYPE || header->typeflag == BLKTYPE);
}


/**
 * @brief File system dependent inode
 */
struct ramfs_inode_info
{
	struct ustar_header* header;

	void* data;
	size_t data_size;

	struct vfs_inode inode; /**< enclosed vfs_inode */
};

/** Accessor to enclosing ramfs_inode_info */
static inline struct ramfs_inode_info*
get_ramfs_inode(struct vfs_inode* vfs_inode)
{
	return container_of(vfs_inode, struct ramfs_inode_info, inode);
}


// objects declaration
static struct vfs_filesystem ramfs;
static struct vfs_superblock_operations ramfs_superblock_op;
static struct vfs_inode_operations ramfs_inode_op;
static struct vfs_file_operations ramfs_file_op;

static int ramfs_inode_info_create(struct ustar_header* header,
				   struct vfs_superblock* sb,
				   struct ramfs_inode_info** result)
{
	struct ramfs_inode_info* ramfs_inode =
		kmalloc(sizeof(struct ramfs_inode_info));
	if (!ramfs_inode)
		return -ENOMEM;

	memset(ramfs_inode, 0, sizeof(struct ramfs_inode_info));

	ramfs_inode->header = header;
	if (header) {
		ramfs_inode->data = header + 1;
		ramfs_inode->data_size = strntol((const char*)header->size,
						 sizeof(header->size), NULL, 8);
	}

	// create inode
	vfs_inode_init(&ramfs_inode->inode, DIRECTORY, sb, &ramfs_inode_op);

	*result = ramfs_inode;

	return 0;
}

static int ramfs_vfs_inode_create(struct ustar_header* header,
				  struct vfs_superblock* sb,
				  struct vfs_inode** result)
{
	struct ramfs_inode_info* ramfs_inode;
	int err;

	err = ramfs_inode_info_create(header, sb, &ramfs_inode);
	if (!err)
		*result = &ramfs_inode->inode;

	return err;
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
	vfs_path_t root_path;

	// create inode
	err = ramfs_inode_info_create(NULL, sb, &ramfs_inode);
	if (err)
		return err;

	err = vfs_path_init(&root_path, ".", 1);
	if (err)
		return err;

	// refs the inode
	err = vfs_cache_node_create(&ramfs_inode->inode, &root_path, result);
	if (err)
		goto out;
	vfs_inode_unref(&ramfs_inode->inode); // unref inode

out:
	vfs_path_reset(&root_path);

	return err;
}

/*
 * vfs_filesystem
 */
static int ramfs_superblock_create(struct vfs_filesystem* this,
				   struct vfs_cache_node* device,
				   int flags, void* data,
				   struct vfs_superblock** result)
{
	int err = -ENOMEM;
	struct vfs_superblock* sb = NULL;
	struct vfs_cache_node* root = NULL;

	// data = ptr to fs in memory
	if (!data)
		return -EINVAL;

	sb = kmalloc(sizeof(struct vfs_superblock));
	if (!sb)
		goto fail;

	err = create_root_node(sb, &root); // refs root
	if (!err) {
		vfs_superblock_init(sb, device,  &ramfs, root, data,
				    &ramfs_superblock_op);
		vfs_cache_node_unref(root); // unref root
		*result = sb;

		return 0;
	}

fail:
	kfree(sb);

	*result = NULL;

	return err;
}

static int ramfs_superblock_destroy(struct vfs_filesystem* this,
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
static int ramfs_alloc_inode(struct vfs_superblock* this,
			     struct vfs_inode** result)
{
	return -EROFS;
}

static int ramfs_free_inode(struct vfs_superblock* this,
			    struct vfs_inode* inode)
{
	return -EROFS;
}


static void inode_read_dev(struct ramfs_inode_info* ramfs_inode,
			   struct vfs_inode* inode)
{
	uint64_t major = strntol((const char*)ramfs_inode->header->devmajor,
				 sizeof(ramfs_inode->header->devmajor), NULL, 8);
	uint64_t minor = strntol((const char*)ramfs_inode->header->devminor,
				 sizeof(ramfs_inode->header->devminor), NULL, 8);

	vfs_inode_init_dev(inode, major, minor);
}

static int ramfs_read_inode(struct vfs_superblock* this,
			    struct vfs_inode* inode)
{
	struct ramfs_inode_info* ramfs_inode = get_ramfs_inode(inode);
	int enum_type = ustar2vfs_type(ramfs_inode->header->typeflag);

	// unsuported type
	if (enum_type < 0)
		return -EINVAL;

	vfs_inode_init(inode, enum_type, this, &ramfs_inode_op);
	inode->size = strntol((const char*)ramfs_inode->header->size,
			      sizeof(ramfs_inode->header->size), NULL, 8);

	if (ustar_header_is_device(ramfs_inode->header))
		inode_read_dev(ramfs_inode, inode);

	return 0;
}

/*
 * inode operations
 */
static int ramfs_open_inode(struct vfs_inode* this, struct vfs_file* file)
{
	file->op = &ramfs_file_op;
	return 0;
}

static int lookup_fullname(const struct vfs_inode* this,
			   struct ustar_header* fh,
			   const vfs_path_t* fullname,
			   struct vfs_inode** result)
{
	size_t cur_fullname_str_len = USTAR_FULLNAME_MAX_LEN + 1;
	char* cur_fullname_str = NULL;
	bool found = false;
	int err = 0;

	cur_fullname_str = kmalloc(cur_fullname_str_len);
	if (!cur_fullname_str)
		return -ENOMEM;

	while (!err && !found && fh && ustar_header_valid(fh)) {
		vfs_path_t fullname_cur;

		err = ustar_header_fullname_init_path_init(&fullname_cur,
							   cur_fullname_str,
							   cur_fullname_str_len,
							   fh, NULL);
		if (!err) {
			found = vfs_path_same(fullname, &fullname_cur);
			if (!found)
				fh = ustar_header_get_next_header(fh);

			vfs_path_reset(&fullname_cur);
		}
	}

	kfree(cur_fullname_str);

	if (err)
		return err;

	return (found && fh) ? ramfs_vfs_inode_create(fh, this->sb, result) : -ENOENT;
}

static int ustar_header_dirname_basename(struct ustar_header* fh, char* buf,
					 size_t buf_size, vfs_path_t* dirname,
					 vfs_path_t* basename)
{
	vfs_path_t fullname;
	int err;

	err = ustar_header_fullname_init(buf, buf_size, fh, NULL);
	if (err)
		return err;

	err = vfs_path_init(&fullname, buf, strlen(buf));
	if (err)
		return err;

	if (dirname) {
		err = vfs_path_dirname(&fullname, dirname);
	}
	if (!err && basename) {
		err = vfs_path_basename(&fullname, basename);
	}

	vfs_path_reset(&fullname);

	return err;
}

static int ramfs_lookup(struct vfs_inode* this, const vfs_path_t* name,
			struct vfs_inode** result)
{
	vfs_path_t this_fullname;
	int err;
	struct ramfs_inode_info* this_ramfs_inode = get_ramfs_inode(this);
	struct ustar_header* h = this_ramfs_inode->header;
	char* this_fullname_str;

	err = ustar_header_fullname_create(h, name, &this_fullname_str);
	if (err)
		return err;

	err = vfs_path_init(&this_fullname, this_fullname_str,
			    strlen(this_fullname_str));
	kfree(this_fullname_str);

	if (!err) {
		// root->header == NULL
		struct ustar_header* next =
			(h) ? ustar_header_get_next_header(h) : this->sb->data;
		err = lookup_fullname(this, next, &this_fullname, result);

		vfs_path_reset(&this_fullname);
	}

	return err;
}

static int ramfs_readlink(struct vfs_inode* this, vfs_path_t** result)
{
	const struct ustar_header* header = get_ramfs_inode(this)->header;
	int err;

	err = vfs_path_create((const char*)header->linkname,
			      ustar_header_linkname_len(header), result);

	return err;
}

static void ramfs_destroy(struct vfs_inode* this)
{
	ramfs_node_info_destroy(get_ramfs_inode(this));
}

/*
 * file operations
 */
int ramfs_open(struct vfs_file* this, int flags)
{
	return 0;
}

int ramfs_close(struct vfs_file* this)
{
	return 0;
}

static int ramfs_readdir(struct vfs_file* this,
			 void (*add_entry)(struct vfs_file*,
					   struct vfs_cache_node*))
{
	struct vfs_inode* inode = vfs_file_get_inode(this);
	struct ustar_header* fh = get_ramfs_inode(inode)->header;
	const vfs_path_t* dirname = &vfs_file_get_cache_node(this)->name;
	vfs_path_t cur_dirname;
	vfs_path_t cur_basename;
	const size_t fullname_str_len = USTAR_FULLNAME_MAX_LEN + 1;
	char* fullname_str;
	size_t offset = 0;
	bool done = false;
	int err = 0;

	fullname_str = kmalloc(fullname_str_len);
	if (!fullname_str)
		return -ENOMEM;

	// root->header == NULL
	fh = (fh) ? ustar_header_get_next_header(fh) : inode->sb->data;
	if (!fh)
		err = -EINVAL;

	while (!err && !done && fh && ustar_header_valid(fh)) {
		err = ustar_header_dirname_basename(fh, fullname_str, fullname_str_len,
						    &cur_dirname, &cur_basename);
		if (err)
			continue;

		if (vfs_path_same(dirname, &cur_dirname)) {
			struct vfs_cache_node* cnode =
				vfs_cache_node_lookup_child(this->cnode, &cur_basename);
			if (!cnode) {
				struct vfs_inode* inode;
				err = ramfs_vfs_inode_create(fh, this->cnode->inode->sb, &inode);
				if (!err) {
					err = vfs_read_and_cache_inode(this->cnode, inode,
								       &cur_basename, &cnode);
				}
			}

			if (cnode) {
				if (!err)
					add_entry(this, cnode);
				vfs_cache_node_unref(cnode);
			}
		}

		fh = ustar_header_get_next_header(fh);

		vfs_path_reset(&cur_dirname);
		vfs_path_reset(&cur_basename);
	}

	kfree(fullname_str);

	return (err) ? err : offset;
}


off_t ramfs_lseek(struct vfs_file* this, off_t offset, int whence)
{
	off_t new;
	struct ramfs_inode_info* ramfs_inode =
		get_ramfs_inode(vfs_file_get_inode(this));

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

ssize_t ramfs_read(struct vfs_file* this, void* buf, size_t count)
{
	struct vfs_inode* inode = vfs_file_get_inode(this);

	if (inode->type == DIRECTORY)
		return -EISDIR;

	const struct ramfs_inode_info* ramfs_inode = get_ramfs_inode(inode);
	size_t left = ramfs_inode->data_size - this->cur;

	if (count > left)
		count = left;

	memcpy(buf, (int8_t*)ramfs_inode->data + this->cur, count);

	return count;
}

ssize_t ramfs_write(struct vfs_file* this, const void* buf, size_t count)
{
	return -EROFS;
}

/*
 * objects initialization
 */
static struct vfs_filesystem ramfs = {
	.name				= "ramfs",
	.superblock_create	= ramfs_superblock_create,
	.superblock_destroy = ramfs_superblock_destroy,
	.fs_list			= LIST_NODE_NULL
};

static struct vfs_superblock_operations ramfs_superblock_op = {
	.alloc_inode	= ramfs_alloc_inode,
	.free_inode		= ramfs_free_inode,
	.read_inode		= ramfs_read_inode
};

static struct vfs_inode_operations ramfs_inode_op = {
	.open		= ramfs_open_inode,
	.lookup		= ramfs_lookup,
	.readlink	= ramfs_readlink,
	.destroy	= ramfs_destroy
};

static struct vfs_file_operations ramfs_file_op = {
	.open		= ramfs_open,
	.close		= ramfs_close,
	.readdir	= ramfs_readdir,
	.lseek		= ramfs_lseek,
	.read		= ramfs_read,
	.write		= ramfs_write,
};

int ramfs_init_and_register(void)
{
	return vfs_filesystem_register(&ramfs);
}
