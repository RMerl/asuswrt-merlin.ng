/*
 * Copyright (C) 2007-2018 Tobias Brunner
 * Copyright (C) 2005-2009 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "peer_cfg.h"

#include <daemon.h>

#include <threading/rwlock.h>
#include <collections/linked_list.h>
#include <utils/identification.h>

ENUM(cert_policy_names, CERT_ALWAYS_SEND, CERT_NEVER_SEND,
	"CERT_ALWAYS_SEND",
	"CERT_SEND_IF_ASKED",
	"CERT_NEVER_SEND",
);

ENUM(unique_policy_names, UNIQUE_NEVER, UNIQUE_KEEP,
	"UNIQUE_NEVER",
	"UNIQUE_NO",
	"UNIQUE_REPLACE",
	"UNIQUE_KEEP",
);

typedef struct private_peer_cfg_t private_peer_cfg_t;

/**
 * Private data of an peer_cfg_t object
 */
struct private_peer_cfg_t {

	/**
	 * Public part
	 */
	peer_cfg_t public;

	/**
	 * Number of references hold by others to this peer_cfg
	 */
	refcount_t refcount;

	/**
	 * Name of the peer_cfg, used to query it
	 */
	char *name;

	/**
	 * IKE config associated to this peer config
	 */
	ike_cfg_t *ike_cfg;

	/**
	 * list of child configs associated to this peer config
	 */
	linked_list_t *child_cfgs;

	/**
	 * lock to access list of child_cfgs
	 */
	rwlock_t *lock;

	/**
	 * should we send a certificate
	 */
	cert_policy_t cert_policy;

	/**
	 * uniqueness of an IKE_SA
	 */
	unique_policy_t unique;

	/**
	 * number of tries after giving up if peer does not respond
	 */
	uint32_t keyingtries;

	/**
	 * enable support for MOBIKE
	 */
	bool use_mobike;

	/**
	 * Use aggressive mode?
	 */
	bool aggressive;

	/**
	 * Use pull or push in mode config?
	 */
	bool pull_mode;

	/**
	 * Time before starting rekeying
	 */
	uint32_t rekey_time;

	/**
	 * Time before starting reauthentication
	 */
	uint32_t reauth_time;

	/**
	 * Time, which specifies the range of a random value subtracted from above.
	 */
	uint32_t jitter_time;

	/**
	 * Delay before deleting a rekeying/reauthenticating SA
	 */
	uint32_t over_time;

	/**
	 * DPD check interval
	 */
	uint32_t dpd;

	/**
	 * DPD timeout interval (used for IKEv1 only)
	 */
	uint32_t dpd_timeout;

	/**
	 * List of virtual IPs (host_t*) to request
	 */
	linked_list_t *vips;

	/**
	 * List of pool names to use for virtual IP lookup
	 */
	linked_list_t *pools;

	/**
	 * local authentication configs (rulesets)
	 */
	linked_list_t *local_auth;

	/**
	 * remote authentication configs (constraints)
	 */
	linked_list_t *remote_auth;

	/**
	 * PPK ID
	 */
	identification_t *ppk_id;

	/**
	 * Whether a PPK is required
	 */
	bool ppk_required;

#ifdef ME
	/**
	 * Is this a mediation connection?
	 */
	bool mediation;

	/**
	 * Name of the mediation connection to mediate through
	 */
	char *mediated_by;

	/**
	 * ID of our peer at the mediation server (= leftid of the peer's conn with
	 * the mediation server)
	 */
	identification_t *peer_id;
#endif /* ME */
};

METHOD(peer_cfg_t, get_name, char*,
	private_peer_cfg_t *this)
{
	return this->name;
}

METHOD(peer_cfg_t, get_ike_version, ike_version_t,
	private_peer_cfg_t *this)
{
	return this->ike_cfg->get_version(this->ike_cfg);
}

METHOD(peer_cfg_t, get_ike_cfg, ike_cfg_t*,
	private_peer_cfg_t *this)
{
	return this->ike_cfg;
}

