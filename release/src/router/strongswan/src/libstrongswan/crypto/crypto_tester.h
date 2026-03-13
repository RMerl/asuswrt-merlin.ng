/*
 * Copyright (C) 2009 Martin Willi
 * Copyright (C) 2016-2019 Andreas Steffen
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
 * @defgroup crypto_tester crypto_tester
 * @{ @ingroup crypto
 */

#ifndef CRYPTO_TESTER_H_
#define CRYPTO_TESTER_H_

typedef struct crypto_tester_t crypto_tester_t;

#include <crypto/crypto_factory.h>

typedef struct crypter_test_vector_t crypter_test_vector_t;
typedef struct aead_test_vector_t aead_test_vector_t;
typedef struct signer_test_vector_t signer_test_vector_t;
typedef struct hasher_test_vector_t hasher_test_vector_t;
typedef struct prf_test_vector_t prf_test_vector_t;
typedef struct xof_test_vector_t xof_test_vector_t;
typedef struct kdf_test_vector_t kdf_test_vector_t;
typedef struct kdf_test_args_t kdf_test_args_t;
typedef struct drbg_test_vector_t drbg_test_vector_t;
typedef struct rng_test_vector_t rng_test_vector_t;
typedef struct ke_test_vector_t ke_test_vector_t;

struct crypter_test_vector_t {
	/** encryption algorithm this vector tests */
	encryption_algorithm_t alg;
	/** key length to use, in bytes */
	size_t key_size;
	/** encryption key of test vector */
	u_char *key;
	/** initialization vector, using crypters blocksize bytes */
	u_char *iv;
	/** length of plain and cipher text */
	size_t len;
	/** plain text */
	u_char *plain;
	/** cipher text */
	u_char *cipher;
};

struct aead_test_vector_t {
	/** encryption algorithm this vector tests */
	encryption_algorithm_t alg;
	/** key length to use, in bytes */
	size_t key_size;
	/** salt length to use, in bytes */
	size_t salt_size;
	/** encryption key of test vector */
	u_char *key;
	/** initialization vector, using crypters blocksize bytes */
	u_char *iv;
	/** length of associated data */
	size_t alen;
	/** associated data */
	u_char *adata;
	/** length of plain text */
	size_t len;
	/** plain text */
	u_char *plain;
	/** cipher text */
	u_char *cipher;
};

struct signer_test_vector_t {
	/** signer algorithm this test vector tests */
	integrity_algorithm_t alg;
	/** key to use, with a length the algorithm expects */
	u_char *key;
	/** size of the input data */
	size_t len;
	/** input data */
	u_char *data;
	/** expected output, with output size of the tested algorithm */
	u_char *mac;
};

struct hasher_test_vector_t {
	/** hash algorithm this test vector tests */
	hash_algorithm_t alg;
	/** length of the input data */
	size_t len;
	/** input data */
	u_char *data;
	/** expected hash, with hash size of the tested algorithm */
	u_char *hash;
};

struct prf_test_vector_t {
	/** prf algorithm this test vector tests */
	pseudo_random_function_t alg;
	/** is this PRF stateful? */
	bool stateful;
	/** key length to use, in bytes */
	size_t key_size;
	/** key to use */
	u_char *key;
	/** size of the seed data */
	size_t len;
	/** seed data */
	u_char *seed;
	/** expected output, with block size of the tested algorithm */
	u_char *out;
};

struct xof_test_vector_t {
	/** xof algorithm this test vector tests */
	ext_out_function_t alg;
	/** size of the seed data */
	size_t len;
	/** seed data */
	u_char *seed;
	/** size of the output */
	size_t out_len;
	/** expected output of size*/
	u_char *out;
};

struct kdf_test_vector_t {
	/** kdf algorithm this test vector tests */
	key_derivation_function_t alg;
	/** argument passed to constructor, type depends on alg */
	union {
		pseudo_random_function_t prf;
	} arg;
	/** optional key */
	chunk_t key;
	/** optional salt */
	chunk_t salt;
	/** expected output */
	chunk_t out;
};

struct kdf_test_args_t {
	/** the arguments used to construct the KDF */
	va_list args;
};

