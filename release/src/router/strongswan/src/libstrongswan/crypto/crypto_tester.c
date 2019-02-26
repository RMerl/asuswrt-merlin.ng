/*
 * Copyright (C) 2009-2010 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
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

#ifdef HAVE_DLADDR
# define _GNU_SOURCE
# include <dlfcn.h>
#endif
#include <time.h>

#include "crypto_tester.h"

#include <utils/debug.h>
#include <collections/linked_list.h>

typedef struct private_crypto_tester_t private_crypto_tester_t;

/**
 * Private data of an crypto_tester_t object.
 */
struct private_crypto_tester_t {

	/**
	 * Public crypto_tester_t interface.
	 */
	crypto_tester_t public;

	/**
	 * List of crypter test vectors
	 */
	linked_list_t *crypter;

	/**
	 * List of aead test vectors
	 */
	linked_list_t *aead;

	/**
	 * List of signer test vectors
	 */
	linked_list_t *signer;

	/**
	 * List of hasher test vectors
	 */
	linked_list_t *hasher;

	/**
	 * List of PRF test vectors
	 */
	linked_list_t *prf;

	/**
	 * List of XOF test vectors
	 */
	linked_list_t *xof;

	/**
	 * List of RNG test vectors
	 */
	linked_list_t *rng;

	/**
	 * List of Diffie-Hellman test vectors
	 */
	linked_list_t *dh;

	/**
	 * Is a test vector required to pass a test?
	 */
	bool required;

	/**
	 * should we run RNG_TRUE tests? Enough entropy?
	 */
	bool rng_true;

	/**
	 * time we test each algorithm
	 */
	int bench_time;

	/**
	 * size of buffer we use for benchmarking
	 */
	int bench_size;
};

/**
 * Get the name of a test vector, if available
 */
static const char* get_name(void *sym)
{
#ifdef HAVE_DLADDR
	Dl_info dli;

	if (dladdr(sym, &dli))
	{
		return dli.dli_sname;
	}
#endif
	return "unknown";
}

#if defined(CLOCK_THREAD_CPUTIME_ID) && defined(HAVE_CLOCK_GETTIME)

/**
 * Start a benchmark timer
 */
static void start_timing(struct timespec *start)
{
	clock_gettime(CLOCK_THREAD_CPUTIME_ID, start);
}

/**
 * End a benchmark timer, return ms
 */
static u_int end_timing(struct timespec *start)
{
	struct timespec end;

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);
	return (end.tv_nsec - start->tv_nsec) / 1000000 +
			(end.tv_sec - start->tv_sec) * 1000;
}

#else /* CLOCK_THREAD_CPUTIME_ID */

/* Make benchmarking a no-op if CLOCK_THREAD_CPUTIME_ID is not available */
#define start_timing(start) ((start)->tv_sec = 0, (start)->tv_nsec = 0)
#define end_timing(...) (this->bench_time)

#endif /* CLOCK_THREAD_CPUTIME_ID */

/**
 * Benchmark a crypter
 */
static u_int bench_crypter(private_crypto_tester_t *this,
	encryption_algorithm_t alg, crypter_constructor_t create, size_t key_size)
{
	crypter_t *crypter;

	crypter = create(alg, key_size);
	if (crypter)
	{
		char iv[crypter->get_iv_size(crypter)];
		char key[crypter->get_key_size(crypter)];
		chunk_t buf;
		struct timespec start;
		u_int runs;

		memset(iv, 0x56, sizeof(iv));
		memset(key, 0x12, sizeof(key));
		if (!crypter->set_key(crypter, chunk_from_thing(key)))
		{
			return 0;
		}

		buf = chunk_alloc(this->bench_size);
		memset(buf.ptr, 0x34, buf.len);

		runs = 0;
		start_timing(&start);
		while (end_timing(&start) < this->bench_time)
		{
			if (crypter->encrypt(crypter, buf, chunk_from_thing(iv), NULL))
			{
				runs++;
			}
			if (crypter->decrypt(crypter, buf, chunk_from_thing(iv), NULL))
			{
				runs++;
			}
		}
		free(buf.ptr);
		crypter->destroy(crypter);

		return runs;
	}
	return 0;
}

