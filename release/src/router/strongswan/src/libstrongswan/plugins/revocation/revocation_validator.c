/*
 * Copyright (C) 2015-2018 Tobias Brunner
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2009 Andreas Steffen
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

#include <time.h>

#include "revocation_validator.h"

#include <utils/debug.h>
#include <credentials/certificates/x509.h>
#include <credentials/certificates/crl.h>
#include <credentials/certificates/ocsp_request.h>
#include <credentials/certificates/ocsp_response.h>
#include <credentials/sets/ocsp_response_wrapper.h>
#include <selectors/traffic_selector.h>
#include <threading/spinlock.h>

/**
 * Default timeout in seconds when fetching OCSP/CRL.
 */
#define DEFAULT_TIMEOUT 10

typedef struct private_revocation_validator_t private_revocation_validator_t;

/**
 * Private data of an revocation_validator_t object.
 */
struct private_revocation_validator_t {

	/**
	 * Public revocation_validator_t interface.
	 */
	revocation_validator_t public;

	/**
	 * Enable OCSP validation
	 */
	bool enable_ocsp;

	/**
	 * Enable CRL validation
	 */
	bool enable_crl;

	/**
	 * Timeout in seconds when fetching
	 */
	u_int timeout;

	/**
	 * Lock to access flags
	 */
	spinlock_t *lock;
};

/**
 * Do an OCSP request
 */
static certificate_t *fetch_ocsp(char *url, certificate_t *subject,
								 certificate_t *issuer, u_int timeout)
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
	ocsp_request = (ocsp_request_t*)request;
	ocsp_response = (ocsp_response_t*)response;
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

/**
 * check the signature of an OCSP response
 */
static bool verify_ocsp(ocsp_response_t *response, certificate_t *ca)
{
	certificate_t *issuer, *subject;
	identification_t *responder;
	ocsp_response_wrapper_t *wrapper;
	enumerator_t *enumerator;
	x509_t *x509;
	bool verified = FALSE, found = FALSE;

	wrapper = ocsp_response_wrapper_create((ocsp_response_t*)response);
	lib->credmgr->add_local_set(lib->credmgr, &wrapper->set, FALSE);

	subject = &response->certificate;
	responder = subject->get_issuer(subject);

	/* check OCSP response using CA or directly delegated OCSP signer */
	enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr, CERT_X509,
													KEY_ANY, responder, FALSE);
	while (enumerator->enumerate(enumerator, &issuer))
	{
		x509 = (x509_t*)issuer;
		if (!issuer->get_validity(issuer, NULL, NULL, NULL))
		{	/* OCSP signer currently invalid */
			continue;
		}
		if (!ca->equals(ca, issuer))
		{	/* delegated OCSP signer? */
			if (!lib->credmgr->issued_by(lib->credmgr, issuer, ca, NULL))
			{	/* OCSP response not signed by CA, nor delegated OCSP signer */
				continue;
			}
			if (!(x509->get_flags(x509) & X509_OCSP_SIGNER))
			{	/* delegated OCSP signer does not have OCSP signer flag */
				continue;
			}
		}
		found = TRUE;
		if (lib->credmgr->issued_by(lib->credmgr, subject, issuer, NULL))
		{
			DBG1(DBG_CFG, "  ocsp response correctly signed by \"%Y\"",
				 issuer->get_subject(issuer));
			verified = TRUE;
			break;
		}
		DBG1(DBG_CFG, "ocsp response verification failed, "
			 "invalid signature");
	}
	enumerator->destroy(enumerator);

	if (!verified)
	{
		/* as fallback, use any locally installed OCSP signer certificate */
		enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
										CERT_X509, KEY_ANY, responder, TRUE);
		while (enumerator->enumerate(enumerator, &issuer))
		{
			x509 = (x509_t*)issuer;
			/* while issued_by() accepts both OCSP signer or CA basic
			 * constraint flags to verify OCSP responses, unrelated but trusted
			 * OCSP signers must explicitly have the OCSP signer flag set. */
			if ((x509->get_flags(x509) & X509_OCSP_SIGNER) &&
				issuer->get_validity(issuer, NULL, NULL, NULL))
			{
				found = TRUE;
				if (lib->credmgr->issued_by(lib->credmgr, subject, issuer, NULL))
				{
					DBG1(DBG_CFG, "  ocsp response correctly signed by \"%Y\"",
						 issuer->get_subject(issuer));
					verified = TRUE;
					break;
				}
				DBG1(DBG_CFG, "ocsp response verification failed, "
					 "invalid signature");
			}
		}
		enumerator->destroy(enumerator);
	}

	lib->credmgr->remove_local_set(lib->credmgr, &wrapper->set);
	wrapper->destroy(wrapper);

	if (!found)
	{
		DBG1(DBG_CFG, "ocsp response verification failed, "
			 "no signer certificate '%Y' found", responder);
	}
	return verified;
}

