/*
 * Copyright (C) 2018 Tobias Brunner
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
 * Implementation of the X25519/X448 Diffie-Hellman algorithm using OpenSSL.
 *
 * @defgroup openssl_x_diffie_hellman openssl_x_diffie_hellman
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_X_DIFFIE_HELLMAN_H_
#define OPENSSL_X_DIFFIE_HELLMAN_H_

#include <library.h>

/**
 * Creates a new key_exchange_t object.
 *
 * @param ke			key exchange method to use
 * @return				object, NULL if not supported
 */
key_exchange_t *openssl_x_diffie_hellman_create(key_exchange_method_t ke);

#endif /** OPENSSL_X_DIFFIE_HELLMAN_H_ @}*/

