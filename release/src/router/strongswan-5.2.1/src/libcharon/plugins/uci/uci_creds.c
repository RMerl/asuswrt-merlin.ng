/*
 * Copyright (C) 2008 Thomas Kallenberg
 * Copyright (C) 2008 Martin Willi
 * Copyright (C) 2008 Tobias Brunner
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

#include "uci_creds.h"

#include <daemon.h>
#include <credentials/keys/shared_key.h>
#include <utils/identification.h>

typedef struct private_uci_creds_t private_uci_creds_t;

/**
 * Private data of an uci_creds_t object
 */
struct private_uci_creds_t {
	/**
	 * Public part
	 */
	uci_creds_t public;

	/**
	 * UCI parser context
	 */
	uci_parser_t *parser;
};

typedef struct {
	/** implements enumerator */
	enumerator_t public;
	/** inneer UCI enumerator */
	enumerator_t *inner;
	/** currently enumerated shared shared */
	shared_key_t *current;
	/** local ID to match */
	identification_t *me;
	/** remote ID to match */
	identification_t *other;
} shared_enumerator_t;

METHOD(enumerator_t, shared_enumerator_enumerate, bool,
	shared_enumerator_t *this, shared_key_t **key, id_match_t *me,
	id_match_t *other)
{
	char *local_id, *remote_id, *psk;
	identification_t *local, *remote;

	while (TRUE)
	{
		/* defaults */
		local_id = "%any";
		remote_id = "%any";
		psk = NULL;

		if (!this->inner->enumerate(this->inner, NULL,
									&local_id, &remote_id, &psk))
		{
			return FALSE;
		}
		if (psk == NULL)
		{
			continue;
		}
		if (me)
		{
			local = identification_create_from_string(local_id);
			*me = this->me ? this->me->matches(this->me, local)
						   : ID_MATCH_ANY;
			local->destroy(local);
			if (!*me)
			{
				continue;
			}
		}
		if (other)
		{
			remote = identification_create_from_string(remote_id);
			*other = this->other ? this->other->matches(this->other, remote)
								 : ID_MATCH_ANY;
			remote->destroy(remote);
			if (!*other)
			{
				continue;
			}
		}
		break;
	}
	DESTROY_IF(this->current);
	this->current = shared_key_create(SHARED_IKE,
								chunk_clone(chunk_create(psk, strlen(psk))));
	*key = this->current;
	return TRUE;
}

METHOD(enumerator_t, shared_enumerator_destroy, void,
	shared_enumerator_t *this)
{
	this->inner->destroy(this->inner);
	DESTROY_IF(this->current);
	free(this);
}

METHOD(credential_set_t, create_shared_enumerator, enumerator_t*,
	private_uci_creds_t *this, shared_key_type_t type,
	identification_t *me, identification_t *other)
{
	shared_enumerator_t *e;

	if (type != SHARED_IKE)
	{
		return NULL;
	}

	INIT(e,
		.public = {
			.enumerate = (void*)_shared_enumerator_enumerate,
			.destroy = _shared_enumerator_destroy,
		},
		.me = me,
		.other = other,
		.inner = this->parser->create_section_enumerator(this->parser,
								"local_id", "remote_id", "psk", NULL),
	);
	if (!e->inner)
	{
		free(e);
		return NULL;
	}
	return &e->public;
}

METHOD(uci_creds_t, destroy, void,
	private_uci_creds_t *this)
{
	free(this);
}

uci_creds_t *uci_creds_create(uci_parser_t *parser)
{
	private_uci_creds_t *this;

	INIT(this,
		.public = {
			.credential_set = {
				.create_shared_enumerator = _create_shared_enumerator,
				.create_private_enumerator = (void*)return_null,
				.create_cert_enumerator = (void*)return_null,
				.create_cdp_enumerator  = (void*)return_null,
				.cache_cert = (void*)nop,
			},
			.destroy = _destroy,
		},
	);

	this->parser = parser;

	return &this->public;
}

