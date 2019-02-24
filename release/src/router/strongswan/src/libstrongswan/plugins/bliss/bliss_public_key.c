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

#include "bliss_public_key.h"
#include "bliss_signature.h"
#include "bliss_bitpacker.h"
#include "ntt_fft.h"
#include "ntt_fft_reduce.h"
#include "bliss_utils.h"

#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <asn1/oid.h>

typedef struct private_bliss_public_key_t private_bliss_public_key_t;

/**
 * Private data structure with signing context.
 */
struct private_bliss_public_key_t {
	/**
	 * Public interface for this signer.
	 */
	bliss_public_key_t public;

	/**
	 * BLISS signature parameter set
	 */
	const bliss_param_set_t *set;

	/**
	 * NTT of BLISS public key a (coefficients of polynomial (2g + 1)/f)
	 */
	uint32_t *A;

	/**
	 * NTT of BLISS public key in Montgomery representation Ar = rA mod
	 */
	uint32_t *Ar;

	/**
	 * reference counter
	 */
	refcount_t ref;
};

METHOD(public_key_t, get_type, key_type_t,
	private_bliss_public_key_t *this)
{
	return KEY_BLISS;
}

/**
 * Verify a BLISS signature based on a SHA-512 hash
 */
static bool verify_bliss(private_bliss_public_key_t *this, hash_algorithm_t alg,
						 chunk_t data, chunk_t signature)
{
	int i, n;
	int32_t *z1, *u;
	int16_t *ud, *z2d;
	uint16_t q, q2, p, *c_indices, *indices;
	uint32_t *az;
	uint8_t data_hash_buf[HASH_SIZE_SHA512];
	chunk_t data_hash;
	hasher_t *hasher;
	ext_out_function_t oracle_alg;
	ntt_fft_t *fft;
	bliss_signature_t *sig;
	bool success = FALSE;

	/* Create data hash using configurable hash algorithm */
	hasher = lib->crypto->create_hasher(lib->crypto, alg);
	if (!hasher )
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

	sig = bliss_signature_create_from_data(this->set, signature);
	if (!sig)
	{
		return FALSE;
	}
	sig->get_parameters(sig, &z1, &z2d, &c_indices);

	if (!bliss_utils_check_norms(this->set, z1, z2d))
	{
		sig->destroy(sig);
		return FALSE;
	}

	/* MGF1 hash algorithm to be used for random oracle */
	oracle_alg = XOF_MGF1_SHA512;

	/* Initialize a couple of needed variables */
	n  = this->set->n;
	q  = this->set->q;
	p  = this->set->p;
	q2 = 2 * q;
	az  = malloc(n * sizeof(uint32_t));
	u   = malloc(n * sizeof(int32_t));
	ud  = malloc(n * sizeof(int16_t));
	indices   = malloc(this->set->kappa * sizeof(uint16_t));

	for (i = 0; i < n; i++)
	{
		az[i] = z1[i] < 0 ? q + z1[i] : z1[i];
	}
	fft = ntt_fft_create(this->set->fft_params);
	fft->transform(fft, az, az, FALSE);

	for (i = 0; i < n; i++)
	{
		az[i] = ntt_fft_mreduce(this->Ar[i] * az[i], this->set->fft_params);
	}
	fft->transform(fft, az, az, TRUE);

	for (i = 0; i < n; i++)
	{
		u[i] = (2 * this->set->q2_inv * az[i]) % q2;
	}

	for (i = 0; i < this->set->kappa; i++)
	{
		u[c_indices[i]] = (u[c_indices[i]] + q * this->set->q2_inv) % q2;
	}
	bliss_utils_round_and_drop(this->set, u, ud);

	for (i = 0; i < n; i++)
	{
		ud[i] += z2d[i];
		if (ud[i] < 0)
		{
			ud[i] += p;
		}
		else if (ud[i] >= p)
		{
			ud[i] -= p;
		}
	}

	/* Detailed debugging information */
	DBG3(DBG_LIB, "  i    u[i]  ud[i] z2d[i]");
	for (i = 0; i < n; i++)
	{
		DBG3(DBG_LIB, "%3d  %6d   %4d  %4d", i, u[i], ud[i], z2d[i]);
	}

	if (!bliss_utils_generate_c(oracle_alg, data_hash, ud, this->set, indices))
	{
		goto end;
	}

	for (i = 0; i < this->set->kappa; i++)
	{
		if (indices[i] != c_indices[i])
		{
			DBG1(DBG_LIB, "signature verification failed");
			goto end;
		}
	}
	success = TRUE;

end:
	/* cleanup */
	sig->destroy(sig);
	fft->destroy(fft);
	free(az);
	free(u);
	free(ud);
	free(indices);

	return success;
}

