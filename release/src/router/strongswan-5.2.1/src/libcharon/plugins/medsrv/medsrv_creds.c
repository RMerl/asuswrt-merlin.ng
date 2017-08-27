/*
 * Copyright (C) 2008 Martin Willi
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

#include "medsrv_creds.h"

#include <daemon.h>
#include <library.h>
#include <collections/enumerator.h>

typedef struct private_medsrv_creds_t private_medsrv_creds_t;

/**
 * Private data of an medsrv_creds_t object
 */
struct private_medsrv_creds_t {

	/**
	 * Public part
	 */
	medsrv_creds_t public;

	/**
	 * underlying database handle
	 */
	database_t *db;
};

/**
 * enumerator over certificates
 */
typedef struct {
	/** implements enumerator */
	enumerator_t public;
	/** inner SQL enumerator */
	enumerator_t *inner;
	/** currently enumerated cert */
	certificate_t *current;
	/** type of requested key */
	key_type_t type;
} cert_enumerator_t;

METHOD(enumerator_t, cert_enumerator_enumerate, bool,
	cert_enumerator_t *this, certificate_t **cert)
{
	certificate_t *trusted;
	public_key_t *public;
	chunk_t chunk;

	DESTROY_IF(this->current);
	while (this->inner->enumerate(this->inner, &chunk))
	{
		public = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_ANY,
									BUILD_BLOB_ASN1_DER, chunk,
									BUILD_END);
		if (public)
		{
			if (this->type == KEY_ANY || this->type == public->get_type(public))
			{
				trusted = lib->creds->create(lib->creds,
										CRED_CERTIFICATE, CERT_TRUSTED_PUBKEY,
										BUILD_PUBLIC_KEY, public, BUILD_END);
				public->destroy(public);
				if (trusted)
				{
					*cert = this->current = trusted;
					return TRUE;
				}
			}
			else
			{
				public->destroy(public);
			}
		}
	}
	this->current = NULL;
	return FALSE;
}

METHOD(enumerator_t, cert_enumerator_destroy, void,
	cert_enumerator_t *this)
{
	DESTROY_IF(this->current);
	this->inner->destroy(this->inner);
	free(this);
}

METHOD(credential_set_t, create_cert_enumerator, enumerator_t*,
	private_medsrv_creds_t *this, certificate_type_t cert, key_type_t key,
	identification_t *id, bool trusted)
{
	cert_enumerator_t *e;

	if ((cert != CERT_TRUSTED_PUBKEY && cert != CERT_ANY) ||
		id == NULL || id->get_type(id) != ID_KEY_ID)
	{
		return NULL;
	}

	INIT(e,
		.public = {
			.enumerate = (void*)_cert_enumerator_enumerate,
			.destroy = _cert_enumerator_destroy,
		},
		.type = key,
		.inner = this->db->query(this->db,
								 "SELECT public_key FROM peer WHERE keyid = ?",
								 DB_BLOB, id->get_encoding(id),
								 DB_BLOB),
	);
	if (!e->inner)
	{
		free(e);
		return NULL;
	}
	return &e->public;
}

METHOD(medsrv_creds_t, destroy, void,
	private_medsrv_creds_t *this)
{
	free(this);
}

/**
 * Described in header.
 */
medsrv_creds_t *medsrv_creds_create(database_t *db)
{
	private_medsrv_creds_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_private_enumerator = (void*)return_null,
				.create_cert_enumerator = _create_cert_enumerator,
				.create_shared_enumerator = (void*)return_null,
				.create_cdp_enumerator = (void*)return_null,
				.cache_cert = (void*)nop,
			},
			.destroy = _destroy,
		},
		.db = db,
	);

	return &this->public;
}

