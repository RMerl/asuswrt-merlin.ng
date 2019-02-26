/*
 * Copyright (C) 2005 Jan Hutter, Martin Willi
 * Copyright (C) 2009-2017 Andreas Steffen
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

#include "x509_pkcs10.h"

#include <library.h>
#include <utils/debug.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <credentials/keys/private_key.h>
#include <collections/linked_list.h>
#include <utils/identification.h>

typedef struct private_x509_pkcs10_t private_x509_pkcs10_t;

/**
 * Private data of a x509_pkcs10_t object.
 */
struct private_x509_pkcs10_t {
	/**
	 * Public interface for this certificate.
	 */
	x509_pkcs10_t public;

	/**
	 * PKCS#10 certificate request encoding in ASN.1 DER format
	 */
	chunk_t encoding;

	/**
	 * PKCS#10 request body over which signature is computed
	 */
	chunk_t certificationRequestInfo;

	/**
	 * Version of the PKCS#10 certificate request
	 */
	u_int version;

	/**
	 * ID representing the certificate subject
	 */
	identification_t *subject;

	/**
	 * List of subjectAltNames as identification_t
	 */
	linked_list_t *subjectAltNames;

	/**
	 * certificate's embedded public key
	 */
	public_key_t *public_key;

	/**
	 * challenge password
	 */
	chunk_t challengePassword;

	/**
	 * Signature scheme
	 */
	signature_params_t *scheme;

	/**
	 * Signature
	 */
	chunk_t signature;

	/**
	 * Is the certificate request self-signed?
	 */
	bool self_signed;

	/**
	 * Certificate request parsed from blob/file?
	 */
	bool parsed;

	/**
	 * reference count
	 */
	refcount_t ref;
};

/**
 * Imported from x509_cert.c
 */
extern bool x509_parse_generalNames(chunk_t blob, int level0, bool implicit,
									linked_list_t *list);
extern chunk_t x509_build_subjectAltNames(linked_list_t *list);

METHOD(certificate_t, get_type, certificate_type_t,
	private_x509_pkcs10_t *this)
{
	return CERT_PKCS10_REQUEST;
}

METHOD(certificate_t, get_subject, identification_t*,
	private_x509_pkcs10_t *this)
{
	return this->subject;
}

METHOD(certificate_t, has_subject, id_match_t,
	private_x509_pkcs10_t *this, identification_t *subject)
{
	return this->subject->matches(this->subject, subject);
}

METHOD(certificate_t, issued_by, bool,
	private_x509_pkcs10_t *this, certificate_t *issuer,
	signature_params_t **scheme)
{
	public_key_t *key;
	bool valid;

	if (&this->public.interface.interface != issuer)
	{
		return FALSE;
	}
	if (this->self_signed)
	{
		valid = TRUE;
	}
	else
	{
		/* get the public key contained in the certificate request */
		key = this->public_key;
		if (!key)
		{
			return FALSE;
		}
		valid = key->verify(key, this->scheme->scheme, this->scheme->params,
							this->certificationRequestInfo, this->signature);
	}
	if (valid && scheme)
	{
		*scheme = signature_params_clone(this->scheme);
	}
	return valid;
}

METHOD(certificate_t, get_public_key, public_key_t*,
	private_x509_pkcs10_t *this)
{
	this->public_key->get_ref(this->public_key);
	return this->public_key;
}

METHOD(certificate_t, get_validity, bool,
	private_x509_pkcs10_t *this, time_t *when, time_t *not_before,
	time_t *not_after)
{
	if (not_before)
	{
		*not_before = 0;
	}
	if (not_after)
	{
		*not_after = ~0;
	}
	return TRUE;
}

METHOD(certificate_t, get_encoding, bool,
	private_x509_pkcs10_t *this, cred_encoding_type_t type, chunk_t *encoding)
{
	if (type == CERT_ASN1_DER)
	{
		*encoding = chunk_clone(this->encoding);
		return TRUE;
	}
	return lib->encoding->encode(lib->encoding, type, NULL, encoding,
					CRED_PART_PKCS10_ASN1_DER, this->encoding, CRED_PART_END);
}

