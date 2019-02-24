/*
 * Copyright (C) 2006-2008 Martin Willi
 * Copyright (C) 2010 Andreas Steffen
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

#include "sql_config.h"

#include <daemon.h>

typedef struct private_sql_config_t private_sql_config_t;

/**
 * Private data of an sql_config_t object
 */
struct private_sql_config_t {

	/**
	 * Public part
	 */
	sql_config_t public;

	/**
	 * database connection
	 */
	database_t *db;
};

/**
 * Forward declaration
 */
static peer_cfg_t *build_peer_cfg(private_sql_config_t *this, enumerator_t *e,
								  identification_t *me, identification_t *other);

/**
 * Build a traffic selector from an SQL query
 */
static traffic_selector_t *build_traffic_selector(private_sql_config_t *this,
												  enumerator_t *e, bool *local)
{
	int type, protocol, start_port, end_port;
	chunk_t start_addr, end_addr;
	traffic_selector_t *ts;
	enum {
		TS_LOCAL = 0,
		TS_REMOTE = 1,
		TS_LOCAL_DYNAMIC = 2,
		TS_REMOTE_DYNAMIC = 3,
	} kind;

	while (e->enumerate(e, &kind, &type, &protocol,
						&start_addr, &end_addr, &start_port, &end_port))
	{
		*local = FALSE;
		switch (kind)
		{
			case TS_LOCAL:
				*local = TRUE;
				/* FALL */
			case TS_REMOTE:
				ts = traffic_selector_create_from_bytes(protocol, type,
								start_addr, start_port,	end_addr, end_port);
				break;
			case TS_LOCAL_DYNAMIC:
				*local = TRUE;
				/* FALL */
			case TS_REMOTE_DYNAMIC:
				ts = traffic_selector_create_dynamic(protocol,
								start_port, end_port);
				break;
			default:
				continue;
		}
		if (ts)
		{
			return ts;
		}
	}
	return NULL;
}

/**
 * Add traffic selectors to a child config
 */
static void add_traffic_selectors(private_sql_config_t *this,
								  child_cfg_t *child, int id)
{
	enumerator_t *e;
	traffic_selector_t *ts;
	bool local;

	e = this->db->query(this->db,
			"SELECT ct.kind, t.type, t.protocol, "
			"t.start_addr, t.end_addr, t.start_port, t.end_port "
			"FROM traffic_selectors AS t "
			"JOIN child_config_traffic_selector AS ct "
			"ON t.id = ct.traffic_selector WHERE ct.child_cfg = ?",
			DB_INT, id,
			DB_INT, DB_INT, DB_INT,
			DB_BLOB, DB_BLOB, DB_INT, DB_INT);
	if (e)
	{
		while ((ts = build_traffic_selector(this, e, &local)))
		{
			child->add_traffic_selector(child, local, ts);
		}
		e->destroy(e);
	}
}

/**
 * Add ESP proposals to a child config
 */
static void add_esp_proposals(private_sql_config_t *this,
							  child_cfg_t *child, int id)
{
	enumerator_t *e;
	proposal_t *proposal;
	char *prop;
	bool use_default = TRUE;

	e = this->db->query(this->db,
			"SELECT p.proposal "
			"FROM proposals AS p JOIN child_config_proposal AS cp "
			"ON p.id = cp.prop WHERE cp.child_cfg = ? ORDER BY cp.prio",
			DB_INT, id, DB_TEXT);
	if (e)
	{
		while (e->enumerate(e, &prop))
		{
			proposal = proposal_create_from_string(PROTO_ESP, prop);
			if (!proposal)
			{
				DBG1(DBG_CFG, "could not create ESP proposal from '%s'", prop);
				break;
			}
			child->add_proposal(child, proposal);
			use_default = FALSE;
		}
		e->destroy(e);
	}
	if (use_default)
	{
		child->add_proposal(child, proposal_create_default(PROTO_ESP));
		child->add_proposal(child, proposal_create_default_aead(PROTO_ESP));
	}
}

/**
 * Build a child config from an SQL query
 */
