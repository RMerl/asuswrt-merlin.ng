/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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
 * @defgroup ke_payload ke_payload
 * @{ @ingroup payloads
 */

#ifndef KE_PAYLOAD_H_
#define KE_PAYLOAD_H_

typedef struct ke_payload_t ke_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>
#include <encoding/payloads/transform_substructure.h>
#include <collections/linked_list.h>
#include <crypto/key_exchange.h>

/**
 * Class representing an IKEv1 or IKEv2 key exchange payload.
 */
struct ke_payload_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Returns the key exchange data of this KE payload.
	 *
	 * @return 		chunk_t pointing to internal data
	 */
	chunk_t (*get_key_exchange_data) (ke_payload_t *this);

	/**
	 * Gets the key exchange method of this KE payload (IKEv2 only).
	 *
	 * @return 		key exchange method of this payload
	 */
	key_exchange_method_t (*get_key_exchange_method)(ke_payload_t *this);

	/**
	 * Destroys a ke_payload_t object.
	 */
	void (*destroy) (ke_payload_t *this);
};

/**
 * Creates an empty ke_payload_t object.
 *
 * @param type		PLV2_KEY_EXCHANGE or PLV1_KEY_EXCHANGE
 * @return			ke_payload_t object
 */
ke_payload_t *ke_payload_create(payload_type_t type);

/**
 * Creates a ke_payload_t from a key_exchange_t.
 *
 * @param type		PLV2_KEY_EXCHANGE or PLV1_KEY_EXCHANGE
 * @param ke		key exchange object containing method and public key
 * @return 			ke_payload_t object, NULL on error
 */
ke_payload_t *ke_payload_create_from_key_exchange(payload_type_t type,
												  key_exchange_t *ke);

#endif /** KE_PAYLOAD_H_ @}*/
