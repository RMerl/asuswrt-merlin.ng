/*
 * Copyright (C) 2013 Martin Willi
 * Copyright (C) 2013 revosec AG
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
 * @defgroup eap_radius_xauth eap_radius_xauth
 * @{ @ingroup eap_radius
 */

#ifndef EAP_RADIUS_XAUTH_H_
#define EAP_RADIUS_XAUTH_H_

#include <sa/xauth/xauth_method.h>

typedef struct eap_radius_xauth_t eap_radius_xauth_t;

/**
 * XAuth backend using plain RADIUS authentication (no EAP involved).
 */
struct eap_radius_xauth_t {

	/**
	 * Implements XAuth module interface
	 */
	xauth_method_t xauth_method;
};

/**
 * Creates the RADIUS XAuth method, acting as server.
 *
 * @param server	ID of the XAuth server
 * @param peer		ID of the XAuth client
 * @param profile	configuration string
 * @return			xauth_generic_t object
 */
eap_radius_xauth_t *eap_radius_xauth_create_server(identification_t *server,
												   identification_t *peer,
												   char *profile);

#endif /** EAP_RADIUS_XAUTH_H_ @}*/
