/*
 * Copyright (C) 2016 Tobias Brunner
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
#include <collections/array.h>

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
	 * Received DNS server attributes, chunk_t
	 */
	array_t *dns;

	/**
	 * Received IPv6 DNS server attributes, chunk_t
	 */
	array_t *dns6;

	/**
	 * Received NBNS server attributes, chunk_t
	 */
	array_t *nbns;
};

METHOD(attribute_handler_t, handle, bool,
	private_nm_handler_t *this, ike_sa_t *ike_sa,
	configuration_attribute_type_t type, chunk_t data)
{
	array_t *list;

	switch (type)
	{
		case INTERNAL_IP4_DNS:
			list = this->dns;
			break;
		case INTERNAL_IP6_DNS:
			list = this->dns6;
			break;
		case INTERNAL_IP4_NBNS:
			list = this->nbns;
			break;
		default:
			return FALSE;
	}
	data = chunk_clone(data);
	array_insert(list, ARRAY_TAIL, &data);
	return TRUE;
}

METHOD(enumerator_t, enumerate_dns6, bool,
	enumerator_t *this, va_list args)
{
	configuration_attribute_type_t *type;
	chunk_t *data;

	VA_ARGS_VGET(args, type, data);
	*type = INTERNAL_IP6_DNS;
	*data = chunk_empty;
	this->venumerate = (void*)return_false;
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
	/* enumerate IPv6 DNS server as next attribute ... */
	this->venumerate = _enumerate_dns6;
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

METHOD(nm_handler_t, create_enumerator, enumerator_t*,
	private_nm_handler_t *this, configuration_attribute_type_t type)
{
	array_t *list;

	switch (type)
	{
		case INTERNAL_IP4_DNS:
			list = this->dns;
			break;
		case INTERNAL_IP6_DNS:
			list = this->dns6;
			break;
		case INTERNAL_IP4_NBNS:
			list = this->nbns;
			break;
		default:
			return enumerator_create_empty();
	}
	return array_create_enumerator(list);
}

METHOD(nm_handler_t, reset, void,
	private_nm_handler_t *this)
{
	chunk_t chunk;

	while (array_remove(this->dns, ARRAY_TAIL, &chunk))
	{
		chunk_free(&chunk);
	}
	while (array_remove(this->dns6, ARRAY_TAIL, &chunk))
	{
		chunk_free(&chunk);
	}
	while (array_remove(this->nbns, ARRAY_TAIL, &chunk))
	{
		chunk_free(&chunk);
	}
}

METHOD(nm_handler_t, destroy, void,
	private_nm_handler_t *this)
{
	reset(this);
	array_destroy(this->dns);
	array_destroy(this->dns6);
	array_destroy(this->nbns);
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
		.dns = array_create(sizeof(chunk_t), 0),
		.dns6 = array_create(sizeof(chunk_t), 0),
		.nbns = array_create(sizeof(chunk_t), 0),
	);

	return &this->public;
}
