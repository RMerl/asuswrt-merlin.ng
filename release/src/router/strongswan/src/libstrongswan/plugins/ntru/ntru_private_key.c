/*
 * Copyright (C) 2014-2016 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2009-2013  Security Innovation
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

#include "ntru_private_key.h"
#include "ntru_trits.h"
#include "ntru_poly.h"
#include "ntru_convert.h"

#include <utils/debug.h>
#include <utils/test.h>

typedef struct private_ntru_private_key_t private_ntru_private_key_t;

/**
 * Private data of an ntru_private_key_t object.
 */
struct private_ntru_private_key_t {

	/**
	 * Public ntru_private_key_t interface.
	 */
	ntru_private_key_t public;

	/**
	 * NTRU Parameter Set
	 */
	const ntru_param_set_t *params;

	/**
	 * Polynomial F which is the private key
	 */
	ntru_poly_t *privkey;

	/**
	 * Polynomial h which is the public key
	 */
	uint16_t *pubkey;

	/**
	 * Encoding of the private key
	 */
	chunk_t encoding;

	/**
	 * Deterministic Random Bit Generator
	 */
	ntru_drbg_t *drbg;

};

METHOD(ntru_private_key_t, get_id, ntru_param_set_id_t,
	private_ntru_private_key_t *this)
{
	return this->params->id;
}

METHOD(ntru_private_key_t, get_public_key, ntru_public_key_t*,
	private_ntru_private_key_t *this)
{
	return ntru_public_key_create(this->drbg, this->params, this->pubkey);
}

/**
 * Generate NTRU encryption private key encoding
 */
static void generate_encoding(private_ntru_private_key_t *this)
{
	size_t pubkey_len, privkey_len, privkey_trits_len, privkey_indices_len;
	int privkey_pack_type;
	uint16_t *indices;
	uint8_t *trits;
	u_char *enc;

	/* compute public key length encoded as packed coefficients */
	pubkey_len =  (this->params->N * this->params->q_bits + 7) / 8;

	/* compute private key length encoded as packed trits coefficients */
	privkey_trits_len = (this->params->N + 4) / 5;

	/* compute private key length encoded as packed indices */
	privkey_indices_len = (this->privkey->get_size(this->privkey) *
						   this->params->N_bits + 7) / 8;

	if (this->params->is_product_form ||
		privkey_indices_len <= privkey_trits_len)
	{
		privkey_pack_type = NTRU_KEY_PACKED_INDICES;
		privkey_len = privkey_indices_len;
	}
	else
	{
		privkey_pack_type = NTRU_KEY_PACKED_TRITS;
		privkey_len = privkey_trits_len;
       }

	/* allocate memory for private key encoding */
	this->encoding = chunk_alloc(2 + NTRU_OID_LEN + pubkey_len + privkey_len);
	enc = this->encoding.ptr;

	/* format header and packed public key */
	*enc++ = NTRU_PRIVKEY_DEFAULT_TAG;
	*enc++ = NTRU_OID_LEN;
	memcpy(enc, this->params->oid, NTRU_OID_LEN);
	enc += NTRU_OID_LEN;
	ntru_elements_2_octets(this->params->N, this->pubkey,
						   this->params->q_bits, enc);
	enc += pubkey_len;

	/* add packed private key */
	indices = this->privkey->get_indices(this->privkey);

	if (privkey_pack_type == NTRU_KEY_PACKED_TRITS)
	{
		/* encode private key as packed trits */
		trits = malloc(this->params->N);
		ntru_indices_2_packed_trits(indices, this->params->dF_r,
							this->params->dF_r, this->params->N, trits, enc);
		memwipe(trits, this->params->N);
		free(trits);
	}
	else
	{
		/* encode private key as packed indices */
		ntru_elements_2_octets(this->privkey->get_size(this->privkey),
							   indices, this->params->N_bits, enc);
	}
}

METHOD(ntru_private_key_t, get_encoding, chunk_t,
	private_ntru_private_key_t *this)
{
	return this->encoding;
}

/** 
 * Checks that the number of 0, +1, and -1 trinary ring elements meet or exceed
 * a minimum weight.
 *
 * @param N			degree of polynomial
 * @param t			array of trinary ring elements
 * @param min_wt	minimum weight
 * @return			TRUE if minimum weight met or exceeded
 */
