/*
 * Copyright (C) 2007 Martin Willi
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

#include "private_key.h"

/**
 * See header.
 */
bool private_key_equals(private_key_t *this, private_key_t *other)
{
	cred_encoding_type_t type;
	chunk_t a, b;

	if (this == other)
	{
		return TRUE;
	}

	for (type = 0; type < CRED_ENCODING_MAX; type++)
	{
		if (this->get_fingerprint(this, type, &a) &&
			other->get_fingerprint(other, type, &b))
		{
			return chunk_equals(a, b);
		}
	}
	return FALSE;
}

/**
 * See header.
 */
bool private_key_belongs_to(private_key_t *private, public_key_t *public)
{
	cred_encoding_type_t type;
	chunk_t a, b;

	for (type = 0; type < CRED_ENCODING_MAX; type++)
	{
		if (private->get_fingerprint(private, type, &a) &&
			public->get_fingerprint(public, type, &b))
		{
			return chunk_equals(a, b);
		}
	}
	return FALSE;
}

/**
 * See header.
 */
bool private_key_has_fingerprint(private_key_t *private, chunk_t fingerprint)
{
	cred_encoding_type_t type;
	chunk_t current;

	for (type = 0; type < KEYID_MAX; type++)
	{
		if (private->get_fingerprint(private, type, &current) &&
			chunk_equals(current, fingerprint))
		{
			return TRUE;
		}
	}
	return FALSE;
}

