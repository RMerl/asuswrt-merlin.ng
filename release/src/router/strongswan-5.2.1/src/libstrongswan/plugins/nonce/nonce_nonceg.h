/*
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
 * Hochschule fuer Technik Rapperswil
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
 * @defgroup nonce_nonceg nonce_nonceg
 * @{ @ingroup nonce_p
 */

#ifndef NONCE_NONCEG_H_
#define NONCE_NONCEG_H_

typedef struct nonce_nonceg_t nonce_nonceg_t;

#include <library.h>

/**
 * nonce_gen_t implementation using an rng plugin
 */
struct nonce_nonceg_t {

	/**
	 * Implements nonce_gen_t.
	 */
	nonce_gen_t nonce_gen;
};

/**
 * Creates an nonce_nonceg_t instance.
 *
 * @return			created nonce_nonceg_t
 */
nonce_nonceg_t *nonce_nonceg_create();

#endif /** NONCE_NONCEG_H_ @} */
