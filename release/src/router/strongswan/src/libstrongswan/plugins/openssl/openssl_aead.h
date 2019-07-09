/*
 * Copyright (C) 2013-2019 Tobias Brunner
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
 * Implements the aead_t interface using OpenSSL.
 *
 * @defgroup openssl_aead openssl_aead
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_AEAD_H_
#define OPENSSL_AEAD_H_

#include <crypto/aead.h>

/**
 * Constructor to create aead_t implementation.
 *
 * @param algo			algorithm to implement
 * @param key_size		key size in bytes
 * @param salt_size		size of implicit salt length
 * @return				aead_t object, NULL if not supported
 */
aead_t *openssl_aead_create(encryption_algorithm_t algo, size_t key_size,
							size_t salt_size);

#endif /** OPENSSL_AEAD_H_ @}*/
