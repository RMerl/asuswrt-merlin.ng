// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
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

#include <glib.h>

#include "src/shared/shell.h"
#include "src/shared/util.h"
#include "gdbus/gdbus.h"
#include "agent.h"
#include "gatt.h"
#include "advertising.h"
#include "adv_monitor.h"

/* String display constants */
#define COLORED_NEW	COLOR_GREEN "NEW" COLOR_OFF
#define COLORED_CHG	COLOR_YELLOW "CHG" COLOR_OFF
#define COLORED_DEL	COLOR_RED "DEL" COLOR_OFF

#define PROMPT_ON	COLOR_BLUE "[bluetooth]" COLOR_OFF "# "
#define PROMPT_OFF	"Waiting to connect to bluetoothd..."

static DBusConnection *dbus_conn;

static GDBusProxy *agent_manager;
static char *auto_register_agent = NULL;

struct adapter {
	GDBusProxy *proxy;
	GDBusProxy *ad_proxy;
	GDBusProxy *adv_monitor_proxy;
	GList *devices;
};

static struct adapter *default_ctrl;
static GDBusProxy *default_dev;
static GDBusProxy *default_attr;
static GList *ctrl_list;
static GList *battery_proxies;

static const char *agent_arguments[] = {
	"on",
	"off",
	"DisplayOnly",
	"DisplayYesNo",
	"KeyboardDisplay",
	"KeyboardOnly",
	"NoInputNoOutput",
	NULL
};

static const char *ad_arguments[] = {
	"on",
	"off",
	"peripheral",
	"broadcast",
	NULL
};

static void proxy_leak(gpointer data)
{
	printf("Leaking proxy %p\n", data);
}