METHOD(crypto_tester_t, test_crypter, bool,
	private_crypto_tester_t *this, encryption_algorithm_t alg, size_t key_size,
	crypter_constructor_t create, u_int *speed, const char *plugin_name)
{
	enumerator_t *enumerator;
	crypter_test_vector_t *vector;
	bool failed = FALSE;
	u_int tested = 0;

	enumerator = this->crypter->create_enumerator(this->crypter);
	while (enumerator->enumerate(enumerator, &vector))
	{
		crypter_t *crypter;
		chunk_t key, iv, plain = chunk_empty, cipher = chunk_empty;

		if (vector->alg != alg)
		{
			continue;
		}
		if (key_size && key_size != vector->key_size)
		{	/* test only vectors with a specific key size, if key size given */
			continue;
		}

		crypter = create(alg, vector->key_size);
		if (!crypter)
		{	/* key size not supported */
			continue;
		}
		tested++;
		failed = TRUE;

		key = chunk_create(vector->key, crypter->get_key_size(crypter));
		if (!crypter->set_key(crypter, key))
		{
			goto failure;
		}
		iv = chunk_create(vector->iv, crypter->get_iv_size(crypter));

		/* allocated encryption */
		plain = chunk_create(vector->plain, vector->len);
		if (!crypter->encrypt(crypter, plain, iv, &cipher))
		{
			goto failure;
		}
		if (!memeq(vector->cipher, cipher.ptr, cipher.len))
		{
			goto failure;
		}
		/* inline decryption */
		if (!crypter->decrypt(crypter, cipher, iv, NULL))
		{
			goto failure;
		}
		if (!memeq(vector->plain, cipher.ptr, cipher.len))
		{
			goto failure;
		}
		/* allocated decryption */
		if (!crypter->decrypt(crypter,
						chunk_create(vector->cipher, vector->len), iv, &plain))
		{
			goto failure;
		}
		if (!memeq(vector->plain, plain.ptr, plain.len))
		{
			goto failure;
		}
		/* inline encryption */
		if (!crypter->encrypt(crypter, plain, iv, NULL))
		{
			goto failure;
		}
		if (!memeq(vector->cipher, plain.ptr, plain.len))
		{
			goto failure;
		}

		failed = FALSE;
failure:
		crypter->destroy(crypter);
		chunk_free(&cipher);
		if (plain.ptr != vector->plain)
		{
			chunk_free(&plain);
		}
		if (failed)
		{
			DBG1(DBG_LIB, "disabled %N[%s]: %s test vector failed",
				 encryption_algorithm_names, alg, plugin_name, get_name(vector));
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!tested)
	{
		if (failed)
		{
			DBG1(DBG_LIB,"disable %N[%s]: %zd byte key size not supported",
				 encryption_algorithm_names, alg, plugin_name, key_size);
			return FALSE;
		}
		else
		{
			DBG1(DBG_LIB, "%s %N[%s]: no test vectors found",
				 this->required ? "disabled" : "enabled ",
				 encryption_algorithm_names, alg, plugin_name);
			return !this->required;
		}
	}
	if (!failed)
	{
		if (speed)
		{
			*speed = bench_crypter(this, alg, create, key_size);
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors, %d points "
				 "(%zd bit key)", encryption_algorithm_names, alg,
				 plugin_name, tested, *speed, key_size * 8);
		}
		else
		{
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors",
				 encryption_algorithm_names, alg, plugin_name, tested);
		}
	}
	return !failed;
}

/**
 * Benchmark an aead transform
 */
static u_int bench_aead(private_crypto_tester_t *this,
	encryption_algorithm_t alg, aead_constructor_t create, size_t key_size)
{
	aead_t *aead;

	aead = create(alg, key_size, 0);
	if (aead)
	{
		char iv[aead->get_iv_size(aead)];
		char key[aead->get_key_size(aead)];
		char assoc[4];
		chunk_t buf;
		struct timespec start;
		u_int runs;
		size_t icv;

		memset(iv, 0x56, sizeof(iv));
		memset(key, 0x12, sizeof(key));
		memset(assoc, 0x78, sizeof(assoc));
		if (!aead->set_key(aead, chunk_from_thing(key)))
		{
			return 0;
		}
		icv = aead->get_icv_size(aead);

		buf = chunk_alloc(this->bench_size + icv);
		memset(buf.ptr, 0x34, buf.len);
		buf.len -= icv;

		runs = 0;
		start_timing(&start);
		while (end_timing(&start) < this->bench_time)
		{
			if (aead->encrypt(aead, buf, chunk_from_thing(assoc),
						chunk_from_thing(iv), NULL))
			{
				runs += 2;
			}
			if (aead->decrypt(aead, chunk_create(buf.ptr, buf.len + icv),
						chunk_from_thing(assoc), chunk_from_thing(iv), NULL))
			{
				runs += 2;
			}
		}
		free(buf.ptr);
		aead->destroy(aead);

		return runs;
	}
	return 0;
}

