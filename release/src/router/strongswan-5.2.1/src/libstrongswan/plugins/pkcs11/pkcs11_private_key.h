/*
 * Copyright (C) 2011 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
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
 * @defgroup pkcs11_private_key pkcs11_private_key
 * @{ @ingroup pkcs11
 */

#ifndef PKCS11_PRIVATE_KEY_H_
#define PKCS11_PRIVATE_KEY_H_

typedef struct pkcs11_private_key_t pkcs11_private_key_t;

#include <credentials/builder.h>
#include <credentials/keys/private_key.h>

#include "pkcs11.h"

/**
 * Private Key implementation on top of PKCS#11.
 */
struct pkcs11_private_key_t {

	/**
	 * Implements private_key_t interface.
	 */
	private_key_t key;
};

/**
 * Open a private key on a PKCS#11 device.
 *
 * Accepts the BUILD_SMARTCARD_KEYID and the BUILD_SMARTCARD_PIN arguments.
 *
 * @param type		type of the key
 * @param args		builder_part_t argument list
 * @return			loaded key, NULL on failure
 */
pkcs11_private_key_t *pkcs11_private_key_connect(key_type_t type, va_list args);

/**
 * Get the Cryptoki mechanism for a signature scheme.
 *
 * Verifies that the given key is usable for this scheme.
 *
 * @param scheme		signature scheme
 * @param type			key type
 * @param keylen		key length in bits
 * @param hash			hash algorithm to apply first (HASH_UNKNOWN if none)
 */
CK_MECHANISM_PTR pkcs11_signature_scheme_to_mech(signature_scheme_t scheme,
												 key_type_t type, size_t keylen,
												 hash_algorithm_t *hash);

/**
 * Get the Cryptoki mechanism for a encryption scheme.
 */
CK_MECHANISM_PTR pkcs11_encryption_scheme_to_mech(encryption_scheme_t scheme);

#endif /** PKCS11_PRIVATE_KEY_H_ @}*/
