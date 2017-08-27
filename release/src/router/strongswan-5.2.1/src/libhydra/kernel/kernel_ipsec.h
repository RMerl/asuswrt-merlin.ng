/*
 * Copyright (C) 2006-2012 Tobias Brunner
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005-2006 Martin Willi
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

/**
 * @defgroup kernel_ipsec kernel_ipsec
 * @{ @ingroup hkernel
 */

#ifndef KERNEL_IPSEC_H_
#define KERNEL_IPSEC_H_

typedef struct kernel_ipsec_t kernel_ipsec_t;

#include <networking/host.h>
#include <ipsec/ipsec_types.h>
#include <selectors/traffic_selector.h>
#include <plugins/plugin.h>
#include <kernel/kernel_interface.h>

/**
 * Interface to the ipsec subsystem of the kernel.
 *
 * The kernel ipsec interface handles the communication with the kernel
 * for SA and policy management. It allows setup of these, and provides
 * further the handling of kernel events.
 * Policy information are cached in the interface. This is necessary to do
 * reference counting. The Linux kernel does not allow the same policy
 * installed twice, but we need this as CHILD_SA exist multiple times
 * when rekeying. Thats why we do reference counting of policies.
 */
struct kernel_ipsec_t {

	/**
	 * Get the feature set supported by this kernel backend.
	 *
	 * @return				ORed feature-set of backend
	 */
	kernel_feature_t (*get_features)(kernel_ipsec_t *this);

	/**
	 * Get a SPI from the kernel.
	 *
	 * @param src		source address of SA
	 * @param dst		destination address of SA
	 * @param protocol	protocol for SA (ESP/AH)
	 * @param reqid		unique ID for this SA
	 * @param spi		allocated spi
	 * @return				SUCCESS if operation completed
	 */
	status_t (*get_spi)(kernel_ipsec_t *this, host_t *src, host_t *dst,
						u_int8_t protocol, u_int32_t reqid, u_int32_t *spi);

	/**
	 * Get a Compression Parameter Index (CPI) from the kernel.
	 *
	 * @param src		source address of SA
	 * @param dst		destination address of SA
	 * @param reqid		unique ID for the corresponding SA
	 * @param cpi		allocated cpi
	 * @return				SUCCESS if operation completed
	 */
	status_t (*get_cpi)(kernel_ipsec_t *this, host_t *src, host_t *dst,
						u_int32_t reqid, u_int16_t *cpi);

	/**
	 * Add an SA to the SAD.
	 *
	 * add_sa() may update an already allocated
	 * SPI (via get_spi). In this case, the replace
	 * flag must be set.
	 * This function does install a single SA for a
	 * single protocol in one direction.
	 *
	 * @param src			source address for this SA
	 * @param dst			destination address for this SA
	 * @param spi			SPI allocated by us or remote peer
	 * @param protocol		protocol for this SA (ESP/AH)
	 * @param reqid			unique ID for this SA
	 * @param mark			mark for this SA
	 * @param tfc			Traffic Flow Confidentiality padding for this SA
	 * @param lifetime		lifetime_cfg_t for this SA
	 * @param enc_alg		Algorithm to use for encryption (ESP only)
	 * @param enc_key		key to use for encryption
	 * @param int_alg		Algorithm to use for integrity protection
	 * @param int_key		key to use for integrity protection
	 * @param mode			mode of the SA (tunnel, transport)
	 * @param ipcomp		IPComp transform to use
	 * @param cpi			CPI for IPComp
	 * @param replay_window	anti-replay window size
	 * @param initiator		TRUE if initiator of the exchange creating this SA
	 * @param encap			enable UDP encapsulation for NAT traversal
	 * @param esn			TRUE to use Extended Sequence Numbers
	 * @param inbound		TRUE if this is an inbound SA
	 * @param src_ts		traffic selector with BEET source address
	 * @param dst_ts		traffic selector with BEET destination address
	 * @return				SUCCESS if operation completed
	 */
	status_t (*add_sa) (kernel_ipsec_t *this,
						host_t *src, host_t *dst, u_int32_t spi,
						u_int8_t protocol, u_int32_t reqid,
						mark_t mark, u_int32_t tfc, lifetime_cfg_t *lifetime,
						u_int16_t enc_alg, chunk_t enc_key,
						u_int16_t int_alg, chunk_t int_key,
						ipsec_mode_t mode, u_int16_t ipcomp, u_int16_t cpi,
						u_int32_t replay_window,
						bool initiator, bool encap, bool esn, bool inbound,
						traffic_selector_t *src_ts, traffic_selector_t *dst_ts);

	/**
	 * Update the hosts on an installed SA.
	 *
	 * We cannot directly update the destination address as the kernel
	 * requires the spi, the protocol AND the destination address (and family)
	 * to identify SAs. Therefore if the destination address changed we
	 * create a new SA and delete the old one.
	 *
	 * @param spi			SPI of the SA
	 * @param protocol		protocol for this SA (ESP/AH)
	 * @param cpi			CPI for IPComp, 0 if no IPComp is used
	 * @param src			current source address
	 * @param dst			current destination address
	 * @param new_src		new source address
	 * @param new_dst		new destination address
	 * @param encap			current use of UDP encapsulation
	 * @param new_encap		new use of UDP encapsulation
	 * @param mark			optional mark for this SA
	 * @return				SUCCESS if operation completed, NOT_SUPPORTED if
	 *					  the kernel interface can't update the SA
	 */
	status_t (*update_sa)(kernel_ipsec_t *this,
						  u_int32_t spi, u_int8_t protocol, u_int16_t cpi,
						  host_t *src, host_t *dst,
						  host_t *new_src, host_t *new_dst,
						  bool encap, bool new_encap, mark_t mark);

