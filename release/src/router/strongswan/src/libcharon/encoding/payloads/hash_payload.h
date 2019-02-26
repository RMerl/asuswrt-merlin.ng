/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
 * @defgroup hash_payload hash_payload
 * @{ @ingroup payloads
 */

#ifndef HASH_PAYLOAD_H_
#define HASH_PAYLOAD_H_

typedef struct hash_payload_t hash_payload_t;

#include <library.h>
#include <encoding/payloads/payload.h>

/**
 * Object representing an IKEv1 hash payload.
 */
struct hash_payload_t {

	/**
	 * The payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Set the hash value.
	 *
	 * @param hash			chunk containing the hash, will be cloned
	 */
	void (*set_hash) (hash_payload_t *this, chunk_t hash);

	/**
	 * Get the hash value.
	 *
	 * @return				chunkt to internal hash data
	 */
	chunk_t (*get_hash) (hash_payload_t *this);

	/**
	 * Destroys an hash_payload_t object.
	 */
	void (*destroy) (hash_payload_t *this);
};

/**
 * Creates an empty hash_payload_t object.
 *
 * @param type		either PLV1_HASH or PLV1_NAT_D
 * @return			hash_payload_t object
 */
hash_payload_t *hash_payload_create(payload_type_t type);

#endif /** HASH_PAYLOAD_H_ @}*/
