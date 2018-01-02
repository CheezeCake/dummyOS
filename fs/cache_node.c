#include <stddef.h>

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

	int err;
	if ((err = vfs_path_init(&root->name, "/", 1)) != 0)
		return err;

	cache_node_root = root;

	return 0;
}

int vfs_cache_node_create(struct vfs_inode* inode, const vfs_path_t* name,
						  struct vfs_cache_node** result)
{
	struct vfs_cache_node* node = kmalloc(sizeof(struct vfs_cache_node));
	if (!node)
		return -ENOMEM;

	int err;
	if ((err = vfs_cache_node_init(node, inode, name)) != 0) {
		kfree(node);
		return err;
	}

	*result = node;

	return 0;
}

int vfs_cache_node_init(struct vfs_cache_node* node, struct vfs_inode* inode,
						const vfs_path_t* name)
{
	memset(node, 0, sizeof(struct vfs_cache_node));

	int err;
	if (name && (err = vfs_path_copy_init(name, &node->name)) != 0)
		return err;

	node->inode = inode;
	if (inode)
		vfs_inode_grab_ref(inode);

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
		vfs_inode_drop_ref(node->inode);

	struct list_node* child;
	list_foreach(&node->children, child) {
		vfs_cache_node_drop_ref(container_of(child, struct vfs_cache_node,
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

	if ((err = vfs_cache_node_create(child_inode, name, &cache_node) != 0))
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
	if (vfs_path_name_str_equals(name, dot, 1))
		return (struct vfs_cache_node*)parent;
	// ".."
	if (vfs_path_name_str_equals(name, dot, 2))
		return parent->parent;

	struct list_node* it;

	list_foreach(&parent->children, it) {
		struct vfs_cache_node* node_it = list_entry(it,
													struct vfs_cache_node,
													cn_children_list);
		if (vfs_path_name_equals(name, &node_it->name))
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

struct vfs_cache_node*
vfs_cache_node_resolve_mounted_fs(struct vfs_cache_node* mountpoint)
{
	struct vfs_cache_node* mounted = mountpoint;
	while (mounted->mounted)
		mounted = mounted->mounted;

	return mounted;
}

void vfs_cache_node_grab_ref(struct vfs_cache_node* node)
{
	refcount_inc(&node->refcnt);
}

void vfs_cache_node_drop_ref(struct vfs_cache_node* node)
{
	refcount_dec(&node->refcnt);
	if (refcount_get(&node->refcnt) == 0)
		destroy(node);
}

int vfs_cache_node_get_ref(const struct vfs_cache_node* node)
{
	return refcount_get(&node->refcnt);
}
