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

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "src/dbus-common.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/error.h"
#include "src/log.h"
#include "attrib/att.h"
#include "attrib/gattrib.h"
#include "attrib/gatt.h"
#include "src/attio.h"
#include "src/textfile.h"

#include "monitor.h"

#define PROXIMITY_INTERFACE "org.bluez.ProximityMonitor1"

#define ALERT_LEVEL_CHR_UUID 0x2A06
#define POWER_LEVEL_CHR_UUID 0x2A07

#define IMMEDIATE_TIMEOUT	5
#define TX_POWER_SIZE		1

enum {
	ALERT_NONE = 0,
	ALERT_MILD,
	ALERT_HIGH,
};

struct monitor {
	struct btd_device *device;
	GAttrib *attrib;
	struct att_range *linkloss;
	struct att_range *txpower;
	struct att_range *immediate;
	struct enabled enabled;
	char *linklosslevel;		/* Link Loss Alert Level */
	char *fallbacklevel;		/* Immediate fallback alert level */
	char *immediatelevel;		/* Immediate Alert Level */
	char *signallevel;		/* Path Loss RSSI level */
	uint16_t linklosshandle;	/* Link Loss Characteristic
					 * Value Handle */
	uint16_t txpowerhandle;		/* Tx Characteristic Value Handle */
	uint16_t immediatehandle;	/* Immediate Alert Value Handle */
	guint immediateto;		/* Reset Immediate Alert to "none" */
	guint attioid;
};

static GSList *monitors = NULL;

static struct monitor *find_monitor(struct btd_device *device)
{
	GSList *l;

	for (l = monitors; l; l = l->next) {
		struct monitor *monitor = l->data;

		if (monitor->device == device)
			return monitor;
	}

	return NULL;
}

static void write_proximity_config(struct btd_device *device, const char *alert,
					const char *level)
{
	char *filename;
	GKeyFile *key_file;
	char *data;
	gsize length = 0;

	filename = btd_device_get_storage_path(device, "proximity");
	if (!filename) {
		warn("Unable to get proximity storage path for device");
		return;
	}

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	if (level)
		g_key_file_set_string(key_file, alert, "Level", level);
	else
		g_key_file_remove_group(key_file, alert, NULL);

	data = g_key_file_to_data(key_file, &length, NULL);
	if (length > 0) {
		create_file(filename, S_IRUSR | S_IWUSR);
		g_file_set_contents(filename, data, length, NULL);
	}

	g_free(data);
	g_free(filename);
	g_key_file_free(key_file);
}

static char *read_proximity_config(struct btd_device *device, const char *alert)
{
	char *filename;
	GKeyFile *key_file;
	char *str;

	filename = btd_device_get_storage_path(device, "proximity");
	if (!filename) {
		warn("Unable to get proximity storage path for device");
		return NULL;
	}

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	str = g_key_file_get_string(key_file, alert, "Level", NULL);

	g_free(filename);
	g_key_file_free(key_file);

	return str;
}

static uint8_t str2level(const char *level)
{
	if (g_strcmp0("high", level) == 0)
		return ALERT_HIGH;
	else if (g_strcmp0("mild", level) == 0)
		return ALERT_MILD;

	return ALERT_NONE;
}

static void linkloss_written(guint8 status, const guint8 *pdu, guint16 plen,
							gpointer user_data)
{
	struct monitor *monitor = user_data;
	struct btd_device *device = monitor->device;
	const char *path = device_get_path(device);

	if (status != 0) {
		error("Link Loss Write Request failed: %s",
							att_ecode2str(status));
		return;
	}

	if (!dec_write_resp(pdu, plen)) {
		error("Link Loss Write Request: protocol error");
		return;
	}

	DBG("Link Loss Alert Level written");

	g_dbus_emit_property_changed(btd_get_dbus_connection(), path,
				PROXIMITY_INTERFACE, "LinkLossAlertLevel");
}