struct drbg_test_vector_t {
	/** drbg type this test vector tests */
	drbg_type_t type;
	/** security strength in bits */
	uint32_t strength;
	/** optional personalization string */
	chunk_t personalization_str;
	/** entropy_input | nonce | entropy_input_reseed */
	chunk_t entropy;
	/** returned output bits */
	chunk_t out;
};

/**
 * Test vector for a RNG.
 *
 * Contains a callback function to analyze the output of a RNG,
 */
struct rng_test_vector_t {
	/** quality of random data this test vector tests */
	rng_quality_t quality;
	/** callback function to test RNG output, returns TRUE if data ok */
	bool (*test)(void *user, chunk_t data);
	/** number of bytes the function requests */
	size_t len;
	/** user data passed back to the test() function on invocation */
	void *user;
};

struct ke_test_vector_t {
	/** key exchange method to test */
	key_exchange_method_t method;
	/** seed from which private key material is derived */
	chunk_t seed;
	/** expected public factor of initiator */
	chunk_t pub_i;
	/** expected public factor of responder */
	chunk_t pub_r;
	/** expected shared secret */
	chunk_t shared;
};

/**
 * Cryptographic primitive testing framework.
 */
struct crypto_tester_t {

	/**
	 * Test a crypter algorithm, optionally using a specified key size.
	 *
	 * @param alg			algorithm to test
	 * @param key_size		key size to test, 0 for default
	 * @param create		constructor function for the crypter
	 * @param speed			speed test result, NULL to omit
	 * @return				TRUE if test passed
	 */
	bool (*test_crypter)(crypto_tester_t *this, encryption_algorithm_t alg,
						 size_t key_size, crypter_constructor_t create,
						 u_int *speed, const char *plugin_name);

	/**
	 * Test an aead algorithm, optionally using a specified key size.
	 *
	 * @param alg			algorithm to test
	 * @param key_size		key size to test, 0 for default
	 * @param salt_size		salt length to test, 0 for default
	 * @param create		constructor function for the aead transform
	 * @param speed			speed test result, NULL to omit
	 * @return				TRUE if test passed
	 */
	bool (*test_aead)(crypto_tester_t *this, encryption_algorithm_t alg,
					  size_t key_size, size_t salt_size,
					  aead_constructor_t create,
					  u_int *speed, const char *plugin_name);
	/**
	 * Test a signer algorithm.
	 *
	 * @param alg			algorithm to test
	 * @param create		constructor function for the signer
	 * @param speed			speed test result, NULL to omit
	 * @return				TRUE if test passed
	 */
	bool (*test_signer)(crypto_tester_t *this, integrity_algorithm_t alg,
						signer_constructor_t create,
						u_int *speed, const char *plugin_name);
	/**
	 * Test a hasher algorithm.
	 *
	 * @param alg			algorithm to test
	 * @param create		constructor function for the hasher
	 * @param speed			speed test result, NULL to omit
	 * @return				TRUE if test passed
	 */
	bool (*test_hasher)(crypto_tester_t *this, hash_algorithm_t alg,
						hasher_constructor_t create,
						u_int *speed, const char *plugin_name);
	/**
	 * Test a PRF algorithm.
	 *
	 * @param alg			algorithm to test
	 * @param create		constructor function for the PRF
	 * @param speed			speed test result, NULL to omit
	 * @return				TRUE if test passed
	 */
	bool (*test_prf)(crypto_tester_t *this, pseudo_random_function_t alg,
					 prf_constructor_t create,
					 u_int *speed, const char *plugin_name);
	/**
	 * Test an XOF algorithm.
	 *
	 * @param alg			algorithm to test
	 * @param create		constructor function for the XOF
	 * @param speed			speed test result, NULL to omit
	 * @return				TRUE if test passed
	 */
	bool (*test_xof)(crypto_tester_t *this, ext_out_function_t alg,
					 xof_constructor_t create,
					 u_int *speed, const char *plugin_name);
	/**
	 * Test a KDF algorithm.
	 *
	 * If constructor arguments are passed, only matching test vectors are
	 * tried. Otherwise, all are tried and implementations are allowed to fail
	 * construction with unsupported arguments.
	 *
	 * @param alg			algorithm to test
	 * @param create		constructor function for the XOF
	 * @param args			optional arguments to pass to constructor
	 * @param speed			speed test result, NULL to omit
	 * @return				TRUE if test passed
	 */
	bool (*test_kdf)(crypto_tester_t *this, key_derivation_function_t alg,
					 kdf_constructor_t create, kdf_test_args_t *args,
					 u_int *speed, const char *plugin_name);
	/**
	 * Test a DRBG type.
	 *
	 * @param type			DRBG type to test
	 * @param create		constructor function for the DRBG
	 * @param speed			speed test result, NULL to omit
	 * @return				TRUE if test passed
	 */
	bool (*test_drbg)(crypto_tester_t *this, drbg_type_t type,
					  drbg_constructor_t create,
					  u_int *speed, const char *plugin_name);
	/**
	 * Test a RNG implementation.
	 *
	 * @param alg			algorithm to test
	 * @param create		constructor function for the RNG
	 * @param speed			speed test result, NULL to omit
	 * @return				TRUE if test passed
	 */
	bool (*test_rng)(crypto_tester_t *this, rng_quality_t quality,
					 rng_constructor_t create,
					 u_int *speed, const char *plugin_name);
	/**
	 * Test a key exchange implementation.
	 *
	 * @param ke			key exchange method to test
	 * @param create		constructor function for the key exchange method
	 * @param speed			speed test result, NULL to omit
	 * @return				TRUE if test passed
	 */
	bool (*test_ke)(crypto_tester_t *this, key_exchange_method_t ke,
					ke_constructor_t create,
					u_int *speed, const char *plugin_name);

