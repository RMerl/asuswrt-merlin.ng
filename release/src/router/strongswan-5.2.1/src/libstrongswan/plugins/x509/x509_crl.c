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

#include "x509_crl.h"

typedef struct private_x509_crl_t private_x509_crl_t;
typedef struct revoked_t revoked_t;

#include <time.h>

#include <utils/debug.h>
#include <library.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <credentials/certificates/x509.h>
#include <credentials/keys/private_key.h>
#include <collections/linked_list.h>

/**
 * entry for a revoked certificate
 */
struct revoked_t {
	/**
	 * serial of the revoked certificate
	 */
	chunk_t serial;

	/**
	 * date of revocation
	 */
	time_t date;

	/**
	 * reason for revocation
	 */
	crl_reason_t reason;
};

/**
 * private data of x509_crl
 */
struct private_x509_crl_t {

	/**
	 * public functions
	 */
	x509_crl_t public;

	/**
	 * X.509 crl encoding in ASN.1 DER format
	 */
	chunk_t encoding;

	/**
	 * X.509 crl body over which signature is computed
	 */
	chunk_t tbsCertList;

	/**
	 * Version of the X.509 crl
	 */
	u_int version;

	/**
	 * ID representing the crl issuer
	 */
	identification_t *issuer;

	/**
	 * CRL number
	 */
	chunk_t crlNumber;

	/**
	 * Time when the crl was generated
	 */
	time_t thisUpdate;

	/**
	 * Time when an update crl will be available
	 */
	time_t nextUpdate;

	/**
	 * list of revoked certificates as revoked_t
	 */
	linked_list_t *revoked;

	/**
	 * List of Freshest CRL distribution points
	 */
	linked_list_t *crl_uris;

	/**
	 * Authority Key Identifier
	 */
	chunk_t authKeyIdentifier;

	/**
	 * Authority Key Serial Number
	 */
	chunk_t authKeySerialNumber;

	/**
	 * Number of BaseCRL, if a delta CRL
	 */
	chunk_t baseCrlNumber;

	/**
	 * Signature algorithm
	 */
	int algorithm;

	/**
	 * Signature
	 */
	chunk_t signature;

	/**
	 * has this CRL been generated
	 */
	bool generated;

	/**
	 * reference counter
	 */
	refcount_t ref;
};

/**
 * from x509_cert
 */
extern chunk_t x509_parse_authorityKeyIdentifier(chunk_t blob, int level0,
												 chunk_t *authKeySerialNumber);

/**
 * from x509_cert
 */
extern void x509_parse_crlDistributionPoints(chunk_t blob, int level0,
											 linked_list_t *list);

/**
 * from x509_cert
 */
extern chunk_t x509_build_crlDistributionPoints(linked_list_t *list, int extn);

/**
  * ASN.1 definition of an X.509 certificate revocation list
 */
