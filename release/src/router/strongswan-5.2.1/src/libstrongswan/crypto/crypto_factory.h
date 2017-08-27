/*
 * Copyright (C) 2008 Martin Willi
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
#include <crypto/nonce_gen.h>
#include <crypto/diffie_hellman.h>
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
 * Constructor function for source of randomness
 */
typedef rng_t* (*rng_constructor_t)(rng_quality_t quality);

/**
 * Constructor function for nonce generators
 */
typedef nonce_gen_t* (*nonce_gen_constructor_t)();

/**
 * Constructor function for diffie hellman
 *
 * The DH constructor accepts additional arguments for:
 * - MODP_CUSTOM: chunk_t generator, chunk_t prime
 */
typedef diffie_hellman_t* (*dh_constructor_t)(diffie_hellman_group_t group, ...);

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
	 * Create a diffie hellman instance.
	 *
	 * Additional arguments are passed to the DH constructor.
	 *
	 * @param group			diffie hellman group
	 * @return				diffie_hellman_t instance, NULL if not supported
	 */
	diffie_hellman_t* (*create_dh)(crypto_factory_t *this,
								   diffie_hellman_group_t group, ...);

	/**
	 * Register a crypter constructor.
	 *
	 * @param algo			algorithm to constructor
	 * @param plugin_name	plugin that registered this algorithm
	 * @param create		constructor function for that algorithm
	 * @return				TRUE if registered, FALSE if test vector failed
	 */
	bool (*add_crypter)(crypto_factory_t *this, encryption_algorithm_t algo,
						const char *plugin_name, crypter_constructor_t create);

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
	 * @param plugin_name	plugin that registered this algorithm
	 * @param create		constructor function for that algorithm
	 * @return				TRUE if registered, FALSE if test vector failed
	 */
	bool (*add_aead)(crypto_factory_t *this, encryption_algorithm_t algo,
					 const char *plugin_name, aead_constructor_t create);

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
	 * Register a diffie hellman constructor.
	 *
	 * @param group			dh group to constructor
	 * @param plugin_name	plugin that registered this algorithm
	 * @param create		constructor function for that algorithm
	 * @return				TRUE if registered, FALSE if test vector failed
	 */
	bool (*add_dh)(crypto_factory_t *this, diffie_hellman_group_t group,
				   const char *plugin_name, dh_constructor_t create);

	/**
	 * Unregister a diffie hellman constructor.
	 *
	 * @param create		constructor function to unregister
	 */
	void (*remove_dh)(crypto_factory_t *this, dh_constructor_t create);

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
	 * Create an enumerator over all registered diffie hellman groups.
	 *
	 * @return				enumerator over diffie_hellman_group_t, plugin
	 */
	enumerator_t* (*create_dh_enumerator)(crypto_factory_t *this);

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
	 * Get the number of test vector failures encountered during add.
	 *
	 * This counter gets incremented only if transforms get tested during
	 * registration.
	 *
	 * @return				number of failed test vectors
	 */
	u_int (*get_test_vector_failures)(crypto_factory_t *this);

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
