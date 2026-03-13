/*
 * Copyright (C) 2012 Reto Buerki
 * Copyright (C) 2012 Adrian-Ken Rueegsegger
 *
 * Copyright (C) secunet Security Networks AG
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
 * @defgroup tkm-ke key exchange
 * @{ @ingroup tkm
 */

#ifndef TKM_KEY_EXCHANGE_H_
#define TKM_KEY_EXCHANGE_H_

typedef struct tkm_key_exchange_t tkm_key_exchange_t;

#include <library.h>
#include <tkm/types.h>

/**
 * key_exchange_t implementation using the trusted key manager.
 */
struct tkm_key_exchange_t {

	/**
	 * Implements key_exchange_t interface.
	 */
	key_exchange_t ke;

	/**
	 * Get Key Exchange context id.
	 *
	 * @return	id of this KE context.
	 */
	ke_id_type (*get_id)(tkm_key_exchange_t * const this);

};

/**
 * Loads IANA KE method identifier to TKM id mapping from config and registers
 * the corresponding KE plugin features.
 *
 * @return          number of registered mappings
 */
int register_ke_mapping();

/**
 * Destroy IANA KE method identifier to TKM id mapping.
 */
void destroy_ke_mapping();

/**
 * Creates a new tkm_key_exchange_t object.
 *
 * @param method		Key exchange method to use
 * @return				tkm_key_exchange_t object, NULL if not supported
 */
tkm_key_exchange_t *tkm_key_exchange_create(key_exchange_method_t method);

#endif /** TKM_KEY_EXCHANGE_H_ @}*/
