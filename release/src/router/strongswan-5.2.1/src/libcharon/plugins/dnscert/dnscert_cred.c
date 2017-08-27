/*
 * Copyright (C) 2013 Tobias Brunner
 * Copyright (C) 2012 Reto Guadagnini
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
/*
 * Copyright (C) 2013 Ruslan Marchenko
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
#include <string.h>

#include "dnscert_cred.h"
#include "dnscert.h"

typedef struct private_dnscert_cred_t private_dnscert_cred_t;

/**
 * Private data of an dnscert_cred_t object
 */
struct private_dnscert_cred_t {

	/**
	 * Public part
	 */
	dnscert_cred_t public;

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
	/** inner enumerator (enumerates CERT resource records) */
	enumerator_t *inner;
	/** response of the DNS resolver which contains the CERTs */
	resolver_response_t *response;
} cert_enumerator_t;

METHOD(enumerator_t, cert_enumerator_enumerate, bool,
	cert_enumerator_t *this, certificate_t **cert)
{
	dnscert_t *cur_crt;
	rr_t *cur_rr;
	chunk_t certificate;

	/* Get the next supported CERT using the inner enumerator. */
	while (this->inner->enumerate(this->inner, &cur_rr))
	{
		cur_crt = dnscert_create_frm_rr(cur_rr);

		if (!cur_crt)
		{
			DBG1(DBG_CFG, "  failed to parse CERT RR, skipping");
			continue;
		}

		if (cur_crt->get_cert_type(cur_crt) != DNSCERT_TYPE_PKIX &&
			cur_crt->get_cert_type(cur_crt) != DNSCERT_TYPE_PGP)
		{
			DBG1(DBG_CFG, "  unsupported CERT type [%d], skipping",
				 cur_crt->get_cert_type(cur_crt));
			cur_crt->destroy(cur_crt);
			continue;
		}
		/* Try to parse PEM certificate container. Both x509 and PGP should
		 * presumably come as PEM encoded certs. */
		certificate = cur_crt->get_certificate(cur_crt);
		*cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_ANY,
								   BUILD_BLOB_PEM, certificate,
								   BUILD_END);
		if (*cert == NULL)
		{
			DBG1(DBG_CFG, "  unable to parse certificate, skipping",
				 cur_crt->get_cert_type(cur_crt));
			cur_crt->destroy(cur_crt);
			continue;
		}
		cur_crt->destroy(cur_crt);
		return TRUE;
	}
	return FALSE;
}

METHOD(enumerator_t, cert_enumerator_destroy, void,
	cert_enumerator_t *this)
{
	this->inner->destroy(this->inner);
	this->response->destroy(this->response);
	free(this);
}

METHOD(credential_set_t, create_cert_enumerator, enumerator_t*,
	private_dnscert_cred_t *this, certificate_type_t cert, key_type_t key,
	identification_t *id, bool trusted)
{
	resolver_response_t *response;
	cert_enumerator_t *e;
	char *fqdn;

	if (!id || id->get_type(id) != ID_FQDN)
	{
		return enumerator_create_empty();
	}

	/* query the DNS for the required CERT RRs */
	if (asprintf(&fqdn, "%Y", id) <= 0)
	{
		DBG1(DBG_CFG, "failed to determine FQDN to retrieve CERT RRs");
		return enumerator_create_empty();
	}

	DBG1(DBG_CFG, "performing a DNS query for CERT RRs of '%s'", fqdn);
	response = this->res->query(this->res, fqdn, RR_CLASS_IN, RR_TYPE_CERT);
	if (!response)
	{
		DBG1(DBG_CFG, "  query for CERT RRs failed");
		free(fqdn);
		return enumerator_create_empty();
	}
	free(fqdn);

	if (!response->has_data(response) ||
		!response->query_name_exist(response))
	{
		DBG1(DBG_CFG, "  unable to retrieve CERT RRs from the DNS");
		response->destroy(response);
		return enumerator_create_empty();
	}

	if (response->get_security_state(response) != SECURE)
	{
		DBG1(DBG_CFG, "  DNSSEC state of CERT RRs is not secure");
		response->destroy(response);
		return enumerator_create_empty();
	}

	INIT(e,
		.public = {
			.enumerate = (void*)_cert_enumerator_enumerate,
			.destroy = _cert_enumerator_destroy,
		},
		.inner = response->get_rr_set(response)->create_rr_enumerator(
									  response->get_rr_set(response)),
		.response = response
	);
	return &e->public;
}

METHOD(dnscert_cred_t, destroy, void,
	private_dnscert_cred_t *this)
{
	this->res->destroy(this->res);
	free(this);
}

/**
 * Described in header.
 */
dnscert_cred_t *dnscert_cred_create(resolver_t *res)
{
	private_dnscert_cred_t *this;

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
