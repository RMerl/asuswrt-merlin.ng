/*
 * Copyright (C) 2008 Martin Willi
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
 * @defgroup crypto_factory crypto_factory
 * @{ @ingroup crypto
 */

#ifndef CRYPTO_FACTORY_H_
#define CRYPTO_FACTORY_H_

typedef struct crypto_factory_t crypto_factory_t;

#include <library.h>
#include <collections/enumerator.h>
#include <crypto/crypters/crypter.h>
#include <crypto/aead.h>
#include <crypto/signers/signer.h>
#include <crypto/hashers/hasher.h>
#include <crypto/prfs/prf.h>
#include <crypto/rngs/rng.h>
#include <crypto/xofs/xof.h>
#include <crypto/kdfs/kdf.h>
#include <crypto/drbgs/drbg.h>
#include <crypto/nonce_gen.h>
#include <crypto/key_exchange.h>
#include <crypto/transform.h>

#define CRYPTO_MAX_ALG_LINE          120   /* characters */

/**
 * Constructor function for crypters
 */
typedef crypter_t* (*crypter_constructor_t)(encryption_algorithm_t algo,
											size_t key_size);
/**
 * Constructor function for aead transforms
 */
typedef aead_t* (*aead_constructor_t)(encryption_algorithm_t algo,
									  size_t key_size, size_t salt_size);
/**
 * Constructor function for signers
 */
typedef signer_t* (*signer_constructor_t)(integrity_algorithm_t algo);

/**
 * Constructor function for hashers
 */
typedef hasher_t* (*hasher_constructor_t)(hash_algorithm_t algo);

/**
 * Constructor function for pseudo random functions
 */
typedef prf_t* (*prf_constructor_t)(pseudo_random_function_t algo);

/**
 * Constructor function for extended output functions
 */
typedef xof_t* (*xof_constructor_t)(ext_out_function_t algo);

/**
 * Constructor function for key derivation functions
 *
 * The additional arguments depend on the algorithm, see comments
 * for key_derivation_function_t.
 */
typedef kdf_t* (*kdf_constructor_t)(key_derivation_function_t algo, va_list args);

/**
 * Constructor function for deterministic random bit generators
 */
typedef drbg_t* (*drbg_constructor_t)(drbg_type_t type, uint32_t strength,
								rng_t *entropy, chunk_t personalization_str);

/**
 * Constructor function for source of randomness
 */
typedef rng_t* (*rng_constructor_t)(rng_quality_t quality);

/**
 * Constructor function for nonce generators
 */
typedef nonce_gen_t* (*nonce_gen_constructor_t)();

/**
 * Constructor function for key exchange methods
 *
 * The key exchange method constructor accepts additional arguments for:
 * - MODP_CUSTOM: chunk_t generator, chunk_t prime
 */
typedef key_exchange_t* (*ke_constructor_t)(key_exchange_method_t method, ...);

/**
 * Handles crypto modules and creates instances.
 */
struct crypto_factory_t {

	/**
	 * Create a crypter instance.
	 *
	 * @param algo			encryption algorithm
	 * @param key_size		length of the key in bytes
	 * @return				crypter_t instance, NULL if not supported
	 */
	crypter_t* (*create_crypter)(crypto_factory_t *this,
								 encryption_algorithm_t algo, size_t key_size);

	/**
	 * Create a aead instance.
	 *
	 * @param algo			encryption algorithm
	 * @param key_size		length of the key in bytes
	 * @param salt_size		size of salt, implicit part of the nonce
	 * @return				aead_t instance, NULL if not supported
	 */
	aead_t* (*create_aead)(crypto_factory_t *this,
						   encryption_algorithm_t algo,
						   size_t key_size, size_t salt_size);

	/**
	 * Create a symmetric signer instance.
	 *
	 * @param algo			MAC algorithm to use
	 * @return				signer_t instance, NULL if not supported
	 */
	signer_t* (*create_signer)(crypto_factory_t *this,
							   integrity_algorithm_t algo);

	/**
	 * Create a hasher instance.
	 *
	 * @param algo			hash algorithm
	 * @return				hasher_t instance, NULL if not supported
	 */
	hasher_t* (*create_hasher)(crypto_factory_t *this, hash_algorithm_t algo);

