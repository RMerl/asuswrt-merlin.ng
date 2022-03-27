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
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "src/plugin.h"
#include "src/dbus-common.h"
#include "attrib/att.h"
#include "src/adapter.h"
#include "src/device.h"
#include "attrib/att-database.h"
#include "src/log.h"
#include "attrib/gatt-service.h"
#include "attrib/gattrib.h"
#include "src/attrib-server.h"
#include "attrib/gatt.h"
#include "src/profile.h"
#include "src/error.h"
#include "src/textfile.h"
#include "src/attio.h"

#define PHONE_ALERT_STATUS_SVC_UUID	0x180E
#define ALERT_NOTIF_SVC_UUID		0x1811

#define ALERT_STATUS_CHR_UUID		0x2A3F
#define RINGER_CP_CHR_UUID		0x2A40
#define RINGER_SETTING_CHR_UUID		0x2A41

#define ALERT_NOTIF_CP_CHR_UUID		0x2A44
#define UNREAD_ALERT_CHR_UUID		0x2A45
#define NEW_ALERT_CHR_UUID		0x2A46
#define SUPP_NEW_ALERT_CAT_CHR_UUID	0x2A47
#define SUPP_UNREAD_ALERT_CAT_CHR_UUID	0x2A48

#define ALERT_OBJECT_PATH		"/org/bluez"
#define ALERT_INTERFACE			"org.bluez.Alert1"
#define ALERT_AGENT_INTERFACE		"org.bluez.AlertAgent1"

/* Maximum length for "Text String Information" */
#define NEW_ALERT_MAX_INFO_SIZE		18
/* Maximum length for New Alert Characteristic Value */
#define NEW_ALERT_CHR_MAX_VALUE_SIZE	(NEW_ALERT_MAX_INFO_SIZE + 2)

enum {
	ENABLE_NEW_INCOMING,
	ENABLE_UNREAD_CAT,
	DISABLE_NEW_INCOMING,
	DISABLE_UNREAD_CAT,
	NOTIFY_NEW_INCOMING,
	NOTIFY_UNREAD_CAT,
};

enum {
	RINGER_SILENT_MODE = 1,
	RINGER_MUTE_ONCE,
	RINGER_CANCEL_SILENT_MODE,
};

/* Ringer Setting characteristic values */
enum {
	RINGER_SILENT,
	RINGER_NORMAL,
};

enum notify_type {
	NOTIFY_RINGER_SETTING = 0,
	NOTIFY_ALERT_STATUS,
	NOTIFY_NEW_ALERT,
	NOTIFY_UNREAD_ALERT,
	NOTIFY_SIZE,
};

struct alert_data {
	const char *category;
	char *srv;
	char *path;
	guint watcher;
};

struct alert_adapter {
	struct btd_adapter *adapter;
	uint16_t supp_new_alert_cat_handle;
	uint16_t supp_unread_alert_cat_handle;
	uint16_t hnd_ccc[NOTIFY_SIZE];
	uint16_t hnd_value[NOTIFY_SIZE];
};

struct notify_data {
	struct alert_adapter *al_adapter;
	enum notify_type type;
	uint8_t *value;
	size_t len;
};

struct notify_callback {
	struct notify_data *notify_data;
	struct btd_device *device;
	guint id;
};

static GSList *registered_alerts = NULL;
static GSList *alert_adapters = NULL;
static uint8_t ringer_setting = RINGER_NORMAL;
static uint8_t alert_status = 0;

static const char * const anp_categories[] = {
	"simple",
	"email",
	"news",
	"call",
	"missed-call",
	"sms-mms",
	"voice-mail",
	"schedule",
	"high-priority",
	"instant-message",
};

static const char * const pasp_categories[] = {
	"ringer",
	"vibrate",
	"display",
};

static int adapter_cmp(gconstpointer a, gconstpointer b)
{
	const struct alert_adapter *al_adapter = a;
	const struct btd_adapter *adapter = b;

	return al_adapter->adapter == adapter ? 0 : -1;
}

static struct alert_adapter *find_alert_adapter(struct btd_adapter *adapter)
{
	GSList *l = g_slist_find_custom(alert_adapters, adapter, adapter_cmp);

	return l ? l->data : NULL;
}

static void alert_data_destroy(gpointer user_data)
{
	struct alert_data *alert = user_data;

	if (alert->watcher)
		g_dbus_remove_watch(btd_get_dbus_connection(), alert->watcher);

	g_free(alert->srv);
	g_free(alert->path);
	g_free(alert);
}