	/**
	 * Query the number of bytes processed by an SA from the SAD.
	 *
	 * @param src			source address for this SA
	 * @param dst			destination address for this SA
	 * @param spi			SPI allocated by us or remote peer
	 * @param protocol		protocol for this SA (ESP/AH)
	 * @param mark			optional mark for this SA
	 * @param[out] bytes	the number of bytes processed by SA
	 * @param[out] packets	number of packets processed by SA
	 * @param[out] time		last (monotonic) time of SA use
	 * @return				SUCCESS if operation completed
	 */
	status_t (*query_sa) (kernel_ipsec_t *this, host_t *src, host_t *dst,
						  u_int32_t spi, u_int8_t protocol, mark_t mark,
						  u_int64_t *bytes, u_int64_t *packets, time_t *time);

	/**
	 * Delete a previusly installed SA from the SAD.
	 *
	 * @param src			source address for this SA
	 * @param dst			destination address for this SA
	 * @param spi			SPI allocated by us or remote peer
	 * @param protocol		protocol for this SA (ESP/AH)
	 * @param cpi			CPI for IPComp or 0
	 * @param mark			optional mark for this SA
	 * @return				SUCCESS if operation completed
	 */
	status_t (*del_sa) (kernel_ipsec_t *this, host_t *src, host_t *dst,
						u_int32_t spi, u_int8_t protocol, u_int16_t cpi,
						mark_t mark);

	/**
	 * Flush all SAs from the SAD.
	 *
	 * @return				SUCCESS if operation completed
	 */
	status_t (*flush_sas) (kernel_ipsec_t *this);

	/**
	 * Add a policy to the SPD.
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
	status_t (*add_policy) (kernel_ipsec_t *this,
							host_t *src, host_t *dst,
							traffic_selector_t *src_ts,
							traffic_selector_t *dst_ts,
							policy_dir_t direction, policy_type_t type,
							ipsec_sa_cfg_t *sa, mark_t mark,
							policy_priority_t priority);

	/**
	 * Query the use time of a policy.
	 *
	 * The use time of a policy is the time the policy was used for the last
	 * time. It is not the system time, but a monotonic timestamp as returned
	 * by time_monotonic.
	 *
	 * @param src_ts		traffic selector to match traffic source
	 * @param dst_ts		traffic selector to match traffic dest
	 * @param direction		direction of traffic, POLICY_(IN|OUT|FWD)
	 * @param mark			optional mark
	 * @param[out] use_time	the monotonic timestamp of this SA's last use
	 * @return				SUCCESS if operation completed
	 */
	status_t (*query_policy) (kernel_ipsec_t *this,
							  traffic_selector_t *src_ts,
							  traffic_selector_t *dst_ts,
							  policy_dir_t direction, mark_t mark,
							  time_t *use_time);

	/**
	 * Remove a policy from the SPD.
	 *
	 * The kernel interface implements reference counting for policies.
	 * If the same policy is installed multiple times (in the case of rekeying),
	 * the reference counter is increased. del_policy() decreases the ref counter
	 * and removes the policy only when no more references are available.
	 *
	 * @param src_ts		traffic selector to match traffic source
	 * @param dst_ts		traffic selector to match traffic dest
	 * @param direction		direction of traffic, POLICY_(IN|OUT|FWD)
	 * @param reqid			unique ID of the associated SA
	 * @param mark			optional mark
	 * @param priority		priority of the policy
	 * @return				SUCCESS if operation completed
	 */
	status_t (*del_policy) (kernel_ipsec_t *this,
							traffic_selector_t *src_ts,
							traffic_selector_t *dst_ts,
							policy_dir_t direction, u_int32_t reqid,
							mark_t mark, policy_priority_t priority);

	/**
	 * Flush all policies from the SPD.
	 *
	 * @return				SUCCESS if operation completed
	 */
	status_t (*flush_policies) (kernel_ipsec_t *this);

	/**
	 * Install a bypass policy for the given socket.
	 *
	 * @param fd			socket file descriptor to setup policy for
	 * @param family		protocol family of the socket
	 * @return				TRUE of policy set up successfully
	 */
	bool (*bypass_socket)(kernel_ipsec_t *this, int fd, int family);

	/**
	 * Enable decapsulation of ESP-in-UDP packets for the given port/socket.
	 *
	 * @param fd			socket file descriptor
	 * @param family		protocol family of the socket
	 * @param port			the UDP port
	 * @return				TRUE if UDP decapsulation was enabled successfully
	 */
	bool (*enable_udp_decap)(kernel_ipsec_t *this, int fd, int family,
							 u_int16_t port);

	/**
	 * Destroy the implementation.
	 */
	void (*destroy) (kernel_ipsec_t *this);
};

/**
 * Helper function to (un-)register IPsec kernel interfaces from plugin features.
 *
 * This function is a plugin_feature_callback_t and can be used with the
 * PLUGIN_CALLBACK macro to register an IPsec kernel interface constructor.
 *
 * @param plugin		plugin registering the kernel interface
 * @param feature		associated plugin feature
 * @param reg			TRUE to register, FALSE to unregister
 * @param data			data passed to callback, an kernel_ipsec_constructor_t
 */
bool kernel_ipsec_register(plugin_t *plugin, plugin_feature_t *feature,
						   bool reg, void *data);

#endif /** KERNEL_IPSEC_H_ @}*/
