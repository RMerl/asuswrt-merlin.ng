/*
 * Copyright (C) 2012 Tobias Brunner
 * Copyright (C) 2012 Giuliano Grassi
 * Copyright (C) 2012 Ralf Sager
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

#include "ipsec_policy_mgr.h"

#include <utils/debug.h>
#include <threading/rwlock.h>
#include <collections/linked_list.h>

/** Base priority for installed policies */
#define PRIO_BASE 384

typedef struct private_ipsec_policy_mgr_t private_ipsec_policy_mgr_t;

/**
 * Private additions to ipsec_policy_mgr_t.
 */
struct private_ipsec_policy_mgr_t {

	/**
	 * Public members of ipsec_policy_mgr_t.
	 */
	ipsec_policy_mgr_t public;

	/**
	 * Installed policies (ipsec_policy_entry_t*)
	 */
	linked_list_t *policies;

	/**
	 * Lock to safely access the list of policies
	 */
	rwlock_t *lock;

};

/**
 * Helper struct to store policies in a list sorted by the same pseudo-priority
 * used by the NETLINK kernel interface.
 */
typedef struct {

	/**
	 * Priority used to sort policies
	 */
	u_int32_t priority;

	/**
	 * The policy
	 */
	ipsec_policy_t *policy;

} ipsec_policy_entry_t;

/**
 * Calculate the pseudo-priority to sort policies.  This is the same algorithm
 * used by the NETLINK kernel interface (i.e. high priority -> low value).
 */
static u_int32_t calculate_priority(policy_priority_t policy_priority,
									traffic_selector_t *src,
									traffic_selector_t *dst)
{
	u_int32_t priority = PRIO_BASE;
	u_int16_t port;
	u_int8_t mask, proto;
	host_t *net;

	switch (policy_priority)
	{
		case POLICY_PRIORITY_FALLBACK:
			priority <<= 1;
			/* fall-through */
		case POLICY_PRIORITY_ROUTED:
			priority <<= 1;
			/* fall-through */
		case POLICY_PRIORITY_DEFAULT:
			priority <<= 1;
			/* fall-through */
		case POLICY_PRIORITY_PASS:
			break;
	}
	/* calculate priority based on selector size, small size = high prio */
	src->to_subnet(src, &net, &mask);
	priority -= mask;
	proto = src->get_protocol(src);
	port = net->get_port(net);
	net->destroy(net);

	dst->to_subnet(dst, &net, &mask);
	priority -= mask;
	proto = max(proto, dst->get_protocol(dst));
	port = max(port, net->get_port(net));
	net->destroy(net);

	priority <<= 2; /* make some room for the two flags */
	priority += port ? 0 : 2;
	priority += proto ? 0 : 1;
	return priority;
}

/**
 * Create a policy entry
 */
static ipsec_policy_entry_t *policy_entry_create(ipsec_policy_t *policy)
{
	ipsec_policy_entry_t *this;

	INIT(this,
		.policy = policy,
		.priority = calculate_priority(policy->get_priority(policy),
									   policy->get_source_ts(policy),
									   policy->get_destination_ts(policy)),
	);
	return this;
}

/**
 * Destroy a policy entry
 */
static void policy_entry_destroy(ipsec_policy_entry_t *this)
{
	this->policy->destroy(this->policy);
	free(this);
}

METHOD(ipsec_policy_mgr_t, add_policy, status_t,
	private_ipsec_policy_mgr_t *this, host_t *src, host_t *dst,
	traffic_selector_t *src_ts, traffic_selector_t *dst_ts,
	policy_dir_t direction, policy_type_t type, ipsec_sa_cfg_t *sa, mark_t mark,
	policy_priority_t priority)
{
	enumerator_t *enumerator;
	ipsec_policy_entry_t *entry, *current;
	ipsec_policy_t *policy;

	if (type != POLICY_IPSEC || direction == POLICY_FWD)
	{	/* we ignore these policies as we currently have no use for them */
		return SUCCESS;
	}

	DBG2(DBG_ESP, "adding policy %R === %R %N", src_ts, dst_ts,
		 policy_dir_names, direction);

	policy = ipsec_policy_create(src, dst, src_ts, dst_ts, direction, type, sa,
								 mark, priority);
	entry = policy_entry_create(policy);

	this->lock->write_lock(this->lock);
	enumerator = this->policies->create_enumerator(this->policies);
	while (enumerator->enumerate(enumerator, (void**)&current))
	{
		if (current->priority >= entry->priority)
		{
			break;
		}
	}
	this->policies->insert_before(this->policies, enumerator, entry);
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return SUCCESS;
}