METHOD(crypto_tester_t, test_aead, bool,
	private_crypto_tester_t *this, encryption_algorithm_t alg, size_t key_size,
	size_t salt_size, aead_constructor_t create,
	u_int *speed, const char *plugin_name)
{
	enumerator_t *enumerator;
	aead_test_vector_t *vector;
	bool failed = FALSE;
	u_int tested = 0;

	enumerator = this->aead->create_enumerator(this->aead);
	while (enumerator->enumerate(enumerator, &vector))
	{
		aead_t *aead;
		chunk_t key, iv, assoc, plain = chunk_empty, cipher = chunk_empty;
		size_t icv;

		if (vector->alg != alg)
		{
			continue;
		}
		if (key_size && key_size != vector->key_size)
		{	/* test only vectors with a specific key size, if key size given */
			continue;
		}
		if (salt_size && salt_size != vector->salt_size)
		{
			continue;
		}

		tested++;
		failed = TRUE;
		aead = create(alg, vector->key_size, vector->salt_size);
		if (!aead)
		{
			DBG1(DBG_LIB, "%N[%s]: %u bit key size not supported",
				 encryption_algorithm_names, alg, plugin_name,
				 BITS_PER_BYTE * vector->key_size);
			continue;
		}

		key = chunk_create(vector->key, aead->get_key_size(aead));
		if (!aead->set_key(aead, key))
		{
			goto failure;
		}
		iv = chunk_create(vector->iv, aead->get_iv_size(aead));
		assoc = chunk_create(vector->adata, vector->alen);
		icv = aead->get_icv_size(aead);

		/* allocated encryption */
		plain = chunk_create(vector->plain, vector->len);
		if (!aead->encrypt(aead, plain, assoc, iv, &cipher))
		{
			goto failure;
		}
		if (!memeq(vector->cipher, cipher.ptr, cipher.len))
		{
			goto failure;
		}
		/* inline decryption */
		if (!aead->decrypt(aead, cipher, assoc, iv, NULL))
		{
			goto failure;
		}
		if (!memeq(vector->plain, cipher.ptr, cipher.len - icv))
		{
			goto failure;
		}
		/* allocated decryption */
		if (!aead->decrypt(aead, chunk_create(vector->cipher, vector->len + icv),
						   assoc, iv, &plain))
		{
			goto failure;
		}
		if (!memeq(vector->plain, plain.ptr, plain.len))
		{
			goto failure;
		}
		plain.ptr = realloc(plain.ptr, plain.len + icv);
		/* inline encryption */
		if (!aead->encrypt(aead, plain, assoc, iv, NULL))
		{
			goto failure;
		}
		if (!memeq(vector->cipher, plain.ptr, plain.len + icv))
		{
			goto failure;
		}

		failed = FALSE;
failure:
		aead->destroy(aead);
		chunk_free(&cipher);
		if (plain.ptr != vector->plain)
		{
			chunk_free(&plain);
		}
		if (failed)
		{
			DBG1(DBG_LIB, "disabled %N[%s]: %s test vector failed",
				 encryption_algorithm_names, alg, plugin_name, get_name(vector));
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!tested)
	{
		if (failed)
		{
			DBG1(DBG_LIB,"disable %N[%s]: %zd byte key size not supported",
				 encryption_algorithm_names, alg, plugin_name, key_size);
			return FALSE;
		}
		else
		{
			DBG1(DBG_LIB, "%s %N[%s]: no test vectors found",
				 this->required ? "disabled" : "enabled ",
				 encryption_algorithm_names, alg, plugin_name);
			return !this->required;
		}
	}
	if (!failed)
	{
		if (speed)
		{
			*speed = bench_aead(this, alg, create, key_size);
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors, %d points "
				 "(%zd bit key)", encryption_algorithm_names, alg,
				 plugin_name, tested, *speed, key_size * 8);
		}
		else
		{
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors",
				 encryption_algorithm_names, alg, plugin_name, tested);
		}
	}
	return !failed;
}

/**
 * Benchmark a signer
 */
static u_int bench_signer(private_crypto_tester_t *this,
	integrity_algorithm_t alg, signer_constructor_t create)
{
	signer_t *signer;

	signer = create(alg);
	if (signer)
	{
		char key[signer->get_key_size(signer)];
		char mac[signer->get_block_size(signer)];
		chunk_t buf;
		struct timespec start;
		u_int runs;

		memset(key, 0x12, sizeof(key));
		if (!signer->set_key(signer, chunk_from_thing(key)))
		{
			return 0;
		}

		buf = chunk_alloc(this->bench_size);
		memset(buf.ptr, 0x34, buf.len);

		runs = 0;
		start_timing(&start);
		while (end_timing(&start) < this->bench_time)
		{
			if (signer->get_signature(signer, buf, mac))
			{
				runs++;
			}
			if (signer->verify_signature(signer, buf, chunk_from_thing(mac)))
			{
				runs++;
			}
		}
		free(buf.ptr);
		signer->destroy(signer);

		return runs;
	}
	return 0;
}

