/*
 * Copyright (C) 2010-2013 Tobias Brunner
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
 * @defgroup kernel_listener kernel_listener
 * @{ @ingroup kernel
 */

#ifndef KERNEL_LISTENER_H_
#define KERNEL_LISTENER_H_

typedef struct kernel_listener_t kernel_listener_t;
typedef struct kernel_acquire_data_t kernel_acquire_data_t;

#include <networking/host.h>
#include <networking/tun_device.h>
#include <selectors/traffic_selector.h>
#include <kernel/kernel_ipsec.h>

/**
 * Data received with a kernel's acquire, has to be cloned/copied by listener.
 */
struct kernel_acquire_data_t {
	/** Optional source of the triggering packet */
	traffic_selector_t *src;
	/** Optional destination of the triggering packet */
	traffic_selector_t *dst;
	/** Optional security label of the triggering packet */
	sec_label_t *label;
};

/**
 * Interface for components interested in kernel events.
 *
 * All hooks are optional.
 */
struct kernel_listener_t {

	/**
	 * Hook called if an acquire event for a policy is received.
	 *
	 * @param reqid			reqid of the policy to acquire
	 * @param data			data from the acquire
	 * @return				TRUE to remain registered, FALSE to unregister
	 */
	bool (*acquire)(kernel_listener_t *this, uint32_t reqid,
					kernel_acquire_data_t *data);

	/**
	 * Hook called if an expire event for an IPsec SA is received.
	 *
	 * @param protocol		protocol of the expired SA
	 * @param spi			spi of the expired SA
	 * @param dst			destination address of expired SA
	 * @param hard			TRUE if it is a hard expire, FALSE otherwise
	 * @return				TRUE to remain registered, FALSE to unregister
	 */
	bool (*expire)(kernel_listener_t *this, uint8_t protocol, uint32_t spi,
				   host_t *dst, bool hard);

	/**
	 * Hook called if the NAT mappings of an IPsec SA changed.
	 *
	 * @param protocol		IPsec protocol of affected SA
	 * @param spi			spi of the SA
	 * @param dst			old destination address of SA
	 * @param remote		new remote host
	 * @return				TRUE to remain registered, FALSE to unregister
	 */
	bool (*mapping)(kernel_listener_t *this, uint8_t protocol, uint32_t spi,
					host_t *dst, host_t *remote);

	/**
	 * Hook called if a migrate event for a policy is received.
	 *
	 * @param reqid			reqid of the policy
	 * @param src_ts		source traffic selector
	 * @param dst_ts		destination traffic selector
	 * @param direction		direction of the policy (in|out)
	 * @param local			local host address to be used in the IKE_SA
	 * @param remote		remote host address to be used in the IKE_SA
	 * @return				TRUE to remain registered, FALSE to unregister
	 */
	bool (*migrate)(kernel_listener_t *this, uint32_t reqid,
					traffic_selector_t *src_ts, traffic_selector_t *dst_ts,
					policy_dir_t direction, host_t *local, host_t *remote);

	/**
	 * Hook called if changes in the networking layer occurred (interfaces
	 * up/down, routes added/deleted etc.).
	 *
	 * @param address		TRUE if address list, FALSE if routing changed
	 * @return				TRUE to remain registered, FALSE to unregister
	 */
	bool (*roam)(kernel_listener_t *this, bool address);

	/**
	 * Hook called after a TUN device was created for a virtual IP address, or
	 * before such a device gets destroyed.
	 *
	 * @param tun			TUN device
	 * @param created		TRUE if created, FALSE if going to be destroyed
	 */
	bool (*tun)(kernel_listener_t *this, tun_device_t *tun, bool created);
};

#endif /** KERNEL_LISTENER_H_ @}*/