bool ntru_check_min_weight(uint16_t N, uint8_t  *t, uint16_t min_wt)
{
	uint16_t wt[3];
	bool success;
	int i;

	wt[0] = wt[1] = wt[2] = 0;

	for (i = 0; i < N; i++)
	{
		++wt[t[i]];
	}
	success = (wt[0] >= min_wt) && (wt[1] >= min_wt) && (wt[2] >= min_wt);

	DBG2(DBG_LIB, "minimum weight = %u, so -1: %u, 0: %u, +1: %u is %sok",
				   min_wt, wt[2], wt[0], wt[1], success ? "" : "not ");

	return success;
}

METHOD(ntru_private_key_t, decrypt, bool,
	private_ntru_private_key_t *this, chunk_t ciphertext, chunk_t *plaintext)
{
	ext_out_function_t alg;
	size_t t_len, seed1_len, seed2_len;
	uint16_t *t1, *t2, *t = NULL;
    uint16_t mod_q_mask, q_mod_p, cmprime_len, cm_len = 0, num_zeros;
	uint8_t *Mtrin, *M, *cm, *mask_trits, *ptr;
	int16_t m1 = 0;
	chunk_t seed = chunk_empty;
	ntru_trits_t *mask;
	ntru_poly_t *r_poly;
	bool msg_rep_good, success = TRUE;
	int i;

	*plaintext = chunk_empty;

	if (ciphertext.len != (this->params->N * this->params->q_bits + 7) / 8)
	{
		DBG1(DBG_LIB, "wrong NTRU ciphertext length");
		return FALSE;
	}

	/* allocate temporary array t */
	t_len  = 2 * this->params->N * sizeof(uint16_t);
	t = malloc(t_len);
	t1 = t;
	t2 = t + this->params->N;
	Mtrin = (uint8_t *)t1;
	M = Mtrin + this->params->N;

	/* set MGF1 algorithm type based on security strength */
	alg = (this->params->sec_strength_len <= 20) ? XOF_MGF1_SHA1 :
												   XOF_MGF1_SHA256;

	/* set constants */
	mod_q_mask = this->params->q - 1;
	q_mod_p = this->params->q % 3;

    /* unpack the ciphertext */
    ntru_octets_2_elements(ciphertext.len, ciphertext.ptr,
						   this->params->q_bits, t2);

	/* form cm':
	 *  F * e
	 *  A = e * (1 + pF) mod q = e + pFe mod q
	 *  a = A in the range [-q/2, q/2)
	 *  cm' = a mod p
	 */
	this->privkey->ring_mult(this->privkey, t2, t1);

	cmprime_len = this->params->N;
	if (this->params->is_product_form)
	{
		--cmprime_len;
		for (i = 0; i < cmprime_len; i++)
		{
			t1[i] = (t2[i] + 3 * t1[i]) & mod_q_mask;
			if (t1[i] >= (this->params->q / 2))
			{
				t1[i] -= q_mod_p;
			}
			Mtrin[i] = (uint8_t)(t1[i] % 3);
			if (Mtrin[i] == 1)
			{
				++m1;
			}
			else if (Mtrin[i] == 2)
			{
				--m1;
			}
		}
	}
	else
	{
		for (i = 0; i < cmprime_len; i++)
		{
			t1[i] = (t2[i] + 3 * t1[i]) & mod_q_mask;
			if (t1[i] >= (this->params->q / 2))
			{
				t1[i] -= q_mod_p;
			}
			Mtrin[i] = (uint8_t)(t1[i] % 3);
		}
	}

    /**
	 * check that the candidate message representative meets
     * minimum weight requirements
     */
	if (this->params->is_product_form)
	{
		msg_rep_good = (abs(m1) <= this->params->min_msg_rep_wt);
	}
	else
	{
		msg_rep_good = ntru_check_min_weight(cmprime_len, Mtrin,
											 this->params->min_msg_rep_wt);
	}
	if (!msg_rep_good)
	{
		DBG1(DBG_LIB, "decryption failed due to insufficient minimum weight");
		success = FALSE;
	}

	/* form cR = e - cm' mod q */
	for (i = 0; i < cmprime_len; i++)
	{
		if (Mtrin[i] == 1)
		{
			t2[i] = (t2[i] - 1) & mod_q_mask;
		}
		else if (Mtrin[i] == 2)
		{
			t2[i] = (t2[i] + 1) & mod_q_mask;
		}
	}
	if (this->params->is_product_form)
	{
		t2[i] = (t2[i] + m1) & mod_q_mask;
	}

	/* allocate memory for the larger of the two seeds */
	seed1_len = (this->params->N + 3)/4;
	seed2_len = 3 + 2*this->params->sec_strength_len + this->params->m_len_max;
	seed = chunk_alloc(max(seed1_len, seed2_len));
	seed.len = seed1_len;

	/* form cR mod 4 */
	ntru_coeffs_mod4_2_octets(this->params->N, t2, seed.ptr);

	/* form mask */
	mask = ntru_trits_create(this->params->N, alg, seed);
	if (!mask)
	{
		DBG1(DBG_LIB, "mask creation failed");
		success = FALSE;
		goto err;
	}

	mask_trits = mask->get_trits(mask);

	/* form cMtrin by subtracting mask from cm', mod p */
	for (i = 0; i < cmprime_len; i++)
	{
		Mtrin[i] -=  mask_trits[i];
		if (Mtrin[i] >= 3)
		{
			Mtrin[i] += 3;
		}
	}
	mask->destroy(mask);

	if (this->params->is_product_form)
	{
		/* set the last trit to zero since that's what it was, and
		 * because it can't be calculated from (cm' - mask) since
		 * we don't have the correct value for the last cm' trit
		 */
		Mtrin[i] = 0;
	}

	/* convert cMtrin to cM (Mtrin to Mbin) */
	if (!ntru_trits_2_bits(Mtrin, this->params->N, M))
	{
		success = FALSE;
		goto err;
	}

	/* skip the random padding */
       ptr = M + this->params->sec_strength_len;

	/* validate the padded message cM and copy cm to m_buf */
	if (this->params->m_len_len == 2)
	{
		cm_len = (uint16_t)(*ptr++) << 16;
	}
	cm_len |= (uint16_t)(*ptr++);

	if (cm_len > this->params->m_len_max)
	{
		cm_len = this->params->m_len_max;
		DBG1(DBG_LIB, "NTRU message length is larger than maximum length");
		success = FALSE;
	}
	cm = ptr;
	ptr += cm_len;

	/* check if the remaining padding consists of zeros */
	num_zeros = this->params->m_len_max - cm_len + 1;
	for (i = 0; i < num_zeros; i++)
	{
		if (ptr[i] != 0)
		{
			DBG1(DBG_LIB, "non-zero trailing padding detected");
			success = FALSE;
			break;
		}
	}

	/* form sData (OID || m || b || hTrunc) */
	ptr = seed.ptr;
	memcpy(ptr, this->params->oid, 3);
	ptr += 3;
	memcpy(ptr, cm, cm_len);
	ptr += cm_len;
	memcpy(ptr, M, this->params->sec_strength_len);
	ptr += this->params->sec_strength_len;
	memcpy(ptr, this->encoding.ptr + 2 + NTRU_OID_LEN,
		   this->params->sec_strength_len);
	ptr += this->params->sec_strength_len;
	seed.len = ptr - seed.ptr;

	/* generate cr */
	DBG2(DBG_LIB, "generate polynomial r");
	r_poly = ntru_poly_create_from_seed(alg, seed, this->params->c_bits,
						this->params->N, this->params->q, this->params->dF_r,
						this->params->dF_r, this->params->is_product_form);
	if (!r_poly)
	{
		success = FALSE;
		goto err;
	}

	/* output plaintext in allocated chunk */
	*plaintext = chunk_clone(chunk_create(cm, cm_len));

	/* form cR' = h * cr */
	r_poly->ring_mult(r_poly, this->pubkey, t1);
	r_poly->destroy(r_poly);

	/* compare cR' to cR */
	for (i = 0; i < this->params->N; i++)
	{
		if (t[i] != t2[i])
		{
			DBG1(DBG_LIB, "cR' does not equal cR'");
			chunk_clear(plaintext);
			success = FALSE;
			break;
		}
	}
	memwipe(t, t_len);

err:
	/* cleanup */
	chunk_clear(&seed);
	free(t);

	return success;
}

