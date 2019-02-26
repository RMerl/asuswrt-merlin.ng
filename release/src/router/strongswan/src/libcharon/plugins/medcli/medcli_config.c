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

#define _GNU_SOURCE
#include <string.h>

#include "medcli_config.h"

#include <daemon.h>
#include <processing/jobs/callback_job.h>

typedef struct private_medcli_config_t private_medcli_config_t;

/**
 * Name of the mediation connection
 */
#define MEDIATION_CONN_NAME "medcli-mediation"

/**
 * Private data of an medcli_config_t object
 */
struct private_medcli_config_t {

	/**
	 * Public part
	 */
	medcli_config_t public;

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

/**
 * create a traffic selector from a CIDR notation string
 */
static traffic_selector_t *ts_from_string(char *str)
{
	if (str)
	{
		traffic_selector_t *ts;

		ts = traffic_selector_create_from_cidr(str, 0, 0, 65535);
		if (ts)
		{
			return ts;
		}
	}
	return traffic_selector_create_dynamic(0, 0, 65535);
}

/**
 * Build a mediation config
 */
static peer_cfg_t *build_mediation_config(private_medcli_config_t *this,
										  peer_cfg_create_t *defaults)
{
	enumerator_t *e;
	auth_cfg_t *auth;
	ike_cfg_t *ike_cfg;
	peer_cfg_t *med_cfg;
	peer_cfg_create_t peer = *defaults;
	chunk_t me, other;
	char *address;

	/* query mediation server config:
	 * - build ike_cfg/peer_cfg for mediation connection on-the-fly
	 */
	e = this->db->query(this->db,
			"SELECT Address, ClientConfig.KeyId, MediationServerConfig.KeyId "
			"FROM MediationServerConfig JOIN ClientConfig",
			DB_TEXT, DB_BLOB, DB_BLOB);
	if (!e || !e->enumerate(e, &address, &me, &other))
	{
		DESTROY_IF(e);
		return NULL;
	}
	ike_cfg = ike_cfg_create(IKEV2, FALSE, FALSE, "0.0.0.0",
							 charon->socket->get_port(charon->socket, FALSE),
							 address, IKEV2_UDP_PORT, FRAGMENTATION_NO, 0);
	ike_cfg->add_proposal(ike_cfg, proposal_create_default(PROTO_IKE));
	ike_cfg->add_proposal(ike_cfg, proposal_create_default_aead(PROTO_IKE));

	peer.mediation = TRUE;
	med_cfg = peer_cfg_create(MEDIATION_CONN_NAME, ike_cfg, &peer);
	e->destroy(e);

	auth = auth_cfg_create();
	auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
	auth->add(auth, AUTH_RULE_IDENTITY,
			  identification_create_from_encoding(ID_KEY_ID, me));
	med_cfg->add_auth_cfg(med_cfg, auth, TRUE);
	auth = auth_cfg_create();
	auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
	auth->add(auth, AUTH_RULE_IDENTITY,
			  identification_create_from_encoding(ID_KEY_ID, other));
	med_cfg->add_auth_cfg(med_cfg, auth, FALSE);
	return med_cfg;
}

METHOD(backend_t, get_peer_cfg_by_name, peer_cfg_t*,
	private_medcli_config_t *this, char *name)
{
	enumerator_t *e;
	auth_cfg_t *auth;
	peer_cfg_t *peer_cfg;
	child_cfg_t *child_cfg;
	chunk_t me, other;
	char *local_net, *remote_net;
	peer_cfg_create_t peer = {
		.cert_policy = CERT_NEVER_SEND,
		.unique = UNIQUE_REPLACE,
		.keyingtries = 1,
		.rekey_time = this->rekey * 60,
		.jitter_time = this->rekey * 5,
		.over_time = this->rekey * 3,
		.dpd = this->dpd,
	};
	child_cfg_create_t child = {
		.lifetime = {
			.time = {
				.life = this->rekey * 60 + this->rekey,
				.rekey = this->rekey,
				.jitter = this->rekey
			},
		},
		.mode = MODE_TUNNEL,
	};

	if (streq(name, "medcli-mediation"))
	{
		return build_mediation_config(this, &peer);
	}

	/* query mediated config:
	 * - use any-any ike_cfg
	 * - build peer_cfg on-the-fly using med_cfg
	 * - add a child_cfg
	 */
	e = this->db->query(this->db,
			"SELECT ClientConfig.KeyId, Connection.KeyId, "
			"Connection.LocalSubnet, Connection.RemoteSubnet "
			"FROM ClientConfig JOIN Connection "
			"WHERE Active AND Alias = ?", DB_TEXT, name,
			DB_BLOB, DB_BLOB, DB_TEXT, DB_TEXT);
	if (!e || !e->enumerate(e, &me, &other, &local_net, &remote_net))
	{
		DESTROY_IF(e);
		return NULL;
	}
	peer.mediated_by = MEDIATION_CONN_NAME;
	peer.peer_id = identification_create_from_encoding(ID_KEY_ID, other);
	peer_cfg = peer_cfg_create(name, this->ike->get_ref(this->ike), &peer);

	auth = auth_cfg_create();
	auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
	auth->add(auth, AUTH_RULE_IDENTITY,
			  identification_create_from_encoding(ID_KEY_ID, me));
	peer_cfg->add_auth_cfg(peer_cfg, auth, TRUE);
	auth = auth_cfg_create();
	auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
	auth->add(auth, AUTH_RULE_IDENTITY,
			  identification_create_from_encoding(ID_KEY_ID, other));
	peer_cfg->add_auth_cfg(peer_cfg, auth, FALSE);

	child_cfg = child_cfg_create(name, &child);
	child_cfg->add_proposal(child_cfg, proposal_create_default(PROTO_ESP));
	child_cfg->add_proposal(child_cfg, proposal_create_default_aead(PROTO_ESP));
	child_cfg->add_traffic_selector(child_cfg, TRUE, ts_from_string(local_net));
	child_cfg->add_traffic_selector(child_cfg, FALSE, ts_from_string(remote_net));
	peer_cfg->add_child_cfg(peer_cfg, child_cfg);
	e->destroy(e);
	return peer_cfg;
}

METHOD(backend_t, create_ike_cfg_enumerator, enumerator_t*,
	private_medcli_config_t *this, host_t *me, host_t *other)
{
	return enumerator_create_single(this->ike, NULL);
}

typedef struct {
	/** implements enumerator */
	enumerator_t public;
	/** inner SQL enumerator */
	enumerator_t *inner;
	/** currently enumerated peer config */
	peer_cfg_t *current;
	/** ike cfg to use in peer cfg */
	ike_cfg_t *ike;
	/** rekey time */
	int rekey;
	/** dpd time */
	int dpd;
} peer_enumerator_t;

METHOD(enumerator_t, peer_enumerator_enumerate, bool,
	peer_enumerator_t *this, va_list args)
{
	char *name, *local_net, *remote_net;
	chunk_t me, other;
	peer_cfg_t **cfg;
	child_cfg_t *child_cfg;
	auth_cfg_t *auth;
	peer_cfg_create_t peer = {
		.cert_policy = CERT_NEVER_SEND,
		.unique = UNIQUE_REPLACE,
		.keyingtries = 1,
		.rekey_time = this->rekey * 60,
		.jitter_time = this->rekey * 5,
		.over_time = this->rekey * 3,
		.dpd = this->dpd,
	};
	child_cfg_create_t child = {
		.lifetime = {
			.time = {
				.life = this->rekey * 60 + this->rekey,
				.rekey = this->rekey,
				.jitter = this->rekey
			},
		},
		.mode = MODE_TUNNEL,
	};

	VA_ARGS_VGET(args, cfg);

	DESTROY_IF(this->current);
	if (!this->inner->enumerate(this->inner, &name, &me, &other,
								&local_net, &remote_net))
	{
		this->current = NULL;
		return FALSE;
	}
	this->current = peer_cfg_create(name, this->ike->get_ref(this->ike), &peer);

	auth = auth_cfg_create();
	auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
	auth->add(auth, AUTH_RULE_IDENTITY,
			  identification_create_from_encoding(ID_KEY_ID, me));
	this->current->add_auth_cfg(this->current, auth, TRUE);
	auth = auth_cfg_create();
	auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
	auth->add(auth, AUTH_RULE_IDENTITY,
			  identification_create_from_encoding(ID_KEY_ID, other));
	this->current->add_auth_cfg(this->current, auth, FALSE);

	child_cfg = child_cfg_create(name, &child);
	child_cfg->add_proposal(child_cfg, proposal_create_default(PROTO_ESP));
	child_cfg->add_proposal(child_cfg, proposal_create_default_aead(PROTO_ESP));
	child_cfg->add_traffic_selector(child_cfg, TRUE, ts_from_string(local_net));
	child_cfg->add_traffic_selector(child_cfg, FALSE, ts_from_string(remote_net));
	this->current->add_child_cfg(this->current, child_cfg);
	*cfg = this->current;
	return TRUE;
}

METHOD(enumerator_t, peer_enumerator_destroy, void,
	peer_enumerator_t *this)
{
	DESTROY_IF(this->current);
	this->inner->destroy(this->inner);
	free(this);
}

METHOD(backend_t, create_peer_cfg_enumerator, enumerator_t*,
	private_medcli_config_t *this, identification_t *me,
	identification_t *other)
{
	peer_enumerator_t *e;

	INIT(e,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _peer_enumerator_enumerate,
			.destroy = _peer_enumerator_destroy,
		},
		.ike = this->ike,
		.rekey = this->rekey,
		.dpd = this->dpd,
	);

