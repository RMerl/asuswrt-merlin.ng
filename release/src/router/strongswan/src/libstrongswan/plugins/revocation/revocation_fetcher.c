/*
 * Copyright (C) 2025 Martin Willi
 * Copyright (C) 2015-2018 Tobias Brunner
 * Copyright (C) 2009-2022 Andreas Steffen
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

#include "revocation_fetcher.h"

#include <utils/debug.h>
#include <threading/mutex.h>
#include <threading/condvar.h>
#include <collections/hashtable.h>
#include <credentials/certificates/crl.h>
#include <credentials/certificates/ocsp_request.h>
#include <credentials/certificates/ocsp_response.h>

/* number of fetch timeouts to degrade a CRL fetch after a failure */
#define CRL_DEGRADATION_TIMES 3

typedef struct private_revocation_fetcher_t private_revocation_fetcher_t;

/**
 * Private data of an revocation_fetcher_t object.
 */
struct private_revocation_fetcher_t {

	/**
	 * Public revocation_fetcher_t interface.
	 */
	revocation_fetcher_t public;

	/**
	 * Mutex to synchronize CRL fetches
	 */
	mutex_t *mutex;

	/**
	 * Active/completed/failed CRL fetches, crl_fetch_t.
	 */
	hashtable_t *crls;
};

typedef struct crl_fetch_t crl_fetch_t;

/**
 * Represents an active/completed/failed CRL fetch.
 */
struct crl_fetch_t {

	/**
	 * URL of the CRL.
	 */
	char *url;

	/**
	 * Condition variable to signal completion of the fetch.
	 */
	condvar_t *condvar;

	/**
	 * Number of threads fetching this CRL.
	 */
	u_int fetchers;

	/**
	 * Has the previous fetch failed, until when is this URL degraded?
	 */
	time_t failing;

	/**
	 * CRL received in the currently active fetch.
	 */
	certificate_t *crl;
};

/**
 * Perform the actual CRL fetch from the given URL.
 */
static certificate_t *do_crl_fetch(private_revocation_fetcher_t *this,
								   char *url, u_int timeout)
{
	certificate_t *crl;
	chunk_t chunk = chunk_empty;

	DBG1(DBG_CFG, "  fetching crl from '%s' ...", url);
	if (lib->fetcher->fetch(lib->fetcher, url, &chunk,
							FETCH_TIMEOUT, timeout,
							FETCH_END) != SUCCESS)
	{
		DBG1(DBG_CFG, "crl fetching failed");
		chunk_free(&chunk);
		return NULL;
	}
	crl = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509_CRL,
							 BUILD_BLOB_PEM, chunk, BUILD_END);
	chunk_free(&chunk);
	if (!crl)
	{
		DBG1(DBG_CFG, "crl fetched successfully but parsing failed");
		return NULL;
	}
	return crl;
}

/**
 * Start a new CRL fetch and signal completion to waiting threads.
 */
static certificate_t *start_crl_fetch(private_revocation_fetcher_t *this,
									  crl_fetch_t *fetch, u_int timeout)
{
	certificate_t *crl;

	fetch->fetchers++;
	this->mutex->unlock(this->mutex);
	crl = do_crl_fetch(this, fetch->url, timeout);
	this->mutex->lock(this->mutex);
	fetch->crl = crl;
	if (crl)
	{
		fetch->failing = 0;
	}
	else
	{
		fetch->failing = time_monotonic(NULL) + timeout * CRL_DEGRADATION_TIMES;
	}
	while (fetch->fetchers > 1)
	{
		fetch->condvar->signal(fetch->condvar);
		fetch->condvar->wait(fetch->condvar, this->mutex);
	}
	fetch->fetchers--;
	fetch->crl = NULL;
	return crl;
}

/**
 * Wait for a CRL fetch performed by another thread to complete.
 */
static certificate_t *wait_for_crl(private_revocation_fetcher_t *this,
								   crl_fetch_t *fetch)
{
	certificate_t *crl = NULL;

	if (fetch->failing && fetch->failing > time_monotonic(NULL))
	{
		DBG1(DBG_CFG, "  crl fetch from '%s' recently failed, skipping",
			 fetch->url);
		return NULL;
	}
	DBG1(DBG_CFG, "  waiting for crl fetch from '%s' ...", fetch->url);
	if (fetch->crl)
	{
		/* fetch is already complete, no need to wait */
		return fetch->crl->get_ref(fetch->crl);
	}
	fetch->fetchers++;
	fetch->condvar->wait(fetch->condvar, this->mutex);
	fetch->fetchers--;
	if (fetch->crl)
	{
		crl = fetch->crl->get_ref(fetch->crl);
	}
	fetch->condvar->signal(fetch->condvar);
	return crl;
}

