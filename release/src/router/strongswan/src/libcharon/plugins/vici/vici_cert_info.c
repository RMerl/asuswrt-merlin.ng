/*
 * Copyright (C) 2015 Andreas Steffen
 * HSR Hochschule fuer Technik Rapperswil
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

#include "vici_cert_info.h"

/**
 * Legacy vici certificate types and directories created by swanctl
 */
typedef struct {

	/** Certificate type string used in legacy vici messages */
	char *type_str;
	/** Base certificate type */
	certificate_type_t type;
	/** X.509 flag */
	x509_flag_t flag;
} cert_type_t;

static cert_type_t cert_types[] = {
	{ "x509",     CERT_X509,           X509_NONE        },
	{ "x509ca",   CERT_X509,           X509_CA          },
	{ "x509ocsp", CERT_X509,           X509_OCSP_SIGNER },
	{ "x509aa",   CERT_X509,           X509_AA          },
	{ "x509ac",   CERT_X509_AC,        X509_NONE        },
	{ "x509crl",  CERT_X509_CRL,       X509_NONE        },
	{ "pubkey",   CERT_TRUSTED_PUBKEY, X509_NONE        },
};

bool vici_cert_info_from_str(char *type_str, certificate_type_t *type,
							 x509_flag_t *flag)
{
	int i;

	for (i = 0; i < countof(cert_types); i++)
	{
		if (strcaseeq(type_str, cert_types[i].type_str))
		{
			*type = cert_types[i].type;
			*flag = cert_types[i].flag;
			return TRUE;
		}
	}
	return FALSE;
}

