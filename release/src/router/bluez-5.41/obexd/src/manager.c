/*
 *
 *  OBEX Server
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
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <inttypes.h>

#include "gdbus/gdbus.h"
#include "gobex/gobex.h"

#include "btio/btio.h"
#include "obexd.h"
#include "obex.h"
#include "obex-priv.h"
#include "server.h"
#include "manager.h"
#include "log.h"
#include "service.h"

#define OBEX_BASE_PATH "/org/bluez/obex"
#define SESSION_BASE_PATH OBEX_BASE_PATH "/server"
#define OBEX_MANAGER_INTERFACE OBEXD_SERVICE ".AgentManager1"
#define ERROR_INTERFACE OBEXD_SERVICE ".Error"
#define TRANSFER_INTERFACE OBEXD_SERVICE ".Transfer1"
#define SESSION_INTERFACE OBEXD_SERVICE ".Session1"
#define AGENT_INTERFACE OBEXD_SERVICE ".Agent1"

#define TIMEOUT 60*1000 /* Timeout for user response (miliseconds) */

struct agent {
	char *bus_name;
	char *path;
	gboolean auth_pending;
	char *new_name;
	char *new_folder;
	unsigned int watch_id;
};

enum {
	TRANSFER_STATUS_QUEUED = 0,
	TRANSFER_STATUS_ACTIVE,
	TRANSFER_STATUS_COMPLETE,
	TRANSFER_STATUS_ERROR
};

struct obex_transfer {
	uint8_t status;
	char *path;
	struct obex_session *session;
};

static struct agent *agent = NULL;

static DBusConnection *connection = NULL;

static void agent_free(struct agent *agent)
{
	if (!agent)
		return;

	g_free(agent->new_folder);
	g_free(agent->new_name);
	g_free(agent->bus_name);
	g_free(agent->path);
	g_free(agent);
}

static inline DBusMessage *invalid_args(DBusMessage *msg)
{
	return g_dbus_create_error(msg,
			ERROR_INTERFACE ".InvalidArguments",
			"Invalid arguments in method call");
}

static inline DBusMessage *not_supported(DBusMessage *msg)
{
	return g_dbus_create_error(msg,
			ERROR_INTERFACE ".NotSupported",
			"Operation is not supported");
}

static inline DBusMessage *agent_already_exists(DBusMessage *msg)
{
	return g_dbus_create_error(msg,
			ERROR_INTERFACE ".AlreadyExists",
			"Agent already exists");
}

static inline DBusMessage *agent_does_not_exist(DBusMessage *msg)
{
	return g_dbus_create_error(msg,
			ERROR_INTERFACE ".DoesNotExist",
			"Agent does not exist");
}

static inline DBusMessage *not_authorized(DBusMessage *msg)
{
	return g_dbus_create_error(msg,
			ERROR_INTERFACE ".NotAuthorized",
			"Not authorized");
}

static void agent_disconnected(DBusConnection *conn, void *user_data)
{
	DBG("Agent exited");
	agent_free(agent);
	agent = NULL;
}

static DBusMessage *register_agent(DBusConnection *conn,
					DBusMessage *msg, void *data)
{
	const char *path, *sender;

	if (agent)
		return agent_already_exists(msg);

	if (!dbus_message_get_args(msg, NULL,
				DBUS_TYPE_OBJECT_PATH, &path,
				DBUS_TYPE_INVALID))
		return invalid_args(msg);

	sender = dbus_message_get_sender(msg);
	agent = g_new0(struct agent, 1);
	agent->bus_name = g_strdup(sender);
	agent->path = g_strdup(path);

	agent->watch_id = g_dbus_add_disconnect_watch(conn, sender,
					agent_disconnected, NULL, NULL);

	DBG("Agent registered");

	return dbus_message_new_method_return(msg);
}

static DBusMessage *unregister_agent(DBusConnection *conn,
					DBusMessage *msg, void *data)
{
	const char *path, *sender;

	if (!agent)
		return agent_does_not_exist(msg);

	if (!dbus_message_get_args(msg, NULL,
				DBUS_TYPE_OBJECT_PATH, &path,
				DBUS_TYPE_INVALID))
		return invalid_args(msg);

	if (strcmp(agent->path, path) != 0)
		return agent_does_not_exist(msg);

	sender = dbus_message_get_sender(msg);
	if (strcmp(agent->bus_name, sender) != 0)
		return not_authorized(msg);

	g_dbus_remove_watch(conn, agent->watch_id);

	agent_free(agent);
	agent = NULL;

	DBG("Agent unregistered");

	return dbus_message_new_method_return(msg);
}

static gboolean get_source(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obex_session *os = data;
	char *s;

	s = os->src;
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &s);

	return TRUE;
}