METHOD(crypto_tester_t, test_signer, bool,
	private_crypto_tester_t *this, integrity_algorithm_t alg,
	signer_constructor_t create, u_int *speed, const char *plugin_name)
{
	enumerator_t *enumerator;
	signer_test_vector_t *vector;
	bool failed = FALSE;
	u_int tested = 0;

	enumerator = this->signer->create_enumerator(this->signer);
	while (enumerator->enumerate(enumerator, &vector))
	{
		signer_t *signer;
		chunk_t key, data, mac = chunk_empty;

		if (vector->alg != alg)
		{
			continue;
		}

		tested++;
		failed = TRUE;
		signer = create(alg);
		if (!signer)
		{
			DBG1(DBG_LIB, "disabled %N[%s]: creating instance failed",
				 integrity_algorithm_names, alg, plugin_name);
			break;
		}

		data = chunk_create(vector->data, vector->len);
		key = chunk_create(vector->key, signer->get_key_size(signer));
		if (!signer->set_key(signer, key))
		{
			goto failure;
		}
		/* do partial append mode and check if key gets set correctly */
		if (!signer->get_signature(signer, data, NULL))
		{
			goto failure;
		}
		if (!signer->set_key(signer, key))
		{
			goto failure;
		}
		/* allocated signature */
		if (!signer->allocate_signature(signer, data, &mac))
		{
			goto failure;
		}
		if (mac.len != signer->get_block_size(signer))
		{
			goto failure;
		}
		if (!memeq(vector->mac, mac.ptr, mac.len))
		{
			goto failure;
		}
		/* signature to existing buffer */
		memset(mac.ptr, 0, mac.len);
		if (!signer->get_signature(signer, data, mac.ptr))
		{
			goto failure;
		}
		if (!memeq(vector->mac, mac.ptr, mac.len))
		{
			goto failure;
		}
		/* signature verification, good case */
		if (!signer->verify_signature(signer, data, mac))
		{
			goto failure;
		}
		/* signature verification, bad case */
		*(mac.ptr + mac.len - 1) += 1;
		if (signer->verify_signature(signer, data, mac))
		{
			goto failure;
		}
		/* signature to existing buffer, using append mode */
		if (data.len > 2)
		{
			if (!signer->allocate_signature(signer,
											chunk_create(data.ptr, 1), NULL))
			{
				goto failure;
			}
			if (!signer->get_signature(signer,
									   chunk_create(data.ptr + 1, 1), NULL))
			{
				goto failure;
			}
			if (!signer->verify_signature(signer, chunk_skip(data, 2),
										  chunk_create(vector->mac, mac.len)))
			{
				goto failure;
			}
		}

		failed = FALSE;
failure:
		signer->destroy(signer);
		chunk_free(&mac);
		if (failed)
		{
			DBG1(DBG_LIB, "disabled %N[%s]: %s test vector failed",
				 integrity_algorithm_names, alg, plugin_name, get_name(vector));
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!tested)
	{
		DBG1(DBG_LIB, "%s %N[%s]: no test vectors found",
			 this->required ? "disabled" : "enabled ",
			 integrity_algorithm_names, alg, plugin_name);
		return !this->required;
	}
	if (!failed)
	{
		if (speed)
		{
			*speed = bench_signer(this, alg, create);
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors, %d points",
				 integrity_algorithm_names, alg, plugin_name, tested, *speed);
		}
		else
		{
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors",
				 integrity_algorithm_names, alg, plugin_name, tested);
		}
	}
	return !failed;
}

/**
 * Benchmark a hasher
 */
static u_int bench_hasher(private_crypto_tester_t *this,
	hash_algorithm_t alg, hasher_constructor_t create)
{
	hasher_t *hasher;

	hasher = create(alg);
	if (hasher)
	{
		char hash[hasher->get_hash_size(hasher)];
		chunk_t buf;
		struct timespec start;
		u_int runs;

		buf = chunk_alloc(this->bench_size);
		memset(buf.ptr, 0x34, buf.len);

		runs = 0;
		start_timing(&start);
		while (end_timing(&start) < this->bench_time)
		{
			if (hasher->get_hash(hasher, buf, hash))
			{
				runs++;
			}
		}
		free(buf.ptr);
		hasher->destroy(hasher);

		return runs;
	}
	return 0;
}

METHOD(crypto_tester_t, test_hasher, bool,
	private_crypto_tester_t *this, hash_algorithm_t alg,
	hasher_constructor_t create, u_int *speed, const char *plugin_name)
{
	enumerator_t *enumerator;
	hasher_test_vector_t *vector;
	bool failed = FALSE;
	u_int tested = 0;

	enumerator = this->hasher->create_enumerator(this->hasher);
	while (enumerator->enumerate(enumerator, &vector))
	{
		hasher_t *hasher;
		chunk_t data, hash;

		if (vector->alg != alg)
		{
			continue;
		}

		tested++;
		failed = TRUE;
		hasher = create(alg);
		if (!hasher)
		{
			DBG1(DBG_LIB, "disabled %N[%s]: creating instance failed",
				 hash_algorithm_names, alg, plugin_name);
			break;
		}

		/* allocated hash */
		data = chunk_create(vector->data, vector->len);
		if (!hasher->allocate_hash(hasher, data, &hash))
		{
			goto failure;
		}
		if (hash.len != hasher->get_hash_size(hasher))
		{
			goto failure;
		}
		if (!memeq(vector->hash, hash.ptr, hash.len))
		{
			goto failure;
		}
		/* hash to existing buffer, with a reset */
		memset(hash.ptr, 0, hash.len);
		if (!hasher->get_hash(hasher, data, NULL))
		{
			goto failure;
		}
		if (!hasher->reset(hasher))
		{
			goto failure;
		}
		if (!hasher->get_hash(hasher, data, hash.ptr))
		{
			goto failure;
		}
		if (!memeq(vector->hash, hash.ptr, hash.len))
		{
			goto failure;
		}
		/* hasher to existing buffer, using append mode */
		if (data.len > 2)
		{
			memset(hash.ptr, 0, hash.len);
			if (!hasher->allocate_hash(hasher, chunk_create(data.ptr, 1), NULL))
			{
				goto failure;
			}
			if (!hasher->get_hash(hasher, chunk_create(data.ptr + 1, 1), NULL))
			{
				goto failure;
			}
			if (!hasher->get_hash(hasher, chunk_skip(data, 2), hash.ptr))
			{
				goto failure;
			}
			if (!memeq(vector->hash, hash.ptr, hash.len))
			{
				goto failure;
			}
		}

		failed = FALSE;
failure:
		hasher->destroy(hasher);
		chunk_free(&hash);
		if (failed)
		{
			DBG1(DBG_LIB, "disabled %N[%s]: %s test vector failed",
				 hash_algorithm_names, alg, plugin_name, get_name(vector));
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!tested)
	{
		DBG1(DBG_LIB, "%s %N[%s]: no test vectors found",
			 this->required ? "disabled" : "enabled ",
			 hash_algorithm_names, alg, plugin_name);
		return !this->required;
	}
	if (!failed)
	{
		if (speed)
		{
			*speed = bench_hasher(this, alg, create);
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors, %d points",
				 hash_algorithm_names, alg, plugin_name, tested, *speed);
		}
		else
		{
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors",
				 hash_algorithm_names, alg, plugin_name, tested);
		}
	}
	return !failed;
}

