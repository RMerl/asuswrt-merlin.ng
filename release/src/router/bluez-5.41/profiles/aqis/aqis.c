/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Instituto Nokia de Tecnologia - INdT
 *  Copyright (C) 2014  Google Inc.
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
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <errno.h>
#include <string.h>
#include <stdbool.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "src/adapter.h"
#include "src/device.h"
#include "src/profile.h"
#include "src/plugin.h"
#include "attrib/gattrib.h"
#include "attrib/att.h"
#include "attrib/gatt.h"
#include "attrib/att-database.h"
#include "src/shared/util.h"
#include "src/attrib-server.h"
#include "src/log.h"

#include "src/shared/queue.h"
#include "src/shared/gatt-db.h"
#include "src/gatt-database.h"
#include "bleencrypt/gatt-amap.h"

static int aqis_gatt_register(struct btd_profile *p, struct btd_adapter *adapter)
{
	amap_gatt_service(adapter);

	return 0;
}

static void aqis_gatt_exit(struct btd_profile *p, struct btd_adapter *adapter)
{
	struct btd_gatt_database *database = btd_adapter_get_database(adapter);

	if (database->amap_handle)
		adapter_service_remove(database->adapter, database->amap_handle);
}

static struct btd_profile aqis_profile = {
	.name		= "AQIS GATT Profile",
	.adapter_probe	= aqis_gatt_register,
	.adapter_remove	= aqis_gatt_exit,
};

static int aqis_init(void)
{
	btd_profile_register(&aqis_profile);

	return 0;
}

static void aqis_exit(void)
{
	btd_profile_unregister(&aqis_profile);
}

BLUETOOTH_PLUGIN_DEFINE(aqis, VERSION, BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
							aqis_init, aqis_exit)