static void alert_release(gpointer user_data)
{
	struct alert_data *alert = user_data;
	DBusMessage *msg;

	msg = dbus_message_new_method_call(alert->srv, alert->path,
							ALERT_AGENT_INTERFACE,
							"Release");
	if (msg)
		g_dbus_send_message(btd_get_dbus_connection(), msg);

	alert_data_destroy(alert);
}

static void alert_destroy(gpointer user_data)
{
	DBG("");

	g_slist_free_full(registered_alerts, alert_release);
	registered_alerts = NULL;
}

static const char *valid_category(const char *category)
{
	unsigned i;

	for (i = 0; i < G_N_ELEMENTS(anp_categories); i++) {
		if (g_str_equal(anp_categories[i], category))
			return anp_categories[i];
	}

	for (i = 0; i < G_N_ELEMENTS(pasp_categories); i++) {
		if (g_str_equal(pasp_categories[i], category))
			return pasp_categories[i];
	}

	return NULL;
}

static struct alert_data *get_alert_data_by_category(const char *category)
{
	GSList *l;
	struct alert_data *alert;

	for (l = registered_alerts; l; l = g_slist_next(l)) {
		alert = l->data;
		if (g_str_equal(alert->category, category))
			return alert;
	}

	return NULL;
}

static gboolean registered_category(const char *category)
{
	struct alert_data *alert;

	alert = get_alert_data_by_category(category);
	if (alert)
		return TRUE;

	return FALSE;
}

static gboolean pasp_category(const char *category)
{
	unsigned i;

	for (i = 0; i < G_N_ELEMENTS(pasp_categories); i++)
		if (g_str_equal(category, pasp_categories[i]))
			return TRUE;

	return FALSE;
}

static gboolean valid_description(const char *category,
						const char *description)
{
	if (!pasp_category(category)) {
		if (strlen(description) >= NEW_ALERT_MAX_INFO_SIZE)
			return FALSE;

		return TRUE;
	}

	if (g_str_equal(description, "active") ||
					g_str_equal(description, "not active"))
		return TRUE;

	if (g_str_equal(category, "ringer"))
		if (g_str_equal(description, "enabled") ||
					g_str_equal(description, "disabled"))
			return TRUE;

	return FALSE;
}

static gboolean valid_count(const char *category, uint16_t count)
{
	if (!pasp_category(category) && count > 0 && count <= 255)
		return TRUE;

	if (pasp_category(category) && count == 1)
		return TRUE;

	return FALSE;
}

static void update_supported_categories(gpointer data, gpointer user_data)
{
	struct alert_adapter *al_adapter = data;
	struct btd_adapter *adapter = al_adapter->adapter;
	uint8_t value[2];
	unsigned int i;

	memset(value, 0, sizeof(value));

	for (i = 0; i < G_N_ELEMENTS(anp_categories); i++) {
		if (registered_category(anp_categories[i]))
			hci_set_bit(i, value);
	}

	attrib_db_update(adapter, al_adapter->supp_new_alert_cat_handle, NULL,
						value, sizeof(value), NULL);

	/* FIXME: For now report all registered categories as supporting unread
	 * status, until it is known which ones should be supported */
	attrib_db_update(adapter, al_adapter->supp_unread_alert_cat_handle,
					NULL, value, sizeof(value), NULL);
}

static void watcher_disconnect(DBusConnection *conn, void *user_data)
{
	struct alert_data *alert = user_data;

	DBG("Category %s was disconnected", alert->category);

	registered_alerts = g_slist_remove(registered_alerts, alert);
	alert_data_destroy(alert);

	g_slist_foreach(alert_adapters, update_supported_categories, NULL);
}

static gboolean is_notifiable_device(struct btd_device *device, uint16_t ccc)
{
	char *filename;
	GKeyFile *key_file;
	char handle[6];
	char *str;
	uint16_t val;
	gboolean result;

	sprintf(handle, "%hu", ccc);

	filename = btd_device_get_storage_path(device, "ccc");
	if (!filename) {
		warn("Unable to get ccc storage path for device");
		return FALSE;
	}

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	str = g_key_file_get_string(key_file, handle, "Value", NULL);
	if (!str) {
		result = FALSE;
		goto end;
	}

	val = strtol(str, NULL, 16);
	if (!(val & 0x0001)) {
		result = FALSE;
		goto end;
	}

	result = TRUE;
end:
	g_free(str);
	g_free(filename);
	g_key_file_free(key_file);

	return result;
}

