/*
 * Copyright (C) 2012-2013 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
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
 * @defgroup esp_context esp_context
 * @{ @ingroup libipsec
 */

#ifndef ESP_CONTEXT_H_
#define ESP_CONTEXT_H_

#include <library.h>
#include <crypto/aead.h>

typedef struct esp_context_t esp_context_t;

/**
 *  ESP context, handles sequence numbers and maintains cryptographic primitives
 */
struct esp_context_t {

	/**
	 * Get AEAD wrapper or method to encrypt/decrypt/authenticate ESP packets.
	 *
	 * @return				AEAD wrapper of method
	 */
	aead_t *(*get_aead)(esp_context_t *this);

	/**
	 * Get the current outbound ESP sequence number or the highest authenticated
	 * inbound sequence number.
	 *
	 * @return			current sequence number, in host byte order
	 */
	uint32_t (*get_seqno)(esp_context_t *this);

	/**
	 * Allocate the next outbound ESP sequence number.
	 *
	 * @param seqno		the sequence number, in host byte order
	 * @return			FALSE if the sequence number cycled or inbound context
	 */
	bool (*next_seqno)(esp_context_t *this, uint32_t *seqno);

	/**
	 * Verify an ESP sequence number.  Checks whether a packet with this
	 * sequence number was already received, using the anti-replay window.
	 * This operation does not modify the internal state.  After the sequence
	 * number is successfully verified and the ESP packet is authenticated,
	 * set_authenticated_seqno() should be called.
	 *
	 * @param seqno		the sequence number to verify, in host byte order
	 * @return			TRUE when sequence number is valid
	 */
	bool (*verify_seqno)(esp_context_t *this, uint32_t seqno);

	/**
	 * Adds a sequence number that was successfully verified and
	 * authenticated.  A user MUST call verify_seqno() immediately before
	 * calling this method.
	 *
	 * @param seqno		verified and authenticated seq number in host byte order
	 */
	void (*set_authenticated_seqno)(esp_context_t *this,
									uint32_t seqno);

	/**
	 * Destroy an esp_context_t
	 */
	void (*destroy)(esp_context_t *this);

};

/**
 * Create an esp_context_t instance
 *
 * @param enc_alg		encryption algorithm
 * @param enc_key		encryption key
 * @param int_alg		integrity protection algorithm
 * @param int_key		integrity protection key
 * @param inbound		TRUE to create an inbound ESP context
 * @return				ESP context instance, or NULL if creation fails
 */
esp_context_t *esp_context_create(int enc_alg, chunk_t enc_key, int int_alg,
								  chunk_t int_key, bool inbound);

#endif /** ESP_CONTEXT_H_ @}*/