/**
 * Get the better of two OCSP responses, and check for usable OCSP info
 */
static certificate_t *get_better_ocsp(certificate_t *cand, certificate_t *best,
									  x509_t *subject, x509_t *issuer,
									  cert_validation_t *valid, bool cache)
{
	ocsp_response_t *response;
	time_t revocation, this_update, next_update, valid_until;
	crl_reason_t reason;
	bool revoked = FALSE;

	response = (ocsp_response_t*)cand;

	/* check ocsp signature */
	if (!verify_ocsp(response, &issuer->interface))
	{
		cand->destroy(cand);
		return best;
	}
	/* check if response contains our certificate */
	switch (response->get_status(response, subject, issuer, &revocation, &reason,
								 &this_update, &next_update))
	{
		case VALIDATION_REVOKED:
			/* subject has been revoked by a valid OCSP response */
			DBG1(DBG_CFG, "certificate was revoked on %T, reason: %N",
						  &revocation, TRUE, crl_reason_names, reason);
			revoked = TRUE;
			break;
		case VALIDATION_GOOD:
			/* results in either good or stale */
			break;
		default:
		case VALIDATION_FAILED:
			/* candidate unusable, does not contain our cert */
			DBG1(DBG_CFG, "  ocsp response contains no status on our certificate");
			cand->destroy(cand);
			return best;
	}

	/* select the better of the two responses */
	if (best == NULL || certificate_is_newer(cand, best))
	{
		DESTROY_IF(best);
		best = cand;
		if (best->get_validity(best, NULL, NULL, &valid_until))
		{
			DBG1(DBG_CFG, "  ocsp response is valid: until %T",
							 &valid_until, FALSE);
			*valid = VALIDATION_GOOD;
			if (cache)
			{	/* cache non-stale only, stale certs get refetched */
				lib->credmgr->cache_cert(lib->credmgr, best);
			}
		}
		else
		{
			DBG1(DBG_CFG, "  ocsp response is stale: since %T",
							 &valid_until, FALSE);
			*valid = VALIDATION_STALE;
		}
	}
	else
	{
		*valid = VALIDATION_STALE;
		cand->destroy(cand);
	}
	if (revoked)
	{	/* revoked always counts, even if stale */
		*valid = VALIDATION_REVOKED;
	}
	return best;
}

/**
 * validate a x509 certificate using OCSP
 */
