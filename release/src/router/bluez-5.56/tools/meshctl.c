// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <wordexp.h>

#include <inttypes.h>
#include <ctype.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include "bluetooth/bluetooth.h"

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/uuid.h"
#include "src/shared/shell.h"
#include "src/shared/util.h"
#include "gdbus/gdbus.h"

#include "mesh/agent.h"
#include "mesh/config-model.h"

#include "mesh-gatt/mesh-net.h"
#include "mesh-gatt/gatt.h"
#include "mesh-gatt/crypto.h"
#include "mesh-gatt/node.h"
#include "mesh-gatt/net.h"
#include "mesh-gatt/keys.h"
#include "mesh-gatt/prov.h"
#include "mesh-gatt/util.h"
#include "mesh-gatt/prov-db.h"
#include "mesh-gatt/onoff-model.h"

/* String display constants */
#define COLORED_NEW	COLOR_GREEN "NEW" COLOR_OFF
#define COLORED_CHG	COLOR_YELLOW "CHG" COLOR_OFF
#define COLORED_DEL	COLOR_RED "DEL" COLOR_OFF

#define PROMPT_ON	COLOR_BLUE "[meshctl]" COLOR_OFF "# "
#define PROMPT_OFF	"Waiting to connect to bluetoothd..."

#define MESH_PROV_DATA_IN_UUID_STR	"00002adb-0000-1000-8000-00805f9b34fb"
#define MESH_PROV_DATA_OUT_UUID_STR	"00002adc-0000-1000-8000-00805f9b34fb"
#define MESH_PROXY_DATA_IN_UUID_STR	"00002add-0000-1000-8000-00805f9b34fb"
#define MESH_PROXY_DATA_OUT_UUID_STR	"00002ade-0000-1000-8000-00805f9b34fb"

static DBusConnection *dbus_conn;

struct adapter {
GDBusProxy *proxy;
	GList *mesh_devices;
};

struct mesh_device {
	GDBusProxy *proxy;
	uint8_t dev_uuid[16];
	gboolean hide;
};

GList *service_list;
GList *char_list;

static GList *ctrl_list;
static struct adapter *default_ctrl;

static char *mesh_prov_db_filename;
static char *mesh_local_config_filename;

static bool discovering = false;
static bool discover_mesh;
static uint16_t prov_net_key_index = NET_IDX_PRIMARY;
static const struct bt_shell_menu main_menu;

#define CONN_TYPE_NETWORK	0x00
#define CONN_TYPE_IDENTITY	0x01
#define CONN_TYPE_PROVISION	0x02
#define CONN_TYPE_INVALID	0xff

#define NET_IDX_INVALID		0xffff

struct {
	GDBusProxy *device;
	GDBusProxy *service;
	GDBusProxy *data_in;
	GDBusProxy *data_out;
	bool session_open;
	uint16_t unicast;
	uint16_t net_idx;
	uint8_t dev_uuid[16];
	uint8_t type;
} connection;

static bool service_is_mesh(GDBusProxy *proxy, const char *target_uuid)
{
	DBusMessageIter iter;
	const char *uuid;

	if (g_dbus_proxy_get_property(proxy, "UUID", &iter) == FALSE)
		return false;

	dbus_message_iter_get_basic(&iter, &uuid);

	if (target_uuid)
		return (!bt_uuid_strcmp(uuid, target_uuid));
	else if (bt_uuid_strcmp(uuid, MESH_PROV_SVC_UUID) ||
				bt_uuid_strcmp(uuid, MESH_PROXY_SVC_UUID))
		return true;
	else
		return false;
}

static bool char_is_mesh(GDBusProxy *proxy, const char *target_uuid)
{
	DBusMessageIter iter;
	const char *uuid;

	if (g_dbus_proxy_get_property(proxy, "UUID", &iter) == FALSE)
		return false;

	dbus_message_iter_get_basic(&iter, &uuid);

	if (target_uuid)
		return (!bt_uuid_strcmp(uuid, target_uuid));

	if (!bt_uuid_strcmp(uuid, MESH_PROV_DATA_IN_UUID_STR))
		return true;

	if (!bt_uuid_strcmp(uuid, MESH_PROV_DATA_OUT_UUID_STR))
		return true;

	if (!bt_uuid_strcmp(uuid, MESH_PROXY_DATA_IN_UUID_STR))
		return true;

	if (!bt_uuid_strcmp(uuid, MESH_PROXY_DATA_OUT_UUID_STR))
		return true;

	return false;
}

static gboolean check_default_ctrl(void)
{
	if (!default_ctrl) {
		bt_shell_printf("No default controller available\n");
		return FALSE;
	}

	return TRUE;
}

static void proxy_leak(gpointer data)
{
	bt_shell_printf("Leaking proxy %p\n", data);
}

static void connect_handler(DBusConnection *connection, void *user_data)
{
	bt_shell_set_prompt(PROMPT_ON);
}

static void disconnect_handler(DBusConnection *connection, void *user_data)
{
	bt_shell_detach();

	bt_shell_set_prompt(PROMPT_OFF);

	g_list_free_full(ctrl_list, proxy_leak);
	ctrl_list = NULL;

	default_ctrl = NULL;
}

static void print_adapter(GDBusProxy *proxy, const char *description)
{
	DBusMessageIter iter;
	const char *address, *name;

	if (g_dbus_proxy_get_property(proxy, "Address", &iter) == FALSE)
		return;

	dbus_message_iter_get_basic(&iter, &address);

	if (g_dbus_proxy_get_property(proxy, "Alias", &iter) == TRUE)
		dbus_message_iter_get_basic(&iter, &name);
	else
		name = "<unknown>";

	bt_shell_printf("%s%s%sController %s %s %s\n",
				description ? "[" : "",
				description ? : "",
				description ? "] " : "",
				address, name,
				default_ctrl &&
				default_ctrl->proxy == proxy ?
				"[default]" : "");

}

static void print_device(GDBusProxy *proxy, const char *description)
{
	DBusMessageIter iter;
	const char *address, *name;

	if (g_dbus_proxy_get_property(proxy, "Address", &iter) == FALSE)
		return;

	dbus_message_iter_get_basic(&iter, &address);

	if (g_dbus_proxy_get_property(proxy, "Alias", &iter) == TRUE)
		dbus_message_iter_get_basic(&iter, &name);
	else
		name = "<unknown>";

	bt_shell_printf("%s%s%sDevice %s %s\n",
				description ? "[" : "",
				description ? : "",
				description ? "] " : "",
				address, name);
}

