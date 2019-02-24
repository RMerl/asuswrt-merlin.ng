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
 * @defgroup ntru_private_key ntru_private_key
 * @{ @ingroup ntru_p
 */

#ifndef NTRU_PRIVATE_KEY_H_
#define NTRU_PRIVATE_KEY_H_

typedef struct ntru_private_key_t ntru_private_key_t;

#include "ntru_drbg.h"
#include "ntru_param_set.h"
#include "ntru_public_key.h"

#include <library.h>

/**
 * Implements an NTRU encryption public/private key pair
 */
struct ntru_private_key_t {

	/**
	 * Returns NTRU parameter set ID of the private key
	 *
	 * @return			NTRU parameter set ID
	 */
	ntru_param_set_id_t (*get_id)(ntru_private_key_t *this);

	/**
	 * Returns the NTRU encryption public key as an encoded binary blob
	 *
	 * @return				NTRU encryption public key (must be freed after use)
	 */
	ntru_public_key_t* (*get_public_key)(ntru_private_key_t *this);

	/**
	 * Returns the packed encoding of the NTRU encryption private key
	 *
	 * @return				Packed encoding of NTRU encryption private key
	 */
	chunk_t (*get_encoding)(ntru_private_key_t *this);

	/**
	 * Decrypts an NTRU ciphertext
	 *
	 * @param ciphertext	NTRU Ciphertext
	 * @param plaintext		Plaintext
	 * @return				TRUE if decryption was successful
	 */
	bool (*decrypt)(ntru_private_key_t *this, chunk_t ciphertext,
					chunk_t *plaintext);

	/**
	 * Destroy ntru_private_key_t object
	 */
	void (*destroy)(ntru_private_key_t *this);
};

/**
 * Creates an NTRU encryption public/private key pair using a NIST DRBG
 *
 * @param drbg			Digital Random Bit Generator used for key generation
 * @param params		NTRU encryption parameter set to be used
 */
ntru_private_key_t *ntru_private_key_create(ntru_drbg_t *drbg,
											const ntru_param_set_t *params);

/**
 * Creates an NTRU encryption private key from encoding
 *
 * @param drbg			Deterministic random bit generator
 * @param data			Encoded NTRU private key
 */
ntru_private_key_t *ntru_private_key_create_from_data(ntru_drbg_t *drbg,
													  chunk_t data);

#endif /** NTRU_PRIVATE_KEY_H_ @}*/

