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

#include "ntru_public_key.h"
#include "ntru_trits.h"
#include "ntru_poly.h"
#include "ntru_convert.h"

#include <utils/debug.h>
#include <utils/test.h>

typedef struct private_ntru_public_key_t private_ntru_public_key_t;

/**
 * Private data of an ntru_public_key_t object.
 */
struct private_ntru_public_key_t {
	/**
	 * Public ntru_public_key_t interface.
	 */
	ntru_public_key_t public;

	/**
	 * NTRU Parameter Set
	 */
	const ntru_param_set_t *params;

	/**
	 * Polynomial h which is the public key
	 */
	uint16_t *pubkey;

	/**
	 * Encoding of the public key
	 */
	chunk_t encoding;

	/**
	 * Deterministic Random Bit Generator
	 */
	ntru_drbg_t *drbg;

};

METHOD(ntru_public_key_t, get_id, ntru_param_set_id_t,
	private_ntru_public_key_t *this)
{
	return this->params->id;
}

/**
 * Generate NTRU encryption public key encoding
 */
static void generate_encoding(private_ntru_public_key_t *this)
{
	size_t pubkey_len;
	u_char *enc;

	/* compute public key length encoded as packed coefficients */
	pubkey_len =  (this->params->N * this->params->q_bits + 7) / 8;

	/* allocate memory for public key encoding */
	this->encoding = chunk_alloc(2 + NTRU_OID_LEN + pubkey_len);
	enc = this->encoding.ptr;

	/* format header and packed public key */
	*enc++ = NTRU_PUBKEY_TAG;
	*enc++ = NTRU_OID_LEN;
	memcpy(enc, this->params->oid, NTRU_OID_LEN);
	enc += NTRU_OID_LEN;
	ntru_elements_2_octets(this->params->N, this->pubkey,
						   this->params->q_bits, enc);
}

METHOD(ntru_public_key_t, get_encoding, chunk_t,
	private_ntru_public_key_t *this)
{
	return this->encoding;
}

#define MAX_SEC_STRENGTH_LEN	32 /* bytes */

/**
 * Shared with ntru_private_key.c
 */
extern bool ntru_check_min_weight(uint16_t N, uint8_t  *t, uint16_t min_wt);

