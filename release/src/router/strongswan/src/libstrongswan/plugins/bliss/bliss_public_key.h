/*
 * Copyright (C) 2014 Andreas Steffen
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
 * @defgroup bliss_public_key bliss_public_key
 * @{ @ingroup bliss_p
 */

#ifndef BLISS_PUBLIC_KEY_H_
#define BLISS_PUBLIC_KEY_H_

#include "bliss_param_set.h"

#include <credentials/builder.h>
#include <credentials/cred_encoding.h>
#include <credentials/keys/public_key.h>

typedef struct bliss_public_key_t bliss_public_key_t;

/**
 * public_key_t implementation of BLISS signature algorithm
 */
struct bliss_public_key_t {

	/**
	 * Implements the public_key_t interface
	 */
	public_key_t key;
};

/**
 * Load a BLISS public key.
 *
 * Accepts BUILD_BLISS_* components.
 *
 * @param type		type of the key, must be KEY_BLISS
 * @param args		builder_part_t argument list
 * @return 			loaded key, NULL on failure
 */
bliss_public_key_t *bliss_public_key_load(key_type_t type, va_list args);

/* The following functions are shared with the bliss_private_key class */

/**
 * Parse an ASN.1 BIT STRING into an array of public key coefficients
 *
 * @param object	packed subjectPublicKey
 * @param set		BLISS parameter set for public key vector
 * @param pubkey	coefficients of public key vector
 * @return			TRUE if parsing successful
 */
bool bliss_public_key_from_asn1(chunk_t object, const bliss_param_set_t *set,
								uint32_t **pubkey);

/**
 * Encode a raw BLISS subjectPublicKey in ASN.1 DER format
 *
 * @param pubkey	coefficients of public key vector
 * @param set		BLISS parameter set for the public key vector
 * @result			ASN.1 encoded subjectPublicKey
 */
chunk_t bliss_public_key_encode(uint32_t *pubkey, const bliss_param_set_t *set);

/**
 * Encode a BLISS subjectPublicKeyInfo record in ASN.1 DER format
 *
 * @param oid		BLISS public key type OID
 * @param pubkey	coefficients of public key vector
 * @param set		BLISS parameter set for the public key vector
 * @result			ASN.1 encoded subjectPublicKeyInfo record
 */
chunk_t bliss_public_key_info_encode(int oid, uint32_t *pubkey,
									 const bliss_param_set_t *set);

/**
 * Generate a BLISS public key fingerprint
 *
 * @param oid		BLISS public key type OID
 * @param pubkey	coefficients of public key vector
 * @param set		BLISS parameter set for the public key vector
 * @param type		type of fingerprint to be generated
 * @param fp		generated fingerprint (must be freed by caller)
 * @result			TRUE if generation was successful
 */
bool bliss_public_key_fingerprint(int oid, uint32_t *pubkey,
								  const bliss_param_set_t *set,
								  cred_encoding_type_t type, chunk_t *fp);

#endif /** BLISS_PUBLIC_KEY_H_ @}*/
