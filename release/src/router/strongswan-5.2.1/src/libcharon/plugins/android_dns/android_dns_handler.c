/*
 * Copyright (C) 2010-2013 Tobias Brunner
 * Copyright (C) 2010 Martin Willi
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

#include "android_dns_handler.h"

#include <networking/host.h>
#include <collections/linked_list.h>

#include <cutils/properties.h>

typedef struct private_android_dns_handler_t private_android_dns_handler_t;

/**
 * Private data of an android_dns_handler_t object.
 */
struct private_android_dns_handler_t {

	/**
	 * Public interface
	 */
	android_dns_handler_t public;

	/**
	 * List of registered DNS servers
	 */
	linked_list_t *dns;
};

/**
 * Prefix to be used when installing DNS servers
 */
#define DNS_PREFIX_DEFAULT  "net"

/**
 * Struct to store a pair of old and installed DNS servers
 */
typedef struct {
	/** installed dns server */
	host_t *dns;
	/** old dns server */
	host_t *old;
} dns_pair_t;

/**
 * Destroy a pair of old and installed DNS servers
 */
static void destroy_dns_pair(dns_pair_t *this)
{
	DESTROY_IF(this->dns);
	DESTROY_IF(this->old);
	free(this);
}

/**
 * Filter pairs of DNS servers
 */
static bool filter_dns_pair(void *data, dns_pair_t **in, host_t **out)
{
	*out = (*in)->dns;
	return TRUE;
}

/**
 * Read DNS server property with a given index
 */
static host_t *get_dns_server(private_android_dns_handler_t *this, int index)
{
	host_t *dns = NULL;
	char key[10], value[PROPERTY_VALUE_MAX],
		 *prefix = DNS_PREFIX_DEFAULT;

	if (snprintf(key, sizeof(key), "%s.dns%d", prefix, index) >= sizeof(key))
	{
		return NULL;
	}

	if (property_get(key, value, NULL) > 0)
	{
		dns = host_create_from_string(value, 0);
	}
	return dns;
}

/**
 * Set DNS server property with a given index
 */
static bool set_dns_server(private_android_dns_handler_t *this, int index,
						   host_t *dns)
{
	char key[10], value[PROPERTY_VALUE_MAX],
		 *prefix = DNS_PREFIX_DEFAULT;

	if (snprintf(key, sizeof(key), "%s.dns%d", prefix, index) >= sizeof(key))
	{
		return FALSE;
	}

	if (dns)
	{
		if (snprintf(value, sizeof(value), "%H", dns) >= sizeof(value))
		{
			return FALSE;
		}
	}
	else
	{
		value[0] = '\0';
	}

	if (property_set(key, value) != 0)
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(attribute_handler_t, handle, bool,
	private_android_dns_handler_t *this, identification_t *id,
	configuration_attribute_type_t type, chunk_t data)
{
	switch (type)
	{
		case INTERNAL_IP4_DNS:
		{
			host_t *dns;
			dns_pair_t *pair;
			int index;

			dns = host_create_from_chunk(AF_INET, data, 0);
			if (dns)
			{
				pair = malloc_thing(dns_pair_t);
				pair->dns = dns;
				index = this->dns->get_count(this->dns) + 1;
				pair->old = get_dns_server(this, index);
				set_dns_server(this, index, dns);
				this->dns->insert_last(this->dns, pair);
				return TRUE;
			}
			return FALSE;
		}
		default:
			return FALSE;
	}
}

METHOD(attribute_handler_t, release, void,
	private_android_dns_handler_t *this, identification_t *server,
	configuration_attribute_type_t type, chunk_t data)
{
	if (type == INTERNAL_IP4_DNS)
	{
		enumerator_t *enumerator;
		dns_pair_t *pair;
		int index;

		enumerator = this->dns->create_enumerator(this->dns);
		for (index = 1; enumerator->enumerate(enumerator, &pair); index++)
		{
			if (chunk_equals(pair->dns->get_address(pair->dns), data))
			{
				this->dns->remove_at(this->dns, enumerator);
				set_dns_server(this, index, pair->old);
				destroy_dns_pair(pair);
			}
		}
		enumerator->destroy(enumerator);
	}
}

METHOD(enumerator_t, enumerate_dns, bool,
	enumerator_t *this, configuration_attribute_type_t *type, chunk_t *data)
{
	*type = INTERNAL_IP4_DNS;
	*data = chunk_empty;
	/* stop enumeration */
	this->enumerate = (void*)return_false;
	return TRUE;
}

METHOD(attribute_handler_t, create_attribute_enumerator, enumerator_t *,
	private_android_dns_handler_t *this, identification_t *id,
	linked_list_t *vips)
{
	enumerator_t *enumerator;

	INIT(enumerator,
		.enumerate = (void*)_enumerate_dns,
		.destroy = (void*)free,
	);
	return enumerator;
}

METHOD(android_dns_handler_t, destroy, void,
	private_android_dns_handler_t *this)
{
	this->dns->destroy_function(this->dns, (void*)destroy_dns_pair);
	free(this);
}

/**
 * See header
 */
android_dns_handler_t *android_dns_handler_create()
{
	private_android_dns_handler_t *this;

	INIT(this,
		.public = {
			.handler = {
				.handle = _handle,
				.release = _release,
				.create_attribute_enumerator = _create_attribute_enumerator,
			},
			.destroy = _destroy,
		},
		.dns = linked_list_create(),
	);

	return &this->public;
}

