#ifndef _LIBK_REFCOUNT_H_
#define _LIBK_REFCOUNT_H_

#include <kernel/atomic.h>

/**
 * @brief Atomic reference counter structure
 */
typedef struct refcount {
	atomic_int_t cnt;
} refcount_t;

/**
 * @brief Initializes the reference counter to one
 */
void refcount_init(refcount_t* r);

/**
 * @brief Initializes the reference counter to zero
 */
void refcount_init_zero(refcount_t* r);

/**
 * @brief Increments the reference counter by one
 */
int refcount_inc(refcount_t* r);

/**
 * @brief Decrements the reference counter by one
 */
int refcount_dec(refcount_t* r);

/**
 * @brief Returns the reference counter value
 */
int refcount_get(const refcount_t* r);

#endif
