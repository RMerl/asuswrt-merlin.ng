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
 * @defgroup align_i align
 * @{ @ingroup utils_i
 */

#ifndef ALIGN_H_
#define ALIGN_H_

/**
 * Macro gives back larger of two values.
 */
#define max(x,y) ({ \
	typeof(x) _x = (x); \
	typeof(y) _y = (y); \
	_x > _y ? _x : _y; })

/**
 * Macro gives back smaller of two values.
 */
#define min(x,y) ({ \
	typeof(x) _x = (x); \
	typeof(y) _y = (y); \
	_x < _y ? _x : _y; })

/**
 * Get the padding required to make size a multiple of alignment
 */
static inline size_t pad_len(size_t size, size_t alignment)
{
	size_t remainder;

	remainder = size % alignment;
	return remainder ? alignment - remainder : 0;
}

/**
 * Round up size to be multiple of alignment
 */
static inline size_t round_up(size_t size, size_t alignment)
{
	return size + pad_len(size, alignment);
}

/**
 * Round down size to be a multiple of alignment
 */
static inline size_t round_down(size_t size, size_t alignment)
{
	return size - (size % alignment);
}

/**
 * malloc(), but returns aligned memory.
 *
 * The returned pointer must be freed using free_align(), not free().
 *
 * @param size			size of allocated data
 * @param align			alignment, up to 255 bytes, usually a power of 2
 * @return				allocated hunk, aligned to align bytes
 */
void* malloc_align(size_t size, uint8_t align);

/**
 * Free a hunk allocated by malloc_align().
 *
 * @param ptr			hunk to free
 */
void free_align(void *ptr);

#endif /** ALIGN_H_ @} */
