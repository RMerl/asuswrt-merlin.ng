/*
 * Copyright (C) 2018 Tobias Brunner
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

#include "mock_net.h"

#include <daemon.h>

#include <assert.h>

typedef struct private_kernel_net_t private_kernel_net_t;

/**
 * Private data
 */
struct private_kernel_net_t {

	/**
	 * Public interface
	 */
	kernel_net_t public;

	/**
	 * Local IP address
	 */
	host_t *host;
};

/**
 * Global instance
 */
static private_kernel_net_t *instance;

METHOD(kernel_net_t, get_source_addr, host_t*,
	private_kernel_net_t *this, host_t *dest, host_t *src)
{
	return this->host->clone(this->host);
}

METHOD(kernel_net_t, get_nexthop, host_t*,
	private_kernel_net_t *this, host_t *dest, int prefix, host_t *src,
	char **iface)
{
	if (iface)
	{
		*iface = strdup("lo");
	}
	return this->host->clone(this->host);
}

METHOD(kernel_net_t, get_interface, bool,
	private_kernel_net_t *this, host_t *host, char **name)
{
	if (host->ip_equals(host, this->host))
	{
		if (name)
		{
			*name = strdup("lo");
		}
		return TRUE;
	}
	return FALSE;
}

METHOD(kernel_net_t, create_address_enumerator, enumerator_t*,
	private_kernel_net_t *this, kernel_address_type_t which)
{
	return enumerator_create_single(this->host, NULL);
}

METHOD(kernel_net_t, destroy, void,
	private_kernel_net_t *this)
{
	this->host->destroy(this->host);
	free(this);
}

/*
 * Described in header
 */
kernel_net_t *mock_net_create()
{
	private_kernel_net_t *this;

	INIT(this,
		.public = {
			.get_source_addr = _get_source_addr,
			.get_nexthop = _get_nexthop,
			.get_interface = _get_interface,
			.create_address_enumerator = _create_address_enumerator,
			.create_local_subnet_enumerator = (void*)enumerator_create_empty,
			.add_ip = (void*)return_failed,
			.del_ip = (void*)return_failed,
			.add_route = (void*)return_failed,
			.del_route = (void*)return_failed,
			.destroy = _destroy,
		},
		.host = host_create_from_string("127.0.0.1", 500),
	);

	instance = this;

	return &this->public;
}
