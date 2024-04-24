/*
 * Copyright (C) 2017-2019 Tobias Brunner
 * Copyright (C) 2008-2009 Martin Willi
 * Copyright (C) 2007-2023 Andreas Steffen
 * Copyright (C) 2003 Christoph Gysin, Simon Zwahlen
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

#include "x509_ocsp_response.h"

#include <time.h>

#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <utils/identification.h>
#include <collections/linked_list.h>
#include <utils/debug.h>

#include <library.h>
#include <credentials/certificates/x509.h>
#include <credentials/certificates/crl.h>
#include <credentials/certificates/ocsp_single_response.h>

/**
 * how long do we use an OCSP response without a nextUpdate
 */
#define OCSP_DEFAULT_LIFETIME 30

/* defined in wincrypt.h */
#ifdef OCSP_RESPONSE
# undef OCSP_RESPONSE
#endif

typedef struct private_x509_ocsp_response_t private_x509_ocsp_response_t;

/**
 * Private data of a ocsp_t object.
 */
struct private_x509_ocsp_response_t {
	/**
	 * Public interface for this ocsp object.
	 */
	x509_ocsp_response_t public;

	/**
	 * complete encoded OCSP response
	 */
	chunk_t encoding;

	/**
	 * data for signature verification
	 */
	chunk_t tbsResponseData;

	/**
	 * signature scheme
	 */
	signature_params_t *scheme;

	/**
	 * signature
	 */
	chunk_t signature;

	/**
	 * OCSP response status
	 */
	ocsp_status_t ocsp_status;

	/**
	 * name or keyid of the responder
	 */
	identification_t *responderId;

	/**
	 * time of response production
	 */
	time_t producedAt;

	/**
	 * latest nextUpdate in this OCSP response
	 */
	time_t usableUntil;

	/**
	 * list of included certificates
	 */
	linked_list_t *certs;

	/**
	 * Linked list of OCSP responses, single_response_t
	 */
	linked_list_t *responses;

	/**
	 * Nonce required for ocsp request and response
	 */
	chunk_t nonce;

	/**
	 * Signer certificate, included in response
	 */
	certificate_t *cert;

	/**
	 * Signer private key to sign response
	 */
	private_key_t *key;

	/**
	 * reference counter
	 */
	refcount_t ref;
};

/* our OCSP response version implementation */
#define OCSP_BASIC_RESPONSE_VERSION 1

METHOD(ocsp_response_t, get_status, cert_validation_t,
	private_x509_ocsp_response_t *this, x509_t *subject, x509_t *issuer,
	time_t *revocation_time, crl_reason_t *revocation_reason,
	time_t *this_update, time_t *next_update)
{
	enumerator_t *enumerator;
	ocsp_single_response_t *response;
	cert_validation_t status = VALIDATION_FAILED;
	certificate_t *issuercert = &issuer->interface;

	enumerator = this->responses->create_enumerator(this->responses);
	while (enumerator->enumerate(enumerator, &response))
	{
		hasher_t *hasher;
		identification_t *id;
		cred_encoding_type_t type;
		chunk_t hash, fingerprint;

		/* check serial first, is cheaper */
		if (!chunk_equals(subject->get_serial(subject), response->serialNumber))
		{
			continue;
		}
		/* check issuerKeyHash if available */
		if (response->issuerKeyHash.ptr)
		{
			public_key_t *public;

			public = issuercert->get_public_key(issuercert);
			if (!public)
			{
				continue;
			}
			switch (response->hashAlgorithm)
			{
				case OID_SHA1:
					type = KEYID_PUBKEY_SHA1;
					break;
				default:
					public->destroy(public);
					continue;
			}
			if (!public->get_fingerprint(public, type, &fingerprint) ||
				!chunk_equals(response->issuerKeyHash, fingerprint))
			{
				public->destroy(public);
				continue;
			}
			public->destroy(public);
		}
		/* check issuerNameHash, if available */
		else if (response->issuerNameHash.ptr)
		{
			id = issuercert->get_subject(issuercert);
			hasher = lib->crypto->create_hasher(lib->crypto,
							hasher_algorithm_from_oid(response->hashAlgorithm));
			if (!hasher ||
				!hasher->allocate_hash(hasher, id->get_encoding(id), &hash))
			{
				DESTROY_IF(hasher);
				continue;
			}
			hasher->destroy(hasher);
			if (!chunk_equals(hash, response->issuerNameHash))
			{
				free(hash.ptr);
				continue;
			}
			free(hash.ptr);
		}
		else
		{
			continue;
		}
		/* got a match */
		status = response->status;
		*revocation_time = response->revocationTime;
		*revocation_reason = response->revocationReason;
		*this_update = response->thisUpdate;
		*next_update = response->nextUpdate;

		break;
	}
	enumerator->destroy(enumerator);
	return status;
}

METHOD(ocsp_response_t, create_cert_enumerator, enumerator_t*,
	private_x509_ocsp_response_t *this)
{
	return this->certs->create_enumerator(this->certs);
}