static void destroy_notify_callback(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct notify_callback *cb = user_data;

	DBG("status=%#x", status);

	btd_device_remove_attio_callback(cb->device, cb->id);
	btd_device_unref(cb->device);
	g_free(cb->notify_data->value);
	g_free(cb->notify_data);
	g_free(cb);
}

static void attio_connected_cb(GAttrib *attrib, gpointer user_data)
{
	struct notify_callback *cb = user_data;
	struct notify_data *nd = cb->notify_data;
	enum notify_type type = nd->type;
	struct alert_adapter *al_adapter = nd->al_adapter;
	size_t len;
	uint8_t *pdu = g_attrib_get_buffer(attrib, &len);


	switch (type) {
	case NOTIFY_RINGER_SETTING:
		len = enc_notification(al_adapter->hnd_value[type],
				&ringer_setting, sizeof(ringer_setting),
				pdu, len);
		break;
	case NOTIFY_ALERT_STATUS:
		len = enc_notification(al_adapter->hnd_value[type],
				&alert_status, sizeof(alert_status),
				pdu, len);
		break;
	case NOTIFY_NEW_ALERT:
	case NOTIFY_UNREAD_ALERT:
		len = enc_notification(al_adapter->hnd_value[type],
					nd->value, nd->len, pdu, len);
		break;
	case NOTIFY_SIZE:
	default:
		DBG("Unknown type, could not send notification");
		goto end;
	}

	DBG("Send notification for handle: 0x%04x, ccc: 0x%04x",
					al_adapter->hnd_value[type],
					al_adapter->hnd_ccc[type]);

	g_attrib_send(attrib, 0, pdu, len, destroy_notify_callback, cb, NULL);

	return;

end:
	btd_device_remove_attio_callback(cb->device, cb->id);
	btd_device_unref(cb->device);
	g_free(cb->notify_data->value);
	g_free(cb->notify_data);
	g_free(cb);
}

static void filter_devices_notify(struct btd_device *device, void *user_data)
{
	struct notify_data *notify_data = user_data;
	struct alert_adapter *al_adapter = notify_data->al_adapter;
	enum notify_type type = notify_data->type;
	struct notify_callback *cb;

	if (!is_notifiable_device(device, al_adapter->hnd_ccc[type]))
		return;

	cb = g_new0(struct notify_callback, 1);
	cb->notify_data = notify_data;
	cb->device = btd_device_ref(device);
	cb->id = btd_device_add_attio_callback(device,
						attio_connected_cb, NULL, cb);
}

static void notify_devices(struct alert_adapter *al_adapter,
			enum notify_type type, uint8_t *value, size_t len)
{
	struct notify_data *notify_data;

	notify_data = g_new0(struct notify_data, 1);
	notify_data->al_adapter = al_adapter;
	notify_data->type = type;
	notify_data->value = g_memdup(value, len);
	notify_data->len = len;

	btd_adapter_for_each_device(al_adapter->adapter, filter_devices_notify,
					notify_data);
}

static void pasp_notification(enum notify_type type)
{
	GSList *it;
	struct alert_adapter *al_adapter;

	for (it = alert_adapters; it; it = g_slist_next(it)) {
		al_adapter = it->data;

		notify_devices(al_adapter, type, NULL, 0);
	}
}

static DBusMessage *register_alert(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	const char *sender = dbus_message_get_sender(msg);
	char *path;
	const char *category;
	const char *c;
	struct alert_data *alert;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &c,
			DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	category = valid_category(c);
	if (!category) {
		DBG("Invalid category: %s", c);
		return btd_error_invalid_args(msg);
	}

	if (registered_category(category)) {
		DBG("Category %s already registered", category);
		return dbus_message_new_method_return(msg);
	}

	alert = g_new0(struct alert_data, 1);
	alert->srv = g_strdup(sender);
	alert->path = g_strdup(path);
	alert->category = category;
	alert->watcher = g_dbus_add_disconnect_watch(conn, alert->srv,
					watcher_disconnect, alert, NULL);

	if (alert->watcher == 0) {
		alert_data_destroy(alert);
		DBG("Could not register disconnect watcher");
		return btd_error_failed(msg,
				"Could not register disconnect watcher");
	}

	registered_alerts = g_slist_append(registered_alerts, alert);

	g_slist_foreach(alert_adapters, update_supported_categories, NULL);

	DBG("RegisterAlert(\"%s\", \"%s\")", alert->category, alert->path);

	return dbus_message_new_method_return(msg);
}