static cert_validation_t check_ocsp(x509_t *subject, x509_t *issuer,
									auth_cfg_t *auth, u_int timeout)
{
	enumerator_t *enumerator;
	cert_validation_t valid = VALIDATION_SKIPPED;
	certificate_t *best = NULL, *current;
	identification_t *keyid = NULL;
	public_key_t *public;
	chunk_t chunk;
	char *uri = NULL;

	/** lookup cache for valid OCSP responses */
	enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
								CERT_X509_OCSP_RESPONSE, KEY_ANY, NULL, FALSE);
	while (enumerator->enumerate(enumerator, &current))
	{
		current->get_ref(current);
		best = get_better_ocsp(current, best, subject, issuer, &valid, FALSE);
		if (best && valid != VALIDATION_STALE)
		{
			DBG1(DBG_CFG, "  using cached ocsp response");
			break;
		}
	}
	enumerator->destroy(enumerator);

	/* derive the authorityKeyIdentifier from the issuer's public key */
	current = &issuer->interface;
	public = current->get_public_key(current);
	if (public && public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &chunk))
	{
		keyid = identification_create_from_encoding(ID_KEY_ID, chunk);
	}
	/** fetch from configured OCSP responder URLs */
	if (keyid && valid != VALIDATION_GOOD && valid != VALIDATION_REVOKED)
	{
		enumerator = lib->credmgr->create_cdp_enumerator(lib->credmgr,
											CERT_X509_OCSP_RESPONSE, keyid);
		while (enumerator->enumerate(enumerator, &uri))
		{
			current = fetch_ocsp(uri, &subject->interface, &issuer->interface,
								 timeout);
			if (current)
			{
				best = get_better_ocsp(current, best, subject, issuer,
									   &valid, TRUE);
				if (best && valid != VALIDATION_STALE)
				{
					break;
				}
			}
		}
		enumerator->destroy(enumerator);
	}
	DESTROY_IF(public);
	DESTROY_IF(keyid);

	/* fallback to URL fetching from subject certificate's URIs */
	if (valid != VALIDATION_GOOD && valid != VALIDATION_REVOKED)
	{
		enumerator = subject->create_ocsp_uri_enumerator(subject);
		while (enumerator->enumerate(enumerator, &uri))
		{
			current = fetch_ocsp(uri, &subject->interface, &issuer->interface,
								 timeout);
			if (current)
			{
				best = get_better_ocsp(current, best, subject, issuer,
									   &valid, TRUE);
				if (best && valid != VALIDATION_STALE)
				{
					break;
				}
			}
		}
		enumerator->destroy(enumerator);
	}
	/* an uri was found, but no result. switch validation state to failed */
	if (valid == VALIDATION_SKIPPED && uri)
	{
		valid = VALIDATION_FAILED;
	}
	auth->add(auth, AUTH_RULE_OCSP_VALIDATION, valid);
	if (valid == VALIDATION_GOOD)
	{	/* successful OCSP check fulfills also CRL constraint */
		auth->add(auth, AUTH_RULE_CRL_VALIDATION, VALIDATION_GOOD);
	}
	DESTROY_IF(best);
	return valid;
}

/**
 * fetch a CRL from an URL
 */
static certificate_t* fetch_crl(char *url, u_int timeout)
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
 * check the signature of an CRL
 */
static bool verify_crl(certificate_t *crl)
{
	certificate_t *issuer;
	enumerator_t *enumerator;
	bool verified = FALSE;

	enumerator = lib->credmgr->create_trusted_enumerator(lib->credmgr,
										KEY_ANY, crl->get_issuer(crl), FALSE);
	while (enumerator->enumerate(enumerator, &issuer, NULL))
	{
		if (lib->credmgr->issued_by(lib->credmgr, crl, issuer, NULL))
		{
			DBG1(DBG_CFG, "  crl correctly signed by \"%Y\"",
						   issuer->get_subject(issuer));
			verified = TRUE;
			break;
		}
	}
	enumerator->destroy(enumerator);

	return verified;
}

/**
 * Report the given CRL's validity and cache it if valid and requested
 */
static bool is_crl_valid(certificate_t *crl, time_t now, bool cache)
{
	time_t valid_until;

	if (crl->get_validity(crl, &now, NULL, &valid_until))
	{
		DBG1(DBG_CFG, "  crl is valid: until %T", &valid_until, FALSE);
		if (cache)
		{
			lib->credmgr->cache_cert(lib->credmgr, crl);
		}
		return TRUE;
	}
	DBG1(DBG_CFG, "  crl is stale: since %T", &valid_until, FALSE);
	return FALSE;
}

