/*
 * Copyright (C) 2011 Martin Willi
 * Copyright (C) 2011 revosec AG
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
 * Copyright (C) 2013 Volker Rümelin
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

#include "isakmp_cert_pre.h"

#include <daemon.h>
#include <sa/ike_sa.h>
#include <encoding/payloads/cert_payload.h>
#include <encoding/payloads/sa_payload.h>
#include <encoding/payloads/certreq_payload.h>
#include <credentials/certificates/x509.h>
#include <credentials/containers/pkcs7.h>


typedef struct private_isakmp_cert_pre_t private_isakmp_cert_pre_t;

/**
 * Private members of a isakmp_cert_pre_t task.
 */
struct private_isakmp_cert_pre_t {

	/**
	 * Public methods and task_t interface.
	 */
	isakmp_cert_pre_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * Send certificate requests?
	 */
	bool send_req;

	/** next message we expect */
	enum {
		CR_SA,
		CR_KE,
		CR_AUTH,
	} state;
};

/**
 * Find the CA certificate for a given certreq payload
 */
static certificate_t* find_certificate(private_isakmp_cert_pre_t *this,
									   certreq_payload_t *certreq)
{
	identification_t *id;
	certificate_t *cert;

	if (certreq->get_cert_type(certreq) != CERT_X509)
	{
		DBG1(DBG_IKE, "%N CERTREQ not supported - ignored",
			 certificate_type_names, certreq->get_cert_type(certreq));
		return NULL;
	}
	id = certreq->get_dn(certreq);
	if (!id)
	{
		DBG1(DBG_IKE, "ignoring certificate request without data",
			 certificate_type_names, certreq->get_cert_type(certreq));
		return NULL;
	}
	cert = lib->credmgr->get_cert(lib->credmgr, CERT_X509, KEY_ANY, id, TRUE);
	if (cert)
	{
		DBG1(DBG_IKE, "received cert request for '%Y'",
			 cert->get_subject(cert));
	}
	else
	{
		DBG1(DBG_IKE, "received cert request for unknown ca '%Y'", id);
	}
	id->destroy(id);

	return cert;
}

/**
 * read certificate requests
 */
static void process_certreqs(private_isakmp_cert_pre_t *this, message_t *message)
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
			case PLV1_CERTREQ:
			{
				certificate_t *cert;

				this->ike_sa->set_condition(this->ike_sa,
											COND_CERTREQ_SEEN, TRUE);
				cert = find_certificate(this, (certreq_payload_t*)payload);
				if (cert)
				{
					auth->add(auth, AUTH_RULE_CA_CERT, cert);
				}
				break;
			}
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Process an X509 certificate payload
 */