static gboolean get_destination(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obex_session *os = data;
	char *s;

	s = os->dst;
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &s);

	return TRUE;
}

static gboolean session_target_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct obex_session *os = data;

	return os->service->target ? TRUE : FALSE;
}

static char *target2str(const uint8_t *t)
{
	if (!t)
		return NULL;

	return g_strdup_printf("%02X%02X%02X%02X-%02X%02X-%02X%02X-"
				"%02X%02X-%02X%02X%02X%02X%02X%02X",
				t[0], t[1], t[2], t[3], t[4], t[5], t[6], t[7],
				t[8], t[9], t[10], t[11], t[12], t[13], t[14],
				t[15]);
}

static gboolean get_target(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obex_session *os = data;
	char *uuid;

	uuid = target2str(os->service->target);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &uuid);
	g_free(uuid);

	return TRUE;
}

static gboolean get_root(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	const char *root;

	root = obex_option_root_folder();
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &root);

	return TRUE;
}

static DBusMessage *transfer_cancel(DBusConnection *connection,
				DBusMessage *msg, void *user_data)
{
	struct obex_transfer *transfer = user_data;
	struct obex_session *os = transfer->session;
	const char *sender;

	if (!os)
		return invalid_args(msg);

	sender = dbus_message_get_sender(msg);
	if (strcmp(agent->bus_name, sender) != 0)
		return not_authorized(msg);

	os->aborted = TRUE;

	return dbus_message_new_method_return(msg);
}

static const char *status2str(uint8_t status)
{
	switch (status) {
	case TRANSFER_STATUS_QUEUED:
		return "queued";
	case TRANSFER_STATUS_ACTIVE:
		return "active";
	case TRANSFER_STATUS_COMPLETE:
		return "complete";
	case TRANSFER_STATUS_ERROR:
	default:
		return "error";
	}
}

static gboolean transfer_get_status(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obex_transfer *transfer = data;
	const char *status = status2str(transfer->status);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &status);

	return TRUE;
}

static gboolean transfer_get_session(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obex_transfer *transfer = data;
	struct obex_session *session = transfer->session;
	char *path;

	if (session == NULL)
		return FALSE;

	path = g_strdup_printf("%s/session%u", SESSION_BASE_PATH, session->id);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);

	g_free(path);

	return TRUE;
}

static gboolean transfer_name_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct obex_transfer *transfer = data;
	struct obex_session *session = transfer->session;

	return session->name != NULL;
}

static gboolean transfer_get_name(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obex_transfer *transfer = data;
	struct obex_session *session = transfer->session;

	if (session->name == NULL)
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &session->name);

	return TRUE;
}

static gboolean transfer_type_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct obex_transfer *transfer = data;
	struct obex_session *session = transfer->session;

	return session->type != NULL;
}

static gboolean transfer_get_type(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obex_transfer *transfer = data;
	struct obex_session *session = transfer->session;

	if (session->type == NULL)
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &session->type);

	return TRUE;
}

static gboolean transfer_size_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct obex_transfer *transfer = data;
	struct obex_session *session = transfer->session;

	return session->size != OBJECT_SIZE_UNKNOWN;
}

static gboolean transfer_get_size(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obex_transfer *transfer = data;
	struct obex_session *session = transfer->session;

	if (session->size == OBJECT_SIZE_UNKNOWN)
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64, &session->size);

	return TRUE;
}

static gboolean transfer_time_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct obex_transfer *transfer = data;
	struct obex_session *session = transfer->session;

	return session->time != 0;
}

static gboolean transfer_get_time(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obex_transfer *transfer = data;
	struct obex_session *session = transfer->session;
	dbus_uint64_t time_u64;

	if (session->size == 0)
		return FALSE;

	time_u64 = session->time;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64, &time_u64);

	return TRUE;
}

static gboolean transfer_filename_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct obex_transfer *transfer = data;
	struct obex_session *session = transfer->session;

	return session->path != NULL;
}

static gboolean transfer_get_filename(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obex_transfer *transfer = data;
	struct obex_session *session = transfer->session;

	if (session->path == NULL)
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &session->path);

	return TRUE;
}

static gboolean transfer_get_transferred(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct obex_transfer *transfer = data;
	struct obex_session *session = transfer->session;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT64,
							&session->offset);

	return TRUE;
}

static const GDBusMethodTable manager_methods[] = {
	{ GDBUS_METHOD("RegisterAgent",
			GDBUS_ARGS({ "agent", "o" }), NULL, register_agent) },
	{ GDBUS_METHOD("UnregisterAgent",
			GDBUS_ARGS({ "agent", "o" }), NULL, unregister_agent) },
	{ }
};

