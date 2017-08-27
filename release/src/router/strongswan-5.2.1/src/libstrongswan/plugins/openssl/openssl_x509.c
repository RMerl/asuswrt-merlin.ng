/*
 * Copyright (C) 2011 Tobias Brunner
 * Hochschule fuer Technik Rapperswil
 *
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

/*
 * Copyright (C) 2013 Michael Rossberg
 * Copyright (C) 2013 Technische Universität Ilmenau
 *
 * Copyright (C) 2010 secunet Security Networks AG
 * Copyright (C) 2010 Thomas Egerer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include "openssl_x509.h"
#include "openssl_util.h"

#include <utils/debug.h>
#include <asn1/oid.h>
#include <collections/linked_list.h>
#include <selectors/traffic_selector.h>

/* IP Addr block extension support was introduced with 0.9.8e */
#if OPENSSL_VERSION_NUMBER < 0x0090805fL
#define OPENSSL_NO_RFC3779
#endif

typedef struct private_openssl_x509_t private_openssl_x509_t;

/**
 * Private data of an openssl_x509_t object.
 */
struct private_openssl_x509_t {

	/**
	 * Public openssl_x509_t interface.
	 */
	openssl_x509_t public;

	/**
	 * OpenSSL certificate representation
	 */
	X509 *x509;

	/**
	 * DER encoded certificate
	 */
	chunk_t encoding;

	/**
	 * SHA1 hash of the certificate
	 */
	chunk_t hash;

	/**
	 * X509 flags
	 */
	x509_flag_t flags;

	/**
	 * Pathlen constraint
	 */
	u_char pathlen;

	/**
	 * certificate subject
	 */
	identification_t *subject;

	/**
	 * certificate issuer
	 */
	identification_t *issuer;

	/**
	 * Certificates public key
	 */
	public_key_t *pubkey;

	/**
	 * subjectKeyIdentifier as read from cert
	 */
	chunk_t subjectKeyIdentifier;

	/**
	 * authorityKeyIdentifier as read from cert
	 */
	chunk_t authKeyIdentifier;

	/**
	 * Start time of certificate validity
	 */
	time_t notBefore;

	/**
	 * End time of certificate validity
	 */
	time_t notAfter;

	/**
	 * Signature scheme of the certificate
	 */
	signature_scheme_t scheme;

	/**
	 * subjectAltNames
	 */
	linked_list_t *subjectAltNames;

	/**
	 * issuerAltNames
	 */
	linked_list_t *issuerAltNames;

	/**
	 * List of CRL URIs, as x509_cdp_t
	 */
	linked_list_t *crl_uris;

	/**
	 * List of OCSP URIs
	 */
	linked_list_t *ocsp_uris;

	/**
	 * List of ipAddrBlocks as traffic_selector_t
	 */
	linked_list_t *ipAddrBlocks;


	/**
	 * References to this cert
	 */
	refcount_t ref;
};

/**
 * Destroy a CRL URI struct
 */
static void crl_uri_destroy(x509_cdp_t *this)
{
	free(this->uri);
	DESTROY_IF(this->issuer);
	free(this);
}

/**
 * Convert a GeneralName to an identification_t.
 */
static identification_t *general_name2id(GENERAL_NAME *name)
{
	if (!name)
	{
		return NULL;
	}
	switch (name->type)
	{
		case GEN_EMAIL:
			return identification_create_from_encoding(ID_RFC822_ADDR,
					openssl_asn1_str2chunk(name->d.rfc822Name));
		case GEN_DNS:
			return identification_create_from_encoding(ID_FQDN,
					openssl_asn1_str2chunk(name->d.dNSName));
		case GEN_URI:
			return identification_create_from_encoding(ID_DER_ASN1_GN_URI,
					openssl_asn1_str2chunk(name->d.uniformResourceIdentifier));
		case GEN_IPADD:
		{
			chunk_t chunk = openssl_asn1_str2chunk(name->d.iPAddress);
			if (chunk.len == 4)
			{
				return identification_create_from_encoding(ID_IPV4_ADDR, chunk);
			}
			if (chunk.len == 16)
			{
				return identification_create_from_encoding(ID_IPV6_ADDR, chunk);
			}
			return NULL;
		}
		case GEN_DIRNAME :
			return openssl_x509_name2id(name->d.directoryName);
		case GEN_OTHERNAME:
			if (OBJ_obj2nid(name->d.otherName->type_id) == NID_ms_upn &&
				name->d.otherName->value->type == V_ASN1_UTF8STRING)
			{
				return identification_create_from_encoding(ID_RFC822_ADDR,
							openssl_asn1_str2chunk(
								name->d.otherName->value->value.utf8string));
			}
			return NULL;
		default:
			return NULL;
	}
}

