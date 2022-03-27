/*
 *
 *  OBEX Server
 *
 *  Copyright (C) 2007-2010  Nokia Corporation
 *  Copyright (C) 2007-2010  Marcel Holtmann <marcel@holtmann.org>
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
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "btio/btio.h"
#include "obexd/src/obexd.h"
#include "obexd/src/plugin.h"
#include "obexd/src/server.h"
#include "obexd/src/obex.h"
#include "obexd/src/transport.h"
#include "obexd/src/service.h"
#include "obexd/src/log.h"

#define BT_RX_MTU 32767
#define BT_TX_MTU 32767

struct bluetooth_profile {
	struct obex_server *server;
	struct obex_service_driver *driver;
	char *uuid;
	char *path;
};

static GSList *profiles = NULL;

static DBusConnection *connection = NULL;

static DBusMessage *profile_release(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	return g_dbus_create_reply(msg, DBUS_TYPE_INVALID);
}

static void connect_event(GIOChannel *io, GError *err, void *user_data)
{
	int sk = g_io_channel_unix_get_fd(io);
	struct bluetooth_profile *profile = user_data;
	struct obex_server *server = profile->server;
	int type;
	int omtu = BT_TX_MTU;
	int imtu = BT_RX_MTU;
	gboolean stream = TRUE;
	socklen_t len = sizeof(int);

	if (err)
		goto drop;

	if (getsockopt(sk, SOL_SOCKET, SO_TYPE, &type, &len) < 0)
		goto done;

	if (type != SOCK_SEQPACKET)
		goto done;

	stream = FALSE;

	/* Read MTU if io is an L2CAP socket */
	bt_io_get(io, NULL, BT_IO_OPT_OMTU, &omtu, BT_IO_OPT_IMTU, &imtu,
							BT_IO_OPT_INVALID);

done:
	if (obex_server_new_connection(server, io, omtu, imtu, stream) < 0)
		g_io_channel_shutdown(io, TRUE, NULL);

	return;

drop:
	error("%s", err->message);
	g_io_channel_shutdown(io, TRUE, NULL);
	return;
}

static DBusMessage *invalid_args(DBusMessage *msg)
{
	return g_dbus_create_error(msg, "org.bluez.Error.InvalidArguments",
					"Invalid arguments in method call");
}

static DBusMessage *profile_new_connection(DBusConnection *conn,
						DBusMessage *msg, void *data)
{
	DBusMessageIter args;
	const char *device;
	int fd;
	GIOChannel *io;

	dbus_message_iter_init(msg, &args);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_OBJECT_PATH)
		return invalid_args(msg);

	dbus_message_iter_get_basic(&args, &device);

	dbus_message_iter_next(&args);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_UNIX_FD)
		return invalid_args(msg);

	dbus_message_iter_get_basic(&args, &fd);

	if (fd < 0) {
		error("bluetooth: NewConnection invalid fd");
		return invalid_args(msg);
	}

	/* Read fd flags to make sure it can be used */
	if (fcntl(fd, F_GETFD) < 0) {
		error("bluetooth: fcntl(%d, F_GETFD): %s (%d)", fd,
						strerror(errno), errno);
		return invalid_args(msg);
	}

	io = g_io_channel_unix_new(fd);
	if (io == NULL)
		return invalid_args(msg);

	DBG("device %s", device);

	connect_event(io, NULL, data);
	g_io_channel_unref(io);

	return g_dbus_create_reply(msg, DBUS_TYPE_INVALID);
}

static DBusMessage *profile_request_disconnection(DBusConnection *conn,
						DBusMessage *msg, void *data)
{
	return g_dbus_create_reply(msg, DBUS_TYPE_INVALID);
}

static DBusMessage *profile_cancel(DBusConnection *conn,
						DBusMessage *msg, void *data)
{
	return g_dbus_create_reply(msg, DBUS_TYPE_INVALID);
}

