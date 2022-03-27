/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012 Texas Instruments Corporation
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

#include <dbus/dbus.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "src/log.h"
#include "src/adapter.h"
#include "src/device.h"
#include "attrib/att-database.h"
#include "attrib/gattrib.h"
#include "attrib/att.h"
#include "attrib/gatt.h"
#include "attrib/gatt-service.h"
#include "src/attrib-server.h"
#include "src/service.h"
#include "src/profile.h"
#include "src/attio.h"
#include "src/dbus-common.h"

#include "reporter.h"
#include "linkloss.h"

struct link_loss_adapter {
	struct btd_adapter *adapter;
	uint16_t alert_lvl_value_handle;
	GSList *connected_devices;
};

struct connected_device {
	struct btd_device *device;
	struct link_loss_adapter *adapter;
	uint8_t alert_level;
	guint callback_id;
	guint local_disc_id;
};

static GSList *link_loss_adapters;

static int lldevice_cmp(gconstpointer a, gconstpointer b)
{
	const struct connected_device *llcondev = a;
	const struct btd_device *device = b;

	if (llcondev->device == device)
		return 0;

	return -1;
}

static struct connected_device *
find_connected_device(struct link_loss_adapter *la, struct btd_device *device)
{
	GSList *l = g_slist_find_custom(la->connected_devices, device,
								lldevice_cmp);
	if (!l)
		return NULL;

	return l->data;
}

static int lladapter_cmp(gconstpointer a, gconstpointer b)
{
	const struct link_loss_adapter *lladapter = a;
	const struct btd_adapter *adapter = b;

	if (lladapter->adapter == adapter)
		return 0;

	return -1;
}

static struct link_loss_adapter *
find_link_loss_adapter(struct btd_adapter *adapter)
{
	GSList *l = g_slist_find_custom(link_loss_adapters, adapter,
							lladapter_cmp);
	if (!l)
		return NULL;

	return l->data;
}

const char *link_loss_get_alert_level(struct btd_device *device)
{
	struct link_loss_adapter *lladapter;
	struct connected_device *condev;

	if (!device)
		return get_alert_level_string(NO_ALERT);

	lladapter = find_link_loss_adapter(device_get_adapter(device));
	if (!lladapter)
		return get_alert_level_string(NO_ALERT);

	condev = find_connected_device(lladapter, device);
	if (!condev)
		return get_alert_level_string(NO_ALERT);

	return get_alert_level_string(condev->alert_level);
}

static void link_loss_emit_alert_signal(struct connected_device *condev)
{
	const char *alert_level_str, *path;

	if (!condev->device)
		return;

	path = device_get_path(condev->device);
	alert_level_str = get_alert_level_string(condev->alert_level);

	DBG("alert %s remote %s", alert_level_str, path);

	g_dbus_emit_property_changed(btd_get_dbus_connection(), path,
			PROXIMITY_REPORTER_INTERFACE, "LinkLossAlertLevel");
}

static uint8_t link_loss_alert_lvl_read(struct attribute *a,
				struct btd_device *device, gpointer user_data)
{
	struct link_loss_adapter *la = user_data;
	struct connected_device *condev;
	uint8_t alert_level = NO_ALERT;

	if (!device)
		goto out;

	condev = find_connected_device(la, device);
	if (!condev)
		goto out;

	alert_level = condev->alert_level;

out:
	DBG("return alert level %d for dev %p", alert_level, device);

	/* update the alert level according to the requesting device */
	attrib_db_update(la->adapter, a->handle, NULL, &alert_level,
						sizeof(alert_level), NULL);

	return 0;
}

/* condev can be NULL */
static void link_loss_remove_condev(struct connected_device *condev)
{
	struct link_loss_adapter *la;

	if (!condev)
		return;

	la = condev->adapter;

	if (condev->callback_id && condev->device)
		btd_device_remove_attio_callback(condev->device,
							condev->callback_id);

	if (condev->local_disc_id && condev->device)
		device_remove_disconnect_watch(condev->device,
							condev->local_disc_id);

	if (condev->device)
		btd_device_unref(condev->device);

	la->connected_devices = g_slist_remove(la->connected_devices, condev);
	g_free(condev);
}

