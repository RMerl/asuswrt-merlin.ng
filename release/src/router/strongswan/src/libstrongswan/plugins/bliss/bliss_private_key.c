/*
 * Copyright (C) 2014-2016 Andreas Steffen
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

#include "bliss_private_key.h"
#include "bliss_public_key.h"
#include "bliss_param_set.h"
#include "bliss_utils.h"
#include "bliss_sampler.h"
#include "bliss_signature.h"
#include "bliss_bitpacker.h"
#include "ntt_fft.h"
#include "ntt_fft_reduce.h"

#include <crypto/xofs/xof_bitspender.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <asn1/oid.h>

#define _GNU_SOURCE
#include <stdlib.h>

typedef struct private_bliss_private_key_t private_bliss_private_key_t;

#define SECRET_KEY_TRIALS_MAX	50

/**
 * Private data of a bliss_private_key_t object.
 */
struct private_bliss_private_key_t {
	/**
	 * Public interface for this signer.
	 */
	bliss_private_key_t public;

	/**
	 * BLISS signature parameter set
	 */
	const bliss_param_set_t *set;

	/**
	 * BLISS secret key S1 (coefficients of polynomial f)
	 */
	int8_t *s1;

	/**
	 * BLISS secret key S2 (coefficients of polynomial 2g + 1)
	 */
	int8_t *s2;

	/**
	 * NTT of BLISS public key a (coefficients of polynomial (2g + 1)/f)
	 */
	uint32_t *A;

	/**
	 * NTT of BLISS public key in Montgomery representation Ar = rA mod
	 */
	uint32_t *Ar;

	/**
	 * reference count
	 */
	refcount_t ref;
};

METHOD(private_key_t, get_type, key_type_t,
	private_bliss_private_key_t *this)
{
	return KEY_BLISS;
}

/**
 * Multiply secret vector s with binary challenge vector c
 */
static void multiply_by_c(int8_t *s, int n, uint16_t *c_indices,
						  uint16_t kappa, int32_t *product)
{
	int i, j, index;

	for (i = 0; i < n; i++)
	{
		product[i] = 0;

		for (j = 0; j < kappa; j++)
		{
			index = c_indices[j];
			if (i - index < 0)
			{
				product[i] -= s[i - index + n];
			}
			else
			{
				product[i] += s[i - index];
			}
		}
	}
}

/**
 * BLISS-B GreedySC algorithm
 */
static void greedy_sc(int8_t *s1, int8_t *s2, int n, uint16_t *c_indices,
					  uint16_t kappa, int32_t *v1, int32_t *v2)
{
	int i, j, index;
	int32_t sign;

	for (i = 0; i < n; i++)
	{
		v1[i] = v2[i] = 0;
	}
	for (j = 0; j < kappa; j++)
	{
		index = c_indices[j];
		sign = 0;

		for (i = 0; i < index; i++)
		{
			sign -= (v1[i] * s1[i - index + n] + v2[i] * s2[i - index + n]);
		}
		for (i = index; i < n; i++)
		{
			sign += (v1[i] * s1[i - index] + v2[i] * s2[i - index]);
		}
		for (i = 0; i < index; i++)
		{
			if (sign > 0)
			{
				v1[i] += s1[i - index + n];
				v2[i] += s2[i - index + n];
			}
			else
			{
				v1[i] -= s1[i - index + n];
				v2[i] -= s2[i - index + n];
			}
		}
		for (i = index; i < n; i++)
		{
			if (sign > 0)
			{
				v1[i] -= s1[i - index];
				v2[i] -= s2[i - index];
			}
			else
			{
				v1[i] += s1[i - index];
				v2[i] += s2[i - index];
			}
		}
	}
}

/**
 * Compute a BLISS signature
 */
