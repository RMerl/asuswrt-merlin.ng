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
 * @defgroup pkcs11_manager pkcs11_manager
 * @{ @ingroup pkcs11
 */

#ifndef PKCS11_MANAGER_H_
#define PKCS11_MANAGER_H_

typedef struct pkcs11_manager_t pkcs11_manager_t;

#include <library.h>

#include "pkcs11_library.h"

/**
 * Token event callback function.
 *
 * @param data		user supplied data, as passed to pkcs11_manager_create()
 * @param p11		loaded PKCS#11 library token belongs to
 * @param slot		slot number the event occurred in
 * @param add		TRUE if token was added to the slot, FALSE if removed
 */
typedef void (*pkcs11_manager_token_event_t)(void *data, pkcs11_library_t *p11,
											 CK_SLOT_ID slot, bool add);


/**
 * Manages multiple PKCS#11 libraries with hot pluggable slots
 */
struct pkcs11_manager_t {

	/**
	 * Create an enumerator over all tokens.
	 *
	 * @return			enumerator over (pkcs11_library_t*,CK_SLOT_ID)
	 */
	enumerator_t* (*create_token_enumerator)(pkcs11_manager_t *this);

	/**
	 * Destroy a pkcs11_manager_t.
	 */
	void (*destroy)(pkcs11_manager_t *this);
};

/**
 * Create a pkcs11_manager instance.
 *
 * @param cb		token event callback function
 * @param data		user data to pass to token event callback
 * @return			instance
 */
pkcs11_manager_t *pkcs11_manager_create(pkcs11_manager_token_event_t cb,
										void *data);

#endif /** PKCS11_MANAGER_H_ @}*/
