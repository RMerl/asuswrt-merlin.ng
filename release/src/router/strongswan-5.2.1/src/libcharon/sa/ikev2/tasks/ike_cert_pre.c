/*
 * Copyright (C) 2008 Tobias Brunner
 * Copyright (C) 2006-2009 Martin Willi
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

#include "ike_cert_pre.h"

#include <daemon.h>
#include <sa/ike_sa.h>
#include <encoding/payloads/cert_payload.h>
#include <encoding/payloads/certreq_payload.h>
#include <credentials/certificates/x509.h>


typedef struct private_ike_cert_pre_t private_ike_cert_pre_t;

/**
 * Private members of a ike_cert_pre_t task.
 */
struct private_ike_cert_pre_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_cert_pre_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * Do we accept HTTP certificate lookup requests
	 */
	bool do_http_lookup;

	/**
	 * whether this is the final authentication round
	 */
	bool final;
};

/**
 * Process a single certificate request payload
 */
static void process_certreq(private_ike_cert_pre_t *this,
							certreq_payload_t *certreq, auth_cfg_t *auth)
{
	enumerator_t *enumerator;
	u_int unknown = 0;
	chunk_t keyid;

	this->ike_sa->set_condition(this->ike_sa, COND_CERTREQ_SEEN, TRUE);

	if (certreq->get_cert_type(certreq) != CERT_X509)
	{
		DBG1(DBG_IKE, "cert payload %N not supported - ignored",
			 certificate_type_names, certreq->get_cert_type(certreq));
		return;
	}

	enumerator = certreq->create_keyid_enumerator(certreq);
	while (enumerator->enumerate(enumerator, &keyid))
	{
		identification_t *id;
		certificate_t *cert;

		id = identification_create_from_encoding(ID_KEY_ID, keyid);
		cert = lib->credmgr->get_cert(lib->credmgr,
									  CERT_X509, KEY_ANY, id, TRUE);
		if (cert)
		{
			DBG1(DBG_IKE, "received cert request for \"%Y\"",
				 cert->get_subject(cert));
			auth->add(auth, AUTH_RULE_CA_CERT, cert);
		}
		else
		{
			DBG2(DBG_IKE, "received cert request for unknown ca with keyid %Y",
				 id);
			unknown++;
		}
		id->destroy(id);
	}
	enumerator->destroy(enumerator);
	if (unknown)
	{
		DBG1(DBG_IKE, "received %u cert requests for an unknown ca",
			 unknown);
	}
}

/**
 * Process a single notify payload
 */
static void process_notify(private_ike_cert_pre_t *this,
						   notify_payload_t *notify)
{
	switch (notify->get_notify_type(notify))
	{
		case HTTP_CERT_LOOKUP_SUPPORTED:
			this->ike_sa->enable_extension(this->ike_sa, EXT_HASH_AND_URL);
			break;
		default:
			break;
	}
}

/**
 * read certificate requests
 */
