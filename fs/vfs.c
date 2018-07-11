#include <dummyos/fcntl.h>
#include <fs/filesystem.h>
#include <fs/inode.h>
#include <fs/vfs.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <kernel/sched/sched.h>
#include <kernel/types.h>
#include <libk/libk.h>
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
extern int8_t _binary_archive_start;

static int mount_root(void)
{
	int err = -ENODEV;

	err = vfs_mount(NULL, vfs_cache_node_get_root(), &_binary_archive_start,
					"ramfs");

	return err;
}

int vfs_init(void)
{
	int err;

	err = vfs_cache_init();
	if (err)
		return err;

	list_init(&mounted_list);

	// parse cmd line?
	return mount_root();
}

static int get_superblock(struct vfs_cache_node* device, const char* filesystem,
						  void* data, struct vfs_superblock** sb)
{
	if (device && device->inode->type != BLOCKDEV)
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
	vfs_cache_node_ref(mountpoint->mounted);

	return 0;
}

int vfs_umount(struct vfs_cache_node* mountpoint)
{
	struct vfs_cache_node* root = mountpoint->mounted;
	// not a mountpoint
	if (!root)
		return -EINVAL;

	struct vfs_superblock* sb = root->inode->sb;
	list_erase(&sb->mounted_list);
	root->mountpoint = NULL;
	vfs_cache_node_unref(root->mounted);
	mountpoint->mounted = NULL;

	return sb->fs->superblock_destroy(sb->fs, sb);
}

static int readlink(const struct vfs_cache_node* symlink,
					struct vfs_cache_node* root,
					struct vfs_cache_node** target,
					unsigned int recursion_level)
{
	struct vfs_cache_node* start = root;
	vfs_path_t* target_path;
	int err;

	if (symlink->inode->type != SYMLINK)
		return -EINVAL;

	// creates target_path
	err = symlink->inode->op->readlink(symlink->inode, &target_path);
	if (err)
		return err;

	if (vfs_path_absolute(target_path))
		start = vfs_cache_node_get_parent(symlink); // refs start

	err = lookup_path(target_path, start, root, target, recursion_level + 1);

	if (start != root)
		vfs_cache_node_unref(start); // unref start

	// destroy target_path
	vfs_path_destroy(target_path);

	return err;
}

int vfs_read_and_cache_inode(struct vfs_cache_node* parent,
							 struct vfs_inode* inode,
							 const vfs_path_t* name,
							 struct vfs_cache_node** cached_result)
{
	int err;

	err = inode->sb->op->read_inode(inode->sb, inode);
	if (err)
		return err;

	err = vfs_cache_node_insert_child(parent, inode, name, cached_result);

	return err;
}

/**
 * Lookup, fetch an inode and insert it in the cache
 */
static int lookup_read_and_cache_inode(struct vfs_cache_node* parent,
									   const vfs_path_t* lookup_pathname,
									   struct vfs_cache_node** cached_result)
{
	struct vfs_inode* parent_inode = parent->inode;
	struct vfs_inode* inode = NULL;
	int err;

	err = parent_inode->op->lookup(parent_inode, lookup_pathname, &inode);
	if (err)
		return err;

	err = vfs_read_and_cache_inode(parent, inode, lookup_pathname,
								   cached_result);

	return err;
}

static int do_lookup(vfs_path_component_t* component,
					 struct vfs_cache_node* const start_node,
					 struct vfs_cache_node* root,
					 struct vfs_cache_node** result,
					 unsigned int recursion_level)
{
	struct vfs_cache_node* current_node = NULL;
	struct vfs_cache_node* tmp = NULL;
	int err = 0;

	current_node = start_node;
	vfs_cache_node_ref(current_node);

	while (!vfs_path_empty(vfs_path_component_as_path(component))) {
		const vfs_path_t* path_component = vfs_path_component_as_path(component);

		log_print("path: "); print_path(path_component);
		log_print("current_node: "); print_path(&current_node->name);

		// if it's a symlink, resolve it
		if (current_node->inode->type == SYMLINK) {
			err = readlink(current_node, root, &tmp, recursion_level);
			if (err)
				goto out;
			vfs_cache_node_unref(current_node);
			current_node = tmp;
		}

		// file in the middle of the path
		if (current_node->inode->type != DIRECTORY) {
			err = -ENOTDIR;
			goto out;
		}

		tmp = vfs_cache_node_resolve_mounted_fs(current_node);
		vfs_cache_node_unref(current_node);
		current_node = tmp;
		log_print("current_node mnt resol: "); print_path(&current_node->name);

		struct vfs_cache_node* looked_up =
			vfs_cache_node_lookup_child(current_node, path_component);
		if (!looked_up) {
			// node is not in the cache, look it up, fetch it and cache it
			err = lookup_read_and_cache_inode(current_node, path_component,
											  &looked_up);
			if (err)
				goto out;
		}

		vfs_path_component_next(component);

		vfs_cache_node_unref(current_node);
		current_node = looked_up;
	}

out:
	if (current_node) {
		if (!err) {
			*result = current_node;
			vfs_cache_node_ref(*result);
		}

		vfs_cache_node_unref(current_node);
	}

	return err;
}

static int lookup(const vfs_path_t* const path, struct vfs_cache_node* start,
				  struct vfs_cache_node* root, struct vfs_cache_node** result,
				  unsigned int recursion_level)
{
	vfs_path_component_t component;
	struct vfs_cache_node* result_node = NULL;
	struct vfs_cache_node* tmp = NULL;
	int err = 0;

	if (recursion_level >= VFS_LOOKUP_MAX_RECURSION_LEVEL)
		return -ELOOP;
	if (!start || !root)
		return -EINVAL;

	start = vfs_cache_node_resolve_mounted_fs(start); // refs start

	err = vfs_path_first_component(path, &component);
	if (err)
		goto fail_component;

	err = do_lookup(&component, start, root, &result_node, recursion_level);
	if (err)
		goto out;

	if (result_node->inode->type == SYMLINK) {
		err = readlink(result_node, root, &tmp, recursion_level);
		if (!err) {
			vfs_cache_node_unref(result_node);
			result_node = tmp;
		}
	}

	if (!err) {
		*result = result_node;
		vfs_cache_node_ref(*result);
	}

	vfs_cache_node_unref(result_node);
out:
	vfs_path_component_reset(&component);
fail_component:
	vfs_cache_node_unref(start); // unref start

	return err;
}

int lookup_path(const vfs_path_t* const path, struct vfs_cache_node* root,
				struct vfs_cache_node* cwd, struct vfs_cache_node** result,
				unsigned int recursion_level)
{
	int err;
	struct vfs_cache_node* start = (vfs_path_absolute(path)) ? root : cwd;

	err = lookup(path, start, root, result, recursion_level);

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

int vfs_open(const vfs_path_t* path, int flags, struct vfs_file* file)
{
	struct vfs_cache_node* cnode;
	struct process* current_proc = sched_get_current_process();
	int err;

	err = vfs_lookup(path, current_proc->root, current_proc->cwd, &cnode);
	if (err)
		return err;

	err = vfs_cache_node_open(cnode, flags, file);
	if (!err)
		vfs_cache_node_unref(cnode);

	return err;
}

int vfs_close(struct vfs_file* file)
{
	int err = 0;

	if (file->op && file->op->close) {
		err = file->op->close(file);
	}

	return err;
}
