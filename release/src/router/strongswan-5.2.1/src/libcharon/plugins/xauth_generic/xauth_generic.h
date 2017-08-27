/*
 * Copyright (C) 2011 Tobias Brunner
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

/**
 * @defgroup xauth_generic_i xauth_generic
 * @{ @ingroup xauth_generic
 */

#ifndef XAUTH_GENERIC_H_
#define XAUTH_GENERIC_H_

typedef struct xauth_generic_t xauth_generic_t;

#include <sa/xauth/xauth_method.h>

/**
 * Implementation of the xauth_method_t interface using cleartext secrets
 * from any credential set.
 */
struct xauth_generic_t {

	/**
	 * Implemented xauth_method_t interface.
	 */
	xauth_method_t xauth_method;
};

/**
 * Creates the generic XAuth method, acting as server.
 *
 * @param server	ID of the XAuth server
 * @param peer		ID of the XAuth client
 * @param profile	configuration string
 * @return			xauth_generic_t object
 */
xauth_generic_t *xauth_generic_create_server(identification_t *server,
											 identification_t *peer,
											 char *profile);

/**
 * Creates the generic XAuth method, acting as peer.
 *
 * @param server	ID of the XAuth server
 * @param peer		ID of the XAuth client
 * @param profile	configuration string
 * @return			xauth_generic_t object
 */
xauth_generic_t *xauth_generic_create_peer(identification_t *server,
										   identification_t *peer,
										   char *profile);

#endif /** XAUTH_GENERIC_H_ @}*/