CALLBACK(filter, bool,
	void *data, enumerator_t *orig, va_list args)
{
	ocsp_single_response_t *response;
	cert_validation_t *status;
	crl_reason_t *revocationReason;
	chunk_t *serialNumber;
	time_t *revocationTime;

	VA_ARGS_VGET(args, serialNumber, status, revocationTime, revocationReason);

	if (orig->enumerate(orig, &response))
	{
		if (serialNumber)
		{
			*serialNumber = response->serialNumber;
		}
		if (status)
		{
			*status = response->status;
		}
		if (revocationTime)
		{
			*revocationTime = response->revocationTime;
		}
		if (revocationReason)
		{
			*revocationReason = response->revocationReason;
		}
		return TRUE;
	}
	return FALSE;
}

METHOD(ocsp_response_t, create_response_enumerator, enumerator_t*,
	private_x509_ocsp_response_t *this)
{
	return enumerator_create_filter(
				this->responses->create_enumerator(this->responses),
				filter, NULL, NULL);
}

METHOD(ocsp_response_t, get_ocsp_status, ocsp_status_t,
	private_x509_ocsp_response_t *this)
{
	return this->ocsp_status;
}

METHOD(ocsp_response_t, get_nonce, chunk_t,
	private_x509_ocsp_response_t *this)
{
	return this->nonce;
}

/**
 * Build singleResponse
 */
static chunk_t build_singleResponse(private_x509_ocsp_response_t *this,
									ocsp_single_response_t *response)
{
	chunk_t certID, certStatus, nextUpdate = chunk_empty;

	certID = asn1_wrap(ASN1_SEQUENCE, "mmmm",
				asn1_algorithmIdentifier(
					hasher_algorithm_to_oid(response->hashAlgorithm)),
				asn1_simple_object(ASN1_OCTET_STRING, response->issuerNameHash),
				asn1_simple_object(ASN1_OCTET_STRING, response->issuerKeyHash),
				asn1_integer("c", response->serialNumber));

	switch (response->status)
	{
		case VALIDATION_GOOD:
			certStatus = asn1_wrap(ASN1_CONTEXT_S_0, "c", chunk_empty);
			break;
		case VALIDATION_REVOKED:
		case VALIDATION_ON_HOLD:
			certStatus = asn1_wrap(ASN1_CONTEXT_C_1, "mm",
							asn1_from_time(&response->revocationTime,
										   ASN1_GENERALIZEDTIME),
							asn1_wrap(ASN1_CONTEXT_C_0, "m",
								asn1_simple_object(ASN1_ENUMERATED,
								 chunk_from_chars(response->revocationReason))));
			break;
		case VALIDATION_FAILED:
		default:
			certStatus = asn1_wrap(ASN1_CONTEXT_S_2, "c", chunk_empty);
	}

	if (response->nextUpdate != 0)
	{
		nextUpdate = asn1_wrap(ASN1_CONTEXT_C_0, "m",
						asn1_from_time(&response->nextUpdate,
									   ASN1_GENERALIZEDTIME));
	}

	return asn1_wrap(ASN1_SEQUENCE, "mmmm",
				certID,
				certStatus,
				asn1_from_time(&response->thisUpdate, ASN1_GENERALIZEDTIME),
				nextUpdate);
}

/**
 * ASN.1 definition of singleResponse
 */
