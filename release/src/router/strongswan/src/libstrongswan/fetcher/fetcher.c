/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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

#include "fetcher.h"

/**
 * See header.
 */
bool fetcher_default_callback(void *userdata, chunk_t chunk)
{
	chunk_t *accu = userdata;

	accu->ptr = realloc(accu->ptr, accu->len + chunk.len);
	if (accu->ptr)
	{
		memcpy(&accu->ptr[accu->len], chunk.ptr, chunk.len);
		accu->len += chunk.len;
		return TRUE;
	}
	return FALSE;
}
