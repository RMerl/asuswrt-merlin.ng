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

#include "certexpire_listener.h"

#include <daemon.h>

typedef struct private_certexpire_listener_t private_certexpire_listener_t;

/**
 * Private data of an certexpire_listener_t object.
 */
struct private_certexpire_listener_t {

	/**
	 * Public certexpire_listener_t interface.
	 */
	certexpire_listener_t public;

	/**
	 * Export facility
	 */
	certexpire_export_t *export;
};

METHOD(listener_t, authorize, bool,
	private_certexpire_listener_t *this, ike_sa_t *ike_sa,
	bool final, bool *success)
{
	enumerator_t *rounds, *enumerator;
	certificate_t *cert, *ca = NULL;
	linked_list_t *trustchain;
	auth_cfg_t *auth;
	auth_rule_t rule;

	/* Check all rounds in final hook, as local authentication data are
	 * not completely available after round-invocation. */
	if (!final)
	{
		return TRUE;
	}

	/* collect local certificates */
	trustchain = linked_list_create();
	rounds = ike_sa->create_auth_cfg_enumerator(ike_sa, TRUE);
	while (rounds->enumerate(rounds, &auth))
	{
		cert = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
		if (cert)
		{
			trustchain->insert_last(trustchain, cert);

			enumerator = auth->create_enumerator(auth);
			while (enumerator->enumerate(enumerator, &rule, &cert))
			{
				if (rule == AUTH_RULE_IM_CERT)
				{
					trustchain->insert_last(trustchain, cert);
				}
				if (rule == AUTH_RULE_CA_CERT)
				{
					/* the last CA cert is the one used in the trustchain.
					 * Previous CA certificates have been received as cert
					 * requests. */
					ca = cert;
				}
			}
			enumerator->destroy(enumerator);
			if (ca)
			{
				trustchain->insert_last(trustchain, ca);
			}
		}
	}
	rounds->destroy(rounds);
	this->export->add(this->export, trustchain, TRUE);
	trustchain->destroy(trustchain);

	/* collect remote certificates */
	trustchain = linked_list_create();
	rounds = ike_sa->create_auth_cfg_enumerator(ike_sa, FALSE);
	while (rounds->enumerate(rounds, &auth))
	{
		cert = auth->get(auth, AUTH_RULE_SUBJECT_CERT);
		if (cert)
		{
			trustchain->insert_last(trustchain, cert);

			enumerator = auth->create_enumerator(auth);
			while (enumerator->enumerate(enumerator, &rule, &cert))
			{
				if (rule == AUTH_RULE_IM_CERT)
				{
					trustchain->insert_last(trustchain, cert);
				}
			}
			enumerator->destroy(enumerator);

			cert = auth->get(auth, AUTH_RULE_CA_CERT);
			if (cert)
			{
				trustchain->insert_last(trustchain, cert);
			}
		}
	}
	rounds->destroy(rounds);
	this->export->add(this->export, trustchain, FALSE);
	trustchain->destroy(trustchain);
	return TRUE;
}

METHOD(certexpire_listener_t, destroy, void,
	private_certexpire_listener_t *this)
{
	free(this);
}

/**
 * See header
 */
certexpire_listener_t *certexpire_listener_create(certexpire_export_t *export)
{
	private_certexpire_listener_t *this;

	INIT(this,
		.public = {
			.listener = {
				.authorize = _authorize,
			},
			.destroy = _destroy,
		},
		.export = export,
	);

	return &this->public;
}