/**
 * Benchmark a PRF
 */
static u_int bench_prf(private_crypto_tester_t *this,
					   pseudo_random_function_t alg, prf_constructor_t create)
{
	prf_t *prf;

	prf = create(alg);
	if (prf)
	{
		char bytes[prf->get_block_size(prf)], key[prf->get_block_size(prf)];
		chunk_t buf;
		struct timespec start;
		u_int runs;

		memset(key, 0x56, prf->get_block_size(prf));
		if (!prf->set_key(prf, chunk_create(key, prf->get_block_size(prf))))
		{
			prf->destroy(prf);
			return 0;
		}

		buf = chunk_alloc(this->bench_size);
		memset(buf.ptr, 0x34, buf.len);

		runs = 0;
		start_timing(&start);
		while (end_timing(&start) < this->bench_time)
		{
			if (prf->get_bytes(prf, buf, bytes))
			{
				runs++;
			}
		}
		free(buf.ptr);
		prf->destroy(prf);

		return runs;
	}
	return 0;
}

METHOD(crypto_tester_t, test_prf, bool,
	private_crypto_tester_t *this, pseudo_random_function_t alg,
	prf_constructor_t create, u_int *speed, const char *plugin_name)
{
	enumerator_t *enumerator;
	prf_test_vector_t *vector;
	bool failed = FALSE;
	u_int tested = 0;

	enumerator = this->prf->create_enumerator(this->prf);
	while (enumerator->enumerate(enumerator, &vector))
	{
		prf_t *prf;
		chunk_t key, seed, out = chunk_empty;

		if (vector->alg != alg)
		{
			continue;
		}

		tested++;
		failed = TRUE;
		prf = create(alg);
		if (!prf)
		{
			DBG1(DBG_LIB, "disabled %N[%s]: creating instance failed",
				 pseudo_random_function_names, alg, plugin_name);
			break;
		}

		seed = chunk_create(vector->seed, vector->len);
		key = chunk_create(vector->key, vector->key_size);
		if (!prf->set_key(prf, key))
		{
			goto failure;
		}
		if (alg != PRF_FIPS_SHA1_160)
		{
			/* do partial append mode and check if key gets set correctly */
			if (!prf->get_bytes(prf, seed, NULL))
			{
				goto failure;
			}
			if (!prf->set_key(prf, key))
			{
				goto failure;
			}
		}
		/* allocated bytes */
		if (!prf->allocate_bytes(prf, seed, &out))
		{
			goto failure;
		}
		if (out.len != prf->get_block_size(prf))
		{
			goto failure;
		}
		if (!memeq(vector->out, out.ptr, out.len))
		{
			goto failure;
		}
		/* bytes to existing buffer */
		memset(out.ptr, 0, out.len);
		if (vector->stateful)
		{
			if (!prf->set_key(prf, key))
			{
				goto failure;
			}
		}
		if (!prf->get_bytes(prf, seed, out.ptr))
		{
			goto failure;
		}
		if (!memeq(vector->out, out.ptr, out.len))
		{
			goto failure;
		}
		/* bytes to existing buffer, using append mode */
		if (alg != PRF_FIPS_SHA1_160 && seed.len > 2)
		{
			memset(out.ptr, 0, out.len);
			if (vector->stateful)
			{
				if (!prf->set_key(prf, key))
				{
					goto failure;
				}
			}
			if (!prf->allocate_bytes(prf, chunk_create(seed.ptr, 1), NULL))
			{
				goto failure;
			}
			if (!prf->get_bytes(prf, chunk_create(seed.ptr + 1, 1), NULL))
			{
				goto failure;
			}
			if (!prf->get_bytes(prf, chunk_skip(seed, 2), out.ptr))
			{
				goto failure;
			}
			if (!memeq(vector->out, out.ptr, out.len))
			{
				goto failure;
			}
		}

		failed = FALSE;
failure:
		prf->destroy(prf);
		chunk_free(&out);
		if (failed)
		{
			DBG1(DBG_LIB, "disabled %N[%s]: %s test vector failed",
				 pseudo_random_function_names, alg, plugin_name, get_name(vector));
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!tested)
	{
		DBG1(DBG_LIB, "%s %N[%s]: no test vectors found",
			 this->required ? "disabled" : "enabled ",
			 pseudo_random_function_names, alg, plugin_name);
		return !this->required;
	}
	if (!failed)
	{
		if (speed)
		{
			*speed = bench_prf(this, alg, create);
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors, %d points",
				 pseudo_random_function_names, alg, plugin_name, tested, *speed);
		}
		else
		{
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors",
				 pseudo_random_function_names, alg, plugin_name, tested);
		}
	}
	return !failed;
}

