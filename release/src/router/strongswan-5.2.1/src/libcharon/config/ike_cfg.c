/*
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

#define _GNU_SOURCE /* for stdndup() */
#include <string.h>

#include "ike_cfg.h"

#include <daemon.h>

ENUM(ike_version_names, IKE_ANY, IKEV2,
	"IKEv1/2",
	"IKEv1",
	"IKEv2",
);

typedef struct private_ike_cfg_t private_ike_cfg_t;

/**
 * Private data of an ike_cfg_t object
 */
struct private_ike_cfg_t {

	/**
	 * Public part
	 */
	ike_cfg_t public;

	/**
	 * Number of references hold by others to this ike_cfg
	 */
	refcount_t refcount;

	/**
	 * IKE version to use
	 */
	ike_version_t version;

	/**
	 * Address list string for local host
	 */
	char *me;

	/**
	 * Address list string for remote host
	 */
	char *other;

	/**
	 * Local single host or DNS names, as allocated char*
	 */
	linked_list_t *my_hosts;

	/**
	 * Remote single host or DNS names, as allocated char*
	 */
	linked_list_t *other_hosts;

	/**
	 * Local ranges/subnets this config matches to, as traffic_selector_t*
	 */
	linked_list_t *my_ranges;

	/**
	 * Remote ranges/subnets this config matches to, as traffic_selector_t*
	 */
	linked_list_t *other_ranges;

	/**
	 * our source port
	 */
	u_int16_t my_port;

	/**
	 * destination port
	 */
	u_int16_t other_port;

	/**
	 * should we send a certificate request?
	 */
	bool certreq;

	/**
	 * enforce UDP encapsulation
	 */
	bool force_encap;

	/**
	 * use IKEv1 fragmentation
	 */
	fragmentation_t fragmentation;

	/**
	 * DSCP value to use on sent IKE packets
	 */
	u_int8_t dscp;

	/**
	 * List of proposals to use
	 */
	linked_list_t *proposals;
};

METHOD(ike_cfg_t, get_version, ike_version_t,
	private_ike_cfg_t *this)
{
	return this->version;
}

METHOD(ike_cfg_t, send_certreq, bool,
	private_ike_cfg_t *this)
{
	return this->certreq;
}

METHOD(ike_cfg_t, force_encap_, bool,
	private_ike_cfg_t *this)
{
	return this->force_encap;
}

METHOD(ike_cfg_t, fragmentation, fragmentation_t,
	private_ike_cfg_t *this)
{
	return this->fragmentation;
}

/**
 * Common function for resolve_me/other
 */
static host_t* resolve(linked_list_t *hosts, int family, u_int16_t port)
{
	enumerator_t *enumerator;
	host_t *host = NULL;
	bool tried = FALSE;
	char *str;

	enumerator = hosts->create_enumerator(hosts);
	while (enumerator->enumerate(enumerator, &str))
	{
		host = host_create_from_dns(str, family, port);
		if (host)
		{
			break;
		}
		tried = TRUE;
	}
	enumerator->destroy(enumerator);

	if (!host && !tried)
	{
		/* we have no single host configured, return %any */
		host = host_create_any(family ?: AF_INET);
		host->set_port(host, port);
	}
	return host;
}

METHOD(ike_cfg_t, resolve_me, host_t*,
	private_ike_cfg_t *this, int family)
{
	return resolve(this->my_hosts, family, this->my_port);
}

METHOD(ike_cfg_t, resolve_other, host_t*,
	private_ike_cfg_t *this, int family)
{
	return resolve(this->other_hosts, family, this->other_port);
}

/**
 * Common function for match_me/other
 */
static u_int match(linked_list_t *hosts, linked_list_t *ranges, host_t *cand)
{
	enumerator_t *enumerator;
	traffic_selector_t *ts;
	char *str;
	host_t *host;
	u_int8_t mask;
	u_int quality = 0;

	/* try single hosts first */
	enumerator = hosts->create_enumerator(hosts);
	while (enumerator->enumerate(enumerator, &str))
	{
		host = host_create_from_dns(str, cand->get_family(cand), 0);
		if (host)
		{
			if (host->ip_equals(host, cand))
			{
				quality = max(quality, 128 + 1);
			}
			if (host->is_anyaddr(host))
			{
				quality = max(quality, 1);
			}
			host->destroy(host);
		}
	}
	enumerator->destroy(enumerator);

	/* then ranges/subnets */
	enumerator = ranges->create_enumerator(ranges);
	while (enumerator->enumerate(enumerator, &ts))
	{
		if (ts->includes(ts, cand))
		{
			if (ts->to_subnet(ts, &host, &mask))
			{
				quality = max(quality, mask + 1);
				host->destroy(host);
			}
			else
			{
				quality = max(quality, 1);
			}
		}
	}
	enumerator->destroy(enumerator);

	return quality;
}