/**
 * Check if the CRL should be used yet
 */
static bool is_crl_not_valid_yet(certificate_t *crl, time_t now)
{
	time_t this_update;

	if (!crl->get_validity(crl, &now, &this_update, NULL))
	{
		if (this_update > now)
		{
			DBG1(DBG_CFG, "  crl is not valid: until %T", &this_update, FALSE);
			return TRUE;
		}
		/* we accept stale CRLs */
	}
	return FALSE;
}

/**
 * Get the better of two CRLs, and check for usable CRL info
 */
static certificate_t *get_better_crl(certificate_t *cand, certificate_t *best,
					x509_t *subject, cert_validation_t *valid,
					bool cache, crl_t *base)
{
	enumerator_t *enumerator;
	time_t now, revocation;
	crl_reason_t reason;
	chunk_t subject_serial, serial;
	crl_t *crl = (crl_t*)cand;

	if (base)
	{
		if (!crl->is_delta_crl(crl, &serial) ||
			!chunk_equals(serial, base->get_serial(base)))
		{
			cand->destroy(cand);
			return best;
		}
	}
	else
	{
		if (crl->is_delta_crl(crl, NULL))
		{
			cand->destroy(cand);
			return best;
		}
	}

	/* check CRL signature */
	if (!verify_crl(cand))
	{
		DBG1(DBG_CFG, "crl response verification failed");
		cand->destroy(cand);
		return best;
	}
	now = time(NULL);
	if (is_crl_not_valid_yet(cand, now))
	{
		cand->destroy(cand);
		return best;
	}

	subject_serial = chunk_skip_zero(subject->get_serial(subject));
	enumerator = crl->create_enumerator(crl);
	while (enumerator->enumerate(enumerator, &serial, &revocation, &reason))
	{
		if (chunk_equals(subject_serial, chunk_skip_zero(serial)))
		{
			if (reason != CRL_REASON_CERTIFICATE_HOLD)
			{
				*valid = VALIDATION_REVOKED;
			}
			else
			{
				/* if the cert is on hold, a newer CRL might not contain it */
				*valid = VALIDATION_ON_HOLD;
			}
			is_crl_valid(cand, now, cache);
			DBG1(DBG_CFG, "certificate was revoked on %T, reason: %N",
				 &revocation, TRUE, crl_reason_names, reason);
			enumerator->destroy(enumerator);
			DESTROY_IF(best);
			return cand;
		}
	}
	enumerator->destroy(enumerator);

	/* select the better of the two CRLs */
	if (best == NULL || crl_is_newer(crl, (crl_t*)best))
	{
		DESTROY_IF(best);
		best = cand;
		if (is_crl_valid(best, now, cache))
		{
			*valid = VALIDATION_GOOD;
		}
		else
		{
			*valid = VALIDATION_STALE;
		}
	}
	else
	{
		*valid = VALIDATION_STALE;
		cand->destroy(cand);
	}
	return best;
}

/**
 * Find or fetch a certificate for a given crlIssuer
 */
