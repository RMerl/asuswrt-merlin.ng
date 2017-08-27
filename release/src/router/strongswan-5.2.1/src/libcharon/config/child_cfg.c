/*
 * Copyright (C) 2008-2009 Tobias Brunner
 * Copyright (C) 2005-2007 Martin Willi
 * Copyright (C) 2005 Jan Hutter
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

#include "child_cfg.h"

#include <stdint.h>

#include <daemon.h>

ENUM(action_names, ACTION_NONE, ACTION_RESTART,
	"clear",
	"hold",
	"restart",
);

/** Default replay window size, if not set using charon.replay_window */
#define DEFAULT_REPLAY_WINDOW 32

typedef struct private_child_cfg_t private_child_cfg_t;

/**
 * Private data of an child_cfg_t object
 */
struct private_child_cfg_t {

	/**
	 * Public part
	 */
	child_cfg_t public;

	/**
	 * Number of references hold by others to this child_cfg
	 */
	refcount_t refcount;

	/**
	 * Name of the child_cfg, used to query it
	 */
	char *name;

	/**
	 * list for all proposals
	 */
	linked_list_t *proposals;

	/**
	 * list for traffic selectors for my site
	 */
	linked_list_t *my_ts;

	/**
	 * list for traffic selectors for others site
	 */
	linked_list_t *other_ts;

	/**
	 * updown script
	 */
	char *updown;

	/**
	 * allow host access
	 */
	bool hostaccess;

	/**
	 * Mode to propose for a initiated CHILD: tunnel/transport
	 */
	ipsec_mode_t mode;

	/**
	 * action to take to start CHILD_SA
	 */
	action_t start_action;

	/**
	 * action to take on DPD
	 */
	action_t dpd_action;

	/**
	 * action to take on CHILD_SA close
	 */
	action_t close_action;

	/**
	 * CHILD_SA lifetime config
	 */
	lifetime_cfg_t lifetime;

	/**
	 * enable IPComp
	 */
	bool use_ipcomp;

	/**
	 * Inactivity timeout
	 */
	u_int32_t inactivity;

	/**
	 * Reqid to install CHILD_SA with
	 */
	u_int32_t reqid;

	/**
	 * Optional mark to install inbound CHILD_SA with
	 */
	mark_t mark_in;

	/**
	 * Optional mark to install outbound CHILD_SA with
	 */
	mark_t mark_out;

	/**
	 * Traffic Flow Confidentiality padding, if enabled
	 */
	u_int32_t tfc;

	/**
	 * set up IPsec transport SA in MIPv6 proxy mode
	 */
	bool proxy_mode;

	/**
	 * enable installation and removal of kernel IPsec policies
	 */
	bool install_policy;

	/**
	 * anti-replay window size
	 */
	u_int32_t replay_window;
};

METHOD(child_cfg_t, get_name, char*,
	private_child_cfg_t *this)
{
	return this->name;
}

METHOD(child_cfg_t, add_proposal, void,
	private_child_cfg_t *this, proposal_t *proposal)
{
	if (proposal)
	{
		this->proposals->insert_last(this->proposals, proposal);
	}
}

static bool match_proposal(proposal_t *item, proposal_t *proposal)
{
	return item->equals(item, proposal);
}

METHOD(child_cfg_t, get_proposals, linked_list_t*,
	private_child_cfg_t *this, bool strip_dh)
{
	enumerator_t *enumerator;
	proposal_t *current;
	linked_list_t *proposals = linked_list_create();

	enumerator = this->proposals->create_enumerator(this->proposals);
	while (enumerator->enumerate(enumerator, &current))
	{
		current = current->clone(current);
		if (strip_dh)
		{
			current->strip_dh(current, MODP_NONE);
		}
		if (proposals->find_first(proposals, (linked_list_match_t)match_proposal,
								  NULL, current) == SUCCESS)
		{
			current->destroy(current);
			continue;
		}
		proposals->insert_last(proposals, current);
	}
	enumerator->destroy(enumerator);

	DBG2(DBG_CFG, "configured proposals: %#P", proposals);

	return proposals;
}

