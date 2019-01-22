#ifndef _FS_VFS_PATH_H_
#define _FS_VFS_PATH_H_

#include <kernel/types.h>
#include <libk/refcount.h>


#define VFS_PATH_MAX_LEN 2048

typedef unsigned short vfs_path_offset_t;
typedef unsigned short vfs_path_size_t;

/**
 * @brief Reference counted string
 *
 * only used internally by vfs_path_t
 */
typedef struct string
{
	char* str;
	vfs_path_size_t size;
	refcount_t refcnt;
} string_t;


/**
 * @brief File system path
 *
 * A path is reprensented by the conbination of a base string, on offset to
 * the start of that string and a size.
 */
typedef struct vfs_path
{
	string_t* base_str;

	vfs_path_offset_t offset;
	vfs_path_size_t size;
} vfs_path_t;

/**
 * @brief File system path component
 */
typedef struct vfs_path_component
{
	vfs_path_t as_path;
} vfs_path_component_t;

/**
 * Helper function to get the vfs_path_t representation of a component.
 */
	static const inline vfs_path_t*
vfs_path_component_as_path(const vfs_path_component_t* component)
{
	return &component->as_path;
}


/**
 * @brief Creates a vfs_path_t object taking ownership of the pointer path
 *
 * @param path the path
 * @param size path size
 * @param result where to store the newly created vfs_path_t
 *
 * @return 0 on success
 */
int vfs_path_create(const char* path, vfs_path_size_t size,
		    vfs_path_t** result);

/**
 * @brief Initializes a vfs_path_t object
 *
 * @param path the vfs_path_t object to initialize
 * @param path_str the string to initialize path with
 *
 * @return 0 on success \n
 *			-ENOMEM if memory could not be allocated
 */
int vfs_path_init(vfs_path_t* path, const char* path_str, vfs_path_size_t size);

/**
 * @brief Creates a vfs_path_t object from another vfs_path_t
 *
 * The created path starts at path+relative_offset and is size bytes long
 *
 * @param path the base path
 * @param relative_offset offset from the start of path
 * @param size the size of the new path
 * @param result where to store the newly created path
 *
 * @return 0 on success
 */
int vfs_path_create_from(const vfs_path_t* path,
			 vfs_path_offset_t relative_offset,
			 vfs_path_size_t size,
			 vfs_path_t** result);

/**
 * @brief Creates a copy of a vfs_path_t object
 *
 * @param path the path to copy
 * @param result where to store the newly created copy
 *
 * @return 0 on sucess
 */
int vfs_path_copy_create(const vfs_path_t* path, vfs_path_t** result);

/**
 * @brief Copies a vfs_path_t object to another
 *
 * @param path the path to copy
 * @param copy the copy
 *
 * @return 0 on success
 */
int vfs_path_copy_init(const vfs_path_t* path, vfs_path_t* copy);

/**
 * @brief Resets a vfs_path_t object destroying all its content without
 * freeing the object itself.
 */
void vfs_path_reset(vfs_path_t* path);

/**
 * @brief Destroys a vfs_path_t object
 * The object is free()'d
 *
 * @param path path to destroy
 */
void vfs_path_destroy(vfs_path_t* path);

/**
 * @brief Checks if a path is absolute
 *
 * @param path the path to check
 *
 * @return true if the path is absolute
 */
bool vfs_path_absolute(const vfs_path_t* path);

/**
 * @brief Checks if a path is empty
 *
 * @param path path to check
 *
 * @return true if empty
 */
bool vfs_path_empty(const vfs_path_t* path);

/**
 * @brief Returns the size of a path
 */
vfs_path_size_t vfs_path_get_size(const vfs_path_t* path);

/**
 * @brief Returns the const char* representation of the string
 *
 * @note may not be null terminated
 */
const char* vfs_path_get_str(const vfs_path_t* path);

/**
 * @brief Initializes a vfs_path_component_t object
 */
int vfs_path_component_init(vfs_path_component_t* component,
			    const vfs_path_t* path);

/**
 * @brief Compares two vfs_component_path
 */
bool vfs_path_component_equals(const vfs_path_component_t* c1,
			       const vfs_path_component_t* c2);

/**
 * @brief Resets a vfs_path_component_t object destroying all its content
 * without freeing the object itself.
 */
void vfs_path_component_reset(vfs_path_component_t* component);

/**
 * @brief Starts splitting the path in components
 *
 * @param component will point to the first component of path
 *
 * @note the component must be reset() by the caller
 */
int vfs_path_first_component(const vfs_path_t* path,
			     vfs_path_component_t* component);

/**
 * @brief Point to the next component
 *
 * @return 0 on success
 */
int vfs_path_component_next(vfs_path_component_t* path);

/**
 * @brief Compare two components
 *
 * @return true if c1 == c2
 */
bool vfs_path_component_equals(const vfs_path_component_t* c1,
			       const vfs_path_component_t* c2);

/*
 * @brief Gets the basename of a path
 *
 * @param basename will represent the basename of path
 */
int vfs_path_basename(const vfs_path_t* path, vfs_path_t* basename);

/*
 * @brief Gets the dirname of a path
 *
 * @param dirname will represent the dirname of path
 */
int vfs_path_dirname(const vfs_path_t* path, vfs_path_t* dirname);

/**
 * @brief Compares two paths
 *
 * @return true if p1 and p2 represent the same path
 */
bool vfs_path_same(const vfs_path_t* p1, const vfs_path_t* p2);

/**
 * @brief Compares a vfs_path_t and a const char*
 *
 * @param p the vfs_path_t object
 * @param s the const char*
 * @param size size of s
 *
 * @return true if p and s represent the same path
 */
bool vfs_path_str_same(const vfs_path_t* p, const char* s,
		       vfs_path_size_t size);

void print_path(const vfs_path_t* path);
#endif