METHOD(peer_cfg_t, add_child_cfg, void,
	private_peer_cfg_t *this, child_cfg_t *child_cfg)
{
	this->lock->write_lock(this->lock);
	this->child_cfgs->insert_last(this->child_cfgs, child_cfg);
	this->lock->unlock(this->lock);
}

typedef struct {
	enumerator_t public;
	linked_list_t *removed;
	linked_list_t *added;
	enumerator_t *wrapped;
	bool add;
} child_cfgs_replace_enumerator_t;

METHOD(enumerator_t, child_cfgs_replace_enumerate, bool,
	child_cfgs_replace_enumerator_t *this, va_list args)
{
	child_cfg_t *child_cfg, **chd;
	bool *added;

	VA_ARGS_VGET(args, chd, added);

	if (!this->wrapped)
	{
		this->wrapped = this->removed->create_enumerator(this->removed);
	}
	while (TRUE)
	{
		if (this->wrapped->enumerate(this->wrapped, &child_cfg))
		{
			if (chd)
			{
				*chd = child_cfg;
			}
			if (added)
			{
				*added = this->add;
			}
			return TRUE;
		}
		if (this->add)
		{
			break;
		}
		this->wrapped->destroy(this->wrapped);
		this->wrapped = this->added->create_enumerator(this->added);
		this->add = TRUE;
	}
	return FALSE;
}

METHOD(enumerator_t, child_cfgs_replace_enumerator_destroy, void,
	child_cfgs_replace_enumerator_t *this)
{
	DESTROY_IF(this->wrapped);
	this->removed->destroy_offset(this->removed, offsetof(child_cfg_t, destroy));
	this->added->destroy_offset(this->added, offsetof(child_cfg_t, destroy));
	free(this);
}

METHOD(peer_cfg_t, replace_child_cfgs, enumerator_t*,
	private_peer_cfg_t *this, peer_cfg_t *other_pub)
{
	private_peer_cfg_t *other = (private_peer_cfg_t*)other_pub;
	linked_list_t *new_cfgs, *removed, *added;
	enumerator_t *mine, *others;
	child_cfg_t *my_cfg, *other_cfg;
	child_cfgs_replace_enumerator_t *enumerator;
	bool found;

	added = linked_list_create();

	other->lock->read_lock(other->lock);
	new_cfgs = linked_list_create_from_enumerator(
					other->child_cfgs->create_enumerator(other->child_cfgs));
	new_cfgs->invoke_offset(new_cfgs, offsetof(child_cfg_t, get_ref));
	other->lock->unlock(other->lock);

	this->lock->write_lock(this->lock);
	removed = this->child_cfgs;
	this->child_cfgs = new_cfgs;
	others = new_cfgs->create_enumerator(new_cfgs);
	mine = removed->create_enumerator(removed);
	while (others->enumerate(others, &other_cfg))
	{
		found = FALSE;
		while (mine->enumerate(mine, &my_cfg))
		{
			if (my_cfg->equals(my_cfg, other_cfg))
			{
				removed->remove_at(removed, mine);
				my_cfg->destroy(my_cfg);
				found = TRUE;
				break;
			}
		}
		removed->reset_enumerator(removed, mine);
		if (!found)
		{
			added->insert_last(added, other_cfg->get_ref(other_cfg));
		}
	}
	others->destroy(others);
	mine->destroy(mine);
	this->lock->unlock(this->lock);

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _child_cfgs_replace_enumerate,
			.destroy = _child_cfgs_replace_enumerator_destroy,
		},
		.removed = removed,
		.added = added,
	);
	return &enumerator->public;
}

/**
 * child_cfg enumerator
 */
typedef struct {
	enumerator_t public;
	enumerator_t *wrapped;
	rwlock_t *lock;
} child_cfg_enumerator_t;

METHOD(peer_cfg_t, remove_child_cfg, void,
	private_peer_cfg_t *this, child_cfg_enumerator_t *enumerator)
{
	this->child_cfgs->remove_at(this->child_cfgs, enumerator->wrapped);
}

