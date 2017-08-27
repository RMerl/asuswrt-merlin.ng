/*
 * Copyright (C) 2014 Tobias Brunner
 * Copyright (C) 2005-2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 * Copyright (C) 2005 Jan Hutter
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
 * @defgroup encrypted_payload encrypted_payload
 * @{ @ingroup payloads
 */

#ifndef ENCRYPTED_PAYLOAD_H_
#define ENCRYPTED_PAYLOAD_H_

typedef struct encrypted_payload_t encrypted_payload_t;

#include <library.h>
#include <crypto/aead.h>
#include <encoding/payloads/payload.h>
#include <encoding/generator.h>

/**
 * The encrypted payload as described in RFC section 3.14.
 */
struct encrypted_payload_t {

	/**
	 * Implements payload_t interface.
	 */
	payload_t payload_interface;

	/**
	 * Get the payload length.
	 *
	 * @return			(expected) payload length
	 */
	size_t (*get_length)(encrypted_payload_t *this);

	/**
	 * Adds a payload to this encryption payload.
	 *
	 * @param payload		payload_t object to add
	 */
	void (*add_payload) (encrypted_payload_t *this, payload_t *payload);

	/**
	 * Remove the first payload in the list
	 *
	 * @param payload		removed payload
	 * @return				payload, NULL if none left
	 */
	payload_t* (*remove_payload)(encrypted_payload_t *this);

	/**
	 * Uses the given generator to generate the contained payloads.
	 *
	 * @param generator		generator used to generate the contained payloads
	 */
	void (*generate_payloads)(encrypted_payload_t *this,
							  generator_t *generator);

	/**
	 * Set the AEAD transform to use.
	 *
	 * @param aead		aead transform to use
	 */
	void (*set_transform) (encrypted_payload_t *this, aead_t *aead);

	/**
	 * Generate, encrypt and sign contained payloads.
	 *
	 * @param mid			message ID
	 * @param assoc			associated data
	 * @return
	 * 						- SUCCESS if encryption successful
	 * 						- FAILED if encryption failed
	 * 						- INVALID_STATE if aead not supplied, but needed
	 */
	status_t (*encrypt) (encrypted_payload_t *this, u_int64_t mid,
						 chunk_t assoc);

	/**
	 * Decrypt, verify and parse contained payloads.
	 *
	 * @param assoc			associated data
	 * @return
	 * 						- SUCCESS if parsing successful
	 *						- PARSE_ERROR if sub-payload parsing failed
	 * 						- VERIFY_ERROR if sub-payload verification failed
	 * 						- FAILED if integrity check failed
	 * 						- INVALID_STATE if aead not supplied, but needed
	 */
	status_t (*decrypt) (encrypted_payload_t *this, chunk_t assoc);

	/**
	 * Destroys an encrypted_payload_t object.
	 */
	void (*destroy) (encrypted_payload_t *this);
};

/**
 * Creates an empty encrypted_payload_t object.
 *
 * @param type		PLV2_ENCRYPTED or PLV1_ENCRYPTED
 * @return			encrypted_payload_t object
 */
encrypted_payload_t *encrypted_payload_create(payload_type_t type);

/**
 * Creates an encrypted payload with the given plain text data and next payload
 * type.
 *
 * @param next		next payload type
 * @param plain		plaintext data (gets adopted)
 * @return			encrypted_payload_t object
 */
encrypted_payload_t *encrypted_payload_create_from_plain(payload_type_t next,
														 chunk_t plain);

#endif /** ENCRYPTED_PAYLOAD_H_ @}*/
