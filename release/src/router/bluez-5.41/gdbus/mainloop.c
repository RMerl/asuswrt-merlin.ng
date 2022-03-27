/*
 *
 *  D-Bus helper library
 *
 *  Copyright (C) 2004-2011  Marcel Holtmann <marcel@holtmann.org>
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

#include <glib.h>
#include <dbus/dbus.h>

#include "gdbus.h"

#define info(fmt...)
#define error(fmt...)
#define debug(fmt...)

struct timeout_handler {
	guint id;
	DBusTimeout *timeout;
};

struct watch_info {
	guint id;
	DBusWatch *watch;
	DBusConnection *conn;
};

struct disconnect_data {
	GDBusWatchFunction function;
	void *user_data;
};

static gboolean disconnected_signal(DBusConnection *conn,
						DBusMessage *msg, void *data)
{
	struct disconnect_data *dc_data = data;

	error("Got disconnected from the system message bus");

	dc_data->function(conn, dc_data->user_data);

	dbus_connection_unref(conn);

	return TRUE;
}

static gboolean message_dispatch(void *data)
{
	DBusConnection *conn = data;

	/* Dispatch messages */
	while (dbus_connection_dispatch(conn) == DBUS_DISPATCH_DATA_REMAINS);

	dbus_connection_unref(conn);

	return FALSE;
}

static inline void queue_dispatch(DBusConnection *conn,
						DBusDispatchStatus status)
{
	if (status == DBUS_DISPATCH_DATA_REMAINS)
		g_idle_add(message_dispatch, dbus_connection_ref(conn));
}

static gboolean watch_func(GIOChannel *chan, GIOCondition cond, gpointer data)
{
	struct watch_info *info = data;
	unsigned int flags = 0;
	DBusDispatchStatus status;
	DBusConnection *conn;

	if (cond & G_IO_IN)  flags |= DBUS_WATCH_READABLE;
	if (cond & G_IO_OUT) flags |= DBUS_WATCH_WRITABLE;
	if (cond & G_IO_HUP) flags |= DBUS_WATCH_HANGUP;
	if (cond & G_IO_ERR) flags |= DBUS_WATCH_ERROR;

	/* Protect connection from being destroyed by dbus_watch_handle */
	conn = dbus_connection_ref(info->conn);

	dbus_watch_handle(info->watch, flags);

	status = dbus_connection_get_dispatch_status(conn);
	queue_dispatch(conn, status);

	dbus_connection_unref(conn);

	return TRUE;
}

static void watch_info_free(void *data)
{
	struct watch_info *info = data;

	if (info->id > 0) {
		g_source_remove(info->id);
		info->id = 0;
	}

	dbus_connection_unref(info->conn);

	g_free(info);
}

static dbus_bool_t add_watch(DBusWatch *watch, void *data)
{
	DBusConnection *conn = data;
	GIOCondition cond = G_IO_HUP | G_IO_ERR;
	GIOChannel *chan;
	struct watch_info *info;
	unsigned int flags;
	int fd;

	if (!dbus_watch_get_enabled(watch))
		return TRUE;

	info = g_new0(struct watch_info, 1);

	fd = dbus_watch_get_unix_fd(watch);
	chan = g_io_channel_unix_new(fd);

	info->watch = watch;
	info->conn = dbus_connection_ref(conn);

	dbus_watch_set_data(watch, info, watch_info_free);

	flags = dbus_watch_get_flags(watch);

	if (flags & DBUS_WATCH_READABLE) cond |= G_IO_IN;
	if (flags & DBUS_WATCH_WRITABLE) cond |= G_IO_OUT;

	info->id = g_io_add_watch(chan, cond, watch_func, info);

	g_io_channel_unref(chan);

	return TRUE;
}

static void remove_watch(DBusWatch *watch, void *data)
{
	if (dbus_watch_get_enabled(watch))
		return;

	/* will trigger watch_info_free() */
	dbus_watch_set_data(watch, NULL, NULL);
}

static void watch_toggled(DBusWatch *watch, void *data)
{
	/* Because we just exit on OOM, enable/disable is
	 * no different from add/remove */
	if (dbus_watch_get_enabled(watch))
		add_watch(watch, data);
	else
		remove_watch(watch, data);
}

static gboolean timeout_handler_dispatch(gpointer data)
{
	struct timeout_handler *handler = data;

	handler->id = 0;

	/* if not enabled should not be polled by the main loop */
	if (!dbus_timeout_get_enabled(handler->timeout))
		return FALSE;

	dbus_timeout_handle(handler->timeout);

	return FALSE;
}

