/*
 * Copyright (C) 2006-2016 Tobias Brunner
 * Copyright (C) 2006 Daniel Roethlisberger
 * Copyright (C) 2005-2006 Martin Willi
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

/*
 * Copyright (c) 2012 Nanoteq Pty Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @defgroup kernel_interface kernel_interface
 * @{ @ingroup kernel
 */

#ifndef KERNEL_INTERFACE_H_
#define KERNEL_INTERFACE_H_

typedef struct kernel_interface_t kernel_interface_t;
typedef enum kernel_feature_t kernel_feature_t;

#include <networking/host.h>
#include <crypto/prf_plus.h>

#include <kernel/kernel_listener.h>
#include <kernel/kernel_ipsec.h>
#include <kernel/kernel_net.h>

/**
 * Default range for SPIs requested from kernels
 */
#define KERNEL_SPI_MIN 0xc0000000
#define KERNEL_SPI_MAX 0xcfffffff

/**
 * Bitfield of optional features a kernel backend supports.
 *
 * This feature-set is for both, kernel_ipsec_t and kernel_net_t. Each
 * backend returns a subset of these features.
 */
enum kernel_feature_t {
	/** IPsec can process ESPv3 (RFC 4303) TFC padded packets */
	KERNEL_ESP_V3_TFC = (1<<0),
	/** Networking requires an "exclude" route for IKE/ESP packets */
	KERNEL_REQUIRE_EXCLUDE_ROUTE = (1<<1),
	/** IPsec implementation requires UDP encapsulation of ESP packets */
	KERNEL_REQUIRE_UDP_ENCAPSULATION = (1<<2),
	/** IPsec backend does not require a policy reinstall on SA updates */
	KERNEL_NO_POLICY_UPDATES = (1<<3),
	/** IPsec backend supports installing SPIs on policies */
	KERNEL_POLICY_SPI = (1<<4),
};

/**
 * Constructor function for ipsec kernel interface
 */
typedef kernel_ipsec_t* (*kernel_ipsec_constructor_t)(void);

/**
 * Constructor function for network kernel interface
 */
typedef kernel_net_t* (*kernel_net_constructor_t)(void);

/**
 * Manager and wrapper for different kernel interfaces.
 *
 * The kernel interface handles the communication with the kernel
 * for SA and policy management and interface and IP address management.
 */
struct kernel_interface_t {

	/**
	 * Get the feature set supported by the net and ipsec kernel backends.
	 *
	 * @return				ORed feature-set of backends
	 */
	kernel_feature_t (*get_features)(kernel_interface_t *this);

	/**
	 * Get a SPI from the kernel.
	 *
	 * @param src		source address of SA
	 * @param dst		destination address of SA
	 * @param protocol	protocol for SA (ESP/AH)
	 * @param spi		allocated spi
	 * @return			SUCCESS if operation completed
	 */
	status_t (*get_spi)(kernel_interface_t *this, host_t *src, host_t *dst,
						uint8_t protocol, uint32_t *spi);

	/**
	 * Get a Compression Parameter Index (CPI) from the kernel.
	 *
	 * @param src		source address of SA
	 * @param dst		destination address of SA
	 * @param cpi		allocated cpi
	 * @return			SUCCESS if operation completed
	 */
	status_t (*get_cpi)(kernel_interface_t *this, host_t *src, host_t *dst,
						uint16_t *cpi);

	/**
	 * Allocate or confirm a reqid to use for a given SA pair.
	 *
	 * Each returned reqid by a successful call to alloc_reqid() must be
	 * released using release_reqid().
	 *
	 * The reqid parameter is an in/out parameter. If it points to non-zero,
	 * the reqid is confirmed and registered for use. If it points to zero,
	 * a reqid is allocated for the given selectors, and returned to reqid.
	 *
	 * @param local_ts	traffic selectors of local side for SA
	 * @param remote_ts	traffic selectors of remote side for SA
	 * @param mark_in	inbound mark on SA
	 * @param mark_out	outbound mark on SA
	 * @param reqid		allocated reqid
	 * @return			SUCCESS if reqid allocated
	 */
	status_t (*alloc_reqid)(kernel_interface_t *this,
							linked_list_t *local_ts, linked_list_t *remote_ts,
							mark_t mark_in, mark_t mark_out,
							uint32_t *reqid);

