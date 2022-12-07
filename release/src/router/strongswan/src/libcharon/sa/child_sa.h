/*
 * Copyright (C) 2006-2019 Tobias Brunner
 * Copyright (C) 2006-2008 Martin Willi
 * Copyright (C) 2006 Daniel Roethlisberger
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

/**
 * @defgroup child_sa child_sa
 * @{ @ingroup sa
 */

#ifndef CHILD_SA_H_
#define CHILD_SA_H_

typedef enum child_sa_state_t child_sa_state_t;
typedef enum child_sa_outbound_state_t child_sa_outbound_state_t;
typedef struct child_sa_t child_sa_t;
typedef struct child_sa_create_t child_sa_create_t;

#include <library.h>
#include <encoding/payloads/proposal_substructure.h>
#include <crypto/proposal/proposal.h>
#include <config/child_cfg.h>

/**
 * States of a CHILD_SA
 */
enum child_sa_state_t {

	/**
	 * Just created, uninstalled CHILD_SA
	 */
	CHILD_CREATED,

	/**
	 * Installed SPD, but no SAD entries
	 */
	CHILD_ROUTED,

	/**
	 * Installing an in-use CHILD_SA
	 */
	CHILD_INSTALLING,

	/**
	 * Installed both SAs of a CHILD_SA
	 */
	CHILD_INSTALLED,

	/**
	 * While updating hosts, in update_hosts()
	 */
	CHILD_UPDATING,

	/**
	 * CHILD_SA which is rekeying
	 */
	CHILD_REKEYING,

	/**
	 * CHILD_SA that was rekeyed, but stays installed
	 */
	CHILD_REKEYED,

	/**
	 * CHILD_SA negotiation failed, but gets retried
	 */
	CHILD_RETRYING,

	/**
	 * CHILD_SA in progress of delete
	 */
	CHILD_DELETING,

	/**
	 * CHILD_SA has been deleted, but not yet destroyed
	 */
	CHILD_DELETED,

	/**
	 * CHILD_SA object gets destroyed
	 */
	CHILD_DESTROYING,
};

/**
 * enum strings for child_sa_state_t.
 */
extern enum_name_t *child_sa_state_names;

/**
 * States of the outbound SA of a CHILD_SA
 */
enum child_sa_outbound_state_t {

	/**
	 * Outbound SA is not installed
	 */
	CHILD_OUTBOUND_NONE = 0,

	/**
	 * Data for the outbound SA has been registered during a rekeying (not set
	 * once the SA and policies are both installed)
	 */
	CHILD_OUTBOUND_REGISTERED = (1<<0),

	/**
	 * The outbound SA has been installed
	 */
	CHILD_OUTBOUND_SA = (1<<1),

	/**
	 * The outbound policies have been installed
	 */
	CHILD_OUTBOUND_POLICIES = (1<<2),

	/**
	 * The outbound SA and policies are both installed
	 */
	CHILD_OUTBOUND_INSTALLED = (CHILD_OUTBOUND_SA|CHILD_OUTBOUND_POLICIES),
};

/**
 * enum strings for child_sa_outbound_state_t.
 */
extern enum_name_t *child_sa_outbound_state_names;

/**
 * Represents an IPsec SAs between two hosts.
 *
 * A child_sa_t contains two SAs. SAs for both
 * directions are managed in one child_sa_t object. Both
 * SAs and the policies have the same reqid.
 *
 * The procedure for child sa setup is as follows:
 * - A gets SPIs for a all protocols in its proposals via child_sa_t.alloc
 * - A send the proposals with the allocated SPIs to B
 * - B selects a suitable proposal
 * - B allocates an SPI for the selected protocol
 * - B calls child_sa_t.install for both, the allocated and received SPI
 * - B sends the proposal with the allocated SPI to A
 * - A calls child_sa_t.install for both, the allocated and received SPI
 *
 * Once SAs are set up, policies can be added using add_policies.
 */
struct child_sa_t {

	/**
	 * Get the name of the config this CHILD_SA uses.
	 *
	 * @return			name
	 */
	char* (*get_name) (child_sa_t *this);

	/**
	 * Get the reqid of the CHILD SA.
	 *
	 * Every CHILD_SA has a reqid. The kernel uses this ID to
	 * identify it.
	 *
	 * @return 			reqid of the CHILD SA
	 */
	uint32_t (*get_reqid)(child_sa_t *this);

