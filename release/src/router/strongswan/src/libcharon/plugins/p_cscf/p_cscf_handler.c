/*
 * Copyright (C) 2016 Tobias Brunner
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

#include "p_cscf_handler.h"

#include <networking/host.h>
#include <utils/debug.h>

typedef struct private_p_cscf_handler_t private_p_cscf_handler_t;

/**
 * Private data
 */
struct private_p_cscf_handler_t {

	/**
	 * Public interface
	 */
	p_cscf_handler_t public;
};

METHOD(attribute_handler_t, handle, bool,
	private_p_cscf_handler_t *this, ike_sa_t *ike_sa,
	configuration_attribute_type_t type, chunk_t data)
{
	host_t *server;
	int family = AF_INET6;

	switch (type)
	{
		case P_CSCF_IP4_ADDRESS:
			family = AF_INET;
			/* fall-through */
		case P_CSCF_IP6_ADDRESS:
			server = host_create_from_chunk(family, data, 0);
			if (!server)
			{
				DBG1(DBG_CFG, "received invalid P-CSCF server IP");
				return FALSE;
			}
			DBG1(DBG_CFG, "received P-CSCF server IP %H", server);
			server->destroy(server);
			return TRUE;
		default:
			return FALSE;
	}
}

METHOD(attribute_handler_t, release, void,
	private_p_cscf_handler_t *this, ike_sa_t *ike_sa,
	configuration_attribute_type_t type, chunk_t data)
{
	switch (type)
	{
		case P_CSCF_IP4_ADDRESS:
		case P_CSCF_IP6_ADDRESS:
			/* nothing to do as we only log the server IPs */
			break;
		default:
			break;
	}
}

/**
 * Data for attribute enumerator
 */
typedef struct {
	enumerator_t public;
	bool request_ipv4;
	bool request_ipv6;
} attr_enumerator_t;

METHOD(enumerator_t, enumerate_attrs, bool,
	attr_enumerator_t *this, va_list args)
{
	configuration_attribute_type_t *type;
	chunk_t *data;

	VA_ARGS_VGET(args, type, data);
	if (this->request_ipv4)
	{
		*type = P_CSCF_IP4_ADDRESS;
		*data = chunk_empty;
		this->request_ipv4 = FALSE;
		return TRUE;
	}
	if (this->request_ipv6)
	{
		*type = P_CSCF_IP6_ADDRESS;
		*data = chunk_empty;
		this->request_ipv6 = FALSE;
		return TRUE;
	}
	return FALSE;
}

CALLBACK(is_family, bool,
	host_t *host, va_list args)
{
	int family;

	VA_ARGS_VGET(args, family);
	return host->get_family(host) == family;
}

/**
 * Check if a list has a host of a given family
 */
static bool has_host_family(linked_list_t *list, int family)
{
	return list->find_first(list, is_family, NULL, family);
}

METHOD(attribute_handler_t, create_attribute_enumerator, enumerator_t *,
	private_p_cscf_handler_t *this, ike_sa_t *ike_sa,
	linked_list_t *vips)
{
	attr_enumerator_t *enumerator;

	if (ike_sa->get_version(ike_sa) == IKEV1)
	{
		return enumerator_create_empty();
	}

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _enumerate_attrs,
			.destroy = (void*)free,
		},
	);
	if (lib->settings->get_bool(lib->settings, "%s.plugins.p-cscf.enable.%s",
								FALSE, lib->ns, ike_sa->get_name(ike_sa)))
	{
		enumerator->request_ipv4 = has_host_family(vips, AF_INET);
		enumerator->request_ipv6 = has_host_family(vips, AF_INET6);
	}
	return &enumerator->public;
}

METHOD(p_cscf_handler_t, destroy, void,
	private_p_cscf_handler_t *this)
{
	free(this);
}

/**
 * See header
 */
p_cscf_handler_t *p_cscf_handler_create()
{
	private_p_cscf_handler_t *this;

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
