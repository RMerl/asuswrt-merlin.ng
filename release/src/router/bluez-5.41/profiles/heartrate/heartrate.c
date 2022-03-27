/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012 Tieto Poland
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

#include <errno.h>
#include <stdbool.h>
#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "src/plugin.h"
#include "src/adapter.h"
#include "src/dbus-common.h"
#include "src/device.h"
#include "src/profile.h"
#include "src/shared/util.h"
#include "src/service.h"
#include "src/error.h"
#include "attrib/gattrib.h"
#include "attrib/att.h"
#include "attrib/gatt.h"
#include "src/attio.h"
#include "src/log.h"

#define HEART_RATE_INTERFACE		"org.bluez.HeartRate1"
#define HEART_RATE_MANAGER_INTERFACE	"org.bluez.HeartRateManager1"
#define HEART_RATE_WATCHER_INTERFACE	"org.bluez.HeartRateWatcher1"

#define HR_VALUE_FORMAT		0x01
#define SENSOR_CONTACT_DETECTED	0x02
#define SENSOR_CONTACT_SUPPORT	0x04
#define ENERGY_EXP_STATUS	0x08
#define RR_INTERVAL		0x10

struct heartrate_adapter {
	struct btd_adapter	*adapter;
	GSList			*devices;
	GSList			*watchers;
};

struct heartrate {
	struct btd_device		*dev;
	struct heartrate_adapter	*hradapter;
	GAttrib				*attrib;
	guint				attioid;
	guint				attionotid;

	struct att_range		*svc_range;	/* primary svc range */

	uint16_t			measurement_ccc_handle;
	uint16_t			hrcp_val_handle;

	gboolean			has_location;
	uint8_t				location;
};

struct watcher {
	struct heartrate_adapter	*hradapter;
	guint				id;
	char				*srv;
	char				*path;
};

struct measurement {
	struct heartrate	*hr;
	uint16_t		value;
	gboolean		has_energy;
	uint16_t		energy;
	gboolean		has_contact;
	gboolean		contact;
	uint16_t		num_interval;
	uint16_t		*interval;
};

static GSList *heartrate_adapters = NULL;

static const char * const location_enum[] = {
	"other",
	"chest",
	"wrist",
	"finger",
	"hand",
	"earlobe",
	"foot",
};

static const char *location2str(uint8_t value)
{
	 if (value < G_N_ELEMENTS(location_enum))
		return location_enum[value];

	error("Body Sensor Location [%d] is RFU", value);

	return NULL;
}

static int cmp_adapter(gconstpointer a, gconstpointer b)
{
	const struct heartrate_adapter *hradapter = a;
	const struct btd_adapter *adapter = b;

	if (adapter == hradapter->adapter)
		return 0;

	return -1;
}

static int cmp_device(gconstpointer a, gconstpointer b)
{
	const struct heartrate *hr = a;
	const struct btd_device *dev = b;

	if (dev == hr->dev)
		return 0;

	return -1;
}

static int cmp_watcher(gconstpointer a, gconstpointer b)
{
	const struct watcher *watcher = a;
	const struct watcher *match = b;
	int ret;

	ret = g_strcmp0(watcher->srv, match->srv);
	if (ret != 0)
		return ret;

	return g_strcmp0(watcher->path, match->path);
}

static struct heartrate_adapter *
find_heartrate_adapter(struct btd_adapter *adapter)
{
	GSList *l = g_slist_find_custom(heartrate_adapters, adapter,
								cmp_adapter);
	if (!l)
		return NULL;

	return l->data;
}

static void destroy_watcher(gpointer user_data)
{
	struct watcher *watcher = user_data;

	g_free(watcher->path);
	g_free(watcher->srv);
	g_free(watcher);
}

static struct watcher *find_watcher(GSList *list, const char *sender,
							const char *path)
{
	struct watcher *match;
	GSList *l;

	match = g_new0(struct watcher, 1);
	match->srv = g_strdup(sender);
	match->path = g_strdup(path);

	l = g_slist_find_custom(list, match, cmp_watcher);
	destroy_watcher(match);

	if (l != NULL)
		return l->data;

	return NULL;
}

static void destroy_heartrate(gpointer user_data)
{
	struct heartrate *hr = user_data;

	if (hr->attioid > 0)
		btd_device_remove_attio_callback(hr->dev, hr->attioid);

	if (hr->attrib != NULL) {
		g_attrib_unregister(hr->attrib, hr->attionotid);
		g_attrib_unref(hr->attrib);
	}

	btd_device_unref(hr->dev);
	g_free(hr->svc_range);
	g_free(hr);
}

