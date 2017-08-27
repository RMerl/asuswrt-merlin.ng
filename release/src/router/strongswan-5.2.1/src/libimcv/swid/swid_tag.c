/*
 * Copyright (C) 2013-2014 Andreas Steffen
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

#include "swid_tag.h"

typedef struct private_swid_tag_t private_swid_tag_t;

/**
 * Private data of a swid_tag_t object.
 *
 */
struct private_swid_tag_t {

	/**
	 * Public swid_tag_t interface.
	 */
	swid_tag_t public;

	/**
	 * UTF-8 XML encoding of SWID tag
	 */
	chunk_t encoding;

	/**
	 * Optional Tag Identifier Instance ID
	 */
	chunk_t instance_id;

	/**
	 * Reference count
	 */
	refcount_t ref;
};

METHOD(swid_tag_t, get_encoding, chunk_t,
	private_swid_tag_t *this)
{
	return this->encoding;
}

METHOD(swid_tag_t, get_instance_id, chunk_t,
	private_swid_tag_t *this)
{
	return this->instance_id;
}

METHOD(swid_tag_t, get_ref, swid_tag_t*,
	private_swid_tag_t *this)
{
	ref_get(&this->ref);
	return &this->public;
}

METHOD(swid_tag_t, destroy, void,
	private_swid_tag_t *this)
{
	if (ref_put(&this->ref))
	{
		free(this->encoding.ptr);
		free(this->instance_id.ptr);
		free(this);
	}
}

/**
 * See header
 */
swid_tag_t *swid_tag_create(chunk_t encoding, chunk_t instance_id)
{
	private_swid_tag_t *this;

	INIT(this,
		.public = {
			.get_encoding = _get_encoding,
			.get_instance_id = _get_instance_id,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.encoding = chunk_clone(encoding),
		.ref = 1,
	);

	if (instance_id.len > 0)
	{
		this->instance_id = chunk_clone(instance_id);
	}

	return &this->public;
}