METHOD(revocation_fetcher_t, fetch_crl, certificate_t*,
	private_revocation_fetcher_t *this, char *url, u_int timeout)
{
	certificate_t *crl;
	crl_fetch_t *fetch;

	this->mutex->lock(this->mutex);
	fetch = this->crls->get(this->crls, url);
	if (!fetch)
	{
		INIT(fetch,
			.url = strdup(url),
			.condvar = condvar_create(CONDVAR_TYPE_DEFAULT),
		);
		this->crls->put(this->crls, fetch->url, fetch);
	}
	if (fetch->fetchers)
	{
		crl = wait_for_crl(this, fetch);
	}
	else
	{
		crl = start_crl_fetch(this, fetch, timeout);
	}
	this->mutex->unlock(this->mutex);
	return crl;
}

METHOD(revocation_fetcher_t, fetch_ocsp, certificate_t*,
	private_revocation_fetcher_t *this, char *url,
    certificate_t *subject, certificate_t *issuer, u_int timeout)
{
	certificate_t *request, *response;
	ocsp_request_t *ocsp_request;
	ocsp_response_t *ocsp_response;
	chunk_t send, receive = chunk_empty;

	/* TODO: requestor name, signature */
	request = lib->creds->create(lib->creds,
						CRED_CERTIFICATE, CERT_X509_OCSP_REQUEST,
						BUILD_CA_CERT, issuer,
						BUILD_CERT, subject, BUILD_END);
	if (!request)
	{
		DBG1(DBG_CFG, "generating ocsp request failed");
		return NULL;
	}

	if (!request->get_encoding(request, CERT_ASN1_DER, &send))
	{
		DBG1(DBG_CFG, "encoding ocsp request failed");
		request->destroy(request);
		return NULL;
	}

	DBG1(DBG_CFG, "  requesting ocsp status from '%s' ...", url);
	if (lib->fetcher->fetch(lib->fetcher, url, &receive,
							FETCH_REQUEST_DATA, send,
							FETCH_REQUEST_TYPE, "application/ocsp-request",
							FETCH_TIMEOUT, timeout,
							FETCH_END) != SUCCESS)
	{
		DBG1(DBG_CFG, "ocsp request to %s failed", url);
		request->destroy(request);
		chunk_free(&receive);
		chunk_free(&send);
		return NULL;
	}
	chunk_free(&send);

	response = lib->creds->create(lib->creds,
								  CRED_CERTIFICATE, CERT_X509_OCSP_RESPONSE,
								  BUILD_BLOB_ASN1_DER, receive, BUILD_END);
	chunk_free(&receive);
	if (!response)
	{
		DBG1(DBG_CFG, "parsing ocsp response failed");
		request->destroy(request);
		return NULL;
	}
	ocsp_response = (ocsp_response_t*)response;
	if (ocsp_response->get_ocsp_status(ocsp_response) != OCSP_SUCCESSFUL)
	{
		response->destroy(response);
		request->destroy(request);
		return NULL;
	}
	ocsp_request = (ocsp_request_t*)request;
	if (ocsp_response->get_nonce(ocsp_response).len &&
		!chunk_equals_const(ocsp_request->get_nonce(ocsp_request),
							ocsp_response->get_nonce(ocsp_response)))
	{
		DBG1(DBG_CFG, "nonce in ocsp response doesn't match");
		request->destroy(request);
		return NULL;
	}
	request->destroy(request);
	return response;
}

CALLBACK(crl_fetch_destroy, void, crl_fetch_t *fetch, const void *key)
{
	fetch->condvar->destroy(fetch->condvar);
	free(fetch->url);
	free(fetch);
}

METHOD(revocation_fetcher_t, destroy, void,
	private_revocation_fetcher_t *this)
{
	this->crls->destroy_function(this->crls, crl_fetch_destroy);
	this->mutex->destroy(this->mutex);
	free(this);
}

/**
 * See header
 */
revocation_fetcher_t *revocation_fetcher_create()
{
	private_revocation_fetcher_t *this;

	INIT(this,
		.public = {
			.fetch_crl = _fetch_crl,
			.fetch_ocsp = _fetch_ocsp,
			.destroy = _destroy,
		},
		.mutex = mutex_create(MUTEX_TYPE_DEFAULT),
		.crls = hashtable_create(hashtable_hash_str, hashtable_equals_str, 8),
	);

	return &this->public;
}
