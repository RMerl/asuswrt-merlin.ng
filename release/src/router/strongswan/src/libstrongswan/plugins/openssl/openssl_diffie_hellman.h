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
 * @defgroup openssl_diffie_hellman openssl_diffie_hellman
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_DIFFIE_HELLMAN_H_
#define OPENSSL_DIFFIE_HELLMAN_H_

typedef struct openssl_diffie_hellman_t openssl_diffie_hellman_t;

#include <library.h>

/**
 * Implementation of the Diffie-Hellman algorithm using OpenSSL.
 */
struct openssl_diffie_hellman_t {

	/**
	 * Implements diffie_hellman_t interface.
	 */
	diffie_hellman_t dh;
};

/**
 * Creates a new openssl_diffie_hellman_t object.
 *
 * @param group			Diffie Hellman group number to use
 * @param ...			expects generator and prime as chunk_t if MODP_CUSTOM
 * @return				openssl_diffie_hellman_t object, NULL if not supported
 */
openssl_diffie_hellman_t *openssl_diffie_hellman_create(
											diffie_hellman_group_t group, ...);

#endif /** OPENSSL_DIFFIE_HELLMAN_H_ @}*/