static const GDBusMethodTable transfer_methods[] = {
	{ GDBUS_METHOD("Cancel", NULL, NULL, transfer_cancel) },
	{ }
};

static const GDBusPropertyTable transfer_properties[] = {
	{ "Status", "s", transfer_get_status },
	{ "Session", "o", transfer_get_session },
	{ "Name", "s", transfer_get_name, NULL, transfer_name_exists },
	{ "Type", "s", transfer_get_type, NULL, transfer_type_exists },
	{ "Size", "t", transfer_get_size, NULL, transfer_size_exists },
	{ "Time", "t", transfer_get_time, NULL, transfer_time_exists },
	{ "Filename", "s", transfer_get_filename, NULL,
						transfer_filename_exists },
	{ "Transferred", "t", transfer_get_transferred },
	{ }
};

static const GDBusPropertyTable session_properties[] = {
	{ "Source", "s", get_source },
	{ "Destination", "s", get_destination },
	{ "Target", "s", get_target, NULL, session_target_exists },
	{ "Root", "s", get_root },
	{ }
};

gboolean manager_init(void)
{
	DBusError err;

	DBG("");

	dbus_error_init(&err);

	connection = g_dbus_setup_bus(DBUS_BUS_SESSION, OBEXD_SERVICE, &err);
	if (connection == NULL) {
		if (dbus_error_is_set(&err) == TRUE) {
			fprintf(stderr, "%s\n", err.message);
			dbus_error_free(&err);
		} else
			fprintf(stderr, "Can't register with session bus\n");
		return FALSE;
	}

	g_dbus_attach_object_manager(connection);

	return g_dbus_register_interface(connection, OBEX_BASE_PATH,
					OBEX_MANAGER_INTERFACE,
					manager_methods, NULL, NULL,
					NULL, NULL);
}

void manager_cleanup(void)
{
	DBG("");

	g_dbus_unregister_interface(connection, OBEX_BASE_PATH,
						OBEX_MANAGER_INTERFACE);

	/* FIXME: Release agent? */

	agent_free(agent);

	g_dbus_detach_object_manager(connection);

	dbus_connection_unref(connection);
}

void manager_emit_transfer_started(struct obex_transfer *transfer)
{
	transfer->status = TRANSFER_STATUS_ACTIVE;

	g_dbus_emit_property_changed(connection, transfer->path,
					TRANSFER_INTERFACE, "Status");
}

static void emit_transfer_completed(struct obex_transfer *transfer,
							gboolean success)
{
	if (transfer->path == NULL)
		return;

	transfer->status = success ? TRANSFER_STATUS_COMPLETE :
						TRANSFER_STATUS_ERROR;

	g_dbus_emit_property_changed(connection, transfer->path,
					TRANSFER_INTERFACE, "Status");
}

static void emit_transfer_progress(struct obex_transfer *transfer,
					uint32_t total, uint32_t transferred)
{
	if (transfer->path == NULL)
		return;

	g_dbus_emit_property_changed(connection, transfer->path,
					TRANSFER_INTERFACE, "Transferred");
}

static void transfer_free(struct obex_transfer *transfer)
{
	g_free(transfer->path);
	g_free(transfer);
}

struct obex_transfer *manager_register_transfer(struct obex_session *os)
{
	struct obex_transfer *transfer;
	static unsigned int id = 0;

	transfer = g_new0(struct obex_transfer, 1);
	transfer->path = g_strdup_printf("%s/session%u/transfer%u",
					SESSION_BASE_PATH, os->id, id++);
	transfer->session = os;

	if (!g_dbus_register_interface(connection, transfer->path,
				TRANSFER_INTERFACE,
				transfer_methods, NULL,
				transfer_properties, transfer, NULL)) {
		error("Cannot register Transfer interface.");
		transfer_free(transfer);
		return NULL;
	}

	return transfer;
}

void manager_unregister_transfer(struct obex_transfer *transfer)
{
	struct obex_session *os;

	if (transfer == NULL)
		return;

	os = transfer->session;

	if (transfer->status == TRANSFER_STATUS_ACTIVE)
		emit_transfer_completed(transfer, os->offset == os->size);

	g_dbus_unregister_interface(connection, transfer->path,
							TRANSFER_INTERFACE);

	transfer_free(transfer);
}

static void agent_cancel(void)
{
	DBusMessage *msg;

	if (agent == NULL)
		return;

	msg = dbus_message_new_method_call(agent->bus_name, agent->path,
						AGENT_INTERFACE, "Cancel");

	g_dbus_send_message(connection, msg);
}

