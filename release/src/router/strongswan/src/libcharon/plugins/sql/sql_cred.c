/*
 * Copyright (C) 2010 Tobias Brunner
 * Copyright (C) 2008 Martin Willi
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

#include <string.h>

#include "sql_cred.h"

#include <daemon.h>

typedef struct private_sql_cred_t private_sql_cred_t;

/**
 * Private data of an sql_cred_t object
 */
struct private_sql_cred_t {

	/**
	 * Public part
	 */
	sql_cred_t public;

	/**
	 * database connection
	 */
	database_t *db;
};


/**
 * enumerator over private keys
 */
typedef struct {
	/** implements enumerator */
	enumerator_t public;
	/** inner SQL enumerator */
	enumerator_t *inner;
	/** currently enumerated private key */
	private_key_t *current;
} private_enumerator_t;

METHOD(enumerator_t, private_enumerator_enumerate, bool,
	   private_enumerator_t *this, va_list args)
{
	private_key_t **key;
	chunk_t blob;
	int type;

	VA_ARGS_VGET(args, key);

	DESTROY_IF(this->current);
	while (this->inner->enumerate(this->inner, &type, &blob))
	{
		this->current = lib->creds->create(lib->creds, CRED_PRIVATE_KEY, type,
										   BUILD_BLOB_PEM, blob,
										   BUILD_END);
		if (this->current)
		{
			*key = this->current;
			return TRUE;
		}
	}
	this->current = NULL;
	return FALSE;
}

METHOD(enumerator_t, private_enumerator_destroy, void,
	   private_enumerator_t *this)
{
	DESTROY_IF(this->current);
	this->inner->destroy(this->inner);
	free(this);
}

METHOD(credential_set_t, create_private_enumerator, enumerator_t*,
	   private_sql_cred_t *this, key_type_t type, identification_t *id)
{
	private_enumerator_t *e;

	INIT(e,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _private_enumerator_enumerate,
			.destroy = _private_enumerator_destroy,
		},
	);
	if (id && id->get_type(id) != ID_ANY)
	{
		e->inner = this->db->query(this->db,
				"SELECT p.type, p.data FROM private_keys AS p "
				"JOIN private_key_identity AS pi ON p.id = pi.private_key "
				"JOIN identities AS i ON pi.identity = i.id "
				"WHERE i.type = ? AND i.data = ? AND (? OR p.type = ?)",
				DB_INT, id->get_type(id), DB_BLOB, id->get_encoding(id),
				DB_INT, type == KEY_ANY, DB_INT, type,
				DB_INT, DB_BLOB);
	}
	else
	{
		e->inner = this->db->query(this->db,
				"SELECT p.type, p.data FROM private_keys AS p "
				"WHERE (? OR p.type = ?)",
				DB_INT, type == KEY_ANY, DB_INT, type,
				DB_INT, DB_BLOB);
	}
	if (!e->inner)
	{
		free(e);
		return NULL;
	}
	return &e->public;
}


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
} cert_enumerator_t;

