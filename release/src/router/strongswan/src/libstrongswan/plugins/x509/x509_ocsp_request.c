/*
 * Copyright (C) 2017-2019 Tobias Brunner
 * Copyright (C) 2008-2009 Martin Willi
 * Copyright (C) 2007-2023 Andreas Steffen, strongSec GmbH
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

#include "x509_ocsp_request.h"

#include <library.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <utils/identification.h>
#include <collections/linked_list.h>
#include <utils/debug.h>
#include <credentials/certificates/x509.h>
#include <credentials/keys/private_key.h>

/* RFC 8954 OCSP Nonce Extension */
#define NONCE_LEN		32

typedef struct private_x509_ocsp_request_t private_x509_ocsp_request_t;

/**
 * private data of x509_ocsp_request
 */
struct private_x509_ocsp_request_t {

	/**
	 * public functions
	 */
	x509_ocsp_request_t public;

	/**
	 * CA the certificates where issued by
	 */
	certificate_t *cacert;

	/**
	 * Requestor name, subject of cert used if not set
	 */
	identification_t *requestor;

	/**
	 * Requestor certificate, included in request
	 */
	certificate_t *cert;

	/**
	 * Requestor private key to sign request
	 */
	private_key_t *key;

	/**
	 * list of X.509 certificates to check
	 */
	linked_list_t *reqCerts;

	/**
	 * nonce used in request
	 */
	chunk_t nonce;

	/**
	 * encoded OCSP request
	 */
	chunk_t encoding;

	/**
	 * data for signature verification
	 */
	chunk_t tbsRequest;

	/**
	 * signature scheme
	 */
	signature_params_t *scheme;

	/**
	 * signature
	 */
	chunk_t signature;

	/**
	 * reference count
	 */
	refcount_t ref;
};

/**
 * Single reqCert object sent in OCSP request
 */
typedef struct {
	/** hash algorithm for the two hashes */
	hash_algorithm_t hashAlgorithm;
	/** hash of issuer DN */
	chunk_t issuerNameHash;
	/** issuerKeyID */
	chunk_t issuerKeyHash;
	/** serial number of certificate */
	chunk_t serialNumber;
} req_cert_t;

/**
 * Clean up a reqCert object
 */
CALLBACK(req_cert_destroy, void,
	req_cert_t *reqCert)
{
	chunk_free(&reqCert->issuerNameHash);
	chunk_free(&reqCert->issuerKeyHash);
	chunk_free(&reqCert->serialNumber);
	free(reqCert);
}

static const chunk_t ASN1_nonce_oid = chunk_from_chars(
	0x06, 0x09,
		  0x2B, 0x06,
				0x01, 0x05, 0x05, 0x07, 0x30, 0x01, 0x02
);
static const chunk_t ASN1_response_oid = chunk_from_chars(
	0x06, 0x09,
		  0x2B, 0x06,
				0x01, 0x05, 0x05, 0x07, 0x30, 0x01, 0x04
);
static const chunk_t ASN1_response_content = chunk_from_chars(
	0x04, 0x0D,
		  0x30, 0x0B,
				0x06, 0x09,
				0x2B, 0x06,
				0x01, 0x05, 0x05, 0x07, 0x30, 0x01, 0x01
);

/**
 * build requestorName
 */
static chunk_t build_requestorName(private_x509_ocsp_request_t *this)
{
	if (this->requestor || this->cert)
	{	/* use requestor name, fallback to his cert subject */
		if (!this->requestor)
		{
			this->requestor = this->cert->get_subject(this->cert);
			this->requestor = this->requestor->clone(this->requestor);
		}
		return asn1_wrap(ASN1_CONTEXT_C_1, "m",
					asn1_simple_object(ASN1_CONTEXT_C_4,
						this->requestor->get_encoding(this->requestor)));

	}
	return chunk_empty;
}

/**
 * build Request, not using singleRequestExtensions
 */
