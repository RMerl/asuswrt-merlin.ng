/*
 * Copyright (C) 2007 Martin Willi
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

/**
 * @defgroup manager manager
 *
 * @defgroup manager_controller controller
 * @ingroup manager
 *
 * @defgroup manager_i manager
 * @{ @ingroup manager
 */

#ifndef MANAGER_H_
#define MANAGER_H_

#include "storage.h"
#include "gateway.h"

#include <fast_context.h>

typedef struct manager_t manager_t;

/**
 * The manager, manages multiple gateways.
 */
struct manager_t {

	/**
	 * implements context_t interface
	 */
	fast_context_t context;

	/**
	 * Create an enumerator over all configured gateways.
	 *
	 * enumerate() arguments: int id, char *name, int port, char *address
	 * If port is 0, address is a Unix socket address.
	 *
	 * @return			enumerator
	 */
	enumerator_t* (*create_gateway_enumerator)(manager_t *this);

	/**
	 * Select a gateway.
	 *
	 * If id is 0, the previously selected gateway is returned. If none has
	 * been selected yet, NULL is returned.
	 *
	 * @param id		id of the gateway (from enumerate), or 0
	 * @return			selected gateway, or NULL
	 */
	gateway_t* (*select_gateway)(manager_t *this, int id);

	/**
	 * Try to log in.
	 *
	 * @param username	username
	 * @param password	cleartext password
	 * @return			TRUE if login successful
	 */
	bool (*login)(manager_t *this, char *username, char *password);

	/**
	 * Check if user logged in.
	 *
	 * @return			TRUE if logged in
	 */
	bool (*logged_in)(manager_t *this);

	/**
	 * Log out.
	 */
	void (*logout)(manager_t *this);
};

/**
 * Create a manager instance.
 */
manager_t *manager_create(storage_t *storage);

#endif /** MANAGER_H_ @}*/