static void update_new_alert(gpointer data, gpointer user_data)
{
	struct alert_adapter *al_adapter = data;
	struct btd_adapter *adapter = al_adapter->adapter;
	uint8_t *value = user_data;

	attrib_db_update(adapter, al_adapter->hnd_value[NOTIFY_NEW_ALERT], NULL,
						&value[1], value[0], NULL);

	notify_devices(al_adapter, NOTIFY_NEW_ALERT, &value[1], value[0]);
}

static void update_phone_alerts(const char *category, const char *description)
{
	unsigned int i;

	if (g_str_equal(category, "ringer")) {
		if (g_str_equal(description, "enabled")) {
			ringer_setting = RINGER_NORMAL;
			pasp_notification(NOTIFY_RINGER_SETTING);
			return;
		} else if (g_str_equal(description, "disabled")) {
			ringer_setting = RINGER_SILENT;
			pasp_notification(NOTIFY_RINGER_SETTING);
			return;
		}
	}

	for (i = 0; i < G_N_ELEMENTS(pasp_categories); i++) {
		if (g_str_equal(pasp_categories[i], category)) {
			if (g_str_equal(description, "active")) {
				alert_status |= (1 << i);
				pasp_notification(NOTIFY_ALERT_STATUS);
			} else if (g_str_equal(description, "not active")) {
				alert_status &= ~(1 << i);
				pasp_notification(NOTIFY_ALERT_STATUS);
			}
			break;
		}
	}
}

static DBusMessage *new_alert(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	const char *sender = dbus_message_get_sender(msg);
	const char *category, *description;
	struct alert_data *alert;
	uint16_t count;
	unsigned int i;
	size_t dlen;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &category,
			DBUS_TYPE_UINT16, &count, DBUS_TYPE_STRING,
			&description, DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	alert = get_alert_data_by_category(category);
	if (!alert) {
		DBG("Category %s not registered", category);
		return btd_error_invalid_args(msg);
	}

	if (!g_str_equal(alert->srv, sender)) {
		DBG("Sender %s is not registered in category %s", sender,
								category);
		return btd_error_invalid_args(msg);
	}

	if (!valid_description(category, description)) {
		DBG("Description %s is invalid for %s category",
							description, category);
		return btd_error_invalid_args(msg);
	}

	if (!valid_count(category, count)) {
		DBG("Count %d is invalid for %s category", count, category);
		return btd_error_invalid_args(msg);
	}

	dlen = strlen(description);

	for (i = 0; i < G_N_ELEMENTS(anp_categories); i++) {
		uint8_t value[NEW_ALERT_CHR_MAX_VALUE_SIZE + 1];
		uint8_t *ptr = value;

		if (!g_str_equal(anp_categories[i], category))
			continue;

		memset(value, 0, sizeof(value));

		*ptr++ = 2; /* Attribute value size */
		*ptr++ = i; /* Category ID (mandatory) */
		*ptr++ = count; /* Number of New Alert (mandatory) */
		/* Text String Information (optional) */
		strncpy((char *) ptr, description,
						NEW_ALERT_MAX_INFO_SIZE - 1);

		if (dlen > 0)
			*value += dlen + 1;

		g_slist_foreach(alert_adapters, update_new_alert, value);
	}

	if (pasp_category(category))
		update_phone_alerts(category, description);

	DBG("NewAlert(\"%s\", %d, \"%s\")", category, count, description);

	return dbus_message_new_method_return(msg);
}

static int agent_ringer_mute_once(void)
{
	struct alert_data *alert;
	DBusMessage *msg;

	alert = get_alert_data_by_category("ringer");
	if (!alert) {
		DBG("Category ringer is not registered");
		return -EINVAL;
	}

	msg = dbus_message_new_method_call(alert->srv, alert->path,
					ALERT_AGENT_INTERFACE, "MuteOnce");
	if (!msg)
		return -ENOMEM;

	dbus_message_set_no_reply(msg, TRUE);
	g_dbus_send_message(btd_get_dbus_connection(), msg);

	return 0;
}