	/**
	 * Get the unique numerical identifier for this CHILD_SA.
	 *
	 * While the same reqid might be shared between multiple SAs, the unique_id
	 * is truly unique for all CHILD_SA instances.
	 *
	 * @return			unique CHILD_SA identifier
	 */
	uint32_t (*get_unique_id)(child_sa_t *this);

	/**
	 * Get the config used to set up this child sa.
	 *
	 * @return			child_cfg
	 */
	child_cfg_t* (*get_config) (child_sa_t *this);

	/**
	 * Get the state of the CHILD_SA.
	 *
	 * @return 			CHILD_SA state
	 */
	child_sa_state_t (*get_state)(child_sa_t *this);

	/**
	 * Get the state of the outbound SA.
	 *
	 * @return 			outbound SA state
	 */
	child_sa_outbound_state_t (*get_outbound_state)(child_sa_t *this);

	/**
	 * Set the state of the CHILD_SA.
	 *
	 * @param state		state to set on CHILD_SA
	 */
	void (*set_state) (child_sa_t *this, child_sa_state_t state);

	/**
	 * Get the SPI of this CHILD_SA.
	 *
	 * Set the boolean parameter inbound to TRUE to
	 * get the SPI for which we receive packets, use
	 * FALSE to get those we use for sending packets.
	 *
	 * @param inbound	TRUE to get inbound SPI, FALSE for outbound.
	 * @return 			SPI of the CHILD SA
	 */
	uint32_t (*get_spi) (child_sa_t *this, bool inbound);

	/**
	 * Get the CPI of this CHILD_SA.
	 *
	 * Set the boolean parameter inbound to TRUE to
	 * get the CPI for which we receive packets, use
	 * FALSE to get those we use for sending packets.
	 *
	 * @param inbound	TRUE to get inbound CPI, FALSE for outbound.
	 * @return 			CPI of the CHILD SA
	 */
	uint16_t (*get_cpi) (child_sa_t *this, bool inbound);

	/**
	 * Get the protocol which this CHILD_SA uses to protect traffic.
	 *
	 * @return 			AH | ESP
	 */
	protocol_id_t (*get_protocol) (child_sa_t *this);

	/**
	 * Set the negotiated protocol to use for this CHILD_SA.
	 *
	 * @param protocol	AH | ESP
	 */
	void (*set_protocol)(child_sa_t *this, protocol_id_t protocol);

	/**
	 * Get the IPsec mode of this CHILD_SA.
	 *
	 * @return			TUNNEL | TRANSPORT | BEET
	 */
	ipsec_mode_t (*get_mode)(child_sa_t *this);

	/**
	 * Set the negotiated IPsec mode to use.
	 *
	 * @param mode		TUNNEL | TRANSPORT | BEET
	 */
	void (*set_mode)(child_sa_t *this, ipsec_mode_t mode);

	/**
	 * Get the used IPComp algorithm.
	 *
	 * @return			IPComp compression algorithm.
	 */
	ipcomp_transform_t (*get_ipcomp)(child_sa_t *this);

	/**
	 * Set the IPComp algorithm to use.
	 *
	 * @param ipcomp	the IPComp transform to use
	 */
	void (*set_ipcomp)(child_sa_t *this, ipcomp_transform_t ipcomp);

	/**
	 * Get the action to enforce if the remote peer closes the CHILD_SA.
	 *
	 * @return			close action
	 */
	action_t (*get_close_action)(child_sa_t *this);

	/**
	 * Override the close action specified by the CHILD_SA config.
	 *
	 * @param			close action to enforce
	 */
	void (*set_close_action)(child_sa_t *this, action_t action);

	/**
	 * Get the action to enforce if the peer is considered dead.
	 *
	 * @return			dpd action
	 */
	action_t (*get_dpd_action)(child_sa_t *this);

	/**
	 * Override the DPD action specified by the CHILD_SA config.
	 *
	 * @param			dpd action to enforce
	 */
	void (*set_dpd_action)(child_sa_t *this, action_t action);

	/**
	 * Get the selected proposal.
	 *
	 * @return			selected proposal
	 */
	proposal_t* (*get_proposal)(child_sa_t *this);

	/**
	 * Set the negotiated proposal.
	 *
	 * @param proposal	selected proposal
	 */
	void (*set_proposal)(child_sa_t *this, proposal_t *proposal);

