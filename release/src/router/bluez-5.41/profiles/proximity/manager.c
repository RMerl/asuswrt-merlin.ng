/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011  Nokia Corporation
 *  Copyright (C) 2011  Marcel Holtmann <marcel@holtmann.org>
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

#include <stdbool.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "src/adapter.h"
#include "src/device.h"
#include "src/profile.h"
#include "src/service.h"
#include "attrib/att.h"
#include "attrib/gattrib.h"
#include "attrib/gatt.h"
#include "monitor.h"
#include "reporter.h"
#include "manager.h"

static struct enabled enabled  = {
	.linkloss = TRUE,
	.pathloss = TRUE,
	.findme = TRUE,
};

static int monitor_linkloss_probe(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct gatt_primary *linkloss;

	linkloss = btd_device_get_primary(device, LINK_LOSS_UUID);
	if (linkloss == NULL)
		return -1;

	return monitor_register_linkloss(device, &enabled, linkloss);
}

static int monitor_immediate_probe(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct gatt_primary *immediate;

	immediate = btd_device_get_primary(device, IMMEDIATE_ALERT_UUID);
	if (immediate == NULL)
		return -1;

	return monitor_register_immediate(device, &enabled, immediate);
}

static int monitor_txpower_probe(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct gatt_primary *txpower;

	txpower = btd_device_get_primary(device, TX_POWER_UUID);
	if (txpower == NULL)
		return -1;

	return monitor_register_txpower(device, &enabled, txpower);
}

static void monitor_linkloss_remove(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);

	monitor_unregister_linkloss(device);
}

static void monitor_immediate_remove(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);

	monitor_unregister_immediate(device);
}

static void monitor_txpower_remove(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);

	monitor_unregister_txpower(device);
}

static struct btd_profile pxp_monitor_linkloss_profile = {
	.name		= "proximity-linkloss",
	.remote_uuid	= LINK_LOSS_UUID,
	.device_probe	= monitor_linkloss_probe,
	.device_remove	= monitor_linkloss_remove,
};

static struct btd_profile pxp_monitor_immediate_profile = {
	.name		= "proximity-immediate",
	.remote_uuid	= IMMEDIATE_ALERT_UUID,
	.device_probe	= monitor_immediate_probe,
	.device_remove	= monitor_immediate_remove,
};

static struct btd_profile pxp_monitor_txpower_profile = {
	.name		= "proximity-txpower",
	.remote_uuid	= TX_POWER_UUID,
	.device_probe	= monitor_txpower_probe,
	.device_remove	= monitor_txpower_remove,
};

static struct btd_profile pxp_reporter_profile = {
	.name		= "Proximity Reporter GATT Driver",
	.remote_uuid	= GATT_UUID,
	.device_probe	= reporter_device_probe,
	.device_remove	= reporter_device_remove,

	.adapter_probe	= reporter_adapter_probe,
	.adapter_remove	= reporter_adapter_remove,
};

static void load_config_file(GKeyFile *config)
{
	char **list;
	int i;

	if (config == NULL)
		return;

	list = g_key_file_get_string_list(config, "General", "Disable",
								NULL, NULL);
	for (i = 0; list && list[i] != NULL; i++) {
		if (g_str_equal(list[i], "FindMe"))
			enabled.findme = FALSE;
		else if (g_str_equal(list[i], "LinkLoss"))
			enabled.linkloss = FALSE;
		else if (g_str_equal(list[i], "PathLoss"))
			enabled.pathloss = FALSE;
	}

	g_strfreev(list);
}

int proximity_manager_init(GKeyFile *config)
{
	load_config_file(config);

	if (btd_profile_register(&pxp_monitor_linkloss_profile) < 0)
		goto fail;

	if (btd_profile_register(&pxp_monitor_immediate_profile) < 0)
		goto fail;

	if (btd_profile_register(&pxp_monitor_txpower_profile) < 0)
		goto fail;

	if (btd_profile_register(&pxp_reporter_profile) < 0)
		goto fail;

	return 0;

fail:
	proximity_manager_exit();

	return -1;
}

void proximity_manager_exit(void)
{
	btd_profile_unregister(&pxp_reporter_profile);
	btd_profile_unregister(&pxp_monitor_txpower_profile);
	btd_profile_unregister(&pxp_monitor_immediate_profile);
	btd_profile_unregister(&pxp_monitor_linkloss_profile);
}