/**
 * Benchmark an XOF
 */
static u_int bench_xof(private_crypto_tester_t *this,
					   ext_out_function_t alg, xof_constructor_t create)
{
	xof_t *xof;

	xof = create(alg);
	if (xof)
	{
		char seed[xof->get_seed_size(xof)];
		char bytes[xof->get_block_size(xof)];
		struct timespec start;
		u_int runs;

		memset(seed, 0x56, xof->get_seed_size(xof));
		if (!xof->set_seed(xof, chunk_create(seed, xof->get_seed_size(xof))))
		{
			xof->destroy(xof);
			return 0;
		}

		runs = 0;
		start_timing(&start);
		while (end_timing(&start) < this->bench_time)
		{
			if (xof->get_bytes(xof, xof->get_block_size(xof), bytes))
			{
				runs++;
			}
		}
		xof->destroy(xof);

		return runs;
	}
	return 0;
}

METHOD(crypto_tester_t, test_xof, bool,
	private_crypto_tester_t *this, ext_out_function_t alg,
	xof_constructor_t create, u_int *speed, const char *plugin_name)
{
	enumerator_t *enumerator;
	xof_test_vector_t *vector;
	bool failed = FALSE;
	u_int tested = 0;

	enumerator = this->xof->create_enumerator(this->xof);
	while (enumerator->enumerate(enumerator, &vector))
	{
		xof_t *xof;
		chunk_t seed, out = chunk_empty;

		if (vector->alg != alg)
		{
			continue;
		}

		tested++;
		failed = TRUE;
		xof = create(alg);
		if (!xof)
		{
			DBG1(DBG_LIB, "disabled %N[%s]: creating instance failed",
				 ext_out_function_names, alg, plugin_name);
			break;
		}

		seed = chunk_create(vector->seed, vector->len);
		if (!xof->set_seed(xof, seed))
		{
			goto failure;
		}
		/* allocated bytes */
		if (!xof->allocate_bytes(xof, vector->out_len, &out))
		{
			goto failure;
		}
		if (out.len != vector->out_len)
		{
			goto failure;
		}
		if (!memeq(vector->out, out.ptr, out.len))
		{
			goto failure;
		}
		/* bytes to existing buffer */
		memset(out.ptr, 0, out.len);
		if (!xof->set_seed(xof, seed))
		{
			goto failure;
		}
		if (!xof->get_bytes(xof, vector->out_len, out.ptr))
		{
			goto failure;
		}
		if (!memeq(vector->out, out.ptr, vector->out_len))
		{
			goto failure;
		}
		/* bytes to existing buffer, using append mode */
		/* TODO */

		failed = FALSE;
failure:
		xof->destroy(xof);
		chunk_free(&out);
		if (failed)
		{
			DBG1(DBG_LIB, "disabled %N[%s]: %s test vector failed",
				 ext_out_function_names, alg, plugin_name, get_name(vector));
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!tested)
	{
		DBG1(DBG_LIB, "%s %N[%s]: no test vectors found",
			 this->required ? "disabled" : "enabled ",
			 ext_out_function_names, alg, plugin_name);
		return !this->required;
	}
	if (!failed)
	{
		if (speed)
		{
			*speed = bench_xof(this, alg, create);
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors, %d points",
				 ext_out_function_names, alg, plugin_name, tested, *speed);
		}
		else
		{
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors",
				 ext_out_function_names, alg, plugin_name, tested);
		}
	}
	return !failed;
}

/**
 * Benchmark a RNG
 */
static u_int bench_rng(private_crypto_tester_t *this,
					   rng_quality_t quality, rng_constructor_t create)
{
	rng_t *rng;

	rng = create(quality);
	if (rng)
	{
		struct timespec start;
		chunk_t buf;
		u_int runs;

		runs = 0;
		buf = chunk_alloc(this->bench_size);
		start_timing(&start);
		while (end_timing(&start) < this->bench_time)
		{
			if (!rng->get_bytes(rng, buf.len, buf.ptr))
			{
				runs = 0;
				break;
			}
			runs++;
		}
		free(buf.ptr);
		rng->destroy(rng);

		return runs;
	}
	return 0;
}