	/**
	 * Create a pseudo random function instance.
	 *
	 * @param algo			PRF algorithm to use
	 * @return				prf_t instance, NULL if not supported
	 */
	prf_t* (*create_prf)(crypto_factory_t *this, pseudo_random_function_t algo);

	/**
	 * Create an extended output function instance.
	 *
	 * @param algo			XOF algorithm to use
	 * @return				xof_t instance, NULL if not supported
	 */
	xof_t* (*create_xof)(crypto_factory_t *this, ext_out_function_t algo);


	/**
	 * Create a key derivation function instance.
	 *
	 * Additional arguments depend on the KDF, please refer to the comments in
	 * key_derivation_function_t.
	 *
	 * @param algo			KDF to create
	 * @param ...			arguments depending on algo
	 * @return				kdf_t instance, NULL if not supported
	 */
	kdf_t* (*create_kdf)(crypto_factory_t *this,
						 key_derivation_function_t algo, ...);

	/**
	 * Create a deterministic random bit generator instance.
	 *
	 * @param type					DRBG type to use
	 * @param strength				security strength in bits
	 * @param entropy				entropy source to be used (adopted)
	 * @param personalization_str	optional personalization string
	 * @return						drbg_t instance, NULL if not supported
	 */
	drbg_t* (*create_drbg)(crypto_factory_t *this, drbg_type_t type,
						   uint32_t strength, rng_t *entropy,
						   chunk_t personalization_str);

	/**
	 * Create a source of randomness.
	 *
	 * @param quality		required randomness quality
	 * @return				rng_t instance, NULL if no RNG with such a quality
	 */
	rng_t* (*create_rng)(crypto_factory_t *this, rng_quality_t quality);

	/**
	 * Create a nonce generator instance.
	 *
	 * @return				nonce_gen_t instance, NULL if not supported
	 */
	nonce_gen_t* (*create_nonce_gen)(crypto_factory_t *this);

	/**
	 * Create a key exchange method instance.
	 *
	 * Additional arguments are passed to the key exchange method constructor.
	 *
	 * @param method		key exchange method
	 * @return				key_exchange_t instance, NULL if not supported
	 */
	key_exchange_t* (*create_ke)(crypto_factory_t *this,
								 key_exchange_method_t method, ...);

	/**
	 * Register a crypter constructor.
	 *
	 * @param algo			algorithm to constructor
	 * @param key size		key size to perform benchmarking for
	 * @param plugin_name	plugin that registered this algorithm
	 * @param create		constructor function for that algorithm
	 * @return				TRUE if registered, FALSE if test vector failed
	 */
	bool (*add_crypter)(crypto_factory_t *this, encryption_algorithm_t algo,
						size_t key_size, const char *plugin_name,
						crypter_constructor_t create);

	/**
	 * Unregister a crypter constructor.
	 *
	 * @param create		constructor function to unregister
	 */
	void (*remove_crypter)(crypto_factory_t *this, crypter_constructor_t create);

	/**
	 * Unregister a aead constructor.
	 *
	 * @param create		constructor function to unregister
	 */
	void (*remove_aead)(crypto_factory_t *this, aead_constructor_t create);

	/**
	 * Register a aead constructor.
	 *
	 * @param algo			algorithm to constructor
	 * @param key size		key size to perform benchmarking for
	 * @param plugin_name	plugin that registered this algorithm
	 * @param create		constructor function for that algorithm
	 * @return				TRUE if registered, FALSE if test vector failed
	 */
	bool (*add_aead)(crypto_factory_t *this, encryption_algorithm_t algo,
					 size_t key_size, const char *plugin_name,
					 aead_constructor_t create);

	/**
	 * Register a signer constructor.
	 *
	 * @param algo			algorithm to constructor
	 * @param plugin_name	plugin that registered this algorithm
	 * @param create		constructor function for that algorithm
	 * @return				TRUE if registered, FALSE if test vector failed
	 */
	bool (*add_signer)(crypto_factory_t *this, integrity_algorithm_t algo,
					    const char *plugin_name, signer_constructor_t create);

	/**
	 * Unregister a signer constructor.
	 *
	 * @param create		constructor function to unregister
	 */
	void (*remove_signer)(crypto_factory_t *this, signer_constructor_t create);