	/* filter on IDs: NULL or ANY or matching KEY_ID */
	e->inner = this->db->query(this->db,
			"SELECT Alias, ClientConfig.KeyId, Connection.KeyId, "
			"Connection.LocalSubnet, Connection.RemoteSubnet "
			"FROM ClientConfig JOIN Connection "
			"WHERE Active AND "
			"(? OR ClientConfig.KeyId = ?) AND (? OR Connection.KeyId = ?)",
			DB_INT, me == NULL || me->get_type(me) == ID_ANY,
			DB_BLOB, me && me->get_type(me) == ID_KEY_ID ?
				me->get_encoding(me) : chunk_empty,
			DB_INT, other == NULL || other->get_type(other) == ID_ANY,
			DB_BLOB, other && other->get_type(other) == ID_KEY_ID ?
				other->get_encoding(other) : chunk_empty,
			DB_TEXT, DB_BLOB, DB_BLOB, DB_TEXT, DB_TEXT);
	if (!e->inner)
	{
		free(e);
		return NULL;
	}
	return &e->public;
}

/**
 * initiate a peer config
 */
static job_requeue_t initiate_config(peer_cfg_t *peer_cfg)
{
	enumerator_t *enumerator;
	child_cfg_t *child_cfg = NULL;;

	enumerator = peer_cfg->create_child_cfg_enumerator(peer_cfg);
	enumerator->enumerate(enumerator, &child_cfg);
	if (child_cfg)
	{
		child_cfg->get_ref(child_cfg);
		peer_cfg->get_ref(peer_cfg);
		enumerator->destroy(enumerator);
		charon->controller->initiate(charon->controller,
									 peer_cfg, child_cfg, NULL, NULL, 0, FALSE);
	}
	else
	{
		enumerator->destroy(enumerator);
	}
	return JOB_REQUEUE_NONE;
}

