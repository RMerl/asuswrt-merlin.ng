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

/**
 * @defgroup child_cfg child_cfg
 * @{ @ingroup config
 */

#ifndef CHILD_CFG_H_
#define CHILD_CFG_H_

typedef enum action_t action_t;
typedef enum child_cfg_option_t child_cfg_option_t;
typedef struct child_cfg_t child_cfg_t;
typedef struct child_cfg_create_t child_cfg_create_t;

#include <library.h>
#include <selectors/traffic_selector.h>
#include <crypto/proposal/proposal.h>
#include <kernel/kernel_ipsec.h>

/**
 * Action to take when connection is loaded, DPD is detected or
 * connection gets closed by peer.
 */
enum action_t {
	/** No action */
	ACTION_NONE,
	/** Route config to establish or reestablish on demand */
	ACTION_ROUTE,
	/** Start or restart config immediately */
	ACTION_RESTART,
};

/**
 * enum names for action_t.
 */
extern enum_name_t *action_names;

/**
 * A child_cfg_t defines the config template for a CHILD_SA.
 *
 * After creation, proposals and traffic selectors may be added to the config.
 * A child_cfg object is referenced multiple times, and is not thread save.
 * Reading from the object is save, adding things is not allowed while other
 * threads may access the object.
 * A reference counter handles the number of references hold to this config.
 *
 * @see peer_cfg_t to get an overview over the configurations.
 */
struct child_cfg_t {

	/**
	 * Get the name of the child_cfg.
	 *
	 * @return				child_cfg's name
	 */
	char *(*get_name) (child_cfg_t *this);

	/**
	 * Add a proposal to the list.
	 *
	 * The proposals are stored by priority, first added
	 * is the most preferred. It is safe to add NULL as proposal, which has no
	 * effect. After add, proposal is owned by child_cfg.
	 *
	 * @param proposal		proposal to add, or NULL
	 */
	void (*add_proposal) (child_cfg_t *this, proposal_t *proposal);

	/**
	 * Get the list of proposals for the CHILD_SA.
	 *
	 * Resulting list and all of its proposals must be freed after use.
	 *
	 * @param strip_dh		TRUE strip out diffie hellman groups
	 * @return				list of proposals
	 */
	linked_list_t* (*get_proposals)(child_cfg_t *this, bool strip_dh);

	/**
	 * Select a proposal from a supplied list.
	 *
	 * Returned propsal is newly created and must be destroyed after usage.
	 *
	 * @param proposals		list from which proposals are selected
	 * @param strip_dh		TRUE strip out diffie hellman groups
	 * @param private		accept algorithms from a private range
	 * @param prefer_self	whether to prefer configured or supplied proposals
	 * @return				selected proposal, or NULL if nothing matches
	 */
	proposal_t* (*select_proposal)(child_cfg_t*this, linked_list_t *proposals,
								   bool strip_dh, bool private,
								   bool prefer_self);

	/**
	 * Add a traffic selector to the config.
	 *
	 * Use the "local" parameter to add it for the local or the remote side.
	 * After add, traffic selector is owned by child_cfg.
	 *
	 * @param local			TRUE for local side, FALSE for remote
	 * @param ts			traffic_selector to add
	 */
	void (*add_traffic_selector)(child_cfg_t *this, bool local,
								 traffic_selector_t *ts);

	/**
	 * Get a list of traffic selectors to use for the CHILD_SA.
	 *
	 * The config contains two set of traffic selectors, one for the local
	 * side, one for the remote side.
	 * If a list with traffic selectors is supplied, these are used to narrow
	 * down the traffic selector list to the greatest common divisor.
	 * Some traffic selector may be "dymamic", meaning they are narrowed down
	 * to a specific address (host-to-host or virtual-IP setups). Use
	 * the "host" parameter to narrow such traffic selectors to that address.
	 * Resulted list and its traffic selectors must be destroyed after use.
	 *
	 * @param local			TRUE for TS on local side, FALSE for remote
	 * @param supplied		list with TS to select from, or NULL
	 * @param hosts			addresses to use for narrowing "dynamic" TS', host_t
	 * @param log			FALSE to avoid logging details about the selection
	 * @return				list containing the traffic selectors
	 */
	linked_list_t *(*get_traffic_selectors)(child_cfg_t *this, bool local,
											linked_list_t *supplied,
											linked_list_t *hosts, bool log);