static void remove_watcher(gpointer user_data)
{
	struct watcher *watcher = user_data;

	g_dbus_remove_watch(btd_get_dbus_connection(), watcher->id);
}

static void destroy_heartrate_adapter(gpointer user_data)
{
	struct heartrate_adapter *hradapter = user_data;

	g_slist_free_full(hradapter->watchers, remove_watcher);

	g_free(hradapter);
}

static void read_sensor_location_cb(guint8 status, const guint8 *pdu,
						guint16 len, gpointer user_data)
{
	struct heartrate *hr = user_data;
	uint8_t value;
	ssize_t vlen;

	if (status != 0) {
		error("Body Sensor Location read failed: %s",
							att_ecode2str(status));
		return;
	}

	vlen = dec_read_resp(pdu, len, &value, sizeof(value));
	if (vlen < 0) {
		error("Protocol error");
		return;
	}

	if (vlen != sizeof(value)) {
		error("Invalid length for Body Sensor Location");
		return;
	}

	hr->has_location = TRUE;
	hr->location = value;
}

static void char_write_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	char *msg = user_data;

	if (status != 0)
		error("%s failed", msg);

	g_free(msg);
}

static void update_watcher(gpointer data, gpointer user_data)
{
	struct watcher *w = data;
	struct measurement *m = user_data;
	struct heartrate *hr = m->hr;
	const char *path = device_get_path(hr->dev);
	DBusMessageIter iter;
	DBusMessageIter dict;
	DBusMessage *msg;

	msg = dbus_message_new_method_call(w->srv, w->path,
			HEART_RATE_WATCHER_INTERFACE, "MeasurementReceived");
	if (msg == NULL)
		return;

	dbus_message_iter_init_append(msg, &iter);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH , &path);

	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
			DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
			DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
			DBUS_DICT_ENTRY_END_CHAR_AS_STRING, &dict);

	dict_append_entry(&dict, "Value", DBUS_TYPE_UINT16, &m->value);

	if (m->has_energy)
		dict_append_entry(&dict, "Energy", DBUS_TYPE_UINT16,
								&m->energy);

	if (m->has_contact)
		dict_append_entry(&dict, "Contact", DBUS_TYPE_BOOLEAN,
								&m->contact);

	if (m->num_interval > 0)
		dict_append_array(&dict, "Interval", DBUS_TYPE_UINT16,
						&m->interval, m->num_interval);

	dbus_message_iter_close_container(&iter, &dict);

	dbus_message_set_no_reply(msg, TRUE);
	g_dbus_send_message(btd_get_dbus_connection(), msg);
}

static void process_measurement(struct heartrate *hr, const uint8_t *pdu,
								uint16_t len)
{
	struct measurement m;
	uint8_t flags;

	flags = *pdu;

	pdu++;
	len--;

	memset(&m, 0, sizeof(m));

	if (flags & HR_VALUE_FORMAT) {
		if (len < 2) {
			error("Heart Rate Measurement field missing");
			return;
		}

		m.value = get_le16(pdu);
		pdu += 2;
		len -= 2;
	} else {
		if (len < 1) {
			error("Heart Rate Measurement field missing");
			return;
		}

		m.value = *pdu;
		pdu++;
		len--;
	}

	if (flags & ENERGY_EXP_STATUS) {
		if (len < 2) {
			error("Energy Expended field missing");
			return;
		}

		m.has_energy = TRUE;
		m.energy = get_le16(pdu);
		pdu += 2;
		len -= 2;
	}

	if (flags & RR_INTERVAL) {
		int i;

		if (len == 0 || (len % 2 != 0)) {
			error("RR-Interval field malformed");
			return;
		}

		m.num_interval = len / 2;
		m.interval = g_new(uint16_t, m.num_interval);

		for (i = 0; i < m.num_interval; pdu += 2, i++)
			m.interval[i] = get_le16(pdu);
	}

	if (flags & SENSOR_CONTACT_SUPPORT) {
		m.has_contact = TRUE;
		m.contact = !!(flags & SENSOR_CONTACT_DETECTED);
	}

	/* Notify all registered watchers */
	m.hr = hr;
	g_slist_foreach(hr->hradapter->watchers, update_watcher, &m);

	g_free(m.interval);
}

