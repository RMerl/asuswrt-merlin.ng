/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup memory_i memory
 * @{ @ingroup utils_i
 */

#ifndef MEMORY_H_
#define MEMORY_H_

/**
 * Helper function that compares two binary blobs for equality
 */
static inline bool memeq(const void *x, const void *y, size_t len)
{
	return memcmp(x, y, len) == 0;
}

/**
 * Same as memeq(), but with a constant runtime, safe for cryptographic use.
 */
bool memeq_const(const void *x, const void *y, size_t len);

/**
 * Calling memcpy() with NULL pointers, even with n == 0, results in undefined
 * behavior according to the C standard.  This version is guaranteed to not
 * access the pointers if n is 0.
 */
static inline void *memcpy_noop(void *dst, const void *src, size_t n)
{
	return n ? memcpy(dst, src, n) : dst;
}
#ifdef memcpy
# undef memcpy
#endif
#define memcpy(d,s,n) memcpy_noop(d,s,n)

/**
 * Calling memmove() with NULL pointers, even with n == 0, results in undefined
 * behavior according to the C standard.  This version is guaranteed to not
 * access the pointers if n is 0.
 */
static inline void *memmove_noop(void *dst, const void *src, size_t n)
{
	return n ? memmove(dst, src, n) : dst;
}
#ifdef memmove
# undef memmove
#endif
#define memmove(d,s,n) memmove_noop(d,s,n)

/**
 * Calling memset() with a NULL pointer, even with n == 0, results in undefined
 * behavior according to the C standard.  This version is guaranteed to not
 * access the pointer if n is 0.
 */
static inline void *memset_noop(void *s, int c, size_t n)
{
	return n ? memset(s, c, n) : s;
}
#ifdef memset
# undef memset
#endif
#define memset(s,c,n) memset_noop(s,c,n)

/**
 * Same as memcpy, but XORs src into dst instead of copy
 */
void memxor(uint8_t dest[], const uint8_t src[], size_t n);

/**
 * Safely overwrite n bytes of memory at ptr with zero, non-inlining variant.
 */
void memwipe_noinline(void *ptr, size_t n);

/**
 * Safely overwrite n bytes of memory at ptr with zero, inlining variant.
 */
static inline void memwipe_inline(void *ptr, size_t n)
{
	volatile char *c = (volatile char*)ptr;
	size_t m, i;

	/* byte wise until long aligned */
	for (i = 0; (uintptr_t)&c[i] % sizeof(long) && i < n; i++)
	{
		c[i] = 0;
	}
	/* word wise */
	if (n >= sizeof(long))
	{
		for (m = n - sizeof(long); i <= m; i += sizeof(long))
		{
			*(volatile long*)&c[i] = 0;
		}
	}
	/* byte wise of the rest */
	for (; i < n; i++)
	{
		c[i] = 0;
	}
}

/**
 * Safely overwrite n bytes of memory at ptr with zero, auto-inlining variant.
 */
static inline void memwipe(void *ptr, size_t n)
{
	if (!ptr)
	{
		return;
	}
	if (__builtin_constant_p(n))
	{
		memwipe_inline(ptr, n);
	}
	else
	{
		memwipe_noinline(ptr, n);
	}
}

/**
 * A variant of strstr with the characteristics of memchr, where haystack is not
 * a null-terminated string but simply a memory area of length n.
 */
void *memstr(const void *haystack, const char *needle, size_t n);

/**
 * Replacement for memrchr(3) if it is not provided by the C library.
 *
 * @param s		start of the memory area to search
 * @param c		character to search
 * @param n		length of memory area to search
 * @return		pointer to the found character or NULL
 */
void *utils_memrchr(const void *s, int c, size_t n);

#ifndef HAVE_MEMRCHR
#define memrchr(s,c,n) utils_memrchr(s,c,n)
#endif

#ifndef HAVE_FMEMOPEN
# ifdef HAVE_FUNOPEN
#  define HAVE_FMEMOPEN
#  define HAVE_FMEMOPEN_FALLBACK
#  include <stdio.h>
/**
 * fmemopen(3) fallback using BSD funopen.
 *
 * We could also provide one using fopencookie(), but should we have it we
 * most likely have fmemopen().
 *
 * fseek() is currently not supported.
 */
FILE *fmemopen(void *buf, size_t size, const char *mode);
# endif /* FUNOPEN */
#endif /* FMEMOPEN */

/**
 * printf hook for memory areas.
 *
 * Arguments are:
 *	u_char *ptr, u_int len
 */
int mem_printf_hook(printf_hook_data_t *data, printf_hook_spec_t *spec,
					const void *const *args);

#endif /** MEMORY_H_ @} */
