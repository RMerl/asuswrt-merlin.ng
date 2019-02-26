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

#include "isakmp_cert_post.h"

#include <daemon.h>
#include <sa/ike_sa.h>
#include <encoding/payloads/cert_payload.h>
#include <encoding/payloads/certreq_payload.h>
#include <encoding/payloads/auth_payload.h>
#include <encoding/payloads/sa_payload.h>
#include <credentials/certificates/x509.h>


typedef struct private_isakmp_cert_post_t private_isakmp_cert_post_t;

/**
 * Private members of a isakmp_cert_post_t task.
 */
struct private_isakmp_cert_post_t {

	/**
	 * Public methods and task_t interface.
	 */
	isakmp_cert_post_t public;

	/**
	 * Assigned IKE_SA.
	 */
	ike_sa_t *ike_sa;

	/**
	 * Are we the initiator?
	 */
	bool initiator;

	/**
	 * States of ike cert pre
	 */
	enum {
		CR_SA,
		CR_KE,
		CR_AUTH,
	} state;
};

/**
 * Check if we actually use certificates for authentication
 */
static bool use_certs(private_isakmp_cert_post_t *this, message_t *message)
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
				case AUTH_RSA:
				case AUTH_ECDSA_256:
				case AUTH_ECDSA_384:
				case AUTH_ECDSA_521:
				case AUTH_XAUTH_INIT_RSA:
				case AUTH_XAUTH_RESP_RSA:
				case AUTH_HYBRID_INIT_RSA:
				case AUTH_HYBRID_RESP_RSA:
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
 * Add certificates to message
 */
static void build_certs(private_isakmp_cert_post_t *this, message_t *message)
{
	peer_cfg_t *peer_cfg;

	peer_cfg = this->ike_sa->get_peer_cfg(this->ike_sa);
	if (!peer_cfg)
	{
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
		{
			cert_payload_t *payload;
			enumerator_t *enumerator;
			certificate_t *cert;
			auth_rule_t type;
			auth_cfg_t *auth;

			auth = this->ike_sa->get_auth_cfg(this->ike_sa, TRUE);
			cert = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
			if (!cert)
			{
				break;
			}
			payload = cert_payload_create_from_cert(PLV1_CERTIFICATE, cert);
			if (!payload)
			{
				break;
			}
			DBG1(DBG_IKE, "sending end entity cert \"%Y\"",
				 cert->get_subject(cert));
			message->add_payload(message, (payload_t*)payload);

			enumerator = auth->create_enumerator(auth);
			while (enumerator->enumerate(enumerator, &type, &cert))
			{
				if (type == AUTH_RULE_IM_CERT)
				{
					payload = cert_payload_create_from_cert(PLV1_CERTIFICATE, cert);
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
	}
}

METHOD(task_t, build_i, status_t,
	private_isakmp_cert_post_t *this, message_t *message)
{
	switch (message->get_exchange_type(message))
	{
		case ID_PROT:
			if (this->state == CR_AUTH)
			{
				build_certs(this, message);
				return SUCCESS;
			}
			return NEED_MORE;
		case AGGRESSIVE:
			if (this->state == CR_AUTH)
			{
				build_certs(this, message);
				return SUCCESS;
			}
			return NEED_MORE;
		default:
			return FAILED;
	}
}

METHOD(task_t, process_r, status_t,
	private_isakmp_cert_post_t *this, message_t *message)
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
					return NEED_MORE;
				case CR_AUTH:
					return NEED_MORE;
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
					return NEED_MORE;
				case CR_AUTH:
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
	private_isakmp_cert_post_t *this, message_t *message)
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
					this->state = CR_AUTH;
					return NEED_MORE;
				case CR_AUTH:
					build_certs(this, message);
					return SUCCESS;
			}
		case AGGRESSIVE:
			switch (this->state)
			{
				case CR_SA:
					build_certs(this, message);
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
	private_isakmp_cert_post_t *this, message_t *message)
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
					this->state = CR_AUTH;
					return NEED_MORE;
				case CR_AUTH:
					return SUCCESS;
				default:
					return FAILED;
			}
		}
		case AGGRESSIVE:
		{
			if (this->state == CR_SA)
			{
				if (!use_certs(this, message))
				{
					return SUCCESS;
				}
				this->state = CR_AUTH;
				return NEED_MORE;
			}
			return SUCCESS;
		}
		default:
			return FAILED;
	}
}

METHOD(task_t, get_type, task_type_t,
	private_isakmp_cert_post_t *this)
{
	return TASK_ISAKMP_CERT_POST;
}

METHOD(task_t, migrate, void,
	private_isakmp_cert_post_t *this, ike_sa_t *ike_sa)
{
	this->ike_sa = ike_sa;
	this->state = CR_SA;
}

METHOD(task_t, destroy, void,
	private_isakmp_cert_post_t *this)
{
	free(this);
}

/*
 * Described in header.
 */
isakmp_cert_post_t *isakmp_cert_post_create(ike_sa_t *ike_sa, bool initiator)
{
	private_isakmp_cert_post_t *this;

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
	);
	if (initiator)
	{
		this->public.task.process = _process_i;
		this->public.task.build = _build_i;
	}
	else
	{
		this->public.task.process = _process_r;
		this->public.task.build = _build_r;
	}
	return &this->public;
}
