/*
 * Copyright (C) 2012-2013 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
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

#include "android_attr.h"
#include "../charonservice.h"

#include <utils/debug.h>
#include <library.h>

typedef struct private_android_attr_t private_android_attr_t;

/**
 * Private data of an android_attr_t object.
 */
struct private_android_attr_t {

	/**
	 * Public interface.
	 */
	android_attr_t public;
};

METHOD(attribute_handler_t, handle, bool,
	private_android_attr_t *this, ike_sa_t *ike_sa,
	configuration_attribute_type_t type, chunk_t data)
{
	vpnservice_builder_t *builder;
	host_t *dns;

	switch (type)
	{
		case INTERNAL_IP4_DNS:
			dns = host_create_from_chunk(AF_INET, data, 0);
			break;
		case INTERNAL_IP6_DNS:
			dns = host_create_from_chunk(AF_INET6, data, 0);
			break;
		default:
			return FALSE;
	}

	if (!dns || dns->is_anyaddr(dns))
	{
		DESTROY_IF(dns);
		return FALSE;
	}
	DBG1(DBG_IKE, "installing DNS server %H", dns);
	builder = charonservice->get_vpnservice_builder(charonservice);
	builder->add_dns(builder, dns);
	dns->destroy(dns);
	return TRUE;
}

METHOD(attribute_handler_t, release, void,
	private_android_attr_t *this, ike_sa_t *ike_sa,
	configuration_attribute_type_t type, chunk_t data)
{
	/* DNS servers cannot be removed from an existing TUN device */
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

METHOD(enumerator_t, enumerate_dns4, bool,
	enumerator_t *this, va_list args)
{
	configuration_attribute_type_t *type;
	chunk_t *data;

	VA_ARGS_VGET(args, type, data);
	*type = INTERNAL_IP4_DNS;
	*data = chunk_empty;
	this->venumerate = _enumerate_dns6;
	return TRUE;
}

METHOD(attribute_handler_t, create_attribute_enumerator, enumerator_t*,
	private_android_attr_t *this, ike_sa_t *ike_sa, linked_list_t *vips)
{
	enumerator_t *enumerator;

	INIT(enumerator,
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_dns4,
			.destroy = (void*)free,
	);
	return enumerator;
}

METHOD(android_attr_t, destroy, void,
	private_android_attr_t *this)
{
	free(this);
}

/**
 * Described in header
 */
android_attr_t *android_attr_create()
{
	private_android_attr_t *this;

	INIT(this,
		.public = {
			.handler = {
				.handle = _handle,
				.release = _release,
				.create_attribute_enumerator = _create_attribute_enumerator,
			},
			.destroy = _destroy,
		},
	);

	return &this->public;
}