static chunk_t build_Request(private_x509_ocsp_request_t *this,
							 req_cert_t *reqCert)
{
	return asn1_wrap(ASN1_SEQUENCE, "m",
			asn1_wrap(ASN1_SEQUENCE, "mmmm",
				asn1_algorithmIdentifier(
					hasher_algorithm_to_oid(reqCert->hashAlgorithm)),
				asn1_simple_object(ASN1_OCTET_STRING, reqCert->issuerNameHash),
				asn1_simple_object(ASN1_OCTET_STRING, reqCert->issuerKeyHash),
				asn1_integer("c", reqCert->serialNumber)));
}

/**
 * build requestList
 */
static chunk_t build_requestList(private_x509_ocsp_request_t *this)
{
	chunk_t list = chunk_empty, request;
	enumerator_t *enumerator;
	req_cert_t *reqCert;

	enumerator = this->reqCerts->create_enumerator(this->reqCerts);
	while (enumerator->enumerate(enumerator, &reqCert))
	{
		request = build_Request(this, reqCert);
		list = chunk_cat("mm", list, request);
	}
	enumerator->destroy(enumerator);

	return asn1_wrap(ASN1_SEQUENCE, "m", list);
}

/**
 * build nonce extension
 */
static chunk_t build_nonce(private_x509_ocsp_request_t *this)
{
	rng_t *rng;
	int nonce_len;

	nonce_len = lib->settings->get_int(lib->settings, "%s.ocsp_nonce_len",
									   NONCE_LEN, lib->ns);

	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng || !rng->allocate_bytes(rng, max(1, nonce_len), &this->nonce))
	{
		DBG1(DBG_LIB, "failed to create RNG");
		DESTROY_IF(rng);
		return chunk_empty;
	}
	rng->destroy(rng);
	return asn1_wrap(ASN1_SEQUENCE, "cm", ASN1_nonce_oid,
				asn1_wrap(ASN1_OCTET_STRING, "m",
					asn1_simple_object(ASN1_OCTET_STRING, this->nonce)));
}

/**
 * build acceptableResponses extension
 */
static chunk_t build_acceptableResponses(private_x509_ocsp_request_t *this)
{
	return asn1_wrap(ASN1_SEQUENCE, "cc",
				ASN1_response_oid,
				ASN1_response_content);
}

/**
 * build requestExtensions
 */
static chunk_t build_requestExtensions(private_x509_ocsp_request_t *this)
{
	return asn1_wrap(ASN1_CONTEXT_C_2, "m",
				asn1_wrap(ASN1_SEQUENCE, "mm",
					build_nonce(this),
					build_acceptableResponses(this)));
}

/**
 * Build tbsRequest
 */
static chunk_t build_tbsRequest(private_x509_ocsp_request_t *this)
{
	return asn1_wrap(ASN1_SEQUENCE, "mmm",
				build_requestorName(this),
				build_requestList(this),
				build_requestExtensions(this));
}

/**
 * Build the optionalSignature
 */
static chunk_t build_optionalSignature(private_x509_ocsp_request_t *this,
									   chunk_t tbsRequest)
{
	int oid;
	signature_scheme_t scheme;
	chunk_t certs = chunk_empty, signature, encoding;

	switch (this->key->get_type(this->key))
	{
		/* TODO: use a generic mapping function */
		case KEY_RSA:
			oid = OID_SHA1_WITH_RSA;
			scheme = SIGN_RSA_EMSA_PKCS1_SHA1;
			break;
		case KEY_ECDSA:
			oid = OID_ECDSA_WITH_SHA1;
			scheme = SIGN_ECDSA_WITH_SHA1_DER;
			break;
		default:
			DBG1(DBG_LIB, "unable to sign OCSP request, %N signature not "
				 "supported", key_type_names, this->key->get_type(this->key));
			return chunk_empty;
	}

	if (!this->key->sign(this->key, scheme, NULL, tbsRequest, &signature))
	{
		DBG1(DBG_LIB, "creating OCSP signature failed, skipped");
		return chunk_empty;
	}
	if (this->cert &&
		this->cert->get_encoding(this->cert, CERT_ASN1_DER, &encoding))
	{
		certs = asn1_wrap(ASN1_CONTEXT_C_0, "m",
					asn1_wrap(ASN1_SEQUENCE, "m", encoding));
	}
	return asn1_wrap(ASN1_CONTEXT_C_0, "m",
				asn1_wrap(ASN1_SEQUENCE, "cmm",
					asn1_algorithmIdentifier(oid),
					asn1_bitstring("m", signature),
					certs));
}