METHOD(ike_cfg_t, match_me, u_int,
	private_ike_cfg_t *this, host_t *host)
{
	return match(this->my_hosts, this->my_ranges, host);
}

METHOD(ike_cfg_t, match_other, u_int,
	private_ike_cfg_t *this, host_t *host)
{
	return match(this->other_hosts, this->other_ranges, host);
}

METHOD(ike_cfg_t, get_my_addr, char*,
	private_ike_cfg_t *this)
{
	return this->me;
}

METHOD(ike_cfg_t, get_other_addr, char*,
	private_ike_cfg_t *this)
{
	return this->other;
}

METHOD(ike_cfg_t, get_my_port, u_int16_t,
	private_ike_cfg_t *this)
{
	return this->my_port;
}

METHOD(ike_cfg_t, get_other_port, u_int16_t,
	private_ike_cfg_t *this)
{
	return this->other_port;
}

METHOD(ike_cfg_t, get_dscp, u_int8_t,
	private_ike_cfg_t *this)
{
	return this->dscp;
}

METHOD(ike_cfg_t, add_proposal, void,
	private_ike_cfg_t *this, proposal_t *proposal)
{
	if (proposal)
	{
		this->proposals->insert_last(this->proposals, proposal);
	}
}

METHOD(ike_cfg_t, get_proposals, linked_list_t*,
	private_ike_cfg_t *this)
{
	enumerator_t *enumerator;
	proposal_t *current;
	linked_list_t *proposals;

	proposals = linked_list_create();
	enumerator = this->proposals->create_enumerator(this->proposals);
	while (enumerator->enumerate(enumerator, &current))
	{
		current = current->clone(current);
		proposals->insert_last(proposals, current);
	}
	enumerator->destroy(enumerator);

	DBG2(DBG_CFG, "configured proposals: %#P", proposals);

	return proposals;
}

METHOD(ike_cfg_t, select_proposal, proposal_t*,
	private_ike_cfg_t *this, linked_list_t *proposals, bool private)
{
	enumerator_t *stored_enum, *supplied_enum;
	proposal_t *stored, *supplied, *selected;

	stored_enum = this->proposals->create_enumerator(this->proposals);
	supplied_enum = proposals->create_enumerator(proposals);


	/* compare all stored proposals with all supplied. Stored ones are preferred.*/
	while (stored_enum->enumerate(stored_enum, (void**)&stored))
	{
		proposals->reset_enumerator(proposals, supplied_enum);

		while (supplied_enum->enumerate(supplied_enum, (void**)&supplied))
		{
			selected = stored->select(stored, supplied, private);
			if (selected)
			{
				/* they match, return */
				stored_enum->destroy(stored_enum);
				supplied_enum->destroy(supplied_enum);
				DBG2(DBG_CFG, "received proposals: %#P", proposals);
				DBG2(DBG_CFG, "configured proposals: %#P", this->proposals);
				DBG2(DBG_CFG, "selected proposal: %P", selected);
				return selected;
			}
		}
	}
	/* no proposal match :-(, will result in a NO_PROPOSAL_CHOSEN... */
	stored_enum->destroy(stored_enum);
	supplied_enum->destroy(supplied_enum);
	DBG1(DBG_CFG, "received proposals: %#P", proposals);
	DBG1(DBG_CFG, "configured proposals: %#P", this->proposals);

	return NULL;
}

METHOD(ike_cfg_t, get_dh_group, diffie_hellman_group_t,
	private_ike_cfg_t *this)
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

METHOD(ike_cfg_t, equals, bool,
	private_ike_cfg_t *this, ike_cfg_t *other_public)
{
	private_ike_cfg_t *other = (private_ike_cfg_t*)other_public;
	enumerator_t *e1, *e2;
	proposal_t *p1, *p2;
	bool eq = TRUE;

	if (this == other)
	{
		return TRUE;
	}
	if (this->public.equals != other->public.equals)
	{
		return FALSE;
	}
	if (this->proposals->get_count(this->proposals) !=
		other->proposals->get_count(other->proposals))
	{
		return FALSE;
	}
	e1 = this->proposals->create_enumerator(this->proposals);
	e2 = other->proposals->create_enumerator(other->proposals);
	while (e1->enumerate(e1, &p1) && e2->enumerate(e2, &p2))
	{
		if (!p1->equals(p1, p2))
		{
			eq = FALSE;
			break;
		}
	}
	e1->destroy(e1);
	e2->destroy(e2);

	return (eq &&
		this->version == other->version &&
		this->certreq == other->certreq &&
		this->force_encap == other->force_encap &&
		this->fragmentation == other->fragmentation &&
		streq(this->me, other->me) &&
		streq(this->other, other->other) &&
		this->my_port == other->my_port &&
		this->other_port == other->other_port);
}