METHOD(enumerator_t, child_cfg_enumerator_destroy, void,
	child_cfg_enumerator_t *this)
{
	this->lock->unlock(this->lock);
	this->wrapped->destroy(this->wrapped);
	free(this);
}

METHOD(enumerator_t, child_cfg_enumerate, bool,
	child_cfg_enumerator_t *this, va_list args)
{
	child_cfg_t **chd;

	VA_ARGS_VGET(args, chd);
	return this->wrapped->enumerate(this->wrapped, chd);
}

METHOD(peer_cfg_t, create_child_cfg_enumerator, enumerator_t*,
	private_peer_cfg_t *this)
{
	child_cfg_enumerator_t *enumerator;

	INIT(enumerator,
		.public = {
			.enumerate = enumerator_enumerate_default,
			.venumerate = _child_cfg_enumerate,
			.destroy = _child_cfg_enumerator_destroy,
		},
		.lock = this->lock,
		.wrapped = this->child_cfgs->create_enumerator(this->child_cfgs),
	);

	this->lock->read_lock(this->lock);
	return &enumerator->public;
}

/**
 * Check how good a list of TS matches a given child config
 */
static int get_ts_match(child_cfg_t *cfg, bool local,
						linked_list_t *sup_list, linked_list_t *hosts)
{
	linked_list_t *cfg_list;
	enumerator_t *sup_enum, *cfg_enum;
	traffic_selector_t *sup_ts, *cfg_ts, *subset;
	int match = 0, round;

	/* fetch configured TS list, narrowing dynamic TS */
	cfg_list = cfg->get_traffic_selectors(cfg, local, NULL, hosts, TRUE);

	/* use a round counter to rate leading TS with higher priority */
	round = sup_list->get_count(sup_list);

	sup_enum = sup_list->create_enumerator(sup_list);
	while (sup_enum->enumerate(sup_enum, &sup_ts))
	{
		cfg_enum = cfg_list->create_enumerator(cfg_list);
		while (cfg_enum->enumerate(cfg_enum, &cfg_ts))
		{
			if (cfg_ts->equals(cfg_ts, sup_ts))
			{	/* equality is honored better than matches */
				match += round * 5;
			}
			else
			{
				subset = cfg_ts->get_subset(cfg_ts, sup_ts);
				if (subset)
				{
					subset->destroy(subset);
					match += round * 1;
				}
			}
		}
		cfg_enum->destroy(cfg_enum);
		round--;
	}
	sup_enum->destroy(sup_enum);

	cfg_list->destroy_offset(cfg_list, offsetof(traffic_selector_t, destroy));

	return match;
}

METHOD(peer_cfg_t, select_child_cfg, child_cfg_t*,
	private_peer_cfg_t *this, linked_list_t *my_ts, linked_list_t *other_ts,
	linked_list_t *my_hosts, linked_list_t *other_hosts)
{
	child_cfg_t *current, *found = NULL;
	enumerator_t *enumerator;
	int best = 0;

	DBG2(DBG_CFG, "looking for a child config for %#R === %#R", my_ts, other_ts);
	enumerator = create_child_cfg_enumerator(this);
	while (enumerator->enumerate(enumerator, &current))
	{
		int my_prio, other_prio;

		my_prio = get_ts_match(current, TRUE, my_ts, my_hosts);
		other_prio = get_ts_match(current, FALSE, other_ts, other_hosts);

		if (my_prio && other_prio)
		{
			DBG2(DBG_CFG, "  candidate \"%s\" with prio %d+%d",
				 current->get_name(current), my_prio, other_prio);
			if (my_prio + other_prio > best)
			{
				best = my_prio + other_prio;
				DESTROY_IF(found);
				found = current->get_ref(current);
			}
		}
	}
	enumerator->destroy(enumerator);
	if (found)
	{
		DBG2(DBG_CFG, "found matching child config \"%s\" with prio %d",
			 found->get_name(found), best);
	}
	return found;
}