static cert_validation_t find_crl(x509_t *subject, identification_t *issuer,
								  crl_t *base, certificate_t **best,
								  bool *uri_found, u_int timeout)
{
	cert_validation_t valid = VALIDATION_SKIPPED;
	enumerator_t *enumerator;
	certificate_t *current;
	char *uri;

	/* find a cached (delta) crl */
	enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
										CERT_X509_CRL, KEY_ANY, issuer, FALSE);
	while (enumerator->enumerate(enumerator, &current))
	{
		current->get_ref(current);
		*best = get_better_crl(current, *best, subject, &valid, FALSE, base);
		if (*best && valid != VALIDATION_STALE)
		{
			DBG1(DBG_CFG, "  using cached crl");
			break;
		}
	}
	enumerator->destroy(enumerator);

	/* fallback to fetching crls from credential sets cdps */
	if (!base && valid != VALIDATION_GOOD && valid != VALIDATION_REVOKED)
	{
		enumerator = lib->credmgr->create_cdp_enumerator(lib->credmgr,
														 CERT_X509_CRL, issuer);
		while (enumerator->enumerate(enumerator, &uri))
		{
			*uri_found = TRUE;
			current = fetch_crl(uri, timeout);
			if (current)
			{
				if (!current->has_issuer(current, issuer))
				{
					DBG1(DBG_CFG, "issuer of fetched CRL '%Y' does not match CRL "
						 "issuer '%Y'", current->get_issuer(current), issuer);
					current->destroy(current);
					continue;
				}
				*best = get_better_crl(current, *best, subject,
									   &valid, TRUE, base);
				if (*best && valid != VALIDATION_STALE)
				{
					break;
				}
			}
		}
		enumerator->destroy(enumerator);
	}
	return valid;
}

/**
 * Check if the issuer of the given CRL matches
 */
static bool check_issuer(certificate_t *crl, x509_t *issuer, x509_cdp_t *cdp)
{
	certificate_t *cissuer = (certificate_t*)issuer;
	identification_t *id;
	chunk_t chunk;
	bool matches = FALSE;

	if (cdp->issuer)
	{
		return crl->has_issuer(crl, cdp->issuer);
	}
	/* check SKI/AKI first, but fall back to DN matching */
	chunk = issuer->get_subjectKeyIdentifier(issuer);
	if (chunk.len)
	{
		id = identification_create_from_encoding(ID_KEY_ID, chunk);
		matches = crl->has_issuer(crl, id);
		id->destroy(id);
	}
	return matches || crl->has_issuer(crl, cissuer->get_subject(cissuer));
}

/**
 * Look for a delta CRL for a given base CRL
 */
static cert_validation_t check_delta_crl(x509_t *subject, x509_t *issuer,
									crl_t *base, cert_validation_t base_valid,
									u_int timeout)
{
	cert_validation_t valid = VALIDATION_SKIPPED;
	certificate_t *best = NULL, *current, *cissuer = (certificate_t*)issuer;
	enumerator_t *enumerator;
	identification_t *id;
	x509_cdp_t *cdp;
	chunk_t chunk;
	bool uri;

	/* find cached delta CRL via subjectKeyIdentifier */
	chunk = issuer->get_subjectKeyIdentifier(issuer);
	if (chunk.len)
	{
		id = identification_create_from_encoding(ID_KEY_ID, chunk);
		valid = find_crl(subject, id, base, &best, &uri, timeout);
		id->destroy(id);
	}

	/* find delta CRL by CRLIssuer */
	enumerator = subject->create_crl_uri_enumerator(subject);
	while (valid != VALIDATION_GOOD && valid != VALIDATION_REVOKED &&
		   enumerator->enumerate(enumerator, &cdp))
	{
		if (cdp->issuer)
		{
			valid = find_crl(subject, cdp->issuer, base, &best, &uri, timeout);
		}
	}
	enumerator->destroy(enumerator);

	/* fetch from URIs found in Freshest CRL extension */
	enumerator = base->create_delta_crl_uri_enumerator(base);
	while (valid != VALIDATION_GOOD && valid != VALIDATION_REVOKED &&
		   enumerator->enumerate(enumerator, &cdp))
	{
		current = fetch_crl(cdp->uri, timeout);
		if (current)
		{
			if (!check_issuer(current, issuer, cdp))
			{
				DBG1(DBG_CFG, "issuer of fetched delta CRL '%Y' does not match "
					 "certificate's %sissuer '%Y'",
					 current->get_issuer(current), cdp->issuer ? "CRL " : "",
					 cdp->issuer ?: cissuer->get_subject(cissuer));
				current->destroy(current);
				continue;
			}
			best = get_better_crl(current, best, subject, &valid, TRUE, base);
			if (best && valid != VALIDATION_STALE)
			{
				break;
			}
		}
	}
	enumerator->destroy(enumerator);

	if (best)
	{
		best->destroy(best);
		return valid;
	}
	return base_valid;
}

