/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup keymat keymat
 * @{ @ingroup sa
 */

#ifndef KEYMAT_H_
#define KEYMAT_H_

typedef struct keymat_t keymat_t;

#include <library.h>
#include <utils/identification.h>
#include <crypto/prfs/prf.h>
#include <crypto/aead.h>
#include <config/proposal.h>
#include <config/peer_cfg.h>
#include <sa/ike_sa_id.h>

/**
 * Constructor function for custom keymat implementations
 *
 * @param initiator		TRUE if the keymat is used as initiator
 * @return				keymat_t implementation
 */
typedef keymat_t* (*keymat_constructor_t)(bool initiator);

/**
 * Derivation an management of sensitive keying material.
 */
struct keymat_t {

	/**
	 * Get IKE version of this keymat.
	 *
	 * @return			IKEV1 for keymat_v1_t, IKEV2 for keymat_v2_t
	 */
	ike_version_t (*get_version)(keymat_t *this);

	/**
	 * Create a diffie hellman object for key agreement.
	 *
	 * The diffie hellman is either for IKE negotiation/rekeying or
	 * CHILD_SA rekeying (using PFS). The resulting DH object must be passed
	 * to derive_keys or to derive_child_keys and destroyed after use.
	 *
	 * Only DH objects allocated through this method are passed to other
	 * keymat_t methods, allowing private DH implementations. In some cases
	 * (such as retrying with a COOKIE), a DH object allocated from a different
	 * keymat_t instance may be passed to other methods.
	 *
	 * @param group			diffie hellman group
	 * @return				DH object, NULL if group not supported
	 */
	diffie_hellman_t* (*create_dh)(keymat_t *this,
								   diffie_hellman_group_t group);

	/**
	 * Create a nonce generator object.
	 *
	 * The nonce generator can be used to create nonces needed during IKE/CHILD
	 * SA establishment or rekeying.
	 *
	 * @return				nonce generator object
	 */
	nonce_gen_t* (*create_nonce_gen)(keymat_t *this);

	/**
	 * Get a AEAD transform to en-/decrypt and sign/verify IKE messages.
	 *
	 * @param in		TRUE for inbound (decrypt), FALSE for outbound (encrypt)
	 * @return			crypter
	 */
	aead_t* (*get_aead)(keymat_t *this, bool in);

	/**
	 * Destroy a keymat_t.
	 */
	void (*destroy)(keymat_t *this);
};

/**
 * Create the appropriate keymat_t implementation based on the IKE version.
 *
 * @param version			requested IKE version
 * @param initiator			TRUE if we are initiator
 * @return					keymat_t implmenetation
 */
keymat_t *keymat_create(ike_version_t version, bool initiator);

/**
 * Look up the key length of an encryption algorithm.
 *
 * @param alg				algorithm to get key length for
 * @return					key length in bits
 */
int keymat_get_keylen_encr(encryption_algorithm_t alg);

/**
 * Look up the key length of an integrity algorithm.
 *
 * @param alg				algorithm to get key length for
 * @return					key length in bits
 */
int keymat_get_keylen_integ(integrity_algorithm_t alg);

/**
 * Register keymat_t constructor for given IKE version.
 *
 * @param version			IKE version of given keymat constructor
 * @param create			keymat constructor function, NULL to unregister
 */
void keymat_register_constructor(ike_version_t version,
								 keymat_constructor_t create);

#endif /** KEYMAT_H_ @}*/
