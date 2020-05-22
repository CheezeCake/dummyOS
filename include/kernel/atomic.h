#ifndef _KERNEL_ATOMIC_H_
#define _KERNEL_ATOMIC_H_

#include <kernel/types.h>

typedef int32_t atomic_int_t;

/**
 * @brief Initialiazes an atomic_int_t with a value
 */
static inline void atomic_int_init(volatile atomic_int_t* v, int32_t i);

/**
 * @brief Returns the value of an atomic_int_t
 */
static inline int32_t atomic_int_load(const volatile atomic_int_t* v);

/**
 * @brief Sets the value of an atomic_int_t
 */
static inline void atomic_int_store(volatile atomic_int_t* v, int32_t i);

/**
 * @brief Increments the value of an atomic_int_t
 */
static inline void atomic_int_inc(volatile atomic_int_t* v);

/**
 * @brief Decrements the value of an atomic_int_t
 */
static inline void atomic_int_dec(volatile atomic_int_t* v);

/**
 * @brief Increments the value of an atomic_int_t and returns the result
 */
static inline int32_t atomic_int_inc_return(volatile atomic_int_t* v);

/**
 * @brief Decrements the value of an atomic_int_t and returns the result
 */
static inline int32_t atomic_int_dec_return(volatile atomic_int_t* v);


/*
 * Include platform specific implementation.
 */
#include <arch/atomic.h>

#endif
