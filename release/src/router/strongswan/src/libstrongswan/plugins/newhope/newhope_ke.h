/*
 * Copyright (C) 2016 Andreas Steffen
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
 * @defgroup newhope_ke newhope_ke
 * @{ @ingroup newhope_p
 */

#ifndef NEWHOPE_KE_H_
#define NEWHOPE_KE_H_

typedef struct newhope_ke_t newhope_ke_t;

#include <library.h>

/**
 * Implementation of a key exchange algorithm using the New Hope algorithm
 */
struct newhope_ke_t {

	/**
	 * Implements diffie_hellman_t interface.
	 */
	diffie_hellman_t dh;
};

/**
 * Creates a new newhope_ke_t object.
 *
 * @param group			New Hope DH group number
 * @param g				not used
 * @param p				not used
 * @return				newhope_ke_t object, NULL if not supported
 */
newhope_ke_t *newhope_ke_create(diffie_hellman_group_t group, chunk_t g, chunk_t p);

#endif /** NEWHOPE_KE_H_ @}*/

