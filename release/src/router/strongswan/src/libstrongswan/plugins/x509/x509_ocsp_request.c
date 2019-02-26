/*
 * Copyright (C) 2008-2009 Martin Willi
 * Copyright (C) 2007-2014 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
 * Copyright (C) 2003 Christoph Gysin, Simon Zwahlen
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
#include <utils/identification.h>
#include <collections/linked_list.h>
#include <utils/debug.h>
#include <credentials/certificates/x509.h>
#include <credentials/keys/private_key.h>

#define NONCE_LEN		16

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
	 * CA the candidates belong to
	 */
	x509_t *ca;

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
	 * list of certificates to check, x509_t
	 */
	linked_list_t *candidates;

	/**
	 * nonce used in request
	 */
	chunk_t nonce;

	/**
	 * encoded OCSP request
	 */
	chunk_t encoding;

	/**
	 * reference count
	 */
	refcount_t ref;
};

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
							 chunk_t issuerNameHash, chunk_t issuerKeyHash,
							 chunk_t serialNumber)
{
	return asn1_wrap(ASN1_SEQUENCE, "m",
				asn1_wrap(ASN1_SEQUENCE, "mmmm",
					asn1_algorithmIdentifier(OID_SHA1),
					asn1_simple_object(ASN1_OCTET_STRING, issuerNameHash),
					asn1_simple_object(ASN1_OCTET_STRING, issuerKeyHash),
					asn1_simple_object(ASN1_INTEGER, serialNumber)));
}

/**
 * build requestList
 */
static chunk_t build_requestList(private_x509_ocsp_request_t *this)
{
	chunk_t issuerNameHash, issuerKeyHash;
	identification_t *issuer;
	x509_t *x509;
	certificate_t *cert;
	chunk_t list = chunk_empty;
	public_key_t *public;

	cert = (certificate_t*)this->ca;
	public = cert->get_public_key(cert);
	if (public)
	{
		hasher_t *hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
		if (hasher)
		{
			if (public->get_fingerprint(public, KEYID_PUBKEY_SHA1,
										&issuerKeyHash))
			{
				enumerator_t *enumerator;

				issuer = cert->get_subject(cert);
				if (hasher->allocate_hash(hasher, issuer->get_encoding(issuer),
										  &issuerNameHash))
				{
					enumerator = this->candidates->create_enumerator(
															this->candidates);
					while (enumerator->enumerate(enumerator, &x509))
					{
						chunk_t request, serialNumber;

						serialNumber = x509->get_serial(x509);
						request = build_Request(this, issuerNameHash,
												issuerKeyHash, serialNumber);
						list = chunk_cat("mm", list, request);
					}
					enumerator->destroy(enumerator);
					chunk_free(&issuerNameHash);
				}
				hasher->destroy(hasher);
			}
		}
		else
		{
			DBG1(DBG_LIB, "creating OCSP request failed, SHA1 not supported");
		}
		public->destroy(public);
	}
	else
	{
		DBG1(DBG_LIB, "creating OCSP request failed, CA certificate has "
			 "no public key");
	}
	return asn1_wrap(ASN1_SEQUENCE, "m", list);
}

/**
 * build nonce extension
 */
