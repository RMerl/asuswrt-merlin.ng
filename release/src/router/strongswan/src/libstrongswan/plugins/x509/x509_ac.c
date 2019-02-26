/*
 * Copyright (C) 2017 Tobias Brunner
 * Copyright (C) 2002 Ueli Galizzi, Ariane Seiler
 * Copyright (C) 2003 Martin Berner, Lukas Suter
 * Copyright (C) 2002-2017 Andreas Steffen
 * Copyright (C) 2009 Martin Willi
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

#include "x509_ac.h"

#include <time.h>

#include <library.h>
#include <utils/debug.h>
#include <asn1/oid.h>
#include <asn1/asn1.h>
#include <asn1/asn1_parser.h>
#include <utils/identification.h>
#include <collections/linked_list.h>
#include <credentials/certificates/x509.h>
#include <credentials/keys/private_key.h>

extern chunk_t x509_parse_authorityKeyIdentifier(chunk_t blob,
									int level0, chunk_t *authKeySerialNumber);

typedef struct private_x509_ac_t private_x509_ac_t;

/**
 * private data of x509_ac_t object
 */
struct private_x509_ac_t {

	/**
	 * public functions
	 */
	x509_ac_t public;

	/**
	 * X.509 attribute certificate encoding in ASN.1 DER format
	 */
	chunk_t encoding;

	/**
	 * X.509 attribute certificate body over which signature is computed
	 */
	chunk_t certificateInfo;

	/**
	 * Version of the X.509 attribute certificate
	 */
	u_int version;

	/**
	 * Serial number of the X.509 attribute certificate
	 */
	chunk_t serialNumber;

	/**
	 * ID representing the issuer of the holder certificate
	 */
	identification_t *holderIssuer;

	/**
	 * Serial number of the holder certificate
	 */
	identification_t *holderSerial;

	/**
	 * ID representing the holder
	 */
	identification_t *entityName;

	/**
	 * ID representing the attribute certificate issuer
	 */
	identification_t *issuerName;

	/**
	 * Start time of certificate validity
	 */
	time_t notBefore;

	/**
	 * End time of certificate validity
	 */
	time_t notAfter;

	/**
	 * List of group attributes, as group_t
	 */
	linked_list_t *groups;

	/**
	 * Authority Key Identifier
	 */
	chunk_t authKeyIdentifier;

	/**
	 * Authority Key Serial Number
	 */
	chunk_t authKeySerialNumber;

	/**
	 * No revocation information available
	 */
	bool noRevAvail;

	/**
	 * Signature scheme
	 */
	signature_params_t *scheme;

	/**
	 * Signature
	 */
	chunk_t signature;

	/**
	 * Holder certificate
	 */
	certificate_t *holderCert;

	/**
	 * Signer certificate
	 */
	certificate_t *signerCert;

	/**
	* Signer private key;
	*/
	private_key_t *signerKey;

	/**
	 * reference count
	 */
	refcount_t ref;
};

/**
 * Group definition, an IETF attribute
 */
typedef struct {
	/** Attribute type */
	ac_group_type_t type;
	/* attribute value */
	chunk_t value;
} group_t;

/**
 * Clean up a group entry
 */
static void group_destroy(group_t *group)
{
	free(group->value.ptr);
	free(group);
}

static chunk_t ASN1_noRevAvail_ext = chunk_from_chars(
	0x30, 0x09,
		  0x06, 0x03,
				0x55, 0x1d, 0x38,
		  0x04, 0x02,
				0x05, 0x00
);

/**
 * declaration of function implemented in x509_cert.c
 */
extern bool x509_parse_generalNames(chunk_t blob, int level0, bool implicit,
									linked_list_t *list);
/**
 * parses a directoryName
 */
static bool parse_directoryName(chunk_t blob, int level, bool implicit,
								identification_t **name)
{
	identification_t *directoryName;
	enumerator_t *enumerator;
	bool first = TRUE;
	linked_list_t *list;

	list = linked_list_create();
	if (!x509_parse_generalNames(blob, level, implicit, list))
	{
		list->destroy(list);
		return FALSE;
	}

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &directoryName))
	{
		if (first)
		{
			*name = directoryName;
			first = FALSE;
		}
		else
		{
			DBG1(DBG_ASN, "more than one directory name - first selected");
			directoryName->destroy(directoryName);
			break;
		}
	}
	enumerator->destroy(enumerator);
	list->destroy(list);

	if (first)
	{
		DBG1(DBG_ASN, "no directoryName found");
		return FALSE;
	}
	return TRUE;
}

/**
 * ASN.1 definition of roleSyntax
 */
