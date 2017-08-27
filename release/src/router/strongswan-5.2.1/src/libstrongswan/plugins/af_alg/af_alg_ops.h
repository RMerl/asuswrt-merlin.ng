/*
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
* @defgroup af_alg_ops af_alg_ops
 * @{ @ingroup af_alg
 */

#ifndef AF_ALG_OPS_H_
#define AF_ALG_OPS_H_

#include <library.h>

#include <linux/if_alg.h>

#ifndef AF_ALG
#define AF_ALG 38
#endif /* AF_ALG */

#ifndef SOL_ALG
#define SOL_ALG 279
#endif /* SOL_ALG */

typedef struct af_alg_ops_t af_alg_ops_t;

/**
 * Helper to run AF_ALG operations.
 */
struct af_alg_ops_t {

	/**
	 * Hash a chunk of data.
	 *
	 * @param data		data to hash
	 * @param out		buffer to write hash to, NULL for append mode
	 * @param outlen	number of bytes to read into out
	 * @return			TRUE if successful
	 */
	bool (*hash)(af_alg_ops_t *this, chunk_t data, char *out, size_t outlen);

	/**
	 * Reset hasher state.
	 */
	void (*reset)(af_alg_ops_t *this);

	/**
	 * En-/Decrypt a chunk of data.
	 *
	 * @param type		crypto operation (ALG_OP_DECRYPT/ALG_OP_ENCRYPT)
	 * @param iv		iv to use
	 * @param data		data to encrypt/decrypt
	 * @param out		buffer write processed data to
	 * @return			TRUE if successful
	 */
	bool (*crypt)(af_alg_ops_t *this, u_int32_t type, chunk_t iv, chunk_t data,
				  char *out);

	/**
	 * Set the key for en-/decryption or HMAC/XCBC operations.
	 *
	 * @param key		key to set for transform
	 * @return			TRUE if successful
	 */
	bool (*set_key)(af_alg_ops_t *this, chunk_t key);

	/**
	 * Destroy a af_alg_ops_t.
	 */
	void (*destroy)(af_alg_ops_t *this);
};

/**
 * Create a af_alg_ops instance.
 *
 * @param type			algorithm type (hash, skcipher)
 * @param alg			algorithm name
 * @return				TRUE if AF_ALG socket bound successfully
 */
af_alg_ops_t *af_alg_ops_create(char *type, char *alg);

#endif /** AF_ALG_OPS_H_ @}*/
