#include <dummyos/errno.h>
#include <fs/cache_node.h>
#include <fs/vfs.h>
#include <kernel/kassert.h>
#include <kernel/kmalloc.h>
#include <libk/libk.h>
#include <libk/refcount.h>
#include <libk/utils.h>

#include <kernel/log.h>

static struct vfs_inode cnode_root_inode;
static struct vfs_cache_node* cache_node_root = NULL;

int vfs_cache_init(void)
{
	vfs_path_t root_path;
	int err;

	vfs_inode_init(&cnode_root_inode, DIRECTORY, NULL, NULL);

	err = vfs_path_init(&root_path, "/", 1);
	if (!err) {
		err = vfs_cache_node_create(&cnode_root_inode, &root_path,
									&cache_node_root);
		vfs_path_reset(&root_path);
	}

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
	if (inode) {
		vfs_inode_ref(inode);
		vfs_inode_add_cnode(inode, node);
	}

	list_init(&node->children);
	list_init(&node->cn_children_list);

	mutex_init(&node->lock);

	refcount_init(&node->refcnt);

	return 0;
}

static void vfs_cache_node_reset(struct vfs_cache_node* node)
{
	vfs_path_reset(&node->name);

	if (node->inode) {
		vfs_inode_remove_cnode(node->inode, node);
		vfs_inode_unref(node->inode);
	}

	if (node->parent) {
		mutex_lock(&node->parent->lock);
		list_erase(&node->cn_children_list);
		mutex_unlock(&node->parent->lock);
		vfs_cache_node_unref(node->parent);
	}

	list_node_t* child;
	list_foreach(&node->children, child) {
		vfs_cache_node_unref(list_entry(child, struct vfs_cache_node,
										cn_children_list));
	}

	mutex_destroy(&node->lock);

	memset(node, 0, sizeof(struct vfs_cache_node));
}

/**
 * @brief Destroys a vfs_cache_node object
 */
static void vfs_cache_node_destroy(struct vfs_cache_node* node)
{
	vfs_cache_node_reset(node);
	kfree(node);
}

static void insert_child(struct vfs_cache_node* parent,
						 struct vfs_cache_node* child)
{
	mutex_lock(&parent->lock);

	child->parent = parent;
	vfs_cache_node_ref(parent);

	list_push_back(&parent->children, &child->cn_children_list);

	mutex_unlock(&parent->lock);
}

int vfs_cache_node_insert_child(struct vfs_cache_node* parent,
								struct vfs_inode* child_inode,
								const vfs_path_t* name,
								struct vfs_cache_node** inserted_child)
{
	struct vfs_cache_node* child;
	int err;

	child = vfs_cache_node_lookup_child(parent, name);
	if (child) {
		vfs_cache_node_unref(child);
		return -EEXIST;
	}

	err = vfs_cache_node_create(child_inode, name, &child);
	if (err)
		return err;

	if (parent)
		insert_child(parent, child);

	if (inserted_child) {
		*inserted_child = child;
		vfs_cache_node_ref(child);
	}

	vfs_cache_node_unref(child);

	return 0;
}

struct vfs_cache_node*
vfs_cache_node_lookup_child(struct vfs_cache_node* parent,
							const vfs_path_t* name)
{
	const char dot[] = "..";
	// "."
	if (vfs_path_str_same(name, dot, 1)) {
		if (parent)
			vfs_cache_node_ref(parent);
		return parent;
	}
	// ".."
	if (vfs_path_str_same(name, dot, 2)) {
		if (parent->parent)
			vfs_cache_node_ref(parent->parent);
		return parent->parent;
	}

	mutex_lock(&parent->lock);

	list_node_t* it;
	list_foreach(&parent->children, it) {
		struct vfs_cache_node* node_it = list_entry(it,
													struct vfs_cache_node,
													cn_children_list);
		if (vfs_path_same(name, &node_it->name)) {
			vfs_cache_node_ref(node_it);
			mutex_unlock(&parent->lock);
			return node_it;
		}
	}

	mutex_unlock(&parent->lock);

	return NULL;
}

struct vfs_cache_node* vfs_cache_node_get_root(void)
{
	return cache_node_root;
}

struct vfs_cache_node*
vfs_cache_node_get_parent(const struct vfs_cache_node* node)
{
	vfs_cache_node_ref(node->parent);
	return node->parent;
}

struct vfs_cache_node*
vfs_cache_node_resolve_mounted_fs(struct vfs_cache_node* mountpoint)
{
	struct vfs_cache_node* mounted = mountpoint;
	while (mounted->mounted)
		mounted = mounted->mounted;

	vfs_cache_node_ref(mounted);
	return mounted;
}

void vfs_cache_node_ref(struct vfs_cache_node* node)
{
	if (node) {
		/* log_printf("REF cnode %p (rc=%d) path:", (void*)node, refcount_get(&node->refcnt)); */
		/* print_path(&node->name); */

		refcount_inc(&node->refcnt);
	}
}

void vfs_cache_node_unref(struct vfs_cache_node* node)
{
	if (node) {
		/* log_printf("UNREF cnode %p (rc=%d) path:", (void*)node, refcount_get(&node->refcnt)); */
		/* print_path(&node->name); */

		if (refcount_dec(&node->refcnt) == 0)
			vfs_cache_node_destroy(node);
	}
}

int vfs_cache_node_get_ref(const struct vfs_cache_node* node)
{
	return refcount_get(&node->refcnt);
}

int vfs_cache_node_open(struct vfs_cache_node* cnode, int flags,
						struct vfs_file** result)
{
	struct vfs_inode* inode = cnode->inode;
	struct vfs_file* file;
	int err;

	err = vfs_file_create(cnode, flags, &file);
	if (err)
		return err;

	err = inode->op->open(inode, file);
	kassert(file->op);
	if (err)
		goto fail_inode_open;

	err = vfs_inode_open_fops(cnode->inode, &file->op);
	if (err)
		goto fail_open_fops;

	err = file->op->open(file, flags);
	if (err)
		goto fail_open;

	*result = file;

	return 0;

fail_open:
	file->op->close(file);
fail_inode_open:
fail_open_fops:
	vfs_file_destroy(file);

	return err;
}

static void indent(size_t n)
{
	for (size_t i = 0; i < n; ++i)
		log_putchar('\t');
}

static void dump(const struct vfs_cache_node* node, size_t n)
{
	if (!node)
		return;

	indent(n); log_printf("rc=%d | ", vfs_cache_node_get_ref(node)); print_path(&node->name);

	if (node->mounted) {
		log_puts("[mounted] "); dump(node->mounted, n);
	}
	else {
		list_node_t* it;
		list_foreach(&node->children, it) {
			dump(list_entry(it, struct vfs_cache_node, cn_children_list),
				 n + 1);
		}
	}
}

void dump_cache_tree(void)
{
	log_puts("### VFS_CACHE DUMP START\n");
	dump(cache_node_root, 0);
	log_puts("### VFS_CACHE DUMP END\n\n");
}
