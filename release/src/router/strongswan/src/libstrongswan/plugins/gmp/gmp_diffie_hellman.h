/*
 * Copyright (C) 2005-2007 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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
 * @defgroup gmp_diffie_hellman gmp_diffie_hellman
 * @{ @ingroup gmp_p
 */

#ifndef GMP_DIFFIE_HELLMAN_H_
#define GMP_DIFFIE_HELLMAN_H_

typedef struct gmp_diffie_hellman_t gmp_diffie_hellman_t;

#include <library.h>

/**
 * Implementation of the Diffie-Hellman algorithm, as in RFC2631. Uses libgmp.
 */
struct gmp_diffie_hellman_t {

	/**
	 * Implements key_exchange_t interface.
	 */
	key_exchange_t ke;
};

/**
 * Creates a new gmp_diffie_hellman_t object.
 *
 * @param group			Diffie Hellman group number to use
 * @return				gmp_diffie_hellman_t object, NULL if not supported
 */
gmp_diffie_hellman_t *gmp_diffie_hellman_create(key_exchange_method_t group);

/**
 * Creates a new gmp_diffie_hellman_t object for MODP_CUSTOM.
 *
 * @param group			MODP_CUSTOM
 * @param ...			expects generator and prime as chunk_t
 * @return				gmp_diffie_hellman_t object, NULL if not supported
 */
gmp_diffie_hellman_t *gmp_diffie_hellman_create_custom(
							key_exchange_method_t group, ...);

#endif /** GMP_DIFFIE_HELLMAN_H_ @}*/