static child_cfg_t *build_child_cfg(private_sql_config_t *this, enumerator_t *e)
{
	int id, lifetime, rekeytime, jitter, hostaccess, mode, ipcomp, reqid;
	int start, dpd, close;
	char *name, *updown;
	child_cfg_t *child_cfg;

	if (e->enumerate(e, &id, &name, &lifetime, &rekeytime, &jitter, &updown,
						&hostaccess, &mode, &start, &dpd, &close, &ipcomp, &reqid))
	{
		child_cfg_create_t child = {
			.mode = mode,
			.reqid = reqid,
			.options = (ipcomp ? OPT_IPCOMP : 0) |
					   (hostaccess ? OPT_HOSTACCESS : 0),
			.lifetime = {
				.time = {
					.life = lifetime, .rekey = rekeytime, .jitter = jitter
				},
			},
			.start_action = start,
			.dpd_action = dpd,
			.close_action = close,
			.updown = updown,
		};
		child_cfg = child_cfg_create(name, &child);
		add_esp_proposals(this, child_cfg, id);
		add_traffic_selectors(this, child_cfg, id);
		return child_cfg;
	}
	return NULL;
}

/**
 * Add child configs to peer config
 */
static void add_child_cfgs(private_sql_config_t *this, peer_cfg_t *peer, int id)
{
	enumerator_t *e;
	child_cfg_t *child_cfg;

	e = this->db->query(this->db,
			"SELECT c.id, c.name, c.lifetime, c.rekeytime, c.jitter, c.updown, "
			"c.hostaccess, c.mode, c.start_action, c.dpd_action, "
			"c.close_action, c.ipcomp, c.reqid "
			"FROM child_configs AS c JOIN peer_config_child_config AS pc "
			"ON c.id = pc.child_cfg WHERE pc.peer_cfg = ?",
			DB_INT, id,
			DB_INT, DB_TEXT, DB_INT, DB_INT, DB_INT, DB_TEXT, DB_INT,
			DB_INT, DB_INT, DB_INT, DB_INT, DB_INT, DB_INT);
	if (e)
	{
		while ((child_cfg = build_child_cfg(this, e)))
		{
			peer->add_child_cfg(peer, child_cfg);
		}
		e->destroy(e);
	}
}

/**
 * Add IKE proposals to an IKE config
 */
static void add_ike_proposals(private_sql_config_t *this,
							  ike_cfg_t *ike_cfg, int id)
{
	enumerator_t *e;
	proposal_t *proposal;
	char *prop;
	bool use_default = TRUE;

	e = this->db->query(this->db,
			"SELECT p.proposal "
			"FROM proposals AS p "
			"JOIN ike_config_proposal AS ip ON p.id = ip.prop "
			"WHERE ip.ike_cfg = ? ORDER BY ip.prio",
			DB_INT, id, DB_TEXT);
	if (e)
	{
		while (e->enumerate(e, &prop))
		{
			proposal = proposal_create_from_string(PROTO_IKE, prop);
			if (!proposal)
			{
				DBG1(DBG_CFG, "could not create IKE proposal from '%s'", prop);
				break;
			}
			ike_cfg->add_proposal(ike_cfg, proposal);
			use_default = FALSE;
		}
		e->destroy(e);
	}
	if (use_default)
	{
		ike_cfg->add_proposal(ike_cfg, proposal_create_default(PROTO_IKE));
		ike_cfg->add_proposal(ike_cfg, proposal_create_default_aead(PROTO_IKE));
	}
}

/**
 * Build an IKE config from an SQL query
 */
static ike_cfg_t *build_ike_cfg(private_sql_config_t *this, enumerator_t *e,
								host_t *my_host, host_t *other_host)
{
	int id, certreq, force_encap;
	char *local, *remote;

	while (e->enumerate(e, &id, &certreq, &force_encap, &local, &remote))
	{
		ike_cfg_t *ike_cfg;

		ike_cfg = ike_cfg_create(IKEV2, certreq, force_encap, local,
								 charon->socket->get_port(charon->socket, FALSE),
								 remote, IKEV2_UDP_PORT, FRAGMENTATION_NO, 0);
		add_ike_proposals(this, ike_cfg, id);
		return ike_cfg;
	}
	return NULL;
}

/**
 * Query an IKE config by its id
 */
static ike_cfg_t* get_ike_cfg_by_id(private_sql_config_t *this, int id)
{
	enumerator_t *e;
	ike_cfg_t *ike_cfg = NULL;

	e = this->db->query(this->db,
			"SELECT c.id, c.certreq, c.force_encap, c.local, c.remote "
			"FROM ike_configs AS c WHERE c.id = ?",
			DB_INT, id,
			DB_INT, DB_INT, DB_INT, DB_TEXT, DB_TEXT);
	if (e)
	{
		ike_cfg = build_ike_cfg(this, e, NULL, NULL);
		e->destroy(e);
	}
	return ike_cfg;
}

