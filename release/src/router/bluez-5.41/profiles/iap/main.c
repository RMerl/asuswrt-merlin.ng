/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Marcel Holtmann <marcel@holtmann.org>
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
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/signalfd.h>

#include <glib.h>

#include "gdbus/gdbus.h"

#define IAP_PATH "/org/bluez/iap"

#define IAP_UUID "00000000-deca-fade-deca-deafdecacafe"

#define IAP_RECORD							\
	"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>			\
	<record>							\
		<attribute id=\"0x0001\">				\
			<sequence>					\
				<uuid value=\"%s\" />			\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0002\">				\
			<uint32 value=\"0x00000000\" />			\
		</attribute>						\
		<attribute id=\"0x0004\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x0100\" />	\
				</sequence>				\
				<sequence>				\
					<uuid value=\"0x0003\" />	\
					<uint8 value=\"0x%02x\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0005\">				\
			<sequence>					\
				<uuid value=\"0x1002\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0006\">				\
			<sequence>					\
				<uint16 value=\"0x656e\" />		\
				<uint16 value=\"0x006a\" />		\
				<uint16 value=\"0x0100\" />		\
				<uint16 value=\"0x6672\" />		\
				<uint16 value=\"0x006a\" />		\
				<uint16 value=\"0x0110\" />		\
				<uint16 value=\"0x6465\" />		\
				<uint16 value=\"0x006a\" />		\
				<uint16 value=\"0x0120\" />		\
				<uint16 value=\"0x6a61\" />		\
				<uint16 value=\"0x006a\" />		\
				<uint16 value=\"0x0130\" />		\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0008\">				\
			<uint8 value=\"0xff\" />			\
		</attribute>						\
		<attribute id=\"0x0009\">				\
			<sequence>					\
				<sequence>				\
					<uuid value=\"0x1101\" />	\
					<uint16 value=\"0x0100\" />	\
				</sequence>				\
			</sequence>					\
		</attribute>						\
		<attribute id=\"0x0100\">				\
			<text value=\"Wireless iAP\" />			\
		</attribute>						\
	</record>"

static GMainLoop *main_loop;

static guint iap_source = 0;

static gboolean iap_handler(GIOChannel *channel, GIOCondition condition,
							gpointer user_data)
{
	unsigned char buf[512];
	ssize_t len;
	int fd;

	if (condition & (G_IO_NVAL | G_IO_ERR | G_IO_HUP)) {
		iap_source = 0;
		return FALSE;
	}

	fd = g_io_channel_unix_get_fd(channel);

	len = read(fd, buf, sizeof(buf));
	if (len < 0) {
		iap_source = 0;
		return FALSE;
	}

	return TRUE;
}

static guint create_source(int fd, GIOFunc func)
{
	GIOChannel *channel;
	guint source;

	channel = g_io_channel_unix_new(fd);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	source = g_io_add_watch(channel,
			G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL, func, NULL);

	g_io_channel_unref(channel);

	return source;
}

static void remove_source(const char *path)
{
	if (iap_source > 0) {
		g_source_remove(iap_source);
		iap_source = 0;
	}
}

static DBusMessage *release_profile(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	g_print("Profile released\n");

	remove_source(IAP_PATH);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *new_connection(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	const char *path, *device;
	int fd;

	g_print("New connection\n");

	path = dbus_message_get_path(msg);

	dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &device,
				DBUS_TYPE_UNIX_FD, &fd, DBUS_TYPE_INVALID);

	g_print("  from %s\n", path);
	g_print("  for device %s with fd %d\n", device, fd);

	if (iap_source == 0)
		iap_source = create_source(fd, iap_handler);
	else
		close(fd);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *request_disconnection(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	DBusMessageIter iter;
	const char *path, *device;

	g_print("Request disconnection\n");

	path = dbus_message_get_path(msg);

	dbus_message_iter_init(msg, &iter);
	dbus_message_iter_get_basic(&iter, &device);

	g_print("  from %s\n", path);
	g_print("  for device %s\n", device);

	remove_source(path);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *cancel_request(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	const char *path;

	g_print("Request canceled\n");

	path = dbus_message_get_path(msg);

	g_print("  from %s\n", path);

	remove_source(path);

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable methods[] = {
	{ GDBUS_METHOD("Release", NULL, NULL, release_profile) },
	{ GDBUS_METHOD("NewConnection",
			GDBUS_ARGS({ "device", "o" },
					{ "fd", "h"}, { "opts", "a{sv}"}),
			NULL, new_connection) },
	{ GDBUS_METHOD("RequestDisconnection",
			GDBUS_ARGS({ "device", "o" }),
			NULL, request_disconnection) },
	{ GDBUS_METHOD("Cancel", NULL, NULL, cancel_request) },
	{ }
};

static void register_profile_setup(DBusMessageIter *iter, void *user_data)
{
	const char *path = IAP_PATH;
	const char *uuid = IAP_UUID;
	DBusMessageIter dict, entry, value;
	dbus_uint16_t channel;
	char *record;
	const char *str;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &uuid);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
				DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
				DBUS_TYPE_STRING_AS_STRING
				DBUS_TYPE_VARIANT_AS_STRING
				DBUS_DICT_ENTRY_END_CHAR_AS_STRING, &dict);

	dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY,
							NULL, &entry);
	str = "Role";
	dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &str);
	dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT,
					DBUS_TYPE_STRING_AS_STRING, &value);
	str = "server";
	dbus_message_iter_append_basic(&value, DBUS_TYPE_STRING, &str);
	dbus_message_iter_close_container(&entry, &value);
	dbus_message_iter_close_container(&dict, &entry);

	dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY,
							NULL, &entry);
	str = "Channel";
	dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &str);
	dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT,
					DBUS_TYPE_UINT16_AS_STRING, &value);
	channel = 23;
	dbus_message_iter_append_basic(&value, DBUS_TYPE_UINT16, &channel);
	dbus_message_iter_close_container(&entry, &value);
	dbus_message_iter_close_container(&dict, &entry);

	dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY,
							NULL, &entry);
	str = "ServiceRecord";
	dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &str);
	dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT,
					DBUS_TYPE_STRING_AS_STRING, &value);
	record = g_strdup_printf(IAP_RECORD, IAP_UUID, channel);
	dbus_message_iter_append_basic(&value, DBUS_TYPE_STRING, &record);
	g_free(record);
	dbus_message_iter_close_container(&entry, &value);
	dbus_message_iter_close_container(&dict, &entry);

	dbus_message_iter_close_container(iter, &dict);
}

