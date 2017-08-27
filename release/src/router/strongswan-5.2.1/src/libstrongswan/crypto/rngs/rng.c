/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
 * Hochschule fuer Technik Rapperswil
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

#include "rng.h"

ENUM(rng_quality_names, RNG_WEAK, RNG_TRUE,
	"RNG_WEAK",
	"RNG_STRONG",
	"RNG_TRUE",
);

/*
 * Described in header.
 */
bool rng_get_bytes_not_zero(rng_t *rng, size_t len, u_int8_t *buffer, bool all)
{
	u_int8_t *pos = buffer, *check = buffer + (all ? len : min(1, len));

	if (!rng->get_bytes(rng, len, pos))
	{
		return FALSE;
	}

	for (; pos < check; pos++)
	{
		while (*pos == 0)
		{
			if (!rng->get_bytes(rng, 1, pos))
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

/*
 * Described in header.
 */
bool rng_allocate_bytes_not_zero(rng_t *rng, size_t len, chunk_t *chunk,
								 bool all)
{
	*chunk = chunk_alloc(len);
	if (!rng_get_bytes_not_zero(rng, len, chunk->ptr, all))
	{
		chunk_clear(chunk);
		return FALSE;
	}
	return TRUE;
}