METHOD(public_key_t, verify, bool,
	private_bliss_public_key_t *this, signature_scheme_t scheme, void *params,
	chunk_t data, chunk_t signature)
{
	switch (scheme)
	{
		case SIGN_BLISS_WITH_SHA2_256:
			return verify_bliss(this, HASH_SHA256, data, signature);
		case SIGN_BLISS_WITH_SHA2_384:
			return verify_bliss(this, HASH_SHA384, data, signature);
		case SIGN_BLISS_WITH_SHA2_512:
			return verify_bliss(this, HASH_SHA512, data, signature);
		case SIGN_BLISS_WITH_SHA3_256:
			return verify_bliss(this, HASH_SHA3_256, data, signature);
		case SIGN_BLISS_WITH_SHA3_384:
			return verify_bliss(this, HASH_SHA3_384, data, signature);
		case SIGN_BLISS_WITH_SHA3_512:
			return verify_bliss(this, HASH_SHA3_512, data, signature);
		default:
			DBG1(DBG_LIB, "signature scheme %N not supported by BLISS",
				 signature_scheme_names, scheme);
			return FALSE;
	}
}

METHOD(public_key_t, encrypt_, bool,
	private_bliss_public_key_t *this, encryption_scheme_t scheme,
	chunk_t plain, chunk_t *crypto)
{
	DBG1(DBG_LIB, "encryption scheme %N not supported",
				   encryption_scheme_names, scheme);
	return FALSE;
}

METHOD(public_key_t, get_keysize, int,
	private_bliss_public_key_t *this)
{
	return this->set->strength;
}

METHOD(public_key_t, get_encoding, bool,
	private_bliss_public_key_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	bool success = TRUE;

	*encoding = bliss_public_key_info_encode(this->set->oid, this->A, this->set);

	if (type != PUBKEY_SPKI_ASN1_DER)
	{
		chunk_t asn1_encoding = *encoding;

		success = lib->encoding->encode(lib->encoding, type,
						NULL, encoding, CRED_PART_BLISS_PUB_ASN1_DER,
						asn1_encoding, CRED_PART_END);
		chunk_clear(&asn1_encoding);
	}
	return success;
}

METHOD(public_key_t, get_fingerprint, bool,
	private_bliss_public_key_t *this, cred_encoding_type_t type, chunk_t *fp)
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

METHOD(public_key_t, get_ref, public_key_t*,
	private_bliss_public_key_t *this)
{
	ref_get(&this->ref);
	return &this->public.key;
}

METHOD(public_key_t, destroy, void,
	private_bliss_public_key_t *this)
{
	if (ref_put(&this->ref))
	{
		lib->encoding->clear_cache(lib->encoding, this);
		free(this->A);
		free(this->Ar);
		free(this);
	}
}

/**
 * ASN.1 definition of a BLISS public key
 */
static const asn1Object_t pubkeyObjects[] = {
	{ 0, "subjectPublicKeyInfo",ASN1_SEQUENCE,		ASN1_OBJ  }, /*  0 */
	{ 1,   "algorithm",			ASN1_EOC,			ASN1_RAW  }, /*  1 */
	{ 1,   "subjectPublicKey",	ASN1_BIT_STRING,	ASN1_BODY }, /*  2 */
	{ 0, "exit",				ASN1_EOC,			ASN1_EXIT }
};
#define BLISS_SUBJECT_PUBLIC_KEY_ALGORITHM	1
#define BLISS_SUBJECT_PUBLIC_KEY			2

/**
 * See header.
 */