METHOD(child_cfg_t, select_proposal, proposal_t*,
	private_child_cfg_t*this, linked_list_t *proposals, bool strip_dh,
	bool private)
{
	enumerator_t *stored_enum, *supplied_enum;
	proposal_t *stored, *supplied, *selected = NULL;

	stored_enum = this->proposals->create_enumerator(this->proposals);
	supplied_enum = proposals->create_enumerator(proposals);

	/* compare all stored proposals with all supplied. Stored ones are preferred. */
	while (stored_enum->enumerate(stored_enum, &stored))
	{
		stored = stored->clone(stored);
		while (supplied_enum->enumerate(supplied_enum, &supplied))
		{
			if (strip_dh)
			{
				stored->strip_dh(stored, MODP_NONE);
			}
			selected = stored->select(stored, supplied, private);
			if (selected)
			{
				DBG2(DBG_CFG, "received proposals: %#P", proposals);
				DBG2(DBG_CFG, "configured proposals: %#P", this->proposals);
				DBG2(DBG_CFG, "selected proposal: %P", selected);
				break;
			}
		}
		stored->destroy(stored);
		if (selected)
		{
			break;
		}
		supplied_enum->destroy(supplied_enum);
		supplied_enum = proposals->create_enumerator(proposals);
	}
	stored_enum->destroy(stored_enum);
	supplied_enum->destroy(supplied_enum);
	if (selected == NULL)
	{
		DBG1(DBG_CFG, "received proposals: %#P", proposals);
		DBG1(DBG_CFG, "configured proposals: %#P", this->proposals);
	}
	return selected;
}

METHOD(child_cfg_t, add_traffic_selector, void,
	private_child_cfg_t *this, bool local, traffic_selector_t *ts)
{
	if (local)
	{
		this->my_ts->insert_last(this->my_ts, ts);
	}
	else
	{
		this->other_ts->insert_last(this->other_ts, ts);
	}
}

METHOD(child_cfg_t, get_traffic_selectors, linked_list_t*,
	private_child_cfg_t *this, bool local, linked_list_t *supplied,
	linked_list_t *hosts)
{
	enumerator_t *e1, *e2;
	traffic_selector_t *ts1, *ts2, *selected;
	linked_list_t *result, *derived;
	host_t *host;

	result = linked_list_create();
	derived = linked_list_create();
	if (local)
	{
		e1 = this->my_ts->create_enumerator(this->my_ts);
	}
	else
	{
		e1 = this->other_ts->create_enumerator(this->other_ts);
	}
	/* In a first step, replace "dynamic" TS with the host list */
	while (e1->enumerate(e1, &ts1))
	{
		if (hosts && hosts->get_count(hosts) &&
			ts1->is_dynamic(ts1))
		{
			e2 = hosts->create_enumerator(hosts);
			while (e2->enumerate(e2, &host))
			{
				ts2 = ts1->clone(ts1);
				ts2->set_address(ts2, host);
				derived->insert_last(derived, ts2);
			}
			e2->destroy(e2);
		}
		else
		{
			derived->insert_last(derived, ts1->clone(ts1));
		}
	}
	e1->destroy(e1);

	DBG2(DBG_CFG, "%s traffic selectors for %s:",
		 supplied ? "selecting" : "proposing", local ? "us" : "other");
	if (supplied == NULL)
	{
		while (derived->remove_first(derived, (void**)&ts1) == SUCCESS)
		{
			DBG2(DBG_CFG, " %R", ts1);
			result->insert_last(result, ts1);
		}
		derived->destroy(derived);
	}
	else
	{
		e1 = derived->create_enumerator(derived);
		e2 = supplied->create_enumerator(supplied);
		/* enumerate all configured/derived selectors */
		while (e1->enumerate(e1, &ts1))
		{
			/* enumerate all supplied traffic selectors */
			while (e2->enumerate(e2, &ts2))
			{
				selected = ts1->get_subset(ts1, ts2);
				if (selected)
				{
					DBG2(DBG_CFG, " config: %R, received: %R => match: %R",
						 ts1, ts2, selected);
					result->insert_last(result, selected);
				}
				else
				{
					DBG2(DBG_CFG, " config: %R, received: %R => no match",
						 ts1, ts2);
				}
			}
			supplied->reset_enumerator(supplied, e2);
		}
		e1->destroy(e1);
		e2->destroy(e2);

		/* check if we/peer did any narrowing, raise alert */
		e1 = derived->create_enumerator(derived);
		e2 = result->create_enumerator(result);
		while (e1->enumerate(e1, &ts1))
		{
			if (!e2->enumerate(e2, &ts2) || !ts1->equals(ts1, ts2))
			{
				charon->bus->alert(charon->bus, ALERT_TS_NARROWED,
								   local, result, this);
				break;
			}
		}
		e1->destroy(e1);
		e2->destroy(e2);

		derived->destroy_offset(derived, offsetof(traffic_selector_t, destroy));
	}

	/* remove any redundant traffic selectors in the list */
	e1 = result->create_enumerator(result);
	e2 = result->create_enumerator(result);
	while (e1->enumerate(e1, &ts1))
	{
		while (e2->enumerate(e2, &ts2))
		{
			if (ts1 != ts2)
			{
				if (ts2->is_contained_in(ts2, ts1))
				{
					result->remove_at(result, e2);
					ts2->destroy(ts2);
					result->reset_enumerator(result, e1);
					break;
				}
				if (ts1->is_contained_in(ts1, ts2))
				{
					result->remove_at(result, e1);
					ts1->destroy(ts1);
					break;
				}
			}
		}
		result->reset_enumerator(result, e2);
	}
	e1->destroy(e1);
	e2->destroy(e2);

	return result;
}

