/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
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

#include "ipsec_policy.h"

#include <utils/debug.h>

typedef struct private_ipsec_policy_t private_ipsec_policy_t;

/**
 * Private additions to ipsec_policy_t.
 */
struct private_ipsec_policy_t {

	/**
	 * Public members
	 */
	ipsec_policy_t public;

	/**
	 * SA source address
	 */
	host_t *src;

	/**
	 * SA destination address
	 */
	host_t *dst;

	/**
	 * Source traffic selector
	 */
	traffic_selector_t *src_ts;

	/**
	 * Destination traffic selector
	 */
	traffic_selector_t *dst_ts;

	/**
	 * If any of the two TS has a protocol selector we cache it here
	 */
	uint8_t protocol;

	/**
	 * Traffic direction
	 */
	policy_dir_t direction;

	/**
	 * Policy type
	 */
	policy_type_t type;

	/**
	 * SA configuration
	 */
	ipsec_sa_cfg_t sa;

	/**
	 * Mark
	 */
	mark_t mark;

	/**
	 * Policy priority
	 */
	policy_priority_t priority;

	/**
	 * Reference counter
	 */
	refcount_t refcount;

};

METHOD(ipsec_policy_t, match, bool,
	private_ipsec_policy_t *this, traffic_selector_t *src_ts,
	traffic_selector_t *dst_ts, policy_dir_t direction, uint32_t reqid,
	mark_t mark, policy_priority_t priority)
{
	return (this->direction == direction &&
			this->priority == priority &&
			this->sa.reqid == reqid &&
			memeq(&this->mark, &mark, sizeof(mark_t)) &&
			this->src_ts->equals(this->src_ts, src_ts) &&
			this->dst_ts->equals(this->dst_ts, dst_ts));
}

/**
 * Match the port of the given host against the given traffic selector.
 */
static inline bool match_port(traffic_selector_t *ts, host_t *host)
{
	uint16_t from, to, port;

	from = ts->get_from_port(ts);
	to = ts->get_to_port(ts);
	if ((from == 0 && to == 0xffff) ||
		(from == 0xffff && to == 0))
	{
		return TRUE;
	}
	port = host->get_port(host);
	return from <= port && port <= to;
}

METHOD(ipsec_policy_t, match_packet, bool,
	private_ipsec_policy_t *this, ip_packet_t *packet)
{
	uint8_t proto = packet->get_next_header(packet);
	host_t *src = packet->get_source(packet),
		   *dst = packet->get_destination(packet);

	return (!this->protocol || this->protocol == proto) &&
		   this->src_ts->includes(this->src_ts, src) &&
		   match_port(this->src_ts, src) &&
		   this->dst_ts->includes(this->dst_ts, dst) &&
		   match_port(this->dst_ts, dst);
}

METHOD(ipsec_policy_t, get_source_ts, traffic_selector_t*,
	private_ipsec_policy_t *this)
{
	return this->src_ts;
}

METHOD(ipsec_policy_t, get_destination_ts, traffic_selector_t*,
	private_ipsec_policy_t *this)
{
	return this->dst_ts;
}

METHOD(ipsec_policy_t, get_reqid, uint32_t,
	private_ipsec_policy_t *this)
{
	return this->sa.reqid;
}

METHOD(ipsec_policy_t, get_direction, policy_dir_t,
	private_ipsec_policy_t *this)
{
	return this->direction;
}

METHOD(ipsec_policy_t, get_priority, policy_priority_t,
	private_ipsec_policy_t *this)
{
	return this->priority;
}

METHOD(ipsec_policy_t, get_type, policy_type_t,
	private_ipsec_policy_t *this)
{
	return this->type;
}

METHOD(ipsec_policy_t, get_ref, ipsec_policy_t*,
	private_ipsec_policy_t *this)
{
	ref_get(&this->refcount);
	return &this->public;
}

METHOD(ipsec_policy_t, destroy, void,
		private_ipsec_policy_t *this)
{
	if (ref_put(&this->refcount))
	{
		this->src->destroy(this->src);
		this->dst->destroy(this->dst);
		this->src_ts->destroy(this->src_ts);
		this->dst_ts->destroy(this->dst_ts);
		free(this);
	}
}

/**
 * Described in header.
 */
ipsec_policy_t *ipsec_policy_create(host_t *src, host_t *dst,
									traffic_selector_t *src_ts,
									traffic_selector_t *dst_ts,
									policy_dir_t direction, policy_type_t type,
									ipsec_sa_cfg_t *sa, mark_t mark,
									policy_priority_t priority)
{
	private_ipsec_policy_t *this;

	INIT(this,
		.public = {
			.match = _match,
			.match_packet = _match_packet,
			.get_source_ts = _get_source_ts,
			.get_destination_ts = _get_destination_ts,
			.get_direction = _get_direction,
			.get_priority = _get_priority,
			.get_reqid = _get_reqid,
			.get_type = _get_type,
			.get_ref = _get_ref,
			.destroy = _destroy,
		},
		.src = src->clone(src),
		.dst = dst->clone(dst),
		.src_ts = src_ts->clone(src_ts),
		.dst_ts = dst_ts->clone(dst_ts),
		.protocol = max(src_ts->get_protocol(src_ts),
						dst_ts->get_protocol(dst_ts)),
		.direction = direction,
		.type = type,
		.sa = *sa,
		.mark = mark,
		.priority = priority,
		.refcount = 1,
	);

	return &this->public;
}