METHOD(certificate_t, equals, bool,
	private_x509_pkcs10_t *this, certificate_t *other)
{
	chunk_t encoding;
	bool equal;

	if (this == (private_x509_pkcs10_t*)other)
	{
		return TRUE;
	}
	if (other->get_type(other) != CERT_PKCS10_REQUEST)
	{
		return FALSE;
	}
	if (other->equals == (void*)equals)
	{	/* skip allocation if we have the same implementation */
		return chunk_equals(this->encoding, ((private_x509_pkcs10_t*)other)->encoding);
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
	private_x509_pkcs10_t *this)
{
	ref_get(&this->ref);
	return &this->public.interface.interface;
}

METHOD(pkcs10_t, get_challengePassword, chunk_t,
	private_x509_pkcs10_t *this)
{
	return this->challengePassword;
}

METHOD(pkcs10_t, create_subjectAltName_enumerator, enumerator_t*,
	private_x509_pkcs10_t *this)
{
	return this->subjectAltNames->create_enumerator(this->subjectAltNames);
}

/**
 * ASN.1 definition of a PKCS#10 extension request
 */
static const asn1Object_t extensionRequestObjects[] = {
	{ 0, "extensions",   ASN1_SEQUENCE,     ASN1_LOOP           }, /* 0 */
	{ 1,   "extension",   ASN1_SEQUENCE,     ASN1_NONE          }, /* 1 */
	{ 2,     "extnID",	  ASN1_OID,          ASN1_BODY          }, /* 2 */
	{ 2,     "critical",  ASN1_BOOLEAN,      ASN1_DEF|ASN1_BODY }, /* 3 */
	{ 2,     "extnValue", ASN1_OCTET_STRING, ASN1_BODY          }, /* 4 */
	{ 1, "end loop",      ASN1_EOC,          ASN1_END			}, /* 5 */
	{ 0, "exit",          ASN1_EOC,          ASN1_EXIT          }
};
#define PKCS10_EXTN_ID			2
#define PKCS10_EXTN_CRITICAL	3
#define PKCS10_EXTN_VALUE		4

/**
 * Parses a PKCS#10 extension request
 */
static bool parse_extension_request(private_x509_pkcs10_t *this, chunk_t blob, int level0)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	int extn_oid = OID_UNKNOWN;
	bool success = FALSE;
	bool critical;

	parser = asn1_parser_create(extensionRequestObjects, blob);
	parser->set_top_level(parser, level0);

	while (parser->iterate(parser, &objectID, &object))
	{
		u_int level = parser->get_level(parser)+1;

		switch (objectID)
		{
			case PKCS10_EXTN_ID:
				extn_oid = asn1_known_oid(object);
				break;
			case PKCS10_EXTN_CRITICAL:
				critical = object.len && *object.ptr;
				DBG2(DBG_ASN, "  %s", critical ? "TRUE" : "FALSE");
				break;
			case PKCS10_EXTN_VALUE:
			{
				switch (extn_oid)
				{
					case OID_SUBJECT_ALT_NAME:
						if (!x509_parse_generalNames(object, level, FALSE,
													 this->subjectAltNames))
						{
							goto end;
						}
						break;
					default:
						break;
				}
				break;
			}
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
 * Parses a PKCS#10 challenge password
 */
static bool parse_challengePassword(private_x509_pkcs10_t *this, chunk_t blob, int level)
{
	char tag;

	if (blob.len < 2)
	{
		DBG1(DBG_ASN, "L%d - challengePassword:  ASN.1 object smaller "
			 "than 2 octets", level);
		return FALSE;
	}
	tag = *blob.ptr;
	if (tag < ASN1_UTF8STRING || tag > ASN1_IA5STRING)
	{
		DBG1(DBG_ASN, "L%d - challengePassword:  ASN.1 object is not "
			 "a character string", level);
		return FALSE;
	}
	if (asn1_length(&blob) == ASN1_INVALID_LENGTH)
	{
		DBG1(DBG_ASN, "L%d - challengePassword:  ASN.1 object has an "
			 "invalid length", level);
		return FALSE;
	}
	DBG2(DBG_ASN, "L%d - challengePassword:", level);
	DBG4(DBG_ASN, "  '%.*s'", (int)blob.len, blob.ptr);
	return TRUE;
}

/**
 * ASN.1 definition of a PKCS#10 certificate request
 */
static const asn1Object_t certificationRequestObjects[] = {
	{ 0, "certificationRequest",       ASN1_SEQUENCE,    ASN1_OBJ  }, /*  0 */
	{ 1,   "certificationRequestInfo", ASN1_SEQUENCE,    ASN1_OBJ  }, /*  1 */
	{ 2,     "version",                ASN1_INTEGER,     ASN1_BODY }, /*  2 */
	{ 2,     "subject",                ASN1_SEQUENCE,    ASN1_OBJ  }, /*  3 */
	{ 2,     "subjectPublicKeyInfo",   ASN1_SEQUENCE,    ASN1_RAW  }, /*  4 */
	{ 2,     "attributes",             ASN1_CONTEXT_C_0, ASN1_LOOP }, /*  5 */
	{ 3,       "attribute",            ASN1_SEQUENCE,    ASN1_NONE }, /*  6 */
	{ 4,         "type",               ASN1_OID,         ASN1_BODY }, /*  7 */
	{ 4,         "values",             ASN1_SET,         ASN1_LOOP }, /*  8 */
	{ 5,           "value",            ASN1_EOC,         ASN1_RAW  }, /*  9 */
	{ 4,         "end loop",           ASN1_EOC,         ASN1_END  }, /* 10 */
	{ 2,     "end loop",               ASN1_EOC,         ASN1_END  }, /* 11 */
	{ 1,   "signatureAlgorithm",       ASN1_EOC,         ASN1_RAW  }, /* 12 */
	{ 1,    "signature",               ASN1_BIT_STRING,  ASN1_BODY }, /* 13 */
	{ 0, "exit",                       ASN1_EOC,         ASN1_EXIT }
};
#define PKCS10_CERT_REQUEST_INFO		 1
#define PKCS10_VERSION					 2
#define PKCS10_SUBJECT					 3
#define PKCS10_SUBJECT_PUBLIC_KEY_INFO	 4
#define PKCS10_ATTR_TYPE				 7
#define PKCS10_ATTR_VALUE				 9
#define PKCS10_ALGORITHM				12
#define PKCS10_SIGNATURE				13

/**
 * Parses a PKCS#10 certificate request
 */
static bool parse_certificate_request(private_x509_pkcs10_t *this)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	int attr_oid = OID_UNKNOWN;
	bool success = FALSE;

	parser = asn1_parser_create(certificationRequestObjects, this->encoding);

	while (parser->iterate(parser, &objectID, &object))
	{
		u_int level = parser->get_level(parser)+1;

		switch (objectID)
		{
			case PKCS10_CERT_REQUEST_INFO:
				this->certificationRequestInfo = object;
				break;
			case PKCS10_VERSION:
				if (object.len > 0 && *object.ptr != 0)
				{
					DBG1(DBG_ASN, "PKCS#10 certificate request format is "
						 "not version 1");
					goto end;
				}
				break;
			case PKCS10_SUBJECT:
				this->subject = identification_create_from_encoding(ID_DER_ASN1_DN, object);
				DBG2(DBG_ASN, "  '%Y'", this->subject);
				break;
			case PKCS10_SUBJECT_PUBLIC_KEY_INFO:
				this->public_key = lib->creds->create(lib->creds, CRED_PUBLIC_KEY,
						KEY_ANY, BUILD_BLOB_ASN1_DER, object, BUILD_END);
				if (!this->public_key)
				{
					goto end;
				}
				break;
			case PKCS10_ATTR_TYPE:
				attr_oid = asn1_known_oid(object);
				break;
			case PKCS10_ATTR_VALUE:
				switch (attr_oid)
				{
					case OID_EXTENSION_REQUEST:
						if (!parse_extension_request(this, object, level))
						{
							goto end;
						}
						break;
					case OID_CHALLENGE_PASSWORD:
						if (!parse_challengePassword(this, object, level))
						{
							goto end;
						}
						break;
					default:
						break;
				}
				break;
			case PKCS10_ALGORITHM:
				INIT(this->scheme);
				if (!signature_params_parse(object, level, this->scheme))
				{
					DBG1(DBG_ASN, "  unable to parse signature algorithm");
					goto end;
				}
				break;
			case PKCS10_SIGNATURE:
				this->signature = chunk_skip(object, 1);
				break;
			default:
				break;
		}
	}
	success = parser->success(parser);

end:
	parser->destroy(parser);
	if (success)
	{
		/* check if the certificate request is self-signed */
		if (issued_by(this, &this->public.interface.interface, NULL))
		{
			this->self_signed = TRUE;
		}
		else
		{
			DBG1(DBG_LIB, "certificate request is not self-signed");
			success = FALSE;
		}
	}
	return success;
}

METHOD(certificate_t, destroy, void,
	private_x509_pkcs10_t *this)
{
	if (ref_put(&this->ref))
	{
		this->subjectAltNames->destroy_offset(this->subjectAltNames,
									offsetof(identification_t, destroy));
		signature_params_destroy(this->scheme);
		DESTROY_IF(this->subject);
		DESTROY_IF(this->public_key);
		chunk_free(&this->encoding);
		if (!this->parsed)
		{	/* only parsed certificate requests point these fields to "encoded" */
			chunk_free(&this->certificationRequestInfo);
			chunk_free(&this->challengePassword);
			chunk_free(&this->signature);
		}
		free(this);
	}
}

/**
 * create an empty but initialized PKCS#10 certificate request
 */
static private_x509_pkcs10_t* create_empty(void)
{
	private_x509_pkcs10_t *this;

	INIT(this,
		.public = {
			.interface = {
				.interface = {
					.get_type = _get_type,
					.get_subject = _get_subject,
					.get_issuer = _get_subject,
					.has_subject = _has_subject,
					.has_issuer = _has_subject,
					.issued_by = _issued_by,
					.get_public_key = _get_public_key,
					.get_validity = _get_validity,
					.get_encoding = _get_encoding,
					.equals = _equals,
					.get_ref = _get_ref,
					.destroy = _destroy,
				},
				.get_challengePassword = _get_challengePassword,
				.create_subjectAltName_enumerator = _create_subjectAltName_enumerator,
			},
		},
		.subjectAltNames = linked_list_create(),
		.ref = 1,
	);

	return this;
}

/**
 * Generate and sign a new certificate request
 */
static bool generate(private_x509_pkcs10_t *cert, private_key_t *sign_key,
					 int digest_alg)
{
	chunk_t key_info, subjectAltNames, attributes;
	chunk_t extensionRequest  = chunk_empty;
	chunk_t challengePassword = chunk_empty, sig_scheme = chunk_empty;
	identification_t *subject;

	subject = cert->subject;
	cert->public_key = sign_key->get_public_key(sign_key);

	/* select signature scheme, if not already specified */
	if (!cert->scheme)
	{
		INIT(cert->scheme,
			.scheme = signature_scheme_from_oid(
								hasher_signature_algorithm_to_oid(digest_alg,
												sign_key->get_type(sign_key))),
		);
	}
	if (cert->scheme->scheme == SIGN_UNKNOWN)
	{
		return FALSE;
	}
	if (!signature_params_build(cert->scheme, &sig_scheme))
	{
		return FALSE;
	}

	if (!cert->public_key->get_encoding(cert->public_key,
										PUBKEY_SPKI_ASN1_DER, &key_info))
	{
		chunk_free(&sig_scheme);
		return FALSE;
	}

	/* encode subjectAltNames */
	subjectAltNames = x509_build_subjectAltNames(cert->subjectAltNames);

	if (subjectAltNames.ptr)
	{
		extensionRequest = asn1_wrap(ASN1_SEQUENCE, "mm",
					asn1_build_known_oid(OID_EXTENSION_REQUEST),
					asn1_wrap(ASN1_SET, "m",
						asn1_wrap(ASN1_SEQUENCE, "m", subjectAltNames)
					));
	}
	if (cert->challengePassword.len > 0)
	{
		asn1_t type = asn1_is_printablestring(cert->challengePassword) ?
								ASN1_PRINTABLESTRING : ASN1_T61STRING;

		challengePassword = asn1_wrap(ASN1_SEQUENCE, "mm",
					asn1_build_known_oid(OID_CHALLENGE_PASSWORD),
					asn1_wrap(ASN1_SET, "m",
						asn1_simple_object(type, cert->challengePassword)
					)
			);
	}
	attributes = asn1_wrap(ASN1_CONTEXT_C_0, "mm", extensionRequest,
												   challengePassword);

	cert->certificationRequestInfo = asn1_wrap(ASN1_SEQUENCE, "ccmm",
							ASN1_INTEGER_0,
							subject->get_encoding(subject),
							key_info,
							attributes);

	if (!sign_key->sign(sign_key, cert->scheme->scheme, cert->scheme->params,
						cert->certificationRequestInfo, &cert->signature))
	{
		chunk_free(&sig_scheme);
		return FALSE;
	}

	cert->encoding = asn1_wrap(ASN1_SEQUENCE, "cmm",
							   cert->certificationRequestInfo,
							   sig_scheme,
							   asn1_bitstring("c", cert->signature));
	return TRUE;
}

/**
 * See header.
 */
x509_pkcs10_t *x509_pkcs10_load(certificate_type_t type, va_list args)
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
		private_x509_pkcs10_t *cert = create_empty();

		cert->encoding = chunk_clone(blob);
		cert->parsed = TRUE;
		if (parse_certificate_request(cert))
		{
			return &cert->public;
		}
		destroy(cert);
	}
	return NULL;
}

