/*
 * Copyright (C) 2012-2017 Tobias Brunner
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
 * @defgroup android_creds android_creds
 * @{ @ingroup android_backend
 */

#ifndef ANDROID_CREDS_H_
#define ANDROID_CREDS_H_

#include <library.h>
#include <credentials/credential_set.h>

typedef struct android_creds_t android_creds_t;

/**
 * Android credential set that provides CA certificates via JNI and supplied
 * user credentials.
 */
struct android_creds_t {

	/**
	 * Implements credential_set_t
	 */
	credential_set_t set;

	/**
	 * Add user name and password for EAP authentication
	 *
	 * @param username			user name
	 * @param password			password
	 */
	void (*add_username_password)(android_creds_t *this, char *username,
								  char *password);

	/**
	 * Load the user certificate and private key
	 *
	 * @return					loaded client certificate, NULL on failure
	 */
	certificate_t *(*load_user_certificate)(android_creds_t *this);

	/**
	 * Clear the cached certificates and stored credentials.
	 */
	void (*clear)(android_creds_t *this);

	/**
	 * Destroy a android_creds instance.
	 */
	void (*destroy)(android_creds_t *this);

};

/**
 * Create an android_creds instance.
 *
 * @param crldir				directory for cached CRLs
 */
android_creds_t *android_creds_create(char *crldir);

#endif /** ANDROID_CREDS_H_ @}*/

