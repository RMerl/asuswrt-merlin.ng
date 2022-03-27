/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
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

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/uio.h>
#include <wordexp.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <glib.h>

#include "gdbus/gdbus.h"
#include "monitor/uuid.h"
#include "display.h"
#include "gatt.h"

#define PROFILE_PATH "/org/bluez/profile"
#define PROFILE_INTERFACE "org.bluez.GattProfile1"

/* String display constants */
#define COLORED_NEW	COLOR_GREEN "NEW" COLOR_OFF
#define COLORED_CHG	COLOR_YELLOW "CHG" COLOR_OFF
#define COLORED_DEL	COLOR_RED "DEL" COLOR_OFF

static GList *services;
static GList *characteristics;
static GList *descriptors;
static GList *managers;

static void print_service(GDBusProxy *proxy, const char *description)
{
	DBusMessageIter iter;
	const char *uuid, *text;
	dbus_bool_t primary;

	if (g_dbus_proxy_get_property(proxy, "UUID", &iter) == FALSE)
		return;

	dbus_message_iter_get_basic(&iter, &uuid);

	if (g_dbus_proxy_get_property(proxy, "Primary", &iter) == FALSE)
		return;

	dbus_message_iter_get_basic(&iter, &primary);

	text = uuidstr_to_str(uuid);
	if (!text)
		text = uuid;

	rl_printf("%s%s%s%s Service\n\t%s\n\t%s\n",
				description ? "[" : "",
				description ? : "",
				description ? "] " : "",
				primary ? "Primary" : "Secondary",
				g_dbus_proxy_get_path(proxy),
				text);
}

void gatt_add_service(GDBusProxy *proxy)
{
	services = g_list_append(services, proxy);

	print_service(proxy, COLORED_NEW);
}

void gatt_remove_service(GDBusProxy *proxy)
{
	GList *l;

	l = g_list_find(services, proxy);
	if (!l)
		return;

	services = g_list_delete_link(services, l);

	print_service(proxy, COLORED_DEL);
}

static void print_characteristic(GDBusProxy *proxy, const char *description)
{
	DBusMessageIter iter;
	const char *uuid, *text;

	if (g_dbus_proxy_get_property(proxy, "UUID", &iter) == FALSE)
		return;

	dbus_message_iter_get_basic(&iter, &uuid);

	text = uuidstr_to_str(uuid);
	if (!text)
		text = uuid;

	rl_printf("%s%s%sCharacteristic\n\t%s\n\t%s\n",
				description ? "[" : "",
				description ? : "",
				description ? "] " : "",
				g_dbus_proxy_get_path(proxy),
				text);
}

static gboolean characteristic_is_child(GDBusProxy *characteristic)
{
	GList *l;
	DBusMessageIter iter;
	const char *service, *path;

	if (!g_dbus_proxy_get_property(characteristic, "Service", &iter))
		return FALSE;

	dbus_message_iter_get_basic(&iter, &service);

	for (l = services; l; l = g_list_next(l)) {
		GDBusProxy *proxy = l->data;

		path = g_dbus_proxy_get_path(proxy);

		if (!strcmp(path, service))
			return TRUE;
	}

	return FALSE;
}

void gatt_add_characteristic(GDBusProxy *proxy)
{
	if (!characteristic_is_child(proxy))
		return;

	characteristics = g_list_append(characteristics, proxy);

	print_characteristic(proxy, COLORED_NEW);
}

void gatt_remove_characteristic(GDBusProxy *proxy)
{
	GList *l;

	l = g_list_find(characteristics, proxy);
	if (!l)
		return;

	characteristics = g_list_delete_link(characteristics, l);

	print_characteristic(proxy, COLORED_DEL);
}

static void print_descriptor(GDBusProxy *proxy, const char *description)
{
	DBusMessageIter iter;
	const char *uuid, *text;

	if (g_dbus_proxy_get_property(proxy, "UUID", &iter) == FALSE)
		return;

	dbus_message_iter_get_basic(&iter, &uuid);

	text = uuidstr_to_str(uuid);
	if (!text)
		text = uuid;

	rl_printf("%s%s%sDescriptor\n\t%s\n\t%s\n",
				description ? "[" : "",
				description ? : "",
				description ? "] " : "",
				g_dbus_proxy_get_path(proxy),
				text);
}

static gboolean descriptor_is_child(GDBusProxy *characteristic)
{
	GList *l;
	DBusMessageIter iter;
	const char *service, *path;

	if (!g_dbus_proxy_get_property(characteristic, "Characteristic", &iter))
		return FALSE;

	dbus_message_iter_get_basic(&iter, &service);

	for (l = characteristics; l; l = g_list_next(l)) {
		GDBusProxy *proxy = l->data;

		path = g_dbus_proxy_get_path(proxy);

		if (!strcmp(path, service))
			return TRUE;
	}

	return FALSE;
}

void gatt_add_descriptor(GDBusProxy *proxy)
{
	if (!descriptor_is_child(proxy))
		return;

	descriptors = g_list_append(descriptors, proxy);

	print_descriptor(proxy, COLORED_NEW);
}