METHOD(peer_cfg_t, get_cert_policy, cert_policy_t,
	private_peer_cfg_t *this)
{
	return this->cert_policy;
}

METHOD(peer_cfg_t, get_unique_policy, unique_policy_t,
	private_peer_cfg_t *this)
{
	return this->unique;
}

METHOD(peer_cfg_t, get_keyingtries, uint32_t,
	private_peer_cfg_t *this)
{
	return this->keyingtries;
}

METHOD(peer_cfg_t, get_rekey_time, uint32_t,
	private_peer_cfg_t *this, bool jitter)
{
	if (this->rekey_time == 0)
	{
		return 0;
	}
	if (this->jitter_time == 0 || !jitter)
	{
		return this->rekey_time;
	}
	return this->rekey_time - (random() % this->jitter_time);
}

METHOD(peer_cfg_t, get_reauth_time, uint32_t,
	private_peer_cfg_t *this, bool jitter)
{
	if (this->reauth_time == 0)
	{
		return 0;
	}
	if (this->jitter_time == 0 || !jitter)
	{
		return this->reauth_time;
	}
	return this->reauth_time - (random() % this->jitter_time);
}

METHOD(peer_cfg_t, get_over_time, uint32_t,
	private_peer_cfg_t *this)
{
	return this->over_time;
}

METHOD(peer_cfg_t, use_mobike, bool,
	private_peer_cfg_t *this)
{
	return this->use_mobike;
}

METHOD(peer_cfg_t, use_aggressive, bool,
	private_peer_cfg_t *this)
{
	return this->aggressive;
}

METHOD(peer_cfg_t, use_pull_mode, bool,
	private_peer_cfg_t *this)
{
	return this->pull_mode;
}

METHOD(peer_cfg_t, get_dpd, uint32_t,
	private_peer_cfg_t *this)
{
	return this->dpd;
}

METHOD(peer_cfg_t, get_dpd_timeout, uint32_t,
	private_peer_cfg_t *this)
{
	return this->dpd_timeout;
}

METHOD(peer_cfg_t, add_virtual_ip, void,
	private_peer_cfg_t *this, host_t *vip)
{
	this->vips->insert_last(this->vips, vip);
}

METHOD(peer_cfg_t, create_virtual_ip_enumerator, enumerator_t*,
	private_peer_cfg_t *this)
{
	return this->vips->create_enumerator(this->vips);
}

METHOD(peer_cfg_t, add_pool, void,
	private_peer_cfg_t *this, char *name)
{
	this->pools->insert_last(this->pools, strdup(name));
}

METHOD(peer_cfg_t, create_pool_enumerator, enumerator_t*,
	private_peer_cfg_t *this)
{
	return this->pools->create_enumerator(this->pools);
}

METHOD(peer_cfg_t, add_auth_cfg, void,
	private_peer_cfg_t *this, auth_cfg_t *cfg, bool local)
{
	if (local)
	{
		this->local_auth->insert_last(this->local_auth, cfg);
	}
	else
	{
		this->remote_auth->insert_last(this->remote_auth, cfg);
	}
}

METHOD(peer_cfg_t, create_auth_cfg_enumerator, enumerator_t*,
	private_peer_cfg_t *this, bool local)
{
	if (local)
	{
		return this->local_auth->create_enumerator(this->local_auth);
	}
	return this->remote_auth->create_enumerator(this->remote_auth);
}

METHOD(peer_cfg_t, get_ppk_id, identification_t*,
	private_peer_cfg_t *this)
{
	return this->ppk_id;
}

METHOD(peer_cfg_t, ppk_required, bool,
	private_peer_cfg_t *this)
{
	return this->ppk_required;
}

#ifdef ME
METHOD(peer_cfg_t, is_mediation, bool,
	private_peer_cfg_t *this)
{
	return this->mediation;
}