static chunk_t build_nonce(private_x509_ocsp_request_t *this)
{
	rng_t *rng;

	rng = lib->crypto->create_rng(lib->crypto, RNG_WEAK);
	if (!rng || !rng->allocate_bytes(rng, NONCE_LEN, &this->nonce))
	{
		DBG1(DBG_LIB, "creating OCSP request nonce failed, no RNG found");
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
 * build tbsRequest
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
		case KEY_BLISS:
			oid = OID_BLISS_WITH_SHA2_512;
			scheme = SIGN_BLISS_WITH_SHA2_512;
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
 *
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


METHOD(certificate_t, get_type, certificate_type_t,
	private_x509_ocsp_request_t *this)
{
	return CERT_X509_OCSP_REQUEST;
}

METHOD(certificate_t, get_subject, identification_t*,
	private_x509_ocsp_request_t *this)
{
	certificate_t *ca = (certificate_t*)this->ca;

	if (this->requestor)
	{
		return this->requestor;
	}
	if (this->cert)
	{
		return this->cert->get_subject(this->cert);
	}
	return ca->get_subject(ca);
}

METHOD(certificate_t, get_issuer, identification_t*,
	private_x509_ocsp_request_t *this)
{
	certificate_t *ca = (certificate_t*)this->ca;

	return ca->get_subject(ca);
}

METHOD(certificate_t, has_subject, id_match_t,
	private_x509_ocsp_request_t *this, identification_t *subject)
{
	certificate_t *current;
	enumerator_t *enumerator;
	id_match_t match, best = ID_MATCH_NONE;

	enumerator = this->candidates->create_enumerator(this->candidates);
	while (enumerator->enumerate(enumerator, &current))
	{
		match = current->has_subject(current, subject);
		if (match > best)
		{
			best = match;
		}
	}
	enumerator->destroy(enumerator);
	return best;
}

METHOD(certificate_t, has_issuer, id_match_t,
	private_x509_ocsp_request_t *this,
							 identification_t *issuer)
{
	certificate_t *ca = (certificate_t*)this->ca;

	return ca->has_subject(ca, issuer);
}

METHOD(certificate_t, issued_by, bool,
	private_x509_ocsp_request_t *this, certificate_t *issuer,
	signature_params_t **scheme)
{
	DBG1(DBG_LIB, "OCSP request validation not implemented!");
	return FALSE;
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
	certificate_t *cert;

	if (this->cert)
	{
		cert = this->cert;
	}
	else
	{
		cert = (certificate_t*)this->ca;
	}
	return cert->get_validity(cert, when, not_before, not_after);
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
		DESTROY_IF((certificate_t*)this->ca);
		DESTROY_IF(this->requestor);
		DESTROY_IF(this->cert);
		DESTROY_IF(this->key);
		this->candidates->destroy_offset(this->candidates, offsetof(certificate_t, destroy));
		chunk_free(&this->nonce);
		chunk_free(&this->encoding);
		free(this);
	}
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
			},
		},
		.candidates = linked_list_create(),
		.ref = 1,
	);

	return this;
}

/**
 * See header.
 */
x509_ocsp_request_t *x509_ocsp_request_gen(certificate_type_t type, va_list args)
{
	private_x509_ocsp_request_t *req;
	certificate_t *cert;
	private_key_t *private;
	identification_t *subject;

	req = create_empty();
	while (TRUE)
	{
		switch (va_arg(args, builder_part_t))
		{
			case BUILD_CA_CERT:
				cert = va_arg(args, certificate_t*);
				if (cert->get_type(cert) == CERT_X509)
				{
					req->ca = (x509_t*)cert->get_ref(cert);
				}
				continue;
			case BUILD_CERT:
				cert = va_arg(args, certificate_t*);
				if (cert->get_type(cert) == CERT_X509)
				{
					req->candidates->insert_last(req->candidates,
												 cert->get_ref(cert));
				}
				continue;
			case BUILD_SIGNING_CERT:
				cert = va_arg(args, certificate_t*);
				req->cert = cert->get_ref(cert);
				continue;
			case BUILD_SIGNING_KEY:
				private = va_arg(args, private_key_t*);
				req->key = private->get_ref(private);
				continue;
			case BUILD_SUBJECT:
				subject = va_arg(args, identification_t*);
				req->requestor = subject->clone(subject);
				continue;
			case BUILD_END:
				break;
			default:
				destroy(req);
				return NULL;
		}
		break;
	}
	if (req->ca)
	{
		req->encoding = build_OCSPRequest(req);
		return &req->public;
	}
	destroy(req);
	return NULL;
}