METHOD(crypto_tester_t, test_rng, bool,
	private_crypto_tester_t *this, rng_quality_t quality,
	rng_constructor_t create, u_int *speed, const char *plugin_name)
{
	enumerator_t *enumerator;
	rng_test_vector_t *vector;
	bool failed = FALSE;
	u_int tested = 0;

	if (!this->rng_true && quality == RNG_TRUE)
	{
		DBG1(DBG_LIB, "enabled  %N[%s]: skipping test (disabled by config)",
			 rng_quality_names, quality, plugin_name);
		return TRUE;
	}

	enumerator = this->rng->create_enumerator(this->rng);
	while (enumerator->enumerate(enumerator, &vector))
	{
		chunk_t data = chunk_empty;
		rng_t *rng;

		if (vector->quality != quality)
		{
			continue;
		}

		tested++;
		failed = TRUE;
		rng = create(quality);
		if (!rng)
		{
			DBG1(DBG_LIB, "disabled %N[%s]: creating instance failed",
				 rng_quality_names, quality, plugin_name);
			break;
		}

		/* allocated bytes */
		if (!rng->allocate_bytes(rng, vector->len, &data) ||
			data.len != vector->len ||
			!vector->test(vector->user, data))
		{
			goto failure;
		}
		/* write bytes into existing buffer */
		memset(data.ptr, 0, data.len);
		if (!rng->get_bytes(rng, vector->len, data.ptr))
		{
			goto failure;
		}
		if (!vector->test(vector->user, data))
		{
			goto failure;
		}

		failed = FALSE;
failure:
		rng->destroy(rng);
		chunk_free(&data);
		if (failed)
		{
			DBG1(DBG_LIB, "disabled %N[%s]: %s test vector failed",
				 rng_quality_names, quality, plugin_name, get_name(vector));
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!tested)
	{
		DBG1(DBG_LIB, "%s %N[%s]: no test vectors found",
			 this->required ? ", disabled" : "enabled ",
			 rng_quality_names, quality, plugin_name);
		return !this->required;
	}
	if (!failed)
	{
		if (speed)
		{
			*speed = bench_rng(this, quality, create);
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors, %d points",
				 rng_quality_names, quality, plugin_name, tested, *speed);
		}
		else
		{
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors",
				 rng_quality_names, quality, plugin_name, tested);
		}
	}
	return !failed;
}

/**
 * Benchmark a DH backend
 */
static u_int bench_dh(private_crypto_tester_t *this,
					  diffie_hellman_group_t group, dh_constructor_t create)
{
	chunk_t pub = chunk_empty, shared = chunk_empty;
	diffie_hellman_t *dh;
	struct timespec start;
	u_int runs;

	runs = 0;
	start_timing(&start);
	while (end_timing(&start) < this->bench_time)
	{
		dh = create(group);
		if (!dh)
		{
			return 0;
		}
		if (dh->get_my_public_value(dh, &pub) &&
			dh->set_other_public_value(dh, pub) &&
			dh->get_shared_secret(dh, &shared))
		{
			runs++;
		}
		chunk_free(&pub);
		chunk_free(&shared);
		dh->destroy(dh);
	}
	return runs;
}

METHOD(crypto_tester_t, test_dh, bool,
	private_crypto_tester_t *this, diffie_hellman_group_t group,
	dh_constructor_t create, u_int *speed, const char *plugin_name)
{
	enumerator_t *enumerator;
	dh_test_vector_t *v;
	bool failed = FALSE;
	u_int tested = 0;

	enumerator = this->dh->create_enumerator(this->dh);
	while (enumerator->enumerate(enumerator, &v))
	{
		diffie_hellman_t *a, *b;
		chunk_t apub, bpub, asec, bsec;

		if (v->group != group)
		{
			continue;
		}

		a = create(group);
		b = create(group);
		if (!a || !b)
		{
			DESTROY_IF(a);
			DESTROY_IF(b);
			failed = TRUE;
			tested++;
			DBG1(DBG_LIB, "disabled %N[%s]: creating instance failed",
				 diffie_hellman_group_names, group, plugin_name);
			break;
		}

		if (!a->set_private_value || !b->set_private_value)
		{	/* does not support testing */
			a->destroy(a);
			b->destroy(b);
			continue;
		}
		failed = TRUE;
		tested++;

		apub = bpub = asec = bsec = chunk_empty;

		if (!a->set_private_value(a, chunk_create(v->priv_a, v->priv_len)) ||
			!b->set_private_value(b, chunk_create(v->priv_b, v->priv_len)))
		{
			goto failure;
		}
		if (!a->get_my_public_value(a, &apub) ||
			!chunk_equals(apub, chunk_create(v->pub_a, v->pub_len)))
		{
			goto failure;
		}
		if (!b->get_my_public_value(b, &bpub) ||
			!chunk_equals(bpub, chunk_create(v->pub_b, v->pub_len)))
		{
			goto failure;
		}
		if (!a->set_other_public_value(a, bpub) ||
			!b->set_other_public_value(b, apub))
		{
			goto failure;
		}
		if (!a->get_shared_secret(a, &asec) ||
			!chunk_equals(asec, chunk_create(v->shared, v->shared_len)))
		{
			goto failure;
		}
		if (!b->get_shared_secret(b, &bsec) ||
			!chunk_equals(bsec, chunk_create(v->shared, v->shared_len)))
		{
			goto failure;
		}

		failed = FALSE;
failure:
		a->destroy(a);
		b->destroy(b);
		chunk_free(&apub);
		chunk_free(&bpub);
		chunk_free(&asec);
		chunk_free(&bsec);
		if (failed)
		{
			DBG1(DBG_LIB, "disabled %N[%s]: %s test vector failed",
				 diffie_hellman_group_names, group, plugin_name, get_name(v));
			break;
		}
	}
	enumerator->destroy(enumerator);
	if (!tested)
	{
		DBG1(DBG_LIB, "%s %N[%s]: no test vectors found / untestable",
			 this->required ? "disabled" : "enabled ",
			 diffie_hellman_group_names, group, plugin_name);
		return !this->required;
	}
	if (!failed)
	{
		if (speed)
		{
			*speed = bench_dh(this, group, create);
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors, %d points",
				 diffie_hellman_group_names, group, plugin_name, tested, *speed);
		}
		else
		{
			DBG1(DBG_LIB, "enabled  %N[%s]: passed %u test vectors",
				 diffie_hellman_group_names, group, plugin_name, tested);
		}
	}
	return !failed;
}