METHOD(ntru_private_key_t, destroy, void,
	private_ntru_private_key_t *this)
{
	DESTROY_IF(this->privkey);
	this->drbg->destroy(this->drbg);
	chunk_clear(&this->encoding);
	free(this->pubkey);
	free(this);
}

/**
 * Multiplies ring element (polynomial) "a" by ring element (polynomial) "b"
 * to produce ring element (polynomial) "c" in (Z/qZ)[X]/(X^N - 1).
 * This is a convolution operation.
 *
 * Ring element "b" has coefficients in the range [0,N).
 *
 * This assumes q is 2^r where 8 < r < 16, so that overflow of the sum
 * beyond 16 bits does not matter.
 *
 * @param a		polynomial a
 * @param b		polynomial b
 * @param N		no. of coefficients in a, b, c
 * @param q		large modulus
 * @param c		polynomial c = a * b
 */
static void ring_mult_c(uint16_t *a, uint16_t *b, uint16_t N, uint16_t q,
					    uint16_t *c)
{
	uint16_t *bptr = b;
	uint16_t mod_q_mask = q - 1;
	int i, k;

	/* c[k] = sum(a[i] * b[k-i]) mod q */
	memset(c, 0, N * sizeof(uint16_t));
	for (k = 0; k < N; k++)
	{
		i = 0;
		while (i <= k)
		{
			c[k] += a[i++] * *bptr--;
		}
		bptr += N;
		while (i < N)
		{
			c[k] += a[i++] * *bptr--;
		}
		c[k] &= mod_q_mask;
		++bptr;
	}
}

