/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2012 Reto Guadagnini
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

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>

#include "ipseckey_cred.h"
#include "ipseckey.h"

#include <bio/bio_reader.h>

typedef struct private_ipseckey_cred_t private_ipseckey_cred_t;

/**
 * Private data of an ipseckey_cred_t object
 */
struct private_ipseckey_cred_t {

	/**
	 * Public part
	 */
	ipseckey_cred_t public;

	/**
	 * DNS resolver
	 */
	resolver_t *res;
};

/**
 * enumerator over certificates
 */
typedef struct {
	/** implements enumerator interface */
	enumerator_t public;
	/** inner enumerator (enumerates IPSECKEY resource records) */
	enumerator_t *inner;
	/** response of the DNS resolver which contains the IPSECKEYs */
	resolver_response_t *response;
	/* IPSECKEYs are not valid before this point in time */
	time_t notBefore;
	/* IPSECKEYs are not valid after this point in time */
	time_t notAfter;
	/* identity to which the IPSECKEY belongs */
	identification_t *identity;
	/** most recently enumerated certificate */
	certificate_t *cert;
} cert_enumerator_t;

METHOD(enumerator_t, cert_enumerator_enumerate, bool,
	cert_enumerator_t *this, va_list args)
{
	certificate_t **cert;
	ipseckey_t *cur_ipseckey;
	public_key_t *public;
	rr_t *cur_rr;
	chunk_t key;

	VA_ARGS_VGET(args, cert);

	/* Get the next supported IPSECKEY using the inner enumerator. */
	while (this->inner->enumerate(this->inner, &cur_rr))
	{
		cur_ipseckey = ipseckey_create_frm_rr(cur_rr);

		if (!cur_ipseckey)
		{
			DBG1(DBG_CFG, "  failed to parse IPSECKEY, skipping");
			continue;
		}

		if (cur_ipseckey->get_algorithm(cur_ipseckey) != IPSECKEY_ALGORITHM_RSA)
		{
			DBG1(DBG_CFG, "  unsupported IPSECKEY algorithm, skipping");
			cur_ipseckey->destroy(cur_ipseckey);
			continue;
		}

		/* wrap the key of the IPSECKEY in a certificate and return this
		 * certificate */
		key = cur_ipseckey->get_public_key(cur_ipseckey);
		public = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_RSA,
									BUILD_BLOB_DNSKEY, key,
									BUILD_END);
		cur_ipseckey->destroy(cur_ipseckey);
		if (!public)
		{
			DBG1(DBG_CFG, "  failed to create public key from IPSECKEY");
			continue;
		}
		DESTROY_IF(this->cert);
		this->cert = lib->creds->create(lib->creds, CRED_CERTIFICATE,
										CERT_TRUSTED_PUBKEY,
										BUILD_PUBLIC_KEY, public,
										BUILD_SUBJECT, this->identity,
										BUILD_NOT_BEFORE_TIME, this->notBefore,
										BUILD_NOT_AFTER_TIME, this->notAfter,
										BUILD_END);
		public->destroy(public);
		if (!this->cert)
		{
			DBG1(DBG_CFG, "  failed to create certificate from IPSECKEY");
			continue;
		}
		*cert = this->cert;
		return TRUE;
	}
	return FALSE;
}

METHOD(enumerator_t, cert_enumerator_destroy, void,
	cert_enumerator_t *this)
{
	DESTROY_IF(this->cert);
	this->inner->destroy(this->inner);
	this->response->destroy(this->response);
	free(this);
}

METHOD(credential_set_t, create_cert_enumerator, enumerator_t*,
	private_ipseckey_cred_t *this, certificate_type_t cert, key_type_t key,
	identification_t *id, bool trusted)
{
	resolver_response_t *response;
	enumerator_t *rrsig_enum;
	cert_enumerator_t *e;
	rr_set_t *rrset;
	rr_t *rrsig;
	bio_reader_t *reader;
	uint32_t nBefore, nAfter;
	chunk_t ignore;
	char *fqdn;

	if (!id || id->get_type(id) != ID_FQDN)
	{
		return enumerator_create_empty();
	}

	/* query the DNS for the required IPSECKEY RRs */
	if (asprintf(&fqdn, "%Y", id) <= 0)
	{
		DBG1(DBG_CFG, "failed to determine FQDN to retrieve IPSECKEY RRs");
		return enumerator_create_empty();
	}
	DBG1(DBG_CFG, "performing a DNS query for IPSECKEY RRs of '%s'", fqdn);
	response = this->res->query(this->res, fqdn, RR_CLASS_IN, RR_TYPE_IPSECKEY);
	if (!response)
	{
		DBG1(DBG_CFG, "  query for IPSECKEY RRs failed");
		free(fqdn);
		return enumerator_create_empty();
	}
	free(fqdn);

	if (!response->has_data(response) ||
		!response->query_name_exist(response))
	{
		DBG1(DBG_CFG, "  unable to retrieve IPSECKEY RRs from the DNS");
		response->destroy(response);
		return enumerator_create_empty();
	}

	if (response->get_security_state(response) != SECURE)
	{
		DBG1(DBG_CFG, "  DNSSEC state of IPSECKEY RRs is not secure");
		response->destroy(response);
		return enumerator_create_empty();
	}

	/* determine the validity period of the retrieved IPSECKEYs
	 *
	 * we use the "Signature Inception" and "Signature Expiration" field
	 * of the first RRSIG RR to determine the validity period of the
	 * IPSECKEY RRs.
	 * TODO: Take multiple RRSIGs into account. */
	rrset = response->get_rr_set(response);
	rrsig_enum = rrset->create_rrsig_enumerator(rrset);
	if (!rrsig_enum || !rrsig_enum->enumerate(rrsig_enum, &rrsig))
	{
		DBG1(DBG_CFG, "  unable to determine the validity period of "
			 "IPSECKEY RRs because no RRSIGs are present");
		DESTROY_IF(rrsig_enum);
		response->destroy(response);
		return enumerator_create_empty();
	}
	rrsig_enum->destroy(rrsig_enum);

	/* parse the RRSIG for its validity period (RFC 4034) */
	reader = bio_reader_create(rrsig->get_rdata(rrsig));
	if (!reader->read_data(reader, 8, &ignore) ||
		!reader->read_uint32(reader, &nAfter) ||
		!reader->read_uint32(reader, &nBefore))
	{
		DBG1(DBG_CFG, "  unable to determine the validity period of RRSIG RRs");
		reader->destroy(reader);
		response->destroy(response);
		return enumerator_create_empty();
	}
	reader->destroy(reader);

	INIT(e,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _cert_enumerator_enumerate,
			.destroy = _cert_enumerator_destroy,
		},
		.inner = rrset->create_rr_enumerator(rrset),
		.response = response,
		.notBefore = nBefore,
		.notAfter = nAfter,
		.identity = id,
	);
	return &e->public;
}

METHOD(ipseckey_cred_t, destroy, void,
	private_ipseckey_cred_t *this)
{
	this->res->destroy(this->res);
	free(this);
}

/**
 * Described in header.
 */
ipseckey_cred_t *ipseckey_cred_create(resolver_t *res)
{
	private_ipseckey_cred_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_private_enumerator = (void*)return_null,
				.create_cert_enumerator = _create_cert_enumerator,
				.create_shared_enumerator = (void*)return_null,
				.create_cdp_enumerator = (void*)return_null,
				.cache_cert = (void*)nop,
			},
			.destroy = _destroy,
		},
		.res = res,
	);

	return &this->public;
}
