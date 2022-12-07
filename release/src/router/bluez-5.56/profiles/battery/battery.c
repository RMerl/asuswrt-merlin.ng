// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Instituto Nokia de Tecnologia - INdT
 *  Copyright (C) 2014  Google Inc.
 *  Copyright (C) 2017  Red Hat Inc.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "src/shared/util.h"
#include "src/shared/att.h"
#include "src/shared/queue.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-client.h"
#include "src/plugin.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/profile.h"
#include "src/service.h"
#include "src/log.h"
#include "src/battery.h"
#include "attrib/att.h"

#define BATTERY_INTERFACE "org.bluez.Battery1"

#define BATT_UUID16 0x180f

/* Generic Attribute/Access Service */
struct batt {
	struct btd_battery *battery;
	struct btd_device *device;
	struct gatt_db *db;
	struct bt_gatt_client *client;
	struct gatt_db_attribute *attr;

	unsigned int batt_level_cb_id;
	uint16_t batt_level_io_handle;

	uint8_t *initial_value;
	uint8_t percentage;
};

static void batt_free(struct batt *batt)
{
	gatt_db_unref(batt->db);
	bt_gatt_client_unref(batt->client);
	btd_device_unref(batt->device);
	g_free (batt->initial_value);
	if (batt->battery)
		btd_battery_unregister(batt->battery);
	g_free(batt);
}

static void batt_reset(struct batt *batt)
{
	batt->attr = NULL;
	gatt_db_unref(batt->db);
	batt->db = NULL;
	bt_gatt_client_unref(batt->client);
	batt->client = NULL;
	g_free (batt->initial_value);
	batt->initial_value = NULL;
	if (batt->battery) {
		btd_battery_unregister(batt->battery);
		batt->battery = NULL;
	}
}

static void parse_battery_level(struct batt *batt,
				const uint8_t *value)
{
	uint8_t percentage;

	percentage = value[0];
	if (batt->percentage != percentage) {
		batt->percentage = percentage;
		DBG("Battery Level updated: %d%%", percentage);
		if (!batt->battery) {
			warn("Trying to update an unregistered battery");
			return;
		}
		btd_battery_update(batt->battery, batt->percentage);
	}
}

static void batt_io_value_cb(uint16_t value_handle, const uint8_t *value,
                             uint16_t length, void *user_data)
{
	struct batt *batt = user_data;

	if (value_handle == batt->batt_level_io_handle) {
		parse_battery_level(batt, value);
	} else {
		g_assert_not_reached();
	}
}

static void batt_io_ccc_written_cb(uint16_t att_ecode, void *user_data)
{
	struct batt *batt = user_data;

	if (att_ecode != 0) {
		error("Battery Level: notifications not enabled %s",
		      att_ecode2str(att_ecode));
		return;
	}

	batt->battery = btd_battery_register(device_get_path(batt->device),
					     "GATT Battery Service", NULL);

	if (!batt->battery) {
		batt_reset(batt);
		return;
	}

	parse_battery_level(batt, batt->initial_value);
	g_free (batt->initial_value);
	batt->initial_value = NULL;

	DBG("Battery Level: notification enabled");
}

static void read_initial_battery_level_cb(bool success,
						uint8_t att_ecode,
						const uint8_t *value,
						uint16_t length,
						void *user_data)
{
	struct batt *batt = user_data;

	if (!success) {
		DBG("Reading battery level failed with ATT errror: %u",
								att_ecode);
		return;
	}

	if (!length)
		return;

	batt->initial_value = g_memdup(value, length);

	/* request notify */
	batt->batt_level_cb_id =
		bt_gatt_client_register_notify(batt->client,
		                               batt->batt_level_io_handle,
		                               batt_io_ccc_written_cb,
		                               batt_io_value_cb,
		                               batt,
		                               NULL);
}

static void handle_battery_level(struct batt *batt, uint16_t value_handle)
{
	batt->batt_level_io_handle = value_handle;

	if (!bt_gatt_client_read_value(batt->client, batt->batt_level_io_handle,
						read_initial_battery_level_cb, batt, NULL))
		DBG("Failed to send request to read battery level");
}