static const GDBusMethodTable profile_methods[] = {
	{ GDBUS_METHOD("Release",
			NULL, NULL,
			profile_release) },
	{ GDBUS_METHOD("NewConnection",
			GDBUS_ARGS({ "device", "o" }, { "fd", "h" },
			{ "options", "a{sv}" }), NULL,
			profile_new_connection) },
	{ GDBUS_METHOD("RequestDisconnection",
			GDBUS_ARGS({ "device", "o" }), NULL,
			profile_request_disconnection) },
	{ GDBUS_METHOD("Cancel",
			NULL, NULL,
			profile_cancel) },
	{ }
};

static void unregister_profile(struct bluetooth_profile *profile)
{
	g_dbus_unregister_interface(connection, profile->path,
						"org.bluez.Profile1");
	g_free(profile->path);
	profile->path = NULL;
}

static void register_profile_reply(DBusPendingCall *call, void *user_data)
{
	struct bluetooth_profile *profile = user_data;
	DBusMessage *reply = dbus_pending_call_steal_reply(call);
	DBusError derr;

	dbus_error_init(&derr);
	if (!dbus_set_error_from_message(&derr, reply)) {
		DBG("Profile %s registered", profile->path);
		goto done;
	}

	unregister_profile(profile);

	error("bluetooth: RequestProfile error: %s, %s", derr.name,
								derr.message);
	dbus_error_free(&derr);
done:
	dbus_message_unref(reply);
}

static void profile_free(void *data)
{
	struct bluetooth_profile *profile = data;

	if (profile->path != NULL)
		unregister_profile(profile);

	g_free(profile->uuid);
	g_free(profile);
}

static void append_variant(DBusMessageIter *iter, int type, void *val)
{
	DBusMessageIter value;
	char sig[2] = { type, '\0' };

	dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT, sig, &value);

	dbus_message_iter_append_basic(&value, type, val);

	dbus_message_iter_close_container(iter, &value);
}


static void dict_append_entry(DBusMessageIter *dict,
			const char *key, int type, void *val)
{
	DBusMessageIter entry;

	if (type == DBUS_TYPE_STRING) {
		const char *str = *((const char **) val);
		if (str == NULL)
			return;
	}

	dbus_message_iter_open_container(dict, DBUS_TYPE_DICT_ENTRY,
							NULL, &entry);

	dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);

	append_variant(&entry, type, val);

	dbus_message_iter_close_container(dict, &entry);
}

static int register_profile(struct bluetooth_profile *profile)
{
	DBusMessage *msg;
	DBusMessageIter iter, opt;
	DBusPendingCall *call;
	dbus_bool_t auto_connect = FALSE;
	char *xml;
	int ret = 0;

	profile->path = g_strconcat("/org/bluez/obex/", profile->uuid, NULL);
	g_strdelimit(profile->path, "-", '_');

	if (!g_dbus_register_interface(connection, profile->path,
					"org.bluez.Profile1", profile_methods,
					NULL, NULL,
					profile, NULL)) {
		error("D-Bus failed to register %s", profile->path);
		g_free(profile->path);
		profile->path = NULL;
		return -1;
	}

	msg = dbus_message_new_method_call("org.bluez", "/org/bluez",
						"org.bluez.ProfileManager1",
						"RegisterProfile");

	dbus_message_iter_init_append(msg, &iter);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH,
							&profile->path);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING,
							&profile->uuid);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&opt);
	dict_append_entry(&opt, "AutoConnect", DBUS_TYPE_BOOLEAN,
								&auto_connect);
	if (profile->driver->record) {
		if (profile->driver->port != 0)
			xml = g_markup_printf_escaped(profile->driver->record,
						profile->driver->channel,
						profile->driver->name,
						profile->driver->port);
		else
			xml = g_markup_printf_escaped(profile->driver->record,
						profile->driver->channel,
						profile->driver->name);
		dict_append_entry(&opt, "ServiceRecord", DBUS_TYPE_STRING,
								&xml);
		g_free(xml);
	}
	dbus_message_iter_close_container(&iter, &opt);

	if (!g_dbus_send_message_with_reply(connection, msg, &call, -1)) {
		ret = -1;
		unregister_profile(profile);
		goto failed;
	}

	dbus_pending_call_set_notify(call, register_profile_reply, profile,
									NULL);
	dbus_pending_call_unref(call);

