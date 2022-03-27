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

#include <string.h>
#include <errno.h>
#include <glib.h>

#include "gdbus/gdbus.h"

#include "obexd/src/log.h"
#include "transfer.h"
#include "session.h"
#include "driver.h"

static GSList *drivers = NULL;

struct obc_driver *obc_driver_find(const char *pattern)
{
	GSList *l;

	for (l = drivers; l; l = l->next) {
		struct obc_driver *driver = l->data;

		if (strcasecmp(pattern, driver->service) == 0)
			return driver;

		if (strcasecmp(pattern, driver->uuid) == 0)
			return driver;
	}

	return NULL;
}

int obc_driver_register(struct obc_driver *driver)
{
	if (!driver) {
		error("Invalid driver");
		return -EINVAL;
	}

	if (obc_driver_find(driver->service)) {
		error("Permission denied: service %s already registered",
			driver->service);
		return -EPERM;
	}

	DBG("driver %p service %s registered", driver, driver->service);

	drivers = g_slist_append(drivers, driver);

	return 0;
}

void obc_driver_unregister(struct obc_driver *driver)
{
	if (!g_slist_find(drivers, driver)) {
		error("Unable to unregister: No such driver %p", driver);
		return;
	}

	DBG("driver %p service %s unregistered", driver, driver->service);

	drivers = g_slist_remove(drivers, driver);
}
