/*
 * Copyright (C) 2017 Tobias Brunner
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

#include "signature_params.h"

#include <asn1/oid.h>
#include <asn1/asn1_parser.h>

/*
 * Described in header
 */
bool rsa_pss_params_set_salt_len(rsa_pss_params_t *params, size_t modbits)
{
	size_t hash_len;

	if (params->salt_len < 0)
	{
		hash_len = hasher_hash_size(params->hash);
		if (!hash_len)
		{
			return FALSE;
		}

		switch (params->salt_len)
		{
			case RSA_PSS_SALT_LEN_DEFAULT:
				params->salt_len = hash_len;
				break;
			case RSA_PSS_SALT_LEN_MAX:
				if (modbits)
				{
					/* emBits = modBits - 1 */
					modbits -= 1;
					/* emLen = ceil(emBits/8) */
					modbits = (modbits+7) / BITS_PER_BYTE;
					/* account for 0x01 separator in DB, 0xbc trailing byte */
					params->salt_len = max(0, (ssize_t)(modbits - hash_len - 2));
					break;
				}
				return FALSE;
			default:
				return FALSE;
		}
	}
	return TRUE;
}

/**
 * Compare two signature schemes and their parameters
 */
static bool compare_params(signature_params_t *a, signature_params_t *b,
						   bool strict)
{
	if (!a && !b)
	{
		return TRUE;
	}
	if (!a || !b)
	{
		return FALSE;
	}
	if (a->scheme != b->scheme)
	{
		return FALSE;
	}
	if (!a->params && !b->params)
	{
		return TRUE;
	}
	if (a->params && b->params)
	{
		switch (a->scheme)
		{
			case SIGN_RSA_EMSA_PSS:
			{
				rsa_pss_params_t *pss_a = a->params, *pss_b = b->params;

				return pss_a->hash == pss_b->hash &&
					   pss_a->mgf1_hash == pss_b->mgf1_hash &&
					   (!strict || pss_a->salt_len == pss_b->salt_len);
			}
			default:
				break;
		}
	}
	return FALSE;
}

/*
 * Described in header
 */
bool signature_params_equal(signature_params_t *a, signature_params_t *b)
{
	return compare_params(a, b, TRUE);
}

/*
 * Described in header
 */
bool signature_params_comply(signature_params_t *c, signature_params_t *s)
{	/* the salt is variable, so it does not necessarily have to be the same */
	return compare_params(c, s, FALSE);
}

/*
 * Described in header
 */
signature_params_t *signature_params_clone(signature_params_t *this)
{
	signature_params_t *clone;

	if (!this)
	{
		return NULL;
	}

	INIT(clone,
		.scheme = this->scheme,
	);
	if (this->params)
	{
		switch (this->scheme)
		{
			case SIGN_RSA_EMSA_PSS:
			{
				rsa_pss_params_t *pss, *pss_clone;

				pss = this->params;
				INIT(pss_clone,
					.hash = pss->hash,
					.mgf1_hash = pss->mgf1_hash,
					.salt_len = pss->salt_len,
					/* ignore salt as only used for unit tests */
				);
				clone->params = pss_clone;
				break;
			}
			default:
				break;
		}
	}
	return clone;
}

/*
 * Described in header
 */
void signature_params_destroy(signature_params_t *this)
{
	if (this)
	{
		free(this->params);
		free(this);
	}
}

/*
 * Described in header
 */
void signature_params_clear(signature_params_t *this)
{
	if (this)
	{
		free(this->params);
		this->params = NULL;
		this->scheme = SIGN_UNKNOWN;
	}
}

/*
 * Described in header
 */
bool signature_params_parse(chunk_t asn1, int level0,
							signature_params_t *params)
{
	chunk_t parameters = chunk_empty;
	int oid;

	oid = asn1_parse_algorithmIdentifier(asn1, level0, &parameters);
	params->scheme = signature_scheme_from_oid(oid);
	switch (params->scheme)
	{
		case SIGN_UNKNOWN:
			return FALSE;
		case SIGN_RSA_EMSA_PSS:
		{
			rsa_pss_params_t *pss = malloc_thing(rsa_pss_params_t);

			if (!rsa_pss_params_parse(parameters, level0+1, pss))
			{
				DBG1(DBG_IKE, "failed parsing RSASSA-PSS parameters");
				free(pss);
				return FALSE;
			}
			params->params = pss;
			break;
		}
		default:
			params->params = NULL;
			break;
	}
	return TRUE;
}

/*
 * Described in header
 */
bool signature_params_build(signature_params_t *params, chunk_t *asn1)
{
	chunk_t parameters = chunk_empty;
	int oid;

	oid = signature_scheme_to_oid(params->scheme);
	if (oid == OID_UNKNOWN)
	{
		return FALSE;
	}
	if (params->scheme == SIGN_RSA_EMSA_PSS &&
		!rsa_pss_params_build(params->params, &parameters))
	{
		return FALSE;
	}
	if (parameters.len)
	{
		*asn1 = asn1_algorithmIdentifier_params(oid, parameters);
	}
	else
	{
		*asn1 = asn1_algorithmIdentifier(oid);
	}
	return TRUE;
}

/**
 * ASN.1 definition of RSASSA-PSS-params
 */