static void process_certreqs(private_ike_cert_pre_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	auth_cfg_t *auth;

	auth = this->ike_sa->get_auth_cfg(this->ike_sa, TRUE);

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		switch (payload->get_type(payload))
		{
			case PLV2_CERTREQ:
				process_certreq(this, (certreq_payload_t*)payload, auth);
				break;
			case PLV2_NOTIFY:
				process_notify(this, (notify_payload_t*)payload);
				break;
			default:
				/* ignore other payloads here, these are handled elsewhere */
				break;
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * tries to extract a certificate from the cert payload or the credential
 * manager (based on the hash of a "Hash and URL" encoded cert).
 * Note: the returned certificate (if any) has to be destroyed
 */
static certificate_t *try_get_cert(cert_payload_t *cert_payload)
{
	certificate_t *cert = NULL;

	switch (cert_payload->get_cert_encoding(cert_payload))
	{
		case ENC_X509_SIGNATURE:
		{
			cert = cert_payload->get_cert(cert_payload);
			break;
		}
		case ENC_X509_HASH_AND_URL:
		{
			identification_t *id;
			chunk_t hash = cert_payload->get_hash(cert_payload);
			if (!hash.ptr)
			{
				/* invalid "Hash and URL" data (logged elsewhere) */
				break;
			}
			id = identification_create_from_encoding(ID_KEY_ID, hash);
			cert = lib->credmgr->get_cert(lib->credmgr,
										  CERT_X509, KEY_ANY, id, FALSE);
			id->destroy(id);
			break;
		}
		default:
		{
			break;
		}
	}
	return cert;
}

/**
 * Process a X509 certificate payload
 */
static void process_x509(cert_payload_t *payload, auth_cfg_t *auth,
						 cert_encoding_t encoding, bool *first)
{
	certificate_t *cert;
	char *url;

	cert = try_get_cert(payload);
	if (cert)
	{
		if (*first)
		{	/* the first is an end entity certificate */
			DBG1(DBG_IKE, "received end entity cert \"%Y\"",
				 cert->get_subject(cert));
			auth->add(auth, AUTH_HELPER_SUBJECT_CERT, cert);
			*first = FALSE;
		}
		else
		{
			DBG1(DBG_IKE, "received issuer cert \"%Y\"",
				 cert->get_subject(cert));
			auth->add(auth, AUTH_HELPER_IM_CERT, cert);
		}
	}
	else if (encoding == ENC_X509_HASH_AND_URL)
	{
		/* we fetch the certificate not yet, but only if
		 * it is really needed during authentication */
		url = payload->get_url(payload);
		if (!url)
		{
			DBG1(DBG_IKE, "received invalid hash-and-url "
				 "encoded cert, ignore");
			return;
		}
		url = strdup(url);
		if (first)
		{	/* first URL is for an end entity certificate */
			DBG1(DBG_IKE, "received hash-and-url for end entity cert \"%s\"",
				 url);
			auth->add(auth, AUTH_HELPER_SUBJECT_HASH_URL, url);
			first = FALSE;
		}
		else
		{
			DBG1(DBG_IKE, "received hash-and-url for issuer cert \"%s\"", url);
			auth->add(auth, AUTH_HELPER_IM_HASH_URL, url);
		}
	}
}

/**
 * Process a CRL certificate payload
 */
static void process_crl(cert_payload_t *payload, auth_cfg_t *auth)
{
	certificate_t *cert;

	cert = payload->get_cert(payload);
	if (cert)
	{
		DBG1(DBG_IKE, "received CRL \"%Y\"", cert->get_subject(cert));
		auth->add(auth, AUTH_HELPER_REVOCATION_CERT, cert);
	}
}

/**
 * Process an attribute certificate payload
 */
static void process_ac(cert_payload_t *payload, auth_cfg_t *auth)
{
	certificate_t *cert;

	cert = payload->get_cert(payload);
	if (cert)
	{
		if (cert->get_issuer(cert))
		{
			DBG1(DBG_IKE, "received attribute certificate issued by \"%Y\"",
				 cert->get_issuer(cert));
		}
		else if (cert->get_subject(cert))
		{
			DBG1(DBG_IKE, "received attribute certificate for \"%Y\"",
				 cert->get_subject(cert));
		}
		auth->add(auth, AUTH_HELPER_AC_CERT, cert);
	}
}

/**
 * Process certificate payloads
 */
static void process_certs(private_ike_cert_pre_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	auth_cfg_t *auth;
	bool first = TRUE;

	auth = this->ike_sa->get_auth_cfg(this->ike_sa, FALSE);

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV2_CERTIFICATE)
		{
			cert_payload_t *cert_payload;
			cert_encoding_t encoding;

			cert_payload = (cert_payload_t*)payload;
			encoding = cert_payload->get_cert_encoding(cert_payload);

			switch (encoding)
			{
				case ENC_X509_HASH_AND_URL:
					if (!this->do_http_lookup)
					{
						DBG1(DBG_IKE, "received hash-and-url encoded cert, but "
							 "we don't accept them, ignore");
						break;
					}
					/* FALL */
				case ENC_X509_SIGNATURE:
					process_x509(cert_payload, auth, encoding, &first);
					break;
				case ENC_CRL:
					process_crl(cert_payload, auth);
					break;
				case ENC_X509_ATTRIBUTE:
					process_ac(cert_payload, auth);
					break;
				case ENC_PKCS7_WRAPPED_X509:
				case ENC_PGP:
				case ENC_DNS_SIGNED_KEY:
				case ENC_KERBEROS_TOKEN:
				case ENC_ARL:
				case ENC_SPKI:
				case ENC_RAW_RSA_KEY:
				case ENC_X509_HASH_AND_URL_BUNDLE:
				case ENC_OCSP_CONTENT:
				default:
					DBG1(DBG_ENC, "certificate encoding %N not supported",
						 cert_encoding_names, encoding);
			}
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * add the keyid of a certificate to the certificate request payload
 */
static void add_certreq(certreq_payload_t **req, certificate_t *cert)
{
	switch (cert->get_type(cert))
	{
		case CERT_X509:
		{
			public_key_t *public;
			chunk_t keyid;
			x509_t *x509 = (x509_t*)cert;

			if (!(x509->get_flags(x509) & X509_CA))
			{	/* no CA cert, skip */
				break;
			}
			public = cert->get_public_key(cert);
			if (!public)
			{
				break;
			}
			if (*req == NULL)
			{
				*req = certreq_payload_create_type(CERT_X509);
			}
			if (public->get_fingerprint(public, KEYID_PUBKEY_INFO_SHA1, &keyid))
			{
				(*req)->add_keyid(*req, keyid);
				DBG1(DBG_IKE, "sending cert request for \"%Y\"",
					 cert->get_subject(cert));
			}
			public->destroy(public);
			break;
		}
		default:
			break;
	}
}

/**
 * add a auth_cfg's CA certificates to the certificate request
 */
static void add_certreqs(certreq_payload_t **req, auth_cfg_t *auth)
{
	enumerator_t *enumerator;
	auth_rule_t type;
	void *value;

	enumerator = auth->create_enumerator(auth);
	while (enumerator->enumerate(enumerator, &type, &value))
	{
		switch (type)
		{
			case AUTH_RULE_CA_CERT:
				add_certreq(req, (certificate_t*)value);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * build certificate requests
 */
static void build_certreqs(private_ike_cert_pre_t *this, message_t *message)
{
	enumerator_t *enumerator;
	ike_cfg_t *ike_cfg;
	peer_cfg_t *peer_cfg;
	certificate_t *cert;
	auth_cfg_t *auth;
	certreq_payload_t *req = NULL;

	ike_cfg = this->ike_sa->get_ike_cfg(this->ike_sa);
	if (!ike_cfg->send_certreq(ike_cfg))
	{
		return;
	}

	/* check if we require a specific CA for that peer */
	peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
	if (peer_cfg)
	{
		enumerator = peer_cfg->create_auth_cfg_enumerator(peer_cfg, FALSE);
		while (enumerator->enumerate(enumerator, &auth))
		{
			add_certreqs(&req, auth);
		}
		enumerator->destroy(enumerator);
	}

	if (!req)
	{
		/* otherwise add all trusted CA certificates */
		enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
												CERT_ANY, KEY_ANY, NULL, TRUE);
		while (enumerator->enumerate(enumerator, &cert))
		{
			add_certreq(&req, cert);
		}
		enumerator->destroy(enumerator);
	}

	if (req)
	{
		message->add_payload(message, (payload_t*)req);

		if (lib->settings->get_bool(lib->settings,
									"%s.hash_and_url", FALSE, lib->ns))
		{
			message->add_notify(message, FALSE, HTTP_CERT_LOOKUP_SUPPORTED,
								chunk_empty);
			this->do_http_lookup = TRUE;
		}
	}
}

/**
 * Check if this is the final authentication round
 */
static bool final_auth(message_t *message)
{
	/* we check for an AUTH payload without a ANOTHER_AUTH_FOLLOWS notify */
	if (message->get_payload(message, PLV2_AUTH) == NULL)
	{
		return FALSE;
	}
	if (message->get_notify(message, ANOTHER_AUTH_FOLLOWS))
	{
		return FALSE;
	}
	return TRUE;
}

METHOD(task_t, build_i, status_t,
	private_ike_cert_pre_t *this, message_t *message)
{
	if (message->get_message_id(message) == 1)
	{	/* initiator sends CERTREQs in first IKE_AUTH */
		build_certreqs(this, message);
	}
	return NEED_MORE;
}

METHOD(task_t, process_r, status_t,
	private_ike_cert_pre_t *this, message_t *message)
{
	if (message->get_exchange_type(message) != IKE_SA_INIT)
	{	/* handle certreqs/certs in any IKE_AUTH, just in case */
		process_certreqs(this, message);
		process_certs(this, message);
	}
	this->final = final_auth(message);
	return NEED_MORE;
}

METHOD(task_t, build_r, status_t,
	private_ike_cert_pre_t *this, message_t *message)
{
	if (message->get_exchange_type(message) == IKE_SA_INIT)
	{
		build_certreqs(this, message);
	}
	if (this->final)
	{
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, process_i, status_t,
	private_ike_cert_pre_t *this, message_t *message)
{
	if (message->get_exchange_type(message) == IKE_SA_INIT)
	{
		process_certreqs(this, message);
	}
	process_certs(this, message);

	if (final_auth(message))
	{
		return SUCCESS;
	}
	return NEED_MORE;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_cert_pre_t *this)
{
	return TASK_IKE_CERT_PRE;
}

METHOD(task_t, migrate, void,
	private_ike_cert_pre_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
}

METHOD(task_t, destroy, void,
	private_ike_cert_pre_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
ike_cert_pre_t *ike_cert_pre_create(ike_sa_t *ike_sa, bool initiator)
{
	private_ike_cert_pre_t *this;

	INIT(this,
		.public = {
			.task = {
				.get_type = _get_type,
				.migrate = _migrate,
				.destroy = _destroy,
			},
		},
		.ike_sa = ike_sa,
		.initiator = initiator,
	);

	if (initiator)
	{
		this->public.task.build = _build_i;
		this->public.task.process = _process_i;
	}
	else
	{
		this->public.task.build = _build_r;
		this->public.task.process = _process_r;
	}

	return &this->public;
}
