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

#include "ike_cert_post.h"

#include <daemon.h>
#include <sa/ike_sa.h>
#include <encoding/payloads/cert_payload.h>
#include <encoding/payloads/certreq_payload.h>
#include <encoding/payloads/auth_payload.h>
#include <credentials/certificates/x509.h>
#include <credentials/certificates/ac.h>


typedef struct private_ike_cert_post_t private_ike_cert_post_t;

/**
 * Private members of a ike_cert_post_t task.
 */
struct private_ike_cert_post_t {

	/**
	 * Public methods and task_t interface.
	 */
	ike_cert_post_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;
};

/**
 * Generates the cert payload, if possible with "Hash and URL"
 */
static cert_payload_t *build_cert_payload(private_ike_cert_post_t *this,
										 certificate_t *cert)
{
	hasher_t *hasher;
	identification_t *id;
	chunk_t hash, encoded ;
	enumerator_t *enumerator;
	char *url;
	cert_payload_t *payload = NULL;

	if (!this->ike_sa->supports_extension(this->ike_sa, EXT_HASH_AND_URL))
	{
		return cert_payload_create_from_cert(PLV2_CERTIFICATE, cert);
	}

	hasher = lib->crypto->create_hasher(lib->crypto, HASH_SHA1);
	if (!hasher)
	{
		DBG1(DBG_IKE, "unable to use hash-and-url: sha1 not supported");
		return cert_payload_create_from_cert(PLV2_CERTIFICATE, cert);
	}

	if (!cert->get_encoding(cert, CERT_ASN1_DER, &encoded))
	{
		DBG1(DBG_IKE, "encoding certificate for cert payload failed");
		hasher->destroy(hasher);
		return NULL;
	}
	if (!hasher->allocate_hash(hasher, encoded, &hash))
	{
		hasher->destroy(hasher);
		chunk_free(&encoded);
		return cert_payload_create_from_cert(PLV2_CERTIFICATE, cert);
	}
	chunk_free(&encoded);
	hasher->destroy(hasher);
	id = identification_create_from_encoding(ID_KEY_ID, hash);

	enumerator = lib->credmgr->create_cdp_enumerator(lib->credmgr, CERT_X509, id);
	if (enumerator->enumerate(enumerator, &url))
	{
		payload = cert_payload_create_from_hash_and_url(hash, url);
		DBG1(DBG_IKE, "sending hash-and-url \"%s\"", url);
	}
	else
	{
		payload = cert_payload_create_from_cert(PLV2_CERTIFICATE, cert);
	}
	enumerator->destroy(enumerator);
	chunk_free(&hash);
	id->destroy(id);
	return payload;
}

/**
 * Add subject certificate to message
 */
static bool add_subject_cert(private_ike_cert_post_t *this, auth_cfg_t *auth,
							 message_t *message)
{
	cert_payload_t *payload;
	certificate_t *cert;

	cert = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
	if (!cert)
	{
		return FALSE;
	}
	payload = build_cert_payload(this, cert);
	if (!payload)
	{
		return FALSE;
	}
	DBG1(DBG_IKE, "sending end entity cert \"%Y\"", cert->get_subject(cert));
	message->add_payload(message, (payload_t*)payload);
	return TRUE;
}

/**
 * Add intermediate CA certificates to message
 */