static void char_discovered_cb(uint8_t status, GSList *characteristics,
								void *user_data)
{
	struct monitor *monitor = user_data;
	struct gatt_char *chr;
	uint8_t value = str2level(monitor->linklosslevel);

	if (status) {
		error("Discover Link Loss handle: %s", att_ecode2str(status));
		return;
	}

	DBG("Setting alert level \"%s\" on Reporter", monitor->linklosslevel);

	/* Assume there is a single Alert Level characteristic */
	chr = characteristics->data;
	monitor->linklosshandle = chr->value_handle;

	gatt_write_char(monitor->attrib, monitor->linklosshandle, &value, 1,
						linkloss_written, monitor);
}

static int write_alert_level(struct monitor *monitor)
{
	struct att_range *linkloss = monitor->linkloss;
	bt_uuid_t uuid;

	if (monitor->linklosshandle) {
		uint8_t value = str2level(monitor->linklosslevel);

		gatt_write_char(monitor->attrib, monitor->linklosshandle,
					&value, 1, linkloss_written, monitor);
		return 0;
	}

	bt_uuid16_create(&uuid, ALERT_LEVEL_CHR_UUID);

	/* FIXME: use cache (requires service changed support) ? */
	gatt_discover_char(monitor->attrib, linkloss->start, linkloss->end,
					&uuid, char_discovered_cb, monitor);

	return 0;
}

static void tx_power_read_cb(guint8 status, const guint8 *pdu, guint16 plen,
							gpointer user_data)
{
	uint8_t value[TX_POWER_SIZE];
	ssize_t vlen;

	if (status != 0) {
		DBG("Tx Power Level read failed: %s", att_ecode2str(status));
		return;
	}

	vlen = dec_read_resp(pdu, plen, value, sizeof(value));
	if (vlen < 0) {
		DBG("Protocol error");
		return;
	}

	if (vlen != 1) {
		DBG("Invalid length for TX Power value: %zd", vlen);
		return;
	}

	DBG("Tx Power Level: %02x", (int8_t) value[0]);
}

static void tx_power_handle_cb(uint8_t status, GSList *characteristics,
								void *user_data)
{
	struct monitor *monitor = user_data;
	struct gatt_char *chr;

	if (status) {
		error("Discover Tx Power handle: %s", att_ecode2str(status));
		return;
	}

	chr = characteristics->data;
	monitor->txpowerhandle = chr->value_handle;

	DBG("Tx Power handle: 0x%04x", monitor->txpowerhandle);

	gatt_read_char(monitor->attrib, monitor->txpowerhandle,
							tx_power_read_cb, monitor);
}

static void read_tx_power(struct monitor *monitor)
{
	struct att_range *txpower = monitor->txpower;
	bt_uuid_t uuid;

	if (monitor->txpowerhandle != 0) {
		gatt_read_char(monitor->attrib, monitor->txpowerhandle,
						tx_power_read_cb, monitor);
		return;
	}

	bt_uuid16_create(&uuid, POWER_LEVEL_CHR_UUID);

	gatt_discover_char(monitor->attrib, txpower->start, txpower->end,
				&uuid, tx_power_handle_cb, monitor);
}

static gboolean immediate_timeout(gpointer user_data)
{
	struct monitor *monitor = user_data;
	const char *path = device_get_path(monitor->device);

	monitor->immediateto = 0;

	if (g_strcmp0(monitor->immediatelevel, "none") == 0)
		return FALSE;

	if (monitor->attrib) {
		uint8_t value = ALERT_NONE;
		gatt_write_cmd(monitor->attrib, monitor->immediatehandle,
				&value, 1, NULL, NULL);
	}

	g_free(monitor->immediatelevel);
	monitor->immediatelevel = g_strdup("none");


	g_dbus_emit_property_changed(btd_get_dbus_connection(), path,
				PROXIMITY_INTERFACE, "ImmediateAlertLevel");

	return FALSE;
}

static void immediate_written(gpointer user_data)
{
	struct monitor *monitor = user_data;
	const char *path = device_get_path(monitor->device);

	g_free(monitor->fallbacklevel);
	monitor->fallbacklevel = NULL;


	g_dbus_emit_property_changed(btd_get_dbus_connection(), path,
				PROXIMITY_INTERFACE, "ImmediateAlertLevel");

	monitor->immediateto = g_timeout_add_seconds(IMMEDIATE_TIMEOUT,
						immediate_timeout, monitor);
}

