/*
 * Copyright (C) 2022 Tobias Brunner
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
 * @defgroup kdf kdf
 * @{ @ingroup crypto
 */

#ifndef KDF_H_
#define KDF_H_

typedef enum key_derivation_function_t key_derivation_function_t;
typedef enum kdf_param_t kdf_param_t;
typedef struct kdf_t kdf_t;

#include <library.h>

/**
 * Key Derivation Functions (KDF).
 */
enum key_derivation_function_t {

	KDF_UNDEFINED,

	/**
	 * RFC 7296 prf, expects a pseudo_random_function_t in the constructor,
	 * parameters are KEY (DH secret) and SALT (nonces).
	 * Has a fixed output length.
	 */
	KDF_PRF,

	/**
	 * RFC 7296 prf+, expects a pseudo_random_function_t in the constructor,
	 * parameters are KEY (SKEYSEED/SK_d) and SALT (nonces etc.).
	 */
	KDF_PRF_PLUS,
};

/**
 * enum name for key_derivation_function_t.
 */
extern enum_name_t *key_derivation_function_names;

/**
 * Parameters for KDFs.
 */
enum kdf_param_t {

	/**
	 * Key used for the key derivation (chunk_t).
	 */
	KDF_PARAM_KEY,

	/**
	 * Salt used for the key derivation (chunk_t).
	 */
	KDF_PARAM_SALT,
};

/**
 * Generic interface for Key Derivation Functions (KDF).
 *
 * Note that in comparison to xof_t, this interface does not support streaming.
 * That is, calling get_bytes() or allocate_bytes() multiple times without
 * changing the input parameters will result in the same output.
 */
struct kdf_t {

	/**
	 * Return the type of KDF.
	 *
	 * @return			KDF type
	 */
	key_derivation_function_t (*get_type)(kdf_t *this);

	/**
	 * Output length for KDFs that produce a fixed amount of output.
	 *
	 * @return			fixed output length, SIZE_MAX for variable length
	 */
	size_t (*get_length)(kdf_t *this);

	/**
	 * Derives a key of the given length and writes it to the buffer.
	 *
	 * @note Fails if out_len doesn't match for KDFs with fixed output length.
	 *
	 * @param out_len	number of key bytes requested
	 * @param buffer	pointer where the derived key will be written
	 * @return			TRUE if key derived successfully
	 */
	bool (*get_bytes)(kdf_t *this, size_t out_len,
					  uint8_t *buffer) __attribute__((warn_unused_result));

	/**
	 * Derives a key of the given length and allocates space for it.
	 *
	 * @note Fails if out_len doesn't match for KDFs with fixed output length.
	 * However, for simplified usage, 0 can be passed for out_len to
	 * automatically allocate a chunk of the correct size.
	 *
	 * @param out_len	number of key bytes requested, or 0 for KDFs with fixed
	 *					output length
	 * @param chunk		chunk which will hold the derived key
	 * @return			TRUE if key derived successfully
	 */
	bool (*allocate_bytes)(kdf_t *this, size_t out_len,
						   chunk_t *chunk) __attribute__((warn_unused_result));

	/**
	 * Set a parameter for this KDF.
	 *
	 * @param param		parameter to set
	 * @param ...		parameter values
	 * @return			TRUE if parameter set successfully
	 */
	bool (*set_param)(kdf_t *this, kdf_param_t param,
					 ...) __attribute__((warn_unused_result));

	/**
	 * Destroys this KDF object.
	 */
	void (*destroy)(kdf_t *this);
};

/**
 * Check if the given KDF type has a fixed output length.
 *
 * @param type			KDF type
 * @return				TRUE if the KDF type has a fixed output length
 */
bool kdf_has_fixed_output_length(key_derivation_function_t type);

#endif /** KDF_H_ @}*/