static const asn1Object_t crlObjects[] = {
	{ 0, "certificateList",				ASN1_SEQUENCE,     ASN1_OBJ  }, /*  0 */
	{ 1,   "tbsCertList",				ASN1_SEQUENCE,     ASN1_OBJ  }, /*  1 */
	{ 2,     "version",					ASN1_INTEGER,      ASN1_OPT |
														   ASN1_BODY }, /*  2 */
	{ 2,     "end opt",					ASN1_EOC,          ASN1_END  }, /*  3 */
	{ 2,     "signature",				ASN1_EOC,          ASN1_RAW  }, /*  4 */
	{ 2,     "issuer",					ASN1_SEQUENCE,     ASN1_OBJ  }, /*  5 */
	{ 2,     "thisUpdate",				ASN1_EOC,          ASN1_RAW  }, /*  6 */
	{ 2,     "nextUpdate",				ASN1_EOC,          ASN1_RAW  }, /*  7 */
	{ 2,     "revokedCertificates",		ASN1_SEQUENCE,     ASN1_OPT |
														   ASN1_LOOP }, /*  8 */
	{ 3,       "certList",				ASN1_SEQUENCE,     ASN1_NONE }, /*  9 */
	{ 4,         "userCertificate",		ASN1_INTEGER,      ASN1_BODY }, /* 10 */
	{ 4,         "revocationDate",		ASN1_EOC,          ASN1_RAW  }, /* 11 */
	{ 4,         "crlEntryExtensions",  ASN1_SEQUENCE,     ASN1_OPT |
														   ASN1_LOOP }, /* 12 */
	{ 5,           "extension",			ASN1_SEQUENCE,	   ASN1_NONE }, /* 13 */
	{ 6,             "extnID",			ASN1_OID,          ASN1_BODY }, /* 14 */
	{ 6,             "critical",		ASN1_BOOLEAN,      ASN1_DEF |
														   ASN1_BODY }, /* 15 */
	{ 6,             "extnValue",		ASN1_OCTET_STRING, ASN1_BODY }, /* 16 */
	{ 4,         "end opt or loop",		ASN1_EOC,          ASN1_END  }, /* 17 */
	{ 2,     "end opt or loop",			ASN1_EOC,          ASN1_END  }, /* 18 */
	{ 2,     "optional extensions",		ASN1_CONTEXT_C_0,  ASN1_OPT  }, /* 19 */
	{ 3,       "crlExtensions",			ASN1_SEQUENCE,     ASN1_LOOP }, /* 20 */
	{ 4,         "extension",			ASN1_SEQUENCE,     ASN1_NONE }, /* 21 */
	{ 5,           "extnID",			ASN1_OID,          ASN1_BODY }, /* 22 */
	{ 5,           "critical",			ASN1_BOOLEAN,      ASN1_DEF |
														   ASN1_BODY }, /* 23 */
	{ 5,           "extnValue",			ASN1_OCTET_STRING, ASN1_BODY }, /* 24 */
	{ 3,       "end loop",				ASN1_EOC,          ASN1_END  }, /* 25 */
	{ 2,     "end opt",					ASN1_EOC,          ASN1_END  }, /* 26 */
	{ 1,   "signatureAlgorithm",		ASN1_EOC,          ASN1_RAW  }, /* 27 */
	{ 1,   "signatureValue",			ASN1_BIT_STRING,   ASN1_BODY }, /* 28 */
	{ 0, "exit",						ASN1_EOC,		   ASN1_EXIT }
};
#define CRL_OBJ_TBS_CERT_LIST			 1
#define CRL_OBJ_VERSION					 2
#define CRL_OBJ_SIG_ALG					 4
#define CRL_OBJ_ISSUER					 5
#define CRL_OBJ_THIS_UPDATE				 6
#define CRL_OBJ_NEXT_UPDATE				 7
#define CRL_OBJ_USER_CERTIFICATE		10
#define CRL_OBJ_REVOCATION_DATE			11
#define CRL_OBJ_CRL_ENTRY_EXTN_ID		14
#define CRL_OBJ_CRL_ENTRY_CRITICAL		15
#define CRL_OBJ_CRL_ENTRY_EXTN_VALUE	16
#define CRL_OBJ_EXTN_ID					22
#define CRL_OBJ_CRITICAL				23
#define CRL_OBJ_EXTN_VALUE				24
#define CRL_OBJ_ALGORITHM				27
#define CRL_OBJ_SIGNATURE				28

/**
 *  Parses an X.509 Certificate Revocation List (CRL)
 */