static const asn1Object_t roleSyntaxObjects[] =
{
	{ 0, "roleSyntax",		ASN1_SEQUENCE,		ASN1_NONE }, /* 0 */
	{ 1,   "roleAuthority",	ASN1_CONTEXT_C_0,	ASN1_OPT |
												ASN1_OBJ  }, /* 1 */
	{ 1,   "end opt",		ASN1_EOC,			ASN1_END  }, /* 2 */
	{ 1,   "roleName",		ASN1_CONTEXT_C_1,	ASN1_OBJ  }, /* 3 */
	{ 0, "exit",			ASN1_EOC,			ASN1_EXIT }
};

/**
 * Parses roleSyntax
 */
static void parse_roleSyntax(chunk_t blob, int level0)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;

	parser = asn1_parser_create(roleSyntaxObjects, blob);
	parser->set_top_level(parser, level0);

	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			default:
				break;
		}
	}
	parser->destroy(parser);
}

/**
 * ASN.1 definition of ietfAttrSyntax
 */
static const asn1Object_t ietfAttrSyntaxObjects[] =
{
	{ 0, "ietfAttrSyntax",		ASN1_SEQUENCE,		ASN1_NONE }, /*  0 */
	{ 1,   "policyAuthority",	ASN1_CONTEXT_C_0,	ASN1_OPT |
													ASN1_BODY }, /*  1 */
	{ 1,   "end opt",			ASN1_EOC,			ASN1_END  }, /*  2 */
	{ 1,   "values",			ASN1_SEQUENCE,		ASN1_LOOP }, /*  3 */
	{ 2,     "octets",			ASN1_OCTET_STRING,	ASN1_OPT |
													ASN1_BODY }, /*  4 */
	{ 2,     "end choice",		ASN1_EOC,			ASN1_END  }, /*  5 */
	{ 2,     "oid",				ASN1_OID,			ASN1_OPT |
													ASN1_BODY }, /*  6 */
	{ 2,     "end choice",		ASN1_EOC,			ASN1_END  }, /*  7 */
	{ 2,     "string",			ASN1_UTF8STRING,	ASN1_OPT |
													ASN1_BODY }, /*  8 */
	{ 2,     "end choice",		ASN1_EOC,			ASN1_END  }, /*  9 */
	{ 1,   "end loop",			ASN1_EOC,			ASN1_END  }, /* 10 */
	{ 0, "exit",				ASN1_EOC,			ASN1_EXIT }
};
#define IETF_ATTR_OCTETS	 4
#define IETF_ATTR_OID		 6
#define IETF_ATTR_STRING	 8

/**
 * Parse group memberships, IETF attributes
 */
static bool parse_groups(private_x509_ac_t *this, chunk_t encoded, int level0)
{
	ac_group_type_t type;
	group_t *group;
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	bool success;

	parser = asn1_parser_create(ietfAttrSyntaxObjects, encoded);
	parser->set_top_level(parser, level0);
	while (parser->iterate(parser, &objectID, &object))
	{
		switch (objectID)
		{
			case IETF_ATTR_OCTETS:
				type = AC_GROUP_TYPE_OCTETS;
				break;
			case IETF_ATTR_OID:
				type = AC_GROUP_TYPE_OID;
				break;
			case IETF_ATTR_STRING:
				type = AC_GROUP_TYPE_STRING;
				break;
			default:
				continue;
		}
		INIT(group,
			.type = type,
			.value = chunk_clone(object),
		);
		this->groups->insert_last(this->groups, group);
	}
	success = parser->success(parser);
	parser->destroy(parser);

	return success;
}

/**
 * ASN.1 definition of an X509 attribute certificate
 */