METHOD(peer_cfg_t, get_mediated_by, char*,
	private_peer_cfg_t *this)
{
	return this->mediated_by;
}

METHOD(peer_cfg_t, get_peer_id, identification_t*,
	private_peer_cfg_t *this)
{
	return this->peer_id;
}
#endif /* ME */

/**
 * check auth configs for equality
 */
static bool auth_cfg_equal(private_peer_cfg_t *this, private_peer_cfg_t *other)
{
	enumerator_t *e1, *e2;
	auth_cfg_t *cfg1, *cfg2;
	bool equal = TRUE;

	if (this->local_auth->get_count(this->local_auth) !=
		other->local_auth->get_count(other->local_auth))
	{
		return FALSE;
	}
	if (this->remote_auth->get_count(this->remote_auth) !=
		other->remote_auth->get_count(other->remote_auth))
	{
		return FALSE;
	}

	e1 = this->local_auth->create_enumerator(this->local_auth);
	e2 = other->local_auth->create_enumerator(other->local_auth);
	while (e1->enumerate(e1, &cfg1) && e2->enumerate(e2, &cfg2))
	{
		if (!cfg1->equals(cfg1, cfg2))
		{
			equal = FALSE;
			break;
		}
	}
	e1->destroy(e1);
	e2->destroy(e2);

	if (!equal)
	{
		return FALSE;
	}

	e1 = this->remote_auth->create_enumerator(this->remote_auth);
	e2 = other->remote_auth->create_enumerator(other->remote_auth);
	while (e1->enumerate(e1, &cfg1) && e2->enumerate(e2, &cfg2))
	{
		if (!cfg1->equals(cfg1, cfg2))
		{
			equal = FALSE;
			break;
		}
	}
	e1->destroy(e1);
	e2->destroy(e2);

	return equal;
}

/**
 * Check if two identities are equal, or both are not set
 */
static bool id_equal(identification_t *this, identification_t *other)
{
	return this == other || (this && other && this->equals(this, other));
}

METHOD(peer_cfg_t, equals, bool,
	private_peer_cfg_t *this, private_peer_cfg_t *other)
{
	if (this == other)
	{
		return TRUE;
	}
	if (this->public.equals != other->public.equals)
	{
		return FALSE;
	}
	if (!this->vips->equals_offset(this->vips, other->vips,
								   offsetof(host_t, ip_equals)))
	{
		return FALSE;
	}
	if (!this->pools->equals_function(this->pools, other->pools, (void*)streq))
	{
		return FALSE;
	}
	return (
		get_ike_version(this) == get_ike_version(other) &&
		this->cert_policy == other->cert_policy &&
		this->unique == other->unique &&
		this->keyingtries == other->keyingtries &&
		this->use_mobike == other->use_mobike &&
		this->rekey_time == other->rekey_time &&
		this->reauth_time == other->reauth_time &&
		this->jitter_time == other->jitter_time &&
		this->over_time == other->over_time &&
		this->dpd == other->dpd &&
		this->aggressive == other->aggressive &&
		this->pull_mode == other->pull_mode &&
		auth_cfg_equal(this, other) &&
		this->ppk_required == other->ppk_required &&
		id_equal(this->ppk_id, other->ppk_id)
#ifdef ME
		&& this->mediation == other->mediation &&
		streq(this->mediated_by, other->mediated_by) &&
		id_equal(this->peer_id, other->peer_id)
#endif /* ME */
		);
}

METHOD(peer_cfg_t, get_ref, peer_cfg_t*,
	private_peer_cfg_t *this)
{
	ref_get(&this->refcount);
	return &this->public;
}