	/**
	 * Get the updown script to run for the CHILD_SA.
	 *
	 * @return				path to updown script
	 */
	char* (*get_updown)(child_cfg_t *this);

	/**
	 * Get the lifetime configuration of a CHILD_SA.
	 *
	 * The rekey limits automatically contain a jitter to avoid simultaneous
	 * rekeying. These values will change with each call to this function.
	 *
	 * @param jitter		subtract jitter value to randomize lifetimes
	 * @return				lifetime_cfg_t (has to be freed)
	 */
	lifetime_cfg_t* (*get_lifetime) (child_cfg_t *this, bool jitter);

	/**
	 * Get the mode to use for the CHILD_SA.
	 *
	 * The mode is either tunnel, transport or BEET. The peer must agree
	 * on the method, fallback is tunnel mode.
	 *
	 * @return				ipsec mode
	 */
	ipsec_mode_t (*get_mode) (child_cfg_t *this);

	/**
	 * Action to take to start CHILD_SA.
	 *
	 * @return				start action
	 */
	action_t (*get_start_action) (child_cfg_t *this);

	/**
	 * Action to take on DPD.
	 *
	 * @return				DPD action
	 */
	action_t (*get_dpd_action) (child_cfg_t *this);

	/**
	 * Get the HW offload mode to use for the CHILD_SA.
	 *
	 * @return				hw offload mode
	 */
	hw_offload_t (*get_hw_offload) (child_cfg_t *this);

	/**
	 * Get the copy mode for the DS header field to use for the CHILD_SA.
	 *
	 * @return				IP header copy mode
	 */
	dscp_copy_t (*get_copy_dscp) (child_cfg_t *this);

	/**
	 * Action to take if CHILD_SA gets closed.
	 *
	 * @return				close action
	 */
	action_t (*get_close_action) (child_cfg_t *this);

	/**
	 * Get the DH group to use for CHILD_SA setup.
	 *
	 * @return				dh group to use
	 */
	diffie_hellman_group_t (*get_dh_group)(child_cfg_t *this);

	/**
	 * Get the inactivity timeout value.
	 *
	 * @return				inactivity timeout in s
	 */
	uint32_t (*get_inactivity)(child_cfg_t *this);

	/**
	 * Specific reqid to use for CHILD_SA.
	 *
	 * @return				reqid
	 */
	uint32_t (*get_reqid)(child_cfg_t *this);

	/**
	 * Optional mark to set on policies/SAs.
	 *
	 * @param inbound		TRUE for inbound, FALSE for outbound
	 * @return				mark
	 */
	mark_t (*get_mark)(child_cfg_t *this, bool inbound);

	/**
	 * Optional mark the SAs should apply after processing packets.
	 *
	 * @param inbound		TRUE for inbound, FALSE for outbound
	 * @return				mark
	 */
	mark_t (*get_set_mark)(child_cfg_t *this, bool inbound);

	/**
	 * Get the TFC padding value to use for CHILD_SA.
	 *
	 * @return				TFC padding, 0 to disable, -1 for MTU
	 */
	uint32_t (*get_tfc)(child_cfg_t *this);

	/**
	 * Get optional manually-set IPsec policy priority
	 *
	 * @return				manually-set IPsec policy priority (automatic if 0)
	 */
	uint32_t (*get_manual_prio)(child_cfg_t *this);

	/**
	 * Get optional network interface restricting IPsec policy
	 *
	 * @return				network interface)
	 */
	char* (*get_interface)(child_cfg_t *this);

	/**
	 * Get anti-replay window size
	 *
	 * @return				anti-replay window size
	 */
	uint32_t (*get_replay_window)(child_cfg_t *this);

