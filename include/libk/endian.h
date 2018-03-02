#ifndef _LIBK_ENDIAN_H_
#define _LIBK_ENDIAN_H_

/**
 * @brief Little-endian to host 32 bits
 */
static inline uint32_t le2h32(uint32_t x);

/**
 * @brief Little-endian to host 16 bits
 */
static inline uint16_t le2h16(uint16_t x);

#if defined(ARCH_ENDIAN_LITTLE)
# include <libk/endian_little.h>
/* #elif defined(ARCH_ENDIAN_ENDIAN) */
/* # include <libk/endian_big.h> */
#else
# error Unsupported endianness
#endif

#endif
