/*
 * Copyright (C) 2010 Martin Willi
 * Copyright (C) 2010 revosec AG
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

#include "config.h"

#include <daemon.h>
#include <conftest.h>

typedef struct private_config_t private_config_t;

/**
 * Private data of an config_t object.
 */
struct private_config_t {

	/**
	 * Public config_t interface.
	 */
	config_t public;

	/**
	 * List of loaded peer configs
	 */
	linked_list_t *configs;
};

/**
 * filter function for ike configs
 */
static bool ike_filter(void *data, peer_cfg_t **in, ike_cfg_t **out)
{
	*out = (*in)->get_ike_cfg(*in);
	return TRUE;
}

METHOD(backend_t, create_ike_cfg_enumerator, enumerator_t*,
	private_config_t *this, host_t *me, host_t *other)
{

	return enumerator_create_filter(
							this->configs->create_enumerator(this->configs),
							(void*)ike_filter, NULL, NULL);
}

METHOD(backend_t, create_peer_cfg_enumerator, enumerator_t*,
	private_config_t *this, identification_t *me, identification_t *other)
{
	return this->configs->create_enumerator(this->configs);
}

METHOD(backend_t, get_peer_cfg_by_name, peer_cfg_t*,
	private_config_t *this, char *name)
{
	enumerator_t *e1, *e2;
	peer_cfg_t *current, *found = NULL;
	child_cfg_t *child;

	e1 = this->configs->create_enumerator(this->configs);
	while (e1->enumerate(e1, &current))
	{
		e2 = current->create_child_cfg_enumerator(current);
		while (e2->enumerate(e2, &child))
		{
			if (streq(child->get_name(child), name))
			{
				found = current;
				found->get_ref(found);
				break;
			}
		}
		e2->destroy(e2);
		if (found)
		{
			break;
		}
	}
	e1->destroy(e1);
	return found;
}

/**
 * Load IKE config for a given section name
 */