static const asn1Object_t acObjects[] =
{
	{ 0, "AttributeCertificate",			ASN1_SEQUENCE,		  ASN1_OBJ  }, /*  0 */
	{ 1,   "AttributeCertificateInfo",		ASN1_SEQUENCE,		  ASN1_OBJ  }, /*  1 */
	{ 2,       "version",					ASN1_INTEGER,		  ASN1_DEF |
																  ASN1_BODY }, /*  2 */
	{ 2,       "holder",					ASN1_SEQUENCE,		  ASN1_NONE }, /*  3 */
	{ 3,         "baseCertificateID",		ASN1_CONTEXT_C_0,	  ASN1_OPT  }, /*  4 */
	{ 4,           "issuer",				ASN1_SEQUENCE,		  ASN1_OBJ  }, /*  5 */
	{ 4,           "serial",				ASN1_INTEGER,		  ASN1_BODY }, /*  6 */
	{ 4,         "issuerUID",				ASN1_BIT_STRING,	  ASN1_OPT |
																  ASN1_BODY }, /*  7 */
	{ 4,         "end opt",					ASN1_EOC,			  ASN1_END  }, /*  8 */
	{ 3,       "end opt",					ASN1_EOC,			  ASN1_END  }, /*  9 */
	{ 3,       "entityName",				ASN1_CONTEXT_C_1,	  ASN1_OPT |
																  ASN1_OBJ  }, /* 10 */
	{ 3,       "end opt",					ASN1_EOC,			  ASN1_END  }, /* 11 */
	{ 3,         "objectDigestInfo",		ASN1_CONTEXT_C_2,	  ASN1_OPT  }, /* 12 */
	{ 4,           "digestedObjectType",	ASN1_ENUMERATED,	  ASN1_BODY }, /* 13 */
	{ 4,           "otherObjectTypeID",		ASN1_OID,			  ASN1_OPT |
																  ASN1_BODY }, /* 14 */
	{ 4,         "end opt",					ASN1_EOC,			  ASN1_END  }, /* 15 */
	{ 4,         "digestAlgorithm",			ASN1_EOC,			  ASN1_RAW  }, /* 16 */
	{ 3,       "end opt",					ASN1_EOC,			  ASN1_END  }, /* 17 */
	{ 2,       "v2Form",					ASN1_CONTEXT_C_0,	  ASN1_NONE }, /* 18 */
	{ 3,         "issuerName",				ASN1_SEQUENCE,		  ASN1_OPT |
																  ASN1_OBJ  }, /* 19 */
	{ 3,       "end opt",					ASN1_EOC,			  ASN1_END  }, /* 20 */
	{ 3,         "baseCertificateID",		ASN1_CONTEXT_C_0,	  ASN1_OPT  }, /* 21 */
	{ 4,           "issuerSerial",			ASN1_SEQUENCE,		  ASN1_NONE }, /* 22 */
	{ 5,             "issuer",				ASN1_SEQUENCE,		  ASN1_OBJ  }, /* 23 */
	{ 5,         "serial",					ASN1_INTEGER,		  ASN1_BODY }, /* 24 */
	{ 5,           "issuerUID",				ASN1_BIT_STRING,	  ASN1_OPT |
																  ASN1_BODY }, /* 25 */
	{ 5,           "end opt",				ASN1_EOC,			  ASN1_END  }, /* 26 */
	{ 3,       "end opt",					ASN1_EOC,			  ASN1_END  }, /* 27 */
	{ 3,       "objectDigestInfo",			ASN1_CONTEXT_C_1,	  ASN1_OPT  }, /* 28 */
	{ 4,           "digestInfo",			ASN1_SEQUENCE,		  ASN1_OBJ  }, /* 29 */
	{ 5,     "digestedObjectType",			ASN1_ENUMERATED,	  ASN1_BODY }, /* 30 */
	{ 5,         "otherObjectTypeID",		ASN1_OID,			  ASN1_OPT |
																  ASN1_BODY }, /* 31 */
	{ 5,           "end opt",				ASN1_EOC,			  ASN1_END  }, /* 32 */
	{ 5,           "digestAlgorithm",		ASN1_EOC,			  ASN1_RAW  }, /* 33 */
	{ 3,       "end opt",					ASN1_EOC,			  ASN1_END  }, /* 34 */
	{ 2,       "signature",					ASN1_EOC,			  ASN1_RAW  }, /* 35 */
	{ 2,       "serialNumber",				ASN1_INTEGER,		  ASN1_BODY }, /* 36 */
	{ 2,       "attrCertValidityPeriod",	ASN1_SEQUENCE,		  ASN1_NONE }, /* 37 */
	{ 3,         "notBeforeTime",			ASN1_GENERALIZEDTIME, ASN1_BODY }, /* 38 */
	{ 3,         "notAfterTime",			ASN1_GENERALIZEDTIME, ASN1_BODY }, /* 39 */
	{ 2,       "attributes",				ASN1_SEQUENCE,		  ASN1_LOOP }, /* 40 */
	{ 3,       "attribute",					ASN1_SEQUENCE,		  ASN1_NONE }, /* 41 */
	{ 4,         "type",					ASN1_OID,			  ASN1_BODY }, /* 42 */
	{ 4,         "values",					ASN1_SET, 			  ASN1_LOOP }, /* 43 */
	{ 5,           "value",					ASN1_EOC, 			  ASN1_RAW  }, /* 44 */
	{ 4,           "end loop",				ASN1_EOC,			  ASN1_END  }, /* 45 */
	{ 2,     "end loop",					ASN1_EOC,			  ASN1_END  }, /* 46 */
	{ 2,     "extensions",					ASN1_SEQUENCE,		  ASN1_LOOP }, /* 47 */
	{ 3,       "extension",					ASN1_SEQUENCE,		  ASN1_NONE }, /* 48 */
	{ 4,         "extnID",					ASN1_OID,			  ASN1_BODY }, /* 49 */
	{ 4,         "critical",				ASN1_BOOLEAN,		  ASN1_DEF |
																  ASN1_BODY }, /* 50 */
	{ 4,         "extnValue",				ASN1_OCTET_STRING,	  ASN1_BODY }, /* 51 */
	{ 2,     "end loop",					ASN1_EOC,			  ASN1_END  }, /* 52 */
	{ 1,   "signatureAlgorithm",			ASN1_EOC,			  ASN1_RAW  }, /* 53 */
	{ 1,   "signatureValue",				ASN1_BIT_STRING,	  ASN1_BODY }, /* 54 */
	{ 0, "exit",							ASN1_EOC,			  ASN1_EXIT }
};
#define AC_OBJ_CERTIFICATE_INFO		 1
#define AC_OBJ_VERSION				 2
#define AC_OBJ_HOLDER_ISSUER		 5
#define AC_OBJ_HOLDER_SERIAL		 6
#define AC_OBJ_ENTITY_NAME			10
#define AC_OBJ_ISSUER_NAME			19
#define AC_OBJ_ISSUER				23
#define AC_OBJ_SIG_ALG				35
#define AC_OBJ_SERIAL_NUMBER		36
#define AC_OBJ_NOT_BEFORE			38
#define AC_OBJ_NOT_AFTER			39
#define AC_OBJ_ATTRIBUTE_TYPE		42
#define AC_OBJ_ATTRIBUTE_VALUE		44
#define AC_OBJ_EXTN_ID				49
#define AC_OBJ_CRITICAL				50
#define AC_OBJ_EXTN_VALUE			51
#define AC_OBJ_ALGORITHM			53
#define AC_OBJ_SIGNATURE			54