/**
 * Build the OCSPRequest data
 */
static chunk_t build_OCSPRequest(private_x509_ocsp_request_t *this)
{
	chunk_t tbsRequest, optionalSignature = chunk_empty;

	tbsRequest = build_tbsRequest(this);
	if (this->key)
	{
		optionalSignature = build_optionalSignature(this, tbsRequest);
	}
	return asn1_wrap(ASN1_SEQUENCE, "mm", tbsRequest, optionalSignature);
}

/**
 * ASN.1 definition of ocspRequest
 */
static const asn1Object_t ocspRequestObjects[] = {
	{ 0, "OCSPRequest",                     ASN1_SEQUENCE,     ASN1_NONE }, /*  0 */
	{ 1,   "tbsRequest",                    ASN1_SEQUENCE,     ASN1_OBJ  }, /*  1 */
	{ 2,     "versionContext",              ASN1_CONTEXT_C_0,  ASN1_NONE |
	                                                           ASN1_DEF  }, /*  2 */
	{ 3,       "version",                   ASN1_INTEGER,      ASN1_BODY }, /*  3 */
	{ 2,     "requestorNameContext",        ASN1_CONTEXT_C_1,  ASN1_OPT  }, /*  4 */
	{ 3,       "requestorName",             ASN1_CONTEXT_C_4,  ASN1_BODY }, /*  5 */
	{ 2,     "end opt",                     ASN1_EOC,          ASN1_END  }, /*  6 */
	{ 2,     "requestList",                 ASN1_SEQUENCE,     ASN1_LOOP }, /*  7 */
	{ 3,       "request",                   ASN1_SEQUENCE,     ASN1_BODY }, /*  8 */
	{ 4,         "reqCert",                 ASN1_SEQUENCE,     ASN1_NONE }, /*  9 */
	{ 5,           "hashAlgorithm",         ASN1_EOC,          ASN1_RAW  }, /* 10 */
	{ 5,           "issuerNameHash",        ASN1_OCTET_STRING, ASN1_BODY }, /* 11 */
	{ 5,           "issuerKeyHash",         ASN1_OCTET_STRING, ASN1_BODY }, /* 12 */
	{ 5,           "serialNumber",          ASN1_INTEGER,      ASN1_BODY }, /* 13 */
	{ 4,         "singleRequestExtensions", ASN1_CONTEXT_C_0,  ASN1_OPT  }, /* 14 */
	{ 4,         "end opt",                 ASN1_EOC,          ASN1_END  }, /* 15 */
	{ 2,     "end loop",                    ASN1_EOC,          ASN1_END  }, /* 16 */
	{ 2,     "requestExtensions",           ASN1_CONTEXT_C_2,  ASN1_OPT  }, /* 17 */
	{ 3,       "Extensions",                ASN1_SEQUENCE,     ASN1_LOOP }, /* 18 */
	{ 4,         "Extension",               ASN1_SEQUENCE,     ASN1_NONE }, /* 19 */
	{ 5,           "extnID",                ASN1_OID,          ASN1_BODY }, /* 20 */
	{ 5,           "critical",              ASN1_BOOLEAN,      ASN1_BODY |
	                                                           ASN1_DEF  }, /* 21 */
	{ 5,           "extnValue",             ASN1_OCTET_STRING, ASN1_BODY }, /* 22 */
	{ 3,       "end loop",                  ASN1_EOC,          ASN1_END  }, /* 23 */
	{ 2,     "end opt",                     ASN1_EOC,          ASN1_END  }, /* 24 */
	{ 1,   "optionalSignature",             ASN1_CONTEXT_C_0,  ASN1_OPT  }, /* 25 */
	{ 2,     "signature",                   ASN1_SEQUENCE,     ASN1_NONE }, /* 26 */
	{ 3,       "signatureAlgorithm",        ASN1_EOC,          ASN1_RAW  }, /* 27 */
	{ 3,       "signature",                 ASN1_BIT_STRING,   ASN1_BODY }, /* 28 */
	{ 3,       "certsContext",              ASN1_CONTEXT_C_0,  ASN1_OPT  }, /* 29 */
	{ 4,         "certs",                   ASN1_SEQUENCE,     ASN1_LOOP }, /* 30 */
	{ 5,           "certificate",           ASN1_EOC,          ASN1_RAW  }, /* 31 */
	{ 4,         "end loop",                ASN1_EOC,          ASN1_END  }, /* 32 */
	{ 3,       "end opt",                   ASN1_EOC,          ASN1_END  }, /* 33 */
	{ 1,   "end opt",                       ASN1_EOC,          ASN1_END  }, /* 34 */
	{ 0, "exit",                            ASN1_EOC,          ASN1_EXIT }
};

