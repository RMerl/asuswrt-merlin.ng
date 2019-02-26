/*
 * Copyright (C) 2008-2018 Tobias Brunner
 * Copyright (C) 2016 Andreas Steffen
 * Copyright (C) 2005-2007 Martin Willi
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
	 * Options
	 */
	child_cfg_option_t options;

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
	 * Inactivity timeout
	 */
	uint32_t inactivity;

	/**
	 * Reqid to install CHILD_SA with
	 */
	uint32_t reqid;

	/**
	 * Optional mark to install inbound CHILD_SA with
	 */
	mark_t mark_in;

	/**
	 * Optional mark to install outbound CHILD_SA with
	 */
	mark_t mark_out;

	/**
	 * Optional mark to set to packets after inbound processing
	 */
	mark_t set_mark_in;

	/**
	 * Optional mark to set to packets after outbound processing
	 */
	mark_t set_mark_out;

	/**
	 * Traffic Flow Confidentiality padding, if enabled
	 */
	uint32_t tfc;

	/**
	 * Optional manually-set IPsec policy priorities
	 */
	uint32_t manual_prio;

	/**
	 * Optional restriction of IPsec policy to a given network interface
	 */
	char *interface;

	/**
	 * anti-replay window size
	 */
	uint32_t replay_window;

	/**
	 * HW offload mode
	 */
	hw_offload_t hw_offload;

	/**
	 * DS header field copy mode
	 */
	dscp_copy_t copy_dscp;
};

METHOD(child_cfg_t, get_name, char*,
	private_child_cfg_t *this)
{
	return this->name;
}

METHOD(child_cfg_t, has_option, bool,
	private_child_cfg_t *this, child_cfg_option_t option)
{
	return this->options & option;
}

METHOD(child_cfg_t, add_proposal, void,
	private_child_cfg_t *this, proposal_t *proposal)
{
	if (proposal)
	{
		this->proposals->insert_last(this->proposals, proposal);
	}
}

