/*
 * Copyright (C) 2013 Tobias Brunner
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
 * @defgroup xauth_noauth_i xauth_noauth
 * @{ @ingroup xauth_noauth
 */

#ifndef XAUTH_NOAUTH_H_
#define XAUTH_NOAUTH_H_

typedef struct xauth_noauth_t xauth_noauth_t;

#include <sa/xauth/xauth_method.h>

/**
 * Implementation of the xauth_method_t interface that does not actually do
 * any authentication but simply concludes the XAuth exchange successfully.
 */
struct xauth_noauth_t {

	/**
	 * Implemented xauth_method_t interface.
	 */
	xauth_method_t xauth_method;
};

/**
 * Creates the noauth XAuth method, acting as server.
 *
 * @param server	ID of the XAuth server
 * @param peer		ID of the XAuth client
 * @param profile	configuration string
 * @return			xauth_noauth_t object
 */
xauth_noauth_t *xauth_noauth_create_server(identification_t *server,
										   identification_t *peer,
										   char *profile);

#endif /** XAUTH_NOAUTH_H_ @}*/
