/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include <glib.h>

#include "obex.h"
#include "server.h"
#include "transport.h"
#include "log.h"

static GSList *drivers = NULL;

static struct obex_transport_driver *obex_transport_driver_find(
							const char *name)
{
	GSList *l;

	for (l = drivers; l; l = l->next) {
		struct obex_transport_driver *driver = l->data;

		if (g_strcmp0(name, driver->name) == 0)
			return driver;
	}

	return NULL;
}

GSList *obex_transport_driver_list(void)
{
	return drivers;
}

int obex_transport_driver_register(struct obex_transport_driver *driver)
{
	if (!driver) {
		error("Invalid driver");
		return -EINVAL;
	}

	if (obex_transport_driver_find(driver->name) != NULL) {
		error("Permission denied: transport %s already registered",
			driver->name);
		return -EPERM;
	}

	DBG("driver %p transport %s registered", driver, driver->name);

	drivers = g_slist_prepend(drivers, driver);

	return 0;
}

void obex_transport_driver_unregister(struct obex_transport_driver *driver)
{
	if (!g_slist_find(drivers, driver)) {
		error("Unable to unregister: No such driver %p", driver);
		return;
	}

	DBG("driver %p transport %s unregistered", driver, driver->name);

	drivers = g_slist_remove(drivers, driver);
}