	/**
	 * Release a previously allocated reqid.
	 *
	 * @param reqid		reqid to release
	 * @param mark_in	inbound mark on SA
	 * @param mark_out	outbound mark on SA
	 * @return			SUCCESS if reqid released
	 */
	status_t (*release_reqid)(kernel_interface_t *this, uint32_t reqid,
							  mark_t mark_in, mark_t mark_out);

	/**
	 * Add an SA to the SAD.
	 *
	 * This function does install a single SA for a single protocol in one
	 * direction.
	 *
	 * @param id			data identifying this SA
	 * @param data			data for this SA
	 * @return				SUCCESS if operation completed
	 */
	status_t (*add_sa)(kernel_interface_t *this, kernel_ipsec_sa_id_t *id,
					   kernel_ipsec_add_sa_t *data);

	/**
	 * Update the hosts on an installed SA.
	 *
	 * We cannot directly update the destination address as the kernel
	 * requires the spi, the protocol AND the destination address (and family)
	 * to identify SAs. Therefore if the destination address changed we
	 * create a new SA and delete the old one.
	 *
	 * @param id			data identifying this SA
	 * @param data			updated data for this SA
	 * @return				SUCCESS if operation completed, NOT_SUPPORTED if
	 *						the kernel interface can't update the SA
	 */
	status_t (*update_sa)(kernel_interface_t *this, kernel_ipsec_sa_id_t *id,
						  kernel_ipsec_update_sa_t *data);

	/**
	 * Query the number of bytes processed by an SA from the SAD.
	 *
	 * @param id			data identifying this SA
	 * @param data			data to query the SA
	 * @param[out] bytes	the number of bytes processed by SA
	 * @param[out] packets	number of packets processed by SA
	 * @param[out] time		last (monotonic) time of SA use
	 * @return				SUCCESS if operation completed
	 */
	status_t (*query_sa)(kernel_interface_t *this, kernel_ipsec_sa_id_t *id,
						 kernel_ipsec_query_sa_t *data, uint64_t *bytes,
						 uint64_t *packets, time_t *time);

	/**
	 * Delete a previously installed SA from the SAD.
	 *
	 * @param id			data identifying this SA
	 * @param data			data to delete the SA
	 * @return				SUCCESS if operation completed
	 */
	status_t (*del_sa)(kernel_interface_t *this, kernel_ipsec_sa_id_t *id,
					   kernel_ipsec_del_sa_t *data);

	/**
	 * Flush all SAs from the SAD.
	 *
	 * @return				SUCCESS if operation completed
	 */
	status_t (*flush_sas)(kernel_interface_t *this);

	/**
	 * Add a policy to the SPD.
	 *
	 * @param id			data identifying this policy
	 * @param data			data for this policy
	 * @return				SUCCESS if operation completed
	 */
	status_t (*add_policy)(kernel_interface_t *this,
						   kernel_ipsec_policy_id_t *id,
						   kernel_ipsec_manage_policy_t *data);

	/**
	 * Query the use time of a policy.
	 *
	 * The use time of a policy is the time the policy was used
	 * for the last time.
	 *
	 * @param id			data identifying this policy
	 * @param data			data to query the policy
	 * @param[out] use_time	the monotonic timestamp of this SA's last use
	 * @return				SUCCESS if operation completed
	 */
	status_t (*query_policy)(kernel_interface_t *this,
							 kernel_ipsec_policy_id_t *id,
							 kernel_ipsec_query_policy_t *data,
							 time_t *use_time);

