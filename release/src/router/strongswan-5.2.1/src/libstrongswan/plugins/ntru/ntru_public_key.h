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
 * @defgroup ntru_public_key ntru_public_key
 * @{ @ingroup ntru_p
 */

#ifndef NTRU_PUBLIC_KEY_H_
#define NTRU_PUBLIC_KEY_H_

typedef struct ntru_public_key_t ntru_public_key_t;

#include "ntru_param_set.h"
#include "ntru_drbg.h"

#include <library.h>

/**
 * Implements an NTRU encryption public key
 */
struct ntru_public_key_t {

	/**
	 * Returns NTRU parameter set ID of the public key
	 *
	 * @return			NTRU parameter set ID
	 */
	ntru_param_set_id_t (*get_id)(ntru_public_key_t *this);

	/**
	 * Returns the packed encoding of the NTRU encryption public key
	 *
	 * @return			Packed encoding of NTRU encryption public key
	 */
	chunk_t (*get_encoding)(ntru_public_key_t *this);

	/**
	 * Encrypts a plaintext with the NTRU public key
	 *
	 * @param ciphertext	Plaintext
	 * @param plaintext		Ciphertext
	 * @return				TRUE if encryption was successful
	 */
	bool (*encrypt)(ntru_public_key_t *this, chunk_t plaintext,
					chunk_t *ciphertext);

	/**
	 * Destroy ntru_public_key_t object
	 */
	void (*destroy)(ntru_public_key_t *this);
};

/**
 * Creates an NTRU encryption public key from coefficients
 *
 * @param drbg			Deterministic random bit generator
 * @param params		NTRU encryption parameter set to be used
 * @param pubkey		Coefficients of public key polynomial h
 */
ntru_public_key_t *ntru_public_key_create(ntru_drbg_t *drbg,
										  ntru_param_set_t *params,
										  uint16_t *pubkey);

/**
 * Creates an NTRU encryption public key from encoding
 *
 * @param drbg			Deterministic random bit generator
 * @param data			Encoded NTRU public key
 */
ntru_public_key_t *ntru_public_key_create_from_data(ntru_drbg_t *drbg,
													chunk_t data);


#endif /** NTRU_PUBLIC_KEY_H_ @}*/