METHOD(x509_t, get_flags, x509_flag_t,
	private_openssl_x509_t *this)
{
	return this->flags;
}

METHOD(x509_t, get_serial, chunk_t,
	private_openssl_x509_t *this)
{
	return openssl_asn1_str2chunk(X509_get_serialNumber(this->x509));
}

METHOD(x509_t, get_subjectKeyIdentifier, chunk_t,
	private_openssl_x509_t *this)
{
	chunk_t fingerprint;

	if (this->subjectKeyIdentifier.len)
	{
		return this->subjectKeyIdentifier;
	}
	if (this->pubkey->get_fingerprint(this->pubkey, KEYID_PUBKEY_SHA1,
									  &fingerprint))
	{
		return fingerprint;
	}
	return chunk_empty;
}

METHOD(x509_t, get_authKeyIdentifier, chunk_t,
	private_openssl_x509_t *this)
{
	if (this->authKeyIdentifier.len)
	{
		return this->authKeyIdentifier;
	}
	return chunk_empty;
}

METHOD(x509_t, get_constraint, u_int,
	private_openssl_x509_t *this, x509_constraint_t type)
{
	switch (type)
	{
		case X509_PATH_LEN:
			return this->pathlen;
		default:
			return X509_NO_CONSTRAINT;
	}
}

METHOD(x509_t, create_subjectAltName_enumerator, enumerator_t*,
	private_openssl_x509_t *this)
{
	return this->subjectAltNames->create_enumerator(this->subjectAltNames);
}

METHOD(x509_t, create_crl_uri_enumerator, enumerator_t*,
	private_openssl_x509_t *this)
{
	return this->crl_uris->create_enumerator(this->crl_uris);
}

METHOD(x509_t, create_ocsp_uri_enumerator, enumerator_t*,
	private_openssl_x509_t *this)
{
	return this->ocsp_uris->create_enumerator(this->ocsp_uris);
}

METHOD(x509_t, create_ipAddrBlock_enumerator, enumerator_t*,
	private_openssl_x509_t *this)
{
	return this->ipAddrBlocks->create_enumerator(this->ipAddrBlocks);
}

METHOD(certificate_t, get_type, certificate_type_t,
	private_openssl_x509_t *this)
{
	return CERT_X509;
}

METHOD(certificate_t, get_subject, identification_t*,
	private_openssl_x509_t *this)
{
	return this->subject;
}

METHOD(certificate_t, get_issuer, identification_t*,
	private_openssl_x509_t *this)
{
	return this->issuer;
}

METHOD(certificate_t, has_subject, id_match_t,
	private_openssl_x509_t *this, identification_t *subject)
{
	identification_t *current;
	enumerator_t *enumerator;
	id_match_t match, best;
	chunk_t encoding;

	if (subject->get_type(subject) == ID_KEY_ID)
	{
		encoding = subject->get_encoding(subject);

		if (chunk_equals(this->hash, encoding))
		{
			return ID_MATCH_PERFECT;
		}
		if (this->subjectKeyIdentifier.len &&
			chunk_equals(this->subjectKeyIdentifier, encoding))
		{
			return ID_MATCH_PERFECT;
		}
		if (this->pubkey &&
			this->pubkey->has_fingerprint(this->pubkey, encoding))
		{
			return ID_MATCH_PERFECT;
		}
		if (chunk_equals(get_serial(this), encoding))
		{
			return ID_MATCH_PERFECT;
		}
	}
	best = this->subject->matches(this->subject, subject);
	enumerator = create_subjectAltName_enumerator(this);
	while (enumerator->enumerate(enumerator, &current))
	{
		match = current->matches(current, subject);
		if (match > best)
		{
			best = match;
		}
	}
	enumerator->destroy(enumerator);
	return best;
}

METHOD(certificate_t, has_issuer, id_match_t,
	private_openssl_x509_t *this, identification_t *issuer)
{
	/* issuerAltNames currently not supported */
	return this->issuer->matches(this->issuer, issuer);
}