	/**
	 * Register a hasher constructor.
	 *
	 * @param algo			algorithm to constructor
	 * @param plugin_name	plugin that registered this algorithm
	 * @param create		constructor function for that algorithm
	 * @return				TRUE if registered, FALSE if test vector failed
	 */
	bool (*add_hasher)(crypto_factory_t *this, hash_algorithm_t algo,
					   const char *plugin_name, hasher_constructor_t create);

	/**
	 * Unregister a hasher constructor.
	 *
	 * @param create		constructor function to unregister
	 */
	void (*remove_hasher)(crypto_factory_t *this, hasher_constructor_t create);

	/**
	 * Register a prf constructor.
	 *
	 * @param algo			algorithm to constructor
	 * @param plugin_name	plugin that registered this algorithm
	 * @param create		constructor function for that algorithm
	 * @return				TRUE if registered, FALSE if test vector failed
	 */
	bool (*add_prf)(crypto_factory_t *this, pseudo_random_function_t algo,
					const char *plugin_name, prf_constructor_t create);

	/**
	 * Unregister a prf constructor.
	 *
	 * @param create		constructor function to unregister
	 */
	void (*remove_prf)(crypto_factory_t *this, prf_constructor_t create);

	/**
	 * Register an xof constructor.
	 *
	 * @param algo			algorithm to constructor
	 * @param plugin_name	plugin that registered this algorithm
	 * @param create		constructor function for that algorithm
	 * @return				TRUE if registered, FALSE if test vector failed
	 */
	bool (*add_xof)(crypto_factory_t *this, ext_out_function_t algo,
					const char *plugin_name, xof_constructor_t create);

	/**
	 * Unregister an xof constructor.
	 *
	 * @param create		constructor function to unregister
	 */
	void (*remove_xof)(crypto_factory_t *this, xof_constructor_t create);

	/**
	 * Register a kdf constructor.
	 *
	 * @param algo			algorithm to constructor
	 * @param plugin_name	plugin that registered this algorithm
	 * @param create		constructor function for that algorithm
	 * @return				TRUE if registered, FALSE if test vector failed
	 */
	bool (*add_kdf)(crypto_factory_t *this, key_derivation_function_t algo,
					const char *plugin_name, kdf_constructor_t create);

	/**
	 * Unregister a kdf constructor.
	 *
	 * @param create		constructor function to unregister
	 */
	void (*remove_kdf)(crypto_factory_t *this, kdf_constructor_t create);

	/**
	 * Register a drbg constructor.
	 *
	 * @param type			type to constructor
	 * @param plugin_name	plugin that registered this algorithm
	 * @param create		constructor function for that algorithm
	 * @return				TRUE if registered, FALSE if test vector failed
	 */
	bool (*add_drbg)(crypto_factory_t *this, drbg_type_t type,
					 const char *plugin_name, drbg_constructor_t create);

	/**
	 * Unregister a drbg constructor.
	 *
	 * @param create		constructor function to unregister
	 */
	void (*remove_drbg)(crypto_factory_t *this, drbg_constructor_t create);

	/**
	 * Register a source of randomness.
	 *
	 * @param quality		quality of randomness this RNG serves
	 * @param plugin_name	plugin that registered this algorithm
	 * @param create		constructor function for such a quality
	 * @return				TRUE if registered, FALSE if test vector failed
	 */
	bool (*add_rng)(crypto_factory_t *this, rng_quality_t quality,
					const char *plugin_name, rng_constructor_t create);

	/**
	 * Unregister a source of randomness.
	 *
	 * @param create		constructor function to unregister
	 */
	void (*remove_rng)(crypto_factory_t *this, rng_constructor_t create);

	/**
	 * Register a nonce generator.
	 *
	 * @param plugin_name	plugin that registered this algorithm
	 * @param create		constructor function for that nonce generator
	 * @return				TRUE if registered, FALSE if test vector failed
	 */
	bool (*add_nonce_gen)(crypto_factory_t *this, const char *plugin_name,
						  nonce_gen_constructor_t create);

	/**
	 * Unregister a nonce generator.
	 *
	 * @param create		constructor function to unregister
	 */
	void (*remove_nonce_gen)(crypto_factory_t *this,
							 nonce_gen_constructor_t create);