/**
 * See header.
 */
x509_pkcs10_t *x509_pkcs10_gen(certificate_type_t type, va_list args)
{
	private_x509_pkcs10_t *cert;
	private_key_t *sign_key = NULL;
	hash_algorithm_t digest_alg = HASH_SHA1;

	cert = create_empty();
	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_SIGNING_KEY:
				sign_key = va_arg(args, private_key_t*);
				continue;
			case BUILD_SUBJECT:
				cert->subject = va_arg(args, identification_t*);
				cert->subject = cert->subject->clone(cert->subject);
				continue;
			case BUILD_SUBJECT_ALTNAMES:
			{
				enumerator_t *enumerator;
				identification_t *id;
				linked_list_t *list;

				list = va_arg(args, linked_list_t*);
				enumerator = list->create_enumerator(list);
				while (enumerator->enumerate(enumerator, &id))
				{
					cert->subjectAltNames->insert_last(cert->subjectAltNames,
													   id->clone(id));
				}
				enumerator->destroy(enumerator);
				continue;
			}
			case BUILD_CHALLENGE_PWD:
				cert->challengePassword = chunk_clone(va_arg(args, chunk_t));
				continue;
			case BUILD_SIGNATURE_SCHEME:
				cert->scheme = va_arg(args, signature_params_t*);
				cert->scheme = signature_params_clone(cert->scheme);
				continue;
			case BUILD_DIGEST_ALG:
				digest_alg = va_arg(args, int);
				continue;
			case BUILD_END:
				break;
			default:
				destroy(cert);
				return NULL;
		}
		break;
	}

	if (sign_key && generate(cert, sign_key, digest_alg))
	{
		return &cert->public;
	}
	destroy(cert);
	return NULL;
}