/**
 * schedule initiation of all "active" connections
 */
static void schedule_autoinit(private_medcli_config_t *this)
{
	enumerator_t *e;
	char *name;

	e = this->db->query(this->db, "SELECT Alias FROM Connection WHERE Active",
						DB_TEXT);
	if (e)
	{
		while (e->enumerate(e, &name))
		{
			peer_cfg_t *peer_cfg;

			peer_cfg = get_peer_cfg_by_name(this, name);
			if (peer_cfg)
			{
				/* schedule asynchronous initiation job */
				lib->processor->queue_job(lib->processor,
						(job_t*)callback_job_create(
									(callback_job_cb_t)initiate_config,
									peer_cfg, (void*)peer_cfg->destroy, NULL));
			}
		}
		e->destroy(e);
	}
}

METHOD(medcli_config_t, destroy, void,
	private_medcli_config_t *this)
{
	this->ike->destroy(this->ike);
	free(this);
}

/**
 * Described in header.
 */
medcli_config_t *medcli_config_create(database_t *db)
{
	private_medcli_config_t *this;

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
		.rekey = lib->settings->get_time(lib->settings, "medcli.rekey", 1200),
		.dpd = lib->settings->get_time(lib->settings, "medcli.dpd", 300),
		.ike = ike_cfg_create(IKEV2, FALSE, FALSE, "0.0.0.0",
							  charon->socket->get_port(charon->socket, FALSE),
							  "0.0.0.0", IKEV2_UDP_PORT,
							  FRAGMENTATION_NO, 0),
	);
	this->ike->add_proposal(this->ike, proposal_create_default(PROTO_IKE));
	this->ike->add_proposal(this->ike, proposal_create_default_aead(PROTO_IKE));

	schedule_autoinit(this);

	return &this->public;
}