/**
 * Finds the inverse of a polynomial a in (Z/2^rZ)[X]/(X^N - 1).
 *
 * This assumes q is 2^r where 8 < r < 16, so that operations mod q can
 * wait until the end, and only 16-bit arrays need to be used.
 *
 * @param a			polynomial a
 * @param N			no. of coefficients in a
 * @param q			large modulus
 * @param t			temporary buffer of size 2N elements
 * @param a_inv 	polynomial for inverse of a
 */
static bool ring_inv(uint16_t *a, uint16_t N, uint16_t q, uint16_t *t,
					 uint16_t *a_inv)
{
	uint8_t *b = (uint8_t *)t;
	uint8_t *c = b + N;
	uint8_t *f = c + N;
	uint8_t *g = (uint8_t *)a_inv;
	uint16_t *t2 = t + N;
	uint16_t deg_b, deg_c, deg_f, deg_g;
    bool done = FALSE;
    int i, j, k = 0;

	/* form a^-1 in (Z/2Z)[X]/X^N - 1) */
	memset(b, 0, 2 * N);					/* clear to init b, c */

	/* b(X) = 1 */
	b[0] = 1;
	deg_b = 0;

	/* c(X) = 0 (cleared above) */
	deg_c = 0;

	/* f(X) = a(X) mod 2 */
	for (i = 0; i < N; i++)
	{
		f[i] = (uint8_t)(a[i] & 1);
	}
	deg_f = N - 1;

	/* g(X) = X^N - 1 */
	g[0] = 1;
	memset(g + 1, 0, N - 1);
	g[N] = 1;
	deg_g = N;

	/* until f(X) = 1 */
	while (!done)
	{
		/* while f[0] = 0, f(X) /= X, c(X) *= X, k++ */
		for (i = 0; (i <= deg_f) && (f[i] == 0); ++i);

		if (i > deg_f)
		{
			return FALSE;
		}
		if (i)
		{
			f = f + i;
			deg_f = deg_f - i;
			deg_c = deg_c + i;
			for (j = deg_c; j >= i; j--)
			{
				c[j] = c[j-i];
			}
			for (j = 0; j < i; j++)
			{
				c[j] = 0;
			}
			k = k + i;
		}

		/* adjust degree of f(X) if the highest coefficients are zero
		 * Note: f[0] = 1 from above so the loop will terminate.
		 */
		while (f[deg_f] == 0)
		{
			--deg_f;
		}

		/* if f(X) = 1, done
		 * Note: f[0] = 1 from above, so only check the x term and up
		 */
		for (i = 1; (i <= deg_f) && (f[i] == 0); ++i);

		if (i > deg_f)
		{
			done = TRUE;
			break;
		}

		/* if deg_f < deg_g, f <-> g, b <-> c */
		if (deg_f < deg_g)
		{
			uint8_t *x;

			x = f;
			f = g;
			g = x;
			deg_f ^= deg_g;
			deg_g ^= deg_f;
			deg_f ^= deg_g;
			x = b;
			b = c;
			c = x;
			deg_b ^= deg_c;
			deg_c ^= deg_b;
			deg_b ^= deg_c;
		}

		/* f(X) += g(X), b(X) += c(X) */
		for (i = 0; i <= deg_g; i++)
		{
			f[i] ^= g[i];
		}
		if (deg_c > deg_b)
		{	
			deg_b = deg_c;
		}
		for (i = 0; i <= deg_c; i++)
		{
			b[i] ^= c[i];
		}
	}

	/* a^-1 in (Z/2Z)[X]/(X^N - 1) = b(X) shifted left k coefficients */
	j = 0;
	if (k >= N)
	{
		k = k - N;
	}
	for (i = k; i < N; i++)
	{
		a_inv[j++] = (uint16_t)(b[i]);
	}
	for (i = 0; i < k; i++)
	{
		a_inv[j++] = (uint16_t)(b[i]);
	}

	/* lift a^-1 in (Z/2Z)[X]/(X^N - 1) to a^-1 in (Z/qZ)[X]/(X^N -1) */
    for (j = 0; j < 4; ++j)				/* assumes 256 < q <= 65536 */
	{
		/* a^-1 = a^-1 * (2 - a * a^-1) mod q */
		memcpy(t2, a_inv, N * sizeof(uint16_t));
		ring_mult_c(a, t2, N, q, t);
		for (i = 0; i < N; ++i)
		{
			t[i] = q - t[i];
		}
		t[0] = t[0] + 2;
		ring_mult_c(t2, t, N, q, a_inv);
	}
	
	return TRUE;
}