bliss_public_key_t *bliss_public_key_load(key_type_t type, va_list args)
{
	private_bliss_public_key_t *this;
	chunk_t blob = chunk_empty, object, param;
	asn1_parser_t *parser;
	bool success = FALSE;
	int objectID, oid, i;
	uint32_t r2;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ASN1_DER:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_END:
				break;
			default:
				return NULL;
		}
		break;
	}

	if (blob.len == 0)
	{
		return NULL;
	}

	INIT(this,
		.public = {
			.key = {
				.get_type = _get_type,
				.verify = _verify,
				.encrypt = _encrypt_,
				.equals = public_key_equals,
				.get_keysize = _get_keysize,
				.get_fingerprint = _get_fingerprint,
				.has_fingerprint = public_key_has_fingerprint,
				.get_encoding = _get_encoding,
				.get_ref = _get_ref,
				.destroy = _destroy,
			},
		},
		.ref = 1,
	);

	parser = asn1_parser_create(pubkeyObjects, blob);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case BLISS_SUBJECT_PUBLIC_KEY_ALGORITHM:
			{
				oid = asn1_parse_algorithmIdentifier(object,
								parser->get_level(parser)+1, &param);
				if (oid != OID_BLISS_PUBLICKEY)
				{
					goto end;
				}
				if (!asn1_parse_simple_object(&param, ASN1_OID,
								parser->get_level(parser)+3, "blissKeyType"))
				{
					goto end;
				}
				oid = asn1_known_oid(param);
				if (oid == OID_UNKNOWN)
				{
					goto end;
				}
				this->set = bliss_param_set_get_by_oid(oid);
				if (this->set == NULL)
				{
					goto end;
				}
				break;
			}
			case BLISS_SUBJECT_PUBLIC_KEY:
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

/**
 * See header.
 */
bool bliss_public_key_from_asn1(chunk_t object, const bliss_param_set_t *set,
								uint32_t **pubkey)
{
	bliss_bitpacker_t *packer;
	uint32_t coefficient;
	uint16_t needed_bits;
	int i;

	/* skip initial bit string octet defining unused bits */
	object = chunk_skip(object, 1);

	needed_bits = set->n * set->q_bits;

	if (8 * object.len < needed_bits)
	{
		return FALSE;
	}
	*pubkey = malloc(set->n * sizeof(uint32_t));

	packer = bliss_bitpacker_create_from_data(object);

	for (i = 0; i < set->n; i++)
	{
		packer->read_bits(packer, &coefficient, set->q_bits);
		if (coefficient >= set->q)
		{
			packer->destroy(packer);
			return FALSE;
		}
		(*pubkey)[i] = coefficient;
	}
	packer->destroy(packer);

	return TRUE;
}

/**
 * See header.
 */
chunk_t bliss_public_key_encode(uint32_t *pubkey, const bliss_param_set_t *set)
{
	bliss_bitpacker_t *packer;
	chunk_t encoding;
	int i;

	packer = bliss_bitpacker_create(set->n * set->q_bits);

	for (i = 0; i < set->n; i++)
	{
		packer->write_bits(packer, pubkey[i], set->q_bits);
	}
	encoding = packer->extract_buf(packer);
	packer->destroy(packer);

	return encoding;
}

/**
 * See header.
 */
chunk_t bliss_public_key_info_encode(int oid, uint32_t *pubkey,
									 const bliss_param_set_t *set)
{
	chunk_t encoding, pubkey_encoding;

	pubkey_encoding = bliss_public_key_encode(pubkey, set);

	encoding = asn1_wrap(ASN1_SEQUENCE, "mm",
					asn1_wrap(ASN1_SEQUENCE, "mm",
						asn1_build_known_oid(OID_BLISS_PUBLICKEY),
						asn1_build_known_oid(oid)),
					asn1_bitstring("m", pubkey_encoding));

	return encoding;
}

/**
 * See header.
 */
bool bliss_public_key_fingerprint(int oid, uint32_t *pubkey,
								  const bliss_param_set_t *set,
								  cred_encoding_type_t type, chunk_t *fp)
{
	hasher_t *hasher;
	chunk_t key;

	switch (type)
	{
		case KEYID_PUBKEY_SHA1:
			key = bliss_public_key_encode(pubkey, set);
			break;
		case KEYID_PUBKEY_INFO_SHA1:
			key = bliss_public_key_info_encode(oid, pubkey, set);
			break;
		default:
			return FALSE;
	}

	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!hasher || !hasher->allocate_hash(hasher, key, fp))
	{
		DBG1(DBG_LIB, "SHA1 hash algorithm not supported, fingerprinting failed");
		DESTROY_IF(hasher);
		free(key.ptr);

		return FALSE;
	}
	hasher->destroy(hasher);
	free(key.ptr);

	return TRUE;
}