static const asn1Object_t RSASSAPSSParamsObjects[] = {
	{ 0, "RSASSA-PSS-params",	ASN1_SEQUENCE,		ASN1_NONE			}, /* 0 */
	{ 1,   "DEFAULT SHA-1",		ASN1_CONTEXT_C_0,	ASN1_DEF			}, /* 1 */
	{ 2,     "hashAlgorithm",	ASN1_EOC,			ASN1_RAW			}, /* 2 */
	{ 1,   "DEFAULT MGF1SHA1",	ASN1_CONTEXT_C_1,	ASN1_DEF			}, /* 3 */
	{ 2,     "maskGenAlgorithm",ASN1_EOC,			ASN1_RAW			}, /* 4 */
	{ 1,   "DEFAULT 20",		ASN1_CONTEXT_C_2,	ASN1_DEF			}, /* 5 */
	{ 2,     "saltLength",		ASN1_INTEGER,		ASN1_BODY			}, /* 6 */
	{ 1,   "DEFAULT 1",			ASN1_CONTEXT_C_3,	ASN1_DEF			}, /* 7 */
	{ 2,     "trailerField",	ASN1_INTEGER,		ASN1_BODY			}, /* 8 */
	{ 0, "exit",				ASN1_EOC,			ASN1_EXIT			}
};
#define RSASSA_PSS_PARAMS_HASH_ALG		2
#define RSASSA_PSS_PARAMS_MGF_ALG		4
#define RSASSA_PSS_PARAMS_SALT_LEN		6
#define RSASSA_PSS_PARAMS_TRAILER		8

/*
 * Described in header
 */
bool rsa_pss_params_parse(chunk_t asn1, int level0, rsa_pss_params_t *params)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID, alg;
	bool success = FALSE;

	params->hash = HASH_SHA1;
	params->mgf1_hash = HASH_SHA1;
	params->salt_len = HASH_SIZE_SHA1;

	parser = asn1_parser_create(RSASSAPSSParamsObjects, asn1);
	parser->set_top_level(parser, level0);

	while (parser->iterate(parser, &objectID, &object))
	{
		u_int level = parser->get_level(parser)+1;

		switch (objectID)
		{
			case RSASSA_PSS_PARAMS_HASH_ALG:
				if (object.len)
				{
					alg = asn1_parse_algorithmIdentifier(object, level, NULL);
					params->hash = hasher_algorithm_from_oid(alg);
					if (params->hash == HASH_UNKNOWN)
					{
						goto end;
					}
				}
				break;
			case RSASSA_PSS_PARAMS_MGF_ALG:
				if (object.len)
				{
					chunk_t hash = chunk_empty;

					alg = asn1_parse_algorithmIdentifier(object, level, &hash);
					if (alg != OID_MGF1)
					{
						goto end;
					}
					if (!hash.len)
					{
						goto end;
					}
					alg = asn1_parse_algorithmIdentifier(hash, level+1, NULL);
					params->mgf1_hash = hasher_algorithm_from_oid(alg);
					if (params->mgf1_hash == HASH_UNKNOWN)
					{
						goto end;
					}
				}
				break;
			case RSASSA_PSS_PARAMS_SALT_LEN:
				if (object.len)
				{
					params->salt_len = (size_t)asn1_parse_integer_uint64(object);
				}
				break;
			case RSASSA_PSS_PARAMS_TRAILER:
				if (object.len && (object.len != 1 || *object.ptr != 1))
				{
					goto end;
				}
				break;
			default:
				break;
		}
	}
	success = parser->success(parser);

end:
	parser->destroy(parser);
	return success;
}

/*
 * Described in header
 */
bool rsa_pss_params_build(rsa_pss_params_t *params, chunk_t *asn1)
{
	chunk_t hash = chunk_empty, mgf = chunk_empty, slen = chunk_empty;
	int alg;

	if (params->hash != HASH_SHA1)
	{	/* with SHA-1 we MUST omit the field */
		alg = hasher_algorithm_to_oid(params->hash);
		if (alg == OID_UNKNOWN)
		{
			return FALSE;
		}
		hash = asn1_algorithmIdentifier(alg);
	}
	if (params->mgf1_hash != HASH_SHA1)
	{	/* with MGF1-SHA1 we MUST omit the field */
		alg = hasher_algorithm_to_oid(params->mgf1_hash);
		if (alg == OID_UNKNOWN)
		{
			chunk_free(&hash);
			return FALSE;
		}
		mgf = asn1_algorithmIdentifier_params(OID_MGF1,
											  asn1_algorithmIdentifier(alg));
	}
	if (params->salt_len < 0)
	{
		chunk_free(&hash);
		chunk_free(&mgf);
		return FALSE;
	}
	else if (params->salt_len != HASH_SIZE_SHA1)
	{
		slen = asn1_integer("m", asn1_integer_from_uint64(params->salt_len));
	}
	*asn1 = asn1_wrap(ASN1_SEQUENCE, "mmm",
				hash.len ? asn1_wrap(ASN1_CONTEXT_C_0, "m", hash) : chunk_empty,
				mgf.len ? asn1_wrap(ASN1_CONTEXT_C_1, "m", mgf) : chunk_empty,
				slen.len ? asn1_wrap(ASN1_CONTEXT_C_2, "m", slen) : chunk_empty);
	return TRUE;
}