METHOD(certificate_t, issued_by, bool,
	private_openssl_x509_t *this, certificate_t *issuer,
	signature_scheme_t *scheme)
{
	public_key_t *key;
	bool valid;
	x509_t *x509 = (x509_t*)issuer;
	chunk_t tbs;

	if (&this->public.x509.interface == issuer)
	{
		if (this->flags & X509_SELF_SIGNED)
		{
			return TRUE;
		}
	}
	else
	{
		if (issuer->get_type(issuer) != CERT_X509)
		{
			return FALSE;
		}
		if (!(x509->get_flags(x509) & X509_CA))
		{
			return FALSE;
		}
		if (!this->issuer->equals(this->issuer, issuer->get_subject(issuer)))
		{
			return FALSE;
		}
	}
	if (this->scheme == SIGN_UNKNOWN)
	{
		return FALSE;
	}
	key = issuer->get_public_key(issuer);
	if (!key)
	{
		return FALSE;
	}
	tbs = openssl_i2chunk(X509_CINF, this->x509->cert_info);
	valid = key->verify(key, this->scheme, tbs,
						openssl_asn1_str2chunk(this->x509->signature));
	free(tbs.ptr);
	key->destroy(key);
	if (valid && scheme)
	{
		*scheme = this->scheme;
	}
	return valid;
}

METHOD(certificate_t, get_public_key, public_key_t*,
	private_openssl_x509_t *this)
{
	return this->pubkey->get_ref(this->pubkey);
}

METHOD(certificate_t, get_validity, bool,
	private_openssl_x509_t *this,
	time_t *when, time_t *not_before, time_t *not_after)
{
	time_t t;

	if (when)
	{
		t = *when;
	}
	else
	{
		t = time(NULL);
	}
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
	private_openssl_x509_t *this, cred_encoding_type_t type, chunk_t *encoding)
{
	if (type == CERT_ASN1_DER)
	{
		*encoding = chunk_clone(this->encoding);
		return TRUE;
	}
	return lib->encoding->encode(lib->encoding, type, NULL, encoding,
						CRED_PART_X509_ASN1_DER, this->encoding, CRED_PART_END);
}