static void add_im_certs(private_ike_cert_post_t *this, auth_cfg_t *auth,
						 message_t *message)
{
	cert_payload_t *payload;
	enumerator_t *enumerator;
	certificate_t *cert;
	auth_rule_t type;

	enumerator = auth->create_enumerator(auth);
	while (enumerator->enumerate(enumerator, &type, &cert))
	{
		if (type == AUTH_RULE_IM_CERT)
		{
			payload = cert_payload_create_from_cert(PLV2_CERTIFICATE, cert);
			if (payload)
			{
				DBG1(DBG_IKE, "sending issuer cert \"%Y\"",
					 cert->get_subject(cert));
				message->add_payload(message, (payload_t*)payload);
			}
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Add any valid attribute certificates of subject to message
 */
static void add_attribute_certs(private_ike_cert_post_t *this,
								auth_cfg_t *auth, message_t *message)
{
	certificate_t *subject, *cert;

	subject = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
	if (subject && subject->get_type(subject) == CERT_X509)
	{
		x509_t *x509 = (x509_t*)subject;
		identification_t *id, *serial;
		enumerator_t *enumerator;
		cert_payload_t *payload;
		ac_t *ac;

		/* we look for attribute certs having our serial and holder issuer,
		 * which is recommended by RFC 5755 */
		serial = identification_create_from_encoding(ID_KEY_ID,
													 x509->get_serial(x509));
		enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
										CERT_X509_AC, KEY_ANY, serial, FALSE);
		while (enumerator->enumerate(enumerator, &ac))
		{
			cert = &ac->certificate;
			id = ac->get_holderIssuer(ac);
			if (id && id->equals(id, subject->get_issuer(subject)) &&
				cert->get_validity(cert, NULL, NULL, NULL))
			{
				payload = cert_payload_create_from_cert(PLV2_CERTIFICATE, cert);
				if (payload)
				{
					DBG1(DBG_IKE, "sending attribute certificate "
						 "issued by \"%Y\"", cert->get_issuer(cert));
					message->add_payload(message, (payload_t*)payload);
				}
			}
		}
		enumerator->destroy(enumerator);
		serial->destroy(serial);
	}
}

/**
 * add certificates to message
 */
static void build_certs(private_ike_cert_post_t *this, message_t *message)
{
	peer_cfg_t *peer_cfg;
	auth_payload_t *payload;
	auth_cfg_t *auth;

	payload = (auth_payload_t*)message->get_payload(message, PLV2_AUTH);
	peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
	if (!peer_cfg || !payload || payload->get_auth_method(payload) == AUTH_PSK)
	{	/* no CERT payload for EAP/PSK */
		return;
	}

	switch (peer_cfg->get_cert_policy(peer_cfg))
	{
		case CERT_NEVER_SEND:
			break;
		case CERT_SEND_IF_ASKED:
			if (!this->ike_sa->has_condition(this->ike_sa, COND_CERTREQ_SEEN))
			{
				break;
			}
			/* FALL */
		case CERT_ALWAYS_SEND:
			auth = this->ike_sa->get_auth_cfg(this->ike_sa, TRUE);
			if (add_subject_cert(this, auth, message))
			{
				add_im_certs(this, auth, message);
				add_attribute_certs(this, auth, message);
			}
			break;
	}
}

METHOD(task_t, build_i, status_t,
	private_ike_cert_post_t *this, message_t *message)
{
	build_certs(this, message);

	return NEED_MORE;
}

METHOD(task_t, process_r, status_t,
	private_ike_cert_post_t *this, message_t *message)
{
	return NEED_MORE;
}

METHOD(task_t, build_r, status_t,
	private_ike_cert_post_t *this, message_t *message)
{
	build_certs(this, message);

	if (this->ike_sa->get_state(this->ike_sa) != IKE_ESTABLISHED)
	{	/* stay alive, we might have additional rounds with certs */
		return NEED_MORE;
	}
	return SUCCESS;
}

METHOD(task_t, process_i, status_t,
	private_ike_cert_post_t *this, message_t *message)
{
	if (this->ike_sa->get_state(this->ike_sa) != IKE_ESTABLISHED)
	{	/* stay alive, we might have additional rounds with CERTS */
		return NEED_MORE;
	}
	return SUCCESS;
}

METHOD(task_t, get_type, task_type_t,
	private_ike_cert_post_t *this)
{
	return TASK_IKE_CERT_POST;
}

METHOD(task_t, migrate, void,
	private_ike_cert_post_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
}

METHOD(task_t, destroy, void,
	private_ike_cert_post_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
ike_cert_post_t *ike_cert_post_create(ike_sa_t *ike_sa, bool initiator)
{
	private_ike_cert_post_t *this;

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
