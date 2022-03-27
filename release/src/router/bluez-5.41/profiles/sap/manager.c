/*
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010 Instituto Nokia de Tecnologia - INdT
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"

#include "src/log.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/profile.h"
#include "src/service.h"

#include "manager.h"
#include "server.h"

static int sap_server_probe(struct btd_profile *p, struct btd_adapter *adapter)
{
	DBG("path %s", adapter_get_path(adapter));

	return sap_server_register(adapter);
}

static void sap_server_remove(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	const char *path = adapter_get_path(adapter);

	DBG("path %s", path);

	sap_server_unregister(path);
}

static struct btd_profile sap_profile = {
	.name		= "sap-server",
	.adapter_probe	= sap_server_probe,
	.adapter_remove	= sap_server_remove,
};

int sap_manager_init(void)
{
	btd_profile_register(&sap_profile);

	return 0;
}

void sap_manager_exit(void)
{
	btd_profile_unregister(&sap_profile);
}