	/**
	 * Remove a policy from the SPD.
	 *
	 * @param id			data identifying this policy
	 * @param data			data for this policy
	 * @return				SUCCESS if operation completed
	 */
	status_t (*del_policy)(kernel_interface_t *this,
						   kernel_ipsec_policy_id_t *id,
						   kernel_ipsec_manage_policy_t *data);

	/**
	 * Flush all policies from the SPD.
	 *
	 * @return				SUCCESS if operation completed
	 */
	status_t (*flush_policies)(kernel_interface_t *this);

	/**
	 * Get our outgoing source address for a destination.
	 *
	 * Does a route lookup to get the source address used to reach dest.
	 * The returned host is allocated and must be destroyed.
	 * An optional src address can be used to check if a route is available
	 * for the given source to dest.
	 *
	 * @param dest			target destination address
	 * @param src			source address to check, or NULL
	 * @return				outgoing source address, NULL if unreachable
	 */
	host_t* (*get_source_addr)(kernel_interface_t *this,
							   host_t *dest, host_t *src);

	/**
	 * Get the next hop for a destination.
	 *
	 * Does a route lookup to get the next hop used to reach dest.
	 * The returned host is allocated and must be destroyed.
	 * An optional src address can be used to check if a route is available
	 * for the given source to dest.
	 *
	 * @param dest			target destination address
	 * @param prefix		prefix length if dest is a subnet, -1 for auto
	 * @param src			source address to check, or NULL
	 * @param[out] iface	allocated name of the interface to reach dest, if
	 *						available (optional)
	 * @return				next hop address, NULL if unreachable
	 */
	host_t* (*get_nexthop)(kernel_interface_t *this, host_t *dest,
						   int prefix, host_t *src, char **iface);

	/**
	 * Get the interface name of a local address. Interfaces that are down or
	 * ignored by config are not considered.
	 *
	 * @param host			address to get interface name from
	 * @param name			allocated interface name (optional)
	 * @return				TRUE if interface found and usable
	 */
	bool (*get_interface)(kernel_interface_t *this, host_t *host, char **name);

	/**
	 * Creates an enumerator over all local addresses.
	 *
	 * This function blocks an internal cached address list until the
	 * enumerator gets destroyed.
	 * The hosts are read-only, do not modify of free.
	 *
	 * @param which			a combination of address types to enumerate
	 * @return				enumerator over host_t's
	 */
	enumerator_t *(*create_address_enumerator) (kernel_interface_t *this,
												kernel_address_type_t which);

	/**
	 * Creates an enumerator over all local subnets.
	 *
	 * Local subnets are subnets the host is directly connected to.
	 *
	 * The enumerator returns the network, subnet mask and interface.
	 *
	 * @return				enumerator over host_t*, uint8_t, char*
	 */
	enumerator_t *(*create_local_subnet_enumerator)(kernel_interface_t *this);

	/**
	 * Add a virtual IP to an interface.
	 *
	 * Virtual IPs are attached to an interface. If an IP is added multiple
	 * times, the IP is refcounted and not removed until del_ip() was called
	 * as many times as add_ip().
	 *
	 * @param virtual_ip	virtual ip address to assign
	 * @param prefix		prefix length to install IP with, -1 for auto
	 * @param iface			interface to install virtual IP on
	 * @return				SUCCESS if operation completed
	 */
	status_t (*add_ip) (kernel_interface_t *this, host_t *virtual_ip, int prefix,
						char *iface);

	/**
	 * Remove a virtual IP from an interface.
	 *
	 * The kernel interface uses refcounting, see add_ip().
	 *
	 * @param virtual_ip	virtual ip address to remove
	 * @param prefix		prefix length of the IP to uninstall, -1 for auto
	 * @param wait			TRUE to wait untily IP is gone
	 * @return				SUCCESS if operation completed
	 */
	status_t (*del_ip) (kernel_interface_t *this, host_t *virtual_ip,
						int prefix, bool wait);