	/**
	 * Add a test vector to test a crypter.
	 *
	 * @param vector		pointer to test vector
	 */
	void (*add_crypter_vector)(crypto_tester_t *this,
							   crypter_test_vector_t *vector);
	/**
	 * Add a test vector to test an aead transform.
	 *
	 * @param vector		pointer to test vector
	 */
	void (*add_aead_vector)(crypto_tester_t *this,
							aead_test_vector_t *vector);
	/**
	 * Add a test vector to test a signer.
	 *
	 * @param vector		pointer to test vector
	 */
	void (*add_signer_vector)(crypto_tester_t *this,
							  signer_test_vector_t *vector);
	/**
	 * Add a test vector to test a hasher.
	 *
	 * @param vector		pointer to test vector
	 */
	void (*add_hasher_vector)(crypto_tester_t *this,
							  hasher_test_vector_t *vector);
	/**
	 * Add a test vector to test a PRF.
	 *
	 * @param vector		pointer to test vector
	 */
	void (*add_prf_vector)(crypto_tester_t *this, prf_test_vector_t *vector);

	/**
	 * Add a test vector to test an XOF.
	 *
	 * @param vector		pointer to test vector
	 */
	void (*add_xof_vector)(crypto_tester_t *this, xof_test_vector_t *vector);

	/**
	 * Add a test vector to test a KDF.
	 *
	 * @param vector		pointer to test vector
	 */
	void (*add_kdf_vector)(crypto_tester_t *this, kdf_test_vector_t *vector);

	/**
	 * Add a test vector to test a DRBG.
	 *
	 * @param vector		pointer to test vector
	 */
	void (*add_drbg_vector)(crypto_tester_t *this, drbg_test_vector_t *vector);

	/**
	 * Add a test vector to test a RNG.
	 *
	 * @param vector		pointer to test vector
	 */
	void (*add_rng_vector)(crypto_tester_t *this, rng_test_vector_t *vector);

	/**
	 * Add a test vector to test a Diffie-Hellman backend.
	 *
	 * @param vector		pointer to test vector
	 */
	void (*add_ke_vector)(crypto_tester_t *this, ke_test_vector_t *vector);

	/**
	 * Destroy a crypto_tester_t.
	 */
	void (*destroy)(crypto_tester_t *this);
};

/**
 * Create a crypto_tester instance.
 */
crypto_tester_t *crypto_tester_create();

#endif /** CRYPTO_TESTER_H_ @}*/