failed:
	dbus_message_unref(msg);
	return ret;
}

static const char *service2uuid(uint16_t service)
{
	switch (service) {
	case OBEX_OPP:
		return OBEX_OPP_UUID;
	case OBEX_FTP:
		return OBEX_FTP_UUID;
	case OBEX_PBAP:
		return OBEX_PSE_UUID;
	case OBEX_IRMC:
		return OBEX_SYNC_UUID;
	case OBEX_PCSUITE:
		return "00005005-0000-1000-8000-0002ee000001";
	case OBEX_SYNCEVOLUTION:
		return "00000002-0000-1000-8000-0002ee000002";
	case OBEX_MAS:
		return OBEX_MAS_UUID;
	case OBEX_MNS:
		return OBEX_MNS_UUID;
	}

	return NULL;
}

static void name_acquired(DBusConnection *conn, void *user_data)
{
	GSList *l;

	DBG("org.bluez appeared");

	for (l = profiles; l; l = l->next) {
		struct bluetooth_profile *profile = l->data;

		if (profile->path != NULL)
			continue;

		if (register_profile(profile) < 0) {
			error("bluetooth: Failed to register profile %s",
							profile->path);
			g_free(profile->path);
			profile->path = NULL;
		}
	}
}

static void name_released(DBusConnection *conn, void *user_data)
{
	GSList *l;

	DBG("org.bluez disappered");

	for (l = profiles; l; l = l->next) {
		struct bluetooth_profile *profile = l->data;

		if (profile->path == NULL)
			continue;

		unregister_profile(profile);
	}
}

static void *bluetooth_start(struct obex_server *server, int *err)
{
	const GSList *l;

	for (l = server->drivers; l; l = l->next) {
		struct obex_service_driver *driver = l->data;
		struct bluetooth_profile *profile;
		const char *uuid;

		uuid = service2uuid(driver->service);
		if (uuid == NULL)
			continue;

		profile = g_new0(struct bluetooth_profile, 1);
		profile->driver = driver;
		profile->server = server;
		profile->uuid = g_strdup(uuid);

		profiles = g_slist_prepend(profiles, profile);
	}

	return profiles;
}

static void bluetooth_stop(void *data)
{
	g_slist_free_full(profiles, profile_free);
	profiles = NULL;
}

static int bluetooth_getpeername(GIOChannel *io, char **name)
{
	GError *gerr = NULL;
	char address[18];

	bt_io_get(io, &gerr, BT_IO_OPT_DEST, address, BT_IO_OPT_INVALID);

	if (gerr) {
		error("%s", gerr->message);
		g_error_free(gerr);
		return -EINVAL;
	}

	*name = g_strdup(address);

	return 0;
}

static int bluetooth_getsockname(GIOChannel *io, char **name)
{
	GError *gerr = NULL;
	char address[18];

	bt_io_get(io, &gerr, BT_IO_OPT_SOURCE, address, BT_IO_OPT_INVALID);

	if (gerr) {
		error("%s", gerr->message);
		g_error_free(gerr);
		return -EINVAL;
	}

	*name = g_strdup(address);

	return 0;
}

static struct obex_transport_driver driver = {
	.name = "bluetooth",
	.start = bluetooth_start,
	.getpeername = bluetooth_getpeername,
	.getsockname = bluetooth_getsockname,
	.stop = bluetooth_stop
};

static unsigned int listener_id = 0;

static int bluetooth_init(void)
{
	connection = g_dbus_setup_private(DBUS_BUS_SYSTEM, NULL, NULL);
	if (connection == NULL)
		return -EPERM;

	listener_id = g_dbus_add_service_watch(connection, "org.bluez",
				name_acquired, name_released, NULL, NULL);

	return obex_transport_driver_register(&driver);
}

static void bluetooth_exit(void)
{
	g_dbus_remove_watch(connection, listener_id);

	g_slist_free_full(profiles, profile_free);

	if (connection)
		dbus_connection_unref(connection);

	obex_transport_driver_unregister(&driver);
}

OBEX_PLUGIN_DEFINE(bluetooth, bluetooth_init, bluetooth_exit)