CALLBACK(match_proposal, bool,
	proposal_t *item, va_list args)
{
	proposal_t *proposal;

	VA_ARGS_VGET(args, proposal);
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
		if (proposals->find_first(proposals, match_proposal, NULL, current))
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
	bool private, bool prefer_self)
{
	enumerator_t *prefer_enum, *match_enum;
	proposal_t *proposal, *match, *selected = NULL;

	if (prefer_self)
	{
		prefer_enum = this->proposals->create_enumerator(this->proposals);
		match_enum = proposals->create_enumerator(proposals);
	}
	else
	{
		prefer_enum = proposals->create_enumerator(proposals);
		match_enum = this->proposals->create_enumerator(this->proposals);
	}

	while (prefer_enum->enumerate(prefer_enum, &proposal))
	{
		proposal = proposal->clone(proposal);
		if (strip_dh)
		{
			proposal->strip_dh(proposal, MODP_NONE);
		}
		if (prefer_self)
		{
			proposals->reset_enumerator(proposals, match_enum);
		}
		else
		{
			this->proposals->reset_enumerator(this->proposals, match_enum);
		}
		while (match_enum->enumerate(match_enum, &match))
		{
			match = match->clone(match);
			if (strip_dh)
			{
				match->strip_dh(match, MODP_NONE);
			}
			selected = proposal->select(proposal, match, prefer_self, private);
			match->destroy(match);
			if (selected)
			{
				DBG2(DBG_CFG, "received proposals: %#P", proposals);
				DBG2(DBG_CFG, "configured proposals: %#P", this->proposals);
				DBG1(DBG_CFG, "selected proposal: %P", selected);
				break;
			}
		}
		proposal->destroy(proposal);
		if (selected)
		{
			break;
		}
	}
	prefer_enum->destroy(prefer_enum);
	match_enum->destroy(match_enum);
	if (!selected)
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
	linked_list_t *hosts, bool log)
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
	/* in a first step, replace "dynamic" TS with the host list */
	while (e1->enumerate(e1, &ts1))
	{
		if (hosts && hosts->get_count(hosts))
		{	/* set hosts if TS is dynamic or as initiator in transport mode */
			bool dynamic = ts1->is_dynamic(ts1),
				 proxy_mode = has_option(this, OPT_PROXY_MODE);
			if (dynamic || (this->mode == MODE_TRANSPORT && !proxy_mode &&
							!supplied))
			{
				e2 = hosts->create_enumerator(hosts);
				while (e2->enumerate(e2, &host))
				{
					ts2 = ts1->clone(ts1);
					if (dynamic || !host->is_anyaddr(host))
					{	/* don't make regular TS larger than they were */
						ts2->set_address(ts2, host);
					}
					derived->insert_last(derived, ts2);
				}
				e2->destroy(e2);
				continue;
			}
		}
		derived->insert_last(derived, ts1->clone(ts1));
	}
	e1->destroy(e1);

	if (log)
	{
		DBG2(DBG_CFG, "%s traffic selectors for %s:",
			 supplied ? "selecting" : "proposing", local ? "us" : "other");
	}
	if (!supplied)
	{
		while (derived->remove_first(derived, (void**)&ts1) == SUCCESS)
		{
			if (log)
			{
				DBG2(DBG_CFG, " %R", ts1);
			}
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
					if (log)
					{
						DBG2(DBG_CFG, " config: %R, received: %R => match: %R",
							 ts1, ts2, selected);
					}
					result->insert_last(result, selected);
				}
				else if (log)
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

/**
 * Applies jitter to the rekey value. Returns the new rekey value.
 * Note: The distribution of random values is not perfect, but it
 * should get the job done.
 */
static uint64_t apply_jitter(uint64_t rekey, uint64_t jitter)
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
	private_child_cfg_t *this, bool jitter)
{
	lifetime_cfg_t *lft = malloc_thing(lifetime_cfg_t);
	memcpy(lft, &this->lifetime, sizeof(lifetime_cfg_t));
	if (!jitter)
	{
		lft->time.jitter = lft->bytes.jitter = lft->packets.jitter = 0;
	}
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

METHOD(child_cfg_t, get_hw_offload, hw_offload_t,
	private_child_cfg_t *this)
{
	return this->hw_offload;
}

METHOD(child_cfg_t, get_copy_dscp, dscp_copy_t,
	private_child_cfg_t *this)
{
	return this->copy_dscp;
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
	uint16_t dh_group = MODP_NONE;

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

METHOD(child_cfg_t, get_inactivity, uint32_t,
	private_child_cfg_t *this)
{
	return this->inactivity;
}

METHOD(child_cfg_t, get_reqid, uint32_t,
	private_child_cfg_t *this)
{
	return this->reqid;
}

METHOD(child_cfg_t, get_mark, mark_t,
	private_child_cfg_t *this, bool inbound)
{
	return inbound ? this->mark_in : this->mark_out;
}

METHOD(child_cfg_t, get_set_mark, mark_t,
	private_child_cfg_t *this, bool inbound)
{
	return inbound ? this->set_mark_in : this->set_mark_out;
}

METHOD(child_cfg_t, get_tfc, uint32_t,
	private_child_cfg_t *this)
{
	return this->tfc;
}

METHOD(child_cfg_t, get_manual_prio, uint32_t,
	private_child_cfg_t *this)
{
	return this->manual_prio;
}

METHOD(child_cfg_t, get_interface, char*,
	private_child_cfg_t *this)
{
	return this->interface;
}

METHOD(child_cfg_t, get_replay_window, uint32_t,
	private_child_cfg_t *this)
{
	return this->replay_window;
}

METHOD(child_cfg_t, set_replay_window, void,
	private_child_cfg_t *this, uint32_t replay_window)
{
	this->replay_window = replay_window;
}

#define LT_PART_EQUALS(a, b) ({ a.life == b.life && a.rekey == b.rekey && a.jitter == b.jitter; })
#define LIFETIME_EQUALS(a, b) ({ LT_PART_EQUALS(a.time, b.time) && LT_PART_EQUALS(a.bytes, b.bytes) && LT_PART_EQUALS(a.packets, b.packets); })

METHOD(child_cfg_t, equals, bool,
	private_child_cfg_t *this, child_cfg_t *other_pub)
{
	private_child_cfg_t *other = (private_child_cfg_t*)other_pub;

	if (this == other)
	{
		return TRUE;
	}
	if (this->public.equals != other->public.equals)
	{
		return FALSE;
	}
	if (!this->proposals->equals_offset(this->proposals, other->proposals,
										offsetof(proposal_t, equals)))
	{
		return FALSE;
	}
	if (!this->my_ts->equals_offset(this->my_ts, other->my_ts,
									offsetof(traffic_selector_t, equals)))
	{
		return FALSE;
	}
	if (!this->other_ts->equals_offset(this->other_ts, other->other_ts,
									   offsetof(traffic_selector_t, equals)))
	{
		return FALSE;
	}
	return this->options == other->options &&
		this->mode == other->mode &&
		this->start_action == other->start_action &&
		this->dpd_action == other->dpd_action &&
		this->close_action == other->close_action &&
		LIFETIME_EQUALS(this->lifetime, other->lifetime) &&
		this->inactivity == other->inactivity &&
		this->reqid == other->reqid &&
		this->mark_in.value == other->mark_in.value &&
		this->mark_in.mask == other->mark_in.mask &&
		this->mark_out.value == other->mark_out.value &&
		this->mark_out.mask == other->mark_out.mask &&
		this->set_mark_in.value == other->set_mark_in.value &&
		this->set_mark_in.mask == other->set_mark_in.mask &&
		this->set_mark_out.value == other->set_mark_out.value &&
		this->set_mark_out.mask == other->set_mark_out.mask &&
		this->tfc == other->tfc &&
		this->manual_prio == other->manual_prio &&
		this->replay_window == other->replay_window &&
		this->hw_offload == other->hw_offload &&
		this->copy_dscp == other->copy_dscp &&
		streq(this->updown, other->updown) &&
		streq(this->interface, other->interface);
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
		free(this->updown);
		free(this->interface);
		free(this->name);
		free(this);
	}
}

/*
 * Described in header-file
 */
child_cfg_t *child_cfg_create(char *name, child_cfg_create_t *data)
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
			.get_mode = _get_mode,
			.get_start_action = _get_start_action,
			.get_dpd_action = _get_dpd_action,
			.get_close_action = _get_close_action,
			.get_lifetime = _get_lifetime,
			.get_dh_group = _get_dh_group,
			.get_inactivity = _get_inactivity,
			.get_reqid = _get_reqid,
			.get_mark = _get_mark,
			.get_set_mark = _get_set_mark,
			.get_tfc = _get_tfc,
			.get_manual_prio = _get_manual_prio,
			.get_interface = _get_interface,
			.get_replay_window = _get_replay_window,
			.set_replay_window = _set_replay_window,
			.has_option = _has_option,
			.equals = _equals,
			.get_ref = _get_ref,
			.destroy = _destroy,
			.get_hw_offload = _get_hw_offload,
			.get_copy_dscp = _get_copy_dscp,
		},
		.name = strdup(name),
		.options = data->options,
		.updown = strdupnull(data->updown),
		.reqid = data->reqid,
		.mode = data->mode,
		.start_action = data->start_action,
		.dpd_action = data->dpd_action,
		.close_action = data->close_action,
		.mark_in = data->mark_in,
		.mark_out = data->mark_out,
		.set_mark_in = data->set_mark_in,
		.set_mark_out = data->set_mark_out,
		.lifetime = data->lifetime,
		.inactivity = data->inactivity,
		.tfc = data->tfc,
		.manual_prio = data->priority,
		.interface = strdupnull(data->interface),
		.refcount = 1,
		.proposals = linked_list_create(),
		.my_ts = linked_list_create(),
		.other_ts = linked_list_create(),
		.replay_window = lib->settings->get_int(lib->settings,
							"%s.replay_window", DEFAULT_REPLAY_WINDOW, lib->ns),
		.hw_offload = data->hw_offload,
		.copy_dscp = data->copy_dscp,
	);

	return &this->public;
}
