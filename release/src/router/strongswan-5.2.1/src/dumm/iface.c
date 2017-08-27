/*
 * Copyright (C) 2008 Tobias Brunner
 * Copyright (C) 2007 Martin Willi
 * Hochschule fuer Technik Rapperswil
 * Copyright (C) 2002 Jeff Dike
 *
 * Based on the "tunctl" utility from Jeff Dike.
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

#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/if_tun.h>

#include <utils/debug.h>
#include <collections/linked_list.h>

#include "iface.h"

typedef struct private_iface_t private_iface_t;

struct private_iface_t {
	/** public interface */
	iface_t public;
	/** device name in guest (eth0) */
	char *guestif;
	/** device name at host (tap0) */
	char *hostif;
	/** bridge this interface is attached to */
	bridge_t *bridge;
	/** guest this interface is attached to */
	guest_t *guest;
	/** mconsole for guest */
	mconsole_t *mconsole;
};

/**
 * bring an interface up or down (host side)
 */
bool iface_control(char *name, bool up)
{
	int s;
	bool good = FALSE;
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (!s)
	{
		return FALSE;
	}
	if (ioctl(s, SIOCGIFFLAGS, &ifr) == 0)
	{
		if (up)
		{
			ifr.ifr_flags |= IFF_UP;
		}
		else
		{
			ifr.ifr_flags &= ~IFF_UP;
		}
		if (ioctl(s, SIOCSIFFLAGS, &ifr) == 0)
		{
			good = TRUE;
		}
	}
	close(s);
	return good;
}

METHOD(iface_t, get_guestif, char*,
	private_iface_t *this)
{
	return this->guestif;
}

METHOD(iface_t, get_hostif, char*,
	private_iface_t *this)
{
	return this->hostif;
}

METHOD(iface_t, add_address, bool,
	private_iface_t *this, host_t *addr, int bits)
{
	return (this->guest->exec(this->guest, NULL, NULL,
			"exec ip addr add %H/%d dev %s", addr, bits, this->guestif) == 0);
}

/**
 * compile a list of the addresses of an interface
 */
static void compile_address_list(linked_list_t *list, char *address)
{
	host_t *host = host_create_from_string(address, 0);
	if (host)
	{
		list->insert_last(list, host);
	}
}

/**
 * delete the list of addresses
 */
static void destroy_address_list(linked_list_t *list)
{
	list->destroy_offset(list, offsetof(host_t, destroy));
}

METHOD(iface_t, create_address_enumerator, enumerator_t*,
	private_iface_t *this)
{
	linked_list_t *addresses = linked_list_create();
	this->guest->exec_str(this->guest, (void(*)(void*,char*))compile_address_list,
			TRUE, addresses,
			"exec ip addr list dev %s scope global | "
			"grep '^ \\+\\(inet6\\? \\)' | "
			"awk -F '( +|/)' '{ print $3 }'", this->guestif);
	return enumerator_create_cleaner(addresses->create_enumerator(addresses),
					(void(*)(void*))destroy_address_list, addresses);
}

METHOD(iface_t, delete_address, bool,
	private_iface_t *this, host_t *addr, int bits)
{
	return (this->guest->exec(this->guest, NULL, NULL,
			"exec ip addr del %H/%d dev %s", addr, bits, this->guestif) == 0);
}

METHOD(iface_t, set_bridge, void,
	private_iface_t *this, bridge_t *bridge)
{
	if (this->bridge == NULL && bridge)
	{
		this->guest->exec(this->guest, NULL, NULL,
						  "exec ip link set %s up", this->guestif);
	}
	else if (this->bridge && bridge == NULL)
	{
		this->guest->exec(this->guest, NULL, NULL,
						  "exec ip link set %s down", this->guestif);
	}
	this->bridge = bridge;
}

METHOD(iface_t, get_bridge, bridge_t*,
	private_iface_t *this)
{
	return this->bridge;
}

METHOD(iface_t, get_guest, guest_t*,
	private_iface_t *this)
{
	return this->guest;
}

/**
 * destroy the tap device
 */
static bool destroy_tap(private_iface_t *this)
{
	struct ifreq ifr;
	int tap;

	if (!iface_control(this->hostif, FALSE))
	{
		DBG1(DBG_LIB, "bringing iface down failed: %m");
	}
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	strncpy(ifr.ifr_name, this->hostif, sizeof(ifr.ifr_name) - 1);

	tap = open(TAP_DEVICE, O_RDWR);
	if (tap < 0)
	{
		DBG1(DBG_LIB, "unable to open tap device %s: %m", TAP_DEVICE);
		return FALSE;
	}
	if (ioctl(tap, TUNSETIFF, &ifr) < 0 ||
		ioctl(tap, TUNSETPERSIST, 0) < 0)
	{
		DBG1(DBG_LIB, "removing %s failed: %m", this->hostif);
		close(tap);
		return FALSE;
	}
	close(tap);
	return TRUE;
}

/**
 * create the tap device
 */
static char* create_tap(private_iface_t *this)
{
	struct ifreq ifr;
	int tap;

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s-%s",
			 this->guest->get_name(this->guest), this->guestif);

	tap = open(TAP_DEVICE, O_RDWR);
	if (tap < 0)
	{
		DBG1(DBG_LIB, "unable to open tap device %s: %m", TAP_DEVICE);
		return NULL;
	}
	if (ioctl(tap, TUNSETIFF, &ifr) < 0 ||
		ioctl(tap, TUNSETPERSIST, 1) < 0 ||
		ioctl(tap, TUNSETOWNER, 0))
	{
		DBG1(DBG_LIB, "creating new tap device failed: %m");
		close(tap);
		return NULL;
	}
	close(tap);
	return strdup(ifr.ifr_name);
}

METHOD(iface_t, destroy, void,
	private_iface_t *this)
{
	if (this->bridge)
	{
		this->bridge->disconnect_iface(this->bridge, &this->public);
	}
	/* TODO: iface mgmt is not blocking yet, so wait some ticks */
	usleep(50000);
	this->mconsole->del_iface(this->mconsole, this->guestif);
	destroy_tap(this);
	free(this->guestif);
	free(this->hostif);
	free(this);
}

/**
 * create the iface instance
 */
iface_t *iface_create(char *name, guest_t *guest, mconsole_t *mconsole)
{
	private_iface_t *this;

	INIT(this,
		.public = {
			.get_hostif = _get_hostif,
			.get_guestif = _get_guestif,
			.add_address = _add_address,
			.create_address_enumerator = _create_address_enumerator,
			.delete_address = _delete_address,
			.set_bridge = _set_bridge,
			.get_bridge = _get_bridge,
			.get_guest = _get_guest,
			.destroy = _destroy,
		},
		.mconsole = mconsole,
		.guestif = strdup(name),
		.guest = guest,
	);
	this->hostif = create_tap(this);
	if (this->hostif == NULL)
	{
		destroy_tap(this);
		free(this->guestif);
		free(this);
		return NULL;
	}
	if (!this->mconsole->add_iface(this->mconsole, this->guestif, this->hostif))
	{
		DBG1(DBG_LIB, "creating interface '%s' in guest failed", this->guestif);
		destroy_tap(this);
		free(this->guestif);
		free(this->hostif);
		free(this);
		return NULL;
	}
	if (!iface_control(this->hostif, TRUE))
	{
		DBG1(DBG_LIB, "bringing iface '%s' up failed: %m", this->hostif);
	}
	return &this->public;
}