static void print_iter(const char *label, const char *name,
						DBusMessageIter *iter)
{
	dbus_bool_t valbool;
	dbus_uint32_t valu32;
	dbus_uint16_t valu16;
	dbus_int16_t vals16;
	unsigned char byte;
	const char *valstr;
	DBusMessageIter subiter;
	char *entry;

	if (iter == NULL) {
		bt_shell_printf("%s%s is nil\n", label, name);
		return;
	}

	switch (dbus_message_iter_get_arg_type(iter)) {
	case DBUS_TYPE_INVALID:
		bt_shell_printf("%s%s is invalid\n", label, name);
		break;
	case DBUS_TYPE_STRING:
	case DBUS_TYPE_OBJECT_PATH:
		dbus_message_iter_get_basic(iter, &valstr);
		bt_shell_printf("%s%s: %s\n", label, name, valstr);
		break;
	case DBUS_TYPE_BOOLEAN:
		dbus_message_iter_get_basic(iter, &valbool);
		bt_shell_printf("%s%s: %s\n", label, name,
					valbool == TRUE ? "yes" : "no");
		break;
	case DBUS_TYPE_UINT32:
		dbus_message_iter_get_basic(iter, &valu32);
		bt_shell_printf("%s%s: 0x%06x\n", label, name, valu32);
		break;
	case DBUS_TYPE_UINT16:
		dbus_message_iter_get_basic(iter, &valu16);
		bt_shell_printf("%s%s: 0x%04x\n", label, name, valu16);
		break;
	case DBUS_TYPE_INT16:
		dbus_message_iter_get_basic(iter, &vals16);
		bt_shell_printf("%s%s: %d\n", label, name, vals16);
		break;
	case DBUS_TYPE_BYTE:
		dbus_message_iter_get_basic(iter, &byte);
		bt_shell_printf("%s%s: 0x%02x\n", label, name, byte);
		break;
	case DBUS_TYPE_VARIANT:
		dbus_message_iter_recurse(iter, &subiter);
		print_iter(label, name, &subiter);
		break;
	case DBUS_TYPE_ARRAY:
		dbus_message_iter_recurse(iter, &subiter);
		while (dbus_message_iter_get_arg_type(&subiter) !=
							DBUS_TYPE_INVALID) {
			print_iter(label, name, &subiter);
			dbus_message_iter_next(&subiter);
		}
		break;
	case DBUS_TYPE_DICT_ENTRY:
		dbus_message_iter_recurse(iter, &subiter);
		entry = g_strconcat(name, "Key", NULL);
		print_iter(label, entry, &subiter);
		g_free(entry);

		entry = g_strconcat(name, " Value", NULL);
		dbus_message_iter_next(&subiter);
		print_iter(label, entry, &subiter);
		g_free(entry);
		break;
	default:
		bt_shell_printf("%s%s has unsupported type\n", label, name);
		break;
	}
}

static void print_property(GDBusProxy *proxy, const char *name)
{
	DBusMessageIter iter;

	if (g_dbus_proxy_get_property(proxy, name, &iter) == FALSE)
		return;

	print_iter("\t", name, &iter);
}

static void forget_mesh_devices()
{
	g_list_free_full(default_ctrl->mesh_devices, g_free);
	default_ctrl->mesh_devices = NULL;
}

static struct mesh_device *find_device_by_uuid(GList *source, uint8_t uuid[16])
{
	GList *list;

	for (list = g_list_first(source); list; list = g_list_next(list)) {
		struct mesh_device *dev = list->data;

		if (!memcmp(dev->dev_uuid, uuid, 16))
			return dev;
	}

	return NULL;
}

static void print_prov_service(struct prov_svc_data *prov_data)
{
	const char *prefix = "\t\t";
	char txt_uuid[16 * 2 + 1];
	int i;

	bt_shell_printf("%sMesh Provisioning Service (%s)\n", prefix,
							MESH_PROV_SVC_UUID);
	for (i = 0; i < 16; ++i) {
		sprintf(txt_uuid + (i * 2), "%2.2x", prov_data->dev_uuid[i]);
	}

	bt_shell_printf("%s\tDevice UUID: %s\n", prefix, txt_uuid);
	bt_shell_printf("%s\tOOB: %4.4x\n", prefix, prov_data->oob);

}

static bool parse_prov_service_data(const char *uuid, uint8_t *data, int len,
								void *data_out)
{
	struct prov_svc_data *prov_data = data_out;
	int i;

	if (len < 18)
		return false;

	for (i = 0; i < 16; ++i) {
		prov_data->dev_uuid[i] = data[i];
	}

	prov_data->oob = get_be16(&data[16]);

	return true;
}

static bool parse_mesh_service_data(const char *uuid, uint8_t *data, int len,
								void *data_out)
{
	const char *prefix = "\t\t";

	if (!(len == 9 && data[0] == 0x00) && !(len == 17 && data[0] == 0x01)) {
		bt_shell_printf("Unexpected mesh proxy service data length %d\n",
									len);
		return false;
	}

	if (data[0] != connection.type)
		return false;

	if (data[0] == CONN_TYPE_IDENTITY) {
		uint8_t *key;

		if (IS_UNASSIGNED(connection.unicast)) {
			/* This would be a bug */
			bt_shell_printf("Error: Searching identity with "
							"unicast 0000\n");
			return false;
		}

		key = keys_net_key_get(prov_net_key_index, true);
		if (!key)
			return false;

		if (!mesh_crypto_identity_check(key, connection.unicast,
					       &data[1]))
			return false;

		if (discovering) {
			bt_shell_printf("\n%sMesh Proxy Service (%s)\n", prefix,
									uuid);
			bt_shell_printf("%sIdentity for node %4.4x\n", prefix,
							connection.unicast);
		}

	} else if (data[0] == CONN_TYPE_NETWORK) {
		uint16_t net_idx = net_validate_proxy_beacon(data + 1);

		if (net_idx == NET_IDX_INVALID || net_idx != connection.net_idx)
			return false;

		if (discovering) {
			bt_shell_printf("\n%sMesh Proxy Service (%s)\n", prefix,
									uuid);
			bt_shell_printf("%sNetwork Beacon for net index %4.4x\n",
							prefix, net_idx);
		}
	}

	return true;
}

static bool parse_service_data(GDBusProxy *proxy, const char *target_uuid,
					void *data_out)
{
	DBusMessageIter iter, entries;
	bool mesh_prov = false;
	bool mesh_proxy = false;

	if (target_uuid) {
		mesh_prov = !strcmp(target_uuid, MESH_PROV_SVC_UUID);
		mesh_proxy = !strcmp(target_uuid, MESH_PROXY_SVC_UUID);
	}

	if (!g_dbus_proxy_get_property(proxy, "ServiceData", &iter))
		return false;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return false;

	dbus_message_iter_recurse(&iter, &entries);

	while (dbus_message_iter_get_arg_type(&entries)
						== DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter value, entry, array;
		const char *uuid_str;
		bt_uuid_t uuid;
		uint8_t *service_data;
		int len;

		dbus_message_iter_recurse(&entries, &entry);
		dbus_message_iter_get_basic(&entry, &uuid_str);

		if (bt_string_to_uuid(&uuid, uuid_str) < 0)
			goto fail;

		dbus_message_iter_next(&entry);

		if (dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_VARIANT)
			goto fail;

		dbus_message_iter_recurse(&entry, &value);

		if (dbus_message_iter_get_arg_type(&value) != DBUS_TYPE_ARRAY)
			goto fail;

		dbus_message_iter_recurse(&value, &array);

		if (dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_BYTE)
			goto fail;

		dbus_message_iter_get_fixed_array(&array, &service_data, &len);

		if (mesh_prov && !strcmp(uuid_str, MESH_PROV_SVC_UUID)) {
			return parse_prov_service_data(uuid_str, service_data,
								len, data_out);
		} else if (mesh_proxy &&
				!strcmp(uuid_str, MESH_PROXY_SVC_UUID)) {
			return parse_mesh_service_data(uuid_str, service_data,
								len, data_out);
		}

		dbus_message_iter_next(&entries);
	}

	if (!target_uuid)
		return true;
fail:
	return false;
}

