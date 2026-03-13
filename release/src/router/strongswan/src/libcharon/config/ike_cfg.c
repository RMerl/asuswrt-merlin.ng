/*
 * Copyright (C) 2012-2019 Tobias Brunner
 * Copyright (C) 2005-2007 Martin Willi
 * Copyright (C) 2005 Jan Hutter
 *
 * Copyright (C) secunet Security Networks AG
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
	uint16_t my_port;

	/**
	 * destination port
	 */
	uint16_t other_port;

	/**
	 * should we send a certificate request?
	 */
	bool certreq;

	/**
	 * should we send an OCSP status request?
	 */
	bool ocsp_certreq;

	/**
	 * enforce UDP encapsulation
	 */
	bool force_encap;

	/**
	 * use IKE fragmentation
	 */
	fragmentation_t fragmentation;

	/**
	 * childless IKE_SAs
	 */
	childless_t childless;

	/**
	 * DSCP value to use on sent IKE packets
	 */
	uint8_t dscp;

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

METHOD(ike_cfg_t, send_ocsp_certreq, bool,
	private_ike_cfg_t *this)
{
	return this->ocsp_certreq;
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

METHOD(ike_cfg_t, childless, childless_t,
	private_ike_cfg_t *this)
{
	return this->childless;
}

/**
 * Common function for resolve_me/other
 */
static host_t* resolve(linked_list_t *hosts, int family, uint16_t port)
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
static u_int match(linked_list_t *hosts, linked_list_t *ranges, uint16_t port,
		host_t *cand)
{
	enumerator_t *enumerator;
	traffic_selector_t *ts;
	char *str;
	host_t *host;
	uint8_t mask;
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
			else if (host->is_anyaddr(host))
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
			}
			else
			{
				quality = max(quality, 1);
			}
			host->destroy(host);
		}
	}
	enumerator->destroy(enumerator);

	/* honor if port matches exactly */
	if (quality && port == cand->get_port(cand))
	{
		quality += 1;
	}
	return quality;
}

METHOD(ike_cfg_t, match_me, u_int,
	private_ike_cfg_t *this, host_t *host)
{
	return match(this->my_hosts, this->my_ranges, this->my_port, host);
}

METHOD(ike_cfg_t, match_other, u_int,
	private_ike_cfg_t *this, host_t *host)
{
	return match(this->other_hosts, this->other_ranges, this->other_port, host);
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

METHOD(ike_cfg_t, get_my_port, uint16_t,
	private_ike_cfg_t *this)
{
	return this->my_port;
}

METHOD(ike_cfg_t, get_other_port, uint16_t,
	private_ike_cfg_t *this)
{
	return this->other_port;
}

METHOD(ike_cfg_t, get_dscp, uint8_t,
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
		current = current->clone(current, 0);
		proposals->insert_last(proposals, current);
	}
	enumerator->destroy(enumerator);

	DBG2(DBG_CFG, "configured proposals: %#P", proposals);

	return proposals;
}

METHOD(ike_cfg_t, has_proposal, bool,
	private_ike_cfg_t *this, proposal_t *match, bool private)
{
	enumerator_t *enumerator;
	proposal_t *proposal;

	enumerator = this->proposals->create_enumerator(this->proposals);
	while (enumerator->enumerate(enumerator, &proposal))
	{
		if (proposal->matches(proposal, match,
							  private ? 0 : PROPOSAL_SKIP_PRIVATE))
		{
			enumerator->destroy(enumerator);
			return TRUE;
		}
	}
	enumerator->destroy(enumerator);
	return FALSE;
}

METHOD(ike_cfg_t, select_proposal, proposal_t*,
	private_ike_cfg_t *this, linked_list_t *proposals,
	proposal_selection_flag_t flags)
{
	return proposal_select(this->proposals, proposals, flags);
}

METHOD(ike_cfg_t, get_algorithm, uint16_t,
	private_ike_cfg_t *this, transform_type_t type)
{
	enumerator_t *enumerator;
	proposal_t *proposal;
	uint16_t alg = 0;

	enumerator = this->proposals->create_enumerator(this->proposals);
	while (enumerator->enumerate(enumerator, &proposal))
	{
		if (proposal->get_algorithm(proposal, type, &alg, NULL))
		{
			break;
		}
	}
	enumerator->destroy(enumerator);
	return alg;
}

