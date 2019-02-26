/*
 * Copyright (C) 2009 Martin Willi
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
 * @defgroup simaka_crypto simaka_crypto
 * @{ @ingroup libsimaka
 */

#ifndef SIMAKA_CRYPTO_H_
#define SIMAKA_CRYPTO_H_

#include <library.h>

typedef struct simaka_crypto_t simaka_crypto_t;

/**
 * EAP-SIM/AKA crypto helper and key derivation class.
 */
struct simaka_crypto_t {

	/**
	 * Get the signer to use for AT_MAC calculation/verification.
	 *
	 * @return		signer reference, NULL if no keys have been derived
	 */
	signer_t* (*get_signer)(simaka_crypto_t *this);

	/**
	 * Get the signer to use for AT_ENCR_DATA encryption/decryption.
	 *
	 * @return		crypter reference, NULL if no keys have been derived
	 */
	crypter_t* (*get_crypter)(simaka_crypto_t *this);

	/**
	 * Get the random number generator.
	 *
	 * @return		rng reference
	 */
	rng_t* (*get_rng)(simaka_crypto_t *this);

	/**
	 * Derive keys after full authentication.
	 *
	 * This methods derives the k_encr/k_auth keys and loads them into the
	 * internal crypter/signer instances. The passed data is method specific:
	 * For EAP-SIM, it is "n*Kc|NONCE_MT|Version List|Selected Version", for
	 * EAP-AKA it is "IK|CK".
	 *
	 * @param id	peer identity
	 * @param data	method specific data
	 * @param mk	chunk receiving allocated master key MK
	 * @param msk	chunk receiving allocated MSK
	 * @return		TRUE if keys allocated and derived successfully
	 */
	bool (*derive_keys_full)(simaka_crypto_t *this, identification_t *id,
							 chunk_t data, chunk_t *mk, chunk_t *msk);

	/**
	 * Derive k_encr/k_auth keys from MK using fast reauthentication.
	 *
	 * This methods derives the k_encr/k_auth keys and loads them into the
	 * internal crypter/signer instances.
	 *
	 * @param mk	master key
	 * @return		TRUE if keys derived successfully
	 */
	bool (*derive_keys_reauth)(simaka_crypto_t *this, chunk_t mk);

	/**
	 * Derive MSK using fast reauthentication.
	 *
	 * @param id		fast reauthentication identity
	 * @param counter	fast reauthentication counter value, network order
	 * @param nonce_s	server generated NONCE_S value
	 * @param mk		master key of last full authentication
	 * @param msk		chunk receiving allocated MSK
	 * @return			TRUE if MSK allocated and derived successfully
	 */
	bool (*derive_keys_reauth_msk)(simaka_crypto_t *this,
								   identification_t *id, chunk_t counter,
								   chunk_t nonce_s, chunk_t mk, chunk_t *msk);

	/**
	 * Clear keys (partially) derived.
	 */
	void (*clear_keys)(simaka_crypto_t *this);

	/**
	 * Destroy a simaka_crypto_t.
	 */
	void (*destroy)(simaka_crypto_t *this);
};

/**
 * Create a simaka_crypto instance.
 *
 * @return		EAP-SIM/AKA crypto instance, NULL if algorithms missing
 */
simaka_crypto_t *simaka_crypto_create();

#endif /** SIMAKA_CRYPTO_H_ @}*/