void gatt_remove_descriptor(GDBusProxy *proxy)
{
	GList *l;

	l = g_list_find(descriptors, proxy);
	if (!l)
		return;

	descriptors = g_list_delete_link(descriptors, l);

	print_descriptor(proxy, COLORED_DEL);
}

static void list_attributes(const char *path, GList *source)
{
	GList *l;

	for (l = source; l; l = g_list_next(l)) {
		GDBusProxy *proxy = l->data;
		const char *proxy_path;

		proxy_path = g_dbus_proxy_get_path(proxy);

		if (!g_str_has_prefix(proxy_path, path))
			continue;

		if (source == services) {
			print_service(proxy, NULL);
			list_attributes(proxy_path, characteristics);
		} else if (source == characteristics) {
			print_characteristic(proxy, NULL);
			list_attributes(proxy_path, descriptors);
		} else if (source == descriptors)
			print_descriptor(proxy, NULL);
	}
}

void gatt_list_attributes(const char *path)
{
	list_attributes(path, services);
}

static GDBusProxy *select_proxy(const char *path, GList *source)
{
	GList *l;

	for (l = source; l; l = g_list_next(l)) {
		GDBusProxy *proxy = l->data;

		if (strcmp(path, g_dbus_proxy_get_path(proxy)) == 0)
			return proxy;
	}

	return NULL;
}

GDBusProxy *gatt_select_attribute(const char *path)
{
	GDBusProxy *proxy;

	proxy = select_proxy(path, services);
	if (proxy)
		return proxy;

	proxy = select_proxy(path, characteristics);
	if (proxy)
		return proxy;

	return select_proxy(path, descriptors);
}

static char *attribute_generator(const char *text, int state, GList *source)
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
		const char *path;

		index++;

		path = g_dbus_proxy_get_path(proxy);

		if (!strncmp(path, text, len))
			return strdup(path);
        }

	return NULL;
}

char *gatt_attribute_generator(const char *text, int state)
{
	static GList *list = NULL;

	if (!state) {
		GList *list1;

		if (list) {
			g_list_free(list);
			list = NULL;
		}

		list1 = g_list_copy(characteristics);
		list1 = g_list_concat(list1, g_list_copy(descriptors));

		list = g_list_copy(services);
		list = g_list_concat(list, list1);
	}

	return attribute_generator(text, state, list);
}

static void read_reply(DBusMessage *message, void *user_data)
{
	DBusError error;
	DBusMessageIter iter, array;
	uint8_t *value;
	int len;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to read: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	dbus_message_iter_init(message, &iter);

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY) {
		rl_printf("Invalid response to read\n");
		return;
	}

	dbus_message_iter_recurse(&iter, &array);
	dbus_message_iter_get_fixed_array(&array, &value, &len);

	if (len < 0) {
		rl_printf("Unable to parse value\n");
		return;
	}

	rl_hexdump(value, len);
}

static void read_setup(DBusMessageIter *iter, void *user_data)
{
	DBusMessageIter dict;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);
	/* TODO: Add offset support */
	dbus_message_iter_close_container(iter, &dict);
}

static void read_attribute(GDBusProxy *proxy)
{
	if (g_dbus_proxy_method_call(proxy, "ReadValue", read_setup, read_reply,
							NULL, NULL) == FALSE) {
		rl_printf("Failed to read\n");
		return;
	}

	rl_printf("Attempting to read %s\n", g_dbus_proxy_get_path(proxy));
}

void gatt_read_attribute(GDBusProxy *proxy)
{
	const char *iface;

	iface = g_dbus_proxy_get_interface(proxy);
	if (!strcmp(iface, "org.bluez.GattCharacteristic1") ||
				!strcmp(iface, "org.bluez.GattDescriptor1")) {
		read_attribute(proxy);
		return;
	}

	rl_printf("Unable to read attribute %s\n",
						g_dbus_proxy_get_path(proxy));
}

static void write_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to write: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}
}

static void write_setup(DBusMessageIter *iter, void *user_data)
{
	struct iovec *iov = user_data;
	DBusMessageIter array, dict;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "y", &array);
	dbus_message_iter_append_fixed_array(&array, DBUS_TYPE_BYTE,
						&iov->iov_base, iov->iov_len);
	dbus_message_iter_close_container(iter, &array);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);
	/* TODO: Add offset support */
	dbus_message_iter_close_container(iter, &dict);
}

static void write_attribute(GDBusProxy *proxy, char *arg)
{
	struct iovec iov;
	uint8_t value[512];
	char *entry;
	unsigned int i;

	for (i = 0; (entry = strsep(&arg, " \t")) != NULL; i++) {
		long int val;
		char *endptr = NULL;

		if (*entry == '\0')
			continue;

		if (i >= G_N_ELEMENTS(value)) {
			rl_printf("Too much data\n");
			return;
		}

		val = strtol(entry, &endptr, 0);
		if (!endptr || *endptr != '\0' || val > UINT8_MAX) {
			rl_printf("Invalid value at index %d\n", i);
			return;
		}

		value[i] = val;
	}

	iov.iov_base = value;
	iov.iov_len = i;

	if (g_dbus_proxy_method_call(proxy, "WriteValue", write_setup,
					write_reply, &iov, NULL) == FALSE) {
		rl_printf("Failed to write\n");
		return;
	}

	rl_printf("Attempting to write %s\n", g_dbus_proxy_get_path(proxy));
}

