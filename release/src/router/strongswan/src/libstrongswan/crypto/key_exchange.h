/*
 * Copyright (C) 2010-2019 Tobias Brunner
 * Copyright (C) 2005-2007 Martin Willi
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
 * @defgroup key_exchange key_exchange
 * @{ @ingroup crypto
 */

#ifndef KEY_EXCHANGE_H_
#define KEY_EXCHANGE_H_

typedef enum key_exchange_method_t key_exchange_method_t;
typedef struct key_exchange_t key_exchange_t;
typedef struct diffie_hellman_params_t diffie_hellman_params_t;

#include <library.h>

/**
 * Key exchange method.
 *
 * The modulus (or group) to use for a Diffie-Hellman calculation.
 * See IKEv2 RFC 3.3.2 and RFC 3526.
 *
 * ECP groups are defined in RFC 4753 and RFC 5114.
 * ECC Brainpool groups are defined in RFC 6954.
 * Curve25519 and Curve448 groups are defined in RFC 8031.
 */
enum key_exchange_method_t {
	KE_NONE       =  0,
	MODP_768_BIT  =  1,
	MODP_1024_BIT =  2,
	MODP_1536_BIT =  5,
	MODP_2048_BIT = 14,
	MODP_3072_BIT = 15,
	MODP_4096_BIT = 16,
	MODP_6144_BIT = 17,
	MODP_8192_BIT = 18,
	ECP_256_BIT   = 19,
	ECP_384_BIT   = 20,
	ECP_521_BIT   = 21,
	MODP_1024_160 = 22,
	MODP_2048_224 = 23,
	MODP_2048_256 = 24,
	ECP_192_BIT   = 25,
	ECP_224_BIT   = 26,
	ECP_224_BP    = 27,
	ECP_256_BP    = 28,
	ECP_384_BP    = 29,
	ECP_512_BP    = 30,
	CURVE_25519   = 31,
	CURVE_448     = 32,
	/** insecure NULL diffie hellman group for testing, in PRIVATE USE */
	MODP_NULL = 1024,
	/** MODP group with custom generator/prime */
	/** Parameters defined by IEEE 1363.1, in PRIVATE USE */
	NTRU_112_BIT = 1030,
	NTRU_128_BIT = 1031,
	NTRU_192_BIT = 1032,
	NTRU_256_BIT = 1033,
	NH_128_BIT   = 1040,
	/** internally used DH group with additional parameters g and p, outside
	 * of PRIVATE USE (i.e. IKEv2 DH group range) so it can't be negotiated */
	MODP_CUSTOM = 65536,
};

/**
 * enum name for key_exchange_method_t.
 */
extern enum_name_t *key_exchange_method_names;

/**
 * enum names for key_exchange_method_t (matching proposal keywords).
 */
extern enum_name_t *key_exchange_method_names_short;

/**
 * Implementation of a key exchange algorithms (e.g. Diffie-Hellman).
 */
struct key_exchange_t {

	/**
	 * Returns the shared secret of this key exchange method.
	 *
	 * @param secret	shared secret (allocated)
	 * @return			TRUE if shared secret computed successfully
	 */
	bool (*get_shared_secret)(key_exchange_t *this, chunk_t *secret)
		__attribute__((warn_unused_result));

	/**
	 * Sets the public key from the peer.
	 *
	 * @note This operation should be relatively quick. Costly public key
	 * validation operations or key derivation should be implemented in
	 * get_shared_secret().
	 *
	 * @param value		public key of peer
	 * @return			TRUE if other public key verified and set
	 */
	bool (*set_public_key)(key_exchange_t *this, chunk_t value)
		__attribute__((warn_unused_result));

	/**
	 * Gets the own public key to transmit.
	 *
	 * @param value		public key (allocated)
	 * @return			TRUE if public key retrieved
	 */
	bool (*get_public_key)(key_exchange_t *this, chunk_t *value)
		__attribute__((warn_unused_result));

	/**
	 * Set an explicit own private key to use.
	 *
	 * Calling this method is usually not required, as the DH backend generates
	 * an appropriate private value itself. It is optional to implement, and
	 * used mostly for testing purposes.  The private key may be the actual key
	 * or a seed for a DRBG.
	 *
	 * @param value		private key value to set
	 */
	bool (*set_private_key)(key_exchange_t *this, chunk_t value)
		__attribute__((warn_unused_result));

	/**
	 * Get the key exchange method used.
	 *
	 * @return			key exchange method set in construction
	 */
	key_exchange_method_t (*get_method)(key_exchange_t *this);

	/**
	 * Destroys a key_exchange_t object.
	 */
	void (*destroy)(key_exchange_t *this);
};

/**
 * Parameters for a specific Diffie-Hellman group.
 */
struct diffie_hellman_params_t {

	/**
	 * The prime of the group
	 */
	const chunk_t prime;

	/**
	 * Generator of the group
	 */
	const chunk_t generator;

	/**
	 * Exponent length to use
	 */
	size_t exp_len;

	/**
	 * Prime order subgroup; for MODP Groups 22-24
	 */
	const chunk_t subgroup;
};

/**
 * Initialize diffie hellman parameters during startup.
 */
void diffie_hellman_init();

/**
 * Get the parameters associated with the specified Diffie-Hellman group.
 *
 * Before calling this method, use diffie_hellman_init() to initialize the
 * DH group table. This is usually done by library_init().
 *
 * @param ke			key exchange method (DH group)
 * @return				The parameters or NULL, if the group is not supported
 */
diffie_hellman_params_t *diffie_hellman_get_params(key_exchange_method_t ke);

/**
 * Check if a given key exchange method is an ECDH group.
 *
 * @param ke			key exchange method to check
 * @return				TRUE if key exchange method is an ECP group
 */
bool key_exchange_is_ecdh(key_exchange_method_t ke);

/**
 * Check if a public key is valid for given key exchange method.
 *
 * @param ke			key exchange method
 * @param value			public key to check
 * @return				TRUE if value looks valid
 */
bool key_exchange_verify_pubkey(key_exchange_method_t ke, chunk_t value);

#endif /** KEY_EXCHANGE_H_ @}*/