static const asn1Object_t singleResponseObjects[] = {
	{ 0, "singleResponse",				ASN1_SEQUENCE,			ASN1_BODY }, /*  0 */
	{ 1,   "certID",					ASN1_SEQUENCE,			ASN1_NONE }, /*  1 */
	{ 2,     "algorithm",				ASN1_EOC,				ASN1_RAW  }, /*  2 */
	{ 2,     "issuerNameHash",			ASN1_OCTET_STRING,		ASN1_BODY }, /*  3 */
	{ 2,     "issuerKeyHash",			ASN1_OCTET_STRING,		ASN1_BODY }, /*  4 */
	{ 2,     "serialNumber",			ASN1_INTEGER,			ASN1_BODY }, /*  5 */
	{ 1,   "certStatusGood",			ASN1_CONTEXT_S_0,		ASN1_OPT  }, /*  6 */
	{ 1,   "end opt",					ASN1_EOC,				ASN1_END  }, /*  7 */
	{ 1,   "certStatusRevoked",			ASN1_CONTEXT_C_1,		ASN1_OPT  }, /*  8 */
	{ 2,     "revocationTime",			ASN1_GENERALIZEDTIME,	ASN1_BODY }, /*  9 */
	{ 2,     "revocationReason",		ASN1_CONTEXT_C_0,		ASN1_OPT  }, /* 10 */
	{ 3,       "crlReason",				ASN1_ENUMERATED,		ASN1_BODY }, /* 11 */
	{ 2,     "end opt",					ASN1_EOC,				ASN1_END  }, /* 12 */
	{ 1,   "end opt",					ASN1_EOC,				ASN1_END  }, /* 13 */
	{ 1,   "certStatusUnknown",			ASN1_CONTEXT_S_2,		ASN1_OPT  }, /* 14 */
	{ 1,   "end opt",					ASN1_EOC,				ASN1_END  }, /* 15 */
	{ 1,   "thisUpdate",				ASN1_GENERALIZEDTIME,	ASN1_BODY }, /* 16 */
	{ 1,   "nextUpdateContext",			ASN1_CONTEXT_C_0,		ASN1_OPT  }, /* 17 */
	{ 2,     "nextUpdate",				ASN1_GENERALIZEDTIME,	ASN1_BODY }, /* 18 */
	{ 1,   "end opt",					ASN1_EOC,				ASN1_END  }, /* 19 */
	{ 1,   "singleExtensionsContext",	ASN1_CONTEXT_C_1,		ASN1_OPT  }, /* 20 */
	{ 2,     "singleExtensions",		ASN1_SEQUENCE,			ASN1_LOOP }, /* 21 */
	{ 3,       "extension",				ASN1_SEQUENCE,			ASN1_NONE }, /* 22 */
	{ 4,         "extnID",				ASN1_OID,				ASN1_BODY }, /* 23 */
	{ 4,         "critical",			ASN1_BOOLEAN,			ASN1_BODY |
																ASN1_DEF  }, /* 24 */
	{ 4,         "extnValue",			ASN1_OCTET_STRING,		ASN1_BODY }, /* 25 */
	{ 2,     "end loop",				ASN1_EOC,				ASN1_END  }, /* 26 */
	{ 1,   "end opt",					ASN1_EOC,				ASN1_END  }, /* 27 */
	{ 0, "exit",						ASN1_EOC,				ASN1_EXIT }
};

#define SINGLE_RESPONSE_ALGORITHM					 2
#define SINGLE_RESPONSE_ISSUER_NAME_HASH			 3
#define SINGLE_RESPONSE_ISSUER_KEY_HASH				 4
#define SINGLE_RESPONSE_SERIAL_NUMBER				 5
#define SINGLE_RESPONSE_CERT_STATUS_GOOD			 6
#define SINGLE_RESPONSE_CERT_STATUS_REVOKED			 8
#define SINGLE_RESPONSE_CERT_STATUS_REVOCATION_TIME	 9
#define SINGLE_RESPONSE_CERT_STATUS_CRL_REASON		11
#define SINGLE_RESPONSE_CERT_STATUS_UNKNOWN			14
#define SINGLE_RESPONSE_THIS_UPDATE					16
#define SINGLE_RESPONSE_NEXT_UPDATE					18
#define SINGLE_RESPONSE_EXT_ID						23
#define SINGLE_RESPONSE_CRITICAL					24
#define SINGLE_RESPONSE_EXT_VALUE					25

/**
 * Parse a single OCSP response
 */
static bool parse_singleResponse(private_x509_ocsp_response_t *this,
								 chunk_t blob, int level0)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	bool success = FALSE;

	ocsp_single_response_t *response;

	response = ocsp_single_response_create();

	/* if nextUpdate is missing, we give it a short lifetime */
	response->nextUpdate = this->producedAt + OCSP_DEFAULT_LIFETIME;

	parser = asn1_parser_create(singleResponseObjects, blob);
	parser->set_top_level(parser, level0);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case SINGLE_RESPONSE_ALGORITHM:
				response->hashAlgorithm = asn1_parse_algorithmIdentifier(object,
											parser->get_level(parser)+1, NULL);
				break;
			case SINGLE_RESPONSE_ISSUER_NAME_HASH:
				response->issuerNameHash = chunk_clone(object);
				break;
			case SINGLE_RESPONSE_ISSUER_KEY_HASH:
				response->issuerKeyHash = chunk_clone(object);
				break;
			case SINGLE_RESPONSE_SERIAL_NUMBER:
				response->serialNumber = chunk_clone(chunk_skip_zero(object));
				break;
			case SINGLE_RESPONSE_CERT_STATUS_GOOD:
				response->status = VALIDATION_GOOD;
				break;
			case SINGLE_RESPONSE_CERT_STATUS_REVOKED:
				response->status = VALIDATION_REVOKED;
				break;
			case SINGLE_RESPONSE_CERT_STATUS_REVOCATION_TIME:
				response->revocationTime = asn1_to_time(&object, ASN1_GENERALIZEDTIME);
				break;
			case SINGLE_RESPONSE_CERT_STATUS_CRL_REASON:
				if (object.len == 1)
				{
					response->revocationReason = *object.ptr;
				}
				break;
			case SINGLE_RESPONSE_CERT_STATUS_UNKNOWN:
				response->status = VALIDATION_FAILED;
				break;
			case SINGLE_RESPONSE_THIS_UPDATE:
				response->thisUpdate = asn1_to_time(&object, ASN1_GENERALIZEDTIME);
				break;
			case SINGLE_RESPONSE_NEXT_UPDATE:
				response->nextUpdate = asn1_to_time(&object, ASN1_GENERALIZEDTIME);
				if (response->nextUpdate > this->usableUntil)
				{
					this->usableUntil = response->nextUpdate;
				}
				break;
		}
	}
	success = parser->success(parser);
	parser->destroy(parser);
	if (success)
	{
		if (this->usableUntil == UNDEFINED_TIME)
		{
			this->usableUntil = this->producedAt + OCSP_DEFAULT_LIFETIME;
		}
		this->responses->insert_last(this->responses, response);
	}
	else
	{
		response->destroy(response);
	}
	return success;
}

