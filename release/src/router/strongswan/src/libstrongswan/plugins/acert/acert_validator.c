/*
 * Copyright (C) 2014 Martin Willi
 * Copyright (C) 2014 revosec AG
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
#include <library.h>

#include "acert_validator.h"

#include <credentials/certificates/x509.h>
#include <credentials/certificates/ac.h>

typedef struct private_acert_validator_t private_acert_validator_t;

/**
 * Private data of an acert_validator_t object.
 */
struct private_acert_validator_t {

	/**
	 * Public acert_validator_t interface.
	 */
	acert_validator_t public;
};

/**
 * Check if an AC can be trusted
 */
static bool verify(private_acert_validator_t *this, certificate_t *ac)
{
	certificate_t *issuer;
	enumerator_t *enumerator;
	bool verified = FALSE;

	if (!ac->get_validity(ac, NULL, NULL, NULL))
	{
		return FALSE;
	}
	DBG1(DBG_CFG, "verifying attribute certificate issued by \"%Y\"",
		 ac->get_issuer(ac));
	enumerator = lib->credmgr->create_trusted_enumerator(lib->credmgr, KEY_ANY,
													ac->get_issuer(ac), TRUE);
	while (enumerator->enumerate(enumerator, &issuer, NULL))
	{
		if (issuer->get_validity(issuer, NULL, NULL, NULL))
		{
			if (lib->credmgr->issued_by(lib->credmgr, ac, issuer, NULL))
			{
				verified = TRUE;
				break;
			}
		}
	}
	enumerator->destroy(enumerator);

	return verified;
}

/**
 * Apply AC group membership to auth config
 */
static void apply(private_acert_validator_t *this, ac_t *ac, auth_cfg_t *auth)
{
	enumerator_t *enumerator;
	ac_group_type_t type;
	chunk_t chunk;

	enumerator = ac->create_group_enumerator(ac);
	while (enumerator->enumerate(enumerator, &type, &chunk))
	{
		if (type == AC_GROUP_TYPE_STRING)
		{
			auth->add(auth, AUTH_RULE_GROUP,
					  identification_create_from_data(chunk));
		}
	}
	enumerator->destroy(enumerator);
}

METHOD(cert_validator_t, validate, bool,
	private_acert_validator_t *this, certificate_t *subject,
	certificate_t *issuer, bool online, u_int pathlen, bool anchor,
	auth_cfg_t *auth)
{
	/* for X.509 end entity certs only */
	if (pathlen == 0 && subject->get_type(subject) == CERT_X509)
	{
		x509_t *x509 = (x509_t*)subject;
		enumerator_t *enumerator;
		identification_t *id, *serial;
		ac_t *ac;

		/* find attribute certificates by serial and issuer. A lookup by
		 * the holder DN would work as well, but RFC 5755 recommends the use
		 * of baseCertificateID. */
		serial = identification_create_from_encoding(ID_KEY_ID,
													 x509->get_serial(x509));
		enumerator = lib->credmgr->create_cert_enumerator(lib->credmgr,
										CERT_X509_AC, KEY_ANY, serial, FALSE);
		while (enumerator->enumerate(enumerator, &ac))
		{
			id = ac->get_holderIssuer(ac);
			if (id && id->equals(id, subject->get_issuer(subject)))
			{
				if (verify(this, &ac->certificate))
				{
					apply(this, ac, auth);
				}
			}
		}
		enumerator->destroy(enumerator);
		serial->destroy(serial);
	}
	return TRUE;
}

METHOD(acert_validator_t, destroy, void,
	private_acert_validator_t *this)
{
	free(this);
}

/**
 * See header
 */
acert_validator_t *acert_validator_create()
{
	private_acert_validator_t *this;

	INIT(this,
		.public = {
			.validator.validate = _validate,
			.destroy = _destroy,
		},
	);

	return &this->public;
}
