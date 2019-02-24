/*
 * Copyright (C) 2011 Andreas Steffen
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

#include "pts_creds.h"

#include <utils/debug.h>
#include <credentials/certificates/x509.h>
#include <credentials/sets/mem_cred.h>

#include <sys/stat.h>

typedef struct private_pts_creds_t private_pts_creds_t;

/**
 * Private data of a pts_creds_t object.
 *
 */
struct private_pts_creds_t {

	/**
	 * Public pts_creds_t interface.
	 */
	pts_creds_t public;

	/**
	 * Credential set
	 */
	mem_cred_t *creds;

};

METHOD(pts_creds_t, get_set, credential_set_t*,
	private_pts_creds_t *this)
{
	return &this->creds->set;
}


METHOD(pts_creds_t, destroy, void,
	private_pts_creds_t *this)
{
	this->creds->destroy(this->creds);
	free(this);
}

/**
 * Load trusted PTS CA certificates from a directory
 */
static void load_cacerts(private_pts_creds_t *this, char *path)
{
	enumerator_t *enumerator;
	struct stat st;
	char *file;

	DBG1(DBG_PTS, "loading PTS ca certificates from '%s'", path);

	enumerator = enumerator_create_directory(path);
	if (!enumerator)
	{
		return;
	}

	while (enumerator->enumerate(enumerator, NULL, &file, &st))
	{
		certificate_t *cert;

		if (!S_ISREG(st.st_mode))
		{
			/* skip special file */
			continue;
		}
		cert = lib->creds->create(lib->creds, CRED_CERTIFICATE, CERT_X509,
								  BUILD_FROM_FILE, file, BUILD_END);
		if (cert)
		{
			x509_t *x509 = (x509_t*)cert;

			if (!(x509->get_flags(x509) & X509_CA))
			{
				DBG1(DBG_PTS, "  ca certificate \"%Y\" lacks ca basic constraint"
							  ", discarded", cert->get_subject(cert));
				cert->destroy(cert);
			}
			else
			{
				DBG1(DBG_PTS, "  loaded ca certificate \"%Y\" from '%s'",
							  cert->get_subject(cert), file);
				this->creds->add_cert(this->creds, TRUE, cert);
			}
		}
		else
		{
			DBG1(DBG_PTS, "  loading ca certificate from '%s' failed", file);
		}
	}
	enumerator->destroy(enumerator);
}

/**
 * See header
 */
pts_creds_t *pts_creds_create(char *path)
{
	private_pts_creds_t *this;

	if (!path)
	{
		DBG1(DBG_PTS, "no PTS cacerts directory defined");
		return NULL;
	}

	INIT(this,
		.public = {
			.get_set = _get_set,
			.destroy = _destroy,
		},
		.creds = mem_cred_create(),
	);

	load_cacerts(this, path);

	return &this->public;
}