	/**
	 * Set anti-replay window size
	 *
	 * @param window        anti-replay window size
	 */
	void (*set_replay_window)(child_cfg_t *this, uint32_t window);

	/**
	 * Check if an option flag is set.
	 *
	 * @param option		option flag to check
	 * @return				TRUE if option flag set, FALSE otherwise
	 */
	bool (*has_option)(child_cfg_t *this, child_cfg_option_t option);

	/**
	 * Check if two child_cfg objects are equal.
	 *
	 * @param other			candidate to check for equality against this
	 * @return				TRUE if equal
	 */
	bool (*equals)(child_cfg_t *this, child_cfg_t *other);

	/**
	 * Increase the reference count.
	 *
	 * @return				reference to this
	 */
	child_cfg_t* (*get_ref) (child_cfg_t *this);

	/**
	 * Destroys the child_cfg object.
	 *
	 * Decrements the internal reference counter and
	 * destroys the child_cfg when it reaches zero.
	 */
	void (*destroy) (child_cfg_t *this);
};

/**
 * Option flags that may be set on a child_cfg_t object
 */
enum child_cfg_option_t {

	/** Use IPsec transport proxy mode */
	OPT_PROXY_MODE = (1<<0),

	/** Use IPComp, if peer supports it */
	OPT_IPCOMP = (1<<1),

	/** Allow access to the local host */
	OPT_HOSTACCESS = (1<<2),

	/** Don't install any IPsec policies */
	OPT_NO_POLICIES = (1<<3),

	/** Install outbound FWD IPsec policies to bypass drop policies */
	OPT_FWD_OUT_POLICIES = (1<<4),

	/** Force 96-bit truncation for SHA-256 */
	OPT_SHA256_96 = (1<<5),

	/** Set mark on inbound SAs */
	OPT_MARK_IN_SA = (1<<6),

	/** Disable copying the DF bit to the outer IPv4 header in tunnel mode */
	OPT_NO_COPY_DF = (1<<7),

	/** Disable copying the ECN header field in tunnel mode */
	OPT_NO_COPY_ECN = (1<<8),
};

/**
 * Data passed to the constructor of a child_cfg_t object.
 */
struct child_cfg_create_t {
	/** Options set for CHILD_SA */
	child_cfg_option_t options;
	/** Specific reqid to use for CHILD_SA, 0 for auto assignment */
	uint32_t reqid;
	/** Optional inbound mark */
	mark_t mark_in;
	/** Optional outbound mark */
	mark_t mark_out;
	/** Optional inbound mark the SA should apply to traffic */
	mark_t set_mark_in;
	/** Optional outbound mark the SA should apply to traffic */
	mark_t set_mark_out;
	/** Mode to propose for CHILD_SA */
	ipsec_mode_t mode;
	/** TFC padding size, 0 to disable, -1 to pad to PMTU */
	uint32_t tfc;
	/** Optional manually-set IPsec policy priority */
	uint32_t priority;
	/** Optional network interface restricting IPsec policy (cloned) */
	char *interface;
	/** lifetime_cfg_t for this child_cfg */
	lifetime_cfg_t lifetime;
	/** Inactivity timeout in s before closing a CHILD_SA */
	uint32_t inactivity;
	/** Start action */
	action_t start_action;
	/** DPD action */
	action_t dpd_action;
	/** Close action */
	action_t close_action;
	/** updown script to execute on up/down event (cloned) */
	char *updown;
	/** HW offload mode */
	hw_offload_t hw_offload;
	/** How to handle the DS header field in tunnel mode */
	dscp_copy_t copy_dscp;
};

/**
 * Create a configuration template for CHILD_SA setup.
 *
 * After a call to create, a reference is obtained (refcount = 1).
 *
 * @param name				name of the child_cfg (cloned)
 * @param data				data for this child_cfg
 * @return					child_cfg_t object
 */
child_cfg_t *child_cfg_create(char *name, child_cfg_create_t *data);

#endif /** CHILD_CFG_H_ @}*/