/**
 * validate a x509 certificate using CRL
 */
static cert_validation_t check_crl(x509_t *subject, x509_t *issuer,
								   auth_cfg_t *auth, u_int timeout)
{
	cert_validation_t valid = VALIDATION_SKIPPED;
	certificate_t *best = NULL, *cissuer = (certificate_t*)issuer;
	identification_t *id;
	x509_cdp_t *cdp;
	bool uri_found = FALSE;
	certificate_t *current;
	enumerator_t *enumerator;
	chunk_t chunk;

	/* use issuers subjectKeyIdentifier to find a cached CRL / fetch from CDP */
	chunk = issuer->get_subjectKeyIdentifier(issuer);
	if (chunk.len)
	{
		id = identification_create_from_encoding(ID_KEY_ID, chunk);
		valid = find_crl(subject, id, NULL, &best, &uri_found, timeout);
		id->destroy(id);
	}

	/* find a cached CRL or fetch via configured CDP via CRLIssuer */
	enumerator = subject->create_crl_uri_enumerator(subject);
	while (valid != VALIDATION_GOOD && valid != VALIDATION_REVOKED &&
		   enumerator->enumerate(enumerator, &cdp))
	{
		if (cdp->issuer)
		{
			valid = find_crl(subject, cdp->issuer, NULL, &best, &uri_found,
							 timeout);
		}
	}
	enumerator->destroy(enumerator);

	/* fallback to fetching CRLs from CDPs found in subjects certificate */
	if (valid != VALIDATION_GOOD && valid != VALIDATION_REVOKED)
	{
		enumerator = subject->create_crl_uri_enumerator(subject);
		while (enumerator->enumerate(enumerator, &cdp))
		{
			uri_found = TRUE;
			current = fetch_crl(cdp->uri, timeout);
			if (current)
			{
				if (!check_issuer(current, issuer, cdp))
				{
					DBG1(DBG_CFG, "issuer of fetched CRL '%Y' does not match "
						 "certificate's %sissuer '%Y'",
						 current->get_issuer(current), cdp->issuer ? "CRL " : "",
						 cdp->issuer ?: cissuer->get_subject(cissuer));
					current->destroy(current);
					continue;
				}
				best = get_better_crl(current, best, subject, &valid,
									  TRUE, NULL);
				if (best && valid != VALIDATION_STALE)
				{
					break;
				}
			}
		}
		enumerator->destroy(enumerator);
	}

	/* look for delta CRLs */
	if (best && (valid == VALIDATION_GOOD || valid == VALIDATION_STALE))
	{
		valid = check_delta_crl(subject, issuer, (crl_t*)best, valid, timeout);
	}

	/* an uri was found, but no result. switch validation state to failed */
	if (valid == VALIDATION_SKIPPED && uri_found)
	{
		valid = VALIDATION_FAILED;
	}
	if (valid == VALIDATION_SKIPPED)
	{	/* if we skipped CRL validation, we use the result of OCSP for
		 * constraint checking */
		auth->add(auth, AUTH_RULE_CRL_VALIDATION,
				  auth->get(auth, AUTH_RULE_OCSP_VALIDATION));
	}
	else
	{
		auth->add(auth, AUTH_RULE_CRL_VALIDATION, valid);
	}
	DESTROY_IF(best);
	return valid;
}