METHOD(enumerator_t, cert_enumerator_enumerate, bool,
	   cert_enumerator_t *this, va_list args)
{
	certificate_t **cert;
	chunk_t blob;
	int type;

	VA_ARGS_VGET(args, cert);

	DESTROY_IF(this->current);
	while (this->inner->enumerate(this->inner, &type, &blob))
	{
		this->current = lib->creds->create(lib->creds, CRED_CERTIFICATE, type,
										   BUILD_BLOB_PEM, blob,
										   BUILD_END);
		if (this->current)
		{
			*cert = this->current;
			return TRUE;
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
	   private_sql_cred_t *this, certificate_type_t cert, key_type_t key,
	   identification_t *id, bool trusted)
{
	cert_enumerator_t *e;

	INIT(e,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _cert_enumerator_enumerate,
			.destroy = _cert_enumerator_destroy,
		},
	);
	if (id && id->get_type(id) != ID_ANY)
	{
		e->inner = this->db->query(this->db,
				"SELECT c.type, c.data FROM certificates AS c "
				"JOIN certificate_identity AS ci ON c.id = ci.certificate "
				"JOIN identities AS i ON ci.identity = i.id "
				"WHERE i.type = ? AND i.data = ? AND "
				"(? OR c.type = ?) AND (? OR c.keytype = ?)",
				DB_INT, id->get_type(id), DB_BLOB, id->get_encoding(id),
				DB_INT, cert == CERT_ANY, DB_INT, cert,
				DB_INT, key == KEY_ANY, DB_INT, key,
				DB_INT, DB_BLOB);
	}
	else
	{
		e->inner = this->db->query(this->db,
				"SELECT c.type, c.data FROM certificates AS c WHERE "
				"(? OR c.type = ?) AND (? OR c.keytype = ?)",
				DB_INT, cert == CERT_ANY, DB_INT, cert,
				DB_INT, key == KEY_ANY, DB_INT, key,
				DB_INT, DB_BLOB);
	}
	if (!e->inner)
	{
		free(e);
		return NULL;
	}
	return &e->public;
}


/**
 * enumerator over shared keys
 */
typedef struct {
	/** implements enumerator */
	enumerator_t public;
	/** inner SQL enumerator */
	enumerator_t *inner;
	/** own identity */
	identification_t *me;
	/** remote identity */
	identification_t *other;
	/** currently enumerated private key */
	shared_key_t *current;
} shared_enumerator_t;

METHOD(enumerator_t, shared_enumerator_enumerate, bool,
	   shared_enumerator_t *this, va_list args)
{
	shared_key_t **shared;
	id_match_t *me, *other;
	chunk_t blob;
	int type;

	VA_ARGS_VGET(args, shared, me, other);

	DESTROY_IF(this->current);
	while (this->inner->enumerate(this->inner, &type, &blob))
	{
		this->current = shared_key_create(type, chunk_clone(blob));
		if (this->current)
		{
			*shared = this->current;
			if (me)
			{
				*me = this->me ? ID_MATCH_PERFECT : ID_MATCH_ANY;
			}
			if (other)
			{
				*other = this->other ? ID_MATCH_PERFECT : ID_MATCH_ANY;
			}
			return TRUE;
		}
	}
	this->current = NULL;
	return FALSE;
}

METHOD(enumerator_t, shared_enumerator_destroy, void,
	   shared_enumerator_t *this)
{
	DESTROY_IF(this->current);
	this->inner->destroy(this->inner);
	free(this);
}

METHOD(credential_set_t, create_shared_enumerator, enumerator_t*,
	   private_sql_cred_t *this, shared_key_type_t type,
	   identification_t *me, identification_t *other)
{
	shared_enumerator_t *e;

	INIT(e,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _shared_enumerator_enumerate,
			.destroy = _shared_enumerator_destroy,
		},
		.me = me,
		.other = other,
	);
	if (!me && !other)
	{
		e->inner = this->db->query(this->db,
				"SELECT s.type, s.data FROM shared_secrets AS s "
				"WHERE (? OR s.type = ?)",
				DB_INT, type == SHARED_ANY, DB_INT, type,
				DB_INT, DB_BLOB);
	}
	else if (me && other)
	{
		e->inner = this->db->query(this->db,
				"SELECT s.type, s.data FROM shared_secrets AS s "
				"JOIN shared_secret_identity AS sm ON s.id = sm.shared_secret "
				"JOIN identities AS m ON sm.identity = m.id "
				"JOIN shared_secret_identity AS so ON s.id = so.shared_secret "
				"JOIN identities AS o ON so.identity = o.id "
				"WHERE m.type = ? AND m.data = ? AND o.type = ? AND o.data = ? "
				"AND (? OR s.type = ?)",
				DB_INT, me->get_type(me), DB_BLOB, me->get_encoding(me),
				DB_INT, other->get_type(other), DB_BLOB, other->get_encoding(other),
				DB_INT, type == SHARED_ANY, DB_INT, type,
				DB_INT, DB_BLOB);
	}
	else
	{
		identification_t *id = me ? me : other;

		e->inner = this->db->query(this->db,
				"SELECT s.type, s.data FROM shared_secrets AS s "
				"JOIN shared_secret_identity AS si ON s.id = si.shared_secret "
				"JOIN identities AS i ON si.identity = i.id "
				"WHERE i.type = ? AND i.data = ? AND (? OR s.type = ?)",
				DB_INT, id->get_type(id), DB_BLOB, id->get_encoding(id),
				DB_INT, type == SHARED_ANY, DB_INT, type,
				DB_INT, DB_BLOB);
	}
	if (!e->inner)
	{
		free(e);
		return NULL;
	}
	return &e->public;
}