/**
 * Parses an X.509 attribute certificate
 */
static bool parse_certificate(private_x509_ac_t *this)
{
	asn1_parser_t *parser;
	chunk_t object;
	int objectID;
	int type     = OID_UNKNOWN;
	int extn_oid = OID_UNKNOWN;
	signature_params_t sig_alg = {};
	bool success = FALSE;
	bool critical;

	parser = asn1_parser_create(acObjects, this->encoding);

	while (parser->iterate(parser, &objectID, &object))
	{
		u_int level = parser->get_level(parser)+1;

		switch (objectID)
		{
			case AC_OBJ_CERTIFICATE_INFO:
				this->certificateInfo = object;
				break;
			case AC_OBJ_VERSION:
				this->version = (object.len) ? (1 + (u_int)*object.ptr) : 1;
				DBG2(DBG_ASN, "  v%d", this->version);
				if (this->version != 2)
				{
					DBG1(DBG_ASN, "v%d attribute certificates are not "
						 "supported", this->version);
					goto end;
				}
				break;
			case AC_OBJ_HOLDER_ISSUER:
				if (!parse_directoryName(object, level, FALSE,
										 &this->holderIssuer))
				{
					goto end;
				}
				break;
			case AC_OBJ_HOLDER_SERIAL:
				this->holderSerial = identification_create_from_encoding(
															ID_KEY_ID, object);
				break;
			case AC_OBJ_ENTITY_NAME:
				if (!parse_directoryName(object, level, TRUE,
										 &this->entityName))
				{
					goto end;
				}
				break;
			case AC_OBJ_ISSUER_NAME:
				if (!parse_directoryName(object, level, FALSE,
										 &this->issuerName))
				{
					goto end;
				}
				break;
			case AC_OBJ_SIG_ALG:
				if (!signature_params_parse(object, level, &sig_alg))
				{
					DBG1(DBG_ASN, "  unable to parse signature algorithm");
					goto end;
				}
				break;
			case AC_OBJ_SERIAL_NUMBER:
				this->serialNumber = chunk_clone(object);
				break;
			case AC_OBJ_NOT_BEFORE:
				this->notBefore = asn1_to_time(&object, ASN1_GENERALIZEDTIME);
				break;
			case AC_OBJ_NOT_AFTER:
				this->notAfter = asn1_to_time(&object, ASN1_GENERALIZEDTIME);
				break;
			case AC_OBJ_ATTRIBUTE_TYPE:
				type = asn1_known_oid(object);
				break;
			case AC_OBJ_ATTRIBUTE_VALUE:
			{
				switch (type)
				{
					case OID_AUTHENTICATION_INFO:
						DBG2(DBG_ASN, "  need to parse authenticationInfo");
						break;
					case OID_ACCESS_IDENTITY:
						DBG2(DBG_ASN, "  need to parse accessIdentity");
						break;
					case OID_CHARGING_IDENTITY:
						DBG2(DBG_ASN, "  need to parse chargingIdentity");
						break;
					case OID_GROUP:
						DBG2(DBG_ASN, "-- > --");
						if (!parse_groups(this, object, level))
						{
							goto end;
						}
						DBG2(DBG_ASN, "-- < --");
						break;
					case OID_ROLE:
						parse_roleSyntax(object, level);
						break;
					default:
						break;
				}
				break;
			}
			case AC_OBJ_EXTN_ID:
				extn_oid = asn1_known_oid(object);
				break;
			case AC_OBJ_CRITICAL:
				critical = object.len && *object.ptr;
				DBG2(DBG_ASN, "  %s",(critical)?"TRUE":"FALSE");
				break;
			case AC_OBJ_EXTN_VALUE:
			{
				switch (extn_oid)
				{
					case OID_CRL_DISTRIBUTION_POINTS:
						DBG2(DBG_ASN, "  need to parse crlDistributionPoints");
						break;
					case OID_AUTHORITY_KEY_ID:
						this->authKeyIdentifier =
								x509_parse_authorityKeyIdentifier(object,
											level, &this->authKeySerialNumber);
						break;
					case OID_TARGET_INFORMATION:
						DBG2(DBG_ASN, "  need to parse targetInformation");
						break;
					case OID_NO_REV_AVAIL:
						this->noRevAvail = TRUE;
						break;
					default:
						break;
				}
				break;
			}
			case AC_OBJ_ALGORITHM:
				INIT(this->scheme);
				if (!signature_params_parse(object, level, this->scheme))
				{
					DBG1(DBG_ASN, "  unable to parse signature algorithm");
					goto end;
				}
				if (!signature_params_equal(this->scheme, &sig_alg))
				{
					DBG1(DBG_ASN, "  signature algorithms do not agree");
					goto end;
				}
				break;
			case AC_OBJ_SIGNATURE:
				this->signature = chunk_skip(object, 1);
				break;
			default:
				break;
		}
	}
	success = parser->success(parser);

end:
	parser->destroy(parser);
	signature_params_clear(&sig_alg);
	return success;
}

