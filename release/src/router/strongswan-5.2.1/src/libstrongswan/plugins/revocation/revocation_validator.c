/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
 * Copyright (C) 2009 Andreas Steffen
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

#include "revocation_validator.h"

#include <utils/debug.h>
#include <credentials/certificates/x509.h>
#include <credentials/certificates/crl.h>
#include <credentials/certificates/ocsp_request.h>
#include <credentials/certificates/ocsp_response.h>
#include <credentials/sets/ocsp_response_wrapper.h>
#include <selectors/traffic_selector.h>

typedef struct private_revocation_validator_t private_revocation_validator_t;

/**
 * Private data of an revocation_validator_t object.
 */
struct private_revocation_validator_t {

	/**
	 * Public revocation_validator_t interface.
	 */
	revocation_validator_t public;
};

/**
 * Do an OCSP request
 */
static certificate_t *fetch_ocsp(char *url, certificate_t *subject,
								 certificate_t *issuer)
{
	certificate_t *request, *response;
	chunk_t send, receive;

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
	request->destroy(request);

	DBG1(DBG_CFG, "  requesting ocsp status from '%s' ...", url);
	if (lib->fetcher->fetch(lib->fetcher, url, &receive,
							FETCH_REQUEST_DATA, send,
							FETCH_REQUEST_TYPE, "application/ocsp-request",
							FETCH_END) != SUCCESS)
	{
		DBG1(DBG_CFG, "ocsp request to %s failed", url);
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
		return NULL;
	}
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
									auth_cfg_t *auth)
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
			current = fetch_ocsp(uri, &subject->interface, &issuer->interface);
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
			current = fetch_ocsp(uri, &subject->interface, &issuer->interface);
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
	if (auth)
	{
		auth->add(auth, AUTH_RULE_OCSP_VALIDATION, valid);
		if (valid == VALIDATION_GOOD)
		{	/* successful OCSP check fulfills also CRL constraint */
			auth->add(auth, AUTH_RULE_CRL_VALIDATION, VALIDATION_GOOD);
		}
	}
	DESTROY_IF(best);
	return valid;
}

/**
 * fetch a CRL from an URL
 */
static certificate_t* fetch_crl(char *url)
{
	certificate_t *crl;
	chunk_t chunk;

	DBG1(DBG_CFG, "  fetching crl from '%s' ...", url);
	if (lib->fetcher->fetch(lib->fetcher, url, &chunk, FETCH_END) != SUCCESS)
	{
		DBG1(DBG_CFG, "crl fetching failed");
		return NULL;
	}
	crl = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509_CRL,
							 BUILD_BLOB_ASN1_DER, chunk, BUILD_END);
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
 * Get the better of two CRLs, and check for usable CRL info
 */