METHOD(ike_cfg_t, get_ref, ike_cfg_t*,
	private_ike_cfg_t *this)
{
	ref_get(&this->refcount);
	return &this->public;
}

METHOD(ike_cfg_t, destroy, void,
	private_ike_cfg_t *this)
{
	if (ref_put(&this->refcount))
	{
		this->proposals->destroy_offset(this->proposals,
										offsetof(proposal_t, destroy));
		free(this->me);
		free(this->other);
		this->my_hosts->destroy_function(this->my_hosts, free);
		this->other_hosts->destroy_function(this->other_hosts, free);
		this->my_ranges->destroy_offset(this->my_ranges,
										offsetof(traffic_selector_t, destroy));
		this->other_ranges->destroy_offset(this->other_ranges,
										offsetof(traffic_selector_t, destroy));
		free(this);
	}
}

/**
 * Try to parse a string as subnet
 */
static traffic_selector_t* make_subnet(char *str)
{
	char *pos;

	pos = strchr(str, '/');
	if (!pos)
	{
		return NULL;
	}
	return traffic_selector_create_from_cidr(str, 0, 0, 0);
}

/**
 * Try to parse a string as an IP range
 */
static traffic_selector_t* make_range(char *str)
{
	traffic_selector_t *ts;
	ts_type_t type;
	char *pos;
	host_t *from, *to;

	pos = strchr(str, '-');
	if (!pos)
	{
		return NULL;
	}
	to = host_create_from_string(pos + 1, 0);
	if (!to)
	{
		return NULL;
	}
	str = strndup(str, pos - str);
	from = host_create_from_string_and_family(str, to->get_family(to), 0);
	free(str);
	if (!from)
	{
		to->destroy(to);
		return NULL;
	}
	if (to->get_family(to) == AF_INET)
	{
		type = TS_IPV4_ADDR_RANGE;
	}
	else
	{
		type = TS_IPV6_ADDR_RANGE;
	}
	ts = traffic_selector_create_from_bytes(0, type,
											from->get_address(from), 0,
											to->get_address(to), 0);
	from->destroy(from);
	to->destroy(to);
	return ts;
}

/**
 * Parse address string into lists of single hosts and ranges/subnets
 */
static void parse_addresses(char *str, linked_list_t *hosts,
							linked_list_t *ranges)
{
	enumerator_t *enumerator;
	traffic_selector_t *ts;

	enumerator = enumerator_create_token(str, ",", " ");
	while (enumerator->enumerate(enumerator, &str))
	{
		ts = make_subnet(str);
		if (ts)
		{
			ranges->insert_last(ranges, ts);
			continue;
		}
		ts = make_range(str);
		if (ts)
		{
			ranges->insert_last(ranges, ts);
			continue;
		}
		hosts->insert_last(hosts, strdup(str));
	}
	enumerator->destroy(enumerator);
}

/**
 * Described in header.
 */
ike_cfg_t *ike_cfg_create(ike_version_t version, bool certreq, bool force_encap,
						  char *me, u_int16_t my_port,
						  char *other, u_int16_t other_port,
						  fragmentation_t fragmentation, u_int8_t dscp)
{
	private_ike_cfg_t *this;

	INIT(this,
		.public = {
			.get_version = _get_version,
			.send_certreq = _send_certreq,
			.force_encap = _force_encap_,
			.fragmentation = _fragmentation,
			.resolve_me = _resolve_me,
			.resolve_other = _resolve_other,
			.match_me = _match_me,
			.match_other = _match_other,
			.get_my_addr = _get_my_addr,
			.get_other_addr = _get_other_addr,
			.get_my_port = _get_my_port,
			.get_other_port = _get_other_port,
			.get_dscp = _get_dscp,
			.add_proposal = _add_proposal,
			.get_proposals = _get_proposals,
			.select_proposal = _select_proposal,
			.get_dh_group = _get_dh_group,
			.equals = _equals,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.refcount = 1,
		.version = version,
		.certreq = certreq,
		.force_encap = force_encap,
		.fragmentation = fragmentation,
		.me = strdup(me),
		.my_ranges = linked_list_create(),
		.my_hosts = linked_list_create(),
		.other = strdup(other),
		.other_ranges = linked_list_create(),
		.other_hosts = linked_list_create(),
		.my_port = my_port,
		.other_port = other_port,
		.dscp = dscp,
		.proposals = linked_list_create(),
	);

	parse_addresses(me, this->my_hosts, this->my_ranges);
	parse_addresses(other, this->other_hosts, this->other_ranges);

	return &this->public;
}