#define OCSP_REQ_TBS_REQUEST         1
#define OCSP_REQ_REQUESTOR           5
#define OCSP_REQ_REQ_CERT            9
#define OCSP_REQ_HASH_ALG           10
#define OCSP_REQ_ISSUER_NAME_HASH   11
#define OCSP_REQ_ISSUER_KEY_HASH    12
#define OCSP_REQ_SERIAL_NUMBER      13
#define OCSP_REQ_EXTN_ID            20
#define OCSP_REQ_CRITICAL           21
#define OCSP_REQ_EXTN_VALUE         22
#define OCSP_REQ_SIG_ALG            27
#define OCSP_REQ_SIGNATURE          28
#define OCSP_REQ_CERTIFICATE	    31

/**
 * Parse the OCSPRequest data
 *
 */
static bool parse_OCSPRequest(private_x509_ocsp_request_t *this)
{
	asn1_parser_t *parser;
	req_cert_t *reqCert = NULL;
	chunk_t object;
	int extn_oid = OID_UNKNOWN;
	int objectID;
	bool critical = FALSE, success = FALSE;

	parser = asn1_parser_create(ocspRequestObjects, this->encoding);

	while (parser->iterate(parser, &objectID, &object))
	{
		u_int level = parser->get_level(parser)+1;

		switch (objectID)
		{

			case OCSP_REQ_TBS_REQUEST:
				this->tbsRequest = object;
				break;
			case OCSP_REQ_REQUESTOR:
				this->requestor = identification_create_from_encoding(ID_DER_ASN1_DN, object);
				break;
			case OCSP_REQ_REQ_CERT:
				INIT(reqCert);
				this->reqCerts->insert_last(this->reqCerts, reqCert);
				break;
			case OCSP_REQ_HASH_ALG:
				reqCert->hashAlgorithm = hasher_algorithm_from_oid(
							asn1_parse_algorithmIdentifier(object, level, NULL));
				if (reqCert->hashAlgorithm == HASH_UNKNOWN)
				{
					DBG1(DBG_ASN, "unknowm hash algorithm");
					goto end;
				}
				break;
			case OCSP_REQ_ISSUER_NAME_HASH:
				reqCert->issuerNameHash = chunk_clone(object);
				break;
			case OCSP_REQ_ISSUER_KEY_HASH:
				reqCert->issuerKeyHash = chunk_clone(object);
				break;
			case OCSP_REQ_SERIAL_NUMBER:
				reqCert->serialNumber = chunk_clone(chunk_skip_zero(object));
				break;
			case OCSP_REQ_EXTN_ID:
				extn_oid = asn1_known_oid(object);
				break;
			case OCSP_REQ_CRITICAL:
				critical = object.len && *object.ptr;
				DBG2(DBG_ASN, "  %s", critical ? "TRUE" : "FALSE");
				break;
			case OCSP_REQ_EXTN_VALUE:
			{
				switch (extn_oid)
				{
					case OID_NONCE:
						if (!asn1_parse_simple_object(&object, ASN1_OCTET_STRING,
												level, "nonce"))
						{
							goto end;
						}
						this->nonce = chunk_clone(object);
						break;
					case OID_RESPONSE:
						if (!asn1_parse_simple_object(&object, ASN1_SEQUENCE,
												level, "acceptableResponses"))
						{
							goto end;
						}
						break;
					default:
						if (critical && lib->settings->get_bool(lib->settings,
							"%s.x509.enforce_critical", TRUE, lib->ns))
						{
							DBG1(DBG_ASN, "critical '%s' extension not supported",
								 (extn_oid == OID_UNKNOWN) ? "unknown" :
								 (char*)oid_names[extn_oid].name);
							goto end;
						}
						break;
				}
				break;
			}
			case OCSP_REQ_SIG_ALG:
				INIT(this->scheme);
				if (!signature_params_parse(object, level, this->scheme))
				{
					DBG1(DBG_ASN, "  unable to parse signature algorithm");
					goto end;
				}

				break;
			case OCSP_REQ_SIGNATURE:
				this->signature = chunk_skip(object, 1);
				break;
			case OCSP_REQ_CERTIFICATE:
				if (this->cert)
				{
					DBG1(DBG_LIB, "  skipping additional signing certificate");
					break;
				}
				this->cert = lib->creds->create(lib->creds,
									CRED_CERTIFICATE,CERT_X509,
									BUILD_BLOB_ASN1_DER, object, BUILD_END);
				if (!this->cert)
				{
					goto end;
				}
				break;
		}
	}
	success = parser->success(parser);

end:
	parser->destroy(parser);
	return success;
}