/**
 * Build responses
 */
static chunk_t build_responses(private_x509_ocsp_response_t *this)
{
	ocsp_single_response_t *response;
	chunk_t responses = chunk_empty, single_response;
	enumerator_t *enumerator;

	enumerator = this->responses->create_enumerator(this->responses);
	while (enumerator->enumerate(enumerator, &response))
	{
		single_response = build_singleResponse(this, response);
		responses = chunk_cat("mm", responses, single_response);
	}
	enumerator->destroy(enumerator);

	return asn1_wrap(ASN1_SEQUENCE, "m", responses);
}

/**
 * ASN.1 definition of responses
 */
static const asn1Object_t responsesObjects[] = {
	{ 0, "responses",			ASN1_SEQUENCE,	ASN1_LOOP }, /* 0 */
	{ 1,   "singleResponse",	ASN1_EOC,		ASN1_RAW  }, /* 1 */
	{ 0, "end loop",			ASN1_EOC,		ASN1_END  }, /* 2 */
	{ 0, "exit",				ASN1_EOC,		ASN1_EXIT }
};
#define RESPONSES_SINGLE_RESPONSE	1

/**
 * Parse all responses
 */
static bool parse_responses(private_x509_ocsp_response_t *this,
							chunk_t blob, int level0)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	bool success = FALSE;

	parser = asn1_parser_create(responsesObjects, blob);
	parser->set_top_level(parser, level0);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case RESPONSES_SINGLE_RESPONSE:
				if (!parse_singleResponse(this, object,
										  parser->get_level(parser)+1))
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

/**
 * Build tbsResponseData
 */
static chunk_t build_tbsResponseData(private_x509_ocsp_response_t *this)
{
	chunk_t responderIdByName;
	chunk_t responseExtensions = chunk_empty;

	responderIdByName = asn1_wrap(ASN1_CONTEXT_C_1, "c",
							this->responderId->get_encoding(this->responderId));

	this->producedAt = time(NULL);

	responseExtensions = asn1_wrap(ASN1_CONTEXT_C_1, "m",
							asn1_wrap(ASN1_SEQUENCE, "m",
								asn1_wrap(ASN1_SEQUENCE, "mm",
									asn1_build_known_oid(OID_NONCE),
									asn1_wrap(ASN1_OCTET_STRING, "m",
										asn1_simple_object(ASN1_OCTET_STRING,
														  this->nonce)))));

	return asn1_wrap(ASN1_SEQUENCE, "mmmm",
				responderIdByName,
				asn1_from_time(&this->producedAt, ASN1_GENERALIZEDTIME),
				build_responses(this),
				responseExtensions);
}

/**
 * Build the signature
 */
static bool build_signature(private_x509_ocsp_response_t *this,
							chunk_t tbsResponseData, chunk_t *signature)
{
	if (!this->key->sign(this->key, this->scheme->scheme, this->scheme->params,
						 tbsResponseData, signature))
	{
		DBG1(DBG_LIB, "creating OCSP response signature failed");
		return FALSE;
	}
	return TRUE;
}

/**
 * Build the basicOCSPResponse
 */
static bool build_basicOCSPResponse(private_x509_ocsp_response_t *this,
									chunk_t *basicResponse)
{
	chunk_t tbsResponseData, sig_scheme, signature;
	chunk_t cert_encoding, certs = chunk_empty;
	x509_t *x509 = (x509_t*)this->cert;

	*basicResponse = chunk_empty;

	if (!signature_params_build(this->scheme, &sig_scheme))
	{
		return FALSE;
	}
	tbsResponseData = build_tbsResponseData(this);

	if (!build_signature(this, tbsResponseData, &signature))
	{
		free(tbsResponseData.ptr);
		free(sig_scheme.ptr);
		return FALSE;
	}

	/* don't include self-signed signer certificates */
	if (!(x509->get_flags(x509) & X509_SELF_SIGNED))
	{
		if (!this->cert->get_encoding(this->cert, CERT_ASN1_DER, &cert_encoding))
		{
			free(tbsResponseData.ptr);
			free(sig_scheme.ptr);
			free(signature.ptr);
			return FALSE;
		}
		certs = asn1_wrap(ASN1_CONTEXT_C_0, "m",
					asn1_wrap(ASN1_SEQUENCE, "m", cert_encoding));
	}

	*basicResponse = asn1_wrap(ASN1_SEQUENCE, "mmmm",
						tbsResponseData, sig_scheme,
						asn1_bitstring("m", signature), certs);
	return TRUE;
}

