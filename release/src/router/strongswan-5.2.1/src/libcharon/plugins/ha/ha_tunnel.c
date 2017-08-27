/*
 * Copyright (C) 2009 Martin Willi
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

#include "ha_tunnel.h"
#include "ha_plugin.h"

#include <daemon.h>
#include <utils/identification.h>
#include <processing/jobs/callback_job.h>

typedef struct private_ha_tunnel_t private_ha_tunnel_t;
typedef struct ha_backend_t ha_backend_t;
typedef struct ha_creds_t ha_creds_t;

/**
 * Serves credentials for the HA SA
 */
struct ha_creds_t {

	/**
	 * Implements credential_set_t
	 */
	credential_set_t public;

	/**
	 * own identity
	 */
	identification_t *local;

	/**
	 * peer identity
	 */
	identification_t *remote;

	/**
	 * Shared key to serve
	 */
	shared_key_t *key;
};

/**
 * Serves configurations for the HA SA
 */
struct ha_backend_t {

	/**
	 * Implements backend_t
	 */
	backend_t public;

	/**
	 * peer config we serve
	 */
	peer_cfg_t *cfg;
};

/**
 * Private data of an ha_tunnel_t object.
 */
struct private_ha_tunnel_t {

	/**
	 * Public ha_tunnel_t interface.
	 */
	ha_tunnel_t public;

	/**
	 * Reqid of installed trap
	 */
	u_int32_t trap;

	/**
	 * backend for HA SA
	 */
	ha_backend_t backend;

	/**
	 * credential set for HA SA
	 */
	ha_creds_t creds;
};

METHOD(ha_tunnel_t, is_sa, bool,
	private_ha_tunnel_t *this, ike_sa_t *ike_sa)
{
	peer_cfg_t *cfg = this->backend.cfg;

	return cfg && ike_sa->get_ike_cfg(ike_sa) == cfg->get_ike_cfg(cfg);
}

/**
 * Enumerator over HA shared_key
 */
typedef struct {
	/** Implements enumerator_t */
	enumerator_t public;
	/** a single secret we serve */
	shared_key_t *key;
} shared_enum_t;

METHOD(enumerator_t, shared_enumerate, bool,
	shared_enum_t *this, shared_key_t **key, id_match_t *me, id_match_t *other)
{
	if (this->key)
	{
		if (me)
		{
			*me = ID_MATCH_PERFECT;
		}
		if (other)
		{
			*other = ID_MATCH_PERFECT;
		}
		*key = this->key;
		this->key = NULL;
		return TRUE;
	}
	return FALSE;
}

METHOD(ha_creds_t, create_shared_enumerator, enumerator_t*,
	ha_creds_t *this, shared_key_type_t type,
	identification_t *me, identification_t *other)
{
	shared_enum_t *enumerator;

	if (type != SHARED_IKE && type != SHARED_ANY)
	{
		return NULL;
	}
	if (me && !me->equals(me, this->local))
	{
		return NULL;
	}
	if (other && !other->equals(other, this->remote))
	{
		return NULL;
	}

	INIT(enumerator,
		.public = {
			.enumerate = (void*)_shared_enumerate,
			.destroy = (void*)free,
		},
		.key = this->key,
	);

	return &enumerator->public;
}

METHOD(backend_t, create_peer_cfg_enumerator, enumerator_t*,
	ha_backend_t *this, identification_t *me, identification_t *other)
{
	return enumerator_create_single(this->cfg, NULL);
}

METHOD(backend_t, create_ike_cfg_enumerator, enumerator_t*,
	ha_backend_t *this, host_t *me, host_t *other)
{
	return enumerator_create_single(this->cfg->get_ike_cfg(this->cfg), NULL);
}

/**
 * Install configs and a a trap for secured HA message exchange
 */