static void link_loss_disc_cb(gpointer user_data)
{
	struct connected_device *condev = user_data;

	DBG("alert loss disconnect device %p", condev->device);

	/* if an alert-level is set, emit a signal */
	if (condev->alert_level != NO_ALERT)
		link_loss_emit_alert_signal(condev);

	/* we are open for more changes now */
	link_loss_remove_condev(condev);
}

static void link_loss_local_disc(struct btd_device *device,
					gboolean removal, void *user_data)
{
	struct connected_device *condev = user_data;

	/* no need to alert on this device - we requested disconnection */
	link_loss_remove_condev(condev);

	DBG("alert level zeroed for locally disconnecting dev %p", device);
}

static uint8_t link_loss_alert_lvl_write(struct attribute *a,
				struct btd_device *device, gpointer user_data)
{
	uint8_t value;
	struct link_loss_adapter *la = user_data;
	struct connected_device *condev = NULL;

	if (!device)
		goto set_error;

	/* condev might remain NULL here if nothing is found */
	condev = find_connected_device(la, device);

	if (a->len == 0) {
		DBG("Illegal alert level length");
		goto set_error;
	}

	value = a->data[0];
	if (value != NO_ALERT && value != MILD_ALERT && value != HIGH_ALERT) {
		DBG("Illegal alert value");
		goto set_error;
	}

	/* Register a disconnect cb if the alert level is non-zero */
	if (value != NO_ALERT && !condev) {
		condev = g_new0(struct connected_device, 1);
		condev->device = btd_device_ref(device);
		condev->adapter = la;
		condev->callback_id = btd_device_add_attio_callback(device,
					NULL, link_loss_disc_cb, condev);
		condev->local_disc_id = device_add_disconnect_watch(device,
					link_loss_local_disc, condev, NULL);

		la->connected_devices = g_slist_append(la->connected_devices,
								condev);
	} else if (value == NO_ALERT && condev) {
		link_loss_remove_condev(condev);
		condev = NULL;
	}

	DBG("alert level set to %d by device %p", value, device);

	if (condev)
		condev->alert_level = value;

	return 0;

set_error:
	error("Set link loss alert level for dev %p", device);
	/* reset alert level on erroneous devices */
	link_loss_remove_condev(condev);
	return ATT_ECODE_IO;
}

void link_loss_register(struct btd_adapter *adapter)
{
	gboolean svc_added;
	bt_uuid_t uuid;
	struct link_loss_adapter *lladapter;

	bt_uuid16_create(&uuid, LINK_LOSS_SVC_UUID);

	lladapter = g_new0(struct link_loss_adapter, 1);
	lladapter->adapter = adapter;

	link_loss_adapters = g_slist_append(link_loss_adapters, lladapter);

	/* Link Loss Service */
	svc_added = gatt_service_add(adapter,
			GATT_PRIM_SVC_UUID, &uuid,
			/* Alert level characteristic */
			GATT_OPT_CHR_UUID16, ALERT_LEVEL_CHR_UUID,
			GATT_OPT_CHR_PROPS,
				GATT_CHR_PROP_READ | GATT_CHR_PROP_WRITE,
			GATT_OPT_CHR_VALUE_CB, ATTRIB_READ,
				link_loss_alert_lvl_read, lladapter,
			GATT_OPT_CHR_VALUE_CB, ATTRIB_WRITE,
				link_loss_alert_lvl_write, lladapter,
			GATT_OPT_CHR_VALUE_GET_HANDLE,
				&lladapter->alert_lvl_value_handle,
			GATT_OPT_INVALID);

	if (!svc_added)
		goto err;

	DBG("Link Loss service added");
	return;

err:
	error("Error adding Link Loss service");
	link_loss_unregister(adapter);
}

static void remove_condev_list_item(gpointer data, gpointer user_data)
{
	struct connected_device *condev = data;

	link_loss_remove_condev(condev);
}

void link_loss_unregister(struct btd_adapter *adapter)
{
	struct link_loss_adapter *lladapter;
	lladapter = find_link_loss_adapter(adapter);
	if (!lladapter)
		return;

	g_slist_foreach(lladapter->connected_devices, remove_condev_list_item,
			NULL);

	link_loss_adapters = g_slist_remove(link_loss_adapters, lladapter);
	g_free(lladapter);
}
