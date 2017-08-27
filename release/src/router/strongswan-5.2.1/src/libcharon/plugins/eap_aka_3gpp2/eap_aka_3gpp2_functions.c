/*
 * Copyright (C) 2008-2009 Martin Willi
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

#include "eap_aka_3gpp2_functions.h"

#include <gmp.h>
#include <limits.h>

#include <daemon.h>

typedef struct private_eap_aka_3gpp2_functions_t private_eap_aka_3gpp2_functions_t;

/**
 * Private data of an eap_aka_3gpp2_functions_t object.
 */
struct private_eap_aka_3gpp2_functions_t {

	/**
	 * Public eap_aka_3gpp2_functions_t interface.
	 */
	eap_aka_3gpp2_functions_t public;

	/**
	 * Used keyed SHA1 function, as PRF
	 */
	prf_t *prf;
};

#define AKA_PAYLOAD_LEN 64

#define F1			  0x42
#define F1STAR		  0x43
#define F2			  0x44
#define F3			  0x45
#define F4			  0x46
#define F5			  0x47
#define F5STAR		  0x48

/** Family key, as proposed in S.S0055 */
static chunk_t fmk = chunk_from_chars(0x41, 0x48, 0x41, 0x47);

/**
 * Binary represnation of the polynom T^160 + T^5 + T^3 + T^2 + 1
 */
static u_int8_t g[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x2d
};

/**
 * Predefined random bits from the RAND Corporation book
 */
static u_int8_t a[] = {
	0x9d, 0xe9, 0xc9, 0xc8, 0xef, 0xd5, 0x78, 0x11,
	0x48, 0x23, 0x14, 0x01, 0x90, 0x1f, 0x2d, 0x49,
	0x3f, 0x4c, 0x63, 0x65
};

/**
 * Predefined random bits from the RAND Corporation book
 */
static u_int8_t b[] = {
	0x75, 0xef, 0xd1, 0x5c, 0x4b, 0x8f, 0x8f, 0x51,
	0x4e, 0xf3, 0xbc, 0xc3, 0x79, 0x4a, 0x76, 0x5e,
	0x7e, 0xec, 0x45, 0xe0
};

/**
 * Multiplicate two mpz_t with bits interpreted as polynoms.
 */
static void mpz_mul_poly(mpz_t r, mpz_t a, mpz_t b)
{
	mpz_t bm, rm;
	int current = 0, shifted = 0, shift;

	mpz_init_set(bm, b);
	mpz_init_set_ui(rm, 0);
	/* scan through a, for each found bit: */
	while ((current = mpz_scan1(a, current)) != ULONG_MAX)
	{
		/* XOR shifted b into r */
		shift = current - shifted;
		mpz_mul_2exp(bm, bm, shift);
		shifted += shift;
		mpz_xor(rm, rm, bm);
		current++;
	}

	mpz_swap(r, rm);
	mpz_clear(rm);
	mpz_clear(bm);
}

/**
 * Calculate the sum of a + b interpreted as polynoms.
 */
static void mpz_add_poly(mpz_t res, mpz_t a, mpz_t b)
{
	/* addition of polynominals is just the XOR */
	mpz_xor(res, a, b);
}

/**
 * Calculate the remainder of a/b interpreted as polynoms.
 */
static void mpz_mod_poly(mpz_t r, mpz_t a, mpz_t b)
{
	/* Example:
	 * a = 10001010
	 * b = 00000101
	 */
	int a_bit, b_bit, diff;
	mpz_t bm, am;

	mpz_init_set(am, a);
	mpz_init(bm);

	a_bit = mpz_sizeinbase(a, 2);
	b_bit = mpz_sizeinbase(b, 2);

	/* don't do anything if b > a */
	if (a_bit >= b_bit)
	{
		/* shift b left to align up most signaficant "1" to a:
		 * a = 10001010
		 * b = 10100000
		 */
		mpz_mul_2exp(bm, b, a_bit - b_bit);
		do
		{
			/* XOR b into a, this kills the most significant "1":
			 * a = 00101010
			 */
			mpz_xor(am, am, bm);
			/* find the next most significant "1" in a, and align up b:
			 * a = 00101010
			 * b = 00101000
			 */
			diff = a_bit - mpz_sizeinbase(am, 2);
			mpz_div_2exp(bm, bm, diff);
			a_bit -= diff;
		}
		while (b_bit <= mpz_sizeinbase(bm, 2));
		/* While b is not shifted to its original value */
	}
	/* after another iteration:
	 * a = 00000010
	 * which is the polynomial modulo
	 */

	mpz_swap(r, am);
	mpz_clear(am);
	mpz_clear(bm);
}