/**
 * build directoryName
 */
static chunk_t build_directoryName(asn1_t tag, chunk_t name)
{
	return asn1_wrap(tag, "m",
				asn1_simple_object(ASN1_CONTEXT_C_4, name));
}

/**
 * build holder
 */
static chunk_t build_holder(private_x509_ac_t *this)
{
	x509_t* x509 = (x509_t*)this->holderCert;
	identification_t *issuer, *subject;

	issuer = this->holderCert->get_issuer(this->holderCert);
	subject = this->holderCert->get_subject(this->holderCert);

	return asn1_wrap(ASN1_SEQUENCE, "mm",
		asn1_wrap(ASN1_CONTEXT_C_0, "mm",
			build_directoryName(ASN1_SEQUENCE, issuer->get_encoding(issuer)),
			asn1_simple_object(ASN1_INTEGER, x509->get_serial(x509))),
		build_directoryName(ASN1_CONTEXT_C_1, subject->get_encoding(subject)));
}

/**
 * build v2Form
 */
static chunk_t build_v2_form(private_x509_ac_t *this)
{
	identification_t *subject;

	subject = this->signerCert->get_subject(this->signerCert);
	return asn1_wrap(ASN1_CONTEXT_C_0, "m",
				build_directoryName(ASN1_SEQUENCE,
					subject->get_encoding(subject)));
}

/**
 * build attrCertValidityPeriod
 */
static chunk_t build_attr_cert_validity(private_x509_ac_t *this)
{
	return asn1_wrap(ASN1_SEQUENCE, "mm",
				asn1_from_time(&this->notBefore, ASN1_GENERALIZEDTIME),
				asn1_from_time(&this->notAfter, ASN1_GENERALIZEDTIME));
}

/**
 * build attribute type
 */
static chunk_t build_attribute_type(int type, chunk_t content)
{
	return asn1_wrap(ASN1_SEQUENCE, "mm",
				asn1_build_known_oid(type),
				asn1_wrap(ASN1_SET, "m", content));
}

/**
 * build attributes
 */