METHOD(child_cfg_t, get_updown, char*,
	private_child_cfg_t *this)
{
	return this->updown;
}

METHOD(child_cfg_t, get_hostaccess, bool,
	private_child_cfg_t *this)
{
	return this->hostaccess;
}

/**
 * Applies jitter to the rekey value. Returns the new rekey value.
 * Note: The distribution of random values is not perfect, but it
 * should get the job done.
 */
static u_int64_t apply_jitter(u_int64_t rekey, u_int64_t jitter)
{
	if (jitter == 0)
	{
		return rekey;
	}
	jitter = (jitter == UINT64_MAX) ? jitter : jitter + 1;
	return rekey - jitter * (random() / (RAND_MAX + 1.0));
}
#define APPLY_JITTER(l) l.rekey = apply_jitter(l.rekey, l.jitter)

METHOD(child_cfg_t, get_lifetime, lifetime_cfg_t*,
	private_child_cfg_t *this)
{
	lifetime_cfg_t *lft = malloc_thing(lifetime_cfg_t);
	memcpy(lft, &this->lifetime, sizeof(lifetime_cfg_t));
	APPLY_JITTER(lft->time);
	APPLY_JITTER(lft->bytes);
	APPLY_JITTER(lft->packets);
	return lft;
}

METHOD(child_cfg_t, get_mode, ipsec_mode_t,
	private_child_cfg_t *this)
{
	return this->mode;
}

METHOD(child_cfg_t, get_start_action, action_t,
	private_child_cfg_t *this)
{
	return this->start_action;
}

METHOD(child_cfg_t, get_dpd_action, action_t,
	private_child_cfg_t *this)
{
	return this->dpd_action;
}

METHOD(child_cfg_t, get_close_action, action_t,
	private_child_cfg_t *this)
{
	return this->close_action;
}