static bool sign_bliss(private_bliss_private_key_t *this, hash_algorithm_t alg,
					   chunk_t data, chunk_t *signature)
{
	ntt_fft_t *fft;
	bliss_signature_t *sig;
	bliss_sampler_t *sampler = NULL;
	rng_t *rng;
	hasher_t *hasher;
	ext_out_function_t mgf1_alg, oracle_alg;
	size_t mgf1_seed_len;
	uint8_t mgf1_seed_buf[HASH_SIZE_SHA512], data_hash_buf[HASH_SIZE_SHA512];
	chunk_t mgf1_seed, data_hash;
	uint16_t q, q2, p, p2, *c_indices, tests = 0;
	uint32_t *ay;
	int32_t *y1, *y2, *z1, *z2, *u, *s1c, *s2c;
	int32_t y1_min = 0, y1i, y1_max = 0, y2_min = 0, y2i, y2_max = 0;
	int32_t scalar, norm, ui;
	int16_t *ud, *uz2d, *z2d, value;
	int i, n;
	double mean1 = 0, mean2 = 0, sigma1 = 0, sigma2 = 0;
	bool accepted, positive, success = FALSE, use_bliss_b;

	/* Initialize signature */
	*signature = chunk_empty;

	/* Create data hash using configurable hash algorithm */
	hasher = lib->crypto->create_hasher(lib->crypto, alg);
	if (!hasher)
	{
		return FALSE;
	}
	data_hash = chunk_create(data_hash_buf, hasher->get_hash_size(hasher));

	if (!hasher->get_hash(hasher, data, data_hash_buf))
	{
		hasher->destroy(hasher);
		return FALSE;
	}
	hasher->destroy(hasher);

	/* Set MGF1 hash algorithm and seed length based on security strength */
	if (this->set->strength > 160)
	{
		mgf1_alg = XOF_MGF1_SHA256;
		mgf1_seed_len = HASH_SIZE_SHA256;
	}
	else
	{
		mgf1_alg = XOF_MGF1_SHA1;
		mgf1_seed_len = HASH_SIZE_SHA1;
	}
	mgf1_seed = chunk_create(mgf1_seed_buf, mgf1_seed_len);

	rng = lib->crypto->create_rng(lib->crypto, RNG_STRONG);
	if (!rng)
	{
		return FALSE;
	}

	/* MGF1 hash algorithm to be used for random oracle */
	oracle_alg = XOF_MGF1_SHA512;

	/* Initialize a couple of needed variables */
	n  = this->set->n;
	q  = this->set->q;
	p  = this->set->p;
	q2 = 2 * q;
	p2 = p / 2;
	ay   = malloc(n * sizeof(uint32_t));
	z2   = malloc(n * sizeof(int32_t));
	s1c  = malloc(n * sizeof(int32_t));
	s2c  = malloc(n * sizeof(int32_t));
	u    = malloc(n * sizeof(int32_t));
	uz2d = malloc(n * sizeof(int16_t));

	sig = bliss_signature_create(this->set);
	sig->get_parameters(sig, &z1, &z2d, &c_indices);
	y1 = z1;
	y2 = z2;
	ud = z2d;

	fft = ntt_fft_create(this->set->fft_params);

	/* Use of the enhanced BLISS-B signature algorithm? */
	switch (this->set->id)
	{
		default:
		case BLISS_I:
		case BLISS_II:
		case BLISS_III:
		case BLISS_IV:
			use_bliss_b = FALSE;
			break;
		case BLISS_B_I:
		case BLISS_B_II:
		case BLISS_B_III:
		case BLISS_B_IV:
			use_bliss_b = TRUE;
			break;
	}

	while (true)
	{
		tests++;

		if (!rng->get_bytes(rng, mgf1_seed_len, mgf1_seed_buf))
		{
			goto end;
		}
		DESTROY_IF(sampler);

		sampler = bliss_sampler_create(mgf1_alg, mgf1_seed, this->set);
		if (!sampler)
		{
			goto end;
		}

		/* Gaussian sampling for vectors y1 and y2 */
		for (i = 0; i < n; i++)
		{
			if (!sampler->gaussian(sampler, &y1i) ||
				!sampler->gaussian(sampler, &y2i))
			{
				goto end;
			}
			y1[i] = y1i;
			y2[i] = y2i;

			/* Collect statistical data on rejection sampling */
			if (i == 0)
			{
				y1_min = y1_max = y1i;
				y2_min = y2_max = y2i;
			}
			else
			{
				if (y1i < y1_min)
				{
					y1_min = y1i;
				}
				else if (y1i > y1_max)
				{
					y1_max = y1i;
				}
				if (y2i < y2_min)
				{
					y2_min = y2i;
				}
				else if (y2i > y2_max)
				{
					y2_max = y2i;
				}
			}
			mean1 += y1i;
			mean2 += y2i;
			sigma1 += y1i * y1i;
			sigma2 += y2i * y2i;

			ay[i] = y1i < 0 ? q + y1i : y1i;
		}

		/* Compute statistics on vectors y1 and y2 */
		mean1 /= n;
		mean2 /= n;
		sigma1 /= n;
		sigma2 /= n;
		sigma2 -= mean1 * mean1;
		sigma2 -= mean2 * mean2;
		DBG2(DBG_LIB, "y1 = %d..%d (sigma2 = %5.0f, mean = %4.1f)",
					   y1_min, y1_max, sigma1, mean1);
		DBG2(DBG_LIB, "y2 = %d..%d (sigma2 = %5.0f, mean = %4.1f)",
					   y2_min, y2_max, sigma2, mean2);

		fft->transform(fft, ay, ay, FALSE);

		for (i = 0; i < n; i++)
		{
			ay[i] = ntt_fft_mreduce(this->Ar[i] * ay[i], this->set->fft_params);
		}
		fft->transform(fft, ay, ay, TRUE);

		for (i = 0; i < n; i++)
		{
			ui = 2 * this->set->q2_inv * (int32_t)ay[i] + y2[i];
			u[i] = ((ui < 0) ? q2 + ui : ui) % q2;
		}
		bliss_utils_round_and_drop(this->set, u, ud);

		/* Detailed debugging information */
		DBG3(DBG_LIB, "  i    u[i]  ud[i]");
		for (i = 0; i < n; i++)
		{
			DBG3(DBG_LIB, "%3d  %6d   %4d", i, u[i], ud[i]);
		}

		if (!bliss_utils_generate_c(oracle_alg, data_hash, ud, this->set,
									c_indices))
		{
			goto end;
		}

		if (use_bliss_b)
		{
			/* Compute v = (s1c, s2c) with the GreedySC algorithm */
			greedy_sc(this->s1, this->s2, n, c_indices, this->set->kappa,
					  s1c, s2c);

			/* Compute norm = ||v||^2 = ||Sc'||^2 */
			norm = bliss_utils_scalar_product(s1c, s1c, n) +
				   bliss_utils_scalar_product(s2c, s2c, n);

			/* Just in case. ||v||^2 <= P_max should always be fulfilled */
			if (norm > this->set->p_max)
			{
				goto end;
			}
		}
		else
		{
			/* Compute s*c */
			multiply_by_c(this->s1, n, c_indices, this->set->kappa, s1c);
			multiply_by_c(this->s2, n, c_indices, this->set->kappa, s2c);

			/* Compute norm = |Sc||^2 */
			norm = bliss_utils_scalar_product(s1c, s1c, n) +
				   bliss_utils_scalar_product(s2c, s2c, n);
		}

		if (!sampler->bernoulli_exp(sampler, this->set->M - norm, &accepted))
		{
			goto end;
		}
		if (use_bliss_b)
		{
			DBG2(DBG_LIB, "norm2(s1*c') + norm2(s2*c') = %u (%u max), %s",
				 norm, this->set->p_max, accepted ? "accepted" : "rejected");

		}
		else
		{
			DBG2(DBG_LIB, "norm2(s1*c) + norm2(s2*c) = %u, %s",
				 norm, accepted ? "accepted" : "rejected");
		}
		if (!accepted)
		{
			continue;
		}

		/* Compute z */
		if (!sampler->sign(sampler, &positive))
		{
			goto end;
		}
		for (i = 0; i < n; i++)
		{
			if (positive)
			{
				z1[i] = y1[i] + s1c[i];
				z2[i] = y2[i] + s2c[i];
			}
			else
			{
				z1[i] = y1[i] - s1c[i];
				z2[i] = y2[i] - s2c[i];
			}
		}
		/* Reject with probability 1/cosh(scalar/sigma^2) */
		scalar = bliss_utils_scalar_product(z1, s1c, n) +
				 bliss_utils_scalar_product(z2, s2c, n);

		if (!sampler->bernoulli_cosh(sampler, scalar, &accepted))
		{
			goto end;
		}
		DBG2(DBG_LIB, "scalar(z1,s1*c) + scalar(z2,s2*c) = %d, %s",
					   scalar, accepted ? "accepted" : "rejected");
		if (!accepted)
		{
			continue;
		}

		/* Compute z2 with dropped bits */
		for (i = 0; i < n; i++)
		{
			u[i] -= z2[i];
			if (u[i] < 0)
			{
				u[i] += q2;
			}
			else if (u[i] >= q2)
			{
				u[i] -= q2;
			}
		}
		bliss_utils_round_and_drop(this->set, u, uz2d);

		for (i = 0; i < n; i++)
		{
			value = ud[i] - uz2d[i];
			if (value <= -p2)
			{
				value += p;
			}
			else if (value > p2)
			{
				value -= p;
			}
			z2d[i] = value;
		}

		if (!bliss_utils_check_norms(this->set, z1, z2d))
		{
			continue;
		}

		*signature = sig->get_encoding(sig);
		if (signature->len == 0)
		{
			DBG1(DBG_LIB, "inefficient Huffman coding of signature");
			continue;
		}
		DBG2(DBG_LIB, "signature generation needed %u round%s", tests,
					  (tests == 1) ? "" : "s");
		break;
	}
	success = TRUE;

end:
	/* cleanup */
	DESTROY_IF(sampler);
	sig->destroy(sig);
	fft->destroy(fft);
	rng->destroy(rng);
	memwipe(s1c, n * sizeof(int32_t));
	memwipe(s2c, n * sizeof(int32_t));
	free(s1c);
	free(s2c);
	free(ay);
	free(z2);
	free(u);
	free(uz2d);

	return success;
}

