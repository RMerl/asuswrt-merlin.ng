/*
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

#ifndef IFACE_H
#define IFACE_H

#include <library.h>
#include <collections/enumerator.h>
#include <networking/host.h>

#define TAP_DEVICE "/dev/net/tun"

typedef struct iface_t iface_t;

#include "mconsole.h"
#include "bridge.h"
#include "guest.h"

/**
 * Interface in a guest, connected to a tap device on the host.
 */
struct iface_t {

	/**
	 * Get the interface name in the guest (e.g. eth0).
	 *
	 * @return			guest interface name
	 */
	char* (*get_guestif)(iface_t *this);

	/**
	 * Get the interface name at the host (e.g. tap0).
	 *
	 * @return			host interface (tap device) name
	 */
	char* (*get_hostif)(iface_t *this);

	/**
	 * Add an address to the interface.
	 *
	 * @param addr		address to add to the interface
	 * @param bits		network prefix length in bits
	 * @return			TRUE if address added
	 */
	bool (*add_address)(iface_t *this, host_t *addr, int bits);

	/**
	 * Create an enumerator over all installed addresses.
	 *
	 * @return			enumerator over host_t*
	 */
	enumerator_t* (*create_address_enumerator)(iface_t *this);

	/**
	 * Remove an address from an interface.
	 *
	 * @note The network prefix length has to be the same as used in add_address
	 *
	 * @param addr		address to remove
	 * @param bits		network prefix length in bits
	 * @return			TRUE if address removed
	 */
	bool (*delete_address)(iface_t *this, host_t *addr, int bits);

	/**
	 * Set the bridge this interface is attached to.
	 *
	 * @param bridge	assigned bridge, or NULL for none
	 */
	void (*set_bridge)(iface_t *this, bridge_t *bridge);

	/**
	 * Get the bridge this iface is connected, or NULL.
	 *
	 * @return			connected bridge, or NULL
	 */
	bridge_t* (*get_bridge)(iface_t *this);

	/**
	 * Get the guest this iface belongs to.
	 *
	 * @return			guest of this iface
	 */
	guest_t* (*get_guest)(iface_t *this);

	/**
	 * Destroy an interface
	 */
	void (*destroy) (iface_t *this);
};

/**
 * Create a new interface for a guest
 *
 * @param name		name of the interface in the guest
 * @param guest		guest this iface is connecting
 * @param mconsole	mconsole of guest
 * @return			interface descriptor, or NULL if failed
 */
iface_t *iface_create(char *name, guest_t *guest, mconsole_t *mconsole);

#endif /* IFACE_H */

