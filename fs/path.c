#include <dummyos/errno.h>
#include <fs/path.h>
#include <kernel/kmalloc.h>
#include <libk/libk.h>
#include <libk/utils.h>

#include <kernel/log.h>

/*
 * utils
 */
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


/*
 * string_t
 */
static char* copy_path_str(const char* path, vfs_path_size_t size)
{
	char* new_path = kmalloc(size);
	if (!new_path)
		return NULL;

	strncpy(new_path, path, size);

	return new_path;
}

static int vfs_path_string_create(const char* path, vfs_path_size_t size,
								   string_t** result)
{
	string_t* str;
	char* path_cpy;

	str = kmalloc(sizeof(string_t));
	if (!str)
		return -ENOMEM;

	path_cpy = copy_path_str(path, size);
	if (!path_cpy) {
		kfree(str);
		return -ENOMEM;
	}

	str->str = path_cpy;
	str->size = size;
	refcount_init(&str->refcnt);

	*result = str;

	return 0;
}

static void vfs_path_string_destroy(string_t* string)
{
	if (refcount_dec(&string->refcnt) == 0) {
		kfree(string->str);
		kfree(string);
	}
}


/*
 * vfs_path_t
 */
int vfs_path_init(vfs_path_t* path, const char* path_str, vfs_path_size_t size)
{
	string_t* str;
	int err;

	if (size > VFS_PATH_MAX_LEN)
		return -ENAMETOOLONG;

	err = vfs_path_string_create(path_str, size, &str);
	if (!err) {
		path->base_str = str;
		path->offset = 0;
		path->size = size;
	}

	return err;
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

int vfs_path_create_from(const vfs_path_t* path,
						 vfs_path_offset_t relative_offset,
						 vfs_path_size_t size,
						 vfs_path_t** result)
{
	vfs_path_t* new_path;
	int err;

	if (size > VFS_PATH_MAX_LEN)
		return -ENAMETOOLONG;

	err = vfs_path_copy_create(path, &new_path);
	if (!err) {
		new_path->offset += relative_offset;
		new_path->size = size;

		*result = new_path;
	}

	return err;
}

int vfs_path_copy_init(const vfs_path_t* path, vfs_path_t* copy)
{
	memcpy(copy, path, sizeof(vfs_path_t));
	refcount_inc(&path->base_str->refcnt);

	return 0;
}

int vfs_path_copy_create(const vfs_path_t* path, vfs_path_t** result)
{
	vfs_path_t* new_path;
	int err;

	new_path = kmalloc(sizeof(vfs_path_t));
	if (!new_path)
		return -ENOMEM;

	err = vfs_path_copy_init(path, new_path);
	if (err) {
		kfree(new_path);
		new_path = NULL;
	}

	*result = new_path;

	return err;
}

void vfs_path_reset(vfs_path_t* path)
{
	if (path->base_str)
		vfs_path_string_destroy(path->base_str);

	memset(path, 0, sizeof(vfs_path_t));
}

void vfs_path_destroy(vfs_path_t* path)
{
	vfs_path_reset(path);
	kfree(path);
}

bool vfs_path_absolute(const vfs_path_t* path)
{
	return (path->size > 0) ? (*vfs_path_get_str(path) == '/') : false;
}

bool vfs_path_empty(const vfs_path_t* path)
{
	return (path->size == 0 || !path->base_str || !path->base_str->str);
}


/*
 * vfs_path_component_t
 */
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

/**
 * Isolate the first component of a path.
 *
 * @return 0 if a component was found and built
 * -EINVAL if not
 */
static int build_component_path(vfs_path_t* path)
{
	const char* const str = vfs_path_get_str(path);

	// start = first non slash
	const vfs_path_offset_t start = first_non_slash(str, path->size);
	// end = first slash since start
	const vfs_path_offset_t end  = start + first_slash(str + start,
													   path->size - start);

	path->offset += start;
	path->size = end - start;

	return 0;
}

int vfs_path_first_component(const vfs_path_t* path,
						   vfs_path_component_t* component)
{
	int err;

	err = vfs_path_component_init(component, path);
	if (!err)
		err = build_component_path(&component->as_path);

	return err;
}

int vfs_path_component_next(vfs_path_component_t* component)
{
	vfs_path_t* path = &component->as_path;

	// shift path to the end of current component
	path->offset = path->offset + path->size;
	path->size = path->base_str->size - path->offset;

	return build_component_path(path);
}

void vfs_path_ignore_trailing_slashes(vfs_path_t* path)
{
	const char* start = vfs_path_get_str(path);
	const char* end = start + path->size;

	// ignore trailing slashes
	while (start < end - 1 && end[-1] == '/')
		--end;

	path->size = (end - start);
}

static const char* basename_start(const vfs_path_t* path)
{
	const char* start = vfs_path_get_str(path);
	const char* end = start + path->size;

	while (start <= end - 1 && end[-1] != '/')
		--end;

	return end;
}

int vfs_path_basename(const vfs_path_t* path, vfs_path_t* basename)
{
	int err;

	err = vfs_path_copy_init(path, basename);
	if (err)
		return err;

	vfs_path_ignore_trailing_slashes(basename);

	const char* start = vfs_path_get_str(basename);
	const char* end = start + basename->size;
	const char* bname_start = basename_start(basename);

	basename->offset += bname_start - start;
	basename->size = end - bname_start;

	return 0;
}

int vfs_path_dirname(const vfs_path_t* path, vfs_path_t* dirname)
{
	int err;

	err = vfs_path_copy_init(path, dirname);
	if (err)
		return err;

	vfs_path_ignore_trailing_slashes(dirname);

	const char* start = vfs_path_get_str(dirname);
	if (!start)
		return -EINVAL;
	const char* bname_start = basename_start(dirname);

	dirname->size = (bname_start - start);
	if (dirname->size == 0) {
		vfs_path_reset(dirname);
		err = vfs_path_init(dirname, ".", 1);
	}
	else {
		vfs_path_ignore_trailing_slashes(dirname);
		if (dirname->size == 0)
			++dirname->size;
	}

	return err;
}

/**
 * "strictly" compares two paths by comparing their underlying strings.
 */
static bool path_string_equals(const vfs_path_t* p1, const vfs_path_t* p2)
{
	// same object
	if (p1 == p2)
		return true;

	// same underlying string, offset and size
	if (p1->base_str == p2->base_str &&
		p1->offset == p2->offset &&
		p1->size == p2->size)
		return true;

	return (p1->size == p2->size &&
			strncmp(vfs_path_get_str(p1), vfs_path_get_str(p2), p1->size) == 0);
}

bool vfs_path_component_equals(const vfs_path_component_t* c1,
							   const vfs_path_component_t* c2)
{
	// same object or strict comparison on paths
	return (c1 == c2 || path_string_equals(&c1->as_path, &c2->as_path));
}

static inline bool vfs_path_component_empty(const vfs_path_component_t* c)
{
	return vfs_path_empty(&c->as_path);
}

vfs_path_size_t vfs_path_get_size(const vfs_path_t* path)
{
	return path->size;
}

const char* vfs_path_get_str(const vfs_path_t* path)
{
	const char* const str = path->base_str->str;
	return (str) ? str + path->offset : NULL;
}

/**
 * Determines if two paths are equivalent by comparing them component by
 * component. Deals with multi-slashes.
 */
static bool path_same(const vfs_path_t* p1, const vfs_path_t* p2)
{
	vfs_path_component_t c1;
	vfs_path_component_t c2;
	int err;
	bool ret = false;

	err = vfs_path_first_component(p1, &c1);
	if (err)
		goto fail_c1;
	err = vfs_path_first_component(p2, &c2);
	if (err)
		goto fail_c2;

	while (!vfs_path_component_empty(&c1) && !vfs_path_component_empty(&c2) &&
		   vfs_path_component_equals(&c1, &c2))
	{
		vfs_path_component_next(&c1);
		vfs_path_component_next(&c2);
	}

	ret = (vfs_path_component_empty(&c1) && vfs_path_component_empty(&c2));

fail_c2:
	vfs_path_component_reset(&c2);
fail_c1:
	vfs_path_component_reset(&c1);

	return ret;
}

bool vfs_path_same(const vfs_path_t* p1, const vfs_path_t* p2)
{
	// string string comparison or equivalence
	return (path_string_equals(p1, p2) || path_same(p1, p2));
}

bool vfs_path_str_same(const vfs_path_t* p, const char* s,
					   vfs_path_size_t size)
{
	vfs_path_t sp;
	bool ret;
	int err;

	err = vfs_path_init(&sp, s, size);
	if (err) {
		ret = false;
	}
	else {
		ret = vfs_path_same(p, &sp);
		vfs_path_reset(&sp);
	}

	return ret;
}

void print_path(const vfs_path_t* path)
{
	const char* str = vfs_path_get_str(path);
	log_printf("[%d] -> ", path->size);
	for (vfs_path_size_t i = 0; i < path->size; ++i)
		log_printf("%c", str[i]);
	log_print("\n");
}