METHOD(cert_validator_t, validate_online, bool,
	private_revocation_validator_t *this, certificate_t *subject,
	certificate_t *issuer, u_int pathlen, bool anchor, auth_cfg_t *auth)
{
	bool enable_ocsp, enable_crl;
	u_int timeout;

	this->lock->lock(this->lock);
	enable_ocsp = this->enable_ocsp;
	enable_crl = this->enable_crl;
	timeout = this->timeout;
	this->lock->unlock(this->lock);

	if ((enable_ocsp || enable_crl) &&
		subject->get_type(subject) == CERT_X509 &&
		issuer->get_type(issuer) == CERT_X509)
	{
		DBG1(DBG_CFG, "checking certificate status of \"%Y\"",
					   subject->get_subject(subject));

		if (enable_ocsp)
		{
			switch (check_ocsp((x509_t*)subject, (x509_t*)issuer, auth, timeout))
			{
				case VALIDATION_GOOD:
					DBG1(DBG_CFG, "certificate status is good");
					return TRUE;
				case VALIDATION_REVOKED:
				case VALIDATION_ON_HOLD:
					/* has already been logged */
					lib->credmgr->call_hook(lib->credmgr, CRED_HOOK_REVOKED,
											subject);
					return FALSE;
				case VALIDATION_SKIPPED:
					DBG2(DBG_CFG, "ocsp check skipped, no ocsp found");
					break;
				case VALIDATION_STALE:
					DBG1(DBG_CFG, "ocsp information stale, fallback to crl");
					break;
				case VALIDATION_FAILED:
					DBG1(DBG_CFG, "ocsp check failed, fallback to crl");
					break;
			}
		}
		else
		{
			auth->add(auth, AUTH_RULE_OCSP_VALIDATION, VALIDATION_SKIPPED);
		}

		if (enable_crl)
		{
			switch (check_crl((x509_t*)subject, (x509_t*)issuer, auth, timeout))
			{
				case VALIDATION_GOOD:
					DBG1(DBG_CFG, "certificate status is good");
					return TRUE;
				case VALIDATION_REVOKED:
				case VALIDATION_ON_HOLD:
					/* has already been logged */
					lib->credmgr->call_hook(lib->credmgr, CRED_HOOK_REVOKED,
											subject);
					return FALSE;
				case VALIDATION_FAILED:
				case VALIDATION_SKIPPED:
					DBG1(DBG_CFG, "certificate status is not available");
					break;
				case VALIDATION_STALE:
					DBG1(DBG_CFG, "certificate status is unknown, crl is stale");
					break;
			}
		}
		else
		{
			auth->add(auth, AUTH_RULE_CRL_VALIDATION,
					  auth->get(auth, AUTH_RULE_OCSP_VALIDATION));
		}

		lib->credmgr->call_hook(lib->credmgr, CRED_HOOK_VALIDATION_FAILED,
								subject);
	}
	return TRUE;
}

METHOD(revocation_validator_t, reload, void,
	private_revocation_validator_t *this)
{
	bool enable_ocsp, enable_crl;
	u_int timeout;

	enable_ocsp = lib->settings->get_bool(lib->settings,
						"%s.plugins.revocation.enable_ocsp", TRUE, lib->ns);
	enable_crl = lib->settings->get_bool(lib->settings,
						"%s.plugins.revocation.enable_crl", TRUE, lib->ns);
	timeout = lib->settings->get_time(lib->settings,
						"%s.plugins.revocation.timeout", DEFAULT_TIMEOUT,
						lib->ns);

	this->lock->lock(this->lock);
	this->enable_ocsp = enable_ocsp;
	this->enable_crl = enable_crl;
	this->timeout = timeout;
	this->lock->unlock(this->lock);

	if (!enable_ocsp)
	{
		DBG1(DBG_LIB, "all OCSP validation disabled");
	}
	if (!enable_crl)
	{
		DBG1(DBG_LIB, "all CRL validation disabled");
	}
}

METHOD(revocation_validator_t, destroy, void,
	private_revocation_validator_t *this)
{
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * See header
 */
revocation_validator_t *revocation_validator_create()
{
	private_revocation_validator_t *this;

	INIT(this,
		.public = {
			.validator.validate_online = _validate_online,
			.reload = _reload,
			.destroy = _destroy,
		},
		.lock = spinlock_create(),
	);

	reload(this);

	return &this->public;
}