METHOD(child_cfg_t, get_dh_group, diffie_hellman_group_t,
	private_child_cfg_t *this)
{
	enumerator_t *enumerator;
	proposal_t *proposal;
	u_int16_t dh_group = MODP_NONE;

	enumerator = this->proposals->create_enumerator(this->proposals);
	while (enumerator->enumerate(enumerator, &proposal))
	{
		if (proposal->get_algorithm(proposal, DIFFIE_HELLMAN_GROUP, &dh_group, NULL))
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	return dh_group;
}

METHOD(child_cfg_t, use_ipcomp, bool,
	private_child_cfg_t *this)
{
	return this->use_ipcomp;
}

METHOD(child_cfg_t, get_inactivity, u_int32_t,
	private_child_cfg_t *this)
{
	return this->inactivity;
}

METHOD(child_cfg_t, get_reqid, u_int32_t,
	private_child_cfg_t *this)
{
	return this->reqid;
}

METHOD(child_cfg_t, get_mark, mark_t,
	private_child_cfg_t *this, bool inbound)
{
	return inbound ? this->mark_in : this->mark_out;
}

METHOD(child_cfg_t, get_tfc, u_int32_t,
	private_child_cfg_t *this)
{
	return this->tfc;
}

METHOD(child_cfg_t, get_replay_window, u_int32_t,
	private_child_cfg_t *this)
{
	return this->replay_window;
}

METHOD(child_cfg_t, set_replay_window, void,
	private_child_cfg_t *this, u_int32_t replay_window)
{
	this->replay_window = replay_window;
}

METHOD(child_cfg_t, set_mipv6_options, void,
	private_child_cfg_t *this, bool proxy_mode, bool install_policy)
{
	this->proxy_mode = proxy_mode;
	this->install_policy = install_policy;
}

METHOD(child_cfg_t, use_proxy_mode, bool,
	private_child_cfg_t *this)
{
	return this->proxy_mode;
}

METHOD(child_cfg_t, install_policy, bool,
	private_child_cfg_t *this)
{
	return this->install_policy;
}

METHOD(child_cfg_t, get_ref, child_cfg_t*,
	private_child_cfg_t *this)
{
	ref_get(&this->refcount);
	return &this->public;
}

METHOD(child_cfg_t, destroy, void,
	private_child_cfg_t *this)
{
	if (ref_put(&this->refcount))
	{
		this->proposals->destroy_offset(this->proposals, offsetof(proposal_t, destroy));
		this->my_ts->destroy_offset(this->my_ts, offsetof(traffic_selector_t, destroy));
		this->other_ts->destroy_offset(this->other_ts, offsetof(traffic_selector_t, destroy));
		if (this->updown)
		{
			free(this->updown);
		}
		free(this->name);
		free(this);
	}
}

/*
 * Described in header-file
 */
child_cfg_t *child_cfg_create(char *name, lifetime_cfg_t *lifetime,
							  char *updown, bool hostaccess,
							  ipsec_mode_t mode, action_t start_action,
							  action_t dpd_action, action_t close_action,
							  bool ipcomp, u_int32_t inactivity, u_int32_t reqid,
							  mark_t *mark_in, mark_t *mark_out, u_int32_t tfc)
{
	private_child_cfg_t *this;

	INIT(this,
		.public = {
			.get_name = _get_name,
			.add_traffic_selector = _add_traffic_selector,
			.get_traffic_selectors = _get_traffic_selectors,
			.add_proposal = _add_proposal,
			.get_proposals = _get_proposals,
			.select_proposal = _select_proposal,
			.get_updown = _get_updown,
			.get_hostaccess = _get_hostaccess,
			.get_mode = _get_mode,
			.get_start_action = _get_start_action,
			.get_dpd_action = _get_dpd_action,
			.get_close_action = _get_close_action,
			.get_lifetime = _get_lifetime,
			.get_dh_group = _get_dh_group,
			.set_mipv6_options = _set_mipv6_options,
			.use_ipcomp = _use_ipcomp,
			.get_inactivity = _get_inactivity,
			.get_reqid = _get_reqid,
			.get_mark = _get_mark,
			.get_tfc = _get_tfc,
			.get_replay_window = _get_replay_window,
			.set_replay_window = _set_replay_window,
			.use_proxy_mode = _use_proxy_mode,
			.install_policy = _install_policy,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.name = strdup(name),
		.updown = strdupnull(updown),
		.hostaccess = hostaccess,
		.mode = mode,
		.start_action = start_action,
		.dpd_action = dpd_action,
		.close_action = close_action,
		.use_ipcomp = ipcomp,
		.inactivity = inactivity,
		.reqid = reqid,
		.proxy_mode = FALSE,
		.install_policy = TRUE,
		.refcount = 1,
		.proposals = linked_list_create(),
		.my_ts = linked_list_create(),
		.other_ts = linked_list_create(),
		.tfc = tfc,
		.replay_window = lib->settings->get_int(lib->settings,
				"%s.replay_window", DEFAULT_REPLAY_WINDOW, lib->ns),
	);

	if (mark_in)
	{
		this->mark_in = *mark_in;
	}
	if (mark_out)
	{
		this->mark_out = *mark_out;
	}
	memcpy(&this->lifetime, lifetime, sizeof(lifetime_cfg_t));

	return &this->public;
}