METHOD(ipsec_policy_mgr_t, del_policy, status_t,
	private_ipsec_policy_mgr_t *this, traffic_selector_t *src_ts,
	traffic_selector_t *dst_ts, policy_dir_t direction, u_int32_t reqid,
	mark_t mark, policy_priority_t policy_priority)
{
	enumerator_t *enumerator;
	ipsec_policy_entry_t *current, *found = NULL;
	u_int32_t priority;

	if (direction == POLICY_FWD)
	{	/* we ignore these policies as we currently have no use for them */
		return SUCCESS;
	}
	DBG2(DBG_ESP, "deleting policy %R === %R %N", src_ts, dst_ts,
		 policy_dir_names, direction);

	priority = calculate_priority(policy_priority, src_ts, dst_ts);

	this->lock->write_lock(this->lock);
	enumerator = this->policies->create_enumerator(this->policies);
	while (enumerator->enumerate(enumerator, (void**)&current))
	{
		if (current->priority == priority &&
			current->policy->match(current->policy, src_ts, dst_ts, direction,
								   reqid, mark, policy_priority))
		{
			this->policies->remove_at(this->policies, enumerator);
			found = current;
			break;
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	if (found)
	{
		policy_entry_destroy(found);
		return SUCCESS;
	}
	return FAILED;
}

METHOD(ipsec_policy_mgr_t, flush_policies, status_t,
	private_ipsec_policy_mgr_t *this)
{
	ipsec_policy_entry_t *entry;

	DBG2(DBG_ESP, "flushing policies");

	this->lock->write_lock(this->lock);
	while (this->policies->remove_last(this->policies,
									  (void**)&entry) == SUCCESS)
	{
		policy_entry_destroy(entry);
	}
	this->lock->unlock(this->lock);
	return SUCCESS;
}

METHOD(ipsec_policy_mgr_t, find_by_packet, ipsec_policy_t*,
	private_ipsec_policy_mgr_t *this, ip_packet_t *packet, bool inbound,
	u_int32_t reqid)
{
	enumerator_t *enumerator;
	ipsec_policy_entry_t *current;
	ipsec_policy_t *found = NULL;

	this->lock->read_lock(this->lock);
	enumerator = this->policies->create_enumerator(this->policies);
	while (enumerator->enumerate(enumerator, (void**)&current))
	{
		ipsec_policy_t *policy = current->policy;

		if ((inbound == (policy->get_direction(policy) == POLICY_IN)) &&
			 policy->match_packet(policy, packet))
		{
			if (reqid == 0 || reqid == policy->get_reqid(policy))
			{
				found = policy->get_ref(policy);
				break;
			}
		}
	}
	enumerator->destroy(enumerator);
	this->lock->unlock(this->lock);
	return found;
}

METHOD(ipsec_policy_mgr_t, destroy, void,
	private_ipsec_policy_mgr_t *this)
{
	flush_policies(this);
	this->policies->destroy(this->policies);
	this->lock->destroy(this->lock);
	free(this);
}

/**
 * Described in header.
 */
ipsec_policy_mgr_t *ipsec_policy_mgr_create()
{
	private_ipsec_policy_mgr_t *this;

	INIT(this,
		.public = {
			.add_policy = _add_policy,
			.del_policy = _del_policy,
			.flush_policies = _flush_policies,
			.find_by_packet = _find_by_packet,
			.destroy = _destroy,
		},
		.policies = linked_list_create(),
		.lock = rwlock_create(RWLOCK_TYPE_DEFAULT),
	);

	return &this->public;
}