static void notify_handler(const uint8_t *pdu, uint16_t len, gpointer user_data)
{
	struct heartrate *hr = user_data;

	/* should be at least opcode (1b) + handle (2b) */
	if (len < 3) {
		error("Invalid PDU received");
		return;
	}

	process_measurement(hr, pdu + 3, len - 3);
}

static void discover_ccc_cb(uint8_t status, GSList *descs, void *user_data)
{
	struct heartrate *hr = user_data;
	struct gatt_desc *desc;
	uint8_t attr_val[2];
	char *msg;

	if (status != 0) {
		error("Discover Heart Rate Measurement descriptors failed: %s",
							att_ecode2str(status));
		return;
	}

	/* There will be only one descriptor on list and it will be CCC */
	desc = descs->data;

	hr->measurement_ccc_handle = desc->handle;

	if (g_slist_length(hr->hradapter->watchers) == 0) {
		put_le16(0x0000, attr_val);
		msg = g_strdup("Disable measurement");
	} else {
		put_le16(GATT_CLIENT_CHARAC_CFG_NOTIF_BIT, attr_val);
		msg = g_strdup("Enable measurement");
	}

	gatt_write_char(hr->attrib, desc->handle, attr_val, sizeof(attr_val),
							char_write_cb, msg);
}

static void discover_measurement_ccc(struct heartrate *hr,
				struct gatt_char *c, struct gatt_char *c_next)
{
	uint16_t start, end;
	bt_uuid_t uuid;

	start = c->value_handle + 1;

	if (c_next != NULL) {
		if (start == c_next->handle)
			return;
		end = c_next->handle - 1;
	} else if (c->value_handle != hr->svc_range->end) {
		end = hr->svc_range->end;
	} else {
		return;
	}

	bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);

	gatt_discover_desc(hr->attrib, start, end, &uuid, discover_ccc_cb, hr);
}

static void discover_char_cb(uint8_t status, GSList *chars, void *user_data)
{
	struct heartrate *hr = user_data;

	if (status) {
		error("Discover HRS characteristics failed: %s",
							att_ecode2str(status));
		return;
	}

	for (; chars; chars = chars->next) {
		struct gatt_char *c = chars->data;

		if (g_strcmp0(c->uuid, HEART_RATE_MEASUREMENT_UUID) == 0) {
			struct gatt_char *c_next =
				(chars->next ? chars->next->data : NULL);

			hr->attionotid = g_attrib_register(hr->attrib,
						ATT_OP_HANDLE_NOTIFY,
						c->value_handle,
						notify_handler, hr, NULL);

			discover_measurement_ccc(hr, c, c_next);
		} else if (g_strcmp0(c->uuid, BODY_SENSOR_LOCATION_UUID) == 0) {
			DBG("Body Sensor Location supported");

			gatt_read_char(hr->attrib, c->value_handle,
						read_sensor_location_cb, hr);
		} else if (g_strcmp0(c->uuid,
					HEART_RATE_CONTROL_POINT_UUID) == 0) {
			DBG("Heart Rate Control Point supported");
			hr->hrcp_val_handle = c->value_handle;
		}
	}
}

static void enable_measurement(gpointer data, gpointer user_data)
{
	struct heartrate *hr = data;
	uint16_t handle = hr->measurement_ccc_handle;
	uint8_t value[2];
	char *msg;

	if (hr->attrib == NULL || !handle)
		return;

	put_le16(GATT_CLIENT_CHARAC_CFG_NOTIF_BIT, value);
	msg = g_strdup("Enable measurement");

	gatt_write_char(hr->attrib, handle, value, sizeof(value),
							char_write_cb, msg);
}

static void disable_measurement(gpointer data, gpointer user_data)
{
	struct heartrate *hr = data;
	uint16_t handle = hr->measurement_ccc_handle;
	uint8_t value[2];
	char *msg;

	if (hr->attrib == NULL || !handle)
		return;

	put_le16(0x0000, value);
	msg = g_strdup("Disable measurement");

	gatt_write_char(hr->attrib, handle, value, sizeof(value),
							char_write_cb, msg);
}

static void attio_connected_cb(GAttrib *attrib, gpointer user_data)
{
	struct heartrate *hr = user_data;

	DBG("");

	hr->attrib = g_attrib_ref(attrib);

	gatt_discover_char(hr->attrib, hr->svc_range->start, hr->svc_range->end,
						NULL, discover_char_cb, hr);
}

