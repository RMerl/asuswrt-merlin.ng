/*
 * Copyright (C) 2009 Martin Willi
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup gcrypt_dh gcrypt_dh
 * @{ @ingroup gcrypt_p
 */

#ifndef GCRYPT_DH_H_
#define GCRYPT_DH_H_

typedef struct gcrypt_dh_t gcrypt_dh_t;

#include <library.h>

/**
 * Implementation of the Diffie-Hellman algorithm using libgcrypt mpi.
 */
struct gcrypt_dh_t {

	/**
	 * Implements key_exchange_t interface.
	 */
	key_exchange_t ke;
};

/**
 * Creates a new gcrypt_dh_t object.
 *
 * @param group			Diffie Hellman group number to use
 * @return				gcrypt_dh_t object, NULL if not supported
 */
gcrypt_dh_t *gcrypt_dh_create(key_exchange_method_t group);

/**
 * Creates a new gcrypt_dh_t object for MODP_CUSTOM.
 *
 * @param group			MODP_CUSTOM
 * @param ...			expects generator and prime as chunk_t
 * @return				gcrypt_dh_t object, NULL if not supported
 */
gcrypt_dh_t *gcrypt_dh_create_custom(key_exchange_method_t group, ...);

#endif /** GCRYPT_DH_H_ @}*/

