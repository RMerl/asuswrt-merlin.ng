/*
 * Copyright (C) 2009 Martin Willi
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

#include "test_vectors_plugin.h"

#include <crypto/crypto_factory.h>
#include <crypto/crypto_tester.h>

/* define symbols of all test vectors */
#define TEST_VECTOR_CRYPTER(x) extern crypter_test_vector_t x;
#define TEST_VECTOR_AEAD(x) extern aead_test_vector_t x;
#define TEST_VECTOR_SIGNER(x) extern signer_test_vector_t x;
#define TEST_VECTOR_HASHER(x) extern hasher_test_vector_t x;
#define TEST_VECTOR_PRF(x) extern prf_test_vector_t x;
#define TEST_VECTOR_XOF(x) extern xof_test_vector_t x;
#define TEST_VECTOR_RNG(x) extern rng_test_vector_t x;
#define TEST_VECTOR_DH(x) extern dh_test_vector_t x;

#include "test_vectors.h"

#undef TEST_VECTOR_CRYPTER
#undef TEST_VECTOR_AEAD
#undef TEST_VECTOR_SIGNER
#undef TEST_VECTOR_HASHER
#undef TEST_VECTOR_PRF
#undef TEST_VECTOR_XOF
#undef TEST_VECTOR_RNG
#undef TEST_VECTOR_DH

#define TEST_VECTOR_CRYPTER(x)
#define TEST_VECTOR_AEAD(x)
#define TEST_VECTOR_SIGNER(x)
#define TEST_VECTOR_HASHER(x)
#define TEST_VECTOR_PRF(x)
#define TEST_VECTOR_XOF(x)
#define TEST_VECTOR_RNG(x)
#define TEST_VECTOR_DH(x)

/* create test vector arrays */
#undef TEST_VECTOR_CRYPTER
#define TEST_VECTOR_CRYPTER(x) &x,
static crypter_test_vector_t *crypter[] = {
#include "test_vectors.h"
};
#undef TEST_VECTOR_CRYPTER
#define TEST_VECTOR_CRYPTER(x)

#undef TEST_VECTOR_AEAD
#define TEST_VECTOR_AEAD(x) &x,
static aead_test_vector_t *aead[] = {
#include "test_vectors.h"
};
#undef TEST_VECTOR_AEAD
#define TEST_VECTOR_AEAD(x)

#undef TEST_VECTOR_SIGNER
#define TEST_VECTOR_SIGNER(x) &x,
static signer_test_vector_t *signer[] = {
#include "test_vectors.h"
};
#undef TEST_VECTOR_SIGNER
#define TEST_VECTOR_SIGNER(x)

#undef TEST_VECTOR_HASHER
#define TEST_VECTOR_HASHER(x) &x,
static hasher_test_vector_t *hasher[] = {
#include "test_vectors.h"
};
#undef TEST_VECTOR_HASHER
#define TEST_VECTOR_HASHER(x)

#undef TEST_VECTOR_PRF
#define TEST_VECTOR_PRF(x) &x,
static prf_test_vector_t *prf[] = {
#include "test_vectors.h"
};
#undef TEST_VECTOR_PRF
#define TEST_VECTOR_PRF(x)

#undef TEST_VECTOR_XOF
#define TEST_VECTOR_XOF(x) &x,
static xof_test_vector_t *xof[] = {
#include "test_vectors.h"
};
#undef TEST_VECTOR_XOF
#define TEST_VECTOR_XOF(x)

#undef TEST_VECTOR_RNG
#define TEST_VECTOR_RNG(x) &x,
static rng_test_vector_t *rng[] = {
#include "test_vectors.h"
};
#undef TEST_VECTOR_RNG
#define TEST_VECTOR_RNG(x)

#undef TEST_VECTOR_DH
#define TEST_VECTOR_DH(x) &x,
static dh_test_vector_t *dh[] = {
#include "test_vectors.h"
};
#undef TEST_VECTOR_DH
#define TEST_VECTOR_DH(x)

typedef struct private_test_vectors_plugin_t private_test_vectors_plugin_t;

/**
 * private data of test_vectors_plugin
 */
struct private_test_vectors_plugin_t {

	/**
	 * public functions
	 */
	test_vectors_plugin_t public;
};

METHOD(plugin_t, get_name, char*,
	private_test_vectors_plugin_t *this)
{
	return "test-vectors";
}

METHOD(plugin_t, get_features, int,
	private_test_vectors_plugin_t *this, plugin_feature_t *features[])
{
	static plugin_feature_t f[] = {
		PLUGIN_NOOP,
			PLUGIN_PROVIDE(CUSTOM, "test-vectors"),
	};
	*features = f;
	return countof(f);
}

METHOD(plugin_t, destroy, void,
	private_test_vectors_plugin_t *this)
{
	free(this);
}

/*
 * see header file
 */
plugin_t *test_vectors_plugin_create()
{
	private_test_vectors_plugin_t *this;
	int i;

	INIT(this,
		.public = {
			.plugin = {
				.get_name = _get_name,
				.get_features = _get_features,
				.destroy = _destroy,
			},
		},
	);

	for (i = 0; i < countof(crypter); i++)
	{
		lib->crypto->add_test_vector(lib->crypto,
									 ENCRYPTION_ALGORITHM, crypter[i]);
	}
	for (i = 0; i < countof(aead); i++)
	{
		lib->crypto->add_test_vector(lib->crypto,
									 AEAD_ALGORITHM, aead[i]);
	}
	for (i = 0; i < countof(signer); i++)
	{
		lib->crypto->add_test_vector(lib->crypto,
									 INTEGRITY_ALGORITHM, signer[i]);
	}
	for (i = 0; i < countof(hasher); i++)
	{
		lib->crypto->add_test_vector(lib->crypto,
									 HASH_ALGORITHM, hasher[i]);
	}
	for (i = 0; i < countof(prf); i++)
	{
		lib->crypto->add_test_vector(lib->crypto,
									 PSEUDO_RANDOM_FUNCTION, prf[i]);
	}
	for (i = 0; i < countof(xof); i++)
	{
		lib->crypto->add_test_vector(lib->crypto,
									 EXTENDED_OUTPUT_FUNCTION, xof[i]);
	}
	for (i = 0; i < countof(rng); i++)
	{
		lib->crypto->add_test_vector(lib->crypto,
									 RANDOM_NUMBER_GENERATOR, rng[i]);
	}
	for (i = 0; i < countof(dh); i++)
	{
		lib->crypto->add_test_vector(lib->crypto,
									 DIFFIE_HELLMAN_GROUP, dh[i]);
	}

	return &this->public.plugin;
}