void gatt_write_attribute(GDBusProxy *proxy, const char *arg)
{
	const char *iface;

	iface = g_dbus_proxy_get_interface(proxy);
	if (!strcmp(iface, "org.bluez.GattCharacteristic1") ||
				!strcmp(iface, "org.bluez.GattDescriptor1")) {
		write_attribute(proxy, (char *) arg);
		return;
	}

	rl_printf("Unable to write attribute %s\n",
						g_dbus_proxy_get_path(proxy));
}

static void notify_reply(DBusMessage *message, void *user_data)
{
	bool enable = GPOINTER_TO_UINT(user_data);
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to %s notify: %s\n",
				enable ? "start" : "stop", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("Notify %s\n", enable == TRUE ? "started" : "stopped");
}

static void notify_attribute(GDBusProxy *proxy, bool enable)
{
	const char *method;

	if (enable == TRUE)
		method = "StartNotify";
	else
		method = "StopNotify";

	if (g_dbus_proxy_method_call(proxy, method, NULL, notify_reply,
				GUINT_TO_POINTER(enable), NULL) == FALSE) {
		rl_printf("Failed to %s notify\n", enable ? "start" : "stop");
		return;
	}
}

void gatt_notify_attribute(GDBusProxy *proxy, bool enable)
{
	const char *iface;

	iface = g_dbus_proxy_get_interface(proxy);
	if (!strcmp(iface, "org.bluez.GattCharacteristic1")) {
		notify_attribute(proxy, enable);
		return;
	}

	rl_printf("Unable to notify attribute %s\n",
						g_dbus_proxy_get_path(proxy));
}

static void register_profile_setup(DBusMessageIter *iter, void *user_data)
{
	wordexp_t *w = user_data;
	DBusMessageIter uuids, opt;
	const char *path = PROFILE_PATH;
	size_t i;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "s", &uuids);
	for (i = 0; i < w->we_wordc; i++)
		dbus_message_iter_append_basic(&uuids, DBUS_TYPE_STRING,
							&w->we_wordv[i]);
	dbus_message_iter_close_container(iter, &uuids);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&opt);
	dbus_message_iter_close_container(iter, &opt);

}

static void register_profile_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to register profile: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("Profile registered\n");
}

void gatt_add_manager(GDBusProxy *proxy)
{
	managers = g_list_append(managers, proxy);
}

void gatt_remove_manager(GDBusProxy *proxy)
{
	managers = g_list_remove(managers, proxy);
}

static int match_proxy(const void *a, const void *b)
{
	GDBusProxy *proxy1 = (void *) a;
	GDBusProxy *proxy2 = (void *) b;

	return strcmp(g_dbus_proxy_get_path(proxy1),
						g_dbus_proxy_get_path(proxy2));
}

static DBusMessage *release_profile(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	g_dbus_unregister_interface(conn, PROFILE_PATH, PROFILE_INTERFACE);

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable methods[] = {
	{ GDBUS_METHOD("Release", NULL, NULL, release_profile) },
	{ }
};

void gatt_register_profile(DBusConnection *conn, GDBusProxy *proxy,
								wordexp_t *w)
{
	GList *l;

	l = g_list_find_custom(managers, proxy, match_proxy);
	if (!l) {
		rl_printf("Unable to find GattManager proxy\n");
		return;
	}

	if (g_dbus_register_interface(conn, PROFILE_PATH,
					PROFILE_INTERFACE, methods,
					NULL, NULL, NULL, NULL) == FALSE) {
		rl_printf("Failed to register profile object\n");
		return;
	}

	if (g_dbus_proxy_method_call(l->data, "RegisterProfile",
						register_profile_setup,
						register_profile_reply, w,
						NULL) == FALSE) {
		rl_printf("Failed register profile\n");
		return;
	}
}

static void unregister_profile_reply(DBusMessage *message, void *user_data)
{
	DBusConnection *conn = user_data;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to unregister profile: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("Profile unregistered\n");

	g_dbus_unregister_interface(conn, PROFILE_PATH, PROFILE_INTERFACE);
}

static void unregister_profile_setup(DBusMessageIter *iter, void *user_data)
{
	const char *path = PROFILE_PATH;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
}

void gatt_unregister_profile(DBusConnection *conn, GDBusProxy *proxy)
{
	GList *l;

	l = g_list_find_custom(managers, proxy, match_proxy);
	if (!l) {
		rl_printf("Unable to find GattManager proxy\n");
		return;
	}

	if (g_dbus_proxy_method_call(l->data, "UnregisterProfile",
						unregister_profile_setup,
						unregister_profile_reply, conn,
						NULL) == FALSE) {
		rl_printf("Failed unregister profile\n");
		return;
	}
}
