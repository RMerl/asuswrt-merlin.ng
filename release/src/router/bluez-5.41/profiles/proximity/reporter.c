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
#include <errno.h>

#include <glib.h>

#include <dbus/dbus.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "src/log.h"
#include "src/dbus-common.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/profile.h"
#include "src/service.h"
#include "src/shared/util.h"
#include "attrib/gattrib.h"
#include "attrib/att.h"
#include "attrib/gatt.h"
#include "attrib/att-database.h"
#include "src/attrib-server.h"

#include "reporter.h"
#include "linkloss.h"
#include "immalert.h"

struct reporter_adapter {
	struct btd_adapter *adapter;
	GSList *devices;
};

static GSList *reporter_adapters;

static int radapter_cmp(gconstpointer a, gconstpointer b)
{
	const struct reporter_adapter *radapter = a;
	const struct btd_adapter *adapter = b;

	if (radapter->adapter == adapter)
		return 0;

	return -1;
}

static struct reporter_adapter *
find_reporter_adapter(struct btd_adapter *adapter)
{
	GSList *l = g_slist_find_custom(reporter_adapters, adapter,
								radapter_cmp);
	if (!l)
		return NULL;

	return l->data;
}

const char *get_alert_level_string(uint8_t level)
{
	switch (level) {
	case NO_ALERT:
		return "none";
	case MILD_ALERT:
		return "mild";
	case HIGH_ALERT:
		return "high";
	}

	return "unknown";
}

static void register_tx_power(struct btd_adapter *adapter)
{
	uint16_t start_handle, h;
	const int svc_size = 4;
	uint8_t atval[256];
	bt_uuid_t uuid;

	bt_uuid16_create(&uuid, TX_POWER_SVC_UUID);
	start_handle = attrib_db_find_avail(adapter, &uuid, svc_size);
	if (start_handle == 0) {
		error("Not enough free handles to register service");
		return;
	}

	DBG("start_handle=0x%04x", start_handle);

	h = start_handle;

	/* Primary service definition */
	bt_uuid16_create(&uuid, GATT_PRIM_SVC_UUID);
	put_le16(TX_POWER_SVC_UUID, &atval[0]);
	attrib_db_add(adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED, atval, 2);

	/* Power level characteristic */
	bt_uuid16_create(&uuid, GATT_CHARAC_UUID);
	atval[0] = GATT_CHR_PROP_READ | GATT_CHR_PROP_NOTIFY;
	put_le16(h + 1, &atval[1]);
	put_le16(POWER_LEVEL_CHR_UUID, &atval[3]);
	attrib_db_add(adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED, atval, 5);

	/* Power level value */
	bt_uuid16_create(&uuid, POWER_LEVEL_CHR_UUID);
	atval[0] = 0x00;
	attrib_db_add(adapter, h++, &uuid, ATT_NONE, ATT_NOT_PERMITTED, atval, 1);

	/* Client characteristic configuration */
	bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);
	atval[0] = 0x00;
	atval[1] = 0x00;
	attrib_db_add(adapter, h++, &uuid, ATT_NONE, ATT_NONE, atval, 2);

	g_assert(h - start_handle == svc_size);
}

static gboolean property_get_link_loss_level(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *device = data;
	const char *level;

	level = link_loss_get_alert_level(device);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &level);

	return TRUE;
}

static gboolean property_get_immediate_alert_level(
					const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *device = data;
	const char *level;

	level = imm_alert_get_level(device);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &level);

	return TRUE;
}

static const GDBusPropertyTable reporter_device_properties[] = {
	{ "LinkLossAlertLevel", "s", property_get_link_loss_level },
	{ "ImmediateAlertLevel", "s", property_get_immediate_alert_level },
	{ }
};

static void unregister_reporter_device(gpointer data, gpointer user_data)
{
	struct btd_device *device = data;
	struct reporter_adapter *radapter = user_data;
	const char *path = device_get_path(device);

	DBG("unregister on device %s", path);

	g_dbus_unregister_interface(btd_get_dbus_connection(), path,
					PROXIMITY_REPORTER_INTERFACE);

	radapter->devices = g_slist_remove(radapter->devices, device);
	btd_device_unref(device);
}

static void register_reporter_device(struct btd_device *device,
					struct reporter_adapter *radapter)
{
	const char *path = device_get_path(device);

	DBG("register on device %s", path);

	g_dbus_register_interface(btd_get_dbus_connection(), path,
					PROXIMITY_REPORTER_INTERFACE,
					NULL, NULL, reporter_device_properties,
					device, NULL);

	btd_device_ref(device);
	radapter->devices = g_slist_prepend(radapter->devices, device);
}

int reporter_device_probe(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct reporter_adapter *radapter;
	struct btd_adapter *adapter = device_get_adapter(device);

	radapter = find_reporter_adapter(adapter);
	if (!radapter)
		return -1;

	register_reporter_device(device, radapter);

	return 0;
}

void reporter_device_remove(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct reporter_adapter *radapter;
	struct btd_adapter *adapter = device_get_adapter(device);

	radapter = find_reporter_adapter(adapter);
	if (!radapter)
		return;

	unregister_reporter_device(device, radapter);
}

int reporter_adapter_probe(struct btd_profile *p, struct btd_adapter *adapter)
{
	struct reporter_adapter *radapter;

	radapter = g_new0(struct reporter_adapter, 1);
	radapter->adapter = adapter;

	link_loss_register(adapter);
	register_tx_power(adapter);
	imm_alert_register(adapter);

	reporter_adapters = g_slist_prepend(reporter_adapters, radapter);
	DBG("Proximity Reporter for adapter %p", adapter);

	return 0;
}

void reporter_adapter_remove(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	struct reporter_adapter *radapter = find_reporter_adapter(adapter);
	if (!radapter)
		return;

	g_slist_foreach(radapter->devices, unregister_reporter_device,
								radapter);

	link_loss_unregister(adapter);
	imm_alert_unregister(adapter);

	reporter_adapters = g_slist_remove(reporter_adapters, radapter);
	g_free(radapter);
}