METHOD(private_key_t, sign, bool,
	private_bliss_private_key_t *this, signature_scheme_t scheme, void *params,
	chunk_t data, chunk_t *signature)
{
	switch (scheme)
	{
		case SIGN_BLISS_WITH_SHA2_256:
			return sign_bliss(this, HASH_SHA256, data, signature);
		case SIGN_BLISS_WITH_SHA2_384:
			return sign_bliss(this, HASH_SHA384, data, signature);
		case SIGN_BLISS_WITH_SHA2_512:
			return sign_bliss(this, HASH_SHA512, data, signature);
		case SIGN_BLISS_WITH_SHA3_256:
			return sign_bliss(this, HASH_SHA3_256, data, signature);
		case SIGN_BLISS_WITH_SHA3_384:
			return sign_bliss(this, HASH_SHA3_384, data, signature);
		case SIGN_BLISS_WITH_SHA3_512:
			return sign_bliss(this, HASH_SHA3_512, data, signature);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported with BLISS",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(private_key_t, decrypt, bool,
	private_bliss_private_key_t *this, encryption_scheme_t scheme,
	chunk_t crypto, chunk_t *plain)
{
	DBG1(DBG_LIB, "encryption scheme %N not supported",
				   encryption_scheme_names, scheme);
	return FALSE;
}

METHOD(private_key_t, get_keysize, int,
	private_bliss_private_key_t *this)
{
	return this->set->strength;
}

METHOD(private_key_t, get_public_key, public_key_t*,
	private_bliss_private_key_t *this)
{
	public_key_t *public;
	chunk_t pubkey;

	pubkey = bliss_public_key_info_encode(this->set->oid, this->A, this->set);
	public = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_BLISS,
								BUILD_BLOB_ASN1_DER, pubkey, BUILD_END);
	free(pubkey.ptr);

	return public;
}

