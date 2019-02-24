/*
 * Copyright (C) 2008-2014 Tobias Brunner
 * Copyright (C) 2005-2008 Martin Willi
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

#include <utils/utils.h>
#include <utils/debug.h>

/**
 * Described in header.
 */
void* malloc_align(size_t size, uint8_t align)
{
	uint8_t pad;
	void *ptr;

	if (align == 0)
	{
		align = 1;
	}
	ptr = malloc(align + sizeof(pad) + size);
	if (!ptr)
	{
		return NULL;
	}
	/* store padding length just before data, down to the allocation boundary
	 * to do some verification during free_align() */
	pad = align - ((uintptr_t)ptr % align);
	memset(ptr, pad, pad);
	return ptr + pad;
}

/**
 * Described in header.
 */
void free_align(void *ptr)
{
	uint8_t pad, *pos;

	pos = ptr - 1;
	/* verify padding to check any corruption */
	for (pad = *pos; (void*)pos >= ptr - pad; pos--)
	{
		if (*pos != pad)
		{
			DBG1(DBG_LIB, "!!!! invalid free_align() !!!!");
			return;
		}
	}
	free(ptr - pad);
}