static int agent_ringer_set_ringer(const char *mode)
{
	struct alert_data *alert;
	DBusMessage *msg;

	alert = get_alert_data_by_category("ringer");
	if (!alert) {
		DBG("Category ringer is not registered");
		return -EINVAL;
	}

	msg = dbus_message_new_method_call(alert->srv, alert->path,
					ALERT_AGENT_INTERFACE, "SetRinger");
	if (!msg)
		return -ENOMEM;

	dbus_message_append_args(msg, DBUS_TYPE_STRING, &mode,
							DBUS_TYPE_INVALID);

	dbus_message_set_no_reply(msg, TRUE);
	g_dbus_send_message(btd_get_dbus_connection(), msg);

	return 0;
}

static void update_unread_alert(gpointer data, gpointer user_data)
{
	struct alert_adapter *al_adapter = data;
	struct btd_adapter *adapter = al_adapter->adapter;
	uint8_t *value = user_data;

	attrib_db_update(adapter,
			al_adapter->hnd_value[NOTIFY_UNREAD_ALERT], NULL, value,
			2, NULL);

	notify_devices(al_adapter, NOTIFY_UNREAD_ALERT, value, 2);
}

static DBusMessage *unread_alert(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	const char *sender = dbus_message_get_sender(msg);
	struct alert_data *alert;
	const char *category;
	unsigned int i;
	uint16_t count;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &category,
						DBUS_TYPE_UINT16, &count,
						DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	alert = get_alert_data_by_category(category);
	if (!alert) {
		DBG("Category %s not registered", category);
		return btd_error_invalid_args(msg);
	}

	if (!valid_count(category, count)) {
		DBG("Count %d is invalid for %s category", count, category);
		return btd_error_invalid_args(msg);
	}

	if (!g_str_equal(alert->srv, sender)) {
		DBG("Sender %s is not registered in category %s", sender,
								category);
		return btd_error_invalid_args(msg);
	}

	for (i = 0; i < G_N_ELEMENTS(anp_categories); i++) {
		if (g_str_equal(anp_categories[i], category)) {
			uint8_t value[2];

			value[0] = i; /* Category ID */
			value[1] = count; /* Unread count */

			g_slist_foreach(alert_adapters, update_unread_alert,
									value);
		}
	}

	DBG("category %s, count %d", category, count);

	return dbus_message_new_method_return(msg);
}

static uint8_t ringer_cp_write(struct attribute *a,
						struct btd_device *device,
						gpointer user_data)
{
	DBG("a = %p", a);

	if (a->len > 1) {
		DBG("Invalid command size (%zu)", a->len);
		return 0;
	}

	switch (a->data[0]) {
	case RINGER_SILENT_MODE:
		DBG("Silent Mode");
		agent_ringer_set_ringer("disabled");
		break;
	case RINGER_MUTE_ONCE:
		DBG("Mute Once");
		agent_ringer_mute_once();
		break;
	case RINGER_CANCEL_SILENT_MODE:
		DBG("Cancel Silent Mode");
		agent_ringer_set_ringer("enabled");
		break;
	default:
		DBG("Invalid command (0x%02x)", a->data[0]);
	}

	return 0;
}

static uint8_t alert_status_read(struct attribute *a,
						struct btd_device *device,
						gpointer user_data)
{
	struct btd_adapter *adapter = user_data;

	DBG("a = %p", a);

	if (a->data == NULL || a->data[0] != alert_status)
		attrib_db_update(adapter, a->handle, NULL, &alert_status,
						sizeof(alert_status), NULL);

	return 0;
}

static uint8_t ringer_setting_read(struct attribute *a,
						struct btd_device *device,
						gpointer user_data)
{
	struct btd_adapter *adapter = user_data;

	DBG("a = %p", a);

	if (a->data == NULL || a->data[0] != ringer_setting)
		attrib_db_update(adapter, a->handle, NULL, &ringer_setting,
						sizeof(ringer_setting), NULL);

	return 0;
}