METHOD(ike_cfg_t, equals, bool,
	private_ike_cfg_t *this, ike_cfg_t *other_public)
{
	private_ike_cfg_t *other = (private_ike_cfg_t*)other_public;

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
	return
		this->version == other->version &&
		this->certreq == other->certreq &&
		this->ocsp_certreq == other->ocsp_certreq &&
		this->force_encap == other->force_encap &&
		this->fragmentation == other->fragmentation &&
		this->childless == other->childless &&
		streq(this->me, other->me) &&
		streq(this->other, other->other) &&
		this->my_port == other->my_port &&
		this->other_port == other->other_port;
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
	host_t *from, *to;

	if (!host_create_from_range(str, &from, &to))
	{
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
int ike_cfg_get_family(ike_cfg_t *cfg, bool local)
{
	private_ike_cfg_t *this = (private_ike_cfg_t*)cfg;
	enumerator_t *enumerator;
	host_t *host;
	char *str;
	int family = AF_UNSPEC;

	if (local)
	{
		enumerator = this->my_hosts->create_enumerator(this->my_hosts);
	}
	else
	{
		enumerator = this->other_hosts->create_enumerator(this->other_hosts);
	}
	while (enumerator->enumerate(enumerator, &str))
	{
		if (streq(str, "%any"))
		{	/* ignore %any as its family is undetermined */
			continue;
		}
		host = host_create_from_string(str, 0);
		if (host)
		{
			if (family == AF_UNSPEC)
			{
				family = host->get_family(host);
			}
			else if (family != host->get_family(host))
			{
				/* more than one address family defined */
				family = AF_UNSPEC;
				host->destroy(host);
				break;
			}
		}
		DESTROY_IF(host);
	}
	enumerator->destroy(enumerator);
	return family;
}

/**
 * Described in header.
 */
bool ike_cfg_has_address(ike_cfg_t *cfg, host_t *addr, bool local)
{
	private_ike_cfg_t *this = (private_ike_cfg_t*)cfg;
	enumerator_t *enumerator;
	host_t *host;
	char *str;
	bool found = FALSE;

	if (local)
	{
		enumerator = this->my_hosts->create_enumerator(this->my_hosts);
	}
	else
	{
		enumerator = this->other_hosts->create_enumerator(this->other_hosts);
	}
	while (enumerator->enumerate(enumerator, &str))
	{
		host = host_create_from_string(str, 0);
		if (host && addr->ip_equals(addr, host))
		{
			host->destroy(host);
			found = TRUE;
			break;
		}
		DESTROY_IF(host);
	}
	enumerator->destroy(enumerator);
	return found;
}

/*
 * Described in header
 */
ike_cfg_t *ike_cfg_create(ike_cfg_create_t *data)
{
	private_ike_cfg_t *this;

	INIT(this,
		.public = {
			.get_version = _get_version,
			.send_certreq = _send_certreq,
			.send_ocsp_certreq = _send_ocsp_certreq,
			.force_encap = _force_encap_,
			.fragmentation = _fragmentation,
			.childless = _childless,
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
			.has_proposal = _has_proposal,
			.get_algorithm = _get_algorithm,
			.equals = _equals,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.refcount = 1,
		.version = data->version,
		.certreq = !data->no_certreq,
		.ocsp_certreq = data->ocsp_certreq,
		.force_encap = data->force_encap,
		.fragmentation = data->fragmentation,
		.childless = data->childless,
		.me = strdup(data->local),
		.my_ranges = linked_list_create(),
		.my_hosts = linked_list_create(),
		.other = strdup(data->remote),
		.other_ranges = linked_list_create(),
		.other_hosts = linked_list_create(),
		.my_port = data->local_port,
		.other_port = data->remote_port,
		.dscp = data->dscp,
		.proposals = linked_list_create(),
	);

	parse_addresses(data->local, this->my_hosts, this->my_ranges);
	parse_addresses(data->remote, this->other_hosts, this->other_ranges);

	return &this->public;
}
