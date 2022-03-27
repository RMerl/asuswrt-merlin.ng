/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Nordic Semiconductor Inc.
 *  Copyright (C) 2012  Instituto Nokia de Tecnologia - INdT
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
#include <errno.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "src/log.h"
#include "src/plugin.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/profile.h"
#include "src/service.h"
#include "src/shared/util.h"
#include "src/shared/att.h"
#include "src/shared/queue.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-client.h"
#include "attrib/att.h"

#define SCAN_INTERVAL_WIN_UUID		0x2A4F
#define SCAN_REFRESH_UUID		0x2A31

#define SCAN_INTERVAL		0x0060
#define SCAN_WINDOW		0x0030
#define SERVER_REQUIRES_REFRESH	0x00

struct scan {
	struct btd_device *device;
	struct gatt_db *db;
	struct bt_gatt_client *client;
	struct gatt_db_attribute *attr;
	uint16_t iwhandle;
	guint refresh_cb_id;
};

static GSList *devices;

static void scan_free(struct scan *scan)
{
	bt_gatt_client_unregister_notify(scan->client, scan->refresh_cb_id);
	gatt_db_unref(scan->db);
	bt_gatt_client_unref(scan->client);
	btd_device_unref(scan->device);
	g_free(scan);
}

static int cmp_device(gconstpointer a, gconstpointer b)
{
	const struct scan *scan = a;
	const struct btd_device *device = b;

	return scan->device == device ? 0 : -1;
}

static void write_scan_params(struct scan *scan)
{
	uint8_t value[4];

	put_le16(SCAN_INTERVAL, &value[0]);
	put_le16(SCAN_WINDOW, &value[2]);

	bt_gatt_client_write_without_response(scan->client, scan->iwhandle,
						false, value, sizeof(value));
}

static void refresh_value_cb(uint16_t value_handle, const uint8_t *value,
					uint16_t length, void *user_data)
{
	struct scan *scan = user_data;

	DBG("Server requires refresh: %d", value[3]);

	if (value[3] == SERVER_REQUIRES_REFRESH)
		write_scan_params(scan);
}

static void refresh_ccc_written_cb(uint16_t att_ecode, void *user_data)
{
	if (att_ecode != 0) {
		error("Scan Refresh: notifications not enabled %s",
						att_ecode2str(att_ecode));
		return;
	}

	DBG("Scan Refresh: notification enabled");
}

static void handle_refresh(struct scan *scan, uint16_t value_handle)
{
	DBG("Scan Refresh handle: 0x%04x", value_handle);

	scan->refresh_cb_id = bt_gatt_client_register_notify(scan->client,
					value_handle, refresh_ccc_written_cb,
						refresh_value_cb, scan,	NULL);
}

static void handle_iwin(struct scan *scan, uint16_t value_handle)
{
	scan->iwhandle = value_handle;

	DBG("Scan Interval Window handle: 0x%04x", scan->iwhandle);

	write_scan_params(scan);
}

static void handle_characteristic(struct gatt_db_attribute *attr,
								void *user_data)
{
	struct scan *scan = user_data;
	uint16_t value_handle;
	bt_uuid_t uuid, scan_interval_wind_uuid, scan_refresh_uuid;

	if (!gatt_db_attribute_get_char_data(attr, NULL, &value_handle, NULL,
								NULL, &uuid)) {
		error("Failed to obtain characteristic data");
		return;
	}

	bt_uuid16_create(&scan_interval_wind_uuid, SCAN_INTERVAL_WIN_UUID);
	bt_uuid16_create(&scan_refresh_uuid, SCAN_REFRESH_UUID);

	if (bt_uuid_cmp(&scan_interval_wind_uuid, &uuid) == 0)
		handle_iwin(scan, value_handle);
	else if (bt_uuid_cmp(&scan_refresh_uuid, &uuid) == 0)
		handle_refresh(scan, value_handle);
	else {
		char uuid_str[MAX_LEN_UUID_STR];

		bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));
		DBG("Unsupported characteristic: %s", uuid_str);
	}
}

static void foreach_scan_param_service(struct gatt_db_attribute *attr,
								void *user_data)
{
	struct scan *scan = user_data;

	if (scan->attr) {
		error("More than one scan params service exists for this "
								"device");
		return;
	}

	scan->attr = attr;
	gatt_db_service_foreach_char(scan->attr, handle_characteristic, scan);
}

static int scan_param_accept(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct gatt_db *db = btd_device_get_gatt_db(device);
	struct bt_gatt_client *client = btd_device_get_gatt_client(device);
	bt_uuid_t scan_parameters_uuid;
	struct scan *scan;
	GSList *l;
	char addr[18];

	ba2str(device_get_address(device), addr);
	DBG("Scan Parameters Client Driver profile accept (%s)", addr);

	l = g_slist_find_custom(devices, device, cmp_device);
	if (!l) {
		error("Scan Parameters service not handled by profile");
		return -1;
	}

	scan = l->data;

	/* Clean-up any old client/db and acquire the new ones */
	scan->attr = NULL;
	gatt_db_unref(scan->db);
	bt_gatt_client_unref(scan->client);


	scan->db = gatt_db_ref(db);
	scan->client = bt_gatt_client_ref(client);

	bt_string_to_uuid(&scan_parameters_uuid, SCAN_PARAMETERS_UUID);
	gatt_db_foreach_service(db, &scan_parameters_uuid,
					foreach_scan_param_service, scan);

	return 0;
}

static void scan_param_remove(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct scan *scan;
	GSList *l;
	char addr[18];

	ba2str(device_get_address(device), addr);
	DBG("GAP profile remove (%s)", addr);

	l = g_slist_find_custom(devices, device, cmp_device);
	if (!l) {
		error("GAP service not handled by profile");
		return;
	}

	scan = l->data;

	devices = g_slist_remove(devices, scan);
	scan_free(scan);
}

static int scan_param_probe(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct scan *scan;
	GSList *l;
	char addr[18];

	ba2str(device_get_address(device), addr);
	DBG("Scan Parameters Client Driver profile probe (%s)", addr);

	/* Ignore, if we were probed for this device already */
	l = g_slist_find_custom(devices, device, cmp_device);
	if (l) {
		error("Profile probed twice for the same device!");
		return -1;
	}

	scan = g_new0(struct scan, 1);
	if (!scan)
		return -1;

	scan->device = btd_device_ref(device);
	devices = g_slist_append(devices, scan);
	return 0;
}

static struct btd_profile scan_profile = {
	.name = "Scan Parameters Client Driver",
	.remote_uuid = SCAN_PARAMETERS_UUID,
	.device_probe = scan_param_probe,
	.device_remove = scan_param_remove,
	.accept = scan_param_accept,
};

static int scan_param_init(void)
{
	return btd_profile_register(&scan_profile);
}

static void scan_param_exit(void)
{
	btd_profile_unregister(&scan_profile);
}

BLUETOOTH_PLUGIN_DEFINE(scanparam, VERSION,
			BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
			scan_param_init, scan_param_exit)