static void process_x509(cert_payload_t *payload, auth_cfg_t *auth, bool *first)
{
	certificate_t *cert;

	cert = payload->get_cert(payload);
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
 * Process a PKCS7 certificate payload
 */
static void process_pkcs7(cert_payload_t *payload, auth_cfg_t *auth)
{
	enumerator_t *enumerator;
	container_t *container;
	certificate_t *cert;
	pkcs7_t *pkcs7;

	container = payload->get_container(payload);
	if (!container)
	{
		return;
	}
	switch (container->get_type(container))
	{
		case CONTAINER_PKCS7_DATA:
		case CONTAINER_PKCS7_SIGNED_DATA:
		case CONTAINER_PKCS7_ENVELOPED_DATA:
			break;
		default:
			container->destroy(container);
			return;
	}

	pkcs7 = (pkcs7_t *)container;
	enumerator = pkcs7->create_cert_enumerator(pkcs7);
	while (enumerator->enumerate(enumerator, &cert))
	{
		if (cert->get_type(cert) == CERT_X509)
		{
			x509_t *x509 = (x509_t*)cert;

			if (x509->get_flags(x509) & X509_CA)
			{
				DBG1(DBG_IKE, "received issuer cert \"%Y\"",
					 cert->get_subject(cert));
				auth->add(auth, AUTH_HELPER_IM_CERT, cert->get_ref(cert));
			}
			else
			{
				DBG1(DBG_IKE, "received end entity cert \"%Y\"",
					 cert->get_subject(cert));
				auth->add(auth, AUTH_HELPER_SUBJECT_CERT, cert->get_ref(cert));
			}
		}
		else
		{
			DBG1(DBG_IKE, "received unsupported cert type %N",
				 certificate_type_names, cert->get_type(cert));
		}
	}
	enumerator->destroy(enumerator);

	container->destroy(container);
}

/**
 * Import received certificates
 */
static void process_certs(private_isakmp_cert_pre_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	auth_cfg_t *auth;
	bool first = TRUE;

	auth = this->ike_sa->get_auth_cfg(this->ike_sa, FALSE);

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV1_CERTIFICATE)
		{
			cert_payload_t *cert_payload;
			cert_encoding_t encoding;

			cert_payload = (cert_payload_t*)payload;
			encoding = cert_payload->get_cert_encoding(cert_payload);

			switch (encoding)
			{
				case ENC_X509_SIGNATURE:
					process_x509(cert_payload, auth, &first);
					break;
				case ENC_CRL:
					process_crl(cert_payload, auth);
					break;
				case ENC_PKCS7_WRAPPED_X509:
					process_pkcs7(cert_payload, auth);
					break;
				case ENC_PGP:
				case ENC_DNS_SIGNED_KEY:
				case ENC_KERBEROS_TOKEN:
				case ENC_ARL:
				case ENC_SPKI:
				case ENC_X509_ATTRIBUTE:
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
 * Add the subject of a CA certificate a message
 */
static void add_certreq(private_isakmp_cert_pre_t *this, message_t *message,
						certificate_t *cert)
{
	if (cert->get_type(cert) == CERT_X509)
	{
		x509_t *x509 = (x509_t*)cert;

		if (x509->get_flags(x509) & X509_CA)
		{
			DBG1(DBG_IKE, "sending cert request for \"%Y\"",
				 cert->get_subject(cert));
			message->add_payload(message, (payload_t*)
							certreq_payload_create_dn(cert->get_subject(cert)));
		}
	}
}

/**
 * Add auth_cfg's CA certificates to the certificate request
 */
static void add_certreqs(private_isakmp_cert_pre_t *this,
						 auth_cfg_t *auth, message_t *message)
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
				add_certreq(this, message, (certificate_t*)value);
				break;
			default:
				break;
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * Build certificate requests
 */
static void build_certreqs(private_isakmp_cert_pre_t *this, message_t *message)
{
	enumerator_t *enumerator;
	ike_cfg_t *ike_cfg;
	peer_cfg_t *peer_cfg;
	certificate_t *cert;
	auth_cfg_t *auth;

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
		if (enumerator->enumerate(enumerator, &auth))
		{
			add_certreqs(this, auth, message);
		}
		enumerator->destroy(enumerator);
	}
	if (!message->get_payload(message, PLV1_CERTREQ))
	{
		/* otherwise add all trusted CA certificates */
		enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
												CERT_ANY, KEY_ANY, NULL, TRUE);
		while (enumerator->enumerate(enumerator, &cert))
		{
			add_certreq(this, message, cert);
		}
		enumerator->destroy(enumerator);
	}
}

/**
 * Check if we actually use certificates for authentication
 */
static bool use_certs(private_isakmp_cert_pre_t *this, message_t *message)
{
	enumerator_t *enumerator;
	payload_t *payload;
	bool use = FALSE;

	enumerator = message->create_payload_enumerator(message);
	while (enumerator->enumerate(enumerator, &payload))
	{
		if (payload->get_type(payload) == PLV1_SECURITY_ASSOCIATION)
		{
			sa_payload_t *sa_payload = (sa_payload_t*)payload;

			switch (sa_payload->get_auth_method(sa_payload))
			{
				case AUTH_HYBRID_INIT_RSA:
				case AUTH_HYBRID_RESP_RSA:
					if (!this->initiator)
					{
						this->send_req = FALSE;
					}
					/* FALL */
				case AUTH_RSA:
				case AUTH_ECDSA_256:
				case AUTH_ECDSA_384:
				case AUTH_ECDSA_521:
				case AUTH_XAUTH_INIT_RSA:
				case AUTH_XAUTH_RESP_RSA:
					use = TRUE;
					break;
				default:
					break;
			}
			break;
		}
	}
	enumerator->destroy(enumerator);

	return use;
}

/**
 * Check if we should send a certificate request
 */
static bool send_certreq(private_isakmp_cert_pre_t *this)
{
	enumerator_t *enumerator;
	peer_cfg_t *peer_cfg;
	auth_cfg_t *auth;
	bool req = FALSE;
	auth_class_t class;

	peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
	if (peer_cfg)
	{
		enumerator = peer_cfg->create_auth_cfg_enumerator(peer_cfg, FALSE);
		if (enumerator->enumerate(enumerator, &auth))
		{
			class = (intptr_t)auth->get(auth, AUTH_RULE_AUTH_CLASS);
			if (class == AUTH_CLASS_PUBKEY)
			{
				req = TRUE;
			}
		}
		enumerator->destroy(enumerator);
	}
	return req;
}

METHOD(task_t, build_i, status_t,
	private_isakmp_cert_pre_t *this, message_t *message)
{
	switch (message->get_exchange_type(message))
	{
		case ID_PROT:
			if (this->state == CR_AUTH)
			{
				build_certreqs(this, message);
			}
			return NEED_MORE;
		case AGGRESSIVE:
			if (this->state == CR_SA)
			{
				if (send_certreq(this))
				{
					build_certreqs(this, message);
				}
			}
			return NEED_MORE;
		default:
			return FAILED;
	}
}

METHOD(task_t, process_r, status_t,
	private_isakmp_cert_pre_t *this, message_t *message)
{
	switch (message->get_exchange_type(message))
	{
		case ID_PROT:
		{
			switch (this->state)
			{
				case CR_SA:
					if (!use_certs(this, message))
					{
						return SUCCESS;
					}
					return NEED_MORE;
				case CR_KE:
					process_certreqs(this, message);
					return NEED_MORE;
				case CR_AUTH:
					process_certreqs(this, message);
					process_certs(this, message);
					return SUCCESS;
				default:
					return FAILED;
			}
		}
		case AGGRESSIVE:
		{
			switch (this->state)
			{
				case CR_SA:
					if (!use_certs(this, message))
					{
						return SUCCESS;
					}
					process_certreqs(this, message);
					return NEED_MORE;
				case CR_AUTH:
					process_certs(this, message);
					return SUCCESS;
				default:
					return FAILED;
			}
		}
		default:
			return FAILED;
	}
}

METHOD(task_t, build_r, status_t,
	private_isakmp_cert_pre_t *this, message_t *message)
{
	switch (message->get_exchange_type(message))
	{
		case ID_PROT:
			switch (this->state)
			{
				case CR_SA:
					this->state = CR_KE;
					return NEED_MORE;
				case CR_KE:
					if (this->send_req)
					{
						build_certreqs(this, message);
					}
					this->state = CR_AUTH;
					return NEED_MORE;
				case CR_AUTH:
					return NEED_MORE;
				default:
					return FAILED;
			}
		case AGGRESSIVE:
			switch (this->state)
			{
				case CR_SA:
					if (this->send_req)
					{
						build_certreqs(this, message);
					}
					this->state = CR_AUTH;
					return NEED_MORE;
				case CR_AUTH:
					return SUCCESS;
				default:
					return FAILED;
			}
		default:
			return FAILED;
	}
}

METHOD(task_t, process_i, status_t,
	private_isakmp_cert_pre_t *this, message_t *message)
{
	switch (message->get_exchange_type(message))
	{
		case ID_PROT:
		{
			switch (this->state)
			{
				case CR_SA:
					if (!use_certs(this, message))
					{
						return SUCCESS;
					}
					this->state = CR_KE;
					return NEED_MORE;
				case CR_KE:
					process_certreqs(this, message);
					this->state = CR_AUTH;
					return NEED_MORE;
				case CR_AUTH:
					process_certs(this, message);
					return SUCCESS;
				default:
					return FAILED;
			}
			break;
		}
		case AGGRESSIVE:
		{
			if (!use_certs(this, message))
			{
				return SUCCESS;
			}
			process_certreqs(this, message);
			process_certs(this, message);
			this->state = CR_AUTH;
			return SUCCESS;
		}
		default:
			return FAILED;
	}
}

METHOD(task_t, get_type, task_type_t,
	private_isakmp_cert_pre_t *this)
{
	return TASK_ISAKMP_CERT_PRE;
}

METHOD(task_t, migrate, void,
	private_isakmp_cert_pre_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
	this->state = CR_SA;
	this->send_req = TRUE;
}

METHOD(task_t, destroy, void,
	private_isakmp_cert_pre_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
isakmp_cert_pre_t *isakmp_cert_pre_create(ike_sa_t *ike_sa, bool initiator)
{
	private_isakmp_cert_pre_t *this;

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
		.state = CR_SA,
		.send_req = TRUE,
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
