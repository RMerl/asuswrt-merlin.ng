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

#include <sys/types.h>
#include <netinet/in.h>
#include <libbridge.h>

#include <utils/debug.h>
#include <collections/linked_list.h>

#include "bridge.h"

typedef struct private_bridge_t private_bridge_t;

struct private_bridge_t {
	/** public interface */
	bridge_t public;
	/** device name */
	char *name;
	/** list of attached interfaces */
	linked_list_t *ifaces;
};

/**
 * defined in iface.c
 */
bool iface_control(char *name, bool up);

METHOD(bridge_t, get_name, char*,
	private_bridge_t *this)
{
	return this->name;
}

METHOD(bridge_t, create_iface_enumerator, enumerator_t*,
	private_bridge_t *this)
{
	return this->ifaces->create_enumerator(this->ifaces);
}

METHOD(bridge_t, disconnect_iface, bool,
	private_bridge_t *this, iface_t *iface)
{
	enumerator_t *enumerator;
	iface_t *current = NULL;
	bool good = FALSE;

	enumerator = this->ifaces->create_enumerator(this->ifaces);
	while (enumerator->enumerate(enumerator, (void**)&current))
	{
		if (current == iface)
		{
			if (br_del_interface(this->name, iface->get_hostif(iface)) != 0)
			{
				DBG1(DBG_LIB, "removing iface '%s' from bridge '%s' in kernel"
					 " failed: %m", iface->get_hostif(iface), this->name);
			}
			else
			{
				iface->set_bridge(iface, NULL);
				this->ifaces->remove_at(this->ifaces, enumerator);
				good = TRUE;
			}
			break;
		}
	}
	if (iface != current)
	{
		DBG1(DBG_LIB, "iface '%s' not found on bridge '%s'",
			 iface->get_hostif(iface), this->name);
	}
	enumerator->destroy(enumerator);
	return good;
}

METHOD(bridge_t, connect_iface, bool,
	private_bridge_t *this, iface_t *iface)
{
	if (br_add_interface(this->name, iface->get_hostif(iface)) != 0)
	{
		DBG1(DBG_LIB, "adding iface '%s' to bridge '%s' failed: %m",
			 iface->get_hostif(iface), this->name);
		return FALSE;
	}
	iface->set_bridge(iface, &this->public);
	this->ifaces->insert_last(this->ifaces, iface);
	return TRUE;
}

/**
 * instance counter to (de-)initialize libbridge
 */
static int instances = 0;

METHOD(bridge_t, destroy, void,
	private_bridge_t *this)
{
	enumerator_t *enumerator;
	iface_t *iface;

	enumerator = this->ifaces->create_enumerator(this->ifaces);
	while (enumerator->enumerate(enumerator, (void**)&iface))
	{
		if (br_del_interface(this->name, iface->get_hostif(iface)) != 0)
		{
			DBG1(DBG_LIB, "disconnecting iface '%s' failed: %m",
				 iface->get_hostif(iface));
		}
		iface->set_bridge(iface, NULL);
	}
	enumerator->destroy(enumerator);
	this->ifaces->destroy(this->ifaces);
	iface_control(this->name, FALSE);
	if (br_del_bridge(this->name) != 0)
	{
		DBG1(DBG_LIB, "deleting bridge '%s' from kernel failed: %m",
			 this->name);
	}
	free(this->name);
	free(this);
	if (--instances == 0)
	{
		br_shutdown();
	}
}

/**
 * create the bridge instance
 */
bridge_t *bridge_create(char *name)
{
	private_bridge_t *this;

	if (instances == 0)
	{
		if (br_init() != 0)
		{
			DBG1(DBG_LIB, "libbridge initialization failed: %m");
			return NULL;
		}
	}

	INIT(this,
		.public = {
			.get_name = _get_name,
			.create_iface_enumerator = _create_iface_enumerator,
			.disconnect_iface = _disconnect_iface,
			.connect_iface = _connect_iface,
			.destroy = _destroy,
		}
	);

	if (br_add_bridge(name) != 0)
	{
		DBG1(DBG_LIB, "creating bridge '%s' failed: %m", name);
		free(this);
		return NULL;
	}
	if (!iface_control(name, TRUE))
	{
		DBG1(DBG_LIB, "bringing bridge '%s' up failed: %m", name);
	}

	this->name = strdup(name);
	this->ifaces = linked_list_create();

	instances++;
	return &this->public;
}
