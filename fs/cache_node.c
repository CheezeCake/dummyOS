#include <fs/cache_node.h>
#include <fs/vfs.h>
#include <kernel/errno.h>
#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <libk/libk.h>
#include <libk/refcount.h>
#include <libk/utils.h>

static struct vfs_cache_node* cache_node_root = NULL;

int vfs_cache_init(struct vfs_cache_node* root)
{
	kassert(root->inode->type == DIRECTORY);

	int err = vfs_path_init(&root->name, "/", 1);
	if (!err)
		cache_node_root = root;

	return err;
}

int vfs_cache_node_create(struct vfs_inode* inode, const vfs_path_t* name,
						  struct vfs_cache_node** result)
{
	struct vfs_cache_node* node = kmalloc(sizeof(struct vfs_cache_node));
	if (!node)
		return -ENOMEM;

	int err = vfs_cache_node_init(node, inode, name);
	if (err) {
		kfree(node);
		node = NULL;
	}

	*result = node;

	return err;
}

int vfs_cache_node_init(struct vfs_cache_node* node, struct vfs_inode* inode,
						const vfs_path_t* name)
{
	memset(node, 0, sizeof(struct vfs_cache_node));

	if (name) {
		int err = vfs_path_copy_init(name, &node->name);
		if (err)
			return err;
	}

	node->inode = inode;
	if (inode)
		vfs_inode_ref(inode);

	list_init(&node->children);
	list_init(&node->cn_children_list);

	refcount_init(&node->refcnt);

	return 0;
}

/**
 * @brief Destroys a vfs_cache_node object
 */
static void destroy(struct vfs_cache_node* node)
{
	vfs_path_destroy(&node->name);

	if (node->inode)
		vfs_inode_unref(node->inode);

	list_node_t* child;
	list_foreach(&node->children, child) {
		vfs_cache_node_unref(container_of(child, struct vfs_cache_node,
											 cn_children_list));
	}
}

int vfs_cache_node_insert_child(struct vfs_cache_node* parent,
								struct vfs_inode* child_inode,
								const vfs_path_t* name,
								struct vfs_cache_node** inserted_child)
{
	int err;
	struct vfs_cache_node* cache_node;

	err = vfs_cache_node_create(child_inode, name, &cache_node);
	if (err)
		return err;

	if (parent) {
		cache_node->parent = parent;
		list_push_back(&parent->children, &cache_node->cn_children_list);
	}
	if (inserted_child)
		*inserted_child = cache_node;

	return 0;
}

struct vfs_cache_node*
vfs_cache_node_lookup_child(const struct vfs_cache_node* parent,
							const vfs_path_t* name)
{
	const char dot[] = "..";
	// "."
	if (vfs_path_str_same(name, dot, 1))
		return (struct vfs_cache_node*)parent;
	// ".."
	if (vfs_path_str_same(name, dot, 2))
		return parent->parent;

	list_node_t* it;
	list_foreach(&parent->children, it) {
		struct vfs_cache_node* node_it = list_entry(it,
													struct vfs_cache_node,
													cn_children_list);
		if (vfs_path_same(name, &node_it->name))
			return node_it;
	}

	return NULL;
}

struct vfs_cache_node* vfs_cache_node_get_root(void)
{
	return cache_node_root;
}

struct vfs_cache_node*
vfs_cache_node_get_parent(const struct vfs_cache_node* node)
{
	return node->parent;
}

int vfs_cache_node_open(struct vfs_cache_node* node, int flags,
						struct vfs_file** result)
{
	int err;
	struct vfs_file* file;
	struct vfs_inode* inode = node->inode;

	err = vfs_file_create(node, flags, &file);
	if (err)
		return err;

	err = inode->op->open(inode, node, flags, file);
	if (err)
		return err;

	// fs dependent open() did not set file->op
	if (!file->op) {
		inode->op->close(inode, file);
		vfs_file_destroy(file);
		return -EIO;
	}

	*result = file;

	return 0;
}

struct vfs_cache_node*
vfs_cache_node_resolve_mounted_fs(struct vfs_cache_node* mountpoint)
{
	struct vfs_cache_node* mounted = mountpoint;
	while (mounted->mounted)
		mounted = mounted->mounted;

	return mounted;
}

void vfs_cache_node_ref(struct vfs_cache_node* node)
{
	refcount_inc(&node->refcnt);
}

void vfs_cache_node_unref(struct vfs_cache_node* node)
{
	if (refcount_dec(&node->refcnt) == 0)
		destroy(node);
}

int vfs_cache_node_get_ref(const struct vfs_cache_node* node)
{
	return refcount_get(&node->refcnt);
}
