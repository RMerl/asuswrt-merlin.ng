/*
 * Keep all the ugly #ifdef for system stuff here
 */

#ifndef __COMPILER_H__
#define __COMPILER_H__

#include <stddef.h>
#include <stdbool.h>

#ifdef USE_HOSTCC

#if defined(__BEOS__)	 || \
    defined(__NetBSD__)  || \
    defined(__FreeBSD__) || \
    defined(__sun__)	 || \
    defined(__APPLE__)
# include <inttypes.h>
#elif defined(__linux__) || defined(__WIN32__) || defined(__MINGW32__) || defined(__OpenBSD__)
# include <stdint.h>
#endif

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if !defined(__WIN32__) && !defined(__MINGW32__)
# include <sys/mman.h>
#endif

/* Not all systems (like Windows) has this define, and yes
 * we do replace/emulate mmap() on those systems ...
 */
#ifndef MAP_FAILED
# define MAP_FAILED ((void *)-1)
#endif

#include <fcntl.h>
#ifndef O_BINARY		/* should be define'd on __WIN32__ */
#define O_BINARY	0
#endif

#ifdef __linux__
# include <endian.h>
# include <byteswap.h>
#elif defined(__MACH__) || defined(__FreeBSD__)
# include <machine/endian.h>
#endif
#ifdef __FreeBSD__
# include <sys/endian.h> /* htole32 and friends */
# define __BYTE_ORDER BYTE_ORDER
# define __LITTLE_ENDIAN LITTLE_ENDIAN
# define __BIG_ENDIAN BIG_ENDIAN
#elif defined(__OpenBSD__)
# include <endian.h>
# define __BYTE_ORDER BYTE_ORDER
# define __LITTLE_ENDIAN LITTLE_ENDIAN
# define __BIG_ENDIAN BIG_ENDIAN
#endif

#include <time.h>

typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef unsigned int uint;
typedef unsigned long ulong;

/* Define these on the host so we can build some target code */
typedef __u32 u32;

#define uswap_16(x) \
	((((x) & 0xff00) >> 8) | \
	 (((x) & 0x00ff) << 8))
#define uswap_32(x) \
	((((x) & 0xff000000) >> 24) | \
	 (((x) & 0x00ff0000) >>  8) | \
	 (((x) & 0x0000ff00) <<  8) | \
	 (((x) & 0x000000ff) << 24))
#define _uswap_64(x, sfx) \
	((((x) & 0xff00000000000000##sfx) >> 56) | \
	 (((x) & 0x00ff000000000000##sfx) >> 40) | \
	 (((x) & 0x0000ff0000000000##sfx) >> 24) | \
	 (((x) & 0x000000ff00000000##sfx) >>  8) | \
	 (((x) & 0x00000000ff000000##sfx) <<  8) | \
	 (((x) & 0x0000000000ff0000##sfx) << 24) | \
	 (((x) & 0x000000000000ff00##sfx) << 40) | \
	 (((x) & 0x00000000000000ff##sfx) << 56))
#if defined(__GNUC__)
# define uswap_64(x) _uswap_64(x, ull)
#else
# define uswap_64(x) _uswap_64(x, )
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define cpu_to_le16(x)		(x)
# define cpu_to_le32(x)		(x)
# define cpu_to_le64(x)		(x)
# define le16_to_cpu(x)		(x)
# define le32_to_cpu(x)		(x)
# define le64_to_cpu(x)		(x)
# define cpu_to_be16(x)		uswap_16(x)
# define cpu_to_be32(x)		uswap_32(x)
# define cpu_to_be64(x)		uswap_64(x)
# define be16_to_cpu(x)		uswap_16(x)
# define be32_to_cpu(x)		uswap_32(x)
# define be64_to_cpu(x)		uswap_64(x)
#else
# define cpu_to_le16(x)		uswap_16(x)
# define cpu_to_le32(x)		uswap_32(x)
# define cpu_to_le64(x)		uswap_64(x)
# define le16_to_cpu(x)		uswap_16(x)
# define le32_to_cpu(x)		uswap_32(x)
# define le64_to_cpu(x)		uswap_64(x)
# define cpu_to_be16(x)		(x)
# define cpu_to_be32(x)		(x)
# define cpu_to_be64(x)		(x)
# define be16_to_cpu(x)		(x)
# define be32_to_cpu(x)		(x)
# define be64_to_cpu(x)		(x)
#endif

#else /* !USE_HOSTCC */

/* Type for `void *' pointers. */
typedef unsigned long int uintptr_t;

#include <linux/string.h>
#include <linux/types.h>
#include <asm/byteorder.h>

#if __SIZEOF_LONG__ == 8
# define __WORDSIZE	64
#elif __SIZEOF_LONG__ == 4
# define __WORDSIZE	32
#else
/*
 * Assume 32-bit for now - only newer toolchains support this feature and
 * this is only required for sandbox support at present.
 */
#define __WORDSIZE	32
#endif

#endif /* USE_HOSTCC */

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

#ifdef __LP64__
#define MEM_SUPPORT_64BIT_DATA	1
#else
#define MEM_SUPPORT_64BIT_DATA	0
#endif

/**
 * tools_build() - check if we are building host tools
 *
 * Return: true if building for the host, false if for a target
 */
static inline bool tools_build(void)
{
#ifdef USE_HOSTCC
	return true;
#else
	return false;
#endif
}

#endif
