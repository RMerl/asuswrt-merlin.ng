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
 * @defgroup xauth_eap_i xauth_eap
 * @{ @ingroup xauth_eap
 */

#ifndef XAUTH_EAP_H_
#define XAUTH_EAP_H_

typedef struct xauth_eap_t xauth_eap_t;

#include <sa/xauth/xauth_method.h>

/**
 * XAuth method that verifies XAuth credentials using EAP methods.
 *
 * To reuse existing authentication infrastructure, this XAuth method uses
 * EAP to verify XAuth Username/Passwords. It is primarily designed to work
 * with the EAP-RADIUS backend and can use any password-based EAP method
 * over it. The credentials are fed locally on the IKE responder to a EAP
 * client which talks to the backend instance, usually a RADIUS server.
 */
struct xauth_eap_t {

	/**
	 * Implemented xauth_method_t interface.
	 */
	xauth_method_t xauth_method;
};

/**
 * Creates the XAuth method using EAP, acting as server.
 *
 * @param server	ID of the XAuth server
 * @param peer		ID of the XAuth client
 * @param profile	configuration string
 * @return			xauth_eap_t object
 */
xauth_eap_t *xauth_eap_create_server(identification_t *server,
									 identification_t *peer,
									 char *profile);

#endif /** XAUTH_EAP_H_ @}*/
