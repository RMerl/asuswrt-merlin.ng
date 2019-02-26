/*
 * Copyright (C) 2008 Tobias Brunner
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
 * @defgroup openssl_ec_public_key openssl_ec_public_key
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_EC_PUBLIC_KEY_H_
#define OPENSSL_EC_PUBLIC_KEY_H_

typedef struct openssl_ec_public_key_t openssl_ec_public_key_t;

#include <credentials/builder.h>
#include <credentials/keys/public_key.h>

/**
 * public_key_t implementation of ECDSA using OpenSSL.
 */
struct openssl_ec_public_key_t {

	/**
	 * Implements the public_key_t interface
	 */
	public_key_t key;
};

/**
 * Load a ECDSA public key using OpenSSL.
 *
 * Accepts a BUILD_BLOB_ASN1_DER argument.
 *
 * @param type		type of the key, must be KEY_ECDSA
 * @param args		builder_part_t argument list
 * @return 			loaded key, NULL on failure
 */
openssl_ec_public_key_t *openssl_ec_public_key_load(key_type_t type,
													va_list args);

#endif /** OPENSSL_EC_PUBLIC_KEY_H_ @}*/