#ifdef ME
/**
 * Query a peer config by its id
 */
static peer_cfg_t *get_peer_cfg_by_id(private_sql_config_t *this, int id)
{
	enumerator_t *e;
	peer_cfg_t *peer_cfg = NULL;

	e = this->db->query(this->db,
			"SELECT c.id, c.name, c.ike_cfg, l.type, l.data, r.type, r.data, "
			"c.cert_policy, c.uniqueid, c.auth_method, c.eap_type, "
			"c.eap_vendor, c.keyingtries, c.rekeytime, c.reauthtime, c.jitter, "
			"c.overtime, c.mobike, c.dpd_delay, c.virtual, c.pool, "
			"c.mediation, c.mediated_by, COALESCE(p.type, 0), p.data "
			"FROM peer_configs AS c "
			"JOIN identities AS l ON c.local_id = l.id "
			"JOIN identities AS r ON c.remote_id = r.id "
			"LEFT JOIN identities AS p ON c.peer_id = p.id "
			"WHERE c.id = ?",
			DB_INT, id,
			DB_INT, DB_TEXT, DB_INT, DB_INT, DB_BLOB, DB_INT, DB_BLOB,
			DB_INT, DB_INT, DB_INT, DB_INT, DB_INT,
			DB_INT, DB_INT, DB_INT, DB_INT, DB_INT, DB_INT,
			DB_INT, DB_TEXT, DB_TEXT,
			DB_INT, DB_INT, DB_INT, DB_BLOB);
	if (e)
	{
		peer_cfg = build_peer_cfg(this, e, NULL, NULL);
		e->destroy(e);
	}
	return peer_cfg;
}
#endif /* ME */

/**
 * Check if the two IDs match (the first one is optional)
 */
static inline bool id_matches(identification_t *id, identification_t *sql_id)
{
	return !id || id->matches(id, sql_id) || sql_id->matches(sql_id, id);
}

/**
 * Build a peer config from an SQL query
 */
static peer_cfg_t *build_peer_cfg(private_sql_config_t *this, enumerator_t *e,
								  identification_t *me, identification_t *other)
{
	int id, ike_cfg, l_type, r_type,
		cert_policy, uniqueid, auth_method, eap_type, eap_vendor, keyingtries,
		rekeytime, reauthtime, jitter, overtime, mobike, dpd_delay,
		mediation, mediated_by, p_type;
	chunk_t l_data, r_data, p_data;
	char *name, *virtual, *pool;
	enumerator_t *enumerator;

	while (e->enumerate(e,
			&id, &name, &ike_cfg, &l_type, &l_data, &r_type, &r_data,
			&cert_policy, &uniqueid, &auth_method, &eap_type, &eap_vendor,
			&keyingtries, &rekeytime, &reauthtime, &jitter, &overtime, &mobike,
			&dpd_delay,	&virtual, &pool,
			&mediation, &mediated_by, &p_type, &p_data))
	{
		identification_t *local_id, *remote_id, *peer_id = NULL;
		peer_cfg_t *peer_cfg, *mediated_cfg = NULL;
		ike_cfg_t *ike;
		host_t *vip = NULL;
		auth_cfg_t *auth;

		local_id = identification_create_from_encoding(l_type, l_data);
		remote_id = identification_create_from_encoding(r_type, r_data);
		if (!id_matches(me, local_id) || !id_matches(other, remote_id))
		{
			local_id->destroy(local_id);
			remote_id->destroy(remote_id);
			continue;
		}
		ike = get_ike_cfg_by_id(this, ike_cfg);

#ifdef ME
		mediated_cfg = mediated_by ? get_peer_cfg_by_id(this, mediated_by)
								   : NULL;
		if (p_type)
		{
			peer_id = identification_create_from_encoding(p_type, p_data);
		}
#endif /* ME */

		if (virtual)
		{
			vip = host_create_from_string(virtual, 0);
		}
		if (ike)
		{
			peer_cfg_create_t peer = {
				.cert_policy = cert_policy,
				.unique = uniqueid,
				.keyingtries = keyingtries,
				.rekey_time = rekeytime,
				.reauth_time = reauthtime,
				.jitter_time = jitter,
				.over_time = overtime,
				.no_mobike = !mobike,
				.dpd = dpd_delay,
#ifdef ME
				.mediation = mediation,
				.mediated_by = mediated_cfg ?
									mediated_cfg->get_name(mediated_cfg) : NULL,
				.peer_id = peer_id,
#endif /* ME */
			};

			peer_cfg = peer_cfg_create(name, ike, &peer);
			if (vip)
			{
				peer_cfg->add_virtual_ip(peer_cfg, vip);
			}
			if (pool)
			{
				/* attr-sql used comma separated pools, but we now completely
				 * support multiple pools directly. Support old SQL configs: */
				enumerator = enumerator_create_token(pool, ",", " ");
				while (enumerator->enumerate(enumerator, &pool))
				{
					peer_cfg->add_pool(peer_cfg, pool);
				}
				enumerator->destroy(enumerator);
			}
			auth = auth_cfg_create();
			auth->add(auth, AUTH_RULE_AUTH_CLASS, auth_method);
			auth->add(auth, AUTH_RULE_IDENTITY, local_id);
			peer_cfg->add_auth_cfg(peer_cfg, auth, TRUE);
			auth = auth_cfg_create();
			auth->add(auth, AUTH_RULE_IDENTITY, remote_id);
			if (eap_type)
			{
				auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_EAP);
				auth->add(auth, AUTH_RULE_EAP_TYPE, eap_type);
				if (eap_vendor)
				{
					auth->add(auth, AUTH_RULE_EAP_VENDOR, eap_vendor);
				}
			}
			peer_cfg->add_auth_cfg(peer_cfg, auth, FALSE);
			add_child_cfgs(this, peer_cfg, id);
			DESTROY_IF(mediated_cfg);
			return peer_cfg;
		}
		DESTROY_IF(ike);
		DESTROY_IF(mediated_cfg);
		DESTROY_IF(peer_id);
		DESTROY_IF(local_id);
		DESTROY_IF(remote_id);
	}
	return NULL;
}