static chunk_t build_attributes(private_x509_ac_t *this)
{
	enumerator_t *enumerator;
	group_t *group;
	chunk_t values;
	size_t size = 0, len;
	u_char *pos;

	/* precalculate the total size of all values */
	enumerator = this->groups->create_enumerator(this->groups);
	while (enumerator->enumerate(enumerator, &group))
	{
		len = group->value.len;
		size += 1 + (len > 0) + (len >= 128) +
				(len >= 256) + (len >= 65536) + len;
	}
	enumerator->destroy(enumerator);

	pos = asn1_build_object(&values, ASN1_SEQUENCE, size);

	enumerator = this->groups->create_enumerator(this->groups);
	while (enumerator->enumerate(enumerator, &group))
	{
		chunk_t attr;
		asn1_t type;

		switch (group->type)
		{
			case AC_GROUP_TYPE_OCTETS:
				type = ASN1_OCTET_STRING;
				break;
			case AC_GROUP_TYPE_STRING:
				type = ASN1_UTF8STRING;
				break;
			case AC_GROUP_TYPE_OID:
				type = ASN1_OID;
				break;
			default:
				continue;
		}
		attr = asn1_simple_object(type, group->value);

		memcpy(pos, attr.ptr, attr.len);
		pos += attr.len;
		free(attr.ptr);
	}
	enumerator->destroy(enumerator);

	return asn1_wrap(ASN1_SEQUENCE, "m",
				build_attribute_type(OID_GROUP,
					asn1_wrap(ASN1_SEQUENCE, "m", values)));
}

/**
 * build authorityKeyIdentifier
 */
static chunk_t build_authorityKeyIdentifier(private_x509_ac_t *this)
{
	chunk_t keyIdentifier = chunk_empty;
	chunk_t authorityCertIssuer;
	chunk_t authorityCertSerialNumber;
	identification_t *issuer;
	public_key_t *public;
	x509_t *x509;

	x509 = (x509_t*)this->signerCert;
	issuer = this->signerCert->get_issuer(this->signerCert);
	public = this->signerCert->get_public_key(this->signerCert);
	if (public)
	{
		if (public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &keyIdentifier))
		{
			this->authKeyIdentifier = chunk_clone(keyIdentifier);
			keyIdentifier = asn1_simple_object(ASN1_CONTEXT_S_0, keyIdentifier);
		}
		public->destroy(public);
	}
	authorityCertIssuer = build_directoryName(ASN1_CONTEXT_C_1,
											issuer->get_encoding(issuer));
	authorityCertSerialNumber = asn1_simple_object(ASN1_CONTEXT_S_2,
											x509->get_serial(x509));
	return asn1_wrap(ASN1_SEQUENCE, "mm",
				asn1_build_known_oid(OID_AUTHORITY_KEY_ID),
				asn1_wrap(ASN1_OCTET_STRING, "m",
					asn1_wrap(ASN1_SEQUENCE, "mmm",
						keyIdentifier,
						authorityCertIssuer,
						authorityCertSerialNumber
					)
				)
		   );
}

/**
 * build extensions
 */
static chunk_t build_extensions(private_x509_ac_t *this)
{
	return asn1_wrap(ASN1_SEQUENCE, "mc",
				build_authorityKeyIdentifier(this),
				ASN1_noRevAvail_ext);
}

/**
 * build attributeCertificateInfo
 */
static chunk_t build_attr_cert_info(private_x509_ac_t *this, chunk_t sig_scheme)
{
	return asn1_wrap(ASN1_SEQUENCE, "cmmcmmmm",
				ASN1_INTEGER_1,
				build_holder(this),
				build_v2_form(this),
				sig_scheme,
				asn1_simple_object(ASN1_INTEGER, this->serialNumber),
				build_attr_cert_validity(this),
				build_attributes(this),
				build_extensions(this));
}

/**
 * build an X.509 attribute certificate
 */
static bool build_ac(private_x509_ac_t *this, hash_algorithm_t digest_alg)
{
	chunk_t signatureValue, attributeCertificateInfo, sig_scheme;
	private_key_t *key = this->signerKey;

	if (!this->scheme)
	{
		INIT(this->scheme,
			.scheme = signature_scheme_from_oid(
								hasher_signature_algorithm_to_oid(digest_alg,
												key->get_type(key))),
		);
	}
	if (this->scheme->scheme == SIGN_UNKNOWN)
	{
		return FALSE;
	}
	if (!signature_params_build(this->scheme, &sig_scheme))
	{
		return FALSE;
	}

	attributeCertificateInfo = build_attr_cert_info(this, sig_scheme);
	if (!key->sign(key, this->scheme->scheme, this->scheme->params,
				   attributeCertificateInfo, &signatureValue))
	{
		free(attributeCertificateInfo.ptr);
		free(sig_scheme.ptr);
		return FALSE;
	}
	this->encoding = asn1_wrap(ASN1_SEQUENCE, "mmm",
						attributeCertificateInfo,
						sig_scheme,
						asn1_bitstring("m", signatureValue));
	return TRUE;
}

METHOD(ac_t, get_serial, chunk_t,
	private_x509_ac_t *this)
{
	return this->serialNumber;
}

METHOD(ac_t, get_holderSerial, chunk_t,
	private_x509_ac_t *this)
{
	if (this->holderSerial)
	{
		return this->holderSerial->get_encoding(this->holderSerial);
	}
	return chunk_empty;
}