static void print_uuids(GDBusProxy *proxy)
{
	DBusMessageIter iter, value;

	if (g_dbus_proxy_get_property(proxy, "UUIDs", &iter) == FALSE)
		return;

	dbus_message_iter_recurse(&iter, &value);

	while (dbus_message_iter_get_arg_type(&value) == DBUS_TYPE_STRING) {
		const char *uuid, *text;

		dbus_message_iter_get_basic(&value, &uuid);

		text = bt_uuidstr_to_str(uuid);
		if (text) {
			char str[26];
			unsigned int n;

			str[sizeof(str) - 1] = '\0';

			n = snprintf(str, sizeof(str), "%s", text);
			if (n > sizeof(str) - 1) {
				str[sizeof(str) - 2] = '.';
				str[sizeof(str) - 3] = '.';
				if (str[sizeof(str) - 4] == ' ')
					str[sizeof(str) - 4] = '.';

				n = sizeof(str) - 1;
			}

			bt_shell_printf("\tUUID: %s%*c(%s)\n",
						str, 26 - n, ' ', uuid);
		} else
			bt_shell_printf("\tUUID: %*c(%s)\n", 26, ' ', uuid);

		dbus_message_iter_next(&value);
	}
}

static gboolean device_is_child(GDBusProxy *device, GDBusProxy *master)
{
	DBusMessageIter iter;
	const char *adapter, *path;

	if (!master)
		return FALSE;

	if (g_dbus_proxy_get_property(device, "Adapter", &iter) == FALSE)
		return FALSE;

	dbus_message_iter_get_basic(&iter, &adapter);
	path = g_dbus_proxy_get_path(master);

	if (!strcmp(path, adapter))
		return TRUE;

	return FALSE;
}

static struct adapter *find_parent(GDBusProxy *device)
{
	GList *list;

	for (list = g_list_first(ctrl_list); list; list = g_list_next(list)) {
		struct adapter *adapter = list->data;

		if (device_is_child(device, adapter->proxy) == TRUE)
			return adapter;
	}
	return NULL;
}

static void set_connected_device(GDBusProxy *proxy)
{
	char *desc = NULL;
	DBusMessageIter iter;
	char buf[10];
	bool mesh;

	connection.device = proxy;

	if (proxy == NULL) {
		memset(&connection, 0, sizeof(connection));
		connection.type = CONN_TYPE_INVALID;
		goto done;
	}

	if (connection.type == CONN_TYPE_IDENTITY) {
		mesh = true;
		snprintf(buf, 10, "Node-%4.4x", connection.unicast);
	} else if (connection.type == CONN_TYPE_NETWORK) {
		mesh = true;
		snprintf(buf, 9, "Net-%4.4x", connection.net_idx);
	} else {
		mesh = false;
	}

	if (!g_dbus_proxy_get_property(proxy, "Alias", &iter) && !mesh)
			goto done;

	dbus_message_iter_get_basic(&iter, &desc);
	desc = g_strdup_printf(COLOR_BLUE "[%s%s%s]" COLOR_OFF "# ", desc,
			       (desc && mesh) ? "-" : "",
				mesh ? buf : "");

done:
	bt_shell_set_prompt(desc ? desc : PROMPT_ON);
	g_free(desc);

	/* If disconnected, return to main menu */
	if (proxy == NULL)
		bt_shell_set_menu(&main_menu);
}

