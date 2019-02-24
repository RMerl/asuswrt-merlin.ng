/*
 * Copyright (C) 2013 Tobias Brunner
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
 * @defgroup pkcs5 pkcs5
 * @{ @ingroup crypto
 */

#ifndef PKCS5_H_
#define PKCS5_H_

typedef struct pkcs5_t pkcs5_t;

#include <utils/chunk.h>

/**
 * PKCS#5 helper class
 */
struct pkcs5_t {

	/**
	 * Decrypt the given data using the given password and the scheme derived
	 * from the initial AlgorithmIdentifier object.
	 *
	 * @param password	password used for decryption
	 * @param data		data to decrypt
	 * @param decrypted	decrypted data gets allocated
	 * @return			TRUE on success, FALSE otherwise
	 */
	bool (*decrypt)(pkcs5_t *this, chunk_t password, chunk_t data,
					chunk_t *decrypted) __attribute__((warn_unused_result));

	/**
	 * Destroy the object and any associated cryptographic primitive.
	 */
	void (*destroy)(pkcs5_t *this);
};

/**
 * Create a PKCS#5 helper object from an ASN.1 encoded AlgorithmIdentifier
 * object.
 *
 * @param blob			ASN.1 encoded AlgorithmIdentifier
 * @param level0		ASN.1 parser level
 * @return				pkcs5_t object, NULL on failure
 */
pkcs5_t *pkcs5_from_algorithmIdentifier(chunk_t blob, int level0);

#endif /** PKCS5_H_ @}*/