METHOD(ac_t, get_holderIssuer, identification_t*,
	private_x509_ac_t *this)
{
	return this->holderIssuer;
}

METHOD(ac_t, get_authKeyIdentifier, chunk_t,
	private_x509_ac_t *this)
{
	return this->authKeyIdentifier;
}

CALLBACK(attr_filter, bool,
	void *null, enumerator_t *orig, va_list args)
{
	group_t *group;
	ac_group_type_t *type;
	chunk_t *out;

	VA_ARGS_VGET(args, type, out);

	while (orig->enumerate(orig, &group))
	{
		if (group->type == AC_GROUP_TYPE_STRING &&
			!chunk_printable(group->value, NULL, 0))
		{	/* skip non-printable strings */
			continue;
		}
		*type = group->type;
		*out = group->value;
		return TRUE;
	}
	return FALSE;
}

METHOD(ac_t, create_group_enumerator, enumerator_t*,
	private_x509_ac_t *this)
{
	return enumerator_create_filter(
							this->groups->create_enumerator(this->groups),
							attr_filter, NULL, NULL);
}

METHOD(certificate_t, get_type, certificate_type_t,
	private_x509_ac_t *this)
{
	return CERT_X509_AC;
}

METHOD(certificate_t, get_subject, identification_t*,
	private_x509_ac_t *this)
{
	if (this->entityName)
	{
		return this->entityName;
	}
	return this->holderSerial;
}

METHOD(certificate_t, get_issuer, identification_t*,
	private_x509_ac_t *this)
{
	return this->issuerName;
}

METHOD(certificate_t, has_subject, id_match_t,
	private_x509_ac_t *this, identification_t *subject)
{
	id_match_t entity = ID_MATCH_NONE, serial = ID_MATCH_NONE;

	if (this->entityName)
	{
		entity = this->entityName->matches(this->entityName, subject);
	}
	if (this->holderSerial)
	{
		serial = this->holderSerial->matches(this->holderSerial, subject);
	}
	return max(entity, serial);
}

METHOD(certificate_t, has_issuer, id_match_t,
	private_x509_ac_t *this, identification_t *issuer)
{
	if (issuer->get_type(issuer) == ID_KEY_ID &&
		this->authKeyIdentifier.ptr &&
		chunk_equals(this->authKeyIdentifier, issuer->get_encoding(issuer)))
	{
		return ID_MATCH_PERFECT;
	}
	return this->issuerName->matches(this->issuerName, issuer);
}

METHOD(certificate_t, issued_by, bool,
	private_x509_ac_t *this, certificate_t *issuer,
	signature_params_t **scheme)
{
	public_key_t *key;
	bool valid;
	x509_t *x509 = (x509_t*)issuer;

	/* check if issuer is an X.509 AA certificate */
	if (issuer->get_type(issuer) != CERT_X509)
	{
		return FALSE;
	}
	if (!(x509->get_flags(x509) & X509_AA))
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
		if (!this->issuerName->equals(this->issuerName,
									  issuer->get_subject(issuer)))
		{
			return FALSE;
		}
	}

	if (!key)
	{
		return FALSE;
	}
	valid = key->verify(key, this->scheme->scheme, this->scheme->params,
						this->certificateInfo, this->signature);
	key->destroy(key);
	if (valid && scheme)
	{
		*scheme = signature_params_clone(this->scheme);
	}
	return valid;
}

METHOD(certificate_t, get_public_key, public_key_t*,
	private_x509_ac_t *this)
{
	return NULL;
}

METHOD(certificate_t, get_ref, certificate_t*,
	private_x509_ac_t *this)
{
	ref_get(&this->ref);
	return &this->public.interface.certificate;
}

METHOD(certificate_t, get_validity, bool,
	private_x509_ac_t *this, time_t *when, time_t *not_before, time_t *not_after)
{
	time_t t = when ? *when : time(NULL);

	if (not_before)
	{
		*not_before = this->notBefore;
	}
	if (not_after)
	{
		*not_after = this->notAfter;
	}
	return (t >= this->notBefore && t <= this->notAfter);
}

METHOD(certificate_t, get_encoding, bool,
	private_x509_ac_t *this, cred_encoding_type_t type, chunk_t *encoding)
{
	if (type == CERT_ASN1_DER)
	{
		*encoding = chunk_clone(this->encoding);
		return TRUE;
	}
	return lib->encoding->encode(lib->encoding, type, NULL, encoding,
					CRED_PART_X509_AC_ASN1_DER, this->encoding, CRED_PART_END);
}

