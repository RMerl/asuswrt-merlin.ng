/*
 * Copyright (C) 2008-2012 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
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
 * @defgroup kernel_net kernel_net
 * @{ @ingroup hkernel
 */

#ifndef KERNEL_NET_H_
#define KERNEL_NET_H_

typedef struct kernel_net_t kernel_net_t;
typedef enum kernel_address_type_t kernel_address_type_t;

#include <collections/enumerator.h>
#include <networking/host.h>
#include <plugins/plugin.h>
#include <kernel/kernel_interface.h>

/**
 * Type of addresses (e.g. when enumerating them)
 */
enum kernel_address_type_t {
	/** normal addresses (on regular, up, non-ignored) interfaces */
	ADDR_TYPE_REGULAR = (1 << 0),
	/** addresses on down interfaces */
	ADDR_TYPE_DOWN =  (1 << 1),
	/** addresses on ignored interfaces */
	ADDR_TYPE_IGNORED = (1 << 2),
	/** addresses on loopback interfaces */
	ADDR_TYPE_LOOPBACK = (1 << 3),
	/** virtual IP addresses */
	ADDR_TYPE_VIRTUAL = (1 << 4),
	/** to enumerate all available addresses */
	ADDR_TYPE_ALL = (1 << 5) - 1,
};

/**
 * Interface to the network subsystem of the kernel.
 *
 * The kernel network interface handles the communication with the kernel
 * for interface and IP address management.
 */
struct kernel_net_t {

	/**
	 * Get the feature set supported by this kernel backend.
	 *
	 * @return				ORed feature-set of backend
	 */
	kernel_feature_t (*get_features)(kernel_net_t *this);

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
	host_t* (*get_source_addr)(kernel_net_t *this, host_t *dest, host_t *src);

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
	 * @return				next hop address, NULL if unreachable
	 */
	host_t* (*get_nexthop)(kernel_net_t *this, host_t *dest, int prefix,
						   host_t *src);

	/**
	 * Get the interface name of a local address. Interfaces that are down or
	 * ignored by config are not considered.
	 *
	 * @param host			address to get interface name from
	 * @param name			allocated interface name (optional)
	 * @return				TRUE if interface found and usable
	 */
	bool (*get_interface) (kernel_net_t *this, host_t *host, char **name);

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
	enumerator_t *(*create_address_enumerator) (kernel_net_t *this,
												kernel_address_type_t which);

	/**
	 * Add a virtual IP to an interface.
	 *
	 * Virtual IPs are attached to an interface. If an IP is added multiple
	 * times, the IP is refcounted and not removed until del_ip() was called
	 * as many times as add_ip().
	 *
	 * @param virtual_ip	virtual ip address to assign
	 * @param prefix		prefix length to install with IP address, -1 for auto
	 * @param iface			interface to install virtual IP on
	 * @return				SUCCESS if operation completed
	 */
	status_t (*add_ip) (kernel_net_t *this, host_t *virtual_ip, int prefix,
						char *iface);

	/**
	 * Remove a virtual IP from an interface.
	 *
	 * The kernel interface uses refcounting, see add_ip().
	 *
	 * @param virtual_ip	virtual ip address to remove
	 * @param prefix		prefix length of the IP to uninstall, -1 for auto
	 * @param wait			TRUE to wait until IP is gone
	 * @return				SUCCESS if operation completed
	 */
	status_t (*del_ip) (kernel_net_t *this, host_t *virtual_ip, int prefix,
						bool wait);

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
	status_t (*add_route) (kernel_net_t *this, chunk_t dst_net,
						   u_int8_t prefixlen, host_t *gateway, host_t *src_ip,
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
	status_t (*del_route) (kernel_net_t *this, chunk_t dst_net,
						   u_int8_t prefixlen, host_t *gateway, host_t *src_ip,
						   char *if_name);

	/**
	 * Destroy the implementation.
	 */
	void (*destroy) (kernel_net_t *this);
};

/**
 * Helper function to (un-)register net kernel interfaces from plugin features.
 *
 * This function is a plugin_feature_callback_t and can be used with the
 * PLUGIN_CALLBACK macro to register an net kernel interface constructor.
 *
 * @param plugin		plugin registering the kernel interface
 * @param feature		associated plugin feature
 * @param reg			TRUE to register, FALSE to unregister
 * @param data			data passed to callback, an kernel_net_constructor_t
 */
bool kernel_net_register(plugin_t *plugin, plugin_feature_t *feature,
						 bool reg, void *data);

#endif /** KERNEL_NET_H_ @}*/