static bool parse(private_x509_crl_t *this)
{
	asn1_parser_t *parser;
	chunk_t object;
	chunk_t extnID = chunk_empty;
	chunk_t userCertificate = chunk_empty;
	int objectID;
	int sig_alg = OID_UNKNOWN;
	bool success = FALSE;
	bool critical = FALSE;
	revoked_t *revoked = NULL;

	parser = asn1_parser_create(crlObjects, this->encoding);

	while (parser->iterate(parser, &objectID, &object))
	{
		u_int level = parser->get_level(parser)+1;

		switch (objectID)
		{
			case CRL_OBJ_TBS_CERT_LIST:
				this->tbsCertList = object;
				break;
			case CRL_OBJ_VERSION:
				this->version = (object.len) ? (1+(u_int)*object.ptr) : 1;
				DBG2(DBG_ASN, "  v%d", this->version);
				break;
			case CRL_OBJ_SIG_ALG:
				sig_alg = asn1_parse_algorithmIdentifier(object, level, NULL);
				break;
			case CRL_OBJ_ISSUER:
				this->issuer = identification_create_from_encoding(ID_DER_ASN1_DN, object);
				DBG2(DBG_ASN, "  '%Y'", this->issuer);
				break;
			case CRL_OBJ_THIS_UPDATE:
				this->thisUpdate = asn1_parse_time(object, level);
				break;
			case CRL_OBJ_NEXT_UPDATE:
				this->nextUpdate = asn1_parse_time(object, level);
				break;
			case CRL_OBJ_USER_CERTIFICATE:
				userCertificate = object;
				break;
			case CRL_OBJ_REVOCATION_DATE:
				revoked = malloc_thing(revoked_t);
				revoked->serial = chunk_clone(userCertificate);
				revoked->date = asn1_parse_time(object, level);
				revoked->reason = CRL_REASON_UNSPECIFIED;
				this->revoked->insert_last(this->revoked, (void *)revoked);
				break;
			case CRL_OBJ_CRL_ENTRY_EXTN_ID:
			case CRL_OBJ_EXTN_ID:
				extnID = object;
				break;
			case CRL_OBJ_CRL_ENTRY_CRITICAL:
			case CRL_OBJ_CRITICAL:
				critical = object.len && *object.ptr;
				DBG2(DBG_ASN, "  %s", critical ? "TRUE" : "FALSE");
				break;
			case CRL_OBJ_CRL_ENTRY_EXTN_VALUE:
			case CRL_OBJ_EXTN_VALUE:
			{
				int extn_oid = asn1_known_oid(extnID);

				switch (extn_oid)
				{
					case OID_CRL_REASON_CODE:
						if (revoked)
						{
							if (object.len && *object.ptr == ASN1_ENUMERATED &&
								asn1_length(&object) == 1)
							{
								revoked->reason = *object.ptr;
							}
							DBG2(DBG_ASN, "  '%N'", crl_reason_names,
								 revoked->reason);
						}
						break;
					case OID_AUTHORITY_KEY_ID:
						this->authKeyIdentifier =
							x509_parse_authorityKeyIdentifier(
									object, level, &this->authKeySerialNumber);
						break;
					case OID_CRL_NUMBER:
						if (!asn1_parse_simple_object(&object, ASN1_INTEGER,
													  level, "crlNumber"))
						{
							goto end;
						}
						this->crlNumber = object;
						break;
					case OID_FRESHEST_CRL:
						x509_parse_crlDistributionPoints(object, level,
														 this->crl_uris);
						break;
					case OID_DELTA_CRL_INDICATOR:
						if (!asn1_parse_simple_object(&object, ASN1_INTEGER,
													level, "deltaCrlIndicator"))
						{
							goto end;
						}
						this->baseCrlNumber = object;
						break;
					case OID_ISSUING_DIST_POINT:
						/* TODO support of IssuingDistributionPoints */
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
			case CRL_OBJ_ALGORITHM:
			{
				this->algorithm = asn1_parse_algorithmIdentifier(object, level, NULL);
				if (this->algorithm != sig_alg)
				{
					DBG1(DBG_ASN, "  signature algorithms do not agree");
					goto end;
				}
				break;
			}
			case CRL_OBJ_SIGNATURE:
				this->signature = object;
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
 * enumerator filter callback for create_enumerator
 */
static bool filter(void *data, revoked_t **revoked, chunk_t *serial, void *p2,
				   time_t *date, void *p3, crl_reason_t *reason)
{
	if (serial)
	{
		*serial = (*revoked)->serial;
	}
	if (date)
	{
		*date = (*revoked)->date;
	}
	if (reason)
	{
		*reason = (*revoked)->reason;
	}
	return TRUE;
}

METHOD(crl_t, get_serial, chunk_t,
	private_x509_crl_t *this)
{
	return this->crlNumber;
}

METHOD(crl_t, get_authKeyIdentifier, chunk_t,
	private_x509_crl_t *this)
{
	return this->authKeyIdentifier;
}

METHOD(crl_t, is_delta_crl, bool,
	private_x509_crl_t *this, chunk_t *base_crl)
{
	if (this->baseCrlNumber.len)
	{
		if (base_crl)
		{
			*base_crl = this->baseCrlNumber;
		}
		return TRUE;
	}
	return FALSE;
}

METHOD(crl_t, create_delta_crl_uri_enumerator, enumerator_t*,
	private_x509_crl_t *this)
{
	return this->crl_uris->create_enumerator(this->crl_uris);
}

METHOD(crl_t, create_enumerator, enumerator_t*,
	private_x509_crl_t *this)
{
	return enumerator_create_filter(
								this->revoked->create_enumerator(this->revoked),
								(void*)filter, NULL, NULL);
}

METHOD(certificate_t, get_type, certificate_type_t,
	private_x509_crl_t *this)
{
	return CERT_X509_CRL;
}

METHOD(certificate_t, get_issuer, identification_t*,
	private_x509_crl_t *this)
{
	return this->issuer;
}

METHOD(certificate_t, has_issuer, id_match_t,
	private_x509_crl_t *this, identification_t *issuer)
{
	if (issuer->get_type(issuer) == ID_KEY_ID && this->authKeyIdentifier.ptr &&
		chunk_equals(this->authKeyIdentifier, issuer->get_encoding(issuer)))
	{
		return ID_MATCH_PERFECT;
	}
	return this->issuer->matches(this->issuer, issuer);
}

METHOD(certificate_t, issued_by, bool,
	private_x509_crl_t *this, certificate_t *issuer, signature_scheme_t *schemep)
{
	public_key_t *key;
	signature_scheme_t scheme;
	bool valid;
	x509_t *x509 = (x509_t*)issuer;

	/* check if issuer is an X.509 CA certificate */
	if (issuer->get_type(issuer) != CERT_X509)
	{
		return FALSE;
	}
	if (!(x509->get_flags(x509) & (X509_CA | X509_CRL_SIGN)))
	{
		return FALSE;
	}

	/* get the public key of the issuer */
	key = issuer->get_public_key(issuer);

	/* compare keyIdentifiers if available, otherwise use DNs */
	if (this->authKeyIdentifier.ptr && key)
	{
		chunk_t fingerprint;

		if (!key->get_fingerprint(key, KEYID_PUBKEY_SHA1, &fingerprint) ||
			!chunk_equals(fingerprint, this->authKeyIdentifier))
		{
			return FALSE;
		}
	}
	else
	{
		if (!this->issuer->equals(this->issuer, issuer->get_subject(issuer)))
		{
			return FALSE;
		}
	}

	/* determine signature scheme */
	scheme = signature_scheme_from_oid(this->algorithm);

	if (scheme == SIGN_UNKNOWN || key == NULL)
	{
		return FALSE;
	}
	valid = key->verify(key, scheme, this->tbsCertList, this->signature);
	key->destroy(key);
	if (valid && schemep)
	{
		*schemep = scheme;
	}
	return valid;
}

METHOD(certificate_t, get_public_key, public_key_t*,
	private_x509_crl_t *this)
{
	return NULL;
}

METHOD(certificate_t, get_ref, certificate_t*,
	private_x509_crl_t *this)
{
	ref_get(&this->ref);
	return &this->public.crl.certificate;
}

METHOD(certificate_t, get_validity, bool,
	private_x509_crl_t *this, time_t *when,
	time_t *not_before, time_t *not_after)
{
	time_t t = when ? *when : time(NULL);

	if (not_before)
	{
		*not_before = this->thisUpdate;
	}
	if (not_after)
	{
		*not_after = this->nextUpdate;
	}
	return (t <= this->nextUpdate);
}

METHOD(certificate_t, get_encoding, bool,
	private_x509_crl_t *this, cred_encoding_type_t type, chunk_t *encoding)
{
	if (type == CERT_ASN1_DER)
	{
		*encoding = chunk_clone(this->encoding);
		return TRUE;
	}
	return lib->encoding->encode(lib->encoding, type, NULL, encoding,
					CRED_PART_X509_CRL_ASN1_DER, this->encoding, CRED_PART_END);
}

METHOD(certificate_t, equals, bool,
	private_x509_crl_t *this, certificate_t *other)
{
	chunk_t encoding;
	bool equal;

	if ((certificate_t*)this == other)
	{
		return TRUE;
	}
	if (other->equals == (void*)equals)
	{	/* skip allocation if we have the same implementation */
		return chunk_equals(this->encoding, ((private_x509_crl_t*)other)->encoding);
	}
	if (!other->get_encoding(other, CERT_ASN1_DER, &encoding))
	{
		return FALSE;
	}
	equal = chunk_equals(this->encoding, encoding);
	free(encoding.ptr);
	return equal;
}

/**
 * Destroy a revoked_t entry
 */
static void revoked_destroy(revoked_t *revoked)
{
	free(revoked->serial.ptr);
	free(revoked);
}

/**
 * Destroy a CDP entry
 */
static void cdp_destroy(x509_cdp_t *this)
{
	free(this->uri);
	DESTROY_IF(this->issuer);
	free(this);
}

METHOD(certificate_t, destroy, void,
	private_x509_crl_t *this)
{
	if (ref_put(&this->ref))
	{
		this->revoked->destroy_function(this->revoked, (void*)revoked_destroy);
		this->crl_uris->destroy_function(this->crl_uris, (void*)cdp_destroy);
		DESTROY_IF(this->issuer);
		free(this->authKeyIdentifier.ptr);
		free(this->encoding.ptr);
		if (this->generated)
		{
			free(this->crlNumber.ptr);
			free(this->baseCrlNumber.ptr);
			free(this->signature.ptr);
			free(this->tbsCertList.ptr);
		}
		free(this);
	}
}

/**
 * create an empty but initialized X.509 crl
 */
static private_x509_crl_t* create_empty(void)
{
	private_x509_crl_t *this;

	INIT(this,
		.public = {
			.crl = {
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
				.get_serial = _get_serial,
				.get_authKeyIdentifier = _get_authKeyIdentifier,
				.is_delta_crl = _is_delta_crl,
				.create_delta_crl_uri_enumerator = _create_delta_crl_uri_enumerator,
				.create_enumerator = _create_enumerator,
			},
		},
		.revoked = linked_list_create(),
		.crl_uris = linked_list_create(),
		.ref = 1,
	);
	return this;
}

/**
 * See header.
 */
x509_crl_t *x509_crl_load(certificate_type_t type, va_list args)
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
		private_x509_crl_t *crl = create_empty();

		crl->encoding = chunk_clone(blob);
		if (parse(crl))
		{
			return &crl->public;
		}
		destroy(crl);
	}
	return NULL;
};

/**
 * Read certificate status from enumerator, copy to crl
 */
static void read_revoked(private_x509_crl_t *crl, enumerator_t *enumerator)
{
	revoked_t *revoked;
	chunk_t serial;
	time_t date;
	crl_reason_t reason;

	while (enumerator->enumerate(enumerator, &serial, &date, &reason))
	{
		INIT(revoked,
			.serial = chunk_clone(serial),
			.date = date,
			.reason = reason,
		);
		crl->revoked->insert_last(crl->revoked, revoked);
	}
}

/**
 * Generate CRL encoding, sign CRL
 */
static bool generate(private_x509_crl_t *this, certificate_t *cert,
					 private_key_t *key, hash_algorithm_t digest_alg)
{
	chunk_t extensions = chunk_empty, certList = chunk_empty, serial;
	chunk_t crlDistributionPoints = chunk_empty, baseCrlNumber = chunk_empty;
	enumerator_t *enumerator;
	crl_reason_t reason;
	time_t date;
	x509_t *x509;

	x509 = (x509_t*)cert;

	this->issuer = cert->get_subject(cert);
	this->issuer = this->issuer->clone(this->issuer);

	this->authKeyIdentifier = chunk_clone(x509->get_subjectKeyIdentifier(x509));

	/* select signature scheme */
	this->algorithm = hasher_signature_algorithm_to_oid(digest_alg,
														key->get_type(key));
	if (this->algorithm == OID_UNKNOWN)
	{
		return FALSE;
	}

	enumerator = create_enumerator(this);
	while (enumerator->enumerate(enumerator, &serial, &date, &reason))
	{
		chunk_t revoked, entry_ext = chunk_empty;

		if (reason != CRL_REASON_UNSPECIFIED)
		{
			entry_ext = asn1_wrap(ASN1_SEQUENCE, "m",
							asn1_wrap(ASN1_SEQUENCE, "mm",
								asn1_build_known_oid(OID_CRL_REASON_CODE),
								asn1_wrap(ASN1_OCTET_STRING, "m",
									asn1_wrap(ASN1_ENUMERATED, "c",
										chunk_from_chars(reason)))));
		}
		revoked = asn1_wrap(ASN1_SEQUENCE, "mmm",
							asn1_integer("c", serial),
							asn1_from_time(&date, ASN1_UTCTIME),
							entry_ext);
		certList = chunk_cat("mm", certList, revoked);
	}
	enumerator->destroy(enumerator);

	crlDistributionPoints = x509_build_crlDistributionPoints(this->crl_uris,
															 OID_FRESHEST_CRL);

	if (this->baseCrlNumber.len)
	{
		baseCrlNumber =  asn1_wrap(ASN1_SEQUENCE, "mmm",
							asn1_build_known_oid(OID_DELTA_CRL_INDICATOR),
							asn1_wrap(ASN1_BOOLEAN, "c",
								chunk_from_chars(0xFF)),
							asn1_wrap(ASN1_OCTET_STRING, "m",
								asn1_integer("c", this->baseCrlNumber)));
	}

	extensions = asn1_wrap(ASN1_CONTEXT_C_0, "m",
					asn1_wrap(ASN1_SEQUENCE, "mmmm",
						asn1_wrap(ASN1_SEQUENCE, "mm",
							asn1_build_known_oid(OID_AUTHORITY_KEY_ID),
							asn1_wrap(ASN1_OCTET_STRING, "m",
								asn1_wrap(ASN1_SEQUENCE, "m",
									asn1_wrap(ASN1_CONTEXT_S_0, "c",
											  this->authKeyIdentifier)))),
						asn1_wrap(ASN1_SEQUENCE, "mm",
							asn1_build_known_oid(OID_CRL_NUMBER),
							asn1_wrap(ASN1_OCTET_STRING, "m",
								asn1_integer("c", this->crlNumber))),
						crlDistributionPoints, baseCrlNumber));

	this->tbsCertList = asn1_wrap(ASN1_SEQUENCE, "cmcmmmm",
							ASN1_INTEGER_1,
							asn1_algorithmIdentifier(this->algorithm),
							this->issuer->get_encoding(this->issuer),
							asn1_from_time(&this->thisUpdate, ASN1_UTCTIME),
							asn1_from_time(&this->nextUpdate, ASN1_UTCTIME),
							asn1_wrap(ASN1_SEQUENCE, "m", certList),
							extensions);

	if (!key->sign(key, signature_scheme_from_oid(this->algorithm),
				   this->tbsCertList, &this->signature))
	{
		return FALSE;
	}
	this->encoding = asn1_wrap(ASN1_SEQUENCE, "cmm",
							this->tbsCertList,
							asn1_algorithmIdentifier(this->algorithm),
							asn1_bitstring("c", this->signature));
	return TRUE;
}

/**
 * See header.
 */
x509_crl_t *x509_crl_gen(certificate_type_t type, va_list args)
{
	hash_algorithm_t digest_alg = HASH_SHA1;
	private_x509_crl_t *crl;
	certificate_t *cert = NULL;
	private_key_t *key = NULL;

	crl = create_empty();
	crl->generated = TRUE;
	while (TRUE)
	{
		builder_part_t part = va_arg(args, builder_part_t);

		switch (part)
		{
			case BUILD_SIGNING_KEY:
				key = va_arg(args, private_key_t*);
				continue;
			case BUILD_SIGNING_CERT:
				cert = va_arg(args, certificate_t*);
				continue;
			case BUILD_NOT_BEFORE_TIME:
				crl->thisUpdate = va_arg(args, time_t);
				continue;
			case BUILD_NOT_AFTER_TIME:
				crl->nextUpdate = va_arg(args, time_t);
				continue;
			case BUILD_SERIAL:
				crl->crlNumber = va_arg(args, chunk_t);
				crl->crlNumber = chunk_clone(crl->crlNumber);
				continue;
			case BUILD_DIGEST_ALG:
				digest_alg = va_arg(args, int);
				continue;
			case BUILD_REVOKED_ENUMERATOR:
				read_revoked(crl, va_arg(args, enumerator_t*));
				continue;
			case BUILD_BASE_CRL:
				crl->baseCrlNumber = va_arg(args, chunk_t);
				crl->baseCrlNumber = chunk_clone(crl->baseCrlNumber);
				break;
			case BUILD_CRL_DISTRIBUTION_POINTS:
			{
				enumerator_t *enumerator;
				linked_list_t *list;
				x509_cdp_t *in, *cdp;

				list = va_arg(args, linked_list_t*);
				enumerator = list->create_enumerator(list);
				while (enumerator->enumerate(enumerator, &in))
				{
					INIT(cdp,
						.uri = strdup(in->uri),
						.issuer = in->issuer ? in->issuer->clone(in->issuer) : NULL,
					);
					crl->crl_uris->insert_last(crl->crl_uris, cdp);
				}
				enumerator->destroy(enumerator);
				continue;
			}
			case BUILD_END:
				break;
			default:
				destroy(crl);
				return NULL;
		}
		break;
	}

	if (key && cert && cert->get_type(cert) == CERT_X509 &&
		generate(crl, cert, key, digest_alg))
	{
		return &crl->public;
	}
	destroy(crl);
	return NULL;
}
