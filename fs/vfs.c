#include <fs/filesystem.h>
#include <fs/inode.h>
#include <fs/vfs.h>
#include <kernel/errno.h>
#include <kernel/types.h>
#include <libk/list.h>

#include <kernel/log.h>

#define VFS_LOOKUP_MAX_RECURSION_LEVEL 16

/**
 * List of currently mounted @ref vfs_superblock
 */
static list_t mounted_list;

static int get_superblock(struct vfs_cache_node* device, const char* filesystem,
						  void* data, struct vfs_superblock** sb);
static int lookup_path(const vfs_path_t* path, struct vfs_cache_node* root,
					   struct vfs_cache_node* cwd, struct vfs_cache_node** result,
					   unsigned int recursion_level);

#include <fs/ramfs/ramfs.h>
/* extern int8_t _binary_archive_start; */

static int mount_root(void)
{
	/* int err; */
	/* struct vfs_superblock* sb; */

	/* err = get_superblock(NULL, "ramfs", &_binary_archive_start, &sb); */
	/* if (!err) */
	/* 	err = vfs_cache_init(sb->root); */

	/* return err; */

	return -ENODEV;
}

int vfs_init(void)
{
	list_init(&mounted_list);
	// parse cmd line?
	return mount_root();
}

static int get_superblock(struct vfs_cache_node* device, const char* filesystem,
						  void* data, struct vfs_superblock** sb)
{
	if (device && device->inode->type != DEVBLOCK)
		return -ENOTBLK;

	struct vfs_filesystem* fs = vfs_filesystem_get_fs(filesystem);
	if (!fs)
		return -ENODEV;

	return fs->superblock_create(fs, device, 0, data, sb);
}

int vfs_mount(struct vfs_cache_node* device, struct vfs_cache_node* mountpoint,
			  void* data, const char* filesystem)
{
	int err;
	struct vfs_superblock* sb;

	err = get_superblock(device, filesystem, data, &sb);
	if (err)
		return err;

	list_push_back(&mounted_list, &sb->mounted_list);
	sb->root->mountpoint = mountpoint;
	mountpoint->mounted = sb->root;

	return 0;
}

int vfs_umount(struct vfs_cache_node* mountpoint)
{
	struct vfs_cache_node* root = mountpoint->mounted;
	// not a mountpoint
	if (!root)
		return -EINVAL;

	struct vfs_superblock* sb = root->inode->sb;
	list_erase(&mounted_list, &sb->mounted_list);
	root->mountpoint = NULL;
	mountpoint->mounted = NULL;

	return sb->fs->superblock_destroy(sb->fs, sb);
}

static int readlink(const struct vfs_cache_node* symlink,
					struct vfs_cache_node* root,
					struct vfs_cache_node** target,
					unsigned int recursion_level)
{
	if (symlink->inode->type != SYMLINK)
		return -EINVAL;

	vfs_path_t target_path;
	vfs_path_t* target_path_p = &target_path;

	int err = symlink->inode->op->readlink(symlink->inode, &target_path_p);
	if (err)
		return err;

	struct vfs_cache_node* const start =
		(vfs_path_absolute(target_path_p))
		? root
		: vfs_cache_node_get_parent(symlink);

	return lookup_path(target_path_p, start, root, target, recursion_level + 1);
}

/**
 * Fetch an inode, insert it in the cache
 */
static int read_and_cache_inode(struct vfs_cache_node* parent,
								const vfs_path_t* lookup_pathname,
								struct vfs_cache_node** cached_result)
{
	struct vfs_inode* parent_inode = parent->inode;
	struct vfs_inode* inode = NULL;
	int err;

	err = parent_inode->op->lookup(parent_inode, lookup_pathname, &inode);
	if (err)
		return err;

	err = inode->sb->op->read_inode(inode->sb, inode);
	if (err)
		return err;

	return vfs_cache_node_insert_child(parent, inode, lookup_pathname,
									   cached_result);
}

static int lookup(vfs_path_component_t* path_component, struct vfs_cache_node* start,
				  struct vfs_cache_node* root, struct vfs_cache_node** result,
				  unsigned int recursion_level)
{
	if (recursion_level >= VFS_LOOKUP_MAX_RECURSION_LEVEL)
		return -ELOOP;

	if (!start || !root)
		return -EINVAL;

	vfs_path_t* path = &path_component->as_path;
	struct vfs_cache_node* current_node = start;

	while (!vfs_path_empty(path)) {
		current_node = vfs_cache_node_resolve_mounted_fs(current_node);

		log_print("path: "); print_path(path);
		log_print("current_node: "); print_path(&current_node->name);

		// if it's a symlink resolve it
		if (current_node->inode->type == SYMLINK)
			return readlink(current_node, root, result, recursion_level);

		// file in the middle of the path
		if (current_node->inode->type != DIRECTORY) {
			*result = NULL;
			return -ENOTDIR;
		}

		struct vfs_cache_node* looked_up;
		looked_up = vfs_cache_node_lookup_child(current_node, path);
		if (!looked_up) {
			// node is not in the cache, fetch and cache it
			int err = read_and_cache_inode(current_node, path, &looked_up);
			if (err)
				return err;
		}

		vfs_path_next_component(path_component);
		current_node = looked_up;
	}

	*result = current_node;

	return 0;
}

int lookup_path(const vfs_path_t* path, struct vfs_cache_node* root,
				struct vfs_cache_node* cwd, struct vfs_cache_node** result,
				unsigned int recursion_level)
{
	int err;
	vfs_path_component_t path_component;
	struct vfs_cache_node* start = (vfs_path_absolute(path)) ? root : cwd;

	err = vfs_path_get_component(path, &path_component);
	if (err)
		return err;

	err = lookup(&path_component, start, root, result, recursion_level);

	vfs_path_destroy(&path_component.as_path);

	return err;
}

int vfs_lookup(const vfs_path_t* path, struct vfs_cache_node* root,
			   struct vfs_cache_node* cwd, struct vfs_cache_node** result)
{
	return lookup_path(path, root, cwd, result, 0);
}

int vfs_lookup_in_fs(const vfs_path_t* path, struct vfs_superblock* sb,
					 struct vfs_cache_node** result)
{
	return vfs_lookup(path, sb->root, NULL, result);
}