static bool uuid_cmp(uint16_t u16, const bt_uuid_t *uuid)
{
	bt_uuid_t lhs;

	bt_uuid16_create(&lhs, u16);

	return bt_uuid_cmp(&lhs, uuid) == 0;
}

static void handle_characteristic(struct gatt_db_attribute *attr,
								void *user_data)
{
	struct batt *batt = user_data;
	uint16_t value_handle;
	bt_uuid_t uuid;

	if (!gatt_db_attribute_get_char_data(attr, NULL, &value_handle, NULL,
								NULL, &uuid)) {
		error("Failed to obtain characteristic data");
		return;
	}

	if (uuid_cmp(GATT_CHARAC_BATTERY_LEVEL, &uuid)) {
		handle_battery_level(batt, value_handle);
	} else {
		char uuid_str[MAX_LEN_UUID_STR];

		bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));
		DBG("Unsupported characteristic: %s", uuid_str);
	}
}

static void handle_batt_service(struct batt *batt)
{
	gatt_db_service_foreach_char(batt->attr, handle_characteristic, batt);
}

static int batt_probe(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct batt *batt = btd_service_get_user_data(service);
	char addr[18];

	ba2str(device_get_address(device), addr);
	DBG("BATT profile probe (%s)", addr);

	/* Ignore, if we were probed for this device already */
	if (batt) {
		error("Profile probed twice for the same device!");
		return -1;
	}

	batt = g_new0(struct batt, 1);
	if (!batt)
		return -1;

	batt->percentage = -1;
	batt->device = btd_device_ref(device);
	btd_service_set_user_data(service, batt);

	return 0;
}

static void batt_remove(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct batt *batt;
	char addr[18];

	ba2str(device_get_address(device), addr);
	DBG("BATT profile remove (%s)", addr);

	batt = btd_service_get_user_data(service);
	if (!batt) {
		error("BATT service not handled by profile");
		return;
	}

	batt_free(batt);
}

static void foreach_batt_service(struct gatt_db_attribute *attr, void *user_data)
{
	struct batt *batt = user_data;

	if (batt->attr) {
		error("More than one BATT service exists for this device");
		return;
	}

	batt->attr = attr;
	handle_batt_service(batt);
}

static int batt_accept(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct gatt_db *db = btd_device_get_gatt_db(device);
	struct bt_gatt_client *client = btd_device_get_gatt_client(device);
	struct batt *batt = btd_service_get_user_data(service);
	char addr[18];
	bt_uuid_t batt_uuid;

	ba2str(device_get_address(device), addr);
	DBG("BATT profile accept (%s)", addr);

	if (!batt) {
		error("BATT service not handled by profile");
		return -1;
	}

	batt->db = gatt_db_ref(db);
	batt->client = bt_gatt_client_clone(client);

	/* Handle the BATT services */
	bt_uuid16_create(&batt_uuid, BATT_UUID16);
	gatt_db_foreach_service(db, &batt_uuid, foreach_batt_service, batt);

	if (!batt->attr) {
		error("BATT attribute not found");
		batt_reset(batt);
		return -1;
	}

	btd_service_connecting_complete(service, 0);

	return 0;
}

static int batt_disconnect(struct btd_service *service)
{
	struct batt *batt = btd_service_get_user_data(service);

	batt_reset(batt);

	btd_service_disconnecting_complete(service, 0);

	return 0;
}

static struct btd_profile batt_profile = {
	.name		= "batt-profile",
	.remote_uuid	= BATTERY_UUID,
	.device_probe	= batt_probe,
	.device_remove	= batt_remove,
	.accept		= batt_accept,
	.disconnect	= batt_disconnect,
	.external	= true,
};

static int batt_init(void)
{
	return btd_profile_register(&batt_profile);
}

static void batt_exit(void)
{
	btd_profile_unregister(&batt_profile);
}

BLUETOOTH_PLUGIN_DEFINE(battery, VERSION, BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
							batt_init, batt_exit)
