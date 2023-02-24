/*
 * Copyright (C) 2008 Tobias Brunner
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
 * @defgroup openssl_ec_diffie_hellman openssl_ec_diffie_hellman
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_EC_DIFFIE_HELLMAN_H_
#define OPENSSL_EC_DIFFIE_HELLMAN_H_

typedef struct openssl_ec_diffie_hellman_t openssl_ec_diffie_hellman_t;

#include <library.h>

/**
 * Implementation of the EC Diffie-Hellman algorithm using OpenSSL.
 */
struct openssl_ec_diffie_hellman_t {

	/**
	 * Implements key_exchange_t interface.
	 */
	key_exchange_t ke;
};

/**
 * Creates a new openssl_ec_diffie_hellman_t object.
 *
 * @param group			EC Diffie-Hellman group number to use
 * @return				openssl_ec_diffie_hellman_t object, NULL if unsupported
 */
openssl_ec_diffie_hellman_t *openssl_ec_diffie_hellman_create(
												key_exchange_method_t group);

/**
 * Map ECDH groups to OpenSSL NIDs for the ECC curve.
 *
 * @param group         ECDH group
 * @return              NID for the curve
 */
int openssl_ecdh_group_to_nid(key_exchange_method_t group);

#endif /** OPENSSL_EC_DIFFIE_HELLMAN_H_ @}*/