/**
 * Step 3 of the various fx() functions:
 * XOR the key into the SHA1 IV
 */
static bool step3(prf_t *prf, u_char k[AKA_K_LEN],
				  u_char payload[AKA_PAYLOAD_LEN], u_int8_t h[HASH_SIZE_SHA1])
{
	/* use the keyed hasher to build the hash */
	return prf->set_key(prf, chunk_create(k, AKA_K_LEN)) &&
		   prf->get_bytes(prf, chunk_create(payload, AKA_PAYLOAD_LEN), h);
}

/**
 * Step 4 of the various fx() functions:
 * Polynomial whiten calculations
 */
static void step4(u_char x[HASH_SIZE_SHA1])
{
	mpz_t xm, am, bm, gm;

	mpz_init(xm);
	mpz_init(am);
	mpz_init(bm);
	mpz_init(gm);

	mpz_import(xm, HASH_SIZE_SHA1, 1, 1, 1, 0, x);
	mpz_import(am, sizeof(a), 1, 1, 1, 0, a);
	mpz_import(bm, sizeof(b), 1, 1, 1, 0, b);
	mpz_import(gm, sizeof(g), 1, 1, 1, 0, g);

	mpz_mul_poly(xm, am, xm);
	mpz_add_poly(xm, bm, xm);
	mpz_mod_poly(xm, xm, gm);

	mpz_export(x, NULL, 1, HASH_SIZE_SHA1, 1, 0, xm);

	mpz_clear(xm);
	mpz_clear(am);
	mpz_clear(bm);
	mpz_clear(gm);
}

/**
 * Calculation function for f2(), f3(), f4()
 */
static bool fx(prf_t *prf, u_char f, u_char k[AKA_K_LEN],
			   u_char rand[AKA_RAND_LEN], u_char out[AKA_MAC_LEN])
{
	u_char payload[AKA_PAYLOAD_LEN];
	u_char h[HASH_SIZE_SHA1];
	u_char i;

	for (i = 0; i < 2; i++)
	{
		memset(payload, 0x5c, AKA_PAYLOAD_LEN);
		payload[11] ^= f;
		memxor(payload + 12, fmk.ptr, fmk.len);
		memxor(payload + 24, rand, AKA_RAND_LEN);

		payload[3]  ^= i;
		payload[19] ^= i;
		payload[35] ^= i;
		payload[51] ^= i;

		if (!step3(prf, k, payload, h))
		{
			return FALSE;
		}
		step4(h);
		memcpy(out + i * 8, h, 8);
	}
	return TRUE;
}

/**
 * Calculation function of f1() and f1star()
 */
static bool f1x(prf_t *prf, u_int8_t f, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char sqn[AKA_SQN_LEN],
				u_char amf[AKA_AMF_LEN], u_char mac[AKA_MAC_LEN])
{
	/* generate MAC = f1(FMK, SQN, RAND, AMF)
	 * K is loaded into hashers IV; FMK, RAND, SQN, AMF are XORed in a 512-bit
	 * payload which gets hashed
	 */
	u_char payload[AKA_PAYLOAD_LEN];
	u_char h[HASH_SIZE_SHA1];

	memset(payload, 0x5c, AKA_PAYLOAD_LEN);
	payload[11] ^= f;
	memxor(payload + 12, fmk.ptr, fmk.len);
	memxor(payload + 16, rand, AKA_RAND_LEN);
	memxor(payload + 34, sqn, AKA_SQN_LEN);
	memxor(payload + 42, amf, AKA_AMF_LEN);

	if (!step3(prf, k, payload, h))
	{
		return FALSE;
	}
	step4(h);
	memcpy(mac, h, AKA_MAC_LEN);
	return TRUE;
}

/**
 * Calculation function of f5() and f5star()
 */
static bool f5x(prf_t *prf, u_char f, u_char k[AKA_K_LEN],
				u_char rand[AKA_RAND_LEN], u_char ak[AKA_AK_LEN])
{
	u_char payload[AKA_PAYLOAD_LEN];
	u_char h[HASH_SIZE_SHA1];

	memset(payload, 0x5c, AKA_PAYLOAD_LEN);
	payload[11] ^= f;
	memxor(payload + 12, fmk.ptr, fmk.len);
	memxor(payload + 16, rand, AKA_RAND_LEN);

	if (!step3(prf, k, payload, h))
	{
		return FALSE;
	}
	step4(h);
	memcpy(ak, h, AKA_AK_LEN);
	return TRUE;
}

