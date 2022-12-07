/*
 * Copyright (C) 2021 Tobias Brunner
 * Copyright (C) 2021 Thomas Egerer
 *
 * Copyright (C) secunet Security Networks AG
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

#include "metadata_int.h"

#include <library.h>

typedef struct private_metadata_t private_metadata_t;

/**
 * Private data
 */
struct private_metadata_t {

	/**
	 * Public interface
	 */
	metadata_t public;

	/**
	 * String representation of type.
	 */
	const char *type;

	/**
	 * Stored value.
	 */
	uint64_t value;
};

/* forward declaration */
static metadata_t *create_generic(const char *type, uint64_t value);

METHOD(metadata_t, get_type, const char*,
	private_metadata_t *this)
{
	return this->type;
}

METHOD(metadata_t, clone_, metadata_t*,
	private_metadata_t *this)
{
	return create_generic(this->type, this->value);
}

METHOD(metadata_t, equals, bool,
	private_metadata_t *this, ...)
{
	/* bool, uint8, uint16 etc. are promoted to int when passed via ... */
	if (streq(METADATA_TYPE_INT, this->type))
	{
		int value;

		VA_ARGS_GET(this, value);
		return value == (int)this->value;
	}
	else if (streq(METADATA_TYPE_UINT64, this->type))
	{
		uint64_t value;

		VA_ARGS_GET(this, value);
		return value == this->value;
	}
	return FALSE;
}

METHOD(metadata_t, get, void,
	private_metadata_t *this, ...)
{
	/* pointers here have to match exactly, so passing e.g. a bool* or uint16_t
	 * are illegal */
	if (streq(METADATA_TYPE_INT, this->type))
	{
		int *value;

		VA_ARGS_GET(this, value);
		*value = this->value;
	}
	else if (streq(METADATA_TYPE_UINT64, this->type))
	{
		uint64_t *value;

		VA_ARGS_GET(this, value);
		*value = this->value;
	}
}

METHOD(metadata_t, destroy, void,
	private_metadata_t *this)
{
	free(this);
}

/**
 * Generic constructor
 */
static metadata_t *create_generic(const char *type, uint64_t value)
{
	private_metadata_t *this;

	INIT(this,
		.public = {
			.get_type = _get_type,
			.clone = _clone_,
			.equals = _equals,
			.get = _get,
			.destroy = _destroy,
		},
		.type = type,
		.value = value,
	);
	return &this->public;
}

/*
 * Described in header
 */
metadata_t *metadata_create_int(const char *type, va_list args)
{
	metadata_t *this = NULL;

	/* bool, uint8, uint16 etc. are promoted to int when passed via ... */
	if (streq(METADATA_TYPE_INT, type))
	{
		int int_val;

		VA_ARGS_VGET(args, int_val);
		this = create_generic(METADATA_TYPE_INT, int_val);
	}
	else if (streq(METADATA_TYPE_UINT64, type))
	{
		uint64_t uint64_val;

		VA_ARGS_VGET(args, uint64_val);
		this = create_generic(METADATA_TYPE_UINT64, uint64_val);
	}
	return this;
}