static void write_immediate_alert(struct monitor *monitor)
{
	uint8_t value = str2level(monitor->immediatelevel);

	gatt_write_cmd(monitor->attrib, monitor->immediatehandle, &value, 1,
						immediate_written, monitor);
}

static void immediate_handle_cb(uint8_t status, GSList *characteristics,
								void *user_data)
{
	struct monitor *monitor = user_data;
	struct gatt_char *chr;

	if (status) {
		error("Discover Immediate Alert handle: %s",
						att_ecode2str(status));
		return;
	}

	chr = characteristics->data;
	monitor->immediatehandle = chr->value_handle;

	DBG("Immediate Alert handle: 0x%04x", monitor->immediatehandle);

	if (monitor->fallbacklevel)
		write_immediate_alert(monitor);
}

static void discover_immediate_handle(struct monitor *monitor)
{
	struct att_range *immediate = monitor->immediate;
	bt_uuid_t uuid;

	bt_uuid16_create(&uuid, ALERT_LEVEL_CHR_UUID);

	gatt_discover_char(monitor->attrib, immediate->start, immediate->end,
					&uuid, immediate_handle_cb, monitor);
}

static void attio_connected_cb(GAttrib *attrib, gpointer user_data)
{
	struct monitor *monitor = user_data;

	monitor->attrib = g_attrib_ref(attrib);

	if (monitor->enabled.linkloss)
		write_alert_level(monitor);

	if (monitor->enabled.pathloss)
		read_tx_power(monitor);

	if (monitor->immediatehandle == 0) {
		if(monitor->enabled.pathloss || monitor->enabled.findme)
			discover_immediate_handle(monitor);
	} else if (monitor->fallbacklevel)
		write_immediate_alert(monitor);
}

static void attio_disconnected_cb(gpointer user_data)
{
	struct monitor *monitor = user_data;
	const char *path = device_get_path(monitor->device);

	g_attrib_unref(monitor->attrib);
	monitor->attrib = NULL;

	if (monitor->immediateto == 0)
		return;

	g_source_remove(monitor->immediateto);
	monitor->immediateto = 0;

	if (g_strcmp0(monitor->immediatelevel, "none") == 0)
		return;

	g_free(monitor->immediatelevel);
	monitor->immediatelevel = g_strdup("none");

	g_dbus_emit_property_changed(btd_get_dbus_connection(), path,
				PROXIMITY_INTERFACE, "ImmediateAlertLevel");
}

static gboolean level_is_valid(const char *level)
{
	return (g_str_equal("none", level) ||
			g_str_equal("mild", level) ||
			g_str_equal("high", level));
}

static gboolean property_get_link_loss_level(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct monitor *monitor = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING,
						&monitor->linklosslevel);

	return TRUE;
}

static void property_set_link_loss_level(const GDBusPropertyTable *property,
		DBusMessageIter *iter, GDBusPendingPropertySet id, void *data)
{
	struct monitor *monitor = data;
	const char *level;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_STRING) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	dbus_message_iter_get_basic(iter, &level);

	if (!level_is_valid(level)) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	if (g_strcmp0(monitor->linklosslevel, level) == 0)
		goto done;

	g_free(monitor->linklosslevel);
	monitor->linklosslevel = g_strdup(level);

	write_proximity_config(monitor->device, "LinkLossAlertLevel", level);

	if (monitor->attrib)
		write_alert_level(monitor);

done:
	g_dbus_pending_property_success(id);
}

static gboolean property_exists_link_loss_level(
				const GDBusPropertyTable *property, void *data)
{
	struct monitor *monitor = data;

	if (!monitor->enabled.linkloss)
		return FALSE;

	return TRUE;
}

static gboolean property_get_immediate_alert_level(
					const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct monitor *monitor = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING,
						&monitor->immediatelevel);

	return TRUE;
}