/**
 * ASN.1 definition of basicResponse
 */
static const asn1Object_t basicResponseObjects[] = {
	{ 0, "BasicOCSPResponse",				ASN1_SEQUENCE,			ASN1_NONE            }, /*  0 */
	{ 1,   "tbsResponseData",				ASN1_SEQUENCE,			ASN1_OBJ             }, /*  1 */
	{ 2,     "versionContext",				ASN1_CONTEXT_C_0,		ASN1_NONE|ASN1_DEF   }, /*  2 */
	{ 3,       "version",					ASN1_INTEGER,			ASN1_BODY            }, /*  3 */
	{ 2,     "responderId",					ASN1_EOC,				ASN1_CHOICE          }, /*  4 */
	{ 3,       "responderIdContext",		ASN1_CONTEXT_C_1,		ASN1_OPT             }, /*  5 */
	{ 4,         "responderIdByName",		ASN1_SEQUENCE,			ASN1_OBJ             }, /*  6 */
	{ 3,       "end choice",				ASN1_EOC,				ASN1_END|ASN1_CH     }, /*  7 */
	{ 3,       "responderIdContext",		ASN1_CONTEXT_C_2,		ASN1_OPT             }, /*  8 */
	{ 4,         "responderIdByKey",		ASN1_OCTET_STRING,		ASN1_BODY            }, /*  9 */
	{ 3,       "end choice",				ASN1_EOC,				ASN1_END|ASN1_CH     }, /* 10 */
	{ 2,     "end choices",					ASN1_EOC,				ASN1_END|ASN1_CHOICE }, /* 11 */
	{ 2,     "producedAt",					ASN1_GENERALIZEDTIME,	ASN1_BODY            }, /* 12 */
	{ 2,     "responses",					ASN1_SEQUENCE,			ASN1_OBJ             }, /* 13 */
	{ 2,     "responseExtensionsContext",	ASN1_CONTEXT_C_1,		ASN1_OPT             }, /* 14 */
	{ 3,       "responseExtensions",		ASN1_SEQUENCE,			ASN1_LOOP            }, /* 15 */
	{ 4,         "extension",				ASN1_SEQUENCE,			ASN1_NONE            }, /* 16 */
	{ 5,           "extnID",				ASN1_OID,				ASN1_BODY            }, /* 17 */
	{ 5,           "critical",				ASN1_BOOLEAN,			ASN1_BODY | ASN1_DEF }, /* 18 */
	{ 5,           "extnValue",				ASN1_OCTET_STRING,		ASN1_BODY            }, /* 19 */
	{ 3,       "end loop",					ASN1_EOC,				ASN1_END             }, /* 20 */
	{ 2,     "end opt",						ASN1_EOC,				ASN1_END             }, /* 21 */
	{ 1,   "signatureAlgorithm",			ASN1_EOC,				ASN1_RAW             }, /* 22 */
	{ 1,   "signature",						ASN1_BIT_STRING,		ASN1_BODY            }, /* 23 */
	{ 1,   "certsContext",					ASN1_CONTEXT_C_0,		ASN1_OPT             }, /* 24 */
	{ 2,     "certs",						ASN1_SEQUENCE,			ASN1_LOOP            }, /* 25 */
	{ 3,       "certificate",				ASN1_SEQUENCE,			ASN1_RAW             }, /* 26 */
	{ 2,     "end loop",					ASN1_EOC,				ASN1_END             }, /* 27 */
	{ 1,   "end opt",						ASN1_EOC,				ASN1_END             }, /* 28 */
	{ 0, "exit",							ASN1_EOC,				ASN1_EXIT            }
};
#define BASIC_RESPONSE_TBS_DATA		 1
#define BASIC_RESPONSE_VERSION		 3
#define BASIC_RESPONSE_ID_BY_NAME	 6
#define BASIC_RESPONSE_ID_BY_KEY	 9
#define BASIC_RESPONSE_PRODUCED_AT	12
#define BASIC_RESPONSE_RESPONSES	13
#define BASIC_RESPONSE_EXT_ID		17
#define BASIC_RESPONSE_CRITICAL		18
#define BASIC_RESPONSE_EXT_VALUE	19
#define BASIC_RESPONSE_ALGORITHM	22
#define BASIC_RESPONSE_SIGNATURE	23
#define BASIC_RESPONSE_CERTIFICATE	26

/**
 * Parse a basicOCSPResponse
 */
