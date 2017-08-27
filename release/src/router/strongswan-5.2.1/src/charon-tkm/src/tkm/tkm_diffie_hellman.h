/*
 * Copyright (C) 2012 Reto Buerki
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
 * @defgroup tkm-dh diffie hellman
 * @{ @ingroup tkm
 */

#ifndef TKM_DIFFIE_HELLMAN_H_
#define TKM_DIFFIE_HELLMAN_H_

typedef struct tkm_diffie_hellman_t tkm_diffie_hellman_t;

#include <library.h>
#include <tkm/types.h>

/**
 * diffie_hellman_t implementation using the trusted key manager.
 */
struct tkm_diffie_hellman_t {

	/**
	 * Implements diffie_hellman_t interface.
	 */
	diffie_hellman_t dh;

	/**
	 * Get Diffie-Hellman context id.
	 *
	 * @return	id of this DH context.
	 */
	dh_id_type (*get_id)(tkm_diffie_hellman_t * const this);

};

/**
 * Loads IANA DH group identifier to TKM id mapping from config and registers
 * the corresponding DH features.
 *
 * @return          number of registered mappings
 */
int register_dh_mapping();

/**
 * Destroy IANA DH group identifier to TKM id mapping.
 */
void destroy_dh_mapping();

/**
 * Creates a new tkm_diffie_hellman_t object.
 *
 * @param group			Diffie Hellman group number to use
 * @return				tkm_diffie_hellman_t object, NULL if not supported
 */
tkm_diffie_hellman_t *tkm_diffie_hellman_create(diffie_hellman_group_t group);

#endif /** TKM_DIFFIE_HELLMAN_H_ @}*/