static void attio_disconnected_cb(gpointer user_data)
{
	struct heartrate *hr = user_data;

	DBG("");

	if (hr->attionotid > 0) {
		g_attrib_unregister(hr->attrib, hr->attionotid);
		hr->attionotid = 0;
	}

	g_attrib_unref(hr->attrib);
	hr->attrib = NULL;
}

static void watcher_exit_cb(DBusConnection *conn, void *user_data)
{
	struct watcher *watcher = user_data;
	struct heartrate_adapter *hradapter = watcher->hradapter;

	DBG("heartrate watcher [%s] disconnected", watcher->path);

	hradapter->watchers = g_slist_remove(hradapter->watchers, watcher);
	g_dbus_remove_watch(conn, watcher->id);

	if (g_slist_length(hradapter->watchers) == 0)
		g_slist_foreach(hradapter->devices, disable_measurement, 0);
}

static DBusMessage *register_watcher(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	struct heartrate_adapter *hradapter = data;
	struct watcher *watcher;
	const char *sender = dbus_message_get_sender(msg);
	char *path;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &path,
							DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	watcher = find_watcher(hradapter->watchers, sender, path);
	if (watcher != NULL)
		return btd_error_already_exists(msg);

	watcher = g_new0(struct watcher, 1);
	watcher->hradapter = hradapter;
	watcher->id = g_dbus_add_disconnect_watch(conn, sender, watcher_exit_cb,
						watcher, destroy_watcher);
	watcher->srv = g_strdup(sender);
	watcher->path = g_strdup(path);

	if (g_slist_length(hradapter->watchers) == 0)
		g_slist_foreach(hradapter->devices, enable_measurement, 0);

	hradapter->watchers = g_slist_prepend(hradapter->watchers, watcher);

	DBG("heartrate watcher [%s] registered", path);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *unregister_watcher(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	struct heartrate_adapter *hradapter = data;
	struct watcher *watcher;
	const char *sender = dbus_message_get_sender(msg);
	char *path;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &path,
							DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	watcher = find_watcher(hradapter->watchers, sender, path);
	if (watcher == NULL)
		return btd_error_does_not_exist(msg);

	hradapter->watchers = g_slist_remove(hradapter->watchers, watcher);
	g_dbus_remove_watch(conn, watcher->id);

	if (g_slist_length(hradapter->watchers) == 0)
		g_slist_foreach(hradapter->devices, disable_measurement, 0);

	DBG("heartrate watcher [%s] unregistered", path);

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable heartrate_manager_methods[] = {
	{ GDBUS_METHOD("RegisterWatcher",
			GDBUS_ARGS({ "agent", "o" }), NULL,
			register_watcher) },
	{ GDBUS_METHOD("UnregisterWatcher",
			GDBUS_ARGS({ "agent", "o" }), NULL,
			unregister_watcher) },
	{ }
};

static gboolean property_get_location(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct heartrate *hr = data;
	char *loc;

	if (!hr->has_location)
		return FALSE;

	loc = g_strdup(location2str(hr->location));

	if (loc == NULL)
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &loc);

	g_free(loc);

	return TRUE;
}

static gboolean property_exists_location(const GDBusPropertyTable *property,
								void *data)
{
	struct heartrate *hr = data;

	if (!hr->has_location || location2str(hr->location) == NULL)
		return FALSE;

	return TRUE;
}

static gboolean property_get_reset_supported(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct heartrate *hr = data;
	dbus_bool_t has_reset = !!hr->hrcp_val_handle;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &has_reset);

	return TRUE;
}

