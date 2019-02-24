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
 * @defgroup tkm-keymat keymat
 * @{ @ingroup tkm
 */

#ifndef TKM_KEYMAT_H_
#define TKM_KEYMAT_H_

#include <sa/ikev2/keymat_v2.h>

typedef struct tkm_keymat_t tkm_keymat_t;

/**
 * Derivation and management of sensitive keying material, TKM variant.
 */
struct tkm_keymat_t {

	/**
	 * Implements keymat_v2_t.
	 */
	keymat_v2_t keymat_v2;

	/**
	 * Get ISA context id.
	 *
	 * @return	id of associated ISA context.
	 */
	isa_id_type (*get_isa_id)(tkm_keymat_t * const this);

	/**
	 * Set IKE AUTH payload.
	 *
	 * @param payload		AUTH payload
	 */
	void (*set_auth_payload)(tkm_keymat_t *this, const chunk_t * const payload);

	/**
	 * Get IKE AUTH payload.
	 *
	 * @return				AUTH payload if set, chunk_empty otherwise
	 */
	chunk_t* (*get_auth_payload)(tkm_keymat_t * const this);

	/**
	 * Get IKE init message of peer.
	 *
	 * @return				init message if set, chunk_empty otherwise
	 */
	chunk_t* (*get_peer_init_msg)(tkm_keymat_t * const this);

};

/**
 * Create TKM keymat instance.
 *
 * @param initiator			TRUE if we are the initiator
 * @return					keymat instance
 */
tkm_keymat_t *tkm_keymat_create(bool initiator);

#endif /** KEYMAT_TKM_H_ @}*/