static void register_profile_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		g_print("Failed to register profile\n");
		return;
	}

	g_print("Profile registered\n");
}

static void connect_handler(DBusConnection *connection, void *user_data)
{
	GDBusClient *client = user_data;
	GDBusProxy *proxy;

	g_print("Bluetooth connected\n");

	proxy = g_dbus_proxy_new(client, "/org/bluez",
					"org.bluez.ProfileManager1");
	if (!proxy)
		return;

	g_dbus_register_interface(connection, IAP_PATH,
					"org.bluez.Profile1",
					methods, NULL, NULL, NULL, NULL);

	g_dbus_proxy_method_call(proxy, "RegisterProfile", 
					register_profile_setup,
					register_profile_reply, NULL, NULL);

	g_dbus_proxy_unref(proxy);
}

static void disconnect_handler(DBusConnection *connection, void *user_data)
{
	g_print("Bluetooth disconnected\n");

	g_dbus_unregister_interface(connection, IAP_PATH,
						"org.bluez.Profile1");
}

static gboolean signal_handler(GIOChannel *channel, GIOCondition condition,
							gpointer user_data)
{
	static unsigned int __terminated = 0;
	struct signalfd_siginfo si;
	ssize_t result;
	int fd;

	if (condition & (G_IO_NVAL | G_IO_ERR | G_IO_HUP)) {
		g_main_loop_quit(main_loop);
		return FALSE;
	}

	fd = g_io_channel_unix_get_fd(channel);

	result = read(fd, &si, sizeof(si));
	if (result != sizeof(si))
		return FALSE;

	switch (si.ssi_signo) {
	case SIGINT:
	case SIGTERM:
		if (__terminated == 0)
			g_main_loop_quit(main_loop);

		__terminated = 1;
		break;
	}

	return TRUE;
}

static guint setup_signalfd(void)
{
	GIOChannel *channel;
	guint source;
	sigset_t mask;
	int fd;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
		perror("Failed to set signal mask");
		return 0;
	}

	fd = signalfd(-1, &mask, 0);
	if (fd < 0) {
		perror("Failed to create signal descriptor");
		return 0;
	}

	channel = g_io_channel_unix_new(fd);

	g_io_channel_set_close_on_unref(channel, TRUE);
	g_io_channel_set_encoding(channel, NULL, NULL);
	g_io_channel_set_buffered(channel, FALSE);

	source = g_io_add_watch(channel,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				signal_handler, NULL);

	g_io_channel_unref(channel);

	return source;
}

static gboolean option_version = FALSE;

static GOptionEntry options[] = {
	{ "version", 'v', 0, G_OPTION_ARG_NONE, &option_version,
				"Show version information and exit" },
	{ NULL },
};

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GError *error = NULL;
	DBusConnection *dbus_conn;
	GDBusClient *client;
	guint signal;

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);

	if (g_option_context_parse(context, &argc, &argv, &error) == FALSE) {
		if (error != NULL) {
			g_printerr("%s\n", error->message);
			g_error_free(error);
		} else
			g_printerr("An unknown error occurred\n");
		exit(1);
	}

	g_option_context_free(context);

	if (option_version == TRUE) {
		g_print("%s\n", VERSION);
		exit(0);
	}

	main_loop = g_main_loop_new(NULL, FALSE);
	dbus_conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, NULL, NULL);

	signal = setup_signalfd();

	client = g_dbus_client_new(dbus_conn, "org.bluez", "/org/bluez");

	g_dbus_client_set_connect_watch(client, connect_handler, client);
	g_dbus_client_set_disconnect_watch(client, disconnect_handler, NULL);

	g_main_loop_run(main_loop);

	g_dbus_client_unref(client);

	g_source_remove(signal);

	dbus_connection_unref(dbus_conn);
	g_main_loop_unref(main_loop);

	return 0;
}
