/*
 * Copyright (C) 2005-2009 Martin Willi
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
 * @defgroup signer signer
 * @{ @ingroup crypto
 */

#ifndef SIGNER_H_
#define SIGNER_H_

typedef enum integrity_algorithm_t integrity_algorithm_t;
typedef struct signer_t signer_t;

#include <library.h>

/**
 * Integrity algorithm, as in IKEv2 RFC 3.3.2.
 *
 * Algorithms not specified in IKEv2 are allocated in private use space.
 */
enum integrity_algorithm_t {
	AUTH_UNDEFINED = 1024,
	/** RFC4306 */
	AUTH_HMAC_MD5_96 = 1,
	/** RFC4306 */
	AUTH_HMAC_SHA1_96 = 2,
	/** RFC4306 */
	AUTH_DES_MAC = 3,
	/** RFC1826 */
	AUTH_KPDK_MD5 = 4,
	/** RFC4306 */
	AUTH_AES_XCBC_96 = 5,
	/** RFC4595 */
	AUTH_HMAC_MD5_128 = 6,
	/** RFC4595 */
	AUTH_HMAC_SHA1_160 = 7,
	/** RFC4494 */
	AUTH_AES_CMAC_96 = 8,
	/** RFC4543 */
	AUTH_AES_128_GMAC = 9,
	/** RFC4543 */
	AUTH_AES_192_GMAC = 10,
	/** RFC4543 */
	AUTH_AES_256_GMAC = 11,
	/** RFC4868 */
	AUTH_HMAC_SHA2_256_128 = 12,
	/** RFC4868 */
	AUTH_HMAC_SHA2_384_192 = 13,
	/** RFC4868 */
	AUTH_HMAC_SHA2_512_256 = 14,
	/** private use */
	AUTH_HMAC_SHA1_128 = 1025,
	/** SHA256 96 bit truncation variant, supported by Linux kernels */
	AUTH_HMAC_SHA2_256_96 = 1026,
	/** SHA256 full length truncation variant, as used in TLS */
	AUTH_HMAC_SHA2_256_256 = 1027,
	/** SHA384 full length truncation variant, as used in TLS */
	AUTH_HMAC_SHA2_384_384 = 1028,
	/** SHA512 full length truncation variant */
	AUTH_HMAC_SHA2_512_512 = 1029,
	/** draft-kanno-ipsecme-camellia-xcbc, not yet assigned by IANA */
	AUTH_CAMELLIA_XCBC_96 = 1030,
};

/**
 * enum names for integrity_algorithm_t.
 */
extern enum_name_t *integrity_algorithm_names;

/**
 * Generic interface for a symmetric signature algorithm.
 */
struct signer_t {
	/**
	 * Generate a signature.
	 *
	 * If buffer is NULL, data is processed and prepended to a next call until
	 * buffer is a valid pointer.
	 *
	 * @param data		a chunk containing the data to sign
	 * @param buffer	pointer where the signature will be written
	 * @return			TRUE if signature created successfully
	 */
	bool (*get_signature)(signer_t *this, chunk_t data,
						  u_int8_t *buffer) __attribute__((warn_unused_result));

	/**
	 * Generate a signature and allocate space for it.
	 *
	 * If chunk is NULL, data is processed and prepended to a next call until
	 * chunk is a valid chunk pointer.
	 *
	 * @param data		a chunk containing the data to sign
	 * @param chunk		chunk which will hold the allocated signature
	 * @return			TRUE if signature allocated successfully
	 */
	bool (*allocate_signature)(signer_t *this, chunk_t data,
						  chunk_t *chunk) __attribute__((warn_unused_result));

	/**
	 * Verify a signature.
	 *
	 * To verify a signature of multiple chunks of data, pass the
	 * data to get_signature() with a NULL buffer. verify_signature() acts
	 * as a final call and includes all data fed to get_signature().
	 *
	 * @param data		a chunk containing the data to verify
	 * @param signature	a chunk containing the signature
	 * @return			TRUE, if signature is valid, FALSE otherwise
	 */
	bool (*verify_signature)(signer_t *this, chunk_t data, chunk_t signature);

	/**
	 * Get the block size of this signature algorithm.
	 *
	 * @return			block size in bytes
	 */
	size_t (*get_block_size)(signer_t *this);

	/**
	 * Get the key size of the signature algorithm.
	 *
	 * @return			key size in bytes
	 */
	size_t (*get_key_size)(signer_t *this);

	/**
	 * Set the key for this object.
	 *
	 * @param key		key to set
	 * @return			TRUE if key set
	 */
	bool (*set_key)(signer_t *this,
					chunk_t key) __attribute__((warn_unused_result));

	/**
	 * Destroys a signer_t object.
	 */
	void (*destroy)(signer_t *this);
};

#endif /** SIGNER_H_ @}*/
