/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
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

#include "gdbus/gdbus.h"

#include "src/shared/tester.h"
#include "emulator/hciemu.h"

static DBusConnection *dbus_conn = NULL;
static GDBusClient *dbus_client = NULL;
static GDBusProxy *adapter_proxy = NULL;

static struct hciemu *hciemu_stack = NULL;

static void connect_handler(DBusConnection *connection, void *user_data)
{
	tester_print("Connected to daemon");

	hciemu_stack = hciemu_new(HCIEMU_TYPE_BREDRLE);
}

static void disconnect_handler(DBusConnection *connection, void *user_data)
{
	tester_print("Disconnected from daemon");

	dbus_connection_unref(dbus_conn);
	dbus_conn = NULL;

	tester_teardown_complete();
}

static gboolean compare_string_property(GDBusProxy *proxy, const char *name,
							const char *value)
{
	DBusMessageIter iter;
	const char *str;

	if (g_dbus_proxy_get_property(proxy, name, &iter) == FALSE)
		return FALSE;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING)
		return FALSE;

	dbus_message_iter_get_basic(&iter, &str);

	return g_str_equal(str, value);
}

static void proxy_added(GDBusProxy *proxy, void *user_data)
{
	const char *interface;

	interface = g_dbus_proxy_get_interface(proxy);

	if (g_str_equal(interface, "org.bluez.Adapter1") == TRUE) {
		if (compare_string_property(proxy, "Address",
				hciemu_get_address(hciemu_stack)) == TRUE) {
			adapter_proxy = proxy;
			tester_print("Found adapter");

			tester_setup_complete();
		}
	}
}

static void proxy_removed(GDBusProxy *proxy, void *user_data)
{
	const char *interface;

	interface = g_dbus_proxy_get_interface(proxy);

	if (g_str_equal(interface, "org.bluez.Adapter1") == TRUE) {
		if (adapter_proxy == proxy) {
			adapter_proxy = NULL;
			tester_print("Adapter removed");

			g_dbus_client_unref(dbus_client);
			dbus_client = NULL;
		}
	}
}

static void test_setup(const void *test_data)
{
	dbus_conn = g_dbus_setup_private(DBUS_BUS_SYSTEM, NULL, NULL);

	dbus_client = g_dbus_client_new(dbus_conn, "org.bluez", "/org/bluez");

	g_dbus_client_set_connect_watch(dbus_client, connect_handler, NULL);
	g_dbus_client_set_disconnect_watch(dbus_client,
						disconnect_handler, NULL);

	g_dbus_client_set_proxy_handlers(dbus_client, proxy_added,
						proxy_removed, NULL, NULL);
}

static void test_run(const void *test_data)
{
	tester_test_passed();
}

static void test_teardown(const void *test_data)
{
	hciemu_unref(hciemu_stack);
	hciemu_stack = NULL;
}

int main(int argc, char *argv[])
{
	tester_init(&argc, &argv);

	tester_add("Adapter setup", NULL, test_setup, test_run, test_teardown);

	return tester_run();
}