METHOD(ntru_public_key_t, encrypt, bool,
	private_ntru_public_key_t *this, chunk_t plaintext, chunk_t *ciphertext)
{
	ext_out_function_t alg;
	size_t t_len, seed1_len, seed2_len;
	uint16_t *t1, *t = NULL;
	uint8_t b[MAX_SEC_STRENGTH_LEN];
	uint8_t *t2, *Mtrin, *M, *mask_trits, *ptr;
	uint16_t mod_q_mask, mprime_len = 0;
	int16_t m1 = 0;
	chunk_t seed = chunk_empty;
	ntru_trits_t *mask;
	ntru_poly_t *r_poly;
	bool msg_rep_good, success = FALSE;
	int i;

	*ciphertext = chunk_empty;

	if (plaintext.len > this->params->m_len_max)
	{
		DBG1(DBG_LIB, "plaintext exceeds maximum size");
		return FALSE;
	}

	if (this->params->sec_strength_len > MAX_SEC_STRENGTH_LEN)
	{
		DBG1(DBG_LIB, "required security strength exceeds %d bits",
			 MAX_SEC_STRENGTH_LEN * BITS_PER_BYTE);
		return FALSE;
	}

	/* allocate temporary array t */
	t_len  = (sizeof(uint16_t) + 3*sizeof(uint8_t)) * this->params->N;
	t = malloc(t_len);
	t1 = t;
	t2 = (uint8_t *)(t1 + this->params->N);
	Mtrin = t2 + this->params->N;
	M = Mtrin + this->params->N;

	/* set hash algorithm based on security strength */
	alg = (this->params->sec_strength_len <= 20) ? XOF_MGF1_SHA1 :
												   XOF_MGF1_SHA256;
	/* set constants */
	mod_q_mask = this->params->q - 1;

	/* allocate memory for the larger of the two seeds */
	seed1_len = (this->params->N + 3)/4;
	seed2_len = 3 + 2*this->params->sec_strength_len + plaintext.len;
	seed = chunk_alloc(max(seed1_len, seed2_len));

	/* loop until a message representative with proper weight is achieved */
	do
	{
		if (!this->drbg->generate(this->drbg,
								  this->params->sec_strength_len * BITS_PER_BYTE,
								  this->params->sec_strength_len, b))
		{
			goto err;
		}

		/* form sData (OID || m || b || hTrunc) */
		ptr = seed.ptr;
		memcpy(ptr, this->params->oid, NTRU_OID_LEN);
		ptr += NTRU_OID_LEN;
		memcpy(ptr, plaintext.ptr, plaintext.len);
		ptr += plaintext.len;
		memcpy(ptr, b, this->params->sec_strength_len);
		ptr += this->params->sec_strength_len;
		memcpy(ptr, this->encoding.ptr + 2 + NTRU_OID_LEN,
			   this->params->sec_strength_len);
		ptr += this->params->sec_strength_len;
		seed.len = seed2_len;

		DBG2(DBG_LIB, "generate polynomial r");
		r_poly = ntru_poly_create_from_seed(alg, seed, this->params->c_bits,
											this->params->N, this->params->q,
											this->params->dF_r, this->params->dF_r,
											this->params->is_product_form);
		if (!r_poly)
		{
		   goto err;
		}

		/* form R = h * r */
		r_poly->ring_mult(r_poly, this->pubkey, t1);
		r_poly->destroy(r_poly);

		/* form R mod 4 */
		ntru_coeffs_mod4_2_octets(this->params->N, t1, seed.ptr);
		seed.len = seed1_len;

		/* form mask */
		mask = ntru_trits_create(this->params->N, alg, seed);
		if (!mask)
		{
			DBG1(DBG_LIB, "mask creation failed");
			goto err;
		}

		/* form the padded message M */
		ptr = M;
		memcpy(ptr, b, this->params->sec_strength_len);
		ptr += this->params->sec_strength_len;
		if (this->params->m_len_len == 2)
		{
			*ptr++ = (uint8_t)((plaintext.len >> 8) & 0xff);
		}
		*ptr++ = (uint8_t)(plaintext.len & 0xff);
		memcpy(ptr, plaintext.ptr, plaintext.len);
		ptr += plaintext.len;

		/* add an extra zero byte in case without it the bit string
		 * is not a multiple of 3 bits and therefore might not be
		 * able to produce enough trits
		 */
		memset(ptr, 0, this->params->m_len_max - plaintext.len + 2);

		/* convert M to trits (Mbin to Mtrin) */
		mprime_len = this->params->N;
		if (this->params->is_product_form)
		{
			--mprime_len;
		}
		ntru_bits_2_trits(M, mprime_len, Mtrin);
		mask_trits = mask->get_trits(mask);


		/* form the msg representative m' by adding Mtrin to mask, mod p */
		if (this->params->is_product_form)
		{
			m1 = 0;
			for (i = 0; i < mprime_len; i++)
			{
				t2[i] = mask_trits[i] + Mtrin[i];
				if (t2[i] >= 3)
				{
					t2[i] -= 3;
				}
				if (t2[i] == 1)
				{
					++m1;
				}
				else if (t2[i] == 2)
				{
					--m1;
				}
			}
		}
		else
		{
			for (i = 0; i < mprime_len; i++)
			{
				t2[i] = mask_trits[i] + Mtrin[i];
				if (t2[i] >= 3)
				{
					t2[i] -= 3;
				}
			}
		}
		mask->destroy(mask);

		/* check that message representative meets minimum weight
		 * requirements
		 */
		if (this->params->is_product_form)
		{
			msg_rep_good = (abs(m1) <= this->params->min_msg_rep_wt);
		}
		else
		{
			msg_rep_good = ntru_check_min_weight(mprime_len, t2,
												 this->params->min_msg_rep_wt);
		}
	}
	while (!msg_rep_good);

	/* form ciphertext e by adding m' to R mod q */
	for (i = 0; i < mprime_len; i++)
	{
		if (t2[i] == 1)
		{
			t1[i] = (t1[i] + 1) & mod_q_mask;
		}
		else if (t2[i] == 2)
		{
			t1[i] = (t1[i] - 1) & mod_q_mask;
		}
	}
	if (this->params->is_product_form)
	{
		t1[i] = (t1[i] - m1) & mod_q_mask;
	}

	/* pack ciphertext */
	*ciphertext = chunk_alloc((this->params->N * this->params->q_bits + 7) / 8);
	ntru_elements_2_octets(this->params->N, t1, this->params->q_bits,
						   ciphertext->ptr);

	memwipe(t, t_len);
	success = TRUE;

err:
	/* cleanup */
	chunk_clear(&seed);
	free(t);

	return success;
}
METHOD(ntru_public_key_t, destroy, void,
	private_ntru_public_key_t *this)
{
	this->drbg->destroy(this->drbg);
	chunk_clear(&this->encoding);
	free(this->pubkey);
	free(this);
}

/*
 * Described in header.
 */
ntru_public_key_t *ntru_public_key_create(ntru_drbg_t *drbg,
										  const ntru_param_set_t *params,
										  uint16_t *pubkey)
{
	private_ntru_public_key_t *this;
	int i;

	INIT(this,
		.public = {
			.get_id = _get_id,
			.get_encoding = _get_encoding,
			.encrypt = _encrypt,
			.destroy = _destroy,
		},
		.params = params,
		.pubkey = malloc(params->N * sizeof(uint16_t)),
		.drbg = drbg->get_ref(drbg),
	);

	for (i = 0; i < params->N; i++)
	{
		this->pubkey[i] = pubkey[i];
	}

	/* generate public key encoding */
	generate_encoding(this);

	return &this->public;
}

/*
 * Described in header.
 */
ntru_public_key_t *ntru_public_key_create_from_data(ntru_drbg_t *drbg,
													chunk_t data)
{
	private_ntru_public_key_t *this;
	size_t header_len, pubkey_packed_len;
	const ntru_param_set_t *params;

	header_len = 2 + NTRU_OID_LEN;

	/* check the NTRU public key header format */
	if (data.len < header_len ||
		data.ptr[0] != NTRU_PUBKEY_TAG ||
		data.ptr[1] != NTRU_OID_LEN)
	{
		DBG1(DBG_LIB, "received NTRU public key with invalid header");
		return NULL;
	}
	params =  ntru_param_set_get_by_oid(data.ptr + 2);

	if (!params)
	{
		DBG1(DBG_LIB, "received NTRU public key with unknown OID");
		return NULL;
	}

	pubkey_packed_len = (params->N * params->q_bits + 7) / 8;

	if (data.len < header_len + pubkey_packed_len)
	{
		DBG1(DBG_LIB, "received NTRU public key with wrong packed key size");
		return NULL;
	}

	INIT(this,
		.public = {
			.get_id = _get_id,
			.get_encoding = _get_encoding,
			.encrypt = _encrypt,
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

	return &this->public;
}

EXPORT_FUNCTION_FOR_TESTS(ntru, ntru_public_key_create_from_data);