METHOD(certificate_t, equals, bool,
	private_x509_ac_t *this, certificate_t *other)
{
	chunk_t encoding;
	bool equal;

	if ((certificate_t*)this == other)
	{
		return TRUE;
	}
	if (other->equals == _equals)
	{	/* skip allocation if we have the same implementation */
		return chunk_equals(this->encoding,
							((private_x509_ac_t*)other)->encoding);
	}
	if (!other->get_encoding(other, CERT_ASN1_DER, &encoding))
	{
		return FALSE;
	}
	equal = chunk_equals(this->encoding, encoding);
	free(encoding.ptr);
	return equal;
}

METHOD(certificate_t, destroy, void,
	private_x509_ac_t *this)
{
	if (ref_put(&this->ref))
	{
		DESTROY_IF(this->holderIssuer);
		DESTROY_IF(this->holderSerial);
		DESTROY_IF(this->entityName);
		DESTROY_IF(this->issuerName);
		DESTROY_IF(this->holderCert);
		DESTROY_IF(this->signerCert);
		DESTROY_IF(this->signerKey);
		this->groups->destroy_function(this->groups, (void*)group_destroy);
		signature_params_destroy(this->scheme);
		free(this->serialNumber.ptr);
		free(this->authKeyIdentifier.ptr);
		free(this->encoding.ptr);
		free(this);
	}
}

/**
 * create an empty but initialized X.509 attribute certificate
 */
static private_x509_ac_t *create_empty(void)
{
	private_x509_ac_t *this;

	INIT(this,
		.public = {
			.interface = {
				.certificate = {
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
				.get_serial = _get_serial,
				.get_holderSerial = _get_holderSerial,
				.get_holderIssuer = _get_holderIssuer,
				.get_authKeyIdentifier = _get_authKeyIdentifier,
				.create_group_enumerator = _create_group_enumerator,
			},
		},
		.groups = linked_list_create(),
		.ref = 1,
	);

	return this;
}

/**
 * See header.
 */
x509_ac_t *x509_ac_load(certificate_type_t type, va_list args)
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
		private_x509_ac_t *ac = create_empty();

		ac->encoding = chunk_clone(blob);
		if (parse_certificate(ac))
		{
			return &ac->public;
		}
		destroy(ac);
	}
	return NULL;
}

/**
 * Add groups from a list into AC group memberships
 */
static void add_groups_from_list(private_x509_ac_t *this, linked_list_t *list)
{
	enumerator_t *enumerator;
	group_t *group;
	char *name;

	enumerator = list->create_enumerator(list);
	while (enumerator->enumerate(enumerator, &name))
	{
		INIT(group,
			.type = AC_GROUP_TYPE_STRING,
			.value = chunk_clone(chunk_from_str(name)),
		);
		this->groups->insert_last(this->groups, group);
	}
	enumerator->destroy(enumerator);
}

/**
 * See header.
 */
x509_ac_t *x509_ac_gen(certificate_type_t type, va_list args)
{
	hash_algorithm_t digest_alg = HASH_SHA1;
	private_x509_ac_t *ac;

	ac = create_empty();
	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_NOT_BEFORE_TIME:
				ac->notBefore = va_arg(args, time_t);
				continue;
			case BUILD_NOT_AFTER_TIME:
				ac->notAfter = va_arg(args, time_t);
				continue;
			case BUILD_SERIAL:
				ac->serialNumber = chunk_clone(va_arg(args, chunk_t));
				continue;
			case BUILD_AC_GROUP_STRINGS:
				add_groups_from_list(ac, va_arg(args, linked_list_t*));
				continue;
			case BUILD_CERT:
				ac->holderCert = va_arg(args, certificate_t*);
				ac->holderCert->get_ref(ac->holderCert);
				continue;
			case BUILD_SIGNING_CERT:
				ac->signerCert = va_arg(args, certificate_t*);
				ac->signerCert->get_ref(ac->signerCert);
				continue;
			case BUILD_SIGNING_KEY:
				ac->signerKey = va_arg(args, private_key_t*);
				ac->signerKey->get_ref(ac->signerKey);
				continue;
			case BUILD_SIGNATURE_SCHEME:
				ac->scheme = va_arg(args, signature_params_t*);
				ac->scheme = signature_params_clone(ac->scheme);
				continue;
			case BUILD_DIGEST_ALG:
				digest_alg = va_arg(args, int);
				continue;
			case BUILD_END:
				break;
			default:
				destroy(ac);
				return NULL;
		}
		break;
	}

	if (ac->signerKey && ac->holderCert && ac->signerCert &&
		ac->holderCert->get_type(ac->holderCert) == CERT_X509 &&
		ac->signerCert->get_type(ac->signerCert) == CERT_X509)
	{
		if (build_ac(ac, digest_alg))
		{
			return &ac->public;
		}
	}
	destroy(ac);
	return NULL;
}