METHOD(peer_cfg_t, destroy, void,
	private_peer_cfg_t *this)
{
	if (ref_put(&this->refcount))
	{
		this->ike_cfg->destroy(this->ike_cfg);
		this->child_cfgs->destroy_offset(this->child_cfgs,
										offsetof(child_cfg_t, destroy));
		this->local_auth->destroy_offset(this->local_auth,
										offsetof(auth_cfg_t, destroy));
		this->remote_auth->destroy_offset(this->remote_auth,
										offsetof(auth_cfg_t, destroy));
		this->vips->destroy_offset(this->vips, offsetof(host_t, destroy));
		this->pools->destroy_function(this->pools, free);
#ifdef ME
		DESTROY_IF(this->peer_id);
		free(this->mediated_by);
#endif /* ME */
		DESTROY_IF(this->ppk_id);
		this->lock->destroy(this->lock);
		free(this->name);
		free(this);
	}
}

/*
 * Described in header-file
 */
peer_cfg_t *peer_cfg_create(char *name, ike_cfg_t *ike_cfg,
							peer_cfg_create_t *data)
{
	private_peer_cfg_t *this;

	if (data->rekey_time && data->jitter_time > data->rekey_time)
	{
		data->jitter_time = data->rekey_time;
	}
	if (data->reauth_time && data->jitter_time > data->reauth_time)
	{
		data->jitter_time = data->reauth_time;
	}
	if (data->dpd && data->dpd_timeout && data->dpd > data->dpd_timeout)
	{
		data->dpd_timeout = data->dpd;
	}

	INIT(this,
		.public = {
			.get_name = _get_name,
			.get_ike_version = _get_ike_version,
			.get_ike_cfg = _get_ike_cfg,
			.add_child_cfg = _add_child_cfg,
			.remove_child_cfg = (void*)_remove_child_cfg,
			.replace_child_cfgs = _replace_child_cfgs,
			.create_child_cfg_enumerator = _create_child_cfg_enumerator,
			.select_child_cfg = _select_child_cfg,
			.get_cert_policy = _get_cert_policy,
			.get_unique_policy = _get_unique_policy,
			.get_keyingtries = _get_keyingtries,
			.get_rekey_time = _get_rekey_time,
			.get_reauth_time = _get_reauth_time,
			.get_over_time = _get_over_time,
			.use_mobike = _use_mobike,
			.use_aggressive = _use_aggressive,
			.use_pull_mode = _use_pull_mode,
			.get_dpd = _get_dpd,
			.get_dpd_timeout = _get_dpd_timeout,
			.add_virtual_ip = _add_virtual_ip,
			.create_virtual_ip_enumerator = _create_virtual_ip_enumerator,
			.add_pool = _add_pool,
			.create_pool_enumerator = _create_pool_enumerator,
			.add_auth_cfg = _add_auth_cfg,
			.create_auth_cfg_enumerator = _create_auth_cfg_enumerator,
			.get_ppk_id = _get_ppk_id,
			.ppk_required = _ppk_required,
			.equals = (void*)_equals,
			.get_ref = _get_ref,
			.destroy = _destroy,
#ifdef ME
			.is_mediation = _is_mediation,
			.get_mediated_by = _get_mediated_by,
			.get_peer_id = _get_peer_id,
#endif /* ME */
		},
		.name = strdup(name),
		.ike_cfg = ike_cfg,
		.child_cfgs = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
		.cert_policy = data->cert_policy,
		.unique = data->unique,
		.keyingtries = data->keyingtries,
		.rekey_time = data->rekey_time,
		.reauth_time = data->reauth_time,
		.jitter_time = data->jitter_time,
		.over_time = data->over_time,
		.use_mobike = !data->no_mobike,
		.aggressive = data->aggressive,
		.pull_mode = !data->push_mode,
		.dpd = data->dpd,
		.dpd_timeout = data->dpd_timeout,
		.ppk_id = data->ppk_id,
		.ppk_required = data->ppk_required,
		.vips = linked_list_create(),
		.pools = linked_list_create(),
		.local_auth = linked_list_create(),
		.remote_auth = linked_list_create(),
		.refcount = 1,
#ifdef ME
		.mediation = data->mediation,
		.mediated_by = strdupnull(data->mediated_by),
		.peer_id = data->peer_id,
#endif /* ME */
	);

	return &this->public;
}