static void register_phone_alert_service(struct alert_adapter *al_adapter)
{
	bt_uuid_t uuid;

	bt_uuid16_create(&uuid, PHONE_ALERT_STATUS_SVC_UUID);

	/* Phone Alert Status Service */
	gatt_service_add(al_adapter->adapter, GATT_PRIM_SVC_UUID, &uuid,
			/* Alert Status characteristic */
			GATT_OPT_CHR_UUID16, ALERT_STATUS_CHR_UUID,
			GATT_OPT_CHR_PROPS, GATT_CHR_PROP_READ |
							GATT_CHR_PROP_NOTIFY,
			GATT_OPT_CHR_VALUE_CB, ATTRIB_READ,
			alert_status_read, al_adapter->adapter,
			GATT_OPT_CCC_GET_HANDLE,
			&al_adapter->hnd_ccc[NOTIFY_ALERT_STATUS],
			GATT_OPT_CHR_VALUE_GET_HANDLE,
			&al_adapter->hnd_value[NOTIFY_ALERT_STATUS],
			/* Ringer Control Point characteristic */
			GATT_OPT_CHR_UUID16, RINGER_CP_CHR_UUID,
			GATT_OPT_CHR_PROPS, GATT_CHR_PROP_WRITE_WITHOUT_RESP,
			GATT_OPT_CHR_VALUE_CB, ATTRIB_WRITE,
			ringer_cp_write, NULL,
			/* Ringer Setting characteristic */
			GATT_OPT_CHR_UUID16, RINGER_SETTING_CHR_UUID,
			GATT_OPT_CHR_PROPS, GATT_CHR_PROP_READ |
							GATT_CHR_PROP_NOTIFY,
			GATT_OPT_CHR_VALUE_CB, ATTRIB_READ,
			ringer_setting_read, al_adapter->adapter,
			GATT_OPT_CCC_GET_HANDLE,
			&al_adapter->hnd_ccc[NOTIFY_RINGER_SETTING],
			GATT_OPT_CHR_VALUE_GET_HANDLE,
			&al_adapter->hnd_value[NOTIFY_RINGER_SETTING],
			GATT_OPT_INVALID);
}

static uint8_t supp_new_alert_cat_read(struct attribute *a,
						struct btd_device *device,
						gpointer user_data)
{
	struct btd_adapter *adapter = user_data;
	uint8_t value[] = { 0x00, 0x00 };

	DBG("a = %p", a);

	if (a->data == NULL)
		attrib_db_update(adapter, a->handle, NULL, value, sizeof(value),
									NULL);

	return 0;
}

static uint8_t supp_unread_alert_cat_read(struct attribute *a,
						struct btd_device *device,
						gpointer user_data)
{
	struct btd_adapter *adapter = user_data;
	uint8_t value[] = { 0x00, 0x00 };

	DBG("a = %p", a);

	if (a->data == NULL)
		attrib_db_update(adapter, a->handle, NULL, value, sizeof(value),
									NULL);

	return 0;
}

static uint8_t alert_notif_cp_write(struct attribute *a,
						struct btd_device *device,
						gpointer user_data)
{
	DBG("a = %p", a);

	if (a->len < 2)
		return 0;

	switch (a->data[0]) {
	case ENABLE_NEW_INCOMING:
		DBG("ENABLE_NEW_INCOMING: 0x%02x", a->data[1]);
		break;
	case ENABLE_UNREAD_CAT:
		DBG("ENABLE_UNREAD_CAT: 0x%02x", a->data[1]);
		break;
	case DISABLE_NEW_INCOMING:
		DBG("DISABLE_NEW_INCOMING: 0x%02x", a->data[1]);
		break;
	case DISABLE_UNREAD_CAT:
		DBG("DISABLE_UNREAD_CAT: 0x%02x", a->data[1]);
		break;
	case NOTIFY_NEW_INCOMING:
		DBG("NOTIFY_NEW_INCOMING: 0x%02x", a->data[1]);
		break;
	case NOTIFY_UNREAD_CAT:
		DBG("NOTIFY_UNREAD_CAT: 0x%02x", a->data[1]);
		break;
	default:
		DBG("0x%02x 0x%02x", a->data[0], a->data[1]);
	}

	return 0;
}

