/*
 * Copyright (C) 2020 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
 * Copyright (C) 2015 Andreas Steffen
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

#include "certificate.h"

#include <utils/debug.h>
#include <credentials/certificates/x509.h>

ENUM(certificate_type_names, CERT_ANY, CERT_GPG,
	"ANY",
	"X509",
	"X509_CRL",
	"OCSP_REQUEST",
	"OCSP_RESPONSE",
	"X509_AC",
	"PUBKEY",
	"PKCS10_REQUEST",
	"PGP",
);

ENUM(cert_validation_names, VALIDATION_GOOD, VALIDATION_REVOKED,
	"GOOD",
	"SKIPPED",
	"STALE",
	"FAILED",
	"ON_HOLD",
	"REVOKED",
);

/**
 * See header
 */
bool certificate_is_newer(certificate_t *this, certificate_t *other)
{
	time_t this_update, that_update;
	char *type = "certificate";
	bool newer;

	if (this->get_type(this) == CERT_X509_CRL)
	{
		type = "crl";
	}
	this->get_validity(this, NULL, &this_update, NULL);
	other->get_validity(other, NULL, &that_update, NULL);
	newer = this_update > that_update;
	DBG1(DBG_LIB, "  %s from %T is %s - existing %s from %T %s",
		 type, &this_update, FALSE, newer ? "newer" : "not newer",
		 type, &that_update, FALSE, newer ? "replaced" : "retained");
	return newer;
}

/*
 * Described in header
 */
bool certificate_matches(certificate_t *cert, certificate_type_t type,
						 key_type_t key, identification_t *id)
{
	public_key_t *public;

	if (type != CERT_ANY && type != cert->get_type(cert))
	{
		return FALSE;
	}
	public = cert->get_public_key(cert);
	if (public)
	{
		if (key == KEY_ANY || key == public->get_type(public))
		{
			if (id && public->has_fingerprint(public, id->get_encoding(id)))
			{
				public->destroy(public);
				return TRUE;
			}
		}
		else
		{
			public->destroy(public);
			return FALSE;
		}
		public->destroy(public);
	}
	else if (key != KEY_ANY)
	{
		return FALSE;
	}
	return !id || cert->has_subject(cert, id);
}
