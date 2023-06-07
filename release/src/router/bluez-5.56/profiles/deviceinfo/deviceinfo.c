// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012 Texas Instruments, Inc.
 *  Copyright (C) 2015 Google Inc.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>
#include <errno.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "src/plugin.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/profile.h"
#include "src/service.h"
#include "attrib/gattrib.h"
#include "src/shared/util.h"
#include "src/shared/queue.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-client.h"
#include "attrib/att.h"
#include "src/log.h"

#define PNP_ID_SIZE	7

static void read_pnpid_cb(bool success, uint8_t att_ecode, const uint8_t *value,
					uint16_t length, void *user_data)
{
	struct btd_device *device = user_data;

	if (!success) {
		error("Error reading PNP_ID value: %s",
						att_ecode2str(att_ecode));
		return;
	}

	if (length < PNP_ID_SIZE) {
		error("Error reading PNP_ID: Invalid pdu length received");
		return;
	}

	btd_device_set_pnpid(device, value[0], get_le16(&value[1]),
				get_le16(&value[3]), get_le16(&value[5]));
}

static void handle_pnpid(struct btd_device *device, uint16_t value_handle)
{
	struct bt_gatt_client *client = btd_device_get_gatt_client(device);

	if (!bt_gatt_client_read_value(client, value_handle,
						read_pnpid_cb, device, NULL))
		DBG("Failed to send request to read pnpid");
}

static void handle_characteristic(struct gatt_db_attribute *attr,
								void *user_data)
{
	struct btd_device *device = user_data;
	uint16_t value_handle;
	bt_uuid_t uuid, pnpid_uuid;

	bt_string_to_uuid(&pnpid_uuid, PNPID_UUID);

	if (!gatt_db_attribute_get_char_data(attr, NULL, &value_handle, NULL,
								NULL, &uuid)) {
		error("Failed to obtain characteristic data");
		return;
	}

	if (bt_uuid_cmp(&pnpid_uuid, &uuid) == 0)
		handle_pnpid(device, value_handle);
	else {
		char uuid_str[MAX_LEN_UUID_STR];

		bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));
		DBG("Unsupported characteristic: %s", uuid_str);
	}
}

static void foreach_deviceinfo_service(struct gatt_db_attribute *attr,
								void *user_data)
{
	struct btd_device *device = user_data;

	gatt_db_service_foreach_char(attr, handle_characteristic, device);
}

static int deviceinfo_probe(struct btd_service *service)
{
	return 0;
}

static void deviceinfo_remove(struct btd_service *service)
{
}


static int deviceinfo_accept(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct gatt_db *db = btd_device_get_gatt_db(device);
	char addr[18];
	bt_uuid_t deviceinfo_uuid;

	ba2str(device_get_address(device), addr);
	DBG("deviceinfo profile accept (%s)", addr);

	/* Handle the device info service */
	bt_string_to_uuid(&deviceinfo_uuid, DEVICE_INFORMATION_UUID);
	gatt_db_foreach_service(db, &deviceinfo_uuid,
					foreach_deviceinfo_service, device);

	btd_service_connecting_complete(service, 0);

	return 0;
}

static int deviceinfo_disconnect(struct btd_service *service)
{
	btd_service_disconnecting_complete(service, 0);

	return 0;
}

static struct btd_profile deviceinfo_profile = {
	.name		= "deviceinfo",
	.remote_uuid	= DEVICE_INFORMATION_UUID,
	.external	= true,
	.device_probe	= deviceinfo_probe,
	.device_remove	= deviceinfo_remove,
	.accept		= deviceinfo_accept,
	.disconnect	= deviceinfo_disconnect,
};

static int deviceinfo_init(void)
{
	return btd_profile_register(&deviceinfo_profile);
}

static void deviceinfo_exit(void)
{
	btd_profile_unregister(&deviceinfo_profile);
}

BLUETOOTH_PLUGIN_DEFINE(deviceinfo, VERSION, BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
					deviceinfo_init, deviceinfo_exit)