/*
 * Described in header.
 */
ntru_private_key_t *ntru_private_key_create(ntru_drbg_t *drbg,
											const ntru_param_set_t *params)
{
	private_ntru_private_key_t *this;
	size_t t_len;
	uint16_t *t1, *t2, *t = NULL;
	uint16_t mod_q_mask;
    ext_out_function_t alg;
	ntru_poly_t *g_poly;
	chunk_t	seed;
	int i;

	INIT(this,
		.public = {
			.get_id = _get_id,
			.get_public_key = _get_public_key,
			.get_encoding = _get_encoding,
			.decrypt = _decrypt,
			.destroy = _destroy,
		},
		.params = params,
		.pubkey = malloc(params->N * sizeof(uint16_t)),
		.drbg = drbg->get_ref(drbg),
	);

	/* set hash algorithm and seed length based on security strength */
	alg = (params->sec_strength_len <= 20) ? XOF_MGF1_SHA1 :
											 XOF_MGF1_SHA256;
	seed =chunk_alloc(params->sec_strength_len + 8);

	/* get random seed for generating trinary F as a list of indices */
	if (!drbg->generate(drbg, params->sec_strength_len * BITS_PER_BYTE,
							  seed.len, seed.ptr))
	{
		goto err;
	}

	DBG2(DBG_LIB, "generate polynomial F");
	this->privkey = ntru_poly_create_from_seed(alg, seed, params->c_bits,
											   params->N, params->q,
											   params->dF_r, params->dF_r,
											   params->is_product_form);
	if (!this->privkey)
	{
		goto err;
	}

	/* allocate temporary array t */
	t_len = 3 * params->N * sizeof(uint16_t);
	t = malloc(t_len);
	t1 = t + 2 * params->N;

	/* extend sparse private key polynomial f to N array elements */ 
	this->privkey->get_array(this->privkey, t1);

	/* set mask for large modulus */
	mod_q_mask = params->q - 1;

	/* form f = 1 + pF */
	for (i = 0; i < params->N; i++)
	{
		t1[i] = (t1[i] * 3) & mod_q_mask;
	}
	t1[0] = (t1[0] + 1) & mod_q_mask;

	/* use the public key array as a temporary buffer */
	t2 = this->pubkey;
 
	/* find f^-1 in (Z/qZ)[X]/(X^N - 1) */
	if (!ring_inv(t1, params->N, params->q, t, t2))
	{
		goto err;
	}

	/* get random seed for generating trinary g as a list of indices */
 	if (!drbg->generate(drbg, params->sec_strength_len * BITS_PER_BYTE,
							  seed.len, seed.ptr))
	{
		goto err;
	}

	DBG2(DBG_LIB, "generate polynomial g");
	g_poly = ntru_poly_create_from_seed(alg, seed, params->c_bits,
										params->N, params->q, params->dg + 1,
										params->dg, FALSE);
	if (!g_poly)
	{
		goto err;
	}

	/* compute public key polynomial h = p * (f^-1 * g) mod q */
	g_poly->ring_mult(g_poly, t2, t2);
	g_poly->destroy(g_poly);

	for (i = 0; i < params->N; i++)
	{
		this->pubkey[i] = (t2[i] * 3) & mod_q_mask;
	}

	/* cleanup temporary storage */
	chunk_clear(&seed);
	memwipe(t, t_len);
	free(t);
	
	/* generate private key encoding */
	generate_encoding(this);

	return &this->public;

err:
	chunk_free(&seed);
	free(t);
	destroy(this);

	return NULL;
}