	/**
	 * Add a route.
	 *
	 * @param dst_net		destination net
	 * @param prefixlen		destination net prefix length
	 * @param gateway		gateway for this route
	 * @param src_ip		source ip of the route
	 * @param if_name		name of the interface the route is bound to
	 * @return				SUCCESS if operation completed
	 *						ALREADY_DONE if the route already exists
	 */
	status_t (*add_route) (kernel_interface_t *this, chunk_t dst_net,
						   uint8_t prefixlen, host_t *gateway, host_t *src_ip,
						   char *if_name);

	/**
	 * Delete a route.
	 *
	 * @param dst_net		destination net
	 * @param prefixlen		destination net prefix length
	 * @param gateway		gateway for this route
	 * @param src_ip		source ip of the route
	 * @param if_name		name of the interface the route is bound to
	 * @return				SUCCESS if operation completed
	 */
	status_t (*del_route) (kernel_interface_t *this, chunk_t dst_net,
						   uint8_t prefixlen, host_t *gateway, host_t *src_ip,
						   char *if_name);

	/**
	 * Set up a bypass policy for a given socket.
	 *
	 * @param fd			socket file descriptor to setup policy for
	 * @param family		protocol family of the socket
	 * @return				TRUE if policy set up successfully
	 */
	bool (*bypass_socket)(kernel_interface_t *this, int fd, int family);

	/**
	 * Enable decapsulation of ESP-in-UDP packets for the given port/socket.
	 *
	 * @param fd			socket file descriptor
	 * @param family		protocol family of the socket
	 * @param port			the UDP port
	 * @return				TRUE if UDP decapsulation was enabled successfully
	 */
	bool (*enable_udp_decap)(kernel_interface_t *this, int fd, int family,
							 uint16_t port);


	/**
	 * manager methods
	 */

	/**
	 * Verifies that the given interface is usable and not excluded by
	 * configuration.
	 *
	 * @param iface			interface name
	 * @return				TRUE if usable
	 */
	bool (*is_interface_usable)(kernel_interface_t *this, const char *iface);

	/**
	 * Check if interfaces are excluded by config.
	 *
	 * @return				TRUE if no interfaces are exclued by config
	 */
	bool (*all_interfaces_usable)(kernel_interface_t *this);

	/**
	 * Tries to find an IP address of a local interface that is included in the
	 * supplied traffic selector.
	 *
	 * @param ts			traffic selector
	 * @param ip			returned IP address (has to be destroyed)
	 * @param vip			set to TRUE if returned address is a virtual IP
	 * @return				SUCCESS if address found
	 */
	status_t (*get_address_by_ts)(kernel_interface_t *this,
								  traffic_selector_t *ts, host_t **ip, bool *vip);

	/**
	 * Register an ipsec kernel interface constructor on the manager.
	 *
	 * @param create		constructor to register
	 * @return				TRUE if the ipsec kernel interface was registered
	 *						successfully, FALSE if an interface was already
	 *						registered or the registration failed
	 */
	bool (*add_ipsec_interface)(kernel_interface_t *this,
								kernel_ipsec_constructor_t create);

	/**
	 * Unregister an ipsec kernel interface constructor.
	 *
	 * @param create		constructor to unregister
	 * @return				TRUE if the ipsec kernel interface was unregistered
	 *						successfully, FALSE otherwise
	 */
	bool (*remove_ipsec_interface)(kernel_interface_t *this,
								   kernel_ipsec_constructor_t create);

	/**
	 * Register a network kernel interface constructor on the manager.
	 *
	 * @param create		constructor to register
	 * @return				TRUE if the kernel net interface was registered
	 *						successfully, FALSE if an interface was already
	 *						registered or the registration failed
	 */
	bool (*add_net_interface)(kernel_interface_t *this,
							  kernel_net_constructor_t create);

	/**
	 * Unregister a network kernel interface constructor.
	 *
	 * @param create		constructor to unregister
	 * @return				TRUE if the kernel net interface was unregistered
	 *						successfully, FALSE otherwise
	 */
	bool (*remove_net_interface)(kernel_interface_t *this,
								 kernel_net_constructor_t create);