METHOD(certificate_t, get_type, certificate_type_t,
	private_x509_ocsp_request_t *this)
{
	return CERT_X509_OCSP_REQUEST;
}

METHOD(certificate_t, get_subject, identification_t*,
	private_x509_ocsp_request_t *this)
{
	if (this->requestor)
	{
		return this->requestor;
	}
	return (this->cert) ? this->cert->get_subject(this->cert) : NULL;
}

METHOD(certificate_t, get_issuer, identification_t*,
	private_x509_ocsp_request_t *this)
{
	return this->cacert ? this->cacert->get_subject(this->cacert) : NULL;
}

METHOD(certificate_t, has_subject, id_match_t,
	private_x509_ocsp_request_t *this, identification_t *subject)
{
	return ID_MATCH_NONE;
}

METHOD(certificate_t, has_issuer, id_match_t,
	private_x509_ocsp_request_t *this,
							 identification_t *issuer)
{
	return this->cacert ? this->cacert->has_subject(this->cacert, issuer) :
						  ID_MATCH_NONE;
}

METHOD(certificate_t, issued_by, bool,
	private_x509_ocsp_request_t *this, certificate_t *issuer,
	signature_params_t **scheme)
{
	public_key_t *key;
	bool valid;

	if (issuer->get_type(issuer) != CERT_X509 || this->cert == NULL ||
	   !issuer->equals(issuer, this->cert))
	{
		return FALSE;
	}

	key = issuer->get_public_key(issuer);
	if (!key)
	{
		return FALSE;
	}
	valid = key->verify(key, this->scheme->scheme, this->scheme->params,
						this->tbsRequest, this->signature);
	key->destroy(key);

	if (valid && scheme)
	{
		*scheme = signature_params_clone(this->scheme);
	}
	return valid;
}

METHOD(certificate_t, get_public_key, public_key_t*,
	private_x509_ocsp_request_t *this)
{
	return NULL;
}

