/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"

#include "gdbus/gdbus.h"

#include "src/dbus-common.h"
#include "src/plugin.h"
#include "src/adapter.h"
#include "src/log.h"

/* http://www.bluetooth.org/Technical/AssignedNumbers/baseband.htm */

#define MAJOR_CLASS_MISCELLANEOUS	0x00
#define MAJOR_CLASS_COMPUTER		0x01

#define MINOR_CLASS_UNCATEGORIZED	0x00
#define MINOR_CLASS_DESKTOP		0x01
#define MINOR_CLASS_SERVER		0x02
#define MINOR_CLASS_LAPTOP		0x03
#define MINOR_CLASS_HANDHELD		0x04
#define MINOR_CLASS_PALM_SIZED		0x05
#define MINOR_CLASS_WEARABLE		0x06
#define MINOR_CLASS_TABLET		0x07

static uint8_t major_class = MAJOR_CLASS_MISCELLANEOUS;
static uint8_t minor_class = MINOR_CLASS_UNCATEGORIZED;

static char *pretty_hostname = NULL;
static char *static_hostname = NULL;

/*
 * Fallback to static hostname only if empty pretty hostname was already
 * received.
 */
static const char *get_hostname(void)
{
	if (pretty_hostname) {
		if (g_str_equal(pretty_hostname, "") == FALSE)
			return pretty_hostname;

		if (static_hostname &&
				g_str_equal(static_hostname, "") == FALSE)
			return static_hostname;
	}

	return NULL;
}

static void update_name(struct btd_adapter *adapter, gpointer user_data)
{
	const char *hostname = get_hostname();

	if (hostname == NULL)
		return;

	if (btd_adapter_is_default(adapter)) {
		DBG("name: %s", hostname);

		adapter_set_name(adapter, hostname);
	} else {
		uint16_t index = btd_adapter_get_index(adapter);
		char *str;

		/* Avoid "some device #0" names, start at #1 */
		str = g_strdup_printf("%s #%u", hostname, index + 1);

		DBG("name: %s", str);

		adapter_set_name(adapter, str);

		g_free(str);
	}
}

static void update_class(struct btd_adapter *adapter, gpointer user_data)
{
	if (major_class == MAJOR_CLASS_MISCELLANEOUS)
		return;

	DBG("major: 0x%02x minor: 0x%02x", major_class, minor_class);

	btd_adapter_set_class(adapter, major_class, minor_class);
}

static const struct {
	const char *chassis;
	uint8_t major_class;
	uint8_t minor_class;
} chassis_table[] = {
	{ "desktop",  MAJOR_CLASS_COMPUTER, MINOR_CLASS_DESKTOP  },
	{ "server",   MAJOR_CLASS_COMPUTER, MINOR_CLASS_SERVER   },
	{ "laptop",   MAJOR_CLASS_COMPUTER, MINOR_CLASS_LAPTOP   },
	{ "handset",  MAJOR_CLASS_COMPUTER, MINOR_CLASS_HANDHELD },
	{ "tablet",   MAJOR_CLASS_COMPUTER, MINOR_CLASS_TABLET   },
	{ }
};

static void property_changed(GDBusProxy *proxy, const char *name,
					DBusMessageIter *iter, void *user_data)
{
	if (g_str_equal(name, "PrettyHostname") == TRUE) {
		if (iter == NULL) {
			g_dbus_proxy_refresh_property(proxy, name);
			return;
		}

		if (dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_STRING) {
			const char *str;

			dbus_message_iter_get_basic(iter, &str);

			DBG("pretty hostname: %s", str);

			g_free(pretty_hostname);
			pretty_hostname = g_strdup(str);

			adapter_foreach(update_name, NULL);
		}
	} else if (g_str_equal(name, "StaticHostname") == TRUE) {
		if (iter == NULL) {
			g_dbus_proxy_refresh_property(proxy, name);
			return;
		}

		if (dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_STRING) {
			const char *str;

			dbus_message_iter_get_basic(iter, &str);

			DBG("static hostname: %s", str);

			g_free(static_hostname);
			static_hostname = g_strdup(str);

			adapter_foreach(update_name, NULL);
		}
	} else if (g_str_equal(name, "Chassis") == TRUE) {
		if (iter == NULL) {
			g_dbus_proxy_refresh_property(proxy, name);
			return;
		}

		if (dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_STRING) {
			const char *str;
			int i;

			dbus_message_iter_get_basic(iter, &str);

			DBG("chassis: %s", str);

			for (i = 0; chassis_table[i].chassis; i++) {
				if (strcmp(chassis_table[i].chassis, str))
					continue;

				major_class = chassis_table[i].major_class;
				minor_class = chassis_table[i].minor_class;

				adapter_foreach(update_class, NULL);
				break;
			}
		}
	}
}