static certificate_t *get_better_crl(certificate_t *cand, certificate_t *best,
					x509_t *subject, cert_validation_t *valid,
					bool cache, crl_t *base)
{
	enumerator_t *enumerator;
	time_t revocation, valid_until;
	crl_reason_t reason;
	chunk_t serial;
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

	enumerator = crl->create_enumerator(crl);
	while (enumerator->enumerate(enumerator, &serial, &revocation, &reason))
	{
		if (chunk_equals(serial, subject->get_serial(subject)))
		{
			DBG1(DBG_CFG, "certificate was revoked on %T, reason: %N",
				 &revocation, TRUE, crl_reason_names, reason);
			if (reason != CRL_REASON_CERTIFICATE_HOLD)
			{
				*valid = VALIDATION_REVOKED;
			}
			else
			{
				/* if the cert is on hold, a newer CRL might not contain it */
				*valid = VALIDATION_ON_HOLD;
			}
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
		if (best->get_validity(best, NULL, NULL, &valid_until))
		{
			DBG1(DBG_CFG, "  crl is valid: until %T", &valid_until, FALSE);
			*valid = VALIDATION_GOOD;
			if (cache)
			{	/* we cache non-stale crls only, as a stale crls are refetched */
				lib->credmgr->cache_cert(lib->credmgr, best);
			}
		}
		else
		{
			DBG1(DBG_CFG, "  crl is stale: since %T", &valid_until, FALSE);
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
								  bool *uri_found)
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
			current = fetch_crl(uri);
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
 * Look for a delta CRL for a given base CRL
 */
static cert_validation_t check_delta_crl(x509_t *subject, x509_t *issuer,
									crl_t *base, cert_validation_t base_valid)
{
	cert_validation_t valid = VALIDATION_SKIPPED;
	certificate_t *best = NULL, *current;
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
		valid = find_crl(subject, id, base, &best, &uri);
		id->destroy(id);
	}

	/* find delta CRL by CRLIssuer */
	enumerator = subject->create_crl_uri_enumerator(subject);
	while (valid != VALIDATION_GOOD && valid != VALIDATION_REVOKED &&
		   enumerator->enumerate(enumerator, &cdp))
	{
		if (cdp->issuer)
		{
			valid = find_crl(subject, cdp->issuer, base, &best, &uri);
		}
	}
	enumerator->destroy(enumerator);

	/* fetch from URIs found in Freshest CRL extension */
	enumerator = base->create_delta_crl_uri_enumerator(base);
	while (valid != VALIDATION_GOOD && valid != VALIDATION_REVOKED &&
		   enumerator->enumerate(enumerator, &cdp))
	{
		current = fetch_crl(cdp->uri);
		if (current)
		{
			if (cdp->issuer && !current->has_issuer(current, cdp->issuer))
			{
				DBG1(DBG_CFG, "issuer of fetched delta CRL '%Y' does not match "
					 "certificates CRL issuer '%Y'",
					 current->get_issuer(current), cdp->issuer);
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
								   auth_cfg_t *auth)
{
	cert_validation_t valid = VALIDATION_SKIPPED;
	certificate_t *best = NULL;
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
		valid = find_crl(subject, id, NULL, &best, &uri_found);
		id->destroy(id);
	}

	/* find a cached CRL or fetch via configured CDP via CRLIssuer */
	enumerator = subject->create_crl_uri_enumerator(subject);
	while (valid != VALIDATION_GOOD && valid != VALIDATION_REVOKED &&
		   enumerator->enumerate(enumerator, &cdp))
	{
		if (cdp->issuer)
		{
			valid = find_crl(subject, cdp->issuer, NULL, &best, &uri_found);
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
			current = fetch_crl(cdp->uri);
			if (current)
			{
				if (cdp->issuer && !current->has_issuer(current, cdp->issuer))
				{
					DBG1(DBG_CFG, "issuer of fetched CRL '%Y' does not match "
						 "certificates CRL issuer '%Y'",
						 current->get_issuer(current), cdp->issuer);
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
		valid = check_delta_crl(subject, issuer, (crl_t*)best, valid);
	}

	/* an uri was found, but no result. switch validation state to failed */
	if (valid == VALIDATION_SKIPPED && uri_found)
	{
		valid = VALIDATION_FAILED;
	}
	if (auth)
	{
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
	}
	DESTROY_IF(best);
	return valid;
}

METHOD(cert_validator_t, validate, bool,
	private_revocation_validator_t *this, certificate_t *subject,
	certificate_t *issuer, bool online, u_int pathlen, bool anchor,
	auth_cfg_t *auth)
{
	if (subject->get_type(subject) == CERT_X509 &&
		issuer->get_type(issuer) == CERT_X509 &&
		online)
	{
		DBG1(DBG_CFG, "checking certificate status of \"%Y\"",
					   subject->get_subject(subject));
		switch (check_ocsp((x509_t*)subject, (x509_t*)issuer,
						   pathlen ? NULL : auth))
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
		switch (check_crl((x509_t*)subject, (x509_t*)issuer,
						  pathlen ? NULL : auth))
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
		lib->credmgr->call_hook(lib->credmgr, CRED_HOOK_VALIDATION_FAILED,
								subject);
	}
	return TRUE;
}

METHOD(revocation_validator_t, destroy, void,
	private_revocation_validator_t *this)
{
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
			.validator.validate = _validate,
			.destroy = _destroy,
		},
	);

	return &this->public;
}
