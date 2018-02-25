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
 * Takes ownership of the path_str pointer
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
 * @brief Destroys a vfs_path_t object
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
 * @brief Starts splitting the path in components
 *
 * @param component will point to the first component of path
 */
int vfs_path_get_component(const vfs_path_t* path,
						   vfs_path_component_t* component);

/*
 * @brief Point to the next component
 *
 * @return @see vfs_path_get_next_compenent()
 */
int vfs_path_next_component(vfs_path_component_t* path);

/**
 * @brief Compare two names
 *
 * @return true if p1 == p2
 */
bool vfs_path_name_equals(const vfs_path_t* p1, const vfs_path_t* p2);

/**
 * @brief Compare two names
 *
 * @param size size of p2
 *
 * @return true if p1 == p2
 */
bool vfs_path_name_str_equals(const vfs_path_t* p1, const char* p2,
							  vfs_path_size_t size);

void print_path(const vfs_path_t* path);
#endif
