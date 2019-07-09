/*
 * Copyright (C) 2012-2014 Tobias Brunner
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
 * @defgroup vpnservice_builder vpnservice_builder
 * @{ @ingroup libandroidbridge
 */

#ifndef VPNSERVICE_BUILDER_H_
#define VPNSERVICE_BUILDER_H_

#include <jni.h>

#include <library.h>
#include <networking/host.h>

typedef struct vpnservice_builder_t vpnservice_builder_t;

/**
 * VpnService.Builder, used to build a TUN device.
 *
 * Communicates with CharonVpnService.BuilderAdapter via JNI
 */
struct vpnservice_builder_t {

	/**
	 * Add an interface address
	 *
	 * @param addr				the desired interface address
	 * @return					TRUE on success
	 */
	bool (*add_address)(vpnservice_builder_t *this, host_t *addr);

	/**
	 * Add a route
	 *
	 * @param net				the network address
	 * @param prefix_length		the prefix length
	 * @return					TRUE on success
	 */
	bool (*add_route)(vpnservice_builder_t *this, host_t *net, int prefix);

	/**
	 * Add a DNS server
	 *
	 * @param dns				the address of the DNS server
	 * @return					TRUE on success
	 */
	bool (*add_dns)(vpnservice_builder_t *this, host_t *dns);

	/**
	 * Set the MTU for the TUN device
	 *
	 * @param mtu				the MTU to set
	 * @return					TRUE on success
	 */
	bool (*set_mtu)(vpnservice_builder_t *this,	int mtu);

	/**
	 * Build the TUN device
	 *
	 * @return					the TUN file descriptor, -1 if failed
	 */
	int (*establish)(vpnservice_builder_t *this);

	/**
	 * Build the TUN device without DNS related data
	 *
	 * @return					the TUN file descriptor, -1 if failed
	 */
	int (*establish_no_dns)(vpnservice_builder_t *this);

	/**
	 * Destroy a vpnservice_builder
	 */
	void (*destroy)(vpnservice_builder_t *this);

};

/**
 * Create a vpnservice_builder instance
 *
 * @param builder				CharonVpnService.BuilderAdapter object
 * @return						vpnservice_builder_t instance
 */
vpnservice_builder_t *vpnservice_builder_create(jobject builder);

#endif /** VPNSERVICE_BUILDER_H_ @}*/