static void agent_reply(DBusPendingCall *call, void *user_data)
{
	DBusMessage *reply = dbus_pending_call_steal_reply(call);
	const char *name;
	DBusError derr;
	gboolean *got_reply = user_data;

	*got_reply = TRUE;

	/* Received a reply after the agent exited */
	if (!agent)
		return;

	agent->auth_pending = FALSE;

	dbus_error_init(&derr);
	if (dbus_set_error_from_message(&derr, reply)) {
		error("Agent replied with an error: %s, %s",
				derr.name, derr.message);

		if (dbus_error_has_name(&derr, DBUS_ERROR_NO_REPLY))
			agent_cancel();

		dbus_error_free(&derr);
		dbus_message_unref(reply);
		return;
	}

	if (dbus_message_get_args(reply, NULL,
				DBUS_TYPE_STRING, &name,
				DBUS_TYPE_INVALID)) {
		/* Splits folder and name */
		const char *slash = strrchr(name, '/');
		DBG("Agent replied with %s", name);
		if (!slash) {
			agent->new_name = g_strdup(name);
			agent->new_folder = NULL;
		} else {
			agent->new_name = g_strdup(slash + 1);
			agent->new_folder = g_strndup(name, slash - name);
		}
	}

	dbus_message_unref(reply);
}

static gboolean auth_error(GIOChannel *io, GIOCondition cond, void *user_data)
{
	agent->auth_pending = FALSE;

	return FALSE;
}

int manager_request_authorization(struct obex_transfer *transfer,
					char **new_folder, char **new_name)
{
	struct obex_session *os = transfer->session;
	DBusMessage *msg;
	DBusPendingCall *call;
	unsigned int watch;
	gboolean got_reply;

	if (!agent)
		return -1;

	if (agent->auth_pending)
		return -EPERM;

	if (!new_folder || !new_name)
		return -EINVAL;

	msg = dbus_message_new_method_call(agent->bus_name, agent->path,
							AGENT_INTERFACE,
							"AuthorizePush");

	dbus_message_append_args(msg, DBUS_TYPE_OBJECT_PATH, &transfer->path,
							DBUS_TYPE_INVALID);

	if (!g_dbus_send_message_with_reply(connection, msg, &call, TIMEOUT)) {
		dbus_message_unref(msg);
		return -EPERM;
	}

	dbus_message_unref(msg);

	agent->auth_pending = TRUE;
	got_reply = FALSE;

	/* Catches errors before authorization response comes */
	watch = g_io_add_watch_full(os->io, G_PRIORITY_DEFAULT,
			G_IO_HUP | G_IO_ERR | G_IO_NVAL,
			auth_error, NULL, NULL);

	dbus_pending_call_set_notify(call, agent_reply, &got_reply, NULL);

	/* Workaround: process events while agent doesn't reply */
	while (agent && agent->auth_pending)
		g_main_context_iteration(NULL, TRUE);

	g_source_remove(watch);

	if (!got_reply) {
		dbus_pending_call_cancel(call);
		agent_cancel();
	}

	dbus_pending_call_unref(call);

	if (!agent || !agent->new_name)
		return -EPERM;

	*new_folder = agent->new_folder;
	*new_name = agent->new_name;
	agent->new_folder = NULL;
	agent->new_name = NULL;

	return 0;
}

static DBusMessage *session_get_capabilities(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	return not_supported(message);
}

static const GDBusMethodTable session_methods[] = {
	{ GDBUS_ASYNC_METHOD("GetCapabilities",
				NULL, GDBUS_ARGS({ "capabilities", "s" }),
				session_get_capabilities) },
	{ }
};

void manager_register_session(struct obex_session *os)
{
	char *path;

	path = g_strdup_printf("%s/session%u", SESSION_BASE_PATH, os->id);

	if (!g_dbus_register_interface(connection, path,
				SESSION_INTERFACE,
				session_methods, NULL,
				session_properties, os, NULL))
		error("Cannot register Session interface.");

	g_free(path);
}

void manager_unregister_session(struct obex_session *os)
{
	char *path;

	path = g_strdup_printf("%s/session%u", SESSION_BASE_PATH, os->id);

	g_dbus_unregister_interface(connection, path, SESSION_INTERFACE);

	g_free(path);
}

void manager_emit_transfer_progress(struct obex_transfer *transfer)
{
	emit_transfer_progress(transfer, transfer->session->size,
						transfer->session->offset);
}

void manager_emit_transfer_completed(struct obex_transfer *transfer)
{
	struct obex_session *session;

	if (transfer == NULL)
		return;

	session = transfer->session;

	if (session == NULL || session->object == NULL)
		return;

	emit_transfer_completed(transfer, !session->aborted);
}

DBusConnection *manager_dbus_get_connection(void)
{
	if (connection == NULL)
		return NULL;

	return dbus_connection_ref(connection);
}
