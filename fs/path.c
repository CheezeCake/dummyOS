#include <fs/path.h>
#include <kernel/errno.h>
#include <kernel/kmalloc.h>
#include <libk/libk.h>
#include <libk/utils.h>

#include <kernel/log.h>

static inline const char* get_string(const vfs_path_t* path)
{
	return (path->base_str->str + path->offset);
}

static inline const char* get_next_string(const vfs_path_t* path)
{
	return (get_string(path) + path->size);
}

static inline vfs_path_offset_t first_x(const char* str, vfs_path_size_t size,
										bool (*const predicate)(char))
{
	vfs_path_offset_t offset = 0;
	while (offset < size && !predicate(str[offset]))
		++offset;

	return offset;
}

static inline bool slash_predicate(char c) { return (c == '/'); }
static inline bool non_slash_predicate(char c) { return !slash_predicate(c); }

static inline
vfs_path_offset_t first_slash(const char* str, vfs_path_size_t size)
{
	return first_x(str, size, slash_predicate);
}

static inline
vfs_path_offset_t first_non_slash(const char* str, vfs_path_size_t size)
{
	return first_x(str, size, non_slash_predicate);
}

static char* copy_path_str(const char* path, vfs_path_size_t size)
{
	char* new_path = kmalloc(size);
	if (!new_path)
		return NULL;

	strncpy(new_path, path, size);

	return new_path;
}


int vfs_path_create(const char* path, vfs_path_size_t size, vfs_path_t** result)
{
	vfs_path_t* new_path = kmalloc(sizeof(vfs_path_t));
	if (!new_path)
		return -ENOMEM;

	int err = vfs_path_init(new_path, path, size);
	if (err) {
		kfree(new_path);
		new_path = NULL;
	}

	*result = new_path;

	return err;
}

static inline void vfs_path_string_init(string_t* str, char* path,
										vfs_path_size_t size)
{
	str->str = path;
	str->size = size;
	refcount_init(&str->refcnt);
}

int vfs_path_init(vfs_path_t* path, const char* path_str, vfs_path_size_t size)
{
	if (size > VFS_PATH_MAX_LEN)
		return -ENAMETOOLONG;

	string_t* str = kmalloc(sizeof(string_t));
	if (!str)
		return -ENOMEM;

	char* new_path_str = copy_path_str(path_str, size);
	if (!new_path_str) {
		kfree(str);
		return -ENOMEM;
	}

	vfs_path_string_init(str, new_path_str, size);

	memset(path, 0, sizeof(vfs_path_t));
	path->base_str = str;
	path->size = size;

	return 0;
}

int vfs_path_create_from(const vfs_path_t* path,
						 vfs_path_offset_t relative_offset,
						 vfs_path_size_t size,
						 vfs_path_t** result)
{
	int err;
	vfs_path_t* new_path;

	if (size > VFS_PATH_MAX_LEN)
		return -ENAMETOOLONG;

	err = vfs_path_copy_create(path, &new_path);
	if (!err) {
		new_path->offset += relative_offset;
		new_path->size = size;
	}

	return err;
}

int vfs_path_copy_create(const vfs_path_t* path, vfs_path_t** result)
{
	vfs_path_t* new_path = kmalloc(sizeof(vfs_path_t));
	if (!new_path)
		return -ENOMEM;

	int err = vfs_path_copy_init(path, new_path);
	if (err)
		kfree(new_path);

	return err;
}

int vfs_path_copy_init(const vfs_path_t* path, vfs_path_t* copy)
{
	memcpy(copy, path, sizeof(vfs_path_t));
	refcount_inc(&path->base_str->refcnt);

	return 0;
}

void vfs_path_reset(vfs_path_t* path)
{
	string_t* str = path->base_str;

	refcount_dec(&str->refcnt);
	if (refcount_get(&str->refcnt) == 0)
		kfree(str);
}

void vfs_path_destroy(vfs_path_t* path)
{
	vfs_path_reset(path);
	kfree(path);
}

bool vfs_path_absolute(const vfs_path_t* path)
{
	return (path->size > 0) ? (*get_string(path) == '/') : false;
}

bool vfs_path_empty(const vfs_path_t* path)
{
	return (path->size == 0 || !path->base_str || !path->base_str->str);
}

int vfs_path_component_init(vfs_path_component_t* component,
							const vfs_path_t* path)
{
	int err;
	vfs_path_t* component_path = &component->as_path;

	err = vfs_path_copy_init(path, component_path);

	return err;
}

void vfs_path_component_reset(vfs_path_component_t* component)
{
	// path was init()'d
	vfs_path_reset(&component->as_path);
}

int vfs_path_get_component(const vfs_path_t* path,
						   vfs_path_component_t* component)
{
	int err;

	err = vfs_path_component_init(component, path);
	if (err)
		return err;

	vfs_path_t* component_path = &component->as_path;

	// find start offset of first component
	const vfs_path_offset_t start = first_non_slash(get_string(component_path),
													component_path->size);
	// find end offset of first component starting from "start"
	const vfs_path_offset_t end = start +
		first_slash(get_string(component_path) + start,
					component_path->size - start);

	component_path->offset += start;
	component_path->size = end - start;

	return 0;
}

int vfs_path_next_component(vfs_path_component_t* component)
{
	vfs_path_t* path = &component->as_path;

	if (vfs_path_empty(path))
		return -EINVAL;

	// shift path to the end of current component
	path->offset = path->offset + path->size;
	path->size = path->base_str->size - path->offset;

	// first non slash starting from end of current component
	const vfs_path_offset_t start = first_non_slash(get_string(path), path->size);
	const vfs_path_offset_t end  = start + first_slash(get_string(path) + start,
													   path->size - start);

	path->offset += start;
	path->size = end - start;

	return (vfs_path_empty(path)) ? -EINVAL : 0;
}

bool vfs_path_name_equals(const vfs_path_t* p1, const vfs_path_t* p2)
{
	const int cmp = memcmp(p1, p2, sizeof(vfs_path_t));
	if (cmp == 0)
		return true;

	if (p1->size != p2->size)
		return false;

	return vfs_path_name_str_equals(p1, get_string(p2), p2->size);
}

bool vfs_path_name_str_equals(const vfs_path_t* p1, const char* p2,
							  vfs_path_size_t size)
{
	return (p1->size == size && strncmp(get_string(p1), p2, size) == 0);
}

void print_path(const vfs_path_t* path)
{
	const char* str = get_string(path);
	log_printf("[%d] -> ", path->size);
	for (vfs_path_size_t i = 0; i < path->size; ++i)
		log_printf("%c", str[i]);
	log_print("\n");
}