/*
 * Described in header.
 */
ntru_private_key_t *ntru_private_key_create_from_data(ntru_drbg_t *drbg,
													  chunk_t data)
{
	private_ntru_private_key_t *this;
	size_t header_len, pubkey_packed_len, privkey_packed_len;
	size_t privkey_packed_trits_len, privkey_packed_indices_len;
	uint8_t *privkey_packed, tag;
	uint16_t *indices, dF;
	const ntru_param_set_t *params;

	header_len = 2 + NTRU_OID_LEN;

	/* check the NTRU public key header format */
	if (data.len < header_len ||
		!(data.ptr[0] == NTRU_PRIVKEY_DEFAULT_TAG ||
		  data.ptr[0] == NTRU_PRIVKEY_TRITS_TAG ||
		  data.ptr[0] == NTRU_PRIVKEY_INDICES_TAG) ||
		data.ptr[1] != NTRU_OID_LEN)
	{
		DBG1(DBG_LIB, "loaded NTRU private key with invalid header");
		return NULL;
	}
	tag = data.ptr[0];
	params = ntru_param_set_get_by_oid(data.ptr + 2);

	if (!params)
	{
		DBG1(DBG_LIB, "loaded NTRU private key with unknown OID");
		return NULL;
	}

	pubkey_packed_len = (params->N * params->q_bits + 7) / 8;
	privkey_packed_trits_len = (params->N + 4) / 5;

	/* check packing type for product-form private keys */
	if (params->is_product_form &&  tag == NTRU_PRIVKEY_TRITS_TAG)
	{
		DBG1(DBG_LIB, "a product-form NTRU private key cannot be trits-encoded");
		return NULL;
	}

	/* set packed-key length for packed indices */
	if (params->is_product_form)
	{
		dF = (uint16_t)((params->dF_r & 0xff) +           /* df1 */
					   ((params->dF_r >>  8) & 0xff) +    /* df2 */
					   ((params->dF_r >> 16) & 0xff));    /* df3 */
	}
	else
	{
		dF = (uint16_t)params->dF_r;
	}
	privkey_packed_indices_len = (2 * dF * params->N_bits + 7) / 8;

	/* set private-key packing type if defaulted */
	if (tag == NTRU_PRIVKEY_DEFAULT_TAG)
	{
		if (params->is_product_form ||
            privkey_packed_indices_len <= privkey_packed_trits_len)
		{
			tag = NTRU_PRIVKEY_INDICES_TAG;
		}		
		else
		{
			tag = NTRU_PRIVKEY_TRITS_TAG;
		}
	}
	privkey_packed_len = (tag == NTRU_PRIVKEY_TRITS_TAG) ?
                		 privkey_packed_trits_len : privkey_packed_indices_len;

	if (data.len < header_len + pubkey_packed_len + privkey_packed_len)
	{
		DBG1(DBG_LIB, "loaded NTRU private key with wrong packed key size");
		return NULL;
	}

	INIT(this,
		.public = {
			.get_id = _get_id,
			.get_public_key = _get_public_key,
			.get_encoding = _get_encoding,
			.decrypt = _decrypt,
			.destroy = _destroy,
		},
		.params = params,
		.pubkey = malloc(params->N * sizeof(uint16_t)),
		.encoding = chunk_clone(data),
		.drbg = drbg->get_ref(drbg),
	);

	/* unpack the encoded public key */
	ntru_octets_2_elements(pubkey_packed_len, data.ptr + header_len,
						   params->q_bits, this->pubkey);

	/* allocate temporary memory for indices */
	indices = malloc(2 * dF * sizeof(uint16_t));

	/* unpack the private key */
	privkey_packed = data.ptr + header_len + pubkey_packed_len;	
	if (tag == NTRU_PRIVKEY_TRITS_TAG)
	{
		ntru_packed_trits_2_indices(privkey_packed, params->N,
									indices, indices + dF);
    }
	else
	{
        ntru_octets_2_elements(privkey_packed_indices_len, privkey_packed,
							   params->N_bits, indices);
    }
	this->privkey = ntru_poly_create_from_data(indices, params->N, params->q,
											   params->dF_r, params->dF_r,
											   params->is_product_form);

	/* cleanup */
	memwipe(indices, 2 * dF * sizeof(uint16_t));
	free(indices);

	return &this->public;
}

EXPORT_FUNCTION_FOR_TESTS(ntru, ntru_private_key_create);

EXPORT_FUNCTION_FOR_TESTS(ntru, ntru_private_key_create_from_data);