METHOD(backend_t, get_peer_cfg_by_name, peer_cfg_t*,
	private_sql_config_t *this, char *name)
{
	enumerator_t *e;
	peer_cfg_t *peer_cfg = NULL;

	e = this->db->query(this->db,
			"SELECT c.id, c.name, c.ike_cfg, l.type, l.data, r.type, r.data, "
			"c.cert_policy, c.uniqueid, c.auth_method, c.eap_type, "
			"c.eap_vendor, c.keyingtries, c.rekeytime, c.reauthtime, c.jitter, "
			"c.overtime, c.mobike, c.dpd_delay, c.virtual, c.pool, "
			"c.mediation, c.mediated_by, COALESCE(p.type, 0), p.data "
			"FROM peer_configs AS c "
			"JOIN identities AS l ON c.local_id = l.id "
			"JOIN identities AS r ON c.remote_id = r.id "
			"LEFT JOIN identities AS p ON c.peer_id = p.id "
			"WHERE c.ike_version = ? AND c.name = ?",
			DB_INT, 2, DB_TEXT, name,
			DB_INT, DB_TEXT, DB_INT, DB_INT, DB_BLOB, DB_INT, DB_BLOB,
			DB_INT, DB_INT, DB_INT, DB_INT, DB_INT,
			DB_INT, DB_INT, DB_INT, DB_INT, DB_INT, DB_INT,
			DB_INT, DB_TEXT, DB_TEXT,
			DB_INT, DB_INT,	DB_INT, DB_BLOB);
	if (e)
	{
		peer_cfg = build_peer_cfg(this, e, NULL, NULL);
		e->destroy(e);
	}
	return peer_cfg;
}

typedef struct {
	/** implements enumerator */
	enumerator_t public;
	/** reference to context */
	private_sql_config_t *this;
	/** filtering own host */
	host_t *me;
	/** filtering remote host */
	host_t *other;
	/** inner SQL enumerator */
	enumerator_t *inner;
	/** currently enumerated peer config */
	ike_cfg_t *current;
} ike_enumerator_t;

METHOD(enumerator_t, ike_enumerator_enumerate, bool,
	ike_enumerator_t *this, va_list args)
{
	ike_cfg_t **cfg;

	VA_ARGS_VGET(args, cfg);
	DESTROY_IF(this->current);
	this->current = build_ike_cfg(this->this, this->inner, this->me, this->other);
	if (this->current)
	{
		*cfg = this->current;
		return TRUE;
	}
	return FALSE;
}

