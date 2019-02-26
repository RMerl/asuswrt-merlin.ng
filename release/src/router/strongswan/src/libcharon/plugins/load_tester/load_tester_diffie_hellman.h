/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup load_tester_diffie_hellman load_tester_diffie_hellman
 * @{ @ingroup load_tester
 */

#ifndef LOAD_TESTER_DIFFIE_HELLMAN_H_
#define LOAD_TESTER_DIFFIE_HELLMAN_H_

#include <crypto/diffie_hellman.h>

typedef struct load_tester_diffie_hellman_t load_tester_diffie_hellman_t;

/**
 * A NULL Diffie Hellman implementation to avoid calculation overhead in tests.
 */
struct load_tester_diffie_hellman_t {

	/**
	 * Implements diffie_hellman_t interface.
	 */
	diffie_hellman_t dh;
};

/**
 * Creates a new gmp_diffie_hellman_t object.
 *
 * @param group			Diffie Hellman group, supports MODP_NULL only
 * @return				gmp_diffie_hellman_t object
 */
load_tester_diffie_hellman_t *load_tester_diffie_hellman_create(
												diffie_hellman_group_t group);

#endif /** LOAD_TESTER_DIFFIE_HELLMAN_H_ @}*/
