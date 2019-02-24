/*
 * Copyright (C) 2011-2016 Tobias Brunner
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
 * @defgroup iv_manager iv_manager
 * @{ @ingroup ikev1
 */

#ifndef IV_MANAGER_H_
#define IV_MANAGER_H_

#include <utils/chunk.h>
#include <crypto/hashers/hasher.h>

typedef struct iv_manager_t iv_manager_t;

/**
 * IV and QM managing instance for IKEv1. Keeps track of phase 2 exchanges
 * and IV, as well as the phase 1 IV.
 */
struct iv_manager_t {

	/**
	 * Set the value of the first phase1 IV.
	 *
	 * @param data			input to calc initial IV from (g^xi | g^xr)
	 * @param hasher		hasher to be used for IV calculation
	 * 						(shared with keymat, must not be destroyed here)
	 * @param block_size	cipher block size of aead
	 * @return				TRUE for success, FALSE otherwise
	 */
	bool (*init_iv_chain)(iv_manager_t *this, chunk_t data, hasher_t *hasher,
						  size_t block_size);

	/**
	 * Returns the IV for a message with the given message ID.
	 *
	 * The return chunk contains internal data and is valid until the next
	 * get_iv/udpate_iv/confirm_iv() call.
	 *
	 * @param mid			message ID
	 * @param iv			chunk receiving IV, internal data
	 * @return				TRUE if IV allocated successfully
	 */
	bool (*get_iv)(iv_manager_t *this, uint32_t mid, chunk_t *iv);

	/**
	 * Updates the IV for the next message with the given message ID.
	 *
	 * A call of confirm_iv() is required in order to actually make the IV
	 * available.  This is needed for the inbound case where we store the last
	 * block of the encrypted message but want to update the IV only after
	 * verification of the decrypted message.
	 *
	 * @param mid			message ID
	 * @param last_block	last block of encrypted message (gets cloned)
	 * @return				TRUE if IV updated successfully
	 */
	bool (*update_iv)(iv_manager_t *this, uint32_t mid, chunk_t last_block);

	/**
	 * Confirms the updated IV for the given message ID.
	 *
	 * To actually make the new IV available via get_iv() this method has to
	 * be called after update_iv().
	 *
	 * @param mid			message ID
	 * @return				TRUE if IV confirmed successfully
	 */
	bool (*confirm_iv)(iv_manager_t *this, uint32_t mid);

	/**
	 * Try to find a QM for the given message ID, if not found, generate it.
	 * The nonces shall be assigned by the caller if they are not set yet.
	 *
	 * @param mid			message ID
	 * @param n_i			chunk pointer to contain Ni_b (Nonce from first
	 * 						message)
	 * @param n_r			chunk pointer to contain Nr_b (Nonce from second
	 * 						message)
	 */
	void (*lookup_quick_mode)(iv_manager_t *this, uint32_t mid, chunk_t **n_i,
							  chunk_t **n_r);

	/**
	 * Remove the QM for the given message ID.
	 *
	 * @param mid			message ID
	 */
	void (*remove_quick_mode)(iv_manager_t *this, uint32_t mid);

	/*
	 * Destroy a iv_manager_t.
	 */
	void (*destroy)(iv_manager_t *this);
};

/**
 * Create an IV and QM manager which is able to store up to max_exchanges
 * initialization vectors and quick modes.
 *
 * @param max_exchanges		maximum number of IVs and QMs to be stored, set
 * 							to 0 to use default (3, or as configured)
 * @return					IV and QM manager instance
 */
iv_manager_t *iv_manager_create(int max_exchanges);

#endif /** IV_MANAGER_H_ @}*/