	/**
	 * Check if this CHILD_SA uses UDP encapsulation.
	 *
	 * @return			TRUE if SA encapsulates ESP packets
	 */
	bool (*has_encap)(child_sa_t *this);

	/**
	 * Get the absolute time when the CHILD_SA expires or gets rekeyed.
	 *
	 * @param hard		TRUE for hard lifetime, FALSE for soft (rekey) lifetime
	 * @return			absolute time
	 */
	time_t (*get_lifetime)(child_sa_t *this, bool hard);

	/**
	 * Get the absolute time when this SA has been installed.
	 *
	 * @return			monotonic absolute install time
	 */
	time_t (*get_installtime)(child_sa_t *this);

	/**
	 * Get last use time and the number of bytes processed.
	 *
	 * @param inbound		TRUE for inbound traffic, FALSE for outbound
	 * @param[out] time		time of last use in seconds (NULL to ignore)
	 * @param[out] bytes	number of processed bytes (NULL to ignore)
	 * @param[out] packets	number of processed packets (NULL to ignore)
	 */
	void (*get_usestats)(child_sa_t *this, bool inbound, time_t *time,
						 uint64_t *bytes, uint64_t *packets);

	/**
	 * Get the mark used with this CHILD_SA.
	 *
	 * @param inbound		TRUE to get inbound mark, FALSE for outbound
	 * @return				mark used with this CHILD_SA
	 */
	mark_t (*get_mark)(child_sa_t *this, bool inbound);

	/**
	 * Get the interface ID used with this CHILD_SA.
	 *
	 * @param inbound		TRUE to get inbound ID, FALSE for outbound
	 * @return				interface ID used with this CHILD_SA
	 */
	uint32_t (*get_if_id)(child_sa_t *this, bool inbound);

	/**
	 * Get the security label used with this CHILD_SA.
	 *
	 * This might be different than the configured label.
	 *
	 * @return				security label used with this CHILD_SA
	 */
	sec_label_t *(*get_label)(child_sa_t *this);

	/**
	 * Create an enumerator over traffic selectors of one side.
	 *
	 * @param local		TRUE for own traffic selectors, FALSE for remote.
	 * @return			enumerator over traffic_selector_t*
	 */
	enumerator_t* (*create_ts_enumerator)(child_sa_t *this, bool local);

	/**
	 * Create an enumerator over installed policies.
	 *
	 * The enumerated traffic selectors is a full mesh of compatible local
	 * and remote traffic selectors.
	 *
	 * @return			enumerator over a pair of traffic_selector_t*
	 */
	enumerator_t* (*create_policy_enumerator)(child_sa_t *this);

	/**
	 * Allocate an SPI to include in a proposal.
	 *
	 * @param protocol	protocol to allocate SPI for (ESP|AH)
	 * @param spi		SPI output pointer
	 * @return			SPI, 0 on failure
	 */
	uint32_t (*alloc_spi)(child_sa_t *this, protocol_id_t protocol);

	/**
	 * Allocate a CPI to use for IPComp.
	 *
	 * @return			CPI, 0 on failure
	 */
	uint16_t (*alloc_cpi)(child_sa_t *this);

	/**
	 * Install an IPsec SA for one direction.
	 *
	 * set_policies() should be called before calling this.
	 *
	 * @param encr		encryption key, if any
	 * @param integ		integrity key
	 * @param spi		SPI to use, allocated for inbound
	 * @param cpi		CPI to use, allocated for outbound
	 * @param initiator	TRUE if initiator of exchange resulting in this SA
	 * @param inbound	TRUE to install an inbound SA, FALSE for outbound
	 * @param tfcv3		TRUE if peer supports ESPv3 TFC
	 * @return			SUCCESS or FAILED
	 */
	status_t (*install)(child_sa_t *this, chunk_t encr, chunk_t integ,
						uint32_t spi, uint16_t cpi,
						bool initiator, bool inbound, bool tfcv3);