static bool parse_basicOCSPResponse(private_x509_ocsp_response_t *this,
									chunk_t blob, int level0)
{
	asn1_parser_t *parser;
	chunk_t object;
	chunk_t responses = chunk_empty;
	int objectID;
	int extn_oid = OID_UNKNOWN;
	u_int responses_level = level0;
	certificate_t *cert;
	bool success = FALSE;

	parser = asn1_parser_create(basicResponseObjects, blob);
	parser->set_top_level(parser, level0);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case BASIC_RESPONSE_TBS_DATA:
				this->tbsResponseData = object;
				break;
			case BASIC_RESPONSE_VERSION:
			{
				u_int version = (object.len)? (1 + (u_int)*object.ptr) : 1;

				if (version != OCSP_BASIC_RESPONSE_VERSION)
				{
					DBG1(DBG_ASN, "  ocsp ResponseData version %d not "
						 "supported", version);
					goto end;
				}
				break;
			}
			case BASIC_RESPONSE_ID_BY_NAME:
				this->responderId = identification_create_from_encoding(
													ID_DER_ASN1_DN, object);
				DBG2(DBG_ASN, "  '%Y'", this->responderId);
				break;
			case BASIC_RESPONSE_ID_BY_KEY:
				this->responderId = identification_create_from_encoding(
													ID_KEY_ID, object);
				DBG2(DBG_ASN, "  '%Y'", this->responderId);
				break;
			case BASIC_RESPONSE_PRODUCED_AT:
				this->producedAt = asn1_to_time(&object, ASN1_GENERALIZEDTIME);
				break;
			case BASIC_RESPONSE_RESPONSES:
				responses = object;
				responses_level = parser->get_level(parser)+1;
				break;
			case BASIC_RESPONSE_EXT_ID:
				extn_oid = asn1_known_oid(object);
				break;
			case BASIC_RESPONSE_CRITICAL:
				DBG2(DBG_ASN, "  %s",
					 object.len && *object.ptr ? "TRUE" : "FALSE");
				break;
			case BASIC_RESPONSE_EXT_VALUE:
				if (extn_oid == OID_NONCE &&
					asn1_parse_simple_object(&object, ASN1_OCTET_STRING,
										parser->get_level(parser)+1, "nonce"))
				{
					this->nonce = chunk_clone(object);
				}
				break;
			case BASIC_RESPONSE_ALGORITHM:
				INIT(this->scheme);
				if (!signature_params_parse(object, parser->get_level(parser)+1,
											this->scheme))
				{
					DBG1(DBG_ASN, "  unable to parse signature algorithm");
					goto end;
				}
				break;
			case BASIC_RESPONSE_SIGNATURE:
				this->signature = chunk_skip(object, 1);
				break;
			case BASIC_RESPONSE_CERTIFICATE:
			{
				cert = lib->creds->create(lib->creds, CRED_CERTIFICATE,CERT_X509,
										  BUILD_BLOB_ASN1_DER, object,
										  BUILD_END);
				if (cert)
				{
					this->certs->insert_last(this->certs, cert);
				}
				break;
			}
		}
	}
	success = parser->success(parser);

end:
	parser->destroy(parser);
	if (success)
	{
		success = parse_responses(this, responses, responses_level);
	}
	return success;
}

/**
 * Build the OCSPResponse
 *
 */
static chunk_t build_OCSPResponse(private_x509_ocsp_response_t *this)
{
	chunk_t response, responseBytes = chunk_empty;

	if (this->ocsp_status == OCSP_SUCCESSFUL)
	{
		if (!build_basicOCSPResponse(this, &response))
		{
			return chunk_empty;
		}
		responseBytes = asn1_wrap(ASN1_CONTEXT_C_0, "m",
							asn1_wrap(ASN1_SEQUENCE, "mm",
								asn1_build_known_oid(OID_BASIC),
								asn1_wrap(ASN1_OCTET_STRING, "m", response)));
	}
	return asn1_wrap(ASN1_SEQUENCE, "mm",
				asn1_simple_object(ASN1_ENUMERATED,
					chunk_from_chars(this->ocsp_status)),
				responseBytes);
}

/**
 * ASN.1 definition of ocspResponse
 */
static const asn1Object_t ocspResponseObjects[] = {
	{ 0, "OCSPResponse",			ASN1_SEQUENCE,		ASN1_NONE }, /* 0 */
	{ 1,   "responseStatus",		ASN1_ENUMERATED,	ASN1_BODY }, /* 1 */
	{ 1,   "responseBytesContext",	ASN1_CONTEXT_C_0,	ASN1_OPT  }, /* 2 */
	{ 2,     "responseBytes",		ASN1_SEQUENCE,		ASN1_NONE }, /* 3 */
	{ 3,       "responseType",		ASN1_OID,			ASN1_BODY }, /* 4 */
	{ 3,       "response",			ASN1_OCTET_STRING,	ASN1_BODY }, /* 5 */
	{ 1,   "end opt",				ASN1_EOC,			ASN1_END  }, /* 6 */
	{ 0, "exit",					ASN1_EOC,			ASN1_EXIT }
};
#define OCSP_RESPONSE_STATUS	1
#define OCSP_RESPONSE_TYPE		4
#define OCSP_RESPONSE			5