static void register_alert_notif_service(struct alert_adapter *al_adapter)
{
	bt_uuid_t uuid;

	bt_uuid16_create(&uuid, ALERT_NOTIF_SVC_UUID);

	/* Alert Notification Service */
	gatt_service_add(al_adapter->adapter, GATT_PRIM_SVC_UUID, &uuid,
			/* Supported New Alert Category */
			GATT_OPT_CHR_UUID16, SUPP_NEW_ALERT_CAT_CHR_UUID,
			GATT_OPT_CHR_PROPS, GATT_CHR_PROP_READ,
			GATT_OPT_CHR_VALUE_CB, ATTRIB_READ,
			supp_new_alert_cat_read, al_adapter->adapter,
			GATT_OPT_CHR_VALUE_GET_HANDLE,
			&al_adapter->supp_new_alert_cat_handle,
			/* New Alert */
			GATT_OPT_CHR_UUID16, NEW_ALERT_CHR_UUID,
			GATT_OPT_CHR_PROPS, GATT_CHR_PROP_NOTIFY,
			GATT_OPT_CCC_GET_HANDLE,
			&al_adapter->hnd_ccc[NOTIFY_NEW_ALERT],
			GATT_OPT_CHR_VALUE_GET_HANDLE,
			&al_adapter->hnd_value[NOTIFY_NEW_ALERT],
			/* Supported Unread Alert Category */
			GATT_OPT_CHR_UUID16, SUPP_UNREAD_ALERT_CAT_CHR_UUID,
			GATT_OPT_CHR_PROPS, GATT_CHR_PROP_READ,
			GATT_OPT_CHR_VALUE_CB, ATTRIB_READ,
			supp_unread_alert_cat_read, al_adapter->adapter,
			GATT_OPT_CHR_VALUE_GET_HANDLE,
			&al_adapter->supp_unread_alert_cat_handle,
			/* Unread Alert Status */
			GATT_OPT_CHR_UUID16, UNREAD_ALERT_CHR_UUID,
			GATT_OPT_CHR_PROPS, GATT_CHR_PROP_NOTIFY,
			GATT_OPT_CCC_GET_HANDLE,
			&al_adapter->hnd_ccc[NOTIFY_UNREAD_ALERT],
			GATT_OPT_CHR_VALUE_GET_HANDLE,
			&al_adapter->hnd_value[NOTIFY_UNREAD_ALERT],
			/* Alert Notification Control Point */
			GATT_OPT_CHR_UUID16, ALERT_NOTIF_CP_CHR_UUID,
			GATT_OPT_CHR_PROPS, GATT_CHR_PROP_WRITE,
			GATT_OPT_CHR_VALUE_CB, ATTRIB_WRITE,
			alert_notif_cp_write, NULL,
			GATT_OPT_INVALID);
}

static int alert_server_probe(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	struct alert_adapter *al_adapter;

	al_adapter = g_new0(struct alert_adapter, 1);
	al_adapter->adapter = btd_adapter_ref(adapter);

	alert_adapters = g_slist_append(alert_adapters, al_adapter);

	register_phone_alert_service(al_adapter);
	register_alert_notif_service(al_adapter);

	return 0;
}

static void alert_server_remove(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	struct alert_adapter *al_adapter;

	al_adapter = find_alert_adapter(adapter);
	if (!al_adapter)
		return;

	alert_adapters = g_slist_remove(alert_adapters, al_adapter);
	btd_adapter_unref(al_adapter->adapter);

	g_free(al_adapter);
}

static struct btd_profile alert_profile = {
	.name = "gatt-alert-server",
	.adapter_probe = alert_server_probe,
	.adapter_remove = alert_server_remove,
};

static const GDBusMethodTable alert_methods[] = {
	{ GDBUS_METHOD("RegisterAlert",
			GDBUS_ARGS({ "category", "s" },
				   { "agent", "o" }), NULL,
			register_alert) },
	{ GDBUS_METHOD("NewAlert",
			GDBUS_ARGS({ "category", "s" },
				   { "count", "q" },
				   { "description", "s" }), NULL,
			new_alert) },
	{ GDBUS_METHOD("UnreadAlert",
			GDBUS_ARGS({ "category", "s" }, { "count", "q" }), NULL,
			unread_alert) },
	{ }
};

static int alert_server_init(void)
{
	if (!g_dbus_register_interface(btd_get_dbus_connection(),
					ALERT_OBJECT_PATH, ALERT_INTERFACE,
					alert_methods, NULL, NULL, NULL,
					alert_destroy)) {
		error("D-Bus failed to register %s interface",
							ALERT_INTERFACE);
		return -EIO;
	}

	return btd_profile_register(&alert_profile);
}

static void alert_server_exit(void)
{
	btd_profile_unregister(&alert_profile);

	g_dbus_unregister_interface(btd_get_dbus_connection(),
					ALERT_OBJECT_PATH, ALERT_INTERFACE);
}

static int alert_init(void)
{
	return alert_server_init();
}

static void alert_exit(void)
{
	alert_server_exit();
}

BLUETOOTH_PLUGIN_DEFINE(alert, VERSION,
			BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
			alert_init, alert_exit)
