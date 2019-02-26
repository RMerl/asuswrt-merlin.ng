/*
 * Copyright (C) 2018 Tobias Brunner
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
 * Implementation of the X25519/X448 Diffie-Hellman algorithm using OpenSSL.
 *
 * @defgroup openssl_x_diffie_hellman openssl_x_diffie_hellman
 * @{ @ingroup openssl_p
 */

#ifndef OPENSSL_X_DIFFIE_HELLMAN_H_
#define OPENSSL_X_DIFFIE_HELLMAN_H_

#include <library.h>

/**
 * Creates a new diffie_hellman_t object.
 *
 * @param group			Diffie Hellman group number to use
 * @return				object, NULL if not supported
 */
diffie_hellman_t *openssl_x_diffie_hellman_create(diffie_hellman_group_t group);

#endif /** OPENSSL_X_DIFFIE_HELLMAN_H_ @}*/