static int hostname_probe(struct btd_adapter *adapter)
{
	DBG("");

	update_name(adapter, NULL);
	update_class(adapter, NULL);

	return 0;
}

static void hostname_remove(struct btd_adapter *adapter)
{
	DBG("");
}

static struct btd_adapter_driver hostname_driver = {
	.name	= "hostname",
	.probe	= hostname_probe,
	.remove	= hostname_remove,
};

static void read_dmi_fallback(void)
{
	char *contents;
	int i, type;
	const char *str;

	if (g_file_get_contents("/sys/class/dmi/id/chassis_type",
					&contents, NULL, NULL) == FALSE)
		return;

	type = atoi(contents);
	if (type < 0 || type > 0x1D)
		return;

	g_free(contents);

	/* from systemd hostname chassis list */
	switch (type) {
	case 0x3:
	case 0x4:
	case 0x6:
	case 0x7:
		str = "desktop";
		break;
	case 0x8:
	case 0x9:
	case 0xA:
	case 0xE:
		str = "laptop";
		break;
	case 0xB:
		str = "handset";
		break;
	case 0x11:
	case 0x1C:
		str = "server";
		break;
	default:
		return;
	}

	DBG("chassis: %s", str);

	for (i = 0; chassis_table[i].chassis; i++) {
		if (!strcmp(chassis_table[i].chassis, str)) {
			major_class = chassis_table[i].major_class;
			minor_class = chassis_table[i].minor_class;
			break;
		}
	}

	DBG("major: 0x%02x minor: 0x%02x", major_class, minor_class);
}

static GDBusClient *hostname_client = NULL;
static GDBusProxy *hostname_proxy = NULL;

static int hostname_init(void)
{
	DBusConnection *conn = btd_get_dbus_connection();
	int err;

	read_dmi_fallback();

	hostname_client = g_dbus_client_new(conn, "org.freedesktop.hostname1",
						"/org/freedesktop/hostname1");
	if (!hostname_client)
		return -EIO;

	hostname_proxy = g_dbus_proxy_new(hostname_client,
						"/org/freedesktop/hostname1",
						"org.freedesktop.hostname1");
	if (!hostname_proxy) {
		g_dbus_client_unref(hostname_client);
		hostname_client = NULL;
		return -EIO;
	}

	g_dbus_proxy_set_property_watch(hostname_proxy, property_changed, NULL);

	err = btd_register_adapter_driver(&hostname_driver);
	if (err < 0) {
		g_dbus_proxy_unref(hostname_proxy);
		hostname_proxy = NULL;
		g_dbus_client_unref(hostname_client);
		hostname_client = NULL;
	}

	return err;
}

static void hostname_exit(void)
{
	btd_unregister_adapter_driver(&hostname_driver);

	if (hostname_proxy) {
		g_dbus_proxy_unref(hostname_proxy);
		hostname_proxy = NULL;
	}

	if (hostname_client) {
		g_dbus_client_unref(hostname_client);
		hostname_client = NULL;
	}

	g_free(pretty_hostname);
	g_free(static_hostname);
}

BLUETOOTH_PLUGIN_DEFINE(hostname, VERSION, BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
						hostname_init, hostname_exit)
