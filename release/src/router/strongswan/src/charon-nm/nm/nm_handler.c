/*
 * Copyright (C) 2009 Martin Willi
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

#include "nm_handler.h"

#include <daemon.h>

typedef struct private_nm_handler_t private_nm_handler_t;

/**
 * Private data of an nm_handler_t object.
 */
struct private_nm_handler_t {

	/**
	 * Public nm_handler_t interface.
	 */
	nm_handler_t public;

	/**
	 * list of received DNS server attributes, pointer to 4 byte data
	 */
	linked_list_t *dns;

	/**
	 * list of received NBNS server attributes, pointer to 4 byte data
	 */
	linked_list_t *nbns;
};

METHOD(attribute_handler_t, handle, bool,
	private_nm_handler_t *this, ike_sa_t *ike_sa,
	configuration_attribute_type_t type, chunk_t data)
{
	linked_list_t *list;

	switch (type)
	{
		case INTERNAL_IP4_DNS:
			list = this->dns;
			break;
		case INTERNAL_IP4_NBNS:
			list = this->nbns;
			break;
		default:
			return FALSE;
	}
	if (data.len != 4)
	{
		return FALSE;
	}
	list->insert_last(list, chunk_clone(data).ptr);
	return TRUE;
}

METHOD(enumerator_t, enumerate_nbns, bool,
	enumerator_t *this, va_list args)
{
	configuration_attribute_type_t *type;
	chunk_t *data;

	VA_ARGS_VGET(args, type, data);
	*type = INTERNAL_IP4_NBNS;
	*data = chunk_empty;
	this->venumerate = (void*)return_false;
	return TRUE;
}

/**
 * Implementation of create_attribute_enumerator().enumerate() for DNS
 */
METHOD(enumerator_t, enumerate_dns, bool,
	enumerator_t *this, va_list args)
{
	configuration_attribute_type_t *type;
	chunk_t *data;

	VA_ARGS_VGET(args, type, data);
	*type = INTERNAL_IP4_DNS;
	*data = chunk_empty;
	/* enumerate WINS server as next attribute ... */
	this->venumerate = _enumerate_nbns;
	return TRUE;
}

METHOD(attribute_handler_t, create_attribute_enumerator, enumerator_t*,
	private_nm_handler_t *this, ike_sa_t *ike_sa, linked_list_t *vips)
{
	if (vips->get_count(vips))
	{
		enumerator_t *enumerator;

		INIT(enumerator,
			/* enumerate DNS attribute first ... */
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_dns,
			.destroy = (void*)free,
		);
		return enumerator;
	}
	return enumerator_create_empty();
}

CALLBACK(filter_chunks, bool,
	void *null, enumerator_t *orig, va_list args)
{
	chunk_t *out;
	char *ptr;

	VA_ARGS_VGET(args, out);

	if (orig->enumerate(orig, &ptr))
	{
		*out = chunk_create(ptr, 4);
		return TRUE;
	}
	return FALSE;
}

METHOD(nm_handler_t, create_enumerator, enumerator_t*,
	private_nm_handler_t *this, configuration_attribute_type_t type)
{
	linked_list_t *list;

	switch (type)
	{
		case INTERNAL_IP4_DNS:
			list = this->dns;
			break;
		case INTERNAL_IP4_NBNS:
			list = this->nbns;
			break;
		default:
			return enumerator_create_empty();
	}
	return enumerator_create_filter(list->create_enumerator(list),
									filter_chunks, NULL, NULL);
}

METHOD(nm_handler_t, reset, void,
	private_nm_handler_t *this)
{
	void *data;

	while (this->dns->remove_last(this->dns, (void**)&data) == SUCCESS)
	{
		free(data);
	}
	while (this->nbns->remove_last(this->nbns, (void**)&data) == SUCCESS)
	{
		free(data);
	}
}

METHOD(nm_handler_t, destroy, void,
	private_nm_handler_t *this)
{
	reset(this);
	this->dns->destroy(this->dns);
	this->nbns->destroy(this->nbns);
	free(this);
}

/**
 * See header
 */
nm_handler_t *nm_handler_create()
{
	private_nm_handler_t *this;

	INIT(this,
		.public = {
			.handler = {
				.handle = _handle,
				.release = nop,
				.create_attribute_enumerator = _create_attribute_enumerator,
			},
			.create_enumerator = _create_enumerator,
			.reset = _reset,
			.destroy = _destroy,
		},
		.dns = linked_list_create(),
		.nbns = linked_list_create(),
	);

	return &this->public;
}