static void property_set_immediate_alert_level(
		const GDBusPropertyTable *property, DBusMessageIter *iter,
		GDBusPendingPropertySet id, void *data)
{
	struct monitor *monitor = data;
	struct btd_device *device = monitor->device;
	const char *level;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_STRING) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	dbus_message_iter_get_basic(iter, &level);

	if (!level_is_valid(level)) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	if (g_strcmp0(monitor->immediatelevel, level) == 0)
		goto done;

	if (monitor->immediateto) {
		g_source_remove(monitor->immediateto);
		monitor->immediateto = 0;
	}

	/* Previous Immediate Alert level if connection/write fails */
	g_free(monitor->fallbacklevel);
	monitor->fallbacklevel = monitor->immediatelevel;

	monitor->immediatelevel = g_strdup(level);

	/*
	 * Means that Link/Path Loss are disabled or there is a pending
	 * writting for Find Me(Immediate Alert characteristic value).
	 * If enabled, Path Loss always registers a connection callback
	 * when the Proximity Monitor starts.
	 */
	if (monitor->attioid == 0)
		monitor->attioid = btd_device_add_attio_callback(device,
							attio_connected_cb,
							attio_disconnected_cb,
							monitor);
	else if (monitor->attrib)
		write_immediate_alert(monitor);

done:
	g_dbus_pending_property_success(id);
}

static gboolean property_exists_immediate_alert_level(
				const GDBusPropertyTable *property, void *data)
{
	struct monitor *monitor = data;

	if (!(monitor->enabled.findme || monitor->enabled.pathloss))
		return FALSE;

	return TRUE;
}

static gboolean property_get_signal_level(
					const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct monitor *monitor = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING,
						&monitor->signallevel);

	return TRUE;
}

static gboolean property_exists_signal_level(const GDBusPropertyTable *property,
								void *data)
{
	struct monitor *monitor = data;

	if (!monitor->enabled.pathloss)
		return FALSE;

	return TRUE;
}

static const GDBusPropertyTable monitor_device_properties[] = {
	{ "LinkLossAlertLevel", "s", property_get_link_loss_level,
					property_set_link_loss_level,
					property_exists_link_loss_level },
	{ "ImmediateAlertLevel", "s", property_get_immediate_alert_level,
					property_set_immediate_alert_level,
					property_exists_immediate_alert_level },
	{ "SignalLevel", "s", property_get_signal_level, NULL,
					property_exists_signal_level },
	{ }
};

static void monitor_destroy(gpointer user_data)
{
	struct monitor *monitor = user_data;

	monitors = g_slist_remove(monitors, monitor);

	btd_device_unref(monitor->device);
	g_free(monitor->linklosslevel);
	g_free(monitor->immediatelevel);
	g_free(monitor->signallevel);
	g_free(monitor);
}

static struct monitor *register_monitor(struct btd_device *device)
{
	const char *path = device_get_path(device);
	struct monitor *monitor;
	char *level;

	monitor = find_monitor(device);
	if (monitor != NULL)
		return monitor;

	level = read_proximity_config(device, "LinkLossAlertLevel");

	monitor = g_new0(struct monitor, 1);
	monitor->device = btd_device_ref(device);
	monitor->linklosslevel = (level ? : g_strdup("high"));
	monitor->signallevel = g_strdup("unknown");
	monitor->immediatelevel = g_strdup("none");

	monitors = g_slist_append(monitors, monitor);

	if (g_dbus_register_interface(btd_get_dbus_connection(), path,
				PROXIMITY_INTERFACE,
				NULL, NULL, monitor_device_properties,
				monitor, monitor_destroy) == FALSE) {
		error("D-Bus failed to register %s interface",
						PROXIMITY_INTERFACE);
		monitor_destroy(monitor);
		return NULL;
	}

	DBG("Registered interface %s on path %s", PROXIMITY_INTERFACE, path);

	return monitor;
}