/**
 * enumerator over CDPs
 */
typedef struct {
	/** implements enumerator_t */
	enumerator_t public;
	/** inner SQL enumerator */
	enumerator_t *inner;
	/** currently enumerated string */
	char *current;
} cdp_enumerator_t;

/**
 * types of CDPs
 */
typedef enum {
	/** any available CDP */
	CDP_TYPE_ANY = 0,
	/** CRL */
	CDP_TYPE_CRL,
	/** OCSP Responder */
	CDP_TYPE_OCSP,
} cdp_type_t;

METHOD(enumerator_t, cdp_enumerator_enumerate, bool,
	   cdp_enumerator_t *this, va_list args)
{
	char *text, **uri;

	VA_ARGS_VGET(args, uri);

	free(this->current);
	while (this->inner->enumerate(this->inner, &text))
	{
		*uri = this->current = strdup(text);
		return TRUE;
	}
	this->current = NULL;
	return FALSE;
}

METHOD(enumerator_t, cdp_enumerator_destroy, void,
	   cdp_enumerator_t *this)
{
	free(this->current);
	this->inner->destroy(this->inner);
	free(this);
}

METHOD(credential_set_t, create_cdp_enumerator, enumerator_t*,
	   private_sql_cred_t *this, certificate_type_t type, identification_t *id)
{
	cdp_enumerator_t *e;
	cdp_type_t cdp_type;

	switch (type)
	{	/* we serve CRLs and OCSP responders */
		case CERT_X509_CRL:
			cdp_type = CDP_TYPE_CRL;
			break;
		case CERT_X509_OCSP_RESPONSE:
			cdp_type = CDP_TYPE_OCSP;
			break;
		case CERT_ANY:
			cdp_type = CDP_TYPE_ANY;
			break;
		default:
			return NULL;
	}
	INIT(e,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _cdp_enumerator_enumerate,
			.destroy = _cdp_enumerator_destroy,
		},
	);
	if (id && id->get_type(id) != ID_ANY)
	{
		e->inner = this->db->query(this->db,
				"SELECT dp.uri FROM certificate_distribution_points AS dp "
				"JOIN certificate_authorities AS ca ON ca.id = dp.ca "
				"JOIN certificates AS c ON c.id = ca.certificate "
				"JOIN certificate_identity AS ci ON c.id = ci.certificate "
				"JOIN identities AS i ON ci.identity = i.id "
				"WHERE i.type = ? AND i.data = ? AND (? OR dp.type = ?)",
				DB_INT, id->get_type(id), DB_BLOB, id->get_encoding(id),
				DB_INT, cdp_type == CDP_TYPE_ANY, DB_INT, cdp_type,
				DB_TEXT);
	}
	else
	{
		e->inner = this->db->query(this->db,
				"SELECT dp.uri FROM certificate_distribution_points AS dp "
				"WHERE (? OR dp.type = ?)",
				DB_INT, cdp_type == CDP_TYPE_ANY, DB_INT, cdp_type,
				DB_TEXT);
	}
	if (!e->inner)
	{
		free(e);
		return NULL;
	}
	return &e->public;
}

METHOD(credential_set_t, cache_cert, void,
	   private_sql_cred_t *this, certificate_t *cert)
{
	/* TODO: implement CRL caching to database */
}

METHOD(sql_cred_t, destroy, void,
	   private_sql_cred_t *this)
{
	free(this);
}

/**
 * Described in header.
 */
sql_cred_t *sql_cred_create(database_t *db)
{
	private_sql_cred_t *this;

	INIT(this,
		.public = {
			.set = {
				.create_private_enumerator = _create_private_enumerator,
				.create_cert_enumerator = _create_cert_enumerator,
				.create_shared_enumerator = _create_shared_enumerator,
				.create_cdp_enumerator = _create_cdp_enumerator,
				.cache_cert = _cache_cert,
			},
			.destroy = _destroy,
		},
		.db = db,
	);

	return &this->public;
}

