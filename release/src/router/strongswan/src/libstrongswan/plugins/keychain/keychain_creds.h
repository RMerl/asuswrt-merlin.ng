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
 * @defgroup keychain_creds keychain_creds
 * @{ @ingroup keychain
 */

#ifndef KEYCHAIN_CREDS_H_
#define KEYCHAIN_CREDS_H_

typedef struct keychain_creds_t keychain_creds_t;

#include <credentials/credential_manager.h>

/**
 * Credential set using OS X Keychain Services.
 */
struct keychain_creds_t {

	/**
	 * Destroy a keychain_creds_t.
	 */
	void (*destroy)(keychain_creds_t *this);
};

/**
 * Create a keychain_creds instance.
 */
keychain_creds_t *keychain_creds_create();

#endif /** KEYCHAIN_CREDS_H_ @}*/
