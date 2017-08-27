/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

/**
 * @defgroup dhcp_provider dhcp_provider
 * @{ @ingroup dhcp
 */

#ifndef DHCP_PROVIDER_H_
#define DHCP_PROVIDER_H_

typedef struct dhcp_provider_t dhcp_provider_t;

#include "dhcp_socket.h"

#include <attributes/attribute_provider.h>

/**
 * DHCP based attribute provider.
 */
struct dhcp_provider_t {

	/**
	 * Implements attribute_provier_t interface.
	 */
	attribute_provider_t provider;

	/**
	 * Destroy a dhcp_provider_t.
	 */
	void (*destroy)(dhcp_provider_t *this);
};

/**
 * Create a dhcp_provider instance.
 *
 * @param socket		socket to use for DHCP communication
 * @return				provider instance
 */
dhcp_provider_t *dhcp_provider_create(dhcp_socket_t *socket);

#endif /** DHCP_PROVIDER_H_ @}*/
