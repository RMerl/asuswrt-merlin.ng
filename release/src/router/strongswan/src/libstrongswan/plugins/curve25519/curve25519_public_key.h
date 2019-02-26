/*
 * Copyright (C) 2016 Andreas Steffen
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
 * @defgroup curve25519_public_key curve25519_public_key
 * @{ @ingroup curve25519_p
 */

#ifndef CURVE25519_PUBLIC_KEY_H_
#define CURVE25519_PUBLIC_KEY_H_

#include <credentials/builder.h>
#include <credentials/cred_encoding.h>
#include <credentials/keys/public_key.h>

typedef struct curve25519_public_key_t curve25519_public_key_t;

#define ED25519_KEY_LEN		32

/**
 * public_key_t implementation of Ed25519 signature algorithm
 */
struct curve25519_public_key_t {

	/**
	 * Implements the public_key_t interface
	 */
	public_key_t key;
};

/**
 * Load an Ed25519 public key.
 *
 * @param type		type of the key, must be KEY_ED25519
 * @param args		builder_part_t argument list
 * @return 			loaded key, NULL on failure
 */
curve25519_public_key_t *curve25519_public_key_load(key_type_t type,
													va_list args);

/* The following functions are shared with the curve25519_private_key class */

/**
 * Encode a Ed25519 subjectPublicKeyInfo record in ASN.1 DER format
 *
 * @param pubkey	Ed25519 public key
 * @result			ASN.1 encoded subjectPublicKeyInfo record
 */
chunk_t curve25519_public_key_info_encode(chunk_t pubkey);

/**
 * Generate a Ed25519 public key fingerprint
 *
 * @param pubkey	Ed25519 public key
 * @param type		type of fingerprint to be generated
 * @param fp		generated fingerprint (must be freed by caller)
 * @result			TRUE if generation was successful
 */
bool curve25519_public_key_fingerprint(chunk_t pubkey,
									   cred_encoding_type_t type, chunk_t *fp);

#endif /** CURVE25519_PUBLIC_KEY_H_ @}*/