METHOD(private_key_t, get_encoding, bool,
	private_bliss_private_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	switch (type)
	{
		case PRIVKEY_ASN1_DER:
		case PRIVKEY_PEM:
		{
			chunk_t s1, s2, pubkey;
			bliss_bitpacker_t *packer;
			size_t s_bits;
			int8_t value;
			bool success = TRUE;
			int i;

			pubkey = bliss_public_key_encode(this->A, this->set);

			/* Use either 2 or 3 bits per array element */
			s_bits = 2 + (this->set->non_zero2 > 0);

			/* Encode secret s1 */
			packer = bliss_bitpacker_create(s_bits * this->set->n);
			for (i = 0; i < this->set->n; i++)
			{
				packer->write_bits(packer, this->s1[i], s_bits);
			}
			s1 = packer->extract_buf(packer);
			packer->destroy(packer);

			/* Encode secret s2 */
			packer = bliss_bitpacker_create(s_bits * this->set->n);
			for (i = 0; i < this->set->n; i++)
			{
				value = this->s2[i];
				if (i == 0)
				{
					value -= 1;
				}
				value /= 2;
				packer->write_bits(packer, value, s_bits);
			}
			s2 = packer->extract_buf(packer);
			packer->destroy(packer);

			*encoding = asn1_wrap(ASN1_SEQUENCE, "mmss",
							asn1_build_known_oid(this->set->oid),
							asn1_bitstring("m", pubkey),
							asn1_bitstring("m", s1),
							asn1_bitstring("m", s2)
						);
			if (type == PRIVKEY_PEM)
			{
				chunk_t asn1_encoding = *encoding;

				success = lib->encoding->encode(lib->encoding, PRIVKEY_PEM,
								NULL, encoding, CRED_PART_BLISS_PRIV_ASN1_DER,
								asn1_encoding, CRED_PART_END);
				chunk_clear(&asn1_encoding);
			}
			return success;
		}
		default:
			return FALSE;
	}
}