	/**
	 * Register a key exchange method constructor.
	 *
	 * @param method		key exchange method to constructor
	 * @param plugin_name	plugin that registered this algorithm
	 * @param create		constructor function for that algorithm
	 * @return				TRUE if registered, FALSE if test vector failed
	 */
	bool (*add_ke)(crypto_factory_t *this, key_exchange_method_t method,
				   const char *plugin_name, ke_constructor_t create);

	/**
	 * Unregister a key exchange method constructor.
	 *
	 * @param create		constructor function to unregister
	 */
	void (*remove_ke)(crypto_factory_t *this, ke_constructor_t create);

	/**
	 * Create an enumerator over all registered crypter algorithms.
	 *
	 * @return				enumerator over encryption_algorithm_t, plugin
	 */
	enumerator_t* (*create_crypter_enumerator)(crypto_factory_t *this);

	/**
	 * Create an enumerator over all registered aead algorithms.
	 *
	 * @return				enumerator over encryption_algorithm_t, plugin
	 */
	enumerator_t* (*create_aead_enumerator)(crypto_factory_t *this);

	/**
	 * Create an enumerator over all registered signer algorithms.
	 *
	 * @return				enumerator over integrity_algorithm_t, plugin
	 */
	enumerator_t* (*create_signer_enumerator)(crypto_factory_t *this);

	/**
	 * Create an enumerator over all registered hasher algorithms.
	 *
	 * @return				enumerator over hash_algorithm_t, plugin
	 */
	enumerator_t* (*create_hasher_enumerator)(crypto_factory_t *this);

	/**
	 * Create an enumerator over all registered PRFs.
	 *
	 * @return				enumerator over pseudo_random_function_t, plugin
	 */
	enumerator_t* (*create_prf_enumerator)(crypto_factory_t *this);

	/**
	 * Create an enumerator over all registered XOFs.
	 *
	 * @return				enumerator over ext_out_function_t, plugin
	 */
	enumerator_t* (*create_xof_enumerator)(crypto_factory_t *this);

	/**
	 * Create an enumerator over all registered KDFs.
	 *
	 * @return				enumerator over key_derivation_function_t, plugin
	 */
	enumerator_t* (*create_kdf_enumerator)(crypto_factory_t *this);

	/**
	 * Create an enumerator over all registered DRBGs.
	 *
	 * @return				enumerator over drbg_type_t, plugin
	 */
	enumerator_t* (*create_drbg_enumerator)(crypto_factory_t *this);

	/**
	 * Create an enumerator over all registered key exchange method.
	 *
	 * @return				enumerator over key_exchange_method_t, plugin
	 */
	enumerator_t* (*create_ke_enumerator)(crypto_factory_t *this);

	/**
	 * Create an enumerator over all registered random generators.
	 *
	 * @return				enumerator over rng_quality_t, plugin
	 */
	enumerator_t* (*create_rng_enumerator)(crypto_factory_t *this);

	/**
	 * Create an enumerator over all registered nonce generators.
	 *
	 * @return				enumerator over plugin
	 */
	enumerator_t* (*create_nonce_gen_enumerator)(crypto_factory_t *this);

	/**
	 * Add a test vector to the crypto factory.
	 *
	 * @param type			type of the test vector
	 * @param vector		pointer to a test vector, defined in crypto_tester.h
	 */
	void (*add_test_vector)(crypto_factory_t *this, transform_type_t type,
							void *vector);

	/**
	 * Create an enumerator verifying transforms using known test vectors.
	 *
	 * The resulting enumerator enumerates over an u_int with the type
	 * specific transform identifier, the plugin name providing the transform,
	 * and a boolean value indicating success/failure for the given transform.
	 *
	 * @param type			transform type to test
	 * @return				enumerator over (u_int, char*, bool)
	 */
	enumerator_t* (*create_verify_enumerator)(crypto_factory_t *this,
											  transform_type_t type);

	/**
	 * Destroy a crypto_factory instance.
	 */
	void (*destroy)(crypto_factory_t *this);
};

/**
 * Create a crypto_factory instance.
 */
crypto_factory_t *crypto_factory_create();

#endif /** CRYPTO_FACTORY_H_ @}*/