static void setup_tunnel(private_ha_tunnel_t *this,
						 char *local, char *remote, char *secret)
{
	peer_cfg_t *peer_cfg;
	ike_cfg_t *ike_cfg;
	auth_cfg_t *auth_cfg;
	child_cfg_t *child_cfg;
	traffic_selector_t *ts;
	lifetime_cfg_t lifetime = {
		.time = {
			.life = 21600, .rekey = 20400, .jitter = 400,
		},
	};

	/* setup credentials */
	this->creds.local = identification_create_from_string(local);
	this->creds.remote = identification_create_from_string(remote);
	this->creds.key = shared_key_create(SHARED_IKE,
							chunk_clone(chunk_create(secret, strlen(secret))));
	this->creds.public.create_private_enumerator = (void*)return_null;
	this->creds.public.create_cert_enumerator = (void*)return_null;
	this->creds.public.create_shared_enumerator = (void*)_create_shared_enumerator;
	this->creds.public.create_cdp_enumerator = (void*)return_null;
	this->creds.public.cache_cert = (void*)nop;

	lib->credmgr->add_set(lib->credmgr, &this->creds.public);

	/* create config and backend */
	ike_cfg = ike_cfg_create(IKEV2, FALSE, FALSE, local,
							 charon->socket->get_port(charon->socket, FALSE),
							 remote, IKEV2_UDP_PORT, FRAGMENTATION_NO, 0);
	ike_cfg->add_proposal(ike_cfg, proposal_create_default(PROTO_IKE));
	ike_cfg->add_proposal(ike_cfg, proposal_create_default_aead(PROTO_IKE));
	peer_cfg = peer_cfg_create("ha", ike_cfg, CERT_NEVER_SEND,
						UNIQUE_KEEP, 0, 86400, 0, 7200, 3600, FALSE, FALSE,
						TRUE, 30, 0, FALSE, NULL, NULL);

	auth_cfg = auth_cfg_create();
	auth_cfg->add(auth_cfg, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PSK);
	auth_cfg->add(auth_cfg, AUTH_RULE_IDENTITY,
				  identification_create_from_string(local));
	peer_cfg->add_auth_cfg(peer_cfg, auth_cfg, TRUE);

	auth_cfg = auth_cfg_create();
	auth_cfg->add(auth_cfg, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PSK);
	auth_cfg->add(auth_cfg, AUTH_RULE_IDENTITY,
				  identification_create_from_string(remote));
	peer_cfg->add_auth_cfg(peer_cfg, auth_cfg, FALSE);

	child_cfg = child_cfg_create("ha", &lifetime, NULL, TRUE, MODE_TRANSPORT,
								 ACTION_NONE, ACTION_NONE, ACTION_NONE, FALSE,
								 0, 0, NULL, NULL, 0);
	ts = traffic_selector_create_dynamic(IPPROTO_UDP, HA_PORT, HA_PORT);
	child_cfg->add_traffic_selector(child_cfg, TRUE, ts);
	ts = traffic_selector_create_dynamic(IPPROTO_ICMP, 0, 65535);
	child_cfg->add_traffic_selector(child_cfg, TRUE, ts);
	ts = traffic_selector_create_dynamic(IPPROTO_UDP, HA_PORT, HA_PORT);
	child_cfg->add_traffic_selector(child_cfg, FALSE, ts);
	ts = traffic_selector_create_dynamic(IPPROTO_ICMP, 0, 65535);
	child_cfg->add_traffic_selector(child_cfg, FALSE, ts);
	child_cfg->add_proposal(child_cfg, proposal_create_default(PROTO_ESP));
	child_cfg->add_proposal(child_cfg, proposal_create_default_aead(PROTO_ESP));
	peer_cfg->add_child_cfg(peer_cfg, child_cfg);

	this->backend.cfg = peer_cfg;
	this->backend.public.create_peer_cfg_enumerator = (void*)_create_peer_cfg_enumerator;
	this->backend.public.create_ike_cfg_enumerator = (void*)_create_ike_cfg_enumerator;
	this->backend.public.get_peer_cfg_by_name = (void*)return_null;

	charon->backends->add_backend(charon->backends, &this->backend.public);

	/* install an acquiring trap */
	this->trap = charon->traps->install(charon->traps, peer_cfg, child_cfg, 0);
}

METHOD(ha_tunnel_t, destroy, void,
	private_ha_tunnel_t *this)
{
	if (this->backend.cfg)
	{
		charon->backends->remove_backend(charon->backends, &this->backend.public);
		this->backend.cfg->destroy(this->backend.cfg);
	}
	if (this->creds.key)
	{
		lib->credmgr->remove_set(lib->credmgr, &this->creds.public);
		this->creds.key->destroy(this->creds.key);
	}
	this->creds.local->destroy(this->creds.local);
	this->creds.remote->destroy(this->creds.remote);
	if (this->trap)
	{
		charon->traps->uninstall(charon->traps, this->trap);
	}
	free(this);
}

/**
 * See header
 */
ha_tunnel_t *ha_tunnel_create(char *local, char *remote, char *secret)
{
	private_ha_tunnel_t *this;

	INIT(this,
		.public = {
			.is_sa = _is_sa,
			.destroy = _destroy,
		},
	);

	setup_tunnel(this, local, remote, secret);

	return &this->public;
}
