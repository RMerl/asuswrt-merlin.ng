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
 * @defgroup rc2_crypter rc2_crypter
 * @{ @ingroup rc2_p
 */

#ifndef RC2_CRYPTER_H_
#define RC2_CRYPTER_H_

typedef struct rc2_crypter_t rc2_crypter_t;

#include <crypto/crypters/crypter.h>

/**
 * Class implementing the RC2 block cipher as defined in RFC 2268.
 */
struct rc2_crypter_t {

	/**
	 * Implements crypter_t interface.
	 */
	crypter_t crypter;
};

/**
 * Constructor to create rc2_crypter_t objects.
 *
 * @param algo			algorithm to implement (ENCR_RC2_CBC)
 * @param key_size		use the RC2_KEY_SIZE macro if the effective key size
 * 						in bits is different than the actual length of the key
 * @return				rc2_crypter_t object, NULL if not supported
 */
rc2_crypter_t *rc2_crypter_create(encryption_algorithm_t algo,
								  size_t key_size);

#endif /** RC2_CRYPTER_H_ @}*/
