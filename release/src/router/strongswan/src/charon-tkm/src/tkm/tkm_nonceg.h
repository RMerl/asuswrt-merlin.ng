/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
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
 * @defgroup tkm-nonceg nonce generator
 * @{ @ingroup tkm
 */

#ifndef TKM_NONCEG_H_
#define TKM_NONCEG_H_

typedef struct tkm_nonceg_t tkm_nonceg_t;

#include <library.h>
#include <tkm/types.h>

/**
 * nonce_gen_t implementation using the trusted key manager.
 */
struct tkm_nonceg_t {

	/**
	 * Implements nonce_gen_t.
	 */
	nonce_gen_t nonce_gen;
};

/**
 * Creates a tkm_nonceg_t instance.
 *
 * @return			created tkm_nonceg_t
 */
tkm_nonceg_t *tkm_nonceg_create();

#endif /** TKM_NONCEG_H_ @}*/
