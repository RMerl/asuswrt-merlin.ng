/*
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2008 Philip Boetschi, Adrian Doerig
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

#define _GNU_SOURCE
#include <string.h>

#include "peer_controller.h"

#include <library.h>
#include <utils/debug.h>
#include <asn1/asn1.h>
#include <asn1/oid.h>
#include <utils/identification.h>
#include <credentials/keys/public_key.h>

typedef struct private_peer_controller_t private_peer_controller_t;

/**
 * private data of the peer_controller
 */
struct private_peer_controller_t {

	/**
	 * public functions
	 */
	peer_controller_t public;

	/**
	 * active user session
	 */
	user_t *user;

	/**
	 * underlying database
	 */
	database_t *db;
};

/**
 * list the configured peer configs
 */
static void list(private_peer_controller_t *this, fast_request_t *request)
{
	enumerator_t *query;

	query = this->db->query(this->db,
			"SELECT id, alias, keyid FROM peer WHERE user = ? ORDER BY alias",
			DB_UINT, this->user->get_user(this->user),
			DB_UINT, DB_TEXT, DB_BLOB);

	if (query)
	{
		u_int id;
		char *alias;
		chunk_t keyid;
		identification_t *identifier;

		while (query->enumerate(query, &id, &alias, &keyid))
		{
			request->setf(request, "peers.%d.alias=%s", id, alias);
			identifier = identification_create_from_encoding(ID_KEY_ID, keyid);
			request->setf(request, "peers.%d.identifier=%Y", id, identifier);
			identifier->destroy(identifier);
		}
		query->destroy(query);
	}
	request->render(request, "templates/peer/list.cs");
}

/**
 * verify a peer alias
 */
static bool verify_alias(private_peer_controller_t *this, fast_request_t *request,
						 char *alias)
{
	if (!alias || *alias == '\0')
	{
		request->setf(request, "error=Alias is missing.");
		return FALSE;
	}
	while (*alias != '\0')
	{
		switch (*alias)
		{
			case 'a' ... 'z':
			case 'A' ... 'Z':
			case '0' ... '9':
			case '-':
			case '_':
			case '@':
			case '.':
				alias++;
				continue;
			default:
				request->setf(request, "error=Alias invalid, "
							  "valid characters: A-Z a-z 0-9 - _ @ .");
				return FALSE;
		}
	}
	return TRUE;
}

/**
 * parse and verify a public key
 */
static bool parse_public_key(private_peer_controller_t *this,
							 fast_request_t *request, char *public_key,
							 chunk_t *encoding, chunk_t *keyid)
{
	public_key_t *public;
	chunk_t blob, id;

	if (!public_key || *public_key == '\0')
	{
		request->setf(request, "error=Public key is missing.");
		return FALSE;
	}
	blob = chunk_clone(chunk_create(public_key, strlen(public_key)));
	public = lib->creds->create(lib->creds, CRED_PUBLIC_KEY, KEY_ANY,
								BUILD_BLOB_PEM, blob,
								BUILD_END);
	chunk_free(&blob);
	if (!public)
	{
		request->setf(request, "error=Parsing public key failed.");
		return FALSE;
	}
	/* TODO: use get_encoding() with an encoding type */
	if (!public->get_fingerprint(public, KEYID_PUBKEY_SHA1, &id) ||
		!public->get_encoding(public, PUBKEY_SPKI_ASN1_DER, encoding))
	{
		request->setf(request, "error=Encoding public key failed.");
		return FALSE;
	}
	*keyid = chunk_clone(id);
	public->destroy(public);
	return TRUE;
}

/**
 * register a new peer
 */
static void add(private_peer_controller_t *this, fast_request_t *request)
{
	char *alias = "", *public_key = "";

	if (request->get_query_data(request, "back"))
	{
		return request->redirect(request, "peer/list");
	}
	while (request->get_query_data(request, "add"))
	{
		chunk_t encoding, keyid;

		alias = request->get_query_data(request, "alias");
		public_key = request->get_query_data(request, "public_key");

		if (!verify_alias(this, request, alias))
		{
			break;
		}
		if (!parse_public_key(this, request, public_key, &encoding, &keyid))
		{
			break;
		}
		if (this->db->execute(this->db, NULL,
						  "INSERT INTO peer (user, alias, public_key, keyid) "
						  "VALUES (?, ?, ?, ?)",
						  DB_UINT, this->user->get_user(this->user),
						  DB_TEXT, alias, DB_BLOB, encoding,
						  DB_BLOB, keyid) <= 0)
		{
			request->setf(request, "error=Peer already exists.");
			free(keyid.ptr);
			free(encoding.ptr);
			break;
		}
		free(keyid.ptr);
		free(encoding.ptr);
		return request->redirect(request, "peer/list");
	}
	request->set(request, "alias", alias);
	request->set(request, "public_key", public_key);

	return request->render(request, "templates/peer/add.cs");
}