METHOD(enumerator_t, ike_enumerator_destroy, void,
	ike_enumerator_t *this)
{
	DESTROY_IF(this->current);
	this->inner->destroy(this->inner);
	free(this);
}

METHOD(backend_t, create_ike_cfg_enumerator, enumerator_t*,
	private_sql_config_t *this, host_t *me, host_t *other)
{
	ike_enumerator_t *e;

	INIT(e,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _ike_enumerator_enumerate,
			.destroy = _ike_enumerator_destroy,
		},
		.this = this,
		.me = me,
		.other = other,
	);
	e->inner = this->db->query(this->db,
							   "SELECT c.id, c.certreq, c.force_encap, "
							   "c.local, c.remote FROM ike_configs AS c",
							   DB_INT, DB_INT, DB_INT, DB_TEXT, DB_TEXT);
	if (!e->inner)
	{
		free(e);
		return NULL;
	}
	return &e->public;
}


typedef struct {
	/** implements enumerator */
	enumerator_t public;
	/** reference to context */
	private_sql_config_t *this;
	/** filtering own identity */
	identification_t *me;
	/** filtering remote identity */
	identification_t *other;
	/** inner SQL enumerator */
	enumerator_t *inner;
	/** currently enumerated peer config */
	peer_cfg_t *current;
} peer_enumerator_t;

METHOD(enumerator_t, peer_enumerator_enumerate, bool,
	peer_enumerator_t *this, va_list args)
{
	peer_cfg_t **cfg;

	VA_ARGS_VGET(args, cfg);
	DESTROY_IF(this->current);
	this->current = build_peer_cfg(this->this, this->inner, this->me, this->other);
	if (this->current)
	{
		*cfg = this->current;
		return TRUE;
	}
	return FALSE;
}

METHOD(enumerator_t, peer_enumerator_destroy, void,
	peer_enumerator_t *this)
{
	DESTROY_IF(this->current);
	this->inner->destroy(this->inner);
	free(this);
}

METHOD(backend_t, create_peer_cfg_enumerator, enumerator_t*,
	private_sql_config_t *this, identification_t *me, identification_t *other)
{
	peer_enumerator_t *e;

	INIT(e,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _peer_enumerator_enumerate,
			.destroy = _peer_enumerator_destroy,
		},
		.this = this,
		.me = me,
		.other = other,
	);

	/* TODO: only get configs whose IDs match exactly or contain wildcards */
	e->inner = this->db->query(this->db,
			"SELECT c.id, c.name, c.ike_cfg, l.type, l.data, r.type, r.data, "
			"c.cert_policy, c.uniqueid, c.auth_method, c.eap_type, "
			"c.eap_vendor, c.keyingtries, c.rekeytime, c.reauthtime, c.jitter, "
			"c.overtime, c.mobike, c.dpd_delay, c.virtual, c.pool, "
			"c.mediation, c.mediated_by, COALESCE(p.type, 0), p.data "
			"FROM peer_configs AS c "
			"JOIN identities AS l ON c.local_id = l.id "
			"JOIN identities AS r ON c.remote_id = r.id "
			"LEFT JOIN identities AS p ON c.peer_id = p.id "
			"WHERE c.ike_version = ?",
			DB_INT, 2,
			DB_INT, DB_TEXT, DB_INT, DB_INT, DB_BLOB, DB_INT, DB_BLOB,
			DB_INT, DB_INT, DB_INT, DB_INT, DB_INT,
			DB_INT, DB_INT, DB_INT, DB_INT, DB_INT, DB_INT,
			DB_INT,	DB_TEXT, DB_TEXT,
			DB_INT, DB_INT, DB_INT, DB_BLOB);
	if (!e->inner)
	{
		free(e);
		return NULL;
	}
	return &e->public;
}

METHOD(sql_config_t, destroy, void,
	private_sql_config_t *this)
{
	free(this);
}

/**
 * Described in header.
 */
sql_config_t *sql_config_create(database_t *db)
{
	private_sql_config_t *this;

	INIT(this,
		.public = {
			.backend = {
				.create_peer_cfg_enumerator = _create_peer_cfg_enumerator,
				.create_ike_cfg_enumerator = _create_ike_cfg_enumerator,
				.get_peer_cfg_by_name = _get_peer_cfg_by_name,
			},
			.destroy = _destroy,
		},
		.db = db
	);

	return &this->public;
}