METHOD(certificate_t, get_validity, bool,
	private_x509_ocsp_request_t *this, time_t *when, time_t *not_before,
	time_t *not_after)
{
	return FALSE;
}

METHOD(certificate_t, get_encoding, bool,
	private_x509_ocsp_request_t *this, cred_encoding_type_t type,
	chunk_t *encoding)
{
	if (type == CERT_ASN1_DER)
	{
		*encoding = chunk_clone(this->encoding);
		return TRUE;
	}
	return lib->encoding->encode(lib->encoding, type, NULL, encoding,
				CRED_PART_X509_OCSP_REQ_ASN1_DER, this->encoding, CRED_PART_END);
}

METHOD(certificate_t, equals, bool,
	private_x509_ocsp_request_t *this, certificate_t *other)
{
	chunk_t encoding;
	bool equal;

	if (this == (private_x509_ocsp_request_t*)other)
	{
		return TRUE;
	}
	if (other->get_type(other) != CERT_X509_OCSP_REQUEST)
	{
		return FALSE;
	}
	if (other->equals == (void*)equals)
	{	/* skip allocation if we have the same implementation */
		return chunk_equals(this->encoding, ((private_x509_ocsp_request_t*)other)->encoding);
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
	private_x509_ocsp_request_t *this)
{
	ref_get(&this->ref);
	return &this->public.interface.interface;
}

METHOD(certificate_t, destroy, void,
	private_x509_ocsp_request_t *this)
{
	if (ref_put(&this->ref))
	{
		DESTROY_IF(this->cacert);
		DESTROY_IF(this->requestor);
		DESTROY_IF(this->cert);
		DESTROY_IF(this->key);
		signature_params_destroy(this->scheme);
		this->reqCerts->destroy_function(this->reqCerts, req_cert_destroy);
		chunk_free(&this->nonce);
		chunk_free(&this->encoding);
		free(this);
	}
}

METHOD(ocsp_request_t, get_nonce, chunk_t,
	private_x509_ocsp_request_t *this)
{
	return this->nonce;
}

METHOD(ocsp_request_t, get_signer_cert, certificate_t*,
	private_x509_ocsp_request_t *this)
{
	return this->cert;
}

CALLBACK(filter, bool,
	void *data, enumerator_t *orig, va_list args)
{
	req_cert_t *reqCert;
	hash_algorithm_t *hashAlgorithm;
	chunk_t *issuerNameHash, *issuerKeyHash, *serialNumber;

	VA_ARGS_VGET(args, hashAlgorithm, issuerNameHash, issuerKeyHash, serialNumber);

	if (orig->enumerate(orig, &reqCert))
	{
		if (hashAlgorithm)
		{
			*hashAlgorithm = reqCert->hashAlgorithm;
		}
		if (issuerNameHash)
		{
			*issuerNameHash = reqCert->issuerNameHash;
		}
		if (issuerKeyHash)
		{
			*issuerKeyHash = reqCert->issuerKeyHash;
		}
		if (serialNumber)
		{
			*serialNumber = reqCert->serialNumber;
		}
		return TRUE;
	}
	return FALSE;
}

METHOD(ocsp_request_t, create_request_enumerator, enumerator_t*,
	private_x509_ocsp_request_t *this)
{
	return enumerator_create_filter(
				this->reqCerts->create_enumerator(this->reqCerts),
				filter, NULL, NULL);
}

/**
 * create an empty but initialized OCSP request
 */
static private_x509_ocsp_request_t *create_empty()
{
	private_x509_ocsp_request_t *this;

	INIT(this,
		.public = {
			.interface = {
				.interface = {
					.get_type = _get_type,
					.get_subject = _get_subject,
					.get_issuer = _get_issuer,
					.has_subject = _has_subject,
					.has_issuer = _has_issuer,
					.issued_by = _issued_by,
					.get_public_key = _get_public_key,
					.get_validity = _get_validity,
					.get_encoding = _get_encoding,
					.equals = _equals,
					.get_ref = _get_ref,
					.destroy = _destroy,
				},
				.get_nonce = _get_nonce,
				.get_signer_cert = _get_signer_cert,
				.create_request_enumerator = _create_request_enumerator,
			},
		},
		.reqCerts = linked_list_create(),
		.ref = 1,
	);

	return this;
}

/**
 * See header.
 */
x509_ocsp_request_t *x509_ocsp_request_gen(certificate_type_t type, va_list args)
{
	private_x509_ocsp_request_t *this;
	private_key_t *private;
	identification_t *subject;
	certificate_t *cert;
	x509_t *x509;
	req_cert_t *reqCert;

	this = create_empty();

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_CA_CERT:
				cert = va_arg(args, certificate_t*);
				if (cert->get_type(cert) == CERT_X509)
				{
					this->cacert = cert->get_ref(cert);
				}
				continue;
			case BUILD_CERT:
				cert = va_arg(args, certificate_t*);
				if (cert->get_type(cert) == CERT_X509)
				{
					x509 = (x509_t*)cert;
					INIT(reqCert,
						.serialNumber = chunk_clone(x509->get_serial(x509)),
					);
					this->reqCerts->insert_last(this->reqCerts, reqCert);
				}
				continue;
			case BUILD_SIGNING_CERT:
				cert = va_arg(args, certificate_t*);
				this->cert = cert->get_ref(cert);
				continue;
			case BUILD_SIGNING_KEY:
				private = va_arg(args, private_key_t*);
				this->key = private->get_ref(private);
				continue;
			case BUILD_SUBJECT:
				subject = va_arg(args, identification_t*);
				this->requestor = subject->clone(subject);
				continue;
			case BUILD_END:
				break;
			default:
				goto error;
		}
		break;
	}

	if (this->cacert)
	{
		chunk_t  issuerNameHash, issuerKeyHash;
		enumerator_t *enumerator;
		identification_t *issuer;
		public_key_t *public;
		req_cert_t *reqCert;
		hasher_t *hasher;

		public = this->cacert->get_public_key(this->cacert);
		if (!public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &issuerKeyHash))
		{
			DBG1(DBG_LIB, "failed to compute SHA1 issuerKeyHash");
			public->destroy(public);
			goto error;
		}
		public->destroy(public);

		hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
		if (!hasher)
		{
			DBG1(DBG_LIB, "failed to create SHA1 hasher");
			goto error;
		}

		issuer = this->cacert->get_subject(this->cacert);
		if (!hasher->allocate_hash(hasher, issuer->get_encoding(issuer),
										  &issuerNameHash))
		{
			DBG1(DBG_LIB, "failed to compute SHA1 issuerNameHash");
			hasher->destroy(hasher);
			goto error;
		}
		hasher->destroy(hasher);

		enumerator = this->reqCerts->create_enumerator(this->reqCerts);
		while (enumerator->enumerate(enumerator, &reqCert))
		{
			reqCert->hashAlgorithm  = HASH_SHA1;
			reqCert->issuerNameHash = chunk_clone(issuerNameHash);
			reqCert->issuerKeyHash  = chunk_clone(issuerKeyHash);
		}
		enumerator->destroy(enumerator);
		chunk_free(&issuerNameHash);

		this->encoding = build_OCSPRequest(this);

		return &this->public;
	}

error:
	destroy(this);
	return NULL;
}

/**
 * load an OCSP request
 */
static x509_ocsp_request_t *load(chunk_t blob)
{
	private_x509_ocsp_request_t *this;

	this = create_empty();
	this->encoding = chunk_clone(blob);

	if (!parse_OCSPRequest(this))
	{
		destroy(this);
		return NULL;
	}
	return &this->public;
}

/**
 * See header.
 */
x509_ocsp_request_t *x509_ocsp_request_load(certificate_type_t type, va_list args)
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
