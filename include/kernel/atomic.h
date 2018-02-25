#ifndef _KERNEL_ATOMIC_H_
#define _KERNEL_ATOMIC_H_

typedef int atomic_int_t;

/**
 * @brief Initialiazes an atomic_int_t with a value
 */
static inline void atomic_int_init(volatile atomic_int_t* v, int i);

/**
 * @brief Returns the value of an atomic_int_t
 */
static inline int atomic_int_get(const volatile atomic_int_t* v);

/**
 * @brief Increments the value of an atomic_int_t
 */
static inline void atomic_int_inc(volatile atomic_int_t* v);

/**
 * @brief Decrements the value of an atomic_int_t
 */
static inline void atomic_int_dec(volatile atomic_int_t* v);


/*
 * Include platform specific implementation.
 */
#include <arch/atomic.h>

#endif