METHOD(crypto_tester_t, add_crypter_vector, void,
	private_crypto_tester_t *this, crypter_test_vector_t *vector)
{
	this->crypter->insert_last(this->crypter, vector);
}

METHOD(crypto_tester_t, add_aead_vector, void,
	private_crypto_tester_t *this, aead_test_vector_t *vector)
{
	this->aead->insert_last(this->aead, vector);
}

METHOD(crypto_tester_t, add_signer_vector, void,
	private_crypto_tester_t *this, signer_test_vector_t *vector)
{
	this->signer->insert_last(this->signer, vector);
}

METHOD(crypto_tester_t, add_hasher_vector, void,
	private_crypto_tester_t *this, hasher_test_vector_t *vector)
{
	this->hasher->insert_last(this->hasher, vector);
}

METHOD(crypto_tester_t, add_prf_vector, void,
	private_crypto_tester_t *this, prf_test_vector_t *vector)
{
	this->prf->insert_last(this->prf, vector);
}

METHOD(crypto_tester_t, add_xof_vector, void,
	private_crypto_tester_t *this, xof_test_vector_t *vector)
{
	this->xof->insert_last(this->xof, vector);
}

METHOD(crypto_tester_t, add_rng_vector, void,
	private_crypto_tester_t *this, rng_test_vector_t *vector)
{
	this->rng->insert_last(this->rng, vector);
}

METHOD(crypto_tester_t, add_dh_vector, void,
	private_crypto_tester_t *this, dh_test_vector_t *vector)
{
	this->dh->insert_last(this->dh, vector);
}

METHOD(crypto_tester_t, destroy, void,
	private_crypto_tester_t *this)
{
	this->crypter->destroy(this->crypter);
	this->aead->destroy(this->aead);
	this->signer->destroy(this->signer);
	this->hasher->destroy(this->hasher);
	this->prf->destroy(this->prf);
	this->xof->destroy(this->xof);
	this->rng->destroy(this->rng);
	this->dh->destroy(this->dh);
	free(this);
}

/**
 * See header
 */
crypto_tester_t *crypto_tester_create()
{
	private_crypto_tester_t *this;

	INIT(this,
		.public = {
			.test_crypter = _test_crypter,
			.test_aead = _test_aead,
			.test_signer = _test_signer,
			.test_hasher = _test_hasher,
			.test_prf = _test_prf,
			.test_xof = _test_xof,
			.test_rng = _test_rng,
			.test_dh = _test_dh,
			.add_crypter_vector = _add_crypter_vector,
			.add_aead_vector = _add_aead_vector,
			.add_signer_vector = _add_signer_vector,
			.add_hasher_vector = _add_hasher_vector,
			.add_prf_vector = _add_prf_vector,
			.add_xof_vector = _add_xof_vector,
			.add_rng_vector = _add_rng_vector,
			.add_dh_vector = _add_dh_vector,
			.destroy = _destroy,
		},
		.crypter = linked_list_create(),
		.aead = linked_list_create(),
		.signer = linked_list_create(),
		.hasher = linked_list_create(),
		.prf = linked_list_create(),
		.xof = linked_list_create(),
		.rng = linked_list_create(),
		.dh = linked_list_create(),

		.required = lib->settings->get_bool(lib->settings,
								"%s.crypto_test.required", FALSE, lib->ns),
		.rng_true = lib->settings->get_bool(lib->settings,
								"%s.crypto_test.rng_true", FALSE, lib->ns),
		.bench_time = lib->settings->get_int(lib->settings,
								"%s.crypto_test.bench_time", 50, lib->ns),
		.bench_size = lib->settings->get_int(lib->settings,
								"%s.crypto_test.bench_size", 1024, lib->ns),
	);

	/* enforce a block size of 16, should be fine for all algorithms */
	this->bench_size = this->bench_size / 16 * 16;

	return &this->public;
}