	/**
	 * Register data for the installation of an outbound SA as responder during
	 * a rekeying.
	 *
	 * If the kernel is able to handle SPIs on policies the SA is installed
	 * immediately, if not it won't be installed until install_outbound() is
	 * called.
	 *
	 * @param encr		encryption key, if any (cloned)
	 * @param integ		integrity key (cloned)
	 * @param spi		SPI to use, allocated for inbound
	 * @param cpi		CPI to use, allocated for outbound
	 * @param initiator	TRUE if initiator of exchange resulting in this SA
	 * @param tfcv3		TRUE if peer supports ESPv3 TFC
	 * @return			SUCCESS or FAILED
	 */
	status_t (*register_outbound)(child_sa_t *this, chunk_t encr, chunk_t integ,
								  uint32_t spi, uint16_t cpi, bool initiator,
								  bool tfcv3);

	/**
	 * Install the outbound policies and, if not already done, the outbound SA
	 * as responder during a rekeying.
	 *
	 * @return			SUCCESS or FAILED
	 */
	status_t (*install_outbound)(child_sa_t *this);

	/**
	 * Remove the outbound SA and the outbound policies after a rekeying.
	 */
	void (*remove_outbound)(child_sa_t *this);

	/**
	 * Configure the policies using some traffic selectors.
	 *
	 * Supplied lists of traffic_selector_t's specify the policies
	 * to use for this child sa.
	 *
	 * Install the policies by calling install_policies().
	 *
	 * This should be called before calling install() so the traffic selectors
	 * may be passed to the kernel interface when installing the SAs.
	 *
	 * @param my_ts		traffic selectors for local site (cloned)
	 * @param other_ts	traffic selectors for remote site (cloned)
	 */
	void (*set_policies)(child_sa_t *this, linked_list_t *my_ts_list,
						 linked_list_t *other_ts_list);

	/**
	 * Install the configured policies.
	 *
	 * If register_outbound() was called previously this only installs the
	 * inbound and forward policies, the outbound policies are installed when
	 * install_outbound() is called.
	 *
	 * @return			SUCCESS or FAILED
	 */
	status_t (*install_policies)(child_sa_t *this);

	/**
	 * Set the outbound SPI of the CHILD_SA that replaced this CHILD_SA during
	 * a rekeying.
	 *
	 * @param spi		outbound SPI of the CHILD_SA that replaced this CHILD_SA
	 */
	void (*set_rekey_spi)(child_sa_t *this, uint32_t spi);

	/**
	 * Get the outbound SPI of the CHILD_SA that replaced this CHILD_SA during
	 * a rekeying.
	 *
	 * @return			outbound SPI of the CHILD_SA that replaced this CHILD_SA
	 */
	uint32_t (*get_rekey_spi)(child_sa_t *this);

	/**
	 * Update hosts and ecapsulation mode in the kernel SAs and policies.
	 *
	 * @param me		the new local host
	 * @param other		the new remote host
	 * @param vips		list of local virtual IPs
	 * @param encap		TRUE to use UDP encapsulation for NAT traversal
	 * @return			SUCCESS or FAILED
	 */
	status_t (*update)(child_sa_t *this, host_t *me, host_t *other,
					   linked_list_t *vips, bool encap);
	/**
	 * Destroys a child_sa.
	 */
	void (*destroy) (child_sa_t *this);
};

/**
 * Data passed to the constructor of a child_sa_t object.
 */
struct child_sa_create_t {
	/** Optional reqid of old CHILD_SA when rekeying */
	uint32_t reqid;
	/** Optional inbound mark when rekeying */
	uint32_t mark_in;
	/** Optional outbound mark when rekeying */
	uint32_t mark_out;
	/** Optional inbound interface ID when rekeying */
	uint32_t if_id_in;
	/** Optional outbound interface ID when rekeying */
	uint32_t if_id_out;
	/** Optional default inbound interface ID, if neither if_id_in, nor config
	 * sets one */
	uint32_t if_id_in_def;
	/** Optional default outbound interface ID, if neither if_id_out, nor config
	 * sets one */
	uint32_t if_id_out_def;
	/** Optional security label to apply on SAs (cloned) */
	sec_label_t *label;
	/** TRUE to enable UDP encapsulation (NAT traversal) */
	bool encap;
};

/**
 * Constructor to create a child SA negotiated with IKE.
 *
 * @param me				own address
 * @param other				remote address
 * @param config			config to use for this CHILD_SA
 * @param data				data for this CHILD_SA
 * @return					child_sa_t object
 */
child_sa_t *child_sa_create(host_t *me, host_t *other, child_cfg_t *config,
							child_sa_create_t *data);

#endif /** CHILD_SA_H_ @}*/