static DBusMessage *hrcp_reset(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	struct heartrate *hr = data;
	uint8_t value;
	char *vmsg;

	if (!hr->hrcp_val_handle)
		return btd_error_not_supported(msg);

	if (!hr->attrib)
		return btd_error_not_available(msg);

	value = 0x01;
	vmsg = g_strdup("Reset Control Point");
	gatt_write_char(hr->attrib, hr->hrcp_val_handle, &value,
					sizeof(value), char_write_cb, vmsg);

	DBG("Energy Expended Value has been reset");

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable heartrate_device_methods[] = {
	{ GDBUS_METHOD("Reset", NULL, NULL, hrcp_reset) },
	{ }
};

static const GDBusPropertyTable heartrate_device_properties[] = {
	{ "Location", "s", property_get_location, NULL,
						property_exists_location },
	{ "ResetSupported", "b", property_get_reset_supported },
	{ }
};

static int heartrate_adapter_register(struct btd_adapter *adapter)
{
	struct heartrate_adapter *hradapter;

	hradapter = g_new0(struct heartrate_adapter, 1);
	hradapter->adapter = adapter;

	if (!g_dbus_register_interface(btd_get_dbus_connection(),
						adapter_get_path(adapter),
						HEART_RATE_MANAGER_INTERFACE,
						heartrate_manager_methods,
						NULL, NULL, hradapter,
						destroy_heartrate_adapter)) {
		error("D-Bus failed to register %s interface",
						HEART_RATE_MANAGER_INTERFACE);
		destroy_heartrate_adapter(hradapter);
		return -EIO;
	}

	heartrate_adapters = g_slist_prepend(heartrate_adapters, hradapter);

	return 0;
}

static void heartrate_adapter_unregister(struct btd_adapter *adapter)
{
	struct heartrate_adapter *hradapter;

	hradapter = find_heartrate_adapter(adapter);
	if (hradapter == NULL)
		return;

	heartrate_adapters = g_slist_remove(heartrate_adapters, hradapter);

	g_dbus_unregister_interface(btd_get_dbus_connection(),
					adapter_get_path(hradapter->adapter),
					HEART_RATE_MANAGER_INTERFACE);
}

static int heartrate_device_register(struct btd_device *device,
						struct gatt_primary *prim)
{
	struct btd_adapter *adapter;
	struct heartrate_adapter *hradapter;
	struct heartrate *hr;

	adapter = device_get_adapter(device);

	hradapter = find_heartrate_adapter(adapter);

	if (hradapter == NULL)
		return -1;

	hr = g_new0(struct heartrate, 1);
	hr->dev = btd_device_ref(device);
	hr->hradapter = hradapter;

	if (!g_dbus_register_interface(btd_get_dbus_connection(),
						device_get_path(device),
						HEART_RATE_INTERFACE,
						heartrate_device_methods,
						NULL,
						heartrate_device_properties,
						hr, destroy_heartrate)) {
		error("D-Bus failed to register %s interface",
						HEART_RATE_INTERFACE);
		destroy_heartrate(hr);
		return -EIO;
	}

	hr->svc_range = g_new0(struct att_range, 1);
	hr->svc_range->start = prim->range.start;
	hr->svc_range->end = prim->range.end;

	hradapter->devices = g_slist_prepend(hradapter->devices, hr);

	hr->attioid = btd_device_add_attio_callback(device, attio_connected_cb,
						attio_disconnected_cb, hr);

	return 0;
}

static void heartrate_device_unregister(struct btd_device *device)
{
	struct btd_adapter *adapter;
	struct heartrate_adapter *hradapter;
	struct heartrate *hr;
	GSList *l;

	adapter = device_get_adapter(device);

	hradapter = find_heartrate_adapter(adapter);
	if (hradapter == NULL)
		return;

	l = g_slist_find_custom(hradapter->devices, device, cmp_device);
	if (l == NULL)
		return;

	hr = l->data;

	hradapter->devices = g_slist_remove(hradapter->devices, hr);

	g_dbus_unregister_interface(btd_get_dbus_connection(),
				device_get_path(device), HEART_RATE_INTERFACE);
}

static int heartrate_adapter_probe(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	return heartrate_adapter_register(adapter);
}

static void heartrate_adapter_remove(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	heartrate_adapter_unregister(adapter);
}

static int heartrate_device_probe(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct gatt_primary *prim;

	prim = btd_device_get_primary(device, HEART_RATE_UUID);
	if (prim == NULL)
		return -EINVAL;

	return heartrate_device_register(device, prim);
}

static void heartrate_device_remove(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);

	heartrate_device_unregister(device);
}

static struct btd_profile hrp_profile = {
	.name		= "Heart Rate GATT Driver",
	.remote_uuid	= HEART_RATE_UUID,

	.device_probe	= heartrate_device_probe,
	.device_remove	= heartrate_device_remove,

	.adapter_probe	= heartrate_adapter_probe,
	.adapter_remove	= heartrate_adapter_remove,
};

static int heartrate_init(void)
{
	return btd_profile_register(&hrp_profile);
}

static void heartrate_exit(void)
{
	btd_profile_unregister(&hrp_profile);
}

BLUETOOTH_PLUGIN_DEFINE(heartrate, VERSION, BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
					heartrate_init, heartrate_exit)