METHOD(certificate_t, equals, bool,
	private_openssl_x509_t *this, certificate_t *other)
{
	chunk_t encoding;
	bool equal;

	if (this == (private_openssl_x509_t*)other)
	{
		return TRUE;
	}
	if (other->get_type(other) != CERT_X509)
	{
		return FALSE;
	}
	if (other->equals == (void*)equals)
	{	/* skip allocation if we have the same implementation */
		encoding = ((private_openssl_x509_t*)other)->encoding;
		return chunk_equals(this->encoding, encoding);
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
	private_openssl_x509_t *this)
{
	ref_get(&this->ref);
	return &this->public.x509.interface;
}

METHOD(certificate_t, destroy, void,
	private_openssl_x509_t *this)
{
	if (ref_put(&this->ref))
	{
		if (this->x509)
		{
			X509_free(this->x509);
		}
		DESTROY_IF(this->subject);
		DESTROY_IF(this->issuer);
		DESTROY_IF(this->pubkey);
		free(this->subjectKeyIdentifier.ptr);
		free(this->authKeyIdentifier.ptr);
		free(this->encoding.ptr);
		free(this->hash.ptr);
		this->subjectAltNames->destroy_offset(this->subjectAltNames,
										offsetof(identification_t, destroy));
		this->issuerAltNames->destroy_offset(this->issuerAltNames,
										offsetof(identification_t, destroy));
		this->crl_uris->destroy_function(this->crl_uris, (void*)crl_uri_destroy);
		this->ocsp_uris->destroy_function(this->ocsp_uris, free);
		this->ipAddrBlocks->destroy_offset(this->ipAddrBlocks,
										offsetof(traffic_selector_t, destroy));
		free(this);
	}
}

/**
 * Create an empty certificate
 */
static private_openssl_x509_t *create_empty()
{
	private_openssl_x509_t *this;

	INIT(this,
		.public = {
			.x509 = {
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
				.get_flags = _get_flags,
				.get_serial = _get_serial,
				.get_subjectKeyIdentifier = _get_subjectKeyIdentifier,
				.get_authKeyIdentifier = _get_authKeyIdentifier,
				.get_constraint = _get_constraint,
				.create_subjectAltName_enumerator = _create_subjectAltName_enumerator,
				.create_crl_uri_enumerator = _create_crl_uri_enumerator,
				.create_ocsp_uri_enumerator = _create_ocsp_uri_enumerator,
				.create_ipAddrBlock_enumerator = _create_ipAddrBlock_enumerator,
				.create_name_constraint_enumerator = (void*)enumerator_create_empty,
				.create_cert_policy_enumerator = (void*)enumerator_create_empty,
				.create_policy_mapping_enumerator = (void*)enumerator_create_empty,
			},
		},
		.subjectAltNames = linked_list_create(),
		.issuerAltNames = linked_list_create(),
		.crl_uris = linked_list_create(),
		.ocsp_uris = linked_list_create(),
		.ipAddrBlocks = linked_list_create(),
		.pathlen = X509_NO_CONSTRAINT,
		.ref = 1,
	);

	return this;
}

/**
 * parse an extionsion containing GENERAL_NAMES into a list
 */
static bool parse_generalNames_ext(linked_list_t *list,
								   X509_EXTENSION *ext)
{
	GENERAL_NAMES *names;
	GENERAL_NAME *name;
	identification_t *id;
	int i, num;

	names = X509V3_EXT_d2i(ext);
	if (!names)
	{
		return FALSE;
	}

	num = sk_GENERAL_NAME_num(names);
	for (i = 0; i < num; i++)
	{
		name = sk_GENERAL_NAME_value(names, i);
		id = general_name2id(name);
		if (id)
		{
			list->insert_last(list, id);
		}
		GENERAL_NAME_free(name);
	}
	sk_GENERAL_NAME_free(names);
	return TRUE;
}

/**
 * parse basic constraints
 */
static bool parse_basicConstraints_ext(private_openssl_x509_t *this,
									   X509_EXTENSION *ext)
{
	BASIC_CONSTRAINTS *constraints;
	long pathlen;

	constraints = (BASIC_CONSTRAINTS*)X509V3_EXT_d2i(ext);
	if (constraints)
	{
		if (constraints->ca)
		{
			this->flags |= X509_CA;
		}
		if (constraints->pathlen)
		{

			pathlen = ASN1_INTEGER_get(constraints->pathlen);
			this->pathlen = (pathlen >= 0 && pathlen < 128) ?
							 pathlen : X509_NO_CONSTRAINT;
		}
		BASIC_CONSTRAINTS_free(constraints);
		return TRUE;
	}
	return FALSE;
}

/**
 * parse key usage
 */
static bool parse_keyUsage_ext(private_openssl_x509_t *this,
							   X509_EXTENSION *ext)
{
	ASN1_BIT_STRING *usage;

	usage = X509V3_EXT_d2i(ext);
	if (usage)
	{
		if (usage->length > 0)
		{
			int flags = usage->data[0];
			if (usage->length > 1)
			{
				flags |= usage->data[1] << 8;
			}
			switch (flags)
			{
				case X509v3_KU_CRL_SIGN:
					this->flags |= X509_CRL_SIGN;
					break;
				case X509v3_KU_KEY_CERT_SIGN:
					/* we use the caBasicContraint, MUST be set */
				default:
					break;
			}
		}
		ASN1_BIT_STRING_free(usage);
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse ExtendedKeyUsage
 */
static bool parse_extKeyUsage_ext(private_openssl_x509_t *this,
								  X509_EXTENSION *ext)
{
	EXTENDED_KEY_USAGE *usage;
	int i;

	usage = X509V3_EXT_d2i(ext);
	if (usage)
	{
		for (i = 0; i < sk_ASN1_OBJECT_num(usage); i++)
		{
			switch (OBJ_obj2nid(sk_ASN1_OBJECT_value(usage, i)))
			{
				case NID_server_auth:
					this->flags |= X509_SERVER_AUTH;
					break;
				case NID_client_auth:
					this->flags |= X509_CLIENT_AUTH;
					break;
				case NID_OCSP_sign:
					this->flags |= X509_OCSP_SIGNER;
					break;
				default:
					break;
			}
		}
		sk_ASN1_OBJECT_pop_free(usage, ASN1_OBJECT_free);
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse CRL distribution points
 */
static bool parse_crlDistributionPoints_ext(private_openssl_x509_t *this,
											X509_EXTENSION *ext)
{
	CRL_DIST_POINTS *cdps;
	DIST_POINT *cdp;
	identification_t *id, *issuer;
	x509_cdp_t *entry;
	char *uri;
	int i, j, k, point_num, name_num, issuer_num;

	cdps = X509V3_EXT_d2i(ext);
	if (!cdps)
	{
		return FALSE;
	}
	point_num = sk_DIST_POINT_num(cdps);
	for (i = 0; i < point_num; i++)
	{
		cdp = sk_DIST_POINT_value(cdps, i);
		if (cdp)
		{
			if (cdp->distpoint && cdp->distpoint->type == 0 &&
				cdp->distpoint->name.fullname)
			{
				name_num = sk_GENERAL_NAME_num(cdp->distpoint->name.fullname);
				for (j = 0; j < name_num; j++)
				{
					id = general_name2id(sk_GENERAL_NAME_value(
											cdp->distpoint->name.fullname, j));
					if (id)
					{
						if (asprintf(&uri, "%Y", id) > 0)
						{
							if (cdp->CRLissuer)
							{
								issuer_num = sk_GENERAL_NAME_num(cdp->CRLissuer);
								for (k = 0; k < issuer_num; k++)
								{
									issuer = general_name2id(
										sk_GENERAL_NAME_value(cdp->CRLissuer, k));
									if (issuer)
									{
										INIT(entry,
											.uri = strdup(uri),
											.issuer = issuer,
										);
										this->crl_uris->insert_last(
														this->crl_uris, entry);
									}
								}
								free(uri);
							}
							else
							{
								INIT(entry,
									.uri = uri,
								);
								this->crl_uris->insert_last(this->crl_uris, entry);
							}
						}
						id->destroy(id);
					}
				}
			}

			DIST_POINT_free(cdp);
		}
	}
	sk_DIST_POINT_free(cdps);
	return TRUE;
}

/**
 * Parse authorityInfoAccess with OCSP URIs
 */
static bool parse_authorityInfoAccess_ext(private_openssl_x509_t *this,
										  X509_EXTENSION *ext)
{
	AUTHORITY_INFO_ACCESS *infos;
	ACCESS_DESCRIPTION *desc;
	identification_t *id;
	int i, num;
	char *uri;

	infos = X509V3_EXT_d2i(ext);
	if (!infos)
	{
		return FALSE;
	}
	num = sk_ACCESS_DESCRIPTION_num(infos);
	for (i = 0; i < num; i++)
	{
		desc = sk_ACCESS_DESCRIPTION_value(infos, i);
		if (desc)
		{
			if (openssl_asn1_known_oid(desc->method) == OID_OCSP)
			{
				id = general_name2id(desc->location);
				if (id)
				{
					if (asprintf(&uri, "%Y", id) > 0)
					{
						this->ocsp_uris->insert_last(this->ocsp_uris, uri);
					}
					id->destroy(id);
				}
			}
			ACCESS_DESCRIPTION_free(desc);
		}
	}
	sk_ACCESS_DESCRIPTION_free(infos);
	return TRUE;
}

#ifndef OPENSSL_NO_RFC3779

/**
 * Parse a single block of ipAddrBlock extension
 */
static void parse_ipAddrBlock_ext_fam(private_openssl_x509_t *this,
									  IPAddressFamily *fam)
{
	const IPAddressOrRanges *list;
	IPAddressOrRange *aor;
	traffic_selector_t *ts;
	ts_type_t type;
	chunk_t from, to;
	int i, afi;

	if (fam->ipAddressChoice->type != IPAddressChoice_addressesOrRanges)
	{
		return;
	}

	afi = v3_addr_get_afi(fam);
	switch (afi)
	{
		case IANA_AFI_IPV4:
			from = chunk_alloca(4);
			to = chunk_alloca(4);
			type = TS_IPV4_ADDR_RANGE;
			break;
		case IANA_AFI_IPV6:
			from = chunk_alloca(16);
			to = chunk_alloca(16);
			type = TS_IPV6_ADDR_RANGE;
			break;
		default:
			return;
	}

	list = fam->ipAddressChoice->u.addressesOrRanges;
	for (i = 0; i < sk_IPAddressOrRange_num(list); i++)
	{
		aor = sk_IPAddressOrRange_value(list, i);
		if (v3_addr_get_range(aor, afi, from.ptr, to.ptr, from.len) > 0)
		{
			ts = traffic_selector_create_from_bytes(0, type, from, 0, to, 65535);
			if (ts)
			{
				this->ipAddrBlocks->insert_last(this->ipAddrBlocks, ts);
			}
		}
	}
}

/**
 * Parse ipAddrBlock extension
 */
static bool parse_ipAddrBlock_ext(private_openssl_x509_t *this,
								  X509_EXTENSION *ext)
{
	STACK_OF(IPAddressFamily) *blocks;
	IPAddressFamily *fam;

	blocks = (STACK_OF(IPAddressFamily)*)X509V3_EXT_d2i(ext);
	if (!blocks)
	{
		return FALSE;
	}

	if (!v3_addr_is_canonical(blocks))
	{
		sk_IPAddressFamily_free(blocks);
		return FALSE;
	}

	while (sk_IPAddressFamily_num(blocks) > 0)
	{
		fam = sk_IPAddressFamily_pop(blocks);
		parse_ipAddrBlock_ext_fam(this, fam);
		IPAddressFamily_free(fam);
	}
	sk_IPAddressFamily_free(blocks);

	this->flags |= X509_IP_ADDR_BLOCKS;
	return TRUE;
}
#endif /* !OPENSSL_NO_RFC3779 */

/**
 * Parse authorityKeyIdentifier extension
 */
static bool parse_authKeyIdentifier_ext(private_openssl_x509_t *this,
										X509_EXTENSION *ext)
{
	AUTHORITY_KEYID *keyid;

	keyid = (AUTHORITY_KEYID*)X509V3_EXT_d2i(ext);
	if (keyid)
	{
		free(this->authKeyIdentifier.ptr);
		this->authKeyIdentifier = chunk_clone(
										openssl_asn1_str2chunk(keyid->keyid));
		AUTHORITY_KEYID_free(keyid);
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse subjectKeyIdentifier extension
 */
static bool parse_subjectKeyIdentifier_ext(private_openssl_x509_t *this,
										   X509_EXTENSION *ext)
{
	chunk_t ostr;

	ostr = openssl_asn1_str2chunk(X509_EXTENSION_get_data(ext));
	/* quick and dirty unwrap of octet string */
	if (ostr.len > 2 &&
		ostr.ptr[0] == V_ASN1_OCTET_STRING && ostr.ptr[1] == ostr.len - 2)
	{
		free(this->subjectKeyIdentifier.ptr);
		this->subjectKeyIdentifier = chunk_clone(chunk_skip(ostr, 2));
		return TRUE;
	}
	return FALSE;
}

/**
 * Parse X509 extensions we are interested in
 */
static bool parse_extensions(private_openssl_x509_t *this)
{
	STACK_OF(X509_EXTENSION) *extensions;
	int i, num;

	extensions = this->x509->cert_info->extensions;
	if (extensions)
	{
		num = sk_X509_EXTENSION_num(extensions);

		for (i = 0; i < num; i++)
		{
			X509_EXTENSION *ext;
			bool ok;

			ext = sk_X509_EXTENSION_value(extensions, i);
			switch (OBJ_obj2nid(X509_EXTENSION_get_object(ext)))
			{
				case NID_info_access:
					ok = parse_authorityInfoAccess_ext(this, ext);
					break;
				case NID_authority_key_identifier:
					ok = parse_authKeyIdentifier_ext(this, ext);
					break;
				case NID_subject_key_identifier:
					ok = parse_subjectKeyIdentifier_ext(this, ext);
					break;
				case NID_subject_alt_name:
					ok = parse_generalNames_ext(this->subjectAltNames, ext);
					break;
				case NID_issuer_alt_name:
					ok = parse_generalNames_ext(this->issuerAltNames, ext);
					break;
				case NID_basic_constraints:
					ok = parse_basicConstraints_ext(this, ext);
					break;
				case NID_key_usage:
					ok = parse_keyUsage_ext(this, ext);
					break;
				case NID_ext_key_usage:
					ok = parse_extKeyUsage_ext(this, ext);
					break;
				case NID_crl_distribution_points:
					ok = parse_crlDistributionPoints_ext(this, ext);
					break;
#ifndef OPENSSL_NO_RFC3779
				case NID_sbgp_ipAddrBlock:
					ok = parse_ipAddrBlock_ext(this, ext);
					break;
#endif /* !OPENSSL_NO_RFC3779 */
				default:
					ok = X509_EXTENSION_get_critical(ext) == 0 ||
						 !lib->settings->get_bool(lib->settings,
									"%s.x509.enforce_critical", TRUE, lib->ns);
					if (!ok)
					{
						char buf[80] = "";

						OBJ_obj2txt(buf, sizeof(buf),
									X509_EXTENSION_get_object(ext), 0);
						DBG1(DBG_LIB, "found unsupported critical X.509 "
							 "extension: %s", buf);
					}
					break;
			}
			if (!ok)
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

/**
 * Parse a DER encoded x509 certificate
 */
static bool parse_certificate(private_openssl_x509_t *this)
{
	const unsigned char *ptr = this->encoding.ptr;
	hasher_t *hasher;
	chunk_t chunk;

	this->x509 = d2i_X509(NULL, &ptr, this->encoding.len);
	if (!this->x509)
	{
		return FALSE;
	}
	if (X509_get_version(this->x509) < 0 || X509_get_version(this->x509) > 2)
	{
		DBG1(DBG_LIB, "unsupported x509 version: %d",
			 X509_get_version(this->x509) + 1);
		return FALSE;
	}

	this->subject = openssl_x509_name2id(X509_get_subject_name(this->x509));
	this->issuer = openssl_x509_name2id(X509_get_issuer_name(this->x509));

	switch (openssl_asn1_known_oid(this->x509->cert_info->key->algor->algorithm))
	{
		case OID_RSA_ENCRYPTION:
			this->pubkey = lib->creds->create(lib->creds,
					CRED_PUBLIC_KEY, KEY_RSA, BUILD_BLOB_ASN1_DER,
					openssl_asn1_str2chunk(X509_get0_pubkey_bitstr(this->x509)),
					BUILD_END);
			break;
		case OID_EC_PUBLICKEY:
			/* for ECDSA, we need the full subjectPublicKeyInfo, as it contains
			 * the curve parameters. */
			chunk = openssl_i2chunk(X509_PUBKEY, X509_get_X509_PUBKEY(this->x509));
			this->pubkey = lib->creds->create(lib->creds,
					CRED_PUBLIC_KEY, KEY_ECDSA, BUILD_BLOB_ASN1_DER,
					chunk, BUILD_END);
			free(chunk.ptr);
			break;
		default:
			DBG1(DBG_LIB, "unsupported public key algorithm");
			break;
	}
	if (!this->subject || !this->issuer || !this->pubkey)
	{
		return FALSE;
	}

	this->notBefore = openssl_asn1_to_time(X509_get_notBefore(this->x509));
	this->notAfter = openssl_asn1_to_time(X509_get_notAfter(this->x509));

	if (!chunk_equals(
			openssl_asn1_obj2chunk(this->x509->cert_info->signature->algorithm),
			openssl_asn1_obj2chunk(this->x509->sig_alg->algorithm)))
	{
		return FALSE;
	}
	this->scheme = signature_scheme_from_oid(openssl_asn1_known_oid(
												this->x509->sig_alg->algorithm));

	if (!parse_extensions(this))
	{
		return FALSE;
	}

	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!hasher || !hasher->allocate_hash(hasher, this->encoding, &this->hash))
	{
		DESTROY_IF(hasher);
		return FALSE;
	}
	hasher->destroy(hasher);

	if (issued_by(this, &this->public.x509.interface, NULL))
	{
		this->flags |= X509_SELF_SIGNED;
	}
	return TRUE;
}

openssl_x509_t *openssl_x509_load(certificate_type_t type, va_list args)
{
	chunk_t blob = chunk_empty;
	x509_flag_t flags = 0;

	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_BLOB_ASN1_DER:
				blob = va_arg(args, chunk_t);
				continue;
			case BUILD_X509_FLAG:
				flags |= va_arg(args, x509_flag_t);
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
		private_openssl_x509_t *this;

		this = create_empty();
		this->encoding = chunk_clone(blob);
		this->flags |= flags;
		if (parse_certificate(this))
		{
			return &this->public;
		}
		DBG1(DBG_LIB, "OpenSSL X.509 parsing failed");
		destroy(this);
	}
	return NULL;
}
