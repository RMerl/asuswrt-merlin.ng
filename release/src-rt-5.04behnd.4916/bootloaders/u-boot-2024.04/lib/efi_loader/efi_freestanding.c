// SPDX-License-Identifier: GPL-2.0+
/*
 * Library for freestanding binary
 *
 * Copyright 2019, Heinrich Schuchardt <xypron.glpk@gmx.de>
 *
 * GCC requires that freestanding programs provide memcpy(), memmove(),
 * memset(), and memcmp().
 */

#include <linux/types.h>

/**
 * memcmp() - compare memory areas
 *
 * @s1:		pointer to first area
 * @s2:		pointer to second area
 * @n:		number of bytes to compare
 * Return:	0 if both memory areas are the same, otherwise the sign of the
 *		result value is the same as the sign of the difference between
 *		the first differing pair of bytes taken as u8.
 */
int memcmp(const void *s1, const void *s2, size_t n)
{
	const u8 *pos1 = s1;
	const u8 *pos2 = s2;

	for (; n; --n) {
		if (*pos1 != *pos2)
			return *pos1 - *pos2;
		++pos1;
		++pos2;
	}
	return 0;
}

/**
 * memcpy() - copy memory area
 *
 * @dest:	destination buffer
 * @src:	source buffer
 * @n:		number of bytes to copy
 * Return:	pointer to destination buffer
 */
void *memmove(void *dest, const void *src, size_t n)
{
	u8 *d = dest;
	const u8 *s = src;

	if (d <= s) {
		for (; n; --n)
			*d++ = *s++;
	} else {
		d += n;
		s += n;
		for (; n; --n)
			*--d = *--s;
	}
	return dest;
}

/**
 * memcpy() - copy memory area
 *
 * @dest:	destination buffer
 * @src:	source buffer
 * @n:		number of bytes to copy
 * Return:	pointer to destination buffer
 */
void *memcpy(void *dest, const void *src, size_t n)
{
	return memmove(dest, src, n);
}

/**
 * memset() - fill memory with a constant byte
 *
 * @s:		destination buffer
 * @c:		byte value
 * @n:		number of bytes to set
 * Return:	pointer to destination buffer
 */
void *memset(void *s, int c, size_t n)
{
	u8 *d = s;

	for (; n; --n)
		*d++ = c;
	return s;
}

/**
 * __cyg_profile_func_enter() - record function entry
 *
 * This is called on every function entry when compiling with
 * -finstrument-functions.
 *
 * We do nothing here.
 *
 * func_ptr:	Pointer to function being entered
 * caller:	Pointer to function which called this function
 */
void notrace
__cyg_profile_func_enter(void *func_ptr, void *caller)
{
}

/**
 * __cyg_profile_func_exit() - record function exit
 *
 * This is called on every function exit when compiling with
 * -finstrument-functions.
 *
 * We do nothing here.
 *
 * func_ptr:	Pointer to function being entered
 * caller:	Pointer to function which called this function
 */
void notrace
__cyg_profile_func_exit(void *func_ptr, void *caller)
{
}