static void setup_standard_input(void)
{
	bt_shell_attach(fileno(stdin));
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
	g_list_free_full(battery_proxies, proxy_leak);
	ctrl_list = NULL;
	battery_proxies = NULL;

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

static void print_fixed_iter(const char *label, const char *name,
						DBusMessageIter *iter)
{
	dbus_bool_t *valbool;
	dbus_uint32_t *valu32;
	dbus_uint16_t *valu16;
	dbus_int16_t *vals16;
	unsigned char *byte;
	int len;

	switch (dbus_message_iter_get_arg_type(iter)) {
	case DBUS_TYPE_BOOLEAN:
		dbus_message_iter_get_fixed_array(iter, &valbool, &len);

		if (len <= 0)
			return;

		bt_shell_printf("%s%s:\n", label, name);
		bt_shell_hexdump((void *)valbool, len * sizeof(*valbool));

		break;
	case DBUS_TYPE_UINT32:
		dbus_message_iter_get_fixed_array(iter, &valu32, &len);

		if (len <= 0)
			return;

		bt_shell_printf("%s%s:\n", label, name);
		bt_shell_hexdump((void *)valu32, len * sizeof(*valu32));

		break;
	case DBUS_TYPE_UINT16:
		dbus_message_iter_get_fixed_array(iter, &valu16, &len);

		if (len <= 0)
			return;

		bt_shell_printf("%s%s:\n", label, name);
		bt_shell_hexdump((void *)valu16, len * sizeof(*valu16));

		break;
	case DBUS_TYPE_INT16:
		dbus_message_iter_get_fixed_array(iter, &vals16, &len);

		if (len <= 0)
			return;

		bt_shell_printf("%s%s:\n", label, name);
		bt_shell_hexdump((void *)vals16, len * sizeof(*vals16));

		break;
	case DBUS_TYPE_BYTE:
		dbus_message_iter_get_fixed_array(iter, &byte, &len);

		if (len <= 0)
			return;

		bt_shell_printf("%s%s:\n", label, name);
		bt_shell_hexdump((void *)byte, len * sizeof(*byte));

		break;
	default:
		return;
	};
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
		bt_shell_printf("%s%s: 0x%08x\n", label, name, valu32);
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
		bt_shell_printf("%s%s: 0x%02x (%d)\n", label, name, byte, byte);
		break;
	case DBUS_TYPE_VARIANT:
		dbus_message_iter_recurse(iter, &subiter);
		print_iter(label, name, &subiter);
		break;
	case DBUS_TYPE_ARRAY:
		dbus_message_iter_recurse(iter, &subiter);

		if (dbus_type_is_fixed(
				dbus_message_iter_get_arg_type(&subiter))) {
			print_fixed_iter(label, name, &subiter);
			break;
		}

		while (dbus_message_iter_get_arg_type(&subiter) !=
							DBUS_TYPE_INVALID) {
			print_iter(label, name, &subiter);
			dbus_message_iter_next(&subiter);
		}
		break;
	case DBUS_TYPE_DICT_ENTRY:
		dbus_message_iter_recurse(iter, &subiter);
		entry = g_strconcat(name, " Key", NULL);
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

static void print_property_with_label(GDBusProxy *proxy, const char *name,
					const char *label)
{
	DBusMessageIter iter;

	if (g_dbus_proxy_get_property(proxy, name, &iter) == FALSE)
		return;

	print_iter("\t", label ? label : name, &iter);
}

static void print_property(GDBusProxy *proxy, const char *name)
{
	print_property_with_label(proxy, name, NULL);
}

static void print_uuid(const char *uuid)
{
	const char *text;

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

		bt_shell_printf("\tUUID: %s%*c(%s)\n", str, 26 - n, ' ', uuid);
	} else
		bt_shell_printf("\tUUID: %*c(%s)\n", 26, ' ', uuid);
}

static void print_uuids(GDBusProxy *proxy)
{
	DBusMessageIter iter, value;

	if (g_dbus_proxy_get_property(proxy, "UUIDs", &iter) == FALSE)
		return;

	dbus_message_iter_recurse(&iter, &value);

	while (dbus_message_iter_get_arg_type(&value) == DBUS_TYPE_STRING) {
		const char *uuid;

		dbus_message_iter_get_basic(&value, &uuid);

		print_uuid(uuid);

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

static gboolean service_is_child(GDBusProxy *service)
{
	DBusMessageIter iter;
	const char *device;

	if (g_dbus_proxy_get_property(service, "Device", &iter) == FALSE)
		return FALSE;

	dbus_message_iter_get_basic(&iter, &device);

	if (!default_ctrl)
		return FALSE;

	return g_dbus_proxy_lookup(default_ctrl->devices, NULL, device,
					"org.bluez.Device1") != NULL;
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

static void set_default_device(GDBusProxy *proxy, const char *attribute)
{
	char *desc = NULL;
	DBusMessageIter iter;
	const char *path;

	default_dev = proxy;

	if (proxy == NULL) {
		default_attr = NULL;
		goto done;
	}

	if (!g_dbus_proxy_get_property(proxy, "Alias", &iter)) {
		if (!g_dbus_proxy_get_property(proxy, "Address", &iter))
			goto done;
	}

	path = g_dbus_proxy_get_path(proxy);

	dbus_message_iter_get_basic(&iter, &desc);
	desc = g_strdup_printf(COLOR_BLUE "[%s%s%s]" COLOR_OFF "# ", desc,
				attribute ? ":" : "",
				attribute ? attribute + strlen(path) : "");

done:
	bt_shell_set_prompt(desc ? desc : PROMPT_ON);
	g_free(desc);
}

static void battery_added(GDBusProxy *proxy)
{
	battery_proxies = g_list_append(battery_proxies, proxy);
}

static void battery_removed(GDBusProxy *proxy)
{
	battery_proxies = g_list_remove(battery_proxies, proxy);
}

static void device_added(GDBusProxy *proxy)
{
	DBusMessageIter iter;
	struct adapter *adapter = find_parent(proxy);

	if (!adapter) {
		/* TODO: Error */
		return;
	}

	adapter->devices = g_list_append(adapter->devices, proxy);
	print_device(proxy, COLORED_NEW);
	bt_shell_set_env(g_dbus_proxy_get_path(proxy), proxy);

	if (default_dev)
		return;

	if (g_dbus_proxy_get_property(proxy, "Connected", &iter)) {
		dbus_bool_t connected;

		dbus_message_iter_get_basic(&iter, &connected);

		if (connected)
			set_default_device(proxy, NULL);
	}
}

static struct adapter *find_ctrl(GList *source, const char *path);

static struct adapter *adapter_new(GDBusProxy *proxy)
{
	struct adapter *adapter = g_malloc0(sizeof(struct adapter));

	ctrl_list = g_list_append(ctrl_list, adapter);

	if (!default_ctrl)
		default_ctrl = adapter;

	return adapter;
}

static void adapter_added(GDBusProxy *proxy)
{
	struct adapter *adapter;
	adapter = find_ctrl(ctrl_list, g_dbus_proxy_get_path(proxy));
	if (!adapter)
		adapter = adapter_new(proxy);

	adapter->proxy = proxy;

	print_adapter(proxy, COLORED_NEW);
	bt_shell_set_env(g_dbus_proxy_get_path(proxy), proxy);
}

static void ad_manager_added(GDBusProxy *proxy)
{
	struct adapter *adapter;
	adapter = find_ctrl(ctrl_list, g_dbus_proxy_get_path(proxy));
	if (!adapter)
		adapter = adapter_new(proxy);

	adapter->ad_proxy = proxy;
}

static void admon_manager_added(GDBusProxy *proxy)
{
	struct adapter *adapter;

	adapter = find_ctrl(ctrl_list, g_dbus_proxy_get_path(proxy));
	if (!adapter)
		adapter = adapter_new(proxy);

	adapter->adv_monitor_proxy = proxy;
	adv_monitor_add_manager(dbus_conn, proxy);
	adv_monitor_register_app(dbus_conn);
}

static void proxy_added(GDBusProxy *proxy, void *user_data)
{
	const char *interface;

	interface = g_dbus_proxy_get_interface(proxy);

	if (!strcmp(interface, "org.bluez.Device1")) {
		device_added(proxy);
	} else if (!strcmp(interface, "org.bluez.Adapter1")) {
		adapter_added(proxy);
	} else if (!strcmp(interface, "org.bluez.AgentManager1")) {
		if (!agent_manager) {
			agent_manager = proxy;

			if (auto_register_agent &&
					!bt_shell_get_env("NON_INTERACTIVE"))
				agent_register(dbus_conn, agent_manager,
							auto_register_agent);
		}
	} else if (!strcmp(interface, "org.bluez.GattService1")) {
		if (service_is_child(proxy))
			gatt_add_service(proxy);
	} else if (!strcmp(interface, "org.bluez.GattCharacteristic1")) {
		gatt_add_characteristic(proxy);
	} else if (!strcmp(interface, "org.bluez.GattDescriptor1")) {
		gatt_add_descriptor(proxy);
	} else if (!strcmp(interface, "org.bluez.GattManager1")) {
		gatt_add_manager(proxy);
	} else if (!strcmp(interface, "org.bluez.LEAdvertisingManager1")) {
		ad_manager_added(proxy);
	} else if (!strcmp(interface, "org.bluez.Battery1")) {
		battery_added(proxy);
	} else if (!strcmp(interface,
				"org.bluez.AdvertisementMonitorManager1")) {
		admon_manager_added(proxy);
	}
}

static void set_default_attribute(GDBusProxy *proxy)
{
	const char *path;

	default_attr = proxy;

	path = g_dbus_proxy_get_path(proxy);

	set_default_device(default_dev, path);
}

static void device_removed(GDBusProxy *proxy)
{
	struct adapter *adapter = find_parent(proxy);
	if (!adapter) {
		/* TODO: Error */
		return;
	}

	adapter->devices = g_list_remove(adapter->devices, proxy);

	print_device(proxy, COLORED_DEL);
	bt_shell_set_env(g_dbus_proxy_get_path(proxy), NULL);

	if (default_dev == proxy)
		set_default_device(NULL, NULL);
}

static void adapter_removed(GDBusProxy *proxy)
{
	GList *ll;

	for (ll = g_list_first(ctrl_list); ll; ll = g_list_next(ll)) {
		struct adapter *adapter = ll->data;

		if (adapter->proxy == proxy) {
			print_adapter(proxy, COLORED_DEL);
			bt_shell_set_env(g_dbus_proxy_get_path(proxy), NULL);

			if (default_ctrl && default_ctrl->proxy == proxy) {
				default_ctrl = NULL;
				set_default_device(NULL, NULL);
			}

			ctrl_list = g_list_remove_link(ctrl_list, ll);
			g_list_free(adapter->devices);
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
	} else if (!strcmp(interface, "org.bluez.AgentManager1")) {
		if (agent_manager == proxy) {
			agent_manager = NULL;
			if (auto_register_agent)
				agent_unregister(dbus_conn, NULL);
		}
	} else if (!strcmp(interface, "org.bluez.GattService1")) {
		gatt_remove_service(proxy);

		if (default_attr == proxy)
			set_default_attribute(NULL);
	} else if (!strcmp(interface, "org.bluez.GattCharacteristic1")) {
		gatt_remove_characteristic(proxy);

		if (default_attr == proxy)
			set_default_attribute(NULL);
	} else if (!strcmp(interface, "org.bluez.GattDescriptor1")) {
		gatt_remove_descriptor(proxy);

		if (default_attr == proxy)
			set_default_attribute(NULL);
	} else if (!strcmp(interface, "org.bluez.GattManager1")) {
		gatt_remove_manager(proxy);
	} else if (!strcmp(interface, "org.bluez.LEAdvertisingManager1")) {
		ad_unregister(dbus_conn, NULL);
	} else if (!strcmp(interface, "org.bluez.Battery1")) {
		battery_removed(proxy);
	} else if (!strcmp(interface,
			"org.bluez.AdvertisementMonitorManager1")) {
		adv_monitor_remove_manager(dbus_conn);
	}
}

static struct adapter *find_ctrl(GList *source, const char *path)
{
	GList *list;

	for (list = g_list_first(source); list; list = g_list_next(list)) {
		struct adapter *adapter = list->data;

		if (!strcasecmp(g_dbus_proxy_get_path(adapter->proxy), path))
			return adapter;
	}

	return NULL;
}

static void property_changed(GDBusProxy *proxy, const char *name,
					DBusMessageIter *iter, void *user_data)
{
	const char *interface;
	struct adapter *ctrl;

	interface = g_dbus_proxy_get_interface(proxy);

	if (!strcmp(interface, "org.bluez.Device1")) {
		if (default_ctrl && device_is_child(proxy,
					default_ctrl->proxy) == TRUE) {
			DBusMessageIter addr_iter;
			char *str;

			if (g_dbus_proxy_get_property(proxy, "Address",
							&addr_iter) == TRUE) {
				const char *address;

				dbus_message_iter_get_basic(&addr_iter,
								&address);
				str = g_strdup_printf("[" COLORED_CHG
						"] Device %s ", address);
			} else
				str = g_strdup("");

			if (strcmp(name, "Connected") == 0) {
				dbus_bool_t connected;

				dbus_message_iter_get_basic(iter, &connected);

				if (connected && default_dev == NULL)
					set_default_device(proxy, NULL);
				else if (!connected && default_dev == proxy)
					set_default_device(NULL, NULL);
			}

			print_iter(str, name, iter);
			g_free(str);
		}
	} else if (!strcmp(interface, "org.bluez.Adapter1")) {
		DBusMessageIter addr_iter;
		char *str;

		if (g_dbus_proxy_get_property(proxy, "Address",
						&addr_iter) == TRUE) {
			const char *address;

			dbus_message_iter_get_basic(&addr_iter, &address);
			str = g_strdup_printf("[" COLORED_CHG
						"] Controller %s ", address);
		} else
			str = g_strdup("");

		print_iter(str, name, iter);
		g_free(str);
	} else if (!strcmp(interface, "org.bluez.LEAdvertisingManager1")) {
		DBusMessageIter addr_iter;
		char *str;

		ctrl = find_ctrl(ctrl_list, g_dbus_proxy_get_path(proxy));
		if (!ctrl)
			return;

		if (g_dbus_proxy_get_property(ctrl->proxy, "Address",
						&addr_iter) == TRUE) {
			const char *address;

			dbus_message_iter_get_basic(&addr_iter, &address);
			str = g_strdup_printf("[" COLORED_CHG
						"] Controller %s ",
						address);
		} else
			str = g_strdup("");

		print_iter(str, name, iter);
		g_free(str);
	} else if (proxy == default_attr) {
		char *str;

		str = g_strdup_printf("[" COLORED_CHG "] Attribute %s ",
						g_dbus_proxy_get_path(proxy));

		print_iter(str, name, iter);
		g_free(str);
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

		if (!strcasecmp(str, address))
			return adapter;
	}

	return NULL;
}

static GDBusProxy *find_battery_by_path(GList *source, const char *path)
{
	GList *list;

	for (list = g_list_first(source); list; list = g_list_next(list)) {
		GDBusProxy *proxy = list->data;

		if (strcmp(g_dbus_proxy_get_path(proxy), path) == 0)
			return proxy;
	}

	return NULL;
}

static GDBusProxy *find_proxy_by_address(GList *source, const char *address)
{
	GList *list;

	for (list = g_list_first(source); list; list = g_list_next(list)) {
		GDBusProxy *proxy = list->data;
		DBusMessageIter iter;
		const char *str;

		if (g_dbus_proxy_get_property(proxy, "Address", &iter) == FALSE)
			continue;

		dbus_message_iter_get_basic(&iter, &str);

		if (!strcasecmp(str, address))
			return proxy;
	}

	return NULL;
}

static gboolean check_default_ctrl(void)
{
	if (!default_ctrl) {
		bt_shell_printf("No default controller available\n");
		return FALSE;
	}

	return TRUE;
}

static gboolean parse_argument(int argc, char *argv[], const char **arg_table,
					const char *msg, dbus_bool_t *value,
					const char **option)
{
	const char **opt;

	if (!strcmp(argv[1], "help")) {
		for (opt = arg_table; opt && *opt; opt++)
			bt_shell_printf("%s\n", *opt);
		bt_shell_noninteractive_quit(EXIT_SUCCESS);
		return FALSE;
	}

	if (!strcmp(argv[1], "on") || !strcmp(argv[1], "yes")) {
		*value = TRUE;
		if (option)
			*option = "";
		return TRUE;
	}

	if (!strcmp(argv[1], "off") || !strcmp(argv[1], "no")) {
		*value = FALSE;
		return TRUE;
	}

	for (opt = arg_table; opt && *opt; opt++) {
		if (strcmp(argv[1], *opt) == 0) {
			*value = TRUE;
			*option = *opt;
			return TRUE;
		}
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
	DBusMessageIter iter;
	const char *address;

	if (argc < 2 || !strlen(argv[1])) {
		if (check_default_ctrl() == FALSE)
			return bt_shell_noninteractive_quit(EXIT_FAILURE);

		adapter = default_ctrl;
	} else {
		adapter = find_ctrl_by_address(ctrl_list, argv[1]);
		if (!adapter) {
			bt_shell_printf("Controller %s not available\n",
								argv[1]);
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	if (!g_dbus_proxy_get_property(adapter->proxy, "Address", &iter))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	dbus_message_iter_get_basic(&iter, &address);

	if (g_dbus_proxy_get_property(adapter->proxy, "AddressType", &iter)) {
		const char *type;

		dbus_message_iter_get_basic(&iter, &type);

		bt_shell_printf("Controller %s (%s)\n", address, type);
	} else {
		bt_shell_printf("Controller %s\n", address);
	}

	print_property(adapter->proxy, "Name");
	print_property(adapter->proxy, "Alias");
	print_property(adapter->proxy, "Class");
	print_property(adapter->proxy, "Powered");
	print_property(adapter->proxy, "Discoverable");
	print_property(adapter->proxy, "DiscoverableTimeout");
	print_property(adapter->proxy, "Pairable");
	print_uuids(adapter->proxy);
	print_property(adapter->proxy, "Modalias");
	print_property(adapter->proxy, "Discovering");
	print_property(adapter->proxy, "Roles");

	if (adapter->ad_proxy) {
		bt_shell_printf("Advertising Features:\n");
		print_property(adapter->ad_proxy, "ActiveInstances");
		print_property(adapter->ad_proxy, "SupportedInstances");
		print_property(adapter->ad_proxy, "SupportedIncludes");
		print_property(adapter->ad_proxy, "SupportedSecondaryChannels");
		print_property(adapter->ad_proxy, "SupportedCapabilities");
	}

	if (adapter->adv_monitor_proxy) {
		bt_shell_printf("Advertisement Monitor Features:\n");
		print_property(adapter->adv_monitor_proxy,
						"SupportedMonitorTypes");
		print_property(adapter->adv_monitor_proxy, "SupportedFeatures");
	}

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

	default_ctrl = adapter;
	print_adapter(adapter->proxy, NULL);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_devices(int argc, char *argv[])
{
	GList *ll;

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);

	for (ll = g_list_first(default_ctrl->devices);
			ll; ll = g_list_next(ll)) {
		GDBusProxy *proxy = ll->data;
		print_device(proxy, NULL);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_paired_devices(int argc, char *argv[])
{
	GList *ll;

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);

	for (ll = g_list_first(default_ctrl->devices);
			ll; ll = g_list_next(ll)) {
		GDBusProxy *proxy = ll->data;
		DBusMessageIter iter;
		dbus_bool_t paired;

		if (g_dbus_proxy_get_property(proxy, "Paired", &iter) == FALSE)
			continue;

		dbus_message_iter_get_basic(&iter, &paired);
		if (!paired)
			continue;

		print_device(proxy, NULL);
	}

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

static void cmd_system_alias(int argc, char *argv[])
{
	char *name;

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	name = g_strdup(argv[1]);

	if (g_dbus_proxy_set_property_basic(default_ctrl->proxy, "Alias",
					DBUS_TYPE_STRING, &name,
					generic_callback, name, g_free) == TRUE)
		return;

	g_free(name);
}

static void cmd_reset_alias(int argc, char *argv[])
{
	char *name;

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	name = g_strdup("");

	if (g_dbus_proxy_set_property_basic(default_ctrl->proxy, "Alias",
					DBUS_TYPE_STRING, &name,
					generic_callback, name, g_free) == TRUE)
		return;

	g_free(name);
}

static void cmd_power(int argc, char *argv[])
{
	dbus_bool_t powered;
	char *str;

	if (!parse_argument(argc, argv, NULL, NULL, &powered, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	str = g_strdup_printf("power %s", powered == TRUE ? "on" : "off");

	if (g_dbus_proxy_set_property_basic(default_ctrl->proxy, "Powered",
					DBUS_TYPE_BOOLEAN, &powered,
					generic_callback, str, g_free) == TRUE)
		return;

	g_free(str);
}

static void cmd_pairable(int argc, char *argv[])
{
	dbus_bool_t pairable;
	char *str;

	if (!parse_argument(argc, argv, NULL, NULL, &pairable, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	str = g_strdup_printf("pairable %s", pairable == TRUE ? "on" : "off");

	if (g_dbus_proxy_set_property_basic(default_ctrl->proxy, "Pairable",
					DBUS_TYPE_BOOLEAN, &pairable,
					generic_callback, str, g_free) == TRUE)
		return;

	g_free(str);

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void cmd_discoverable(int argc, char *argv[])
{
	dbus_bool_t discoverable;
	char *str;

	if (!parse_argument(argc, argv, NULL, NULL, &discoverable, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	str = g_strdup_printf("discoverable %s",
				discoverable == TRUE ? "on" : "off");

	if (g_dbus_proxy_set_property_basic(default_ctrl->proxy, "Discoverable",
					DBUS_TYPE_BOOLEAN, &discoverable,
					generic_callback, str, g_free) == TRUE)
		return;

	g_free(str);

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void cmd_discoverable_timeout(int argc, char *argv[])
{
	uint32_t value;
	char *endptr = NULL;
	char *str;

	if (argc < 2) {
		DBusMessageIter iter;

		if (!g_dbus_proxy_get_property(default_ctrl->proxy,
					"DiscoverableTimeout", &iter)) {
			bt_shell_printf("Unable to get DiscoverableTimeout\n");
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}

		dbus_message_iter_get_basic(&iter, &value);

		bt_shell_printf("DiscoverableTimeout: %d seconds\n", value);

		return;
	}

	value = strtol(argv[1], &endptr, 0);
	if (!endptr || *endptr != '\0' || value > UINT32_MAX) {
		bt_shell_printf("Invalid argument\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	str = g_strdup_printf("discoverable-timeout %d", value);

	if (g_dbus_proxy_set_property_basic(default_ctrl->proxy,
					"DiscoverableTimeout",
					DBUS_TYPE_UINT32, &value,
					generic_callback, str, g_free))
		return;

	g_free(str);

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void cmd_agent(int argc, char *argv[])
{
	dbus_bool_t enable;
	const char *capability;

	if (!parse_argument(argc, argv, agent_arguments, "capability",
						&enable, &capability))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (enable == TRUE) {
		g_free(auto_register_agent);
		auto_register_agent = g_strdup(capability);

		if (agent_manager)
			agent_register(dbus_conn, agent_manager,
						auto_register_agent);
		else
			bt_shell_printf("Agent registration enabled\n");
	} else {
		g_free(auto_register_agent);
		auto_register_agent = NULL;

		if (agent_manager)
			agent_unregister(dbus_conn, agent_manager);
		else
			bt_shell_printf("Agent registration disabled\n");
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_default_agent(int argc, char *argv[])
{
	agent_default(dbus_conn, agent_manager);
}

#define	DISTANCE_VAL_INVALID	0x7FFF

static struct set_discovery_filter_args {
	char *transport;
	char *pattern;
	dbus_uint16_t rssi;
	dbus_int16_t pathloss;
	char **uuids;
	size_t uuids_len;
	dbus_bool_t duplicate;
	dbus_bool_t discoverable;
	bool set;
	bool active;
} filter = {
	.rssi = DISTANCE_VAL_INVALID,
	.pathloss = DISTANCE_VAL_INVALID,
	.set = true,
};

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

	bt_shell_printf("Discovery %s\n", enable ? "started" : "stopped");

	filter.active = enable;
	/* Leave the discovery running even on noninteractive mode */
}

static void clear_discovery_filter(DBusMessageIter *iter, void *user_data)
{
	DBusMessageIter dict;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
				DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
				DBUS_TYPE_STRING_AS_STRING
				DBUS_TYPE_VARIANT_AS_STRING
				DBUS_DICT_ENTRY_END_CHAR_AS_STRING, &dict);

	dbus_message_iter_close_container(iter, &dict);
}

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

	if (args->discoverable)
		g_dbus_dict_append_entry(&dict, "Discoverable",
						DBUS_TYPE_BOOLEAN,
						&args->discoverable);

	if (args->pattern != NULL)
		g_dbus_dict_append_entry(&dict, "Pattern", DBUS_TYPE_STRING,
						&args->pattern);

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

	filter.set = true;

	bt_shell_printf("SetDiscoveryFilter success\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void set_discovery_filter(bool cleared)
{
	GDBusSetupFunction func;

	if (check_default_ctrl() == FALSE || filter.set)
		return;

	func = cleared ? clear_discovery_filter : set_discovery_filter_setup;

	if (g_dbus_proxy_method_call(default_ctrl->proxy, "SetDiscoveryFilter",
					func, set_discovery_filter_reply,
					&filter, NULL) == FALSE) {
		bt_shell_printf("Failed to set discovery filter\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	filter.set = true;
}

static void cmd_scan(int argc, char *argv[])
{
	dbus_bool_t enable;
	const char *method;

	if (!parse_argument(argc, argv, NULL, NULL, &enable, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (enable == TRUE) {
		set_discovery_filter(false);
		method = "StartDiscovery";
	} else
		method = "StopDiscovery";

	if (g_dbus_proxy_method_call(default_ctrl->proxy, method,
				NULL, start_discovery_reply,
				GUINT_TO_POINTER(enable), NULL) == FALSE) {
		bt_shell_printf("Failed to %s discovery\n",
					enable == TRUE ? "start" : "stop");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_scan_filter_uuids(int argc, char *argv[])
{
	if (argc < 2 || !strlen(argv[1])) {
		char **uuid;

		for (uuid = filter.uuids; uuid && *uuid; uuid++)
			print_uuid(*uuid);

		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	g_strfreev(filter.uuids);
	filter.uuids = NULL;
	filter.uuids_len = 0;

	if (!strcmp(argv[1], "all"))
		goto commit;

	filter.uuids = g_strdupv(&argv[1]);
	if (!filter.uuids) {
		bt_shell_printf("Failed to parse input\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	filter.uuids_len = g_strv_length(filter.uuids);

commit:
	filter.set = false;

	if (filter.active)
		set_discovery_filter(false);
}

static void cmd_scan_filter_rssi(int argc, char *argv[])
{
	if (argc < 2 || !strlen(argv[1])) {
		if (filter.rssi != DISTANCE_VAL_INVALID)
			bt_shell_printf("RSSI: %d\n", filter.rssi);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	filter.pathloss = DISTANCE_VAL_INVALID;
	filter.rssi = atoi(argv[1]);

	filter.set = false;

	if (filter.active)
		set_discovery_filter(false);
}

static void cmd_scan_filter_pathloss(int argc, char *argv[])
{
	if (argc < 2 || !strlen(argv[1])) {
		if (filter.pathloss != DISTANCE_VAL_INVALID)
			bt_shell_printf("Pathloss: %d\n",
						filter.pathloss);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	filter.rssi = DISTANCE_VAL_INVALID;
	filter.pathloss = atoi(argv[1]);

	filter.set = false;

	if (filter.active)
		set_discovery_filter(false);
}

static void cmd_scan_filter_transport(int argc, char *argv[])
{
	if (argc < 2 || !strlen(argv[1])) {
		if (filter.transport)
			bt_shell_printf("Transport: %s\n",
					filter.transport);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	g_free(filter.transport);
	filter.transport = g_strdup(argv[1]);

	filter.set = false;

	if (filter.active)
		set_discovery_filter(false);
}

static void cmd_scan_filter_duplicate_data(int argc, char *argv[])
{
	if (argc < 2 || !strlen(argv[1])) {
		bt_shell_printf("DuplicateData: %s\n",
				filter.duplicate ? "on" : "off");
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	if (!strcmp(argv[1], "on"))
		filter.duplicate = true;
	else if (!strcmp(argv[1], "off"))
		filter.duplicate = false;
	else {
		bt_shell_printf("Invalid option: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	filter.set = false;

	if (filter.active)
		set_discovery_filter(false);
}

static void cmd_scan_filter_discoverable(int argc, char *argv[])
{
	if (argc < 2 || !strlen(argv[1])) {
		bt_shell_printf("Discoverable: %s\n",
				filter.discoverable ? "on" : "off");
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	if (!strcmp(argv[1], "on"))
		filter.discoverable = true;
	else if (!strcmp(argv[1], "off"))
		filter.discoverable = false;
	else {
		bt_shell_printf("Invalid option: %s\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	filter.set = false;

	if (filter.active)
		set_discovery_filter(false);
}

static void cmd_scan_filter_pattern(int argc, char *argv[])
{
	if (argc < 2 || !strlen(argv[1])) {
		bt_shell_printf("Pattern: %s\n", filter.pattern);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	free(filter.pattern);
	filter.pattern = strdup(argv[1]);

	filter.set = false;

	if (filter.active)
		set_discovery_filter(false);
}

static void filter_clear_uuids(void)
{
	g_strfreev(filter.uuids);
	filter.uuids = NULL;
	filter.uuids_len = 0;
}

static void filter_clear_rssi(void)
{
	filter.rssi = DISTANCE_VAL_INVALID;
}

static void filter_clear_pathloss(void)
{
	filter.pathloss = DISTANCE_VAL_INVALID;
}

static void filter_clear_transport(void)
{
	g_free(filter.transport);
	filter.transport = NULL;
}

static void filter_clear_duplicate(void)
{
	filter.duplicate = false;
}

static void filter_clear_discoverable(void)
{
	filter.discoverable = false;
}

static void filter_clear_pattern(void)
{
	free(filter.pattern);
	filter.pattern = NULL;
}

struct clear_entry {
	const char *name;
	void (*clear) (void);
};

static const struct clear_entry filter_clear[] = {
	{ "uuids", filter_clear_uuids },
	{ "rssi", filter_clear_rssi },
	{ "pathloss", filter_clear_pathloss },
	{ "transport", filter_clear_transport },
	{ "duplicate-data", filter_clear_duplicate },
	{ "discoverable", filter_clear_discoverable },
	{ "pattern", filter_clear_pattern },
	{}
};

static char *filter_clear_generator(const char *text, int state)
{
	static int index, len;
	const char *arg;

	if (!state) {
		index = 0;
		len = strlen(text);
	}

	while ((arg = filter_clear[index].name)) {
		index++;

		if (!strncmp(arg, text, len))
			return strdup(arg);
	}

	return NULL;
}

static gboolean data_clear(const struct clear_entry *entry_table,
							const char *name)
{
	const struct clear_entry *entry;
	bool all = false;

	if (!name || !strlen(name) || !strcmp("all", name))
		all = true;

	for (entry = entry_table; entry && entry->name; entry++) {
		if (all || !strcmp(entry->name, name)) {
			entry->clear();
			if (!all)
				goto done;
		}
	}

	if (!all) {
		bt_shell_printf("Invalid argument %s\n", name);
		return FALSE;
	}

done:
	return TRUE;
}

static void cmd_scan_filter_clear(int argc, char *argv[])
{
	bool all = false;

	if (argc < 2 || !strlen(argv[1]))
		all = true;

	if (!data_clear(filter_clear, all ? "all" : argv[1]))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	filter.set = false;

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	set_discovery_filter(all);
}

static struct GDBusProxy *find_device(int argc, char *argv[])
{
	GDBusProxy *proxy;

	if (argc < 2 || !strlen(argv[1])) {
		if (default_dev)
			return default_dev;
		bt_shell_printf("Missing device address argument\n");
		return NULL;
	}

	if (check_default_ctrl() == FALSE)
		return NULL;

	proxy = find_proxy_by_address(default_ctrl->devices, argv[1]);
	if (!proxy) {
		bt_shell_printf("Device %s not available\n", argv[1]);
		return NULL;
	}

	return proxy;
}

static void cmd_info(int argc, char *argv[])
{
	GDBusProxy *proxy;
	GDBusProxy *battery_proxy;
	DBusMessageIter iter;
	const char *address;

	proxy = find_device(argc, argv);
	if (!proxy)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (g_dbus_proxy_get_property(proxy, "Address", &iter) == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	dbus_message_iter_get_basic(&iter, &address);

	if (g_dbus_proxy_get_property(proxy, "AddressType", &iter) == TRUE) {
		const char *type;

		dbus_message_iter_get_basic(&iter, &type);

		bt_shell_printf("Device %s (%s)\n", address, type);
	} else {
		bt_shell_printf("Device %s\n", address);
	}

	print_property(proxy, "Name");
	print_property(proxy, "Alias");
	print_property(proxy, "Class");
	print_property(proxy, "Appearance");
	print_property(proxy, "Icon");
	print_property(proxy, "Paired");
	print_property(proxy, "Trusted");
	print_property(proxy, "Blocked");
	print_property(proxy, "Connected");
	print_property(proxy, "WakeAllowed");
	print_property(proxy, "LegacyPairing");
	print_uuids(proxy);
	print_property(proxy, "Modalias");
	print_property(proxy, "ManufacturerData");
	print_property(proxy, "ServiceData");
	print_property(proxy, "RSSI");
	print_property(proxy, "TxPower");
	print_property(proxy, "AdvertisingFlags");
	print_property(proxy, "AdvertisingData");

	battery_proxy = find_battery_by_path(battery_proxies,
					g_dbus_proxy_get_path(proxy));
	print_property_with_label(battery_proxy, "Percentage",
					"Battery Percentage");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void pair_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to pair: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Pairing successful\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static const char *proxy_address(GDBusProxy *proxy)
{
	DBusMessageIter iter;
	const char *addr;

	if (!g_dbus_proxy_get_property(proxy, "Address", &iter))
		return NULL;

	dbus_message_iter_get_basic(&iter, &addr);

	return addr;
}

static void cmd_pair(int argc, char *argv[])
{
	GDBusProxy *proxy;

	proxy = find_device(argc, argv);
	if (!proxy)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (g_dbus_proxy_method_call(proxy, "Pair", NULL, pair_reply,
							NULL, NULL) == FALSE) {
		bt_shell_printf("Failed to pair\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to pair with %s\n", proxy_address(proxy));
}

static void cancel_pairing_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to cancel pairing: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	bt_shell_printf("Cancel pairing successful\n");
}

static void cmd_cancel_pairing(int argc, char *argv[])
{
	GDBusProxy *proxy;

	proxy = find_device(argc, argv);
	if (!proxy)
		return;

	if (g_dbus_proxy_method_call(proxy, "CancelPairing", NULL,
				cancel_pairing_reply, NULL, NULL) == FALSE) {
		bt_shell_printf("Failed to cancel pairing\n");
		return;
	}

	bt_shell_printf("Attempting to cancel pairing with %s\n",
							proxy_address(proxy));
}

static void cmd_trust(int argc, char *argv[])
{
	GDBusProxy *proxy;
	dbus_bool_t trusted;
	char *str;

	proxy = find_device(argc, argv);
	if (!proxy)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	trusted = TRUE;

	str = g_strdup_printf("%s trust", proxy_address(proxy));

	if (g_dbus_proxy_set_property_basic(proxy, "Trusted",
					DBUS_TYPE_BOOLEAN, &trusted,
					generic_callback, str, g_free) == TRUE)
		return;

	g_free(str);

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void cmd_untrust(int argc, char *argv[])
{
	GDBusProxy *proxy;
	dbus_bool_t trusted;
	char *str;

	proxy = find_device(argc, argv);
	if (!proxy)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	trusted = FALSE;

	str = g_strdup_printf("%s untrust", proxy_address(proxy));

	if (g_dbus_proxy_set_property_basic(proxy, "Trusted",
					DBUS_TYPE_BOOLEAN, &trusted,
					generic_callback, str, g_free) == TRUE)
		return;

	g_free(str);

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void cmd_block(int argc, char *argv[])
{
	GDBusProxy *proxy;
	dbus_bool_t blocked;
	char *str;

	proxy = find_device(argc, argv);
	if (!proxy)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	blocked = TRUE;

	str = g_strdup_printf("%s block", proxy_address(proxy));

	if (g_dbus_proxy_set_property_basic(proxy, "Blocked",
					DBUS_TYPE_BOOLEAN, &blocked,
					generic_callback, str, g_free) == TRUE)
		return;

	g_free(str);

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void cmd_unblock(int argc, char *argv[])
{
	GDBusProxy *proxy;
	dbus_bool_t blocked;
	char *str;

	proxy = find_device(argc, argv);
	if (!proxy)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	blocked = FALSE;

	str = g_strdup_printf("%s unblock", proxy_address(proxy));

	if (g_dbus_proxy_set_property_basic(proxy, "Blocked",
					DBUS_TYPE_BOOLEAN, &blocked,
					generic_callback, str, g_free) == TRUE)
		return;

	g_free(str);

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void remove_device_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to remove device: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Device has been removed\n");
	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void remove_device_setup(DBusMessageIter *iter, void *user_data)
{
	const char *path = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
}

static void remove_device(GDBusProxy *proxy)
{
	char *path;

	if (!default_ctrl)
		return;

	path = g_strdup(g_dbus_proxy_get_path(proxy));

	if (g_dbus_proxy_method_call(default_ctrl->proxy, "RemoveDevice",
						remove_device_setup,
						remove_device_reply,
						path, g_free) == FALSE) {
		bt_shell_printf("Failed to remove device\n");
		g_free(path);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
}

static void cmd_remove(int argc, char *argv[])
{
	GDBusProxy *proxy;

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (strcmp(argv[1], "*") == 0) {
		GList *list;

		for (list = default_ctrl->devices; list;
						list = g_list_next(list)) {
			GDBusProxy *proxy = list->data;

			remove_device(proxy);
		}
		return;
	}

	proxy = find_proxy_by_address(default_ctrl->devices, argv[1]);
	if (!proxy) {
		bt_shell_printf("Device %s not available\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	remove_device(proxy);
}

static void connect_reply(DBusMessage *message, void *user_data)
{
	GDBusProxy *proxy = user_data;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to connect: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Connection successful\n");

	set_default_device(proxy, NULL);
	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_connect(int argc, char *argv[])
{
	GDBusProxy *proxy;

	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	proxy = find_proxy_by_address(default_ctrl->devices, argv[1]);
	if (!proxy) {
		bt_shell_printf("Device %s not available\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (g_dbus_proxy_method_call(proxy, "Connect", NULL, connect_reply,
							proxy, NULL) == FALSE) {
		bt_shell_printf("Failed to connect\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to connect to %s\n", argv[1]);
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

	bt_shell_printf("Successful disconnected\n");

	if (proxy == default_dev)
		set_default_device(NULL, NULL);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_disconn(int argc, char *argv[])
{
	GDBusProxy *proxy;

	proxy = find_device(argc, argv);
	if (!proxy)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (g_dbus_proxy_method_call(proxy, "Disconnect", NULL, disconn_reply,
							proxy, NULL) == FALSE) {
		bt_shell_printf("Failed to disconnect\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to disconnect from %s\n",
						proxy_address(proxy));
}

static void cmd_list_attributes(int argc, char *argv[])
{
	GDBusProxy *proxy;
	const char *path;

	if (argc > 1 && !strcmp(argv[1], "local")) {
		path = argv[1];
		goto done;
	}

	proxy = find_device(argc, argv);
	if (!proxy)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	path = g_dbus_proxy_get_path(proxy);

done:
	gatt_list_attributes(path);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_set_alias(int argc, char *argv[])
{
	char *name;

	if (!default_dev) {
		bt_shell_printf("No device connected\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	name = g_strdup(argv[1]);

	if (g_dbus_proxy_set_property_basic(default_dev, "Alias",
					DBUS_TYPE_STRING, &name,
					generic_callback, name, g_free) == TRUE)
		return;

	g_free(name);

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void cmd_select_attribute(int argc, char *argv[])
{
	GDBusProxy *proxy;

	if (!default_dev) {
		bt_shell_printf("No device connected\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	proxy = gatt_select_attribute(default_attr, argv[1]);
	if (proxy) {
		set_default_attribute(proxy);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static struct GDBusProxy *find_attribute(int argc, char *argv[])
{
	GDBusProxy *proxy;

	if (argc < 2 || !strlen(argv[1])) {
		if (default_attr)
			return default_attr;
		bt_shell_printf("Missing attribute argument\n");
		return NULL;
	}

	proxy = gatt_select_attribute(default_attr, argv[1]);
	if (!proxy) {
		bt_shell_printf("Attribute %s not available\n", argv[1]);
		return NULL;
	}

	return proxy;
}

static void cmd_attribute_info(int argc, char *argv[])
{
	GDBusProxy *proxy;
	DBusMessageIter iter;
	const char *iface, *uuid, *text;

	proxy = find_attribute(argc, argv);
	if (!proxy)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (g_dbus_proxy_get_property(proxy, "UUID", &iter) == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	dbus_message_iter_get_basic(&iter, &uuid);

	text = bt_uuidstr_to_str(uuid);
	if (!text)
		text = g_dbus_proxy_get_path(proxy);

	iface = g_dbus_proxy_get_interface(proxy);
	if (!strcmp(iface, "org.bluez.GattService1")) {
		bt_shell_printf("Service - %s\n", text);

		print_property(proxy, "UUID");
		print_property(proxy, "Primary");
		print_property(proxy, "Characteristics");
		print_property(proxy, "Includes");
	} else if (!strcmp(iface, "org.bluez.GattCharacteristic1")) {
		bt_shell_printf("Characteristic - %s\n", text);

		print_property(proxy, "UUID");
		print_property(proxy, "Service");
		print_property(proxy, "Value");
		print_property(proxy, "Notifying");
		print_property(proxy, "Flags");
		print_property(proxy, "Descriptors");
	} else if (!strcmp(iface, "org.bluez.GattDescriptor1")) {
		bt_shell_printf("Descriptor - %s\n", text);

		print_property(proxy, "UUID");
		print_property(proxy, "Characteristic");
		print_property(proxy, "Value");
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_read(int argc, char *argv[])
{
	if (!default_attr) {
		bt_shell_printf("No attribute selected\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	gatt_read_attribute(default_attr, argc, argv);
}

static void cmd_write(int argc, char *argv[])
{
	if (!default_attr) {
		bt_shell_printf("No attribute selected\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	gatt_write_attribute(default_attr, argc, argv);
}

static void cmd_acquire_write(int argc, char *argv[])
{
	if (!default_attr) {
		bt_shell_printf("No attribute selected\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	gatt_acquire_write(default_attr, argv[1]);
}

static void cmd_release_write(int argc, char *argv[])
{
	if (!default_attr) {
		bt_shell_printf("No attribute selected\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	gatt_release_write(default_attr, argv[1]);
}

static void cmd_acquire_notify(int argc, char *argv[])
{
	if (!default_attr) {
		bt_shell_printf("No attribute selected\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	gatt_acquire_notify(default_attr, argv[1]);
}

static void cmd_release_notify(int argc, char *argv[])
{
	if (!default_attr) {
		bt_shell_printf("No attribute selected\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	gatt_release_notify(default_attr, argv[1]);
}

static void cmd_notify(int argc, char *argv[])
{
	dbus_bool_t enable;

	if (!parse_argument(argc, argv, NULL, NULL, &enable, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!default_attr) {
		bt_shell_printf("No attribute selected\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	gatt_notify_attribute(default_attr, enable ? true : false);
}

static void cmd_clone(int argc, char *argv[])
{
	GDBusProxy *proxy;

	proxy = default_attr ? default_attr : default_dev;
	if (!proxy) {
		bt_shell_printf("Not connected\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	gatt_clone_attribute(proxy, argc, argv);
}

static void cmd_register_app(int argc, char *argv[])
{
	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	gatt_register_app(dbus_conn, default_ctrl->proxy, argc, argv);
}

static void cmd_unregister_app(int argc, char *argv[])
{
	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	gatt_unregister_app(dbus_conn, default_ctrl->proxy);
}

static void cmd_register_service(int argc, char *argv[])
{
	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	gatt_register_service(dbus_conn, default_ctrl->proxy, argc, argv);
}

static void cmd_register_includes(int argc, char *argv[])
{
	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	gatt_register_include(dbus_conn, default_ctrl->proxy, argc, argv);
}

static void cmd_unregister_includes(int argc, char *argv[])
{
	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	gatt_unregister_include(dbus_conn, default_ctrl->proxy, argc, argv);
}

static void cmd_unregister_service(int argc, char *argv[])
{
	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	gatt_unregister_service(dbus_conn, default_ctrl->proxy, argc, argv);
}

static void cmd_register_characteristic(int argc, char *argv[])
{
	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	gatt_register_chrc(dbus_conn, default_ctrl->proxy, argc, argv);
}

static void cmd_unregister_characteristic(int argc, char *argv[])
{
	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	gatt_unregister_chrc(dbus_conn, default_ctrl->proxy, argc, argv);
}

static void cmd_register_descriptor(int argc, char *argv[])
{
	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	gatt_register_desc(dbus_conn, default_ctrl->proxy, argc, argv);
}

static void cmd_unregister_descriptor(int argc, char *argv[])
{
	if (check_default_ctrl() == FALSE)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	gatt_unregister_desc(dbus_conn, default_ctrl->proxy, argc, argv);
}

static char *generic_generator(const char *text, int state,
					GList *source, const char *property)
{
	static int index, len;
	GList *list;

	if (!state) {
		index = 0;
		len = strlen(text);
	}

	for (list = g_list_nth(source, index); list;
						list = g_list_next(list)) {
		GDBusProxy *proxy = list->data;
		DBusMessageIter iter;
		const char *str;

		index++;

		if (g_dbus_proxy_get_property(proxy, property, &iter) == FALSE)
			continue;

		dbus_message_iter_get_basic(&iter, &str);

		if (!strncasecmp(str, text, len))
			return strdup(str);
	}

	return NULL;
}

static char *ctrl_generator(const char *text, int state)
{
	static int index = 0;
	static int len = 0;
	GList *list;

	if (!state) {
		index = 0;
		len = strlen(text);
	}

	for (list = g_list_nth(ctrl_list, index); list;
						list = g_list_next(list)) {
		struct adapter *adapter = list->data;
		DBusMessageIter iter;
		const char *str;

		index++;

		if (g_dbus_proxy_get_property(adapter->proxy,
					"Address", &iter) == FALSE)
			continue;

		dbus_message_iter_get_basic(&iter, &str);

		if (!strncasecmp(str, text, len))
			return strdup(str);
	}

	return NULL;
}

static char *dev_generator(const char *text, int state)
{
	return generic_generator(text, state,
			default_ctrl ? default_ctrl->devices : NULL, "Address");
}

static char *attribute_generator(const char *text, int state)
{
	return gatt_attribute_generator(text, state);
}

static char *argument_generator(const char *text, int state,
					const char *args_list[])
{
	static int index, len;
	const char *arg;

	if (!state) {
		index = 0;
		len = strlen(text);
	}

	while ((arg = args_list[index])) {
		index++;

		if (!strncmp(arg, text, len))
			return strdup(arg);
	}

	return NULL;
}

static char *capability_generator(const char *text, int state)
{
	return argument_generator(text, state, agent_arguments);
}

static void cmd_advertise(int argc, char *argv[])
{
	dbus_bool_t enable;
	const char *type;

	if (!parse_argument(argc, argv, ad_arguments, "type",
					&enable, &type))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	if (!default_ctrl || !default_ctrl->ad_proxy) {
		bt_shell_printf("LEAdvertisingManager not found\n");
		bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (enable == TRUE)
		ad_register(dbus_conn, default_ctrl->ad_proxy, type);
	else
		ad_unregister(dbus_conn, default_ctrl->ad_proxy);
}

static char *ad_generator(const char *text, int state)
{
	return argument_generator(text, state, ad_arguments);
}

static void cmd_advertise_uuids(int argc, char *argv[])
{
	ad_advertise_uuids(dbus_conn, argc, argv);
}

static void cmd_advertise_service(int argc, char *argv[])
{
	ad_advertise_service(dbus_conn, argc, argv);
}

static void cmd_advertise_manufacturer(int argc, char *argv[])
{
	ad_advertise_manufacturer(dbus_conn, argc, argv);
}

static void cmd_advertise_data(int argc, char *argv[])
{
	ad_advertise_data(dbus_conn, argc, argv);
}

static void cmd_advertise_discoverable(int argc, char *argv[])
{
	dbus_bool_t discoverable;

	if (argc < 2) {
		ad_advertise_discoverable(dbus_conn, NULL);
		return;
	}

	if (!parse_argument(argc, argv, NULL, NULL, &discoverable, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	ad_advertise_discoverable(dbus_conn, &discoverable);
}

static void cmd_advertise_discoverable_timeout(int argc, char *argv[])
{
	long int value;
	char *endptr = NULL;

	if (argc < 2) {
		ad_advertise_discoverable_timeout(dbus_conn, NULL);
		return;
	}

	value = strtol(argv[1], &endptr, 0);
	if (!endptr || *endptr != '\0' || value > UINT16_MAX) {
		bt_shell_printf("Invalid argument\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	ad_advertise_discoverable_timeout(dbus_conn, &value);
}

static void cmd_advertise_tx_power(int argc, char *argv[])
{
	dbus_bool_t powered;

	if (argc < 2) {
		ad_advertise_tx_power(dbus_conn, NULL);
		return;
	}

	if (!parse_argument(argc, argv, NULL, NULL, &powered, NULL))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	ad_advertise_tx_power(dbus_conn, &powered);
}

static void cmd_advertise_name(int argc, char *argv[])
{
	if (argc < 2) {
		ad_advertise_local_name(dbus_conn, NULL);
		return;
	}

	if (strcmp(argv[1], "on") == 0 || strcmp(argv[1], "yes") == 0) {
		ad_advertise_name(dbus_conn, true);
		return;
	}

	if (strcmp(argv[1], "off") == 0 || strcmp(argv[1], "no") == 0) {
		ad_advertise_name(dbus_conn, false);
		return;
	}

	ad_advertise_local_name(dbus_conn, argv[1]);
}

static void cmd_advertise_appearance(int argc, char *argv[])
{
	long int value;
	char *endptr = NULL;

	if (argc < 2) {
		ad_advertise_local_appearance(dbus_conn, NULL);
		return;
	}

	if (strcmp(argv[1], "on") == 0 || strcmp(argv[1], "yes") == 0) {
		ad_advertise_appearance(dbus_conn, true);
		return;
	}

	if (strcmp(argv[1], "off") == 0 || strcmp(argv[1], "no") == 0) {
		ad_advertise_appearance(dbus_conn, false);
		return;
	}

	value = strtol(argv[1], &endptr, 0);
	if (!endptr || *endptr != '\0' || value > UINT16_MAX) {
		bt_shell_printf("Invalid argument\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	ad_advertise_local_appearance(dbus_conn, &value);
}

static void cmd_advertise_duration(int argc, char *argv[])
{
	long int value;
	char *endptr = NULL;

	if (argc < 2) {
		ad_advertise_duration(dbus_conn, NULL);
		return;
	}

	value = strtol(argv[1], &endptr, 0);
	if (!endptr || *endptr != '\0' || value > UINT16_MAX) {
		bt_shell_printf("Invalid argument\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	ad_advertise_duration(dbus_conn, &value);
}

static void cmd_advertise_timeout(int argc, char *argv[])
{
	long int value;
	char *endptr = NULL;

	if (argc < 2) {
		ad_advertise_timeout(dbus_conn, NULL);
		return;
	}

	value = strtol(argv[1], &endptr, 0);
	if (!endptr || *endptr != '\0' || value > UINT16_MAX) {
		bt_shell_printf("Invalid argument\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	ad_advertise_timeout(dbus_conn, &value);
}

static void cmd_advertise_secondary(int argc, char *argv[])
{
	if (argc < 2) {
		ad_advertise_secondary(dbus_conn, NULL);
		return;
	}

	ad_advertise_secondary(dbus_conn, argv[1]);
}

static void ad_clear_uuids(void)
{
	ad_disable_uuids(dbus_conn);
}

static void ad_clear_service(void)
{
	ad_disable_service(dbus_conn);
}

static void ad_clear_manufacturer(void)
{
	ad_disable_manufacturer(dbus_conn);
}

static void ad_clear_data(void)
{
	ad_disable_data(dbus_conn);
}

static void ad_clear_tx_power(void)
{
	dbus_bool_t powered = false;

	ad_advertise_tx_power(dbus_conn, &powered);
}

static void ad_clear_name(void)
{
	ad_advertise_name(dbus_conn, false);
}

static void ad_clear_appearance(void)
{
	ad_advertise_appearance(dbus_conn, false);
}

static void ad_clear_duration(void)
{
	long int value = 0;

	ad_advertise_duration(dbus_conn, &value);
}

static void ad_clear_timeout(void)
{
	long int value = 0;

	ad_advertise_timeout(dbus_conn, &value);
}

static void ad_clear_secondary(void)
{
	const char *value = "";

	ad_advertise_secondary(dbus_conn, value);
}

static const struct clear_entry ad_clear[] = {
	{ "uuids",		ad_clear_uuids },
	{ "service",		ad_clear_service },
	{ "manufacturer",	ad_clear_manufacturer },
	{ "data",		ad_clear_data },
	{ "tx-power",		ad_clear_tx_power },
	{ "name",		ad_clear_name },
	{ "appearance",		ad_clear_appearance },
	{ "duration",		ad_clear_duration },
	{ "timeout",		ad_clear_timeout },
	{ "secondary",		ad_clear_secondary },
	{}
};

static void cmd_ad_clear(int argc, char *argv[])
{
	bool all = false;

	if (argc < 2 || !strlen(argv[1]))
		all = true;

	if(!data_clear(ad_clear, all ? "all" : argv[1]))
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void print_add_or_pattern_usage(void)
{
	bt_shell_printf("pattern format:\n"
			"\t<start_position> <ad_data_type> <content_of_pattern>\n");
	bt_shell_printf("e.g.\n"
			"\tadd-or-pattern 1 2 01ab55 3 4 23cd66\n");
}

static void cmd_adv_monitor_print_usage(int argc, char *argv[])
{
	if (strcmp(argv[1], "add-or-pattern") == 0)
		print_add_or_pattern_usage();
	else
		bt_shell_printf("Invalid argument %s", argv[1]);
}

static void cmd_adv_monitor_set_rssi_threshold(int argc, char *argv[])
{
	int low_threshold, high_threshold;

	low_threshold = atoi(argv[1]);
	high_threshold = atoi(argv[2]);
	adv_monitor_set_rssi_threshold(low_threshold, high_threshold);
}

static void cmd_adv_monitor_set_rssi_timeout(int argc, char *argv[])
{
	int low_timeout, high_timeout;

	low_timeout = atoi(argv[1]);
	high_timeout = atoi(argv[2]);
	adv_monitor_set_rssi_timeout(low_timeout, high_timeout);
}

static void cmd_adv_monitor_set_rssi_sampling_period(int argc, char *argv[])
{
	int sampling = atoi(argv[1]);

	adv_monitor_set_rssi_sampling_period(sampling);
}

static void cmd_adv_monitor_add_or_monitor(int argc, char *argv[])
{
	adv_monitor_add_monitor(dbus_conn, "or_patterns", argc, argv);
}

static void cmd_adv_monitor_print_monitor(int argc, char *argv[])
{
	int monitor_idx;

	if (strcmp(argv[1], "all") == 0)
		monitor_idx = -1;
	else
		monitor_idx = atoi(argv[1]);
	adv_monitor_print_monitor(dbus_conn, monitor_idx);
}

static void cmd_adv_monitor_remove_monitor(int argc, char *argv[])
{
	int monitor_idx;

	if (strcmp(argv[1], "all") == 0)
		monitor_idx = -1;
	else
		monitor_idx = atoi(argv[1]);
	adv_monitor_remove_monitor(dbus_conn, monitor_idx);
}

static void cmd_adv_monitor_get_supported_info(int argc, char *argv[])
{
	adv_monitor_get_supported_info();
}

static const struct bt_shell_menu advertise_menu = {
	.name = "advertise",
	.desc = "Advertise Options Submenu",
	.entries = {
	{ "uuids", "[uuid1 uuid2 ...]", cmd_advertise_uuids,
			"Set/Get advertise uuids" },
	{ "service", "[uuid] [data=xx xx ...]", cmd_advertise_service,
			"Set/Get advertise service data" },
	{ "manufacturer", "[id] [data=xx xx ...]",
			cmd_advertise_manufacturer,
			"Set/Get advertise manufacturer data" },
	{ "data", "[type] [data=xx xx ...]", cmd_advertise_data,
			"Set/Get advertise data" },
	{ "discoverable", "[on/off]", cmd_advertise_discoverable,
			"Set/Get advertise discoverable" },
	{ "discoverable-timeout", "[seconds]",
			cmd_advertise_discoverable_timeout,
			"Set/Get advertise discoverable timeout" },
	{ "tx-power", "[on/off]", cmd_advertise_tx_power,
			"Show/Enable/Disable TX power to be advertised",
							NULL },
	{ "name", "[on/off/name]", cmd_advertise_name,
			"Configure local name to be advertised" },
	{ "appearance", "[on/off/value]", cmd_advertise_appearance,
			"Configure custom appearance to be advertised" },
	{ "duration", "[seconds]", cmd_advertise_duration,
			"Set/Get advertise duration" },
	{ "timeout", "[seconds]", cmd_advertise_timeout,
			"Set/Get advertise timeout" },
	{ "secondary", "[1M/2M/Coded]", cmd_advertise_secondary,
			"Set/Get advertise secondary channel" },
	{ "clear", "[uuids/service/manufacturer/config-name...]", cmd_ad_clear,
			"Clear advertise config" },
	{ } },
};

static const struct bt_shell_menu advertise_monitor_menu = {
	.name = "monitor",
	.desc = "Advertisement Monitor Options Submenu",
	.entries = {
	{ "set-rssi-threshold", "<low_threshold> <high_threshold>",
				cmd_adv_monitor_set_rssi_threshold,
				"Set RSSI threshold parameter" },
	{ "set-rssi-timeout", "<low_timeout> <high_timeout>",
				cmd_adv_monitor_set_rssi_timeout,
				"Set RSSI timeout parameter" },
	{ "set-rssi-sampling-period", "<sampling_period>",
				cmd_adv_monitor_set_rssi_sampling_period,
				"Set RSSI sampling period parameter" },
	{ "add-or-pattern", "[patterns=pattern1 pattern2 ...]",
				cmd_adv_monitor_add_or_monitor,
				"Register 'or pattern' type monitor with the "
				"specified RSSI parameters" },
	{ "get-pattern", "<monitor-id/all>",
				cmd_adv_monitor_print_monitor,
				"Get advertisement monitor" },
	{ "remove-pattern", "<monitor-id/all>",
				cmd_adv_monitor_remove_monitor,
				"Remove advertisement monitor" },
	{ "get-supported-info", NULL,
				cmd_adv_monitor_get_supported_info,
				"Get advertisement manager supported "
				"features and supported monitor types" },
	{ "print-usage", "<add-or-pattern>",
				cmd_adv_monitor_print_usage,
				"Print the command usage"},
	{ } },
};

static const struct bt_shell_menu scan_menu = {
	.name = "scan",
	.desc = "Scan Options Submenu",
	.entries = {
	{ "uuids", "[all/uuid1 uuid2 ...]", cmd_scan_filter_uuids,
				"Set/Get UUIDs filter" },
	{ "rssi", "[rssi]", cmd_scan_filter_rssi,
				"Set/Get RSSI filter, and clears pathloss" },
	{ "pathloss", "[pathloss]", cmd_scan_filter_pathloss,
				"Set/Get Pathloss filter, and clears RSSI" },
	{ "transport", "[transport]", cmd_scan_filter_transport,
				"Set/Get transport filter" },
	{ "duplicate-data", "[on/off]", cmd_scan_filter_duplicate_data,
				"Set/Get duplicate data filter",
				NULL },
	{ "discoverable", "[on/off]", cmd_scan_filter_discoverable,
				"Set/Get discoverable filter",
				NULL },
	{ "pattern", "[value]", cmd_scan_filter_pattern,
				"Set/Get pattern filter",
				NULL },
	{ "clear",
	"[uuids/rssi/pathloss/transport/duplicate-data/discoverable/pattern]",
				cmd_scan_filter_clear,
				"Clears discovery filter.",
				filter_clear_generator },
	{ } },
};

static const struct bt_shell_menu gatt_menu = {
	.name = "gatt",
	.desc = "Generic Attribute Submenu",
	.entries = {
	{ "list-attributes", "[dev/local]", cmd_list_attributes,
				"List attributes", dev_generator },
	{ "select-attribute", "<attribute/UUID>",  cmd_select_attribute,
				"Select attribute", attribute_generator },
	{ "attribute-info", "[attribute/UUID]",  cmd_attribute_info,
				"Select attribute", attribute_generator },
	{ "read", "[offset]", cmd_read, "Read attribute value" },
	{ "write", "<data=xx xx ...> [offset] [type]", cmd_write,
						"Write attribute value" },
	{ "acquire-write", NULL, cmd_acquire_write,
					"Acquire Write file descriptor" },
	{ "release-write", NULL, cmd_release_write,
					"Release Write file descriptor" },
	{ "acquire-notify", NULL, cmd_acquire_notify,
					"Acquire Notify file descriptor" },
	{ "release-notify", NULL, cmd_release_notify,
					"Release Notify file descriptor" },
	{ "notify",       "<on/off>", cmd_notify, "Notify attribute value",
							NULL },
	{ "clone",	  "[dev/attribute/UUID]", cmd_clone,
						"Clone a device or attribute" },
	{ "register-application", "[UUID ...]", cmd_register_app,
						"Register profile to connect" },
	{ "unregister-application", NULL, cmd_unregister_app,
						"Unregister profile" },
	{ "register-service", "<UUID> [handle]", cmd_register_service,
					"Register application service."  },
	{ "unregister-service", "<UUID/object>", cmd_unregister_service,
					"Unregister application service" },
	{ "register-includes", "<UUID> [handle]", cmd_register_includes,
					"Register as Included service in." },
	{ "unregister-includes", "<Service-UUID><Inc-UUID>",
			cmd_unregister_includes,
				 "Unregister Included service." },
	{ "register-characteristic",
			"<UUID> <Flags=read,write,notify...> [handle]",
			cmd_register_characteristic,
			"Register application characteristic" },
	{ "unregister-characteristic", "<UUID/object>",
				cmd_unregister_characteristic,
				"Unregister application characteristic" },
	{ "register-descriptor", "<UUID> <Flags=read,write...> [handle]",
					cmd_register_descriptor,
					"Register application descriptor" },
	{ "unregister-descriptor", "<UUID/object>",
					cmd_unregister_descriptor,
					"Unregister application descriptor" },
	{ } },
};

static const struct bt_shell_menu main_menu = {
	.name = "main",
	.entries = {
	{ "list",         NULL,       cmd_list, "List available controllers" },
	{ "show",         "[ctrl]",   cmd_show, "Controller information",
							ctrl_generator },
	{ "select",       "<ctrl>",   cmd_select, "Select default controller",
							ctrl_generator },
	{ "devices",      NULL,       cmd_devices, "List available devices" },
	{ "paired-devices", NULL,     cmd_paired_devices,
					"List paired devices"},
	{ "system-alias", "<name>",   cmd_system_alias,
					"Set controller alias" },
	{ "reset-alias",  NULL,       cmd_reset_alias,
					"Reset controller alias" },
	{ "power",        "<on/off>", cmd_power, "Set controller power",
							NULL },
	{ "pairable",     "<on/off>", cmd_pairable,
					"Set controller pairable mode",
							NULL },
	{ "discoverable", "<on/off>", cmd_discoverable,
					"Set controller discoverable mode",
							NULL },
	{ "discoverable-timeout", "[value]", cmd_discoverable_timeout,
					"Set discoverable timeout", NULL },
	{ "agent",        "<on/off/capability>", cmd_agent,
				"Enable/disable agent with given capability",
							capability_generator},
	{ "default-agent",NULL,       cmd_default_agent,
				"Set agent as the default one" },
	{ "advertise",    "<on/off/type>", cmd_advertise,
				"Enable/disable advertising with given type",
							ad_generator},
	{ "set-alias",    "<alias>",  cmd_set_alias, "Set device alias" },
	{ "scan",         "<on/off>", cmd_scan, "Scan for devices", NULL },
	{ "info",         "[dev]",    cmd_info, "Device information",
							dev_generator },
	{ "pair",         "[dev]",    cmd_pair, "Pair with device",
							dev_generator },
	{ "cancel-pairing",  "[dev]",    cmd_cancel_pairing,
				"Cancel pairing with device", dev_generator },
	{ "trust",        "[dev]",    cmd_trust, "Trust device",
							dev_generator },
	{ "untrust",      "[dev]",    cmd_untrust, "Untrust device",
							dev_generator },
	{ "block",        "[dev]",    cmd_block, "Block device",
								dev_generator },
	{ "unblock",      "[dev]",    cmd_unblock, "Unblock device",
								dev_generator },
	{ "remove",       "<dev>",    cmd_remove, "Remove device",
							dev_generator },
	{ "connect",      "<dev>",    cmd_connect, "Connect device",
							dev_generator },
	{ "disconnect",   "[dev]",    cmd_disconn, "Disconnect device",
							dev_generator },
	{ } },
};

static const struct option options[] = {
	{ "agent",	required_argument, 0, 'a' },
	{ 0, 0, 0, 0 }
};

static const char *agent_option;

static const char **optargs[] = {
	&agent_option
};

static const char *help[] = {
	"Register agent handler: <capability>"
};

static const struct bt_shell_opt opt = {
	.options = options,
	.optno = sizeof(options) / sizeof(struct option),
	.optstr = "a:",
	.optarg = optargs,
	.help = help,
};

static void client_ready(GDBusClient *client, void *user_data)
{
	setup_standard_input();
}

int main(int argc, char *argv[])
{
	GDBusClient *client;
	int status;

	bt_shell_init(argc, argv, &opt);
	bt_shell_set_menu(&main_menu);
	bt_shell_add_submenu(&advertise_menu);
	bt_shell_add_submenu(&advertise_monitor_menu);
	bt_shell_add_submenu(&scan_menu);
	bt_shell_add_submenu(&gatt_menu);
	bt_shell_set_prompt(PROMPT_OFF);

	if (agent_option)
		auto_register_agent = g_strdup(agent_option);
	else
		auto_register_agent = g_strdup("");

	dbus_conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, NULL);
	g_dbus_attach_object_manager(dbus_conn);

	bt_shell_set_env("DBUS_CONNECTION", dbus_conn);

	client = g_dbus_client_new(dbus_conn, "org.bluez", "/org/bluez");

	g_dbus_client_set_connect_watch(client, connect_handler, NULL);
	g_dbus_client_set_disconnect_watch(client, disconnect_handler, NULL);
	g_dbus_client_set_signal_watch(client, message_handler, NULL);

	g_dbus_client_set_proxy_handlers(client, proxy_added, proxy_removed,
							property_changed, NULL);

	g_dbus_client_set_ready_watch(client, client_ready, NULL);

	status = bt_shell_run();

	g_dbus_client_unref(client);

	dbus_connection_unref(dbus_conn);

	g_list_free_full(ctrl_list, proxy_leak);

	g_free(auto_register_agent);

	return status;
}
