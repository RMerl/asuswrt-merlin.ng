/*
 * Copyright (C) 2021 Tobias Brunner
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

#define _GNU_SOURCE
#include <stdio.h>
#ifdef USE_SELINUX
#include <selinux/selinux.h>
#endif

#include "sec_label.h"

ENUM(sec_label_mode_names, SEC_LABEL_MODE_SYSTEM, SEC_LABEL_MODE_SELINUX,
	"system",
	"simple",
	"selinux",
);

typedef struct private_sec_label_t private_sec_label_t;

/**
 * Private data.
 */
struct private_sec_label_t {

	/**
	 * Public interface
	 */
	sec_label_t public;

	/**
	 * Encoded label value
	 */
	chunk_t encoding;

	/**
	 * String representation of the label
	 */
	char *str;
};

static sec_label_t *create_sec_label(chunk_t encoding, char *str);

METHOD(sec_label_t, get_encoding, chunk_t,
	private_sec_label_t *this)
{
	return this->encoding;
}

METHOD(sec_label_t, get_string, char*,
	private_sec_label_t *this)
{
	return this->str;
}

METHOD(sec_label_t, clone_, sec_label_t*,
	private_sec_label_t *this)
{
	return create_sec_label(chunk_clone(this->encoding), strdup(this->str));
}

METHOD(sec_label_t, equals, bool,
	private_sec_label_t *this, sec_label_t *other_pub)
{
	private_sec_label_t *other = (private_sec_label_t*)other_pub;

	if (!other_pub)
	{
		return FALSE;
	}
	return chunk_equals_const(this->encoding, other->encoding);
}

METHOD(sec_label_t, matches, bool,
	private_sec_label_t *this, sec_label_t *other_pub)
{
	if (!other_pub)
	{
		return FALSE;
	}
#ifdef USE_SELINUX
	if (is_selinux_enabled())
	{	/* if disabled, the following matches anything against anything */
		private_sec_label_t *other = (private_sec_label_t*)other_pub;
		return selinux_check_access(other->str, this->str, "association",
									"polmatch", NULL) == 0;
	}
#endif
	return equals(this, other_pub);
}

METHOD(sec_label_t, hash, u_int,
	private_sec_label_t *this, u_int inc)
{
	return chunk_hash_inc(this->encoding, inc);
}

METHOD(sec_label_t, destroy, void,
	private_sec_label_t *this)
{
	chunk_free(&this->encoding);
	free(this->str);
	free(this);
}

/**
 * Internal constructor, data is adopted
 */
static sec_label_t *create_sec_label(chunk_t encoding, char *str)
{
	private_sec_label_t *this;

	INIT(this,
		.public = {
			.get_encoding = _get_encoding,
			.get_string = _get_string,
			.clone = _clone_,
			.matches = _matches,
			.equals = _equals,
			.hash = _hash,
			.destroy = _destroy,
		},
		.encoding = encoding,
		.str = str,
	);
	return &this->public;
}

/*
 * Described in header
 */
sec_label_t *sec_label_from_encoding(const chunk_t value)
{
	chunk_t cloned, sanitized = chunk_empty;
	char *str;

	if (!value.len || (value.len == 1 && !value.ptr[0]))
	{
		DBG1(DBG_LIB, "invalid empty security label");
		return NULL;
	}
	else if (value.ptr[value.len-1])
	{
		DBG1(DBG_LIB, "adding null-terminator to security label");
		cloned = chunk_cat("cc", value, chunk_from_chars(0x00));
	}
	else
	{
		cloned = chunk_clone(value);
	}

	/* create a sanitized version while ignoring the null-terminator */
	if (!chunk_printable(chunk_create(cloned.ptr, cloned.len-1), &sanitized, '?'))
	{
#ifdef USE_SELINUX
		/* don't accept labels with non-printable characters if we use SELinux */
		DBG1(DBG_LIB, "invalid security label with non-printable characters %B",
			 &value);
		chunk_free(&sanitized);
		chunk_free(&cloned);
		return NULL;
#endif
	}
	if (asprintf(&str, "%.*s", (int)sanitized.len, sanitized.ptr) <= 0)
	{
		chunk_free(&sanitized);
		chunk_free(&cloned);
		return NULL;
	}
	chunk_free(&sanitized);

	return create_sec_label(cloned, str);
}

/*
 * Described in header
 */
sec_label_t *sec_label_from_string(const char *value)
{
	if (!value)
	{
		return NULL;
	}
	return sec_label_from_encoding(chunk_create((char*)value, strlen(value)+1));
}

/*
 * Described in header
 */
bool sec_label_mode_from_string(const char *value, sec_label_mode_t *mode)
{
	sec_label_mode_t def = sec_label_mode_default();

	return enum_from_name(sec_label_mode_names, value, mode) &&
		(def == SEC_LABEL_MODE_SELINUX || *mode != SEC_LABEL_MODE_SELINUX);
}

/*
 * Described in header
 */
sec_label_mode_t sec_label_mode_default()
{
#ifdef USE_SELINUX
	if (is_selinux_enabled())
	{
		return SEC_LABEL_MODE_SELINUX;
	}
#endif
	return SEC_LABEL_MODE_SIMPLE;
}