/**
 * Parse OCSPResponse object
 */
static bool parse_OCSPResponse(private_x509_ocsp_response_t *this)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	int responseType = OID_UNKNOWN;
	bool success = FALSE;

	parser = asn1_parser_create(ocspResponseObjects, this->encoding);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case OCSP_RESPONSE_STATUS:
				if (object.len != 1)
				{
					goto end;
				}
				this->ocsp_status = (ocsp_status_t)*object.ptr;
				switch (this->ocsp_status)
				{
					case OCSP_SUCCESSFUL:
						break;
					default:
						DBG1(DBG_LIB, "  ocsp response status: %N",
							 ocsp_status_names, this->ocsp_status);
						success = TRUE;
						break;
				}
				break;
			case OCSP_RESPONSE_TYPE:
				responseType = asn1_known_oid(object);
				break;
			case OCSP_RESPONSE:
				switch (responseType)
				{
					case OID_BASIC:
						success = parse_basicOCSPResponse(this, object,
												parser->get_level(parser)+1);
						break;
					default:
						DBG1(DBG_LIB, "  ocsp response type %#B not supported",
							 &object);
						goto end;
				}
				break;
		}
	}
	success &= parser->success(parser);

end:
	parser->destroy(parser);
	return success;
}

METHOD(certificate_t, get_type, certificate_type_t,
	private_x509_ocsp_response_t *this)
{
	return CERT_X509_OCSP_RESPONSE;
}

METHOD(certificate_t, get_issuer, identification_t*,
	private_x509_ocsp_response_t *this)
{
	return this->responderId;
}

METHOD(certificate_t, has_issuer, id_match_t,
	private_x509_ocsp_response_t *this, identification_t *issuer)
{
	if (this->responderId)
	{
		return this->responderId->matches(this->responderId, issuer);
	}
	return ID_MATCH_NONE;
}

METHOD(certificate_t, issued_by, bool,
	private_x509_ocsp_response_t *this, certificate_t *issuer,
	signature_params_t **scheme)
{
	public_key_t *key;
	bool valid;
	x509_t *x509 = (x509_t*)issuer;

	if (issuer->get_type(issuer) != CERT_X509 || !this->responderId)
	{
		return FALSE;
	}
	if (this->responderId->get_type(this->responderId) == ID_KEY_ID)
	{
		chunk_t fingerprint;

		key = issuer->get_public_key(issuer);
		if (!key ||
			!key->get_fingerprint(key, KEYID_PUBKEY_SHA1, &fingerprint) ||
			!chunk_equals(fingerprint,
						  this->responderId->get_encoding(this->responderId)))
		{
			DESTROY_IF(key);
			return FALSE;
		}
		key->destroy(key);
	}
	else
	{
		if (!this->responderId->equals(this->responderId,
									   issuer->get_subject(issuer)))
		{
			return FALSE;
		}
	}
	if (!(x509->get_flags(x509) & X509_OCSP_SIGNER) &&
		!(x509->get_flags(x509) & X509_CA))
	{
		return FALSE;
	}

	key = issuer->get_public_key(issuer);
	if (!key)
	{
		return FALSE;
	}
	valid = key->verify(key, this->scheme->scheme, this->scheme->params,
						this->tbsResponseData, this->signature);
	key->destroy(key);
	if (valid && scheme)
	{
		*scheme = signature_params_clone(this->scheme);
	}
	return valid;
}

METHOD(certificate_t, get_public_key, public_key_t*,
	private_x509_ocsp_response_t *this)
{
	return NULL;
}

METHOD(certificate_t, get_validity, bool,
	private_x509_ocsp_response_t *this, time_t *when,
	time_t *not_before, time_t *not_after)
{
	time_t t = when ? *when : time(NULL);

	if (not_before)
	{
		*not_before = this->producedAt;
	}
	if (not_after)
	{
		*not_after = this->usableUntil;
	}
	return (t < this->usableUntil);
}

METHOD(certificate_t, get_encoding, bool,
	private_x509_ocsp_response_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	if (type == CERT_ASN1_DER)
	{
		*encoding = chunk_clone(this->encoding);
		return TRUE;
	}
	return lib->encoding->encode(lib->encoding, type, NULL, encoding,
				CRED_PART_X509_OCSP_RES_ASN1_DER, this->encoding, CRED_PART_END);
}

METHOD(certificate_t, equals, bool,
	private_x509_ocsp_response_t *this, certificate_t *other)
{
	chunk_t encoding;
	bool equal;

	if (this == (private_x509_ocsp_response_t*)other)
	{
		return TRUE;
	}
	if (other->get_type(other) != CERT_X509_OCSP_RESPONSE)
	{
		return FALSE;
	}
	if (other->equals == (void*)equals)
	{	/* skip allocation if we have the same implementation */
		return chunk_equals(this->encoding, ((private_x509_ocsp_response_t*)other)->encoding);
	}
	if (!other->get_encoding(other, CERT_ASN1_DER, &encoding))
	{
		return FALSE;
	}
	equal = chunk_equals(this->encoding, encoding);
	free(encoding.ptr);
	return equal;
}

