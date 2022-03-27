/*
 *
 *  OBEX Client
 *
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <syslog.h>

#include <glib.h>

#include "gdbus/gdbus.h"

#include "obexd/src/log.h"
#include "obexd/src/manager.h"
#include "transfer.h"
#include "session.h"
#include "bluetooth.h"
#include "opp.h"
#include "ftp.h"
#include "pbap.h"
#include "sync.h"
#include "map.h"
#include "manager.h"

#define CLIENT_INTERFACE	"org.bluez.obex.Client1"
#define ERROR_INTERFACE		"org.bluez.obex.Error"
#define CLIENT_PATH		"/org/bluez/obex"

struct send_data {
	DBusConnection *connection;
	DBusMessage *message;
};

static GSList *sessions = NULL;

static void shutdown_session(struct obc_session *session)
{
	obc_session_shutdown(session);
	obc_session_unref(session);
}

static void release_session(struct obc_session *session)
{
	sessions = g_slist_remove(sessions, session);
	shutdown_session(session);
}

static void unregister_session(void *data)
{
	struct obc_session *session = data;

	if (g_slist_find(sessions, session) == NULL)
		return;

	sessions = g_slist_remove(sessions, session);
	obc_session_unref(session);
}

static void create_callback(struct obc_session *session,
						struct obc_transfer *transfer,
						GError *err, void *user_data)
{
	struct send_data *data = user_data;
	const char *path;

	if (err != NULL) {
		DBusMessage *error = g_dbus_create_error(data->message,
					ERROR_INTERFACE ".Failed",
					"%s", err->message);
		g_dbus_send_message(data->connection, error);
		shutdown_session(session);
		goto done;
	}


	path = obc_session_register(session, unregister_session);
	if (path == NULL) {
		DBusMessage *error = g_dbus_create_error(data->message,
					ERROR_INTERFACE ".Failed",
					NULL);
		g_dbus_send_message(data->connection, error);
		shutdown_session(session);
		goto done;
	}

	sessions = g_slist_append(sessions, session);
	g_dbus_send_reply(data->connection, data->message,
				DBUS_TYPE_OBJECT_PATH, &path,
				DBUS_TYPE_INVALID);

done:
	dbus_message_unref(data->message);
	dbus_connection_unref(data->connection);
	g_free(data);
}

static int parse_device_dict(DBusMessageIter *iter,
		const char **source, const char **target, uint8_t *channel)
{
	while (dbus_message_iter_get_arg_type(iter) == DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter entry, value;
		const char *key;

		dbus_message_iter_recurse(iter, &entry);
		dbus_message_iter_get_basic(&entry, &key);

		dbus_message_iter_next(&entry);
		dbus_message_iter_recurse(&entry, &value);

		switch (dbus_message_iter_get_arg_type(&value)) {
		case DBUS_TYPE_STRING:
			if (g_str_equal(key, "Source") == TRUE)
				dbus_message_iter_get_basic(&value, source);
			else if (g_str_equal(key, "Target") == TRUE)
				dbus_message_iter_get_basic(&value, target);
			break;
		case DBUS_TYPE_BYTE:
			if (g_str_equal(key, "Channel") == TRUE)
				dbus_message_iter_get_basic(&value, channel);
			break;
		}

		dbus_message_iter_next(iter);
	}

	return 0;
}

static struct obc_session *find_session(const char *path)
{
	GSList *l;

	for (l = sessions; l; l = l->next) {
		struct obc_session *session = l->data;

		if (g_strcmp0(obc_session_get_path(session), path) == 0)
			return session;
	}

	return NULL;
}

static DBusMessage *create_session(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	DBusMessageIter iter, dict;
	struct obc_session *session;
	struct send_data *data;
	const char *source = NULL, *dest = NULL, *target = NULL;
	uint8_t channel = 0;

	dbus_message_iter_init(message, &iter);
	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING)
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".InvalidArguments", NULL);

	dbus_message_iter_get_basic(&iter, &dest);
	dbus_message_iter_next(&iter);

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".InvalidArguments", NULL);

	dbus_message_iter_recurse(&iter, &dict);

	parse_device_dict(&dict, &source, &target, &channel);
	if (dest == NULL || target == NULL)
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".InvalidArguments", NULL);

	data = g_try_malloc0(sizeof(*data));
	if (data == NULL)
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".Error.NoMemory", NULL);

	data->connection = dbus_connection_ref(connection);
	data->message = dbus_message_ref(message);

	session = obc_session_create(source, dest, target, channel,
					dbus_message_get_sender(message),
					create_callback, data);
	if (session != NULL) {
		return NULL;
	}

	dbus_message_unref(data->message);
	dbus_connection_unref(data->connection);
	g_free(data);

	return g_dbus_create_error(message, ERROR_INTERFACE ".Failed", NULL);
}

static DBusMessage *remove_session(DBusConnection *connection,
				DBusMessage *message, void *user_data)
{
	struct obc_session *session;
	const char *sender, *path;

	if (dbus_message_get_args(message, NULL,
			DBUS_TYPE_OBJECT_PATH, &path,
			DBUS_TYPE_INVALID) == FALSE)
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".InvalidArguments", NULL);

	session = find_session(path);
	if (session == NULL)
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".InvalidArguments", NULL);

	sender = dbus_message_get_sender(message);
	if (g_str_equal(sender, obc_session_get_owner(session)) == FALSE)
		return g_dbus_create_error(message,
				ERROR_INTERFACE ".NotAuthorized",
				"Not Authorized");

	release_session(session);

	return dbus_message_new_method_return(message);
}

static const GDBusMethodTable client_methods[] = {
	{ GDBUS_ASYNC_METHOD("CreateSession",
			GDBUS_ARGS({ "destination", "s" }, { "args", "a{sv}" }),
			GDBUS_ARGS({ "session", "o" }), create_session) },
	{ GDBUS_ASYNC_METHOD("RemoveSession",
			GDBUS_ARGS({ "session", "o" }), NULL, remove_session) },
	{ }
};

static DBusConnection *conn = NULL;

static struct obc_module {
	const char *name;
	int (*init) (void);
	void (*exit) (void);
} modules[] = {
	{ "bluetooth", bluetooth_init, bluetooth_exit },
	{ "opp", opp_init, opp_exit },
	{ "ftp", ftp_init, ftp_exit },
	{ "pbap", pbap_init, pbap_exit },
	{ "sync", sync_init, sync_exit },
	{ "map", map_init, map_exit },
	{ }
};

int client_manager_init(void)
{
	DBusError derr;
	struct obc_module *module;

	dbus_error_init(&derr);

	conn = manager_dbus_get_connection();
	if (conn == NULL) {
		error("Can't get client D-Bus connection");
		return -1;
	}

	if (g_dbus_register_interface(conn, CLIENT_PATH, CLIENT_INTERFACE,
						client_methods, NULL, NULL,
							NULL, NULL) == FALSE) {
		error("Can't register client interface");
		dbus_connection_unref(conn);
		conn = NULL;
		return -1;
	}

	for (module = modules; module && module->init; module++) {
		if (module->init() < 0)
			continue;

		DBG("Module %s loaded", module->name);
	}

	return 0;
}

void client_manager_exit(void)
{
	struct obc_module *module;

	if (conn == NULL)
		return;

	for (module = modules; module && module->exit; module++)
		module->exit();

	g_dbus_unregister_interface(conn, CLIENT_PATH, CLIENT_INTERFACE);

	dbus_connection_unref(conn);
}
