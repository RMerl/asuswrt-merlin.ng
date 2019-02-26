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
 * @defgroup tkm-credential credential set
 * @{ @ingroup tkm
 */

#ifndef TKM_CRED_H_
#define TKM_CRED_H_

typedef struct tkm_cred_t tkm_cred_t;

#include <credentials/credential_set.h>

/**
 * TKM in-memory credential set.
 */
struct tkm_cred_t {

	/**
	 * Implements credential_set_t.
	 */
	credential_set_t set;

	/**
	 * Destroy a tkm_cred_t.
	 */
	void (*destroy)(tkm_cred_t *this);

};

/**
 * Create a tkm_cred instance.
 */
tkm_cred_t *tkm_cred_create();

#endif /** TKM_CRED_H_ @}*/
