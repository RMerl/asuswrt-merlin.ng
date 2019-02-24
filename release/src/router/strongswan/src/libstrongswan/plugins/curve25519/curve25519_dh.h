/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
 * @defgroup curve25519_dh curve25519_dh
 * @{ @ingroup curve25519_p
 */

#ifndef CURVE25519_DH_H_
#define CURVE25519_DH_H_

typedef struct curve25519_dh_t curve25519_dh_t;

#include <library.h>

/**
 * Diffie-Hellman implementation using Curve25519.
 */
struct curve25519_dh_t {

	/**
	 * Implements diffie_hellman_t interface.
	 */
	diffie_hellman_t dh;
};

/**
 * Creates a new curve25519_dh_t object.
 *
 * @param group			DH group, CURVE_25519
 * @return				curve25519_dh_t object, NULL on error
 */
curve25519_dh_t *curve25519_dh_create(diffie_hellman_group_t group);

#endif /** CURVE25519_DH_H_ @}*/
