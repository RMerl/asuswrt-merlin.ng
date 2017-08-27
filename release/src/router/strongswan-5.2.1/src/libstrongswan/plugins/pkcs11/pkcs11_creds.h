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
 * @defgroup pkcs11_creds pkcs11_creds
 * @{ @ingroup pkcs11
 */

#ifndef PKCS11_CREDS_H_
#define PKCS11_CREDS_H_

typedef struct pkcs11_creds_t pkcs11_creds_t;

#include "pkcs11_library.h"

#include <credentials/credential_manager.h>

/**
 * Credential set on top on a PKCS#11 token.
 */
struct pkcs11_creds_t {

	/**
	 * Implements credential_set_t.
	 */
	credential_set_t set;

	/**
	 * Get the PKCS#11 library this set uses.
	 *
	 * @return		library
	 */
	pkcs11_library_t* (*get_library)(pkcs11_creds_t *this);

	/**
	 * Get the slot of the token this set uses.
	 *
	 * @return		slot
	 */
	CK_SLOT_ID (*get_slot)(pkcs11_creds_t *this);

	/**
	 * Destroy a pkcs11_creds_t.
	 */
	void (*destroy)(pkcs11_creds_t *this);
};

/**
 * Create a pkcs11_creds instance.
 *
 * @param p11			loaded PKCS#11 library
 * @param slot			slot of the token we hand out credentials
 */
pkcs11_creds_t *pkcs11_creds_create(pkcs11_library_t *p11, CK_SLOT_ID slot);

/**
 * Load a specific certificate from a token.
 *
 * Requires a BUILD_PKCS11_KEYID argument, and optionally BUILD_PKCS11_MODULE
 * and/or BUILD_PKCS11_SLOT.
 *
 * @param type			certificate type, must be CERT_X509
 * @param args			variable argument list, containing BUILD_PKCS11_KEYID.
 * @return				loaded certificate, or NULL on failure
 */
certificate_t *pkcs11_creds_load(certificate_type_t type, va_list args);

#endif /** PKCS11_CREDS_H_ @}*/