static void timeout_handler_free(void *data)
{
	struct timeout_handler *handler = data;

	if (handler->id > 0) {
		g_source_remove(handler->id);
		handler->id = 0;
	}

	g_free(handler);
}

static dbus_bool_t add_timeout(DBusTimeout *timeout, void *data)
{
	int interval = dbus_timeout_get_interval(timeout);
	struct timeout_handler *handler;

	if (!dbus_timeout_get_enabled(timeout))
		return TRUE;

	handler = g_new0(struct timeout_handler, 1);

	handler->timeout = timeout;

	dbus_timeout_set_data(timeout, handler, timeout_handler_free);

	handler->id = g_timeout_add(interval, timeout_handler_dispatch,
								handler);

	return TRUE;
}

static void remove_timeout(DBusTimeout *timeout, void *data)
{
	/* will trigger timeout_handler_free() */
	dbus_timeout_set_data(timeout, NULL, NULL);
}

static void timeout_toggled(DBusTimeout *timeout, void *data)
{
	if (dbus_timeout_get_enabled(timeout))
		add_timeout(timeout, data);
	else
		remove_timeout(timeout, data);
}

static void dispatch_status(DBusConnection *conn,
					DBusDispatchStatus status, void *data)
{
	if (!dbus_connection_get_is_connected(conn))
		return;

	queue_dispatch(conn, status);
}

static inline void setup_dbus_with_main_loop(DBusConnection *conn)
{
	dbus_connection_set_watch_functions(conn, add_watch, remove_watch,
						watch_toggled, conn, NULL);

	dbus_connection_set_timeout_functions(conn, add_timeout, remove_timeout,
						timeout_toggled, NULL, NULL);

	dbus_connection_set_dispatch_status_function(conn, dispatch_status,
								NULL, NULL);
}

static gboolean setup_bus(DBusConnection *conn, const char *name,
						DBusError *error)
{
	gboolean result;
	DBusDispatchStatus status;

	if (name != NULL) {
		result = g_dbus_request_name(conn, name, error);

		if (error != NULL) {
			if (dbus_error_is_set(error) == TRUE)
				return FALSE;
		}

		if (result == FALSE)
			return FALSE;
	}

	setup_dbus_with_main_loop(conn);

	status = dbus_connection_get_dispatch_status(conn);
	queue_dispatch(conn, status);

	return TRUE;
}

DBusConnection *g_dbus_setup_bus(DBusBusType type, const char *name,
							DBusError *error)
{
	DBusConnection *conn;

	conn = dbus_bus_get(type, error);

	if (error != NULL) {
		if (dbus_error_is_set(error) == TRUE)
			return NULL;
	}

	if (conn == NULL)
		return NULL;

	if (setup_bus(conn, name, error) == FALSE) {
		dbus_connection_unref(conn);
		return NULL;
	}

	return conn;
}

DBusConnection *g_dbus_setup_private(DBusBusType type, const char *name,
							DBusError *error)
{
	DBusConnection *conn;

	conn = dbus_bus_get_private(type, error);

	if (error != NULL) {
		if (dbus_error_is_set(error) == TRUE)
			return NULL;
	}

	if (conn == NULL)
		return NULL;

	if (setup_bus(conn, name, error) == FALSE) {
		dbus_connection_close(conn);
		dbus_connection_unref(conn);
		return NULL;
	}

	return conn;
}

gboolean g_dbus_request_name(DBusConnection *connection, const char *name,
							DBusError *error)
{
	int result;

	result = dbus_bus_request_name(connection, name,
					DBUS_NAME_FLAG_DO_NOT_QUEUE, error);

	if (error != NULL) {
		if (dbus_error_is_set(error) == TRUE)
			return FALSE;
	}

	if (result != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		if (error != NULL)
			dbus_set_error(error, name, "Name already in use");

		return FALSE;
	}

	return TRUE;
}

gboolean g_dbus_set_disconnect_function(DBusConnection *connection,
				GDBusWatchFunction function,
				void *user_data, DBusFreeFunction destroy)
{
	struct disconnect_data *dc_data;

	dc_data = g_new0(struct disconnect_data, 1);

	dc_data->function = function;
	dc_data->user_data = user_data;

	dbus_connection_set_exit_on_disconnect(connection, FALSE);

	if (g_dbus_add_signal_watch(connection, NULL, NULL,
				DBUS_INTERFACE_LOCAL, "Disconnected",
				disconnected_signal, dc_data, g_free) == 0) {
		error("Failed to add watch for D-Bus Disconnected signal");
		g_free(dc_data);
		return FALSE;
	}

	return TRUE;
}