static void connect_reply(DBusMessage *message, void *user_data)
{
	GDBusProxy *proxy = user_data;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to connect: %s\n", error.name);
		dbus_error_free(&error);
		set_connected_device(NULL);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Connection successful\n");

	set_connected_device(proxy);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void update_device_info(GDBusProxy *proxy)
{
	struct adapter *adapter = find_parent(proxy);
	DBusMessageIter iter;
	struct prov_svc_data prov_data;

	if (!adapter) {
		/* TODO: Error */
		return;
	}

	if (adapter != default_ctrl)
		return;

	if (!g_dbus_proxy_get_property(proxy, "Address", &iter))
		return;

	if (parse_service_data(proxy, MESH_PROV_SVC_UUID, &prov_data)) {
		struct mesh_device *dev;

		dev = find_device_by_uuid(adapter->mesh_devices,
							prov_data.dev_uuid);

		/* Display provisioning service once per discovery session */
		if (discovering && (!dev || !dev->hide))
						print_prov_service(&prov_data);

		if (dev) {
			dev->proxy = proxy;
			dev->hide = discovering;
			return;
		}

		dev = g_malloc0(sizeof(struct mesh_device));
		if (!dev)
			return;

		dev->proxy = proxy;
		dev->hide = discovering;

		memcpy(dev->dev_uuid, prov_data.dev_uuid, 16);

		adapter->mesh_devices = g_list_append(adapter->mesh_devices,
							dev);
		print_device(proxy, COLORED_NEW);

		node_create_new(&prov_data);

	} else if (parse_service_data(proxy, MESH_PROXY_SVC_UUID, NULL) &&
								discover_mesh) {
		bool res;

		g_dbus_proxy_method_call(default_ctrl->proxy, "StopDiscovery",
						NULL, NULL, NULL, NULL);
		discover_mesh = false;

		forget_mesh_devices();

		res = g_dbus_proxy_method_call(proxy, "Connect", NULL,
						connect_reply, proxy, NULL);

		if (!res)
			bt_shell_printf("Failed to connect to mesh\n");

		else
			bt_shell_printf("Trying to connect to mesh\n");

	}
}

static void adapter_added(GDBusProxy *proxy)
{
	struct adapter *adapter = g_malloc0(sizeof(struct adapter));

	adapter->proxy = proxy;
	ctrl_list = g_list_append(ctrl_list, adapter);

	if (!default_ctrl)
		default_ctrl = adapter;

	print_adapter(proxy, COLORED_NEW);
}

static void data_out_notify(GDBusProxy *proxy, bool enable,
				GDBusReturnFunction cb)
{
	struct mesh_node *node;

	node = node_find_by_uuid(connection.dev_uuid);

	if (!mesh_gatt_notify(proxy, enable, cb, node))
		bt_shell_printf("Failed to %s notification on %s\n", enable ?
				"start" : "stop", g_dbus_proxy_get_path(proxy));
	else
		bt_shell_printf("%s notification on %s\n", enable ?
			  "Start" : "Stop", g_dbus_proxy_get_path(proxy));
}

struct disconnect_data {
	GDBusReturnFunction cb;
	void *data;
};

static void disconnect(GDBusReturnFunction cb, void *user_data)
{
	GDBusProxy *proxy;
	DBusMessageIter iter;
	const char *addr;

	proxy = connection.device;
	if (!proxy)
		return;

	if (g_dbus_proxy_method_call(proxy, "Disconnect", NULL, cb, user_data,
							NULL) == FALSE) {
		bt_shell_printf("Failed to disconnect\n");
		return;
	}

	if (g_dbus_proxy_get_property(proxy, "Address", &iter) == TRUE)
			dbus_message_iter_get_basic(&iter, &addr);

	bt_shell_printf("Attempting to disconnect from %s\n", addr);
}

static void disc_notify_cb(DBusMessage *message, void *user_data)
{
	struct disconnect_data *disc_data = user_data;

	disconnect(disc_data->cb, disc_data->data);

	g_free(user_data);
}

static void disconnect_device(GDBusReturnFunction cb, void *user_data)
{
	DBusMessageIter iter;

	net_session_close(connection.data_in);

	/* Stop notificiation on prov_out or proxy out characteristics */
	if (connection.data_out) {
		if (g_dbus_proxy_get_property(connection.data_out, "Notifying",
							&iter) == TRUE) {
			struct disconnect_data *disc_data;
			disc_data = g_malloc(sizeof(struct disconnect_data));
			disc_data->cb = cb;
			disc_data->data = user_data;

			if (mesh_gatt_notify(connection.data_out, false,
						disc_notify_cb, disc_data))
				return;
		}
	}

	disconnect(cb, user_data);
}

static void mesh_prov_done(void *user_data, int status);

static void notify_prov_out_cb(DBusMessage *message, void *user_data)
{
	struct mesh_node *node = user_data;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to start notify: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	bt_shell_printf("Notify for Mesh Provisioning Out Data started\n");

	if (connection.type != CONN_TYPE_PROVISION) {
		bt_shell_printf("Error: wrong connection type %d (expected %d)\n",
			connection.type, CONN_TYPE_PROVISION);
		return;
	}

	if (!connection.data_in) {
		bt_shell_printf("Error: don't have mesh provisioning data in\n");
		return;
	}

	if (!node) {
		bt_shell_printf("Error: provisioning node not present\n");
		return;
	}

	if(!prov_open(node, connection.data_in, prov_net_key_index,
			mesh_prov_done, node))
	{
		bt_shell_printf("Failed to start provisioning\n");
		node_free(node);
		disconnect_device(NULL, NULL);
	} else
		bt_shell_printf("Initiated provisioning\n");

}

static void session_open_cb (int status)
{
	if (status) {
		bt_shell_printf("Failed to open Mesh session\n");
		disconnect_device(NULL, NULL);
		return;
	}

	bt_shell_printf("Mesh session is open\n");

	/* Get composition data for a newly provisioned node */
	if (connection.type == CONN_TYPE_IDENTITY)
		config_client_get_composition(connection.unicast);
}

static void notify_proxy_out_cb(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to start notify: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	bt_shell_printf("Notify for Mesh Proxy Out Data started\n");

	if (connection.type != CONN_TYPE_IDENTITY &&
			connection.type != CONN_TYPE_NETWORK) {
		bt_shell_printf("Error: wrong connection type %d "
				"(expected %d or %d)\n", connection.type,
				CONN_TYPE_IDENTITY, CONN_TYPE_NETWORK);
		return;
	}

	if (!connection.data_in) {
		bt_shell_printf("Error: don't have mesh proxy data in\n");
		return;
	}

	bt_shell_printf("Trying to open mesh session\n");
	net_session_open(connection.data_in, true, session_open_cb);
	connection.session_open = true;
}

static GDBusProxy *get_characteristic(GDBusProxy *device, const char *char_uuid)
{
	GList *l;
	GDBusProxy *service;
	const char *svc_uuid;

	if (connection.type == CONN_TYPE_PROVISION) {
		svc_uuid = MESH_PROV_SVC_UUID;
	} else {
		svc_uuid = MESH_PROXY_SVC_UUID;
	}
	for (l = service_list; l; l = l->next) {
		if (mesh_gatt_is_child(l->data, device, "Device") &&
					service_is_mesh(l->data, svc_uuid))
			break;
	}

	if (l)
		service = l->data;
	else {
		bt_shell_printf("Mesh service not found\n");
		return	NULL;
	}

	for (l = char_list; l; l = l->next) {
		if (mesh_gatt_is_child(l->data, service, "Service") &&
					char_is_mesh(l->data, char_uuid)) {
			bt_shell_printf("Found matching char: path %s,"
					" uuid %s\n",
					g_dbus_proxy_get_path(l->data),
					char_uuid);
			return l->data;
		}
	}
	return NULL;
}

static void mesh_session_setup(GDBusProxy *proxy)
{
	if (connection.type == CONN_TYPE_PROVISION) {
		connection.data_in = get_characteristic(proxy,
						MESH_PROV_DATA_IN_UUID_STR);
		if (!connection.data_in)
			goto fail;

		connection.data_out = get_characteristic(proxy,
						MESH_PROV_DATA_OUT_UUID_STR);
		if (!connection.data_out)
			goto fail;

		data_out_notify(connection.data_out, true, notify_prov_out_cb);

	} else if (connection.type != CONN_TYPE_INVALID){
		connection.data_in = get_characteristic(proxy,
						MESH_PROXY_DATA_IN_UUID_STR);
		if (!connection.data_in)
			goto fail;

		connection.data_out = get_characteristic(proxy,
						MESH_PROXY_DATA_OUT_UUID_STR);
		if (!connection.data_out)
			goto fail;

		data_out_notify(connection.data_out, true, notify_proxy_out_cb);
	}

	return;

fail:

	bt_shell_printf("Services resolved, mesh characteristics not found\n");
}

static void proxy_added(GDBusProxy *proxy, void *user_data)
{
	const char *interface;

	interface = g_dbus_proxy_get_interface(proxy);

	if (!strcmp(interface, "org.bluez.Device1")) {
		update_device_info(proxy);

	} else if (!strcmp(interface, "org.bluez.Adapter1")) {

		adapter_added(proxy);

	} else if (!strcmp(interface, "org.bluez.GattService1") &&
						service_is_mesh(proxy, NULL)) {

		bt_shell_printf("Service added %s\n",
				g_dbus_proxy_get_path(proxy));
		service_list = g_list_append(service_list, proxy);

	} else if (!strcmp(interface, "org.bluez.GattCharacteristic1") &&
						char_is_mesh(proxy, NULL)) {

		bt_shell_printf("Char added %s:\n",
				g_dbus_proxy_get_path(proxy));

		char_list = g_list_append(char_list, proxy);
	}
}

static void start_discovery_reply(DBusMessage *message, void *user_data)
{
	dbus_bool_t enable = GPOINTER_TO_UINT(user_data);
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to %s discovery: %s\n",
				enable == TRUE ? "start" : "stop", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Discovery %s\n",
			enable == TRUE ? "started" : "stopped");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static struct mesh_device *find_device_by_proxy(GList *source,
							GDBusProxy *proxy)
{
	GList *list;

	for (list = g_list_first(source); list; list = g_list_next(list)) {
		struct mesh_device *dev = list->data;
		GDBusProxy *proxy = dev->proxy;

		if (dev->proxy == proxy)
			return dev;
	}

	return NULL;
}

static void device_removed(GDBusProxy *proxy)
{
	struct adapter *adapter = find_parent(proxy);
	struct mesh_device *dev;

	if (!adapter) {
		/* TODO: Error */
		return;
	}

	dev = find_device_by_proxy(adapter->mesh_devices, proxy);
	if (dev)
		adapter->mesh_devices = g_list_remove(adapter->mesh_devices,
									dev);

	print_device(proxy, COLORED_DEL);

	if (connection.device == proxy)
		set_connected_device(NULL);

}

static void adapter_removed(GDBusProxy *proxy)
{
	GList *ll;
	for (ll = g_list_first(ctrl_list); ll; ll = g_list_next(ll)) {
		struct adapter *adapter = ll->data;

		if (adapter->proxy == proxy) {
			print_adapter(proxy, COLORED_DEL);

			if (default_ctrl && default_ctrl->proxy == proxy) {
				default_ctrl = NULL;
				set_connected_device(NULL);
			}

			ctrl_list = g_list_remove_link(ctrl_list, ll);

			g_list_free_full(adapter->mesh_devices, g_free);
			g_free(adapter);
			g_list_free(ll);
			return;
		}
	}
}

static void proxy_removed(GDBusProxy *proxy, void *user_data)
{
	const char *interface;

	interface = g_dbus_proxy_get_interface(proxy);

	if (!strcmp(interface, "org.bluez.Device1")) {
		device_removed(proxy);
	} else if (!strcmp(interface, "org.bluez.Adapter1")) {
		adapter_removed(proxy);
	} else if (!strcmp(interface, "org.bluez.GattService1")) {
		if (proxy == connection.service) {
			if (service_is_mesh(proxy, MESH_PROXY_SVC_UUID)) {
				data_out_notify(connection.data_out,
								false, NULL);
				net_session_close(connection.data_in);
			}
			connection.service = NULL;
			connection.data_in = NULL;
			connection.data_out = NULL;
		}

		service_list = g_list_remove(service_list, proxy);

	} else if (!strcmp(interface, "org.bluez.GattCharacteristic1")) {
		char_list = g_list_remove(char_list, proxy);
	}
}

static int get_characteristic_value(DBusMessageIter *value, uint8_t *buf)
{
	DBusMessageIter array;
	uint8_t *data;
	int len;

	if (dbus_message_iter_get_arg_type(value) != DBUS_TYPE_ARRAY)
		return 0;

	dbus_message_iter_recurse(value, &array);

	if (dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_BYTE)
		return 0;

	dbus_message_iter_get_fixed_array(&array, &data, &len);
	memcpy(buf, data, len);

	return len;
}

static bool process_mesh_characteristic(GDBusProxy *proxy)
{
	DBusMessageIter iter;
	const char *uuid;
	uint8_t *res;
	uint8_t buf[256];
	bool is_prov;

	if (g_dbus_proxy_get_property(proxy, "UUID", &iter) == FALSE)
		return false;

	dbus_message_iter_get_basic(&iter, &uuid);

	if (g_dbus_proxy_get_property(proxy, "Value", &iter) == FALSE)
		return false;

	is_prov = !bt_uuid_strcmp(uuid, MESH_PROV_DATA_OUT_UUID_STR);

	if (is_prov || !bt_uuid_strcmp(uuid, MESH_PROXY_DATA_OUT_UUID_STR))
	{
		struct mesh_node *node;
		uint16_t len;

		len = get_characteristic_value(&iter, buf);

		if (!len || len > 69)
			return false;

		res = buf;
		len = mesh_gatt_sar(&res, len);

		if (!len)
			return false;

		if (is_prov) {
			node = node_find_by_uuid(connection.dev_uuid);

			if (!node) {
				bt_shell_printf("Node not found?\n");
				return false;
			}

			return prov_data_ready(node, res, len);
		}

		return net_data_ready(res, len);
	}

	return false;
}


static void property_changed(GDBusProxy *proxy, const char *name,
					DBusMessageIter *iter, void *user_data)
{
	const char *interface;

	interface = g_dbus_proxy_get_interface(proxy);

	if (!strcmp(interface, "org.bluez.Device1")) {

		if (default_ctrl && device_is_child(proxy,
					default_ctrl->proxy) == TRUE) {

			if (strcmp(name, "Connected") == 0) {
				dbus_bool_t connected;
				dbus_message_iter_get_basic(iter, &connected);

				if (connected && connection.device == NULL)
					set_connected_device(proxy);
				else if (!connected &&
						connection.device == proxy) {
					net_session_close(connection.data_in);
					set_connected_device(NULL);
				}
			} else if ((strcmp(name, "Alias") == 0) &&
						connection.device == proxy) {
				/* Re-generate prompt */
				set_connected_device(proxy);
			} else if (!strcmp(name, "ServiceData")) {
				update_device_info(proxy);
			} else if (!strcmp(name, "ServicesResolved")) {
				gboolean resolved;

				dbus_message_iter_get_basic(iter, &resolved);

				bt_shell_printf("Services resolved %s\n",
						resolved ? "yes" : "no");

				if (resolved)
					mesh_session_setup(connection.device);
			}

		}
	} else if (!strcmp(interface, "org.bluez.Adapter1")) {
		DBusMessageIter addr_iter;
		char *str;

		bt_shell_printf("Adapter property changed \n");
		if (g_dbus_proxy_get_property(proxy, "Address",
						&addr_iter) == TRUE) {
			const char *address;

			dbus_message_iter_get_basic(&addr_iter, &address);
			str = g_strdup_printf("[" COLORED_CHG
						"] Controller %s ", address);
		} else
			str = g_strdup("");

		if (strcmp(name, "Discovering") == 0) {
			int temp;

			dbus_message_iter_get_basic(iter, &temp);
			discovering = !!temp;
		}

		print_iter(str, name, iter);
		g_free(str);
	} else if (!strcmp(interface, "org.bluez.GattService1")) {
		bt_shell_printf("Service property changed %s\n",
						g_dbus_proxy_get_path(proxy));
	} else if (!strcmp(interface, "org.bluez.GattCharacteristic1")) {
		bt_shell_printf("Characteristic property changed %s\n",
						g_dbus_proxy_get_path(proxy));

		if ((strcmp(name, "Value") == 0) &&
				((connection.type == CONN_TYPE_PROVISION) ||
						connection.session_open))
			process_mesh_characteristic(proxy);
	}
}

static void message_handler(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	bt_shell_printf("[SIGNAL] %s.%s\n", dbus_message_get_interface(message),
					dbus_message_get_member(message));
}

static struct adapter *find_ctrl_by_address(GList *source, const char *address)
{
	GList *list;

	for (list = g_list_first(source); list; list = g_list_next(list)) {
		struct adapter *adapter = list->data;
		DBusMessageIter iter;
		const char *str;

		if (g_dbus_proxy_get_property(adapter->proxy,
					"Address", &iter) == FALSE)
			continue;

		dbus_message_iter_get_basic(&iter, &str);

		if (!strcmp(str, address))
			return adapter;
	}

	return NULL;
}

static gboolean parse_argument_on_off(int argc, char *argv[],
					dbus_bool_t *value)
{
	if (!strcmp(argv[1], "on") || !strcmp(argv[1], "yes")) {
		*value = TRUE;
		return TRUE;
	}

	if (!strcmp(argv[1], "off") || !strcmp(argv[1], "no")) {
		*value = FALSE;
		return TRUE;
	}

	bt_shell_printf("Invalid argument %s\n", argv[1]);
	return FALSE;
}

static void cmd_list(int argc, char *argv[])
{
	GList *list;

	for (list = g_list_first(ctrl_list); list; list = g_list_next(list)) {
		struct adapter *adapter = list->data;
		print_adapter(adapter->proxy, NULL);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_show(int argc, char *argv[])
{
	struct adapter *adapter;
	GDBusProxy *proxy;
	DBusMessageIter iter;
	const char *address;


	if (argc < 2 || !strlen(argv[1])) {
		if (check_default_ctrl() == FALSE)
			return bt_shell_noninteractive_quit(EXIT_FAILURE);

		proxy = default_ctrl->proxy;
	} else {
		adapter = find_ctrl_by_address(ctrl_list, argv[1]);
		if (!adapter) {
			bt_shell_printf("Controller %s not available\n",
								argv[1]);
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
		proxy = adapter->proxy;
	}

	if (g_dbus_proxy_get_property(proxy, "Address", &iter) == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	dbus_message_iter_get_basic(&iter, &address);
	bt_shell_printf("Controller %s\n", address);

	print_property(proxy, "Name");
	print_property(proxy, "Alias");
	print_property(proxy, "Class");
	print_property(proxy, "Powered");
	print_property(proxy, "Discoverable");
	print_uuids(proxy);
	print_property(proxy, "Modalias");
	print_property(proxy, "Discovering");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_select(int argc, char *argv[])
{
	struct adapter *adapter;

	adapter = find_ctrl_by_address(ctrl_list, argv[1]);
	if (!adapter) {
		bt_shell_printf("Controller %s not available\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (default_ctrl && default_ctrl->proxy == adapter->proxy)
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);

	forget_mesh_devices();

	default_ctrl = adapter;
	print_adapter(adapter->proxy, NULL);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void generic_callback(const DBusError *error, void *user_data)
{
	char *str = user_data;

	if (dbus_error_is_set(error)) {
		bt_shell_printf("Failed to set %s: %s\n", str, error->name);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	} else {
		bt_shell_printf("Changing %s succeeded\n", str);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}
}

static void cmd_power(int argc, char *argv[])
{
	dbus_bool_t powered;
	char *str;

	if (parse_argument_on_off(argc, argv, &powered) == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	str = g_strdup_printf("power %s", powered == TRUE ? "on" : "off");

	if (g_dbus_proxy_set_property_basic(default_ctrl->proxy, "Powered",
					DBUS_TYPE_BOOLEAN, &powered,
					generic_callback, str, g_free) == TRUE)
		return;

	g_free(str);

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

#define	DISTANCE_VAL_INVALID	0x7FFF

struct set_discovery_filter_args {
	char *transport;
	dbus_uint16_t rssi;
	dbus_int16_t pathloss;
	char **uuids;
	size_t uuids_len;
	dbus_bool_t duplicate;
};

static void set_discovery_filter_setup(DBusMessageIter *iter, void *user_data)
{
	struct set_discovery_filter_args *args = user_data;
	DBusMessageIter dict;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
				DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
				DBUS_TYPE_STRING_AS_STRING
				DBUS_TYPE_VARIANT_AS_STRING
				DBUS_DICT_ENTRY_END_CHAR_AS_STRING, &dict);

	g_dbus_dict_append_array(&dict, "UUIDs", DBUS_TYPE_STRING,
							&args->uuids,
							args->uuids_len);

	if (args->pathloss != DISTANCE_VAL_INVALID)
		g_dbus_dict_append_entry(&dict, "Pathloss", DBUS_TYPE_UINT16,
						&args->pathloss);

	if (args->rssi != DISTANCE_VAL_INVALID)
		g_dbus_dict_append_entry(&dict, "RSSI", DBUS_TYPE_INT16,
						&args->rssi);

	if (args->transport != NULL)
		g_dbus_dict_append_entry(&dict, "Transport", DBUS_TYPE_STRING,
						&args->transport);
	if (args->duplicate)
		g_dbus_dict_append_entry(&dict, "DuplicateData",
						DBUS_TYPE_BOOLEAN,
						&args->duplicate);

	dbus_message_iter_close_container(iter, &dict);
}


static void set_discovery_filter_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);
	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("SetDiscoveryFilter failed: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("SetDiscoveryFilter success\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static gint filtered_scan_rssi = DISTANCE_VAL_INVALID;
static gint filtered_scan_pathloss = DISTANCE_VAL_INVALID;
static char **filtered_scan_uuids;
static size_t filtered_scan_uuids_len;
static char *filtered_scan_transport = "le";

static void set_scan_filter_commit(void)
{
	struct set_discovery_filter_args args;

	args.pathloss = filtered_scan_pathloss;
	args.rssi = filtered_scan_rssi;
	args.transport = filtered_scan_transport;
	args.uuids = filtered_scan_uuids;
	args.uuids_len = filtered_scan_uuids_len;
	args.duplicate = TRUE;

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (g_dbus_proxy_method_call(default_ctrl->proxy, "SetDiscoveryFilter",
		set_discovery_filter_setup, set_discovery_filter_reply,
		&args, NULL) == FALSE) {
		bt_shell_printf("Failed to set discovery filter\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void set_scan_filter_uuids(char *filters[])
{
	g_strfreev(filtered_scan_uuids);
	filtered_scan_uuids = NULL;
	filtered_scan_uuids_len = 0;

	if (!filters)
		goto commit;

	filtered_scan_uuids = g_strdupv(filters);
	if (!filtered_scan_uuids) {
		bt_shell_printf("Failed to parse input\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	filtered_scan_uuids_len = g_strv_length(filtered_scan_uuids);

commit:
	set_scan_filter_commit();
}

static void cmd_scan_unprovisioned(int argc, char *argv[])
{
	dbus_bool_t enable;
	char *filters[] = { MESH_PROV_SVC_UUID, NULL };
	const char *method;

	if (parse_argument_on_off(argc, argv, &enable) == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (enable == TRUE) {
		discover_mesh = false;
		set_scan_filter_uuids(filters);
		method = "StartDiscovery";
	} else {
		method = "StopDiscovery";
	}

	if (g_dbus_proxy_method_call(default_ctrl->proxy, method,
				NULL, start_discovery_reply,
				GUINT_TO_POINTER(enable), NULL) == FALSE) {
		bt_shell_printf("Failed to %s discovery\n",
					enable == TRUE ? "start" : "stop");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_info(int argc, char *argv[])
{
	GDBusProxy *proxy;
	DBusMessageIter iter;
	const char *address;

	proxy = connection.device;
	if (!proxy)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (g_dbus_proxy_get_property(proxy, "Address", &iter) == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	dbus_message_iter_get_basic(&iter, &address);
	bt_shell_printf("Device %s\n", address);

	print_property(proxy, "Name");
	print_property(proxy, "Alias");
	print_property(proxy, "Class");
	print_property(proxy, "Appearance");
	print_property(proxy, "Icon");
	print_property(proxy, "Trusted");
	print_property(proxy, "Blocked");
	print_property(proxy, "Connected");
	print_uuids(proxy);
	print_property(proxy, "Modalias");
	print_property(proxy, "ManufacturerData");
	print_property(proxy, "ServiceData");
	print_property(proxy, "RSSI");
	print_property(proxy, "TxPower");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static const char *security2str(uint8_t level)
{
	switch (level) {
	case 0:
		return "low";
	case 1:
		return "medium";
	case 2:
		return "high";
	default:
		return "invalid";
	}
}

static void cmd_security(int argc, char *argv[])
{
	uint8_t level;
	char *end;

	if (argc == 1)
		goto done;

	level = strtol(argv[1], &end, 10);
	if (end == argv[1] || !prov_set_sec_level(level)) {
		bt_shell_printf("Invalid security level %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

done:
	bt_shell_printf("Provision Security Level set to %u (%s)\n",
			prov_get_sec_level(),
			security2str(prov_get_sec_level()));

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_connect(int argc, char *argv[])
{
	char *filters[] = { MESH_PROXY_SVC_UUID, NULL };

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	memset(&connection, 0, sizeof(connection));

	if (argc < 2 || !strlen(argv[1])) {
		connection.net_idx = NET_IDX_PRIMARY;
	} else {
		char *end;
		connection.net_idx = strtol(argv[1], &end, 16);
		if (end == argv[1]) {
			connection.net_idx = NET_IDX_INVALID;
			bt_shell_printf("Invalid network index %s\n", argv[1]);
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}

		if (argc > 2)
			connection.unicast = strtol(argv[2], NULL, 16);
	}

	if (discovering)
		g_dbus_proxy_method_call(default_ctrl->proxy, "StopDiscovery",
						NULL, NULL, NULL, NULL);

	set_scan_filter_uuids(filters);
	discover_mesh = true;

	if (connection.unicast == UNASSIGNED_ADDRESS) {
		connection.type = CONN_TYPE_NETWORK;
		bt_shell_printf("Looking for mesh network with net index "
				"%4.4x\n", connection.net_idx);
	} else {
		connection.type = CONN_TYPE_IDENTITY;
		bt_shell_printf("Looking for node id %4.4x"
				" on network with net index %4.4x\n",
				connection.unicast, connection.net_idx);
	}

	if (g_dbus_proxy_method_call(default_ctrl->proxy,
			"StartDiscovery", NULL, start_discovery_reply,
				GUINT_TO_POINTER(TRUE), NULL) == FALSE) {
		bt_shell_printf("Failed to start mesh proxy discovery\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	g_dbus_proxy_method_call(default_ctrl->proxy, "StartDiscovery",
						NULL, NULL, NULL, NULL);

}

static void prov_disconn_reply(DBusMessage *message, void *user_data)
{
	struct mesh_node *node = user_data;
	char *filters[] = { MESH_PROXY_SVC_UUID, NULL };
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to disconnect: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	set_connected_device(NULL);

	set_scan_filter_uuids(filters);
	discover_mesh = true;

	connection.type = CONN_TYPE_IDENTITY;
	connection.data_in = NULL;
	connection.data_out = NULL;
	connection.unicast = node_get_primary(node);

	if (g_dbus_proxy_method_call(default_ctrl->proxy,
			"StartDiscovery", NULL, start_discovery_reply,
				GUINT_TO_POINTER(TRUE), NULL) == FALSE) {
		bt_shell_printf("Failed to start mesh proxy discovery\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

}

static void disconn_reply(DBusMessage *message, void *user_data)
{
	GDBusProxy *proxy = user_data;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to disconnect: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Successfully disconnected\n");

	if (proxy != connection.device)
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);

	set_connected_device(NULL);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_disconn(int argc, char *argv[])
{
	if (connection.type == CONN_TYPE_PROVISION) {
		struct mesh_node *node = node_find_by_uuid(connection.dev_uuid);
		if (node)
			node_free(node);
	}

	disconnect_device(disconn_reply, connection.device);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void mesh_prov_done(void *user_data, int status)
{
	struct mesh_node *node = user_data;

	if (status){
		bt_shell_printf("Provisioning failed\n");
		node_free(node);
		disconnect_device(NULL, NULL);
		return;
	}

	bt_shell_printf("Provision success. Assigned Primary Unicast %4.4x\n",
						node_get_primary(node));

	if (!prov_db_add_new_node(node))
		bt_shell_printf("Failed to add node to provisioning DB\n");

	disconnect_device(prov_disconn_reply, node);
}

static void cmd_start_prov(int argc, char *argv[])
{
	GDBusProxy *proxy;
	struct mesh_device *dev;
	struct mesh_node *node;
	int len;

	len = strlen(argv[1]);
	if ( len > 32 || len % 2) {
		bt_shell_printf("Incorrect UUID size %d\n", len);
	}

	disconnect_device(NULL, NULL);

	memset(connection.dev_uuid, 0, 16);
	str2hex(argv[1], len, connection.dev_uuid, len/2);

	node = node_find_by_uuid(connection.dev_uuid);
	if (!node) {
		bt_shell_printf("Device with UUID %s not found.\n", argv[1]);
		bt_shell_printf("Stale services? Remove device and "
						"re-discover\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	/* TODO: add command to remove a node from mesh, i.e., "unprovision" */
	if (node_is_provisioned(node)) {
		bt_shell_printf("Already provisioned with unicast %4.4x\n",
				node_get_primary(node));
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	dev = find_device_by_uuid(default_ctrl->mesh_devices,
				  connection.dev_uuid);
	if (!dev || !dev->proxy) {
		bt_shell_printf("Could not find device proxy\n");
		memset(connection.dev_uuid, 0, 16);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	proxy = dev->proxy;
	if (discovering)
		g_dbus_proxy_method_call(default_ctrl->proxy, "StopDiscovery",
						NULL, NULL, NULL, NULL);
	forget_mesh_devices();

	connection.type = CONN_TYPE_PROVISION;

	if (g_dbus_proxy_method_call(proxy, "Connect", NULL, connect_reply,
							proxy, NULL) == FALSE) {
		bt_shell_printf("Failed to connect ");
		print_device(proxy, NULL);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	} else {
		bt_shell_printf("Trying to connect ");
		print_device(proxy, NULL);
	}

}

static void cmd_print_mesh(int argc, char *argv[])
{
	if (!prov_db_show(mesh_prov_db_filename)) {
		bt_shell_printf("Unavailable\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

 static void cmd_print_local(int argc, char *argv[])
{
	if (!prov_db_show(mesh_local_config_filename)) {
		bt_shell_printf("Unavailable\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static const struct bt_shell_menu main_menu = {
	.name = "main",
	.entries = {
	{ "list",         NULL,       cmd_list, "List available controllers"},
	{ "show",         "[ctrl]",   cmd_show, "Controller information"},
	{ "select",       "<ctrl>",   cmd_select, "Select default controller"},
	{ "security",     "[0(low)/1(medium)/2(high)]", cmd_security,
				"Display or change provision security level"},
	{ "info",         "[dev]",    cmd_info, "Device information"},
	{ "connect",      "[net_idx] [dst]", cmd_connect,
				"Connect to mesh network or node on network"},
	{ "discover-unprovisioned", "<on/off>", cmd_scan_unprovisioned,
					"Look for devices to provision" },
	{ "provision",    "<uuid>",   cmd_start_prov, "Initiate provisioning"},
	{ "power",        "<on/off>", cmd_power, "Set controller power" },
	{ "disconnect",   "[dev]",    cmd_disconn, "Disconnect device"},
	{ "mesh-info",    NULL,       cmd_print_mesh,
					"Mesh networkinfo (provisioner)" },
	{ "local-info",    NULL,      cmd_print_local, "Local mesh node info" },
	{ } },
};

static const char *config_dir;

static const struct option options[] = {
	{ "config",	required_argument, 0, 'c' },
	{ 0, 0, 0, 0 }
};

static const char **optargs[] = {
	&config_dir
};

static const char *help[] = {
	"Read local mesh config JSON files from <directory>"
};

static const struct bt_shell_opt opt = {
	.options = options,
	.optno = sizeof(options) / sizeof(struct option),
	.optstr = "c:",
	.optarg = optargs,
	.help = help,
};

static void client_ready(GDBusClient *client, void *user_data)
{
	bt_shell_attach(fileno(stdin));
}

int main(int argc, char *argv[])
{
	GDBusClient *client;
	int status;
	int len;
	int extra;
	char *mesh_dir = NULL;

	bt_shell_init(argc, argv, &opt);
	bt_shell_set_menu(&main_menu);
	bt_shell_set_prompt(PROMPT_OFF);

	if (!config_dir) {
		char *home;

		home = getenv("XDG_CONFIG_HOME");
		if (home) {
			mesh_dir = g_strdup_printf("%s/meshctl", home);
		}

		if (!mesh_dir) {
			home = getenv("HOME");
			if (home)
				mesh_dir = g_strdup_printf("%s/.config/meshctl",
									home);
		}

		if (!mesh_dir) {
			g_printerr("Configuration directory not found\n");
			goto fail;
		}

	} else {
		mesh_dir = g_strdup_printf("%s", config_dir);
	}


	g_print("Reading prov_db.json and local_node.json from %s directory\n",
								mesh_dir);

	len = strlen(mesh_dir);

	if (len && mesh_dir[len - 1] != '/')
		extra = 1;
	else
		extra = 0;

	mesh_local_config_filename = g_malloc(len + strlen("local_node.json")
									+ 2);
	if (!mesh_local_config_filename)
		goto fail;

	mesh_prov_db_filename = g_malloc(len + strlen("prov_db.json") + 2);
	if (!mesh_prov_db_filename)
		goto fail;

	sprintf(mesh_local_config_filename, "%s", mesh_dir);

	if (extra)
		sprintf(mesh_local_config_filename + len , "%c", '/');

	sprintf(mesh_local_config_filename + len + extra, "%s",
							"local_node.json");
	len = len + extra + strlen("local_node.json");

	if (!prov_db_read_local_node(mesh_local_config_filename, true)) {
		g_printerr("Failed to parse local node configuration file %s\n",
			mesh_local_config_filename);
		goto fail;
	}

	sprintf(mesh_prov_db_filename, "%s", mesh_dir);
	len = strlen(mesh_dir);

	g_free(mesh_dir);

	if (extra)
		sprintf(mesh_prov_db_filename + len , "%c", '/');

	sprintf(mesh_prov_db_filename + len + extra, "%s", "prov_db.json");

	if (!prov_db_read(mesh_prov_db_filename)) {
		g_printerr("Failed to parse provisioning database file %s\n",
			mesh_prov_db_filename);
		goto fail;
	}

	dbus_conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, NULL);
	client = g_dbus_client_new(dbus_conn, "org.bluez", "/org/bluez");

	g_dbus_client_set_connect_watch(client, connect_handler, NULL);
	g_dbus_client_set_disconnect_watch(client, disconnect_handler, NULL);
	g_dbus_client_set_signal_watch(client, message_handler, NULL);

	g_dbus_client_set_proxy_handlers(client, proxy_added, proxy_removed,
							property_changed, NULL);

	g_dbus_client_set_ready_watch(client, client_ready, NULL);

	if (!config_client_init())
		g_printerr("Failed to initialize mesh configuration client\n");

	if (!config_server_init())
		g_printerr("Failed to initialize mesh configuration server\n");

	if (!onoff_client_init(PRIMARY_ELEMENT_IDX))
		g_printerr("Failed to initialize mesh generic On/Off client\n");

	status = bt_shell_run();

	g_dbus_client_unref(client);

	dbus_connection_unref(dbus_conn);

	node_cleanup();

	g_list_free(char_list);
	g_list_free(service_list);
	g_list_free_full(ctrl_list, proxy_leak);

	return status;

fail:
	bt_shell_cleanup();
	g_free(mesh_dir);

	return EXIT_FAILURE;
}