METHOD(certificate_t, get_ref, certificate_t*,
	private_x509_ocsp_response_t *this)
{
	ref_get(&this->ref);
	return &this->public.interface.certificate;
}

METHOD(certificate_t, destroy, void,
	private_x509_ocsp_response_t *this)
{
	if (ref_put(&this->ref))
	{
		this->certs->destroy_offset(this->certs,
								offsetof(certificate_t, destroy));
		this->responses->destroy_offset(this->responses,
								offsetof(ocsp_single_response_t, destroy));
		DESTROY_IF(this->cert);
		DESTROY_IF(this->key);
		DESTROY_IF(this->responderId);
		signature_params_destroy(this->scheme);
		free(this->nonce.ptr);
		free(this->encoding.ptr);
		free(this);
	}
}

/**
 * create an empty but initialized OCSP response
 */
static private_x509_ocsp_response_t *create_empty()
{
	private_x509_ocsp_response_t *this;

	INIT(this,
		.public = {
			.interface = {
				.certificate = {
					.get_type = _get_type,
					.get_subject = _get_issuer,
					.get_issuer = _get_issuer,
					.has_subject = _has_issuer,
					.has_issuer = _has_issuer,
					.issued_by = _issued_by,
					.get_public_key = _get_public_key,
					.get_validity = _get_validity,
					.get_encoding = _get_encoding,
					.equals = _equals,
					.get_ref = _get_ref,
					.destroy = _destroy,
				},
				.get_ocsp_status = _get_ocsp_status,
				.get_nonce = _get_nonce,
				.get_status = _get_status,
				.create_cert_enumerator = _create_cert_enumerator,
				.create_response_enumerator = _create_response_enumerator,
			},
		},
		.ref = 1,
		.producedAt = UNDEFINED_TIME,
		.usableUntil = UNDEFINED_TIME,
		.responses = linked_list_create(),
		.certs = linked_list_create(),
	);

	return this;
}

/**
 * See header.
 */
x509_ocsp_response_t *x509_ocsp_response_gen(certificate_type_t type, va_list args)
{
	private_x509_ocsp_response_t *this;
	private_key_t *private;
	certificate_t *cert;
	chunk_t nonce;
	identification_t *subject;
	enumerator_t *enumerator;
	ocsp_single_response_t *response;

	this = create_empty();

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_OCSP_STATUS:
				this->ocsp_status = va_arg(args, ocsp_status_t);
				continue;
			case BUILD_OCSP_RESPONSES:
				enumerator = va_arg(args, enumerator_t*);
				while (enumerator->enumerate(enumerator, &response))
				{
					this->responses->insert_last(this->responses,
												response->get_ref(response));
				}
				continue;
			case BUILD_SIGNING_CERT:
				cert = va_arg(args, certificate_t*);
				if (cert)
				{
					subject = cert->get_subject(cert);
					this->cert = cert->get_ref(cert);
					this->responderId = subject->clone(subject);
				}
				continue;
			case BUILD_SIGNING_KEY:
				private = va_arg(args, private_key_t*);
				if (private)
				{
					this->key = private->get_ref(private);
				}
				continue;
			case BUILD_SIGNATURE_SCHEME:
				this->scheme = va_arg(args, signature_params_t*);
				this->scheme = signature_params_clone(this->scheme);
				continue;
			case BUILD_NONCE:
				nonce = va_arg(args, chunk_t);
				this->nonce = chunk_clone(nonce);
				continue;
			case BUILD_END:
				break;
			default:
				goto error;
		}
		break;
	}

	if (this->ocsp_status == OCSP_SUCCESSFUL)
	{
		if (!this->key)
		{
			DBG1(DBG_LIB, "no OCSP signing key defined");
			goto error;
		}

		/* select signature scheme, if not already specified */
		if (!this->scheme)
		{
			INIT(this->scheme,
				.scheme = signature_scheme_from_oid(
							hasher_signature_algorithm_to_oid(HASH_SHA256,
								this->key->get_type(this->key))),
			);
		}
		if (this->scheme->scheme == SIGN_UNKNOWN)
		{
			goto error;
		}
	}

	this->encoding = build_OCSPResponse(this);
	return &this->public;

error:
	destroy(this);
	return NULL;
}

/**
 * load an OCSP response
 */
static x509_ocsp_response_t *load(chunk_t blob)
{
	private_x509_ocsp_response_t *this;

	this = create_empty();
	this->encoding = chunk_clone(blob);

	if (!parse_OCSPResponse(this))
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}

/**
 * See header.
 */
x509_ocsp_response_t *x509_ocsp_response_load(certificate_type_t type,
											  va_list args)
{
	chunk_t blob = chunk_empty;

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
	if (blob.ptr)
	{
		return load(blob);
	}
	return NULL;
}