/**
 * pem encode a public key into an allocated string
 */
char* pem_encode(chunk_t der)
{
	static const char *begin = "-----BEGIN PUBLIC KEY-----\n";
	static const char *end = "-----END PUBLIC KEY-----";
	size_t len;
	char *pem;
	chunk_t base64;
	int i = 0;

	base64 = chunk_to_base64(der, NULL);
	len = strlen(begin) + base64.len + base64.len/64 + strlen(end) + 2;
	pem = malloc(len + 1);

	strcpy(pem, begin);
	do
	{
		strncat(pem, base64.ptr + i, 64);
		strcat(pem, "\n");
		i += 64;
	}
	while (i < base64.len - 2);
	strcat(pem, end);

	free(base64.ptr);
	return pem;
}

/**
 * edit a peer
 */
static void edit(private_peer_controller_t *this, fast_request_t *request, int id)
{
	char *alias = "", *public_key = "", *pem;
	chunk_t encoding, keyid;

	if (request->get_query_data(request, "back"))
	{
		return request->redirect(request, "peer/list");
	}
	if (request->get_query_data(request, "delete"))
	{
		this->db->execute(this->db, NULL,
						  "DELETE FROM peer WHERE id = ? AND user = ?",
						  DB_INT, id, DB_UINT, this->user->get_user(this->user));
		return request->redirect(request, "peer/list");
	}
	if (request->get_query_data(request, "save"))
	{
		while (TRUE)
		{
			alias = request->get_query_data(request, "alias");
			public_key = request->get_query_data(request, "public_key");

			if (!verify_alias(this, request, alias))
			{
				break;
			}
			if (!parse_public_key(this, request, public_key, &encoding, &keyid))
			{
				break;
			}
			if (this->db->execute(this->db, NULL,
				  "UPDATE peer SET alias = ?, public_key = ?, keyid = ? "
				  "WHERE id = ? AND user = ?",
				  DB_TEXT, alias, DB_BLOB, encoding, DB_BLOB, keyid,
				  DB_INT, id, DB_UINT, this->user->get_user(this->user)) < 0)
			{
				request->setf(request, "error=Peer already exists.");
				free(keyid.ptr);
				free(encoding.ptr);
				break;
			}
			free(keyid.ptr);
			free(encoding.ptr);
			return request->redirect(request, "peer/list");
		}
	}
	else
	{
		enumerator_t *query = this->db->query(this->db,
				"SELECT alias, public_key FROM peer WHERE id = ? AND user = ?",
				DB_INT, id, DB_UINT, this->user->get_user(this->user),
				DB_TEXT, DB_BLOB);
		if (query && query->enumerate(query, &alias, &encoding))
		{
			alias = strdupa(alias);
			pem = pem_encode(encoding);
			public_key = strdupa(pem);
			free(pem);
		}
		else
		{
			return request->redirect(request, "peer/list");
		}
		DESTROY_IF(query);
	}
	request->set(request, "alias", alias);
	request->set(request, "public_key", public_key);
	return request->render(request, "templates/peer/edit.cs");
}

/**
 * delete a peer from the database
 */
static void delete(private_peer_controller_t *this, fast_request_t *request, int id)
{
	this->db->execute(this->db, NULL,
					  "DELETE FROM peer WHERE id = ? AND user = ?",
					  DB_INT, id, DB_UINT, this->user->get_user(this->user));
}

METHOD(fast_controller_t, get_name, char*,
	private_peer_controller_t *this)
{
	return "peer";
}

METHOD(fast_controller_t, handle, void,
	private_peer_controller_t *this, fast_request_t *request, char *action,
	char *idstr, char *p3, char *p4, char *p5)
{
	if (action)
	{
		int id = 0;
		if (idstr)
		{
			id = atoi(idstr);
		}

		if (streq(action, "list"))
		{
			return list(this, request);
		}
		else if (streq(action, "add"))
		{
			return add(this, request);
		}
		else if (streq(action, "edit") && id)
		{
			return edit(this, request, id);
		}
		else if (streq(action, "delete") && id)
		{
			delete(this, request, id);
		}
	}
	request->redirect(request, "peer/list");
}

METHOD(fast_controller_t, destroy, void,
	private_peer_controller_t *this)
{
	free(this);
}

/*
 * see header file
 */
fast_controller_t *peer_controller_create(user_t *user, database_t *db)
{
	private_peer_controller_t *this;

	INIT(this,
		.public = {
			.controller = {
				.get_name = _get_name,
				.handle = _handle,
				.destroy = _destroy,
			},
		},
		.user = user,
		.db = db,
	);

	return &this->public.controller;
}