static void update_monitor(struct monitor *monitor)
{
	if (monitor->txpower != NULL && monitor->immediate != NULL)
		monitor->enabled.pathloss = TRUE;
	else
		monitor->enabled.pathloss = FALSE;

	DBG("Link Loss: %s, Path Loss: %s, FindMe: %s",
				monitor->enabled.linkloss ? "TRUE" : "FALSE",
				monitor->enabled.pathloss ? "TRUE" : "FALSE",
				monitor->enabled.findme ? "TRUE" : "FALSE");

	if (!monitor->enabled.linkloss && !monitor->enabled.pathloss)
		return;

	if (monitor->attioid != 0)
		return;

	monitor->attioid = btd_device_add_attio_callback(monitor->device,
							attio_connected_cb,
							attio_disconnected_cb,
							monitor);
}

int monitor_register_linkloss(struct btd_device *device,
						struct enabled *enabled,
						struct gatt_primary *linkloss)
{
	struct monitor *monitor;

	if (!enabled->linkloss)
		return 0;

	monitor = register_monitor(device);
	if (monitor == NULL)
		return -1;

	monitor->linkloss = g_new0(struct att_range, 1);
	monitor->linkloss->start = linkloss->range.start;
	monitor->linkloss->end = linkloss->range.end;
	monitor->enabled.linkloss = TRUE;

	update_monitor(monitor);

	return 0;
}

int monitor_register_txpower(struct btd_device *device,
						struct enabled *enabled,
						struct gatt_primary *txpower)
{
	struct monitor *monitor;

	if (!enabled->pathloss)
		return 0;

	monitor = register_monitor(device);
	if (monitor == NULL)
		return -1;

	monitor->txpower = g_new0(struct att_range, 1);
	monitor->txpower->start = txpower->range.start;
	monitor->txpower->end = txpower->range.end;

	update_monitor(monitor);

	return 0;
}

int monitor_register_immediate(struct btd_device *device,
						struct enabled *enabled,
						struct gatt_primary *immediate)
{
	struct monitor *monitor;

	if (!enabled->pathloss && !enabled->findme)
		return 0;

	monitor = register_monitor(device);
	if (monitor == NULL)
		return -1;

	monitor->immediate = g_new0(struct att_range, 1);
	monitor->immediate->start = immediate->range.start;
	monitor->immediate->end = immediate->range.end;
	monitor->enabled.findme = enabled->findme;

	update_monitor(monitor);

	return 0;
}

static void cleanup_monitor(struct monitor *monitor)
{
	struct btd_device *device = monitor->device;
	const char *path = device_get_path(device);

	if (monitor->immediate != NULL || monitor->txpower != NULL)
		return;

	if (monitor->immediateto != 0) {
		g_source_remove(monitor->immediateto);
		monitor->immediateto = 0;
	}

	if (monitor->linkloss != NULL)
		return;

	if (monitor->attioid != 0) {
		btd_device_remove_attio_callback(device, monitor->attioid);
		monitor->attioid = 0;
	}

	if (monitor->attrib != NULL) {
		g_attrib_unref(monitor->attrib);
		monitor->attrib = NULL;
	}

	g_dbus_unregister_interface(btd_get_dbus_connection(), path,
							PROXIMITY_INTERFACE);
}

void monitor_unregister_linkloss(struct btd_device *device)
{
	struct monitor *monitor;

	monitor = find_monitor(device);
	if (monitor == NULL)
		return;

	g_free(monitor->linkloss);
	monitor->linkloss = NULL;
	monitor->enabled.linkloss = FALSE;

	cleanup_monitor(monitor);
}

void monitor_unregister_txpower(struct btd_device *device)
{
	struct monitor *monitor;

	monitor = find_monitor(device);
	if (monitor == NULL)
		return;

	g_free(monitor->txpower);
	monitor->txpower = NULL;
	monitor->enabled.pathloss = FALSE;

	cleanup_monitor(monitor);
}

void monitor_unregister_immediate(struct btd_device *device)
{
	struct monitor *monitor;

	monitor = find_monitor(device);
	if (monitor == NULL)
		return;

	g_free(monitor->immediate);
	monitor->immediate = NULL;
	monitor->enabled.findme = FALSE;
	monitor->enabled.pathloss = FALSE;

	cleanup_monitor(monitor);
}
