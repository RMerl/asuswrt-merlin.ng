/*
 * Copyright (C) 2017 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
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
 * @defgroup private_key private_key
 * @{ @ingroup keys
 */

#ifndef PRIVATE_KEY_H_
#define PRIVATE_KEY_H_

typedef struct private_key_t private_key_t;

#include <credentials/cred_encoding.h>
#include <credentials/keys/public_key.h>

/**
 * Abstract private key interface.
 */
struct private_key_t {

	/**
	 * Get the key type.
	 *
	 * @return			type of the key
	 */
	key_type_t (*get_type)(private_key_t *this);

	/**
	 * Get signature schemes supported by this key.
	 *
	 * This is useful for keys that only support certain hash algorithms or
	 * require specific parameters for RSA/PSS signatures.
	 *
	 * @note Implementing this method is optional. If multiple schemes are
	 * returned, they should be ordered by decreasing preference.
	 *
	 * @return			enumerator over signature_params_t*
	 */
	enumerator_t *(*supported_signature_schemes)(private_key_t *this);

	/**
	 * Create a signature over a chunk of data.
	 *
	 * @param scheme	signature scheme to use
	 * @param params	optional parameters required by the specified scheme
	 * @param data		chunk of data to sign
	 * @param signature	where to allocate created signature
	 * @return			TRUE if signature created
	 */
	bool (*sign)(private_key_t *this, signature_scheme_t scheme, void *params,
				 chunk_t data, chunk_t *signature);
	/**
	 * Decrypt a chunk of data.
	 *
	 * @param scheme	expected encryption scheme used
	 * @param params	optional parameters required by the specified scheme
	 * @param crypto	chunk containing encrypted data
	 * @param plain		where to allocate decrypted data
	 * @return			TRUE if data decrypted and plaintext allocated
	 */
	bool (*decrypt)(private_key_t *this, encryption_scheme_t scheme,
					void *params, chunk_t crypto, chunk_t *plain);

	/**
	 * Get the strength of the key in bits.
	 *
	 * @return			strength of the key in bits
	 */
	int (*get_keysize) (private_key_t *this);

	/**
	 * Get the public part from the private key.
	 *
	 * @return			public key
	 */
	public_key_t* (*get_public_key)(private_key_t *this);

	/**
	 * Check if two private keys are equal.
	 *
	 * @param other		other private key
	 * @return			TRUE, if equality
	 */
	bool (*equals) (private_key_t *this, private_key_t *other);

	/**
	 * Check if a private key belongs to a public key.
	 *
	 * @param public	public key
	 * @return			TRUE, if keys belong together
	 */
	bool (*belongs_to) (private_key_t *this, public_key_t *public);

	/**
	 * Get the fingerprint of the key.
	 *
	 * @param type		type of fingerprint, one of KEYID_*
	 * @param fp		fingerprint, points to internal data
	 * @return			TRUE if fingerprint type supported
	 */
	bool (*get_fingerprint)(private_key_t *this, cred_encoding_type_t type,
							chunk_t *fp);

	/**
	 * Check if a key has a given fingerprint of any kind.
	 *
	 * @param fp		fingerprint to check
	 * @return			TRUE if key has given fingerprint
	 */
	bool (*has_fingerprint)(private_key_t *this, chunk_t fp);

	/**
	 * Get the key in an encoded form as a chunk.
	 *
	 * @param type		type of the encoding, one of PRIVKEY_*
	 * @param encoding	encoding of the key, allocated
	 * @return			TRUE if encoding supported
	 */
	bool (*get_encoding)(private_key_t *this, cred_encoding_type_t type,
						 chunk_t *encoding);

	/**
	 * Increase the refcount to this private key.
	 *
	 * @return			this, with an increased refcount
	 */
	private_key_t* (*get_ref)(private_key_t *this);

	/**
	 * Decrease refcount, destroy private_key if no more references.
	 */
	void (*destroy)(private_key_t *this);
};

/**
 * Generic private key equals() implementation, usable by implementers.
 *
 * @param private		private key to check
 * @param other			key to compare
 * @return				TRUE if this is equal to other
 */
bool private_key_equals(private_key_t *private, private_key_t *other);

/**
 * Generic private key belongs_to() implementation, usable by implementers.
 *
 * @param private		private key to check
 * @param public		public key to compare
 * @return				TRUE if this is equal to other
 */
bool private_key_belongs_to(private_key_t *private, public_key_t *public);

/**
 * Generic private key has_fingerprint() implementation, usable by implementers.
 *
 * @param private		private key to check
 * @param fingerprint	fingerprint to check
 * @return				TRUE if key has given fingerprint
 */
bool private_key_has_fingerprint(private_key_t *private, chunk_t fingerprint);

#endif /** PRIVATE_KEY_H_ @}*/