METHOD(private_key_t, get_fingerprint, bool,
	private_bliss_private_key_t *this, cred_encoding_type_t type, chunk_t *fp)
{
	bool success;

	if (lib->encoding->get_cache(lib->encoding, type, this, fp))
	{
		return TRUE;
	}
	success = bliss_public_key_fingerprint(this->set->oid, this->A,
										   this->set, type, fp);
	if (success)
	{
		lib->encoding->cache(lib->encoding, type, this, *fp);
	}
	return success;
}

METHOD(private_key_t, get_ref, private_key_t*,
	private_bliss_private_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(private_key_t, destroy, void,
	private_bliss_private_key_t *this)
{
	if (ref_put(&this->ref))
	{
		lib->encoding->clear_cache(lib->encoding, this);
		if (this->s1)
		{
			memwipe(this->s1, this->set->n * sizeof(int8_t));
			free(this->s1);
		}
		if (this->s2)
		{
			memwipe(this->s2, this->set->n * sizeof(int8_t));
			free(this->s2);
		}
		free(this->A);
		free(this->Ar);
		free(this);
	}
}

/**
 * Internal generic constructor
 */
static private_bliss_private_key_t *bliss_private_key_create_empty(void)
{
	private_bliss_private_key_t *this;

	INIT(this,
		.public = {
			.key = {
				.get_type = _get_type,
				.sign = _sign,
				.decrypt = _decrypt,
				.get_keysize = _get_keysize,
				.get_public_key = _get_public_key,
				.equals = private_key_equals,
				.belongs_to = private_key_belongs_to,
				.get_fingerprint = _get_fingerprint,
				.has_fingerprint = private_key_has_fingerprint,
				.get_encoding = _get_encoding,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
		},
		.ref = 1,
	);
	return this;
}

/**
 * Compute the scalar product of a vector x with a negative wrapped vector y
 */
static int16_t wrapped_product(int8_t *x, int8_t *y, int n, int shift)
{
	int16_t product = 0;
	int i;

	for (i = 0; i < n - shift; i++)
	{
		product += x[i] * y[i + shift];
	}
	for (i = n - shift; i < n; i++)
	{
		product -= x[i] * y[i + shift - n];
	}
	return product;
}

/**
 * Apply a negative wrapped rotation to a vector x
 */
static void wrap(int16_t *x, int n, int shift, int16_t *x_wrapped)
{
	int i;

	for (i = 0; i < n - shift; i++)
	{
		x_wrapped[i + shift] = x[i];
	}
	for (i = n - shift; i < n; i++)
	{
		x_wrapped[i + shift - n] = -x[i];
	}
}

/**
 * int16_t compare function needed for qsort()
 */
static int compare(const int16_t *a, const int16_t *b)
{
	int16_t temp = *a - *b;

	if (temp > 0)
	{
		return 1;
	}
	else if (temp < 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

/**
 * Compute the Nk(S) norm of S = (s1, s2)
 */
static uint32_t nks_norm(int8_t *s1, int8_t *s2, int n, uint16_t kappa)
{
	int16_t t[n], t_wrapped[n], max_kappa[n];
	uint32_t nks = 0;
	int i, j;

	for (i = 0; i < n; i++)
	{
		t[i] = wrapped_product(s1, s1, n, i) + wrapped_product(s2, s2, n, i);
	}

	for (i = 0; i < n; i++)
	{
		wrap(t, n, i, t_wrapped);
		qsort(t_wrapped, n, sizeof(int16_t), (void*)compare);
		max_kappa[i] = 0;

		for (j = 1; j <= kappa; j++)
		{
			max_kappa[i] += t_wrapped[n - j];
		}
	}
	qsort(max_kappa, n, sizeof(int16_t), (void*)compare);

	for (i = 1; i <= kappa; i++)
	{
		nks += max_kappa[n - i];
	}
	return nks;
}

/**
 * Compute the inverse x1 of x modulo q as x^(-1) = x^(q-2) mod q
 */
static uint32_t invert(private_bliss_private_key_t *this, uint32_t x)
{
	uint32_t x1, x2;
	uint16_t q2;
	int i, i_max;

	q2 = this->set->q - 2;
	x1 = (q2 & 1) ? x : 1;
	x2 = x;
	i_max = 15;

	while ((q2 & (1 << i_max)) == 0)
	{
		i_max--;
	}
	for (i = 1; i <= i_max; i++)
	{
		x2 = ntt_fft_mreduce(x2 * x2, this->set->fft_params);

		if (q2 & (1 << i))
		{
			x1 = ntt_fft_mreduce(x1 * x2, this->set->fft_params);
		}
	}

	return x1;
}

/**
 * Create a vector with sparse and small coefficients from seed
 */
static int8_t* create_vector_from_seed(private_bliss_private_key_t *this,
									   ext_out_function_t alg, chunk_t seed)
{
	xof_bitspender_t *bitspender;
	uint32_t index, sign;
	int8_t *vector;
	int non_zero;

	bitspender = xof_bitspender_create(alg, seed, FALSE);
	if (!bitspender)
	{
	    return NULL;
	}

	vector = malloc(sizeof(int8_t) * this->set->n);
	memset(vector, 0x00, this->set->n);

	non_zero = this->set->non_zero1;
	while (non_zero)
	{
		if (!bitspender->get_bits(bitspender, this->set->n_bits, &index))
		{
			free(vector);
			return NULL;
		}
		if (vector[index] != 0)
		{
			continue;
		}

		if (!bitspender->get_bits(bitspender, 1, &sign))
		{
			free(vector);
			return NULL;
		}
		vector[index] = sign ? 1 : -1;
		non_zero--;
	}

	non_zero = this->set->non_zero2;
	while (non_zero)
	{
		if (!bitspender->get_bits(bitspender, this->set->n_bits, &index))
		{
			free(vector);
			return NULL;
		}
		if (vector[index] != 0)
		{
			continue;
		}

		if (!bitspender->get_bits(bitspender, 1, &sign))
		{
			free(vector);
			return NULL;
		}
		vector[index] = sign ? 2 : -2;
		non_zero--;
	}
	bitspender->destroy(bitspender);

	return vector;
}

/**
 * Generate the secret key S = (s1, s2) fulfilling the Nk(S) norm
 */
static bool create_secret(private_bliss_private_key_t *this, rng_t *rng,
						 int8_t **s1, int8_t **s2, int *trials)
{
	uint8_t seed_buf[32];
	uint8_t *f, *g;
	uint32_t l2_norm, nks;
	int i, n;
	chunk_t seed;
	size_t seed_len;
	ext_out_function_t alg;

	n = this->set->n;
	*s1 = NULL;
	*s2 = NULL;

	/* Set MGF1 hash algorithm and seed length based on security strength */
	if (this->set->strength > 160)
	{
		alg = XOF_MGF1_SHA256;
		seed_len = HASH_SIZE_SHA256;
	}
	else
	{
		alg = XOF_MGF1_SHA1;
		seed_len = HASH_SIZE_SHA1;
	}
	seed = chunk_create(seed_buf, seed_len);

	while (*trials < SECRET_KEY_TRIALS_MAX)
	{
		(*trials)++;

		if (!rng->get_bytes(rng, seed_len, seed_buf))
		{
			return FALSE;
		}
		f = create_vector_from_seed(this, alg, seed);
		if (f == NULL)
		{
			return FALSE;
		}
		if (!rng->get_bytes(rng, seed_len, seed_buf))
		{
			free(f);
			return FALSE;
		}
		g = create_vector_from_seed(this, alg, seed);
		if (g == NULL)
		{
			free(f);
			return FALSE;
		}

		/* Compute 2g + 1 */
		for (i = 0; i < n; i++)
		{
			g[i] *= 2;
		}
		g[0] += 1;

		l2_norm = wrapped_product(f, f, n, 0) +  wrapped_product(g, g, n, 0);
		nks = nks_norm(f, g, n, this->set->kappa);

		switch (this->set->id)
		{
			case BLISS_I:
			case BLISS_II:
			case BLISS_III:
			case BLISS_IV:
				DBG2(DBG_LIB, "l2 norm of s1||s2: %d, Nk(S): %u (%u max)",
							   l2_norm, nks, this->set->nks_max);
				if (nks < this->set->nks_max)
				{
					*s1 = f;
					*s2 = g;
					return TRUE;
				}
				free(f);
				free(g);
				break;
			case BLISS_B_I:
			case BLISS_B_II:
			case BLISS_B_III:
			case BLISS_B_IV:
				DBG2(DBG_LIB, "l2 norm of s1||s2: %d, Nk(S): %u",
							   l2_norm, nks);
				*s1 = f;
				*s2 = g;
				return TRUE;
		}
	}

	return FALSE;
}

/**
 * See header.
 */
bliss_private_key_t *bliss_private_key_gen(key_type_t type, va_list args)
{
	private_bliss_private_key_t *this;
	u_int key_size = BLISS_B_I;
	int i, n, trials = 0;
	uint32_t *S1, *S2, *a;
	uint16_t q;
	bool success = FALSE;
	const bliss_param_set_t *set;
	ntt_fft_t *fft;
	rng_t *rng;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_KEY_SIZE:
				key_size = va_arg(args, u_int);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	if (lib->settings->get_bool(lib->settings, "%s.plugins.bliss.use_bliss_b",
								TRUE, lib->ns))
	{
		switch (key_size)
		{
			case BLISS_I:
				key_size = BLISS_B_I;
				break;
			case BLISS_II:
				key_size = BLISS_B_II;
				break;
			case BLISS_III:
				key_size = BLISS_B_III;
				break;
			case BLISS_IV:
				key_size = BLISS_B_IV;
				break;
			default:
				break;
		}
	}

	/* Only BLISS or BLISS-B types I, III, or IV are currently supported */
	set = bliss_param_set_get_by_id(key_size);
	if (!set)
	{
		DBG1(DBG_LIB, "BLISS parameter set %u not supported", key_size);
		return NULL;
	}

	/* Some shortcuts for often used variables */
	n = set->n;
	q = set->q;

	if (set->fft_params->n != n || set->fft_params->q != q)
	{
		DBG1(DBG_LIB, "FFT parameters do not match BLISS parameters");
		return NULL;
	}
	this = bliss_private_key_create_empty();
	this->set = set;

	/* We derive the public key from the private key using the FFT */
	fft = ntt_fft_create(set->fft_params);

	/* Some vectors needed to derive the publi key */
	S1 = malloc(n * sizeof(uint32_t));
	S2 = malloc(n * sizeof(uint32_t));
	a  = malloc(n * sizeof(uint32_t));
	this->A  = malloc(n * sizeof(uint32_t));
	this->Ar = malloc(n * sizeof(uint32_t));

	/* Instantiate a true random generator */
	rng = lib->crypto->create_rng(lib->crypto, RNG_TRUE);

	/* Loop until we have an invertible polynomial s1 */
	do
	{
		if (!create_secret(this, rng, &this->s1, &this->s2, &trials))
		{
			break;
		}

		/* Convert signed arrays to unsigned arrays before FFT */
		for (i = 0; i < n; i++)
		{
			S1[i] = (this->s1[i] < 0) ? this->s1[i] + q :  this->s1[i];
			S2[i] = (this->s2[i] > 0) ? q - this->s2[i] : -this->s2[i];
		}
		fft->transform(fft, S1, S1, FALSE);
		fft->transform(fft, S2, S2, FALSE);

		success = TRUE;

		for (i = 0; i < n; i++)
		{
			if (S1[i] == 0)
			{
				DBG1(DBG_LIB, "S1[%d] is zero - s1 is not invertible", i);
				free(this->s1);
				free(this->s2);
				this->s1 = NULL;
				this->s2 = NULL;
				success = FALSE;
				break;
			}
			this->Ar[i] = invert(this, S1[i]);
			this->Ar[i] = ntt_fft_mreduce(S2[i] * this->Ar[i], set->fft_params);
			this->A[i]  = ntt_fft_mreduce(this->Ar[i], set->fft_params);
		}
	}
	while (!success && trials < SECRET_KEY_TRIALS_MAX);

	DBG1(DBG_LIB, "secret key generation %s after %d trial%s",
		 success ? "succeeded" : "failed", trials, (trials == 1) ? "" : "s");

	if (success)
	{
		fft->transform(fft, this->Ar, a, TRUE);

		DBG4(DBG_LIB, "   i   f   g     a     F     G     A");
		for (i = 0; i < n; i++)
		{
			DBG4(DBG_LIB, "%4d %3d %3d %5u %5u %5u %5u",
						  i, this->s1[i], this->s2[i],
						  ntt_fft_mreduce(a[i], set->fft_params),
				 		  S1[i], S2[i], this->A[i]);
		}
	}
	else
	{
		destroy(this);
	}

	/* Cleanup */
	fft->destroy(fft);
	rng->destroy(rng);
	memwipe(S1, n * sizeof(uint32_t));
	memwipe(S2, n * sizeof(uint32_t));
	free(S1);
	free(S2);
	free(a);

	return success ? &this->public : NULL;
}

/**
 * ASN.1 definition of a BLISS private key
 */
static const asn1Object_t privkeyObjects[] = {
	{ 0, "BLISSPrivateKey",		ASN1_SEQUENCE,   ASN1_NONE }, /*  0 */
	{ 1,   "keyType",			ASN1_OID,        ASN1_BODY }, /*  1 */
	{ 1,   "public",			ASN1_BIT_STRING, ASN1_BODY }, /*  2 */
	{ 1,   "secret1",			ASN1_BIT_STRING, ASN1_BODY }, /*  3 */
	{ 1,   "secret2",			ASN1_BIT_STRING, ASN1_BODY }, /*  4 */
	{ 0, "exit",				ASN1_EOC,        ASN1_EXIT }
};
#define PRIV_KEY_TYPE			1
#define PRIV_KEY_PUBLIC			2
#define PRIV_KEY_SECRET1		3
#define PRIV_KEY_SECRET2		4

/**
 * See header.
 */
bliss_private_key_t *bliss_private_key_load(key_type_t type, va_list args)
{
	private_bliss_private_key_t *this;
	chunk_t key = chunk_empty, object;
	bliss_bitpacker_t *packer;
	asn1_parser_t *parser;
	size_t s_bits = 0;
	int8_t s, s_min = 0, s_max = 0;
	uint32_t s_sign = 0x02, s_mask = 0xfffffffc, value, r2;
	bool success = FALSE;
	int objectID, oid, i;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ASN1_DER:
				key = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	if (key.len == 0)
	{
		return NULL;
	}
	this = bliss_private_key_create_empty();

	parser = asn1_parser_create(privkeyObjects, key);
	parser->set_flags(parser, FALSE, TRUE);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case PRIV_KEY_TYPE:
				oid = asn1_known_oid(object);
				if (oid == OID_UNKNOWN)
				{
					goto end;
				}
				this->set = bliss_param_set_get_by_oid(oid);
				if (this->set == NULL)
				{
					goto end;
				}
				if (lib->settings->get_bool(lib->settings,
							"%s.plugins.bliss.use_bliss_b",TRUE, lib->ns))
				{
					switch (this->set->id)
					{
						case BLISS_I:
							this->set = bliss_param_set_get_by_id(BLISS_B_I);
							break;
						case BLISS_III:
							this->set = bliss_param_set_get_by_id(BLISS_B_III);
							break;
						case BLISS_IV:
							this->set = bliss_param_set_get_by_id(BLISS_B_IV);
							break;
						default:
							break;
					}
				}
				if (this->set->non_zero2)
				{
					s_min = -2;
					s_max =  2;
					s_bits = 3;
				}
				else
				{
					s_min = -1;
					s_max =  1;
					s_bits = 2;
				}
				s_sign = 1 << (s_bits - 1);
				s_mask = ((1 << (32 - s_bits)) - 1) << s_bits;
				break;
			case PRIV_KEY_PUBLIC:
				if (!bliss_public_key_from_asn1(object, this->set, &this->A))
				{
					goto end;
				}
				this->Ar = malloc(this->set->n * sizeof(uint32_t));
				r2 = this->set->fft_params->r2;

				for (i = 0; i < this->set->n; i++)
				{
					this->Ar[i] = ntt_fft_mreduce(this->A[i] * r2,
												  this->set->fft_params);
				}
				break;
			case PRIV_KEY_SECRET1:
				if (object.len != 1 + (s_bits * this->set->n + 7)/8)
				{
					goto end;
				}
				this->s1 = malloc(this->set->n);

				/* Skip unused bits octet */
				object = chunk_skip(object, 1);
				packer = bliss_bitpacker_create_from_data(object);
				for (i = 0; i < this->set->n; i++)
				{
					packer->read_bits(packer, &value, s_bits);
					s = (value & s_sign) ? value | s_mask : value;
					if (s < s_min || s > s_max)
					{
						packer->destroy(packer);
						goto end;
					}
					this->s1[i] = s;
				}
				packer->destroy(packer);
				break;
			case PRIV_KEY_SECRET2:
				if (object.len != 1 + (s_bits * this->set->n + 7)/8)
				{
					goto end;
				}
				this->s2 = malloc(this->set->n);

				/* Skip unused bits octet */
				object = chunk_skip(object, 1);
				packer = bliss_bitpacker_create_from_data(object);
				for (i = 0; i < this->set->n; i++)
				{
					packer->read_bits(packer, &value, s_bits);
					s = (value & s_sign) ? value | s_mask : value;
					if (s < s_min || s > s_max)
					{
						packer->destroy(packer);
						goto end;
					}
					this->s2[i] = 2 * s;
					if (i == 0)
					{
						this->s2[0] += 1;
					}
				}
				packer->destroy(packer);
				break;
		}
	}
	success = parser->success(parser);

end:
	parser->destroy(parser);
	if (!success)
	{
		destroy(this);
		return NULL;
	}

	return &this->public;
}

