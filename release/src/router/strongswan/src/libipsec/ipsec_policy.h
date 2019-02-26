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

/**
 * @defgroup ipsec_policy ipsec_policy
 * @{ @ingroup libipsec
 */

#ifndef IPSEC_POLICY_H
#define IPSEC_POLICY_H

#include "ip_packet.h"

#include <library.h>
#include <networking/host.h>
#include <ipsec/ipsec_types.h>
#include <selectors/traffic_selector.h>

typedef struct ipsec_policy_t ipsec_policy_t;

/**
 * IPsec Policy
 */
struct ipsec_policy_t {

	/**
	 * Get the source traffic selector of this policy
	 *
	 * @return			the source traffic selector
	 */
	traffic_selector_t *(*get_source_ts)(ipsec_policy_t *this);

	/**
	 * Get the destination traffic selector of this policy
	 *
	 * @return			the destination traffic selector
	 */
	traffic_selector_t *(*get_destination_ts)(ipsec_policy_t *this);

	/**
	 * Get the direction of this policy
	 *
	 * @return			direction
	 */
	policy_dir_t (*get_direction)(ipsec_policy_t *this);

	/**
	 * Get the priority of this policy
	 *
	 * @return			priority
	 */
	policy_priority_t (*get_priority)(ipsec_policy_t *this);

	/**
	 * Get the type of this policy (e.g. IPsec)
	 *
	 * @return			the policy type
	 */
	policy_type_t (*get_type)(ipsec_policy_t *this);

	/**
	 * Get the reqid associated to this policy
	 *
	 * @return			the reqid
	 */
	uint32_t (*get_reqid)(ipsec_policy_t *this);

	/**
	 * Get another reference to this policy
	 *
	 * @return			additional reference to the policy
	 */
	ipsec_policy_t *(*get_ref)(ipsec_policy_t *this);

	/**
	 * Check if this policy matches all given parameters
	 *
	 * @param src_ts		source traffic selector
	 * @param dst_ts		destination traffic selector
	 * @param direction		traffic direction
	 * @param reqid			reqid of the policy
	 * @param mark			mark for this policy
	 * @param prioirty		policy priority
	 * @return				TRUE if policy matches all parameters
	 */
	bool (*match)(ipsec_policy_t *this, traffic_selector_t *src_ts,
				  traffic_selector_t *dst_ts, policy_dir_t direction,
				  uint32_t reqid, mark_t mark, policy_priority_t priority);

	/**
	 * Check if this policy matches the given IP packet
	 *
	 * @param packet		IP packet
	 * @return				TRUE if policy matches the packet
	 */
	bool (*match_packet)(ipsec_policy_t *this, ip_packet_t *packet);

	/**
	 * Destroy an ipsec_policy_t
	 */
	void (*destroy)(ipsec_policy_t *this);

};

/**
 * Create an ipsec_policy_t instance
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
 * @return				ipsec policy instance
 */
ipsec_policy_t *ipsec_policy_create(host_t *src, host_t *dst,
									traffic_selector_t *src_ts,
									traffic_selector_t *dst_ts,
									policy_dir_t direction,	policy_type_t type,
									ipsec_sa_cfg_t *sa, mark_t mark,
									policy_priority_t priority);

#endif /** IPSEC_POLICY_H @}*/