	/**
	 * Add a listener to the kernel interface.
	 *
	 * @param listener		listener to add
	 */
	void (*add_listener)(kernel_interface_t *this,
						 kernel_listener_t *listener);

	/**
	 * Remove a listener from the kernel interface.
	 *
	 * @param listener		listener to remove
	 */
	void (*remove_listener)(kernel_interface_t *this,
							kernel_listener_t *listener);

	/**
	 * Raise an acquire event.
	 *
	 * @param reqid			reqid of the policy to acquire
	 * @param src_ts		source traffic selector
	 * @param dst_ts		destination traffic selector
	 */
	void (*acquire)(kernel_interface_t *this, uint32_t reqid,
					traffic_selector_t *src_ts, traffic_selector_t *dst_ts);

	/**
	 * Raise an expire event.
	 *
	 * @param protocol		protocol of the expired SA
	 * @param spi			spi of the expired SA
	 * @param dst			destination address of expired SA
	 * @param hard			TRUE if it is a hard expire, FALSE otherwise
	 */
	void (*expire)(kernel_interface_t *this, uint8_t protocol, uint32_t spi,
				   host_t *dst, bool hard);

	/**
	 * Raise a mapping event.
	 *
	 * @param protocol		protocol of affected SA
	 * @param spi			spi of the SA
	 * @param dst			original destination address of SA
	 * @param remote		new remote host
	 */
	void (*mapping)(kernel_interface_t *this, uint8_t protocol, uint32_t spi,
					host_t *dst, host_t *remote);

	/**
	 * Raise a migrate event.
	 *
	 * @param reqid			reqid of the policy
	 * @param src_ts		source traffic selector
	 * @param dst_ts		destination traffic selector
	 * @param direction		direction of the policy (in|out)
	 * @param local			local host address to be used in the IKE_SA
	 * @param remote		remote host address to be used in the IKE_SA
	 */
	void (*migrate)(kernel_interface_t *this, uint32_t reqid,
					traffic_selector_t *src_ts, traffic_selector_t *dst_ts,
					policy_dir_t direction, host_t *local, host_t *remote);

	/**
	 * Raise a roam event.
	 *
	 * @param address		TRUE if address list, FALSE if routing changed
	 */
	void (*roam)(kernel_interface_t *this, bool address);

	/**
	 * Raise a tun event.
	 *
	 * @param tun			TUN device
	 * @param created		TRUE if created, FALSE if going to be destroyed
	 */
	void (*tun)(kernel_interface_t *this, tun_device_t *tun, bool created);

	/**
	 * Register a new algorithm with the kernel interface.
	 *
	 * @param alg_id			the IKE id of the algorithm
	 * @param type				the transform type of the algorithm
	 * @param kernel_id			the kernel id of the algorithm
	 * @param kernel_name		the kernel name of the algorithm
	 */
	void (*register_algorithm)(kernel_interface_t *this, uint16_t alg_id,
							   transform_type_t type, uint16_t kernel_id,
							   char *kernel_name);

	/**
	 * Return the kernel-specific id and/or name for an algorithms depending on
	 * the arguments specified.
	 *
	 * @param alg_id			the IKE id of the algorithm
	 * @param type				the transform type of the algorithm
	 * @param kernel_id			the kernel id of the algorithm (optional)
	 * @param kernel_name		the kernel name of the algorithm (optional)
	 * @return					TRUE if algorithm was found
	 */
	bool (*lookup_algorithm)(kernel_interface_t *this, uint16_t alg_id,
							 transform_type_t type, uint16_t *kernel_id,
							 char **kernel_name);

	/**
	 * Destroys a kernel_interface_t object.
	 */
	void (*destroy) (kernel_interface_t *this);
};

/**
 * Creates an object of type kernel_interface_t.
 */
kernel_interface_t *kernel_interface_create(void);

#endif /** KERNEL_INTERFACE_H_ @}*/
