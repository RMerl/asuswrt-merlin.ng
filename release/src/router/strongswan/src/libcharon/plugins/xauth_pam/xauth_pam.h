/*
 * Copyright (C) 2012 Martin Willi
 * Copyright (C) 2012 revosec AG
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
 * @defgroup xauth_pam_i xauth_pam
 * @{ @ingroup xauth_pam
 */

#ifndef XAUTH_PAM_H_
#define XAUTH_PAM_H_

typedef struct xauth_pam_t xauth_pam_t;

#include <sa/xauth/xauth_method.h>

/**
 * XAuth plugin using Pluggable Authentication Modules to verify credentials.
 */
struct xauth_pam_t {

	/**
	 * Implemented xauth_method_t interface.
	 */
	xauth_method_t xauth_method;
};

/**
 * Creates the XAuth method using PAM, acting as server.
 *
 * @param server	ID of the XAuth server
 * @param peer		ID of the XAuth client
 * @param profile	configuration string
 * @return			xauth_pam_t object
 */
xauth_pam_t *xauth_pam_create_server(identification_t *server,
									 identification_t *peer, char *profile);

#endif /** XAUTH_PAM_H_ @}*/
