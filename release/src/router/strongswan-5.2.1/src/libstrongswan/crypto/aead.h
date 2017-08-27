/*
 * Copyright (C) 2013 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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
 * @defgroup aead aead
 * @{ @ingroup crypto
 */

#ifndef AEAD_H_
#define AEAD_H_

typedef struct aead_t aead_t;

#include <library.h>
#include <crypto/crypters/crypter.h>
#include <crypto/signers/signer.h>
#include <crypto/iv/iv_gen.h>

/**
 * Authenticated encryption / authentication decryption interface.
 */
struct aead_t {

	/**
	 * Encrypt and sign data, sign associated data.
	 *
	 * The plain data must be a multiple of get_block_size(), the IV must
	 * have a length of get_iv_size().
	 * If encrypted is NULL, the encryption is done inline. The buffer must
	 * have space for additional get_icv_size() data, the ICV value is
	 * appended silently to the plain chunk.
	 *
	 * @param plain			data to encrypt and sign
	 * @param assoc			associated data to sign
	 * @param iv			initialization vector
	 * @param encrypted		allocated encryption result
	 * @return				TRUE if successfully encrypted
	 */
	bool (*encrypt)(aead_t *this, chunk_t plain, chunk_t assoc, chunk_t iv,
					chunk_t *encrypted) __attribute__((warn_unused_result));

	/**
	 * Decrypt and verify data, verify associated data.
	 *
	 * The IV must have a length of get_iv_size().
	 * If plain is NULL, the decryption is done inline. The decrypted data
	 * is returned in the encrypted chunk, the last get_icv_size() bytes
	 * contain the verified ICV.
	 *
	 * @param encrypted		data to decrypt and verify
	 * @param assoc			associated data to verify
	 * @param iv			initialization vector
	 * @param plain			allocated result, if successful
	 * @return				TRUE if MAC verification successful
	 */
	bool (*decrypt)(aead_t *this, chunk_t encrypted, chunk_t assoc, chunk_t iv,
					chunk_t *plain);

	/**
	 * Get the block size for encryption.
	 *
	 * @return				block size in bytes
	 */
	size_t (*get_block_size)(aead_t *this);

	/**
	 * Get the integrity check value size of the algorithm.
	 *
	 * @return				ICV size in bytes
	 */
	size_t (*get_icv_size)(aead_t *this);

	/**
	 * Get the size of the initialization vector.
	 *
	 * @return				IV size in bytes
	 */
	size_t (*get_iv_size)(aead_t *this);

	/**
	 * Get the IV generator implementation
	 *
	 * @return				IV generator
	 */
	iv_gen_t *(*get_iv_gen)(aead_t *this);

	/**
	 * Get the size of the key material (for encryption and authentication).
	 *
	 * This includes any additional bytes requires for the implicit nonce part.
	 * For AEADs based on traditional ciphers, the length is for both
	 * the integrity and the encryption key in total.
	 *
	 * @return				key size in bytes
	 */
	size_t (*get_key_size)(aead_t *this);

	/**
	 * Set the key for encryption and authentication.
	 *
	 * If the AEAD uses an implicit nonce, the last part of the key shall
	 * be the implicit nonce. For AEADs based on traditional ciphers, the
	 * key shall include both integrity and encryption keys, concatenated
	 * in that order.
	 *
	 * @param key			encryption and authentication key
	 * @return				TRUE if key set successfully
	 */
	bool (*set_key)(aead_t *this,
					chunk_t key) __attribute__((warn_unused_result));

	/**
	 * Destroy an aead_t.
	 */
	void (*destroy)(aead_t *this);
};

/**
 * Create a aead instance using traditional transforms.
 *
 * @param crypter		encryption transform for this aead
 * @param signer		integrity transform for this aead
 * @return				aead transform
 */
aead_t *aead_create(crypter_t *crypter, signer_t *signer);

#endif /** AEAD_H_ @}*/
