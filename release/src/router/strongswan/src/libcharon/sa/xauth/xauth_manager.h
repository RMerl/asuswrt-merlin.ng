/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
 * @defgroup xauth_manager xauth_manager
 * @{ @ingroup xauth
 */

#ifndef XAUTH_MANAGER_H_
#define XAUTH_MANAGER_H_

#include <sa/xauth/xauth_method.h>

typedef struct xauth_manager_t xauth_manager_t;

/**
 * The XAuth manager manages all XAuth implementations and creates instances.
 *
 * A plugin registers it's implemented XAuth method at the manager by
 * providing type and a constructor function. The manager then instantiates
 * xauth_method_t instances through the provided constructor to handle
 * XAuth authentication.
 */
struct xauth_manager_t {

	/**
	 * Register a XAuth method implementation.
	 *
	 * @param name			backend name to register
	 * @param role			XAUTH_SERVER or XAUTH_PEER
	 * @param constructor	constructor function, returns an xauth_method_t
	 */
	void (*add_method)(xauth_manager_t *this, char *name,
					   xauth_role_t role, xauth_constructor_t constructor);

	/**
	 * Unregister a XAuth method implementation using it's constructor.
	 *
	 * @param constructor	constructor function, as added in add_method
	 */
	void (*remove_method)(xauth_manager_t *this, xauth_constructor_t constructor);

	/**
	 * Create a new XAuth method instance.
	 *
	 * The name may contain an option string, separated by a colon. This option
	 * string gets passed to the XAuth constructor to specify the behavior
	 * of the XAuth method.
	 *
	 * @param name			backend name, with optional config string
	 * @param role			XAUTH_SERVER or XAUTH_PEER
	 * @param server		identity of the server
	 * @param peer			identity of the peer (client)
	 * @return				XAUTH method instance, NULL if no constructor found
	 */
	xauth_method_t* (*create_instance)(xauth_manager_t *this,
							char *name, xauth_role_t role,
							identification_t *server, identification_t *peer);

	/**
	 * Destroy a eap_manager instance.
	 */
	void (*destroy)(xauth_manager_t *this);
};

/**
 * Create a eap_manager instance.
 */
xauth_manager_t *xauth_manager_create();

#endif /** XAUTH_MANAGER_H_ @}*/
