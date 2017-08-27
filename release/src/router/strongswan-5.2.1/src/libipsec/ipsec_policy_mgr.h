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

/**
 * @defgroup ipsec_policy_mgr ipsec_policy_mgr
 * @{ @ingroup libipsec
 */

#ifndef IPSEC_POLICY_MGR_H_
#define IPSEC_POLICY_MGR_H_

#include "ipsec_policy.h"
#include "ip_packet.h"

#include <library.h>
#include <networking/host.h>
#include <collections/linked_list.h>
#include <ipsec/ipsec_types.h>
#include <selectors/traffic_selector.h>

typedef struct ipsec_policy_mgr_t ipsec_policy_mgr_t;

/**
 * IPsec policy manager
 *
 * The first methods are modeled after those in kernel_ipsec_t.
 *
 * @note Only policies of type POLICY_IPSEC are currently used, also policies
 * with direction POLICY_FWD are ignored.  Any packets that do not match an
 * installed policy will be dropped.
 */
struct ipsec_policy_mgr_t {

	/**
	 * Add a policy
	 *
	 * A policy is always associated to an SA. Traffic which matches a
	 * policy is handled by the SA with the same reqid.
	 *
	 * @param src			source address of SA
	 * @param dst			dest address of SA
	 * @param src_ts		traffic selector to match traffic source
	 * @param dst_ts		traffic selector to match traffic dest
	 * @param direction		direction of traffic, POLICY_(IN|OUT|FWD)
	 * @param type			type of policy, POLICY_(IPSEC|PASS|DROP)
	 * @param sa			details about the SA(s) tied to this policy
	 * @param mark			mark for this policy
	 * @param priority		priority of this policy
	 * @return				SUCCESS if operation completed
	 */
	status_t (*add_policy)(ipsec_policy_mgr_t *this,
						   host_t *src, host_t *dst, traffic_selector_t *src_ts,
						   traffic_selector_t *dst_ts, policy_dir_t direction,
						   policy_type_t type, ipsec_sa_cfg_t *sa, mark_t mark,
						   policy_priority_t priority);

	/**
	 * Remove a policy
	 *
	 * @param src_ts		traffic selector to match traffic source
	 * @param dst_ts		traffic selector to match traffic dest
	 * @param direction		direction of traffic, POLICY_(IN|OUT|FWD)
	 * @param reqid			unique ID of the associated SA
	 * @param mark			optional mark
	 * @param priority		priority of the policy
	 * @return				SUCCESS if operation completed
	 */
	status_t (*del_policy)(ipsec_policy_mgr_t *this,
						   traffic_selector_t *src_ts,
						   traffic_selector_t *dst_ts,
						   policy_dir_t direction, u_int32_t reqid, mark_t mark,
						   policy_priority_t priority);

	/**
	 * Flush all policies
	 *
	 * @return				SUCCESS if operation completed
	 */
	status_t (*flush_policies)(ipsec_policy_mgr_t *this);

	/**
	 * Find the policy that matches the given IP packet best
	 *
	 * @param packet		IP packet to match
	 * @param inbound		TRUE for an inbound packet
	 * @param reqid			require a policy with a specific reqid, 0 for any
	 * @return				reference to the policy, or NULL if none found
	 */
	ipsec_policy_t *(*find_by_packet)(ipsec_policy_mgr_t *this,
									  ip_packet_t *packet, bool inbound,
									  u_int32_t reqid);

	/**
	 * Destroy an ipsec_policy_mgr_t
	 */
	void (*destroy)(ipsec_policy_mgr_t *this);

};

/**
 * Create an ipsec_policy_mgr instance
 *
 * @return			ipsec_policy_mgr
 */
ipsec_policy_mgr_t *ipsec_policy_mgr_create();

#endif /** IPSEC_POLICY_MGR_H_ @}*/