static ike_cfg_t *load_ike_config(private_config_t *this,
								  settings_t *settings, char *config)
{
	enumerator_t *enumerator;
	ike_cfg_t *ike_cfg;
	proposal_t *proposal;
	char *token;

	ike_cfg = ike_cfg_create(IKEV2, TRUE,
		settings->get_bool(settings, "configs.%s.fake_nat", FALSE, config),
		settings->get_str(settings, "configs.%s.lhost", "%any", config),
		settings->get_int(settings, "configs.%s.lport", 500, config),
		settings->get_str(settings, "configs.%s.rhost", "%any", config),
		settings->get_int(settings, "configs.%s.rport", 500, config),
		FRAGMENTATION_NO, 0);
	token = settings->get_str(settings, "configs.%s.proposal", NULL, config);
	if (token)
	{
		enumerator = enumerator_create_token(token, ",", " ");
		while (enumerator->enumerate(enumerator, &token))
		{
			proposal = proposal_create_from_string(PROTO_IKE, token);
			if (proposal)
			{
				ike_cfg->add_proposal(ike_cfg, proposal);
			}
			else
			{
				DBG1(DBG_CFG, "parsing proposal '%s' failed, skipped", token);
			}
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		ike_cfg->add_proposal(ike_cfg, proposal_create_default(PROTO_IKE));
		ike_cfg->add_proposal(ike_cfg, proposal_create_default_aead(PROTO_IKE));
	}
	return ike_cfg;
}
/**
 * Load CHILD config for given section names
 */
static child_cfg_t *load_child_config(private_config_t *this,
							settings_t *settings, char *config, char *child)
{
	child_cfg_t *child_cfg;
	lifetime_cfg_t lifetime = {};
	enumerator_t *enumerator;
	proposal_t *proposal;
	traffic_selector_t *ts;
	ipsec_mode_t mode = MODE_TUNNEL;
	char *token;
	u_int32_t tfc;

	if (settings->get_bool(settings, "configs.%s.%s.transport",
						   FALSE, config, child))
	{
		mode = MODE_TRANSPORT;
	}
	tfc = settings->get_int(settings, "configs.%s.%s.tfc_padding",
							0, config, child);
	child_cfg = child_cfg_create(child, &lifetime, NULL, FALSE, mode,
								 ACTION_NONE, ACTION_NONE, ACTION_NONE,
								 FALSE, 0, 0, NULL, NULL, tfc);

	token = settings->get_str(settings, "configs.%s.%s.proposal",
							  NULL, config, child);
	if (token)
	{
		enumerator = enumerator_create_token(token, ",", " ");
		while (enumerator->enumerate(enumerator, &token))
		{
			proposal = proposal_create_from_string(PROTO_ESP, token);
			if (proposal)
			{
				child_cfg->add_proposal(child_cfg, proposal);
			}
			else
			{
				DBG1(DBG_CFG, "parsing proposal '%s' failed, skipped", token);
			}
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		child_cfg->add_proposal(child_cfg, proposal_create_default(PROTO_ESP));
		child_cfg->add_proposal(child_cfg,
								proposal_create_default_aead(PROTO_ESP));
	}

	token = settings->get_str(settings, "configs.%s.%s.lts", NULL, config, child);
	if (token)
	{
		enumerator = enumerator_create_token(token, ",", " ");
		while (enumerator->enumerate(enumerator, &token))
		{
			ts = traffic_selector_create_from_cidr(token, 0, 0, 65535);
			if (ts)
			{
				child_cfg->add_traffic_selector(child_cfg, TRUE, ts);
			}
			else
			{
				DBG1(DBG_CFG, "invalid local ts: %s, skipped", token);
			}
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		ts = traffic_selector_create_dynamic(0, 0, 65535);
		child_cfg->add_traffic_selector(child_cfg, TRUE, ts);
	}

	token = settings->get_str(settings, "configs.%s.%s.rts", NULL, config, child);
	if (token)
	{
		enumerator = enumerator_create_token(token, ",", " ");
		while (enumerator->enumerate(enumerator, &token))
		{
			ts = traffic_selector_create_from_cidr(token, 0, 0, 65535);
			if (ts)
			{
				child_cfg->add_traffic_selector(child_cfg, FALSE, ts);
			}
			else
			{
				DBG1(DBG_CFG, "invalid remote ts: %s, skipped", token);
			}
		}
		enumerator->destroy(enumerator);
	}
	else
	{
		ts = traffic_selector_create_dynamic(0, 0, 65535);
		child_cfg->add_traffic_selector(child_cfg, FALSE, ts);
	}
	return child_cfg;
}

/**
 * Load peer config for a given section name
 */
static peer_cfg_t *load_peer_config(private_config_t *this,
									settings_t *settings, char *config)
{
	ike_cfg_t *ike_cfg;
	peer_cfg_t *peer_cfg;
	auth_cfg_t *auth;
	child_cfg_t *child_cfg;
	enumerator_t *enumerator;
	identification_t *lid, *rid;
	char *child, *policy, *pool;
	uintptr_t strength;

	ike_cfg = load_ike_config(this, settings, config);
	peer_cfg = peer_cfg_create(config, ike_cfg, CERT_ALWAYS_SEND,
							   UNIQUE_NO, 1, 0, 0, 0, 0, FALSE, FALSE, TRUE,
							   0, 0, FALSE, NULL, NULL);

	auth = auth_cfg_create();
	auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
	lid = identification_create_from_string(
				settings->get_str(settings, "configs.%s.lid", "%any", config));
	auth->add(auth, AUTH_RULE_IDENTITY, lid);
	peer_cfg->add_auth_cfg(peer_cfg, auth, TRUE);

	auth = auth_cfg_create();
	auth->add(auth, AUTH_RULE_AUTH_CLASS, AUTH_CLASS_PUBKEY);
	rid = identification_create_from_string(
				settings->get_str(settings, "configs.%s.rid", "%any", config));
	strength = settings->get_int(settings, "configs.%s.rsa_strength", 0, config);
	if (strength)
	{
		auth->add(auth, AUTH_RULE_RSA_STRENGTH, strength);
	}
	strength = settings->get_int(settings, "configs.%s.ecdsa_strength", 0, config);
	if (strength)
	{
		auth->add(auth, AUTH_RULE_ECDSA_STRENGTH, strength);
	}
	policy = settings->get_str(settings, "configs.%s.cert_policy", NULL, config);
	if (policy)
	{
		auth->add(auth, AUTH_RULE_CERT_POLICY, strdup(policy));
	}
	auth->add(auth, AUTH_RULE_IDENTITY, rid);
	peer_cfg->add_auth_cfg(peer_cfg, auth, FALSE);
	pool = settings->get_str(settings, "configs.%s.named_pool", NULL, config);
	if (pool)
	{
		peer_cfg->add_pool(peer_cfg, pool);
	}

	DBG1(DBG_CFG, "loaded config %s: %Y - %Y", config, lid, rid);

	enumerator = settings->create_section_enumerator(settings,
													 "configs.%s", config);
	while (enumerator->enumerate(enumerator, &child))
	{
		child_cfg = load_child_config(this, settings, config, child);
		peer_cfg->add_child_cfg(peer_cfg, child_cfg);
	}
	enumerator->destroy(enumerator);
	return peer_cfg;
}

METHOD(config_t, load, void,
	private_config_t *this, settings_t *settings)
{
	enumerator_t *enumerator;
	char *config;

	enumerator = settings->create_section_enumerator(settings, "configs");
	while (enumerator->enumerate(enumerator, &config))
	{
		this->configs->insert_last(this->configs,
								load_peer_config(this, settings, config));
	}
	enumerator->destroy(enumerator);
}

METHOD(config_t, destroy, void,
	private_config_t *this)
{
	this->configs->destroy_offset(this->configs, offsetof(peer_cfg_t, destroy));
	free(this);
}

/**
 * See header
 */
config_t *config_create()
{
	private_config_t *this;

	INIT(this,
		.public = {
			.backend = {
				.create_ike_cfg_enumerator = _create_ike_cfg_enumerator,
				.create_peer_cfg_enumerator = _create_peer_cfg_enumerator,
				.get_peer_cfg_by_name = _get_peer_cfg_by_name,
			},
			.load = _load,
			.destroy = _destroy,
		},
		.configs = linked_list_create(),
	);

	return &this->public;
}
