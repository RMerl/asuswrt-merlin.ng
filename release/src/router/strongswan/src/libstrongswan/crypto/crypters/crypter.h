/*
 * Copyright (C) 2005-2006 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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
 * @defgroup crypter crypter
 * @{ @ingroup crypto
 */

#ifndef CRYPTER_H_
#define CRYPTER_H_

typedef enum encryption_algorithm_t encryption_algorithm_t;
typedef struct crypter_t crypter_t;

#include <library.h>

/**
 * Encryption algorithm, as in IKEv2 RFC 3.3.2.
 */
enum encryption_algorithm_t {
	ENCR_DES_IV64 =            1,
	ENCR_DES =                 2,
	ENCR_3DES =                3,
	ENCR_RC5 =                 4,
	ENCR_IDEA =                5,
	ENCR_CAST =                6,
	ENCR_BLOWFISH =            7,
	ENCR_3IDEA =               8,
	ENCR_DES_IV32 =            9,
	ENCR_NULL =               11,
	ENCR_AES_CBC =            12,
	/** CTR as specified for IPsec (RFC5930/RFC3686), nonce appended to key */
	ENCR_AES_CTR =            13,
	ENCR_AES_CCM_ICV8 =       14,
	ENCR_AES_CCM_ICV12 =      15,
	ENCR_AES_CCM_ICV16 =      16,
	ENCR_AES_GCM_ICV8 =       18,
	ENCR_AES_GCM_ICV12 =      19,
	ENCR_AES_GCM_ICV16 =      20,
	ENCR_NULL_AUTH_AES_GMAC = 21,
	ENCR_CAMELLIA_CBC =       23,
	/* CTR as specified for IPsec (RFC5529), nonce appended to key */
	ENCR_CAMELLIA_CTR =       24,
	ENCR_CAMELLIA_CCM_ICV8 =  25,
	ENCR_CAMELLIA_CCM_ICV12 = 26,
	ENCR_CAMELLIA_CCM_ICV16 = 27,
	ENCR_CHACHA20_POLY1305 =  28,
	ENCR_UNDEFINED =        1024,
	ENCR_DES_ECB =          1025,
	ENCR_SERPENT_CBC =      1026,
	ENCR_TWOFISH_CBC =      1027,
	/* see macros below to handle RC2 (effective) key length */
	ENCR_RC2_CBC =          1028,
};

#define DES_BLOCK_SIZE			 8
#define BLOWFISH_BLOCK_SIZE		 8
#define AES_BLOCK_SIZE			16
#define CAMELLIA_BLOCK_SIZE		16
#define SERPENT_BLOCK_SIZE		16
#define TWOFISH_BLOCK_SIZE		16

/**
 * For RC2, if the effective key size in bits is not key_size * 8, it should
 * be encoded with the macro below. It can be decoded with the other two macros.
 * After decoding the value should be validated.
 */
#define RC2_KEY_SIZE(kl, eff) ((kl) | ((eff) << 8))
#define RC2_EFFECTIVE_KEY_LEN(ks) ((ks) >> 8)
#define RC2_KEY_LEN(ks) ((ks) & 0xff)

/**
 * enum name for encryption_algorithm_t.
 */
extern enum_name_t *encryption_algorithm_names;

/**
 * Generic interface for symmetric encryption algorithms.
 */
struct crypter_t {

	/**
	 * Encrypt a chunk of data and allocate space for the encrypted value.
	 *
	 * The length of the iv must equal to get_iv_size(), while the length
	 * of data must be a multiple of get_block_size().
	 * If encrypted is NULL, the encryption is done in-place (overwriting data).
	 *
	 * @param data			data to encrypt
	 * @param iv			initializing vector
	 * @param encrypted		chunk to allocate encrypted data, or NULL
	 * @return				TRUE if encryption successful
	 */
	bool (*encrypt)(crypter_t *this, chunk_t data, chunk_t iv,
					chunk_t *encrypted) __attribute__((warn_unused_result));

	/**
	 * Decrypt a chunk of data and allocate space for the decrypted value.
	 *
	 * The length of the iv must equal to get_iv_size(), while the length
	 * of data must be a multiple of get_block_size().
	 * If decrpyted is NULL, the encryption is done in-place (overwriting data).
	 *
	 * @param data			data to decrypt
	 * @param iv			initializing vector
	 * @param encrypted		chunk to allocate decrypted data, or NULL
	 * @return				TRUE if decryption successful
	 */
	bool (*decrypt)(crypter_t *this, chunk_t data, chunk_t iv,
					chunk_t *decrypted) __attribute__((warn_unused_result));

	/**
	 * Get the block size of the crypto algorithm.
	 *
	 * get_block_size() returns the smallest block the crypter can handle,
	 * not the block size of the underlying crypto algorithm. For counter mode,
	 * it is usually 1.
	 *
	 * @return				block size in bytes
	 */
	size_t (*get_block_size)(crypter_t *this);

	/**
	 * Get the IV size of the crypto algorithm.
	 *
	 * @return				initialization vector size in bytes
	 */
	size_t (*get_iv_size)(crypter_t *this);

	/**
	 * Get the key size of the crypto algorithm.
	 *
	 * get_key_size() might return a key length different from the key
	 * size passed to the factory constructor. For Counter Mode, the nonce
	 * is handled as a part of the key material and is passed to set_key().
	 *
	 * @return				key size in bytes
	 */
	size_t (*get_key_size)(crypter_t *this);

	/**
	 * Set the key.
	 *
	 * The length of the key must match get_key_size().
	 *
	 * @param key			key to set
	 * @return				TRUE if key set successfully
	 */
	bool (*set_key)(crypter_t *this,
					chunk_t key) __attribute__((warn_unused_result));

	/**
	 * Destroys a crypter_t object.
	 */
	void (*destroy)(crypter_t *this);
};

/**
 * Conversion of ASN.1 OID to encryption algorithm.
 *
 * @param oid			ASN.1 OID
 * @param key_size		returns size of encryption key in bits
 * @return				encryption algorithm, ENCR_UNDEFINED if OID unsupported
 */
encryption_algorithm_t encryption_algorithm_from_oid(int oid, size_t *key_size);

/**
 * Conversion of encryption algorithm to ASN.1 OID.
 *
 * @param alg			encryption algorithm
 * @param key_size		size of encryption key in bits
 * @return				ASN.1 OID, OID_UNKNOWN if OID is unknown
 */
int encryption_algorithm_to_oid(encryption_algorithm_t alg, size_t key_size);

/**
 * Check if an encryption algorithm identifier is an AEAD algorithm.
 *
 * @param alg			algorithm identifier
 * @return				TRUE if it is an AEAD algorithm
 */
bool encryption_algorithm_is_aead(encryption_algorithm_t alg);

#endif /** CRYPTER_H_ @}*/