/**
 * Calculate MAC from RAND, SQN, AMF using K
 */
METHOD(eap_aka_3gpp2_functions_t, f1, bool,
	private_eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
	u_char rand[AKA_RAND_LEN], u_char sqn[AKA_SQN_LEN],
	u_char amf[AKA_AMF_LEN], u_char mac[AKA_MAC_LEN])
{
	if (f1x(this->prf, F1, k, rand, sqn, amf, mac))
	{
		DBG3(DBG_IKE, "MAC %b", mac, AKA_MAC_LEN);
		return TRUE;
	}
	return FALSE;
}

/**
 * Calculate MACS from RAND, SQN, AMF using K
 */
METHOD(eap_aka_3gpp2_functions_t, f1star, bool,
	private_eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
	u_char rand[AKA_RAND_LEN], u_char sqn[AKA_SQN_LEN],
	u_char amf[AKA_AMF_LEN], u_char macs[AKA_MAC_LEN])
{
	if (f1x(this->prf, F1STAR, k, rand, sqn, amf, macs))
	{
		DBG3(DBG_IKE, "MACS %b", macs, AKA_MAC_LEN);
		return TRUE;
	}
	return FALSE;
}

/**
 * Calculate RES from RAND using K
 */
METHOD(eap_aka_3gpp2_functions_t, f2, bool,
	private_eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
	u_char rand[AKA_RAND_LEN], u_char res[AKA_RES_MAX])
{
	if (fx(this->prf, F2, k, rand, res))
	{
		DBG3(DBG_IKE, "RES %b", res, AKA_RES_MAX);
		return TRUE;
	}
	return FALSE;
}

/**
 * Calculate CK from RAND using K
 */
METHOD(eap_aka_3gpp2_functions_t, f3, bool,
	private_eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
	u_char rand[AKA_RAND_LEN], u_char ck[AKA_CK_LEN])
{
	if (fx(this->prf, F3, k, rand, ck))
	{
		DBG3(DBG_IKE, "CK %b", ck, AKA_CK_LEN);
		return TRUE;
	}
	return FALSE;
}

/**
 * Calculate IK from RAND using K
 */
METHOD(eap_aka_3gpp2_functions_t, f4, bool,
	private_eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
	u_char rand[AKA_RAND_LEN], u_char ik[AKA_IK_LEN])
{
	if (fx(this->prf, F4, k, rand, ik))
	{
		DBG3(DBG_IKE, "IK %b", ik, AKA_IK_LEN);
		return TRUE;
	}
	return FALSE;
}

/**
 * Calculate AK from a RAND using K
 */
METHOD(eap_aka_3gpp2_functions_t, f5, bool,
	private_eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
	u_char rand[AKA_RAND_LEN], u_char ak[AKA_AK_LEN])
{
	if (f5x(this->prf, F5, k, rand, ak))
	{
		DBG3(DBG_IKE, "AK %b", ak, AKA_AK_LEN);
		return TRUE;
	}
	return FALSE;
}

/**
 * Calculate AKS from a RAND using K
 */
METHOD(eap_aka_3gpp2_functions_t, f5star, bool,
	private_eap_aka_3gpp2_functions_t *this, u_char k[AKA_K_LEN],
	u_char rand[AKA_RAND_LEN], u_char aks[AKA_AK_LEN])
{
	if (f5x(this->prf, F5STAR, k, rand, aks))
	{
		DBG3(DBG_IKE, "AKS %b", aks, AKA_AK_LEN);
		return TRUE;
	}
	return FALSE;
}

METHOD(eap_aka_3gpp2_functions_t, destroy, void,
	private_eap_aka_3gpp2_functions_t *this)
{
	this->prf->destroy(this->prf);
	free(this);
}

/**
 * See header
 */
eap_aka_3gpp2_functions_t *eap_aka_3gpp2_functions_create()
{
	private_eap_aka_3gpp2_functions_t *this;

	INIT(this,
		.public = {
			.f1 = _f1,
			.f1star = _f1star,
			.f2 = _f2,
			.f3 = _f3,
			.f4 = _f4,
			.f5 = _f5,
			.f5star = _f5star,
			.destroy = _destroy,
		},
		.prf = lib->crypto->create_prf(lib->crypto, PRF_KEYED_SHA1),
	);
	if (!this->prf)
	{
		DBG1(DBG_CFG, "%N not supported, unable to use 3GPP2 algorithm",
			 pseudo_random_function_names, PRF_KEYED_SHA1);
		free(this);
		return NULL;
	}
	return &this->public;
}

