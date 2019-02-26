/*
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

#include "medsrv_config.h"

#include <daemon.h>

typedef struct private_medsrv_config_t private_medsrv_config_t;

/**
 * Private data of an medsrv_config_t object
 */
struct private_medsrv_config_t {

	/**
	 * Public part
	 */
	medsrv_config_t public;

	/**
	 * database connection
	 */
	database_t *db;

	/**
	 * rekey time
	 */
	int rekey;

	/**
	 * dpd delay
	 */
	int dpd;

	/**
	 * default ike config
	 */
	ike_cfg_t *ike;
};

METHOD(backend_t, get_peer_cfg_by_name, peer_cfg_t*,
	private_medsrv_config_t *this, char *name)
{
	return NULL;
}

METHOD(backend_t, create_ike_cfg_enumerator, enumerator_t*,
	private_medsrv_config_t *this, host_t *me, host_t *other)
{
	return enumerator_create_single(this->ike, NULL);
}

METHOD(backend_t, create_peer_cfg_enumerator, enumerator_t*,
	private_medsrv_config_t *this, identification_t *me,
	identification_t *other)
{
	enumerator_t *e;

	if (!me || !other || other->get_type(other) != ID_KEY_ID)
	{
		return NULL;
	}
	e = this->db->query(this->db,
			"SELECT CONCAT(peer.alias, CONCAT('@', user.login)) FROM "
			"peer JOIN user ON peer.user = user.id "
			"WHERE peer.keyid = ?", DB_BLOB, other->get_encoding(other),
			DB_TEXT);
	if (e)
	{
		peer_cfg_t *peer_cfg;
		auth_cfg_t *auth;
		char *name;

		if (e->enumerate(e, &name))
		{
			peer_cfg_create_t peer = {
				.cert_policy = CERT_NEVER_SEND,
				.unique = UNIQUE_REPLACE,
				.keyingtries = 1,
				.rekey_time = this->rekey * 60,
				.jitter_time = this->rekey * 5,
				.over_time = this->rekey * 3,
				.dpd = this->dpd,
				.mediation = TRUE,
			};
			peer_cfg = peer_cfg_create(name, this->ike->get_ref(this->ike),
									   &peer);
			e->destroy(e);

			auth = auth_cfg_create();
			auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
			auth->add(auth, AUTH_RULE_IDENTITY, me->clone(me));
			peer_cfg->add_auth_cfg(peer_cfg, auth, TRUE);
			auth = auth_cfg_create();
			auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
			auth->add(auth, AUTH_RULE_IDENTITY, other->clone(other));
			peer_cfg->add_auth_cfg(peer_cfg, auth, FALSE);

			return enumerator_create_single(peer_cfg, (void*)peer_cfg->destroy);
		}
		e->destroy(e);
	}
	return NULL;
}

METHOD(medsrv_config_t, destroy, void,
	private_medsrv_config_t *this)
{
	this->ike->destroy(this->ike);
	free(this);
}

/**
 * Described in header.
 */
medsrv_config_t *medsrv_config_create(database_t *db)
{
	private_medsrv_config_t *this;

	INIT(this,
		.public = {
			.backend = {
				.create_peer_cfg_enumerator = _create_peer_cfg_enumerator,
				.create_ike_cfg_enumerator = _create_ike_cfg_enumerator,
				.get_peer_cfg_by_name = _get_peer_cfg_by_name,
			},
			.destroy = _destroy,
		},
		.db = db,
		.rekey = lib->settings->get_time(lib->settings, "medsrv.rekey", 1200),
		.dpd = lib->settings->get_time(lib->settings, "medsrv.dpd", 300),
		.ike = ike_cfg_create(IKEV2, FALSE, FALSE, "0.0.0.0",
							  charon->socket->get_port(charon->socket, FALSE),
							  "0.0.0.0", IKEV2_UDP_PORT,
							  FRAGMENTATION_NO, 0),
	);
	this->ike->add_proposal(this->ike, proposal_create_default(PROTO_IKE));
	this->ike->add_proposal(this->ike, proposal_create_default_aead(PROTO_IKE));

	return &this->public;
}
