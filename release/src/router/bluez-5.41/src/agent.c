/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
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

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <glib.h>
#include <dbus/dbus.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"

#include "gdbus/gdbus.h"

#include "log.h"
#include "error.h"
#include "hcid.h"
#include "dbus-common.h"
#include "adapter.h"
#include "device.h"
#include "agent.h"
#include "shared/queue.h"

#define IO_CAPABILITY_DISPLAYONLY	0x00
#define IO_CAPABILITY_DISPLAYYESNO	0x01
#define IO_CAPABILITY_KEYBOARDONLY	0x02
#define IO_CAPABILITY_NOINPUTNOOUTPUT	0x03
#define IO_CAPABILITY_KEYBOARDDISPLAY	0x04
#define IO_CAPABILITY_INVALID		0xFF

#define REQUEST_TIMEOUT (60 * 1000)		/* 60 seconds */
#define AGENT_INTERFACE "org.bluez.Agent1"

static GHashTable *agent_list;
struct queue *default_agents = NULL;

typedef enum {
	AGENT_REQUEST_PASSKEY,
	AGENT_REQUEST_CONFIRMATION,
	AGENT_REQUEST_AUTHORIZATION,
	AGENT_REQUEST_PINCODE,
	AGENT_REQUEST_AUTHORIZE_SERVICE,
	AGENT_REQUEST_DISPLAY_PINCODE,
} agent_request_type_t;

struct agent {
	int ref;
	char *owner;
	char *path;
	uint8_t capability;
	struct agent_request *request;
	guint watch;
};

struct agent_request {
	agent_request_type_t type;
	struct agent *agent;
	DBusMessage *msg;
	DBusPendingCall *call;
	void *cb;
	void *user_data;
	GDestroyNotify destroy;
};

static void agent_release(struct agent *agent)
{
	DBusMessage *message;

	DBG("Releasing agent %s, %s", agent->owner, agent->path);

	if (agent->request)
		agent_cancel(agent);

	message = dbus_message_new_method_call(agent->owner, agent->path,
						AGENT_INTERFACE, "Release");
	if (message == NULL) {
		error("Couldn't allocate D-Bus message");
		return;
	}

	g_dbus_send_message(btd_get_dbus_connection(), message);
}

static int send_cancel_request(struct agent_request *req)
{
	DBusMessage *message;

	DBG("Sending Cancel request to %s, %s", req->agent->owner,
							req->agent->path);

	message = dbus_message_new_method_call(req->agent->owner, req->agent->path,
						AGENT_INTERFACE, "Cancel");
	if (message == NULL) {
		error("Couldn't allocate D-Bus message");
		return -ENOMEM;
	}

	g_dbus_send_message(btd_get_dbus_connection(), message);

	return 0;
}

static void agent_request_free(struct agent_request *req, gboolean destroy)
{
	if (req->msg)
		dbus_message_unref(req->msg);
	if (req->call)
		dbus_pending_call_unref(req->call);
	if (req->agent && req->agent->request)
		req->agent->request = NULL;
	if (destroy && req->destroy)
		req->destroy(req->user_data);
	g_free(req);
}

static void set_io_cap(struct btd_adapter *adapter, gpointer user_data)
{
	struct agent *agent = user_data;
	uint8_t io_cap;

	if (agent)
		io_cap = agent->capability;
	else
		io_cap = IO_CAPABILITY_NOINPUTNOOUTPUT;

	adapter_set_io_capability(adapter, io_cap);
}

static bool add_default_agent(struct agent *agent)
{
	if (queue_peek_head(default_agents) == agent)
		return true;

	queue_remove(default_agents, agent);

	if (!queue_push_head(default_agents, agent))
		return false;

	DBG("Default agent set to %s %s", agent->owner, agent->path);

	adapter_foreach(set_io_cap, agent);

	return true;
}

static void remove_default_agent(struct agent *agent)
{
	if (queue_peek_head(default_agents) != agent) {
		queue_remove(default_agents, agent);
		return;
	}

	queue_remove(default_agents, agent);

	agent = queue_peek_head(default_agents);
	if (agent)
		DBG("Default agent set to %s %s", agent->owner, agent->path);
	else
		DBG("Default agent cleared");

	adapter_foreach(set_io_cap, agent);
}

static void agent_disconnect(DBusConnection *conn, void *user_data)
{
	struct agent *agent = user_data;

	DBG("Agent %s disconnected", agent->owner);

	if (agent->watch > 0) {
		g_dbus_remove_watch(conn, agent->watch);
		agent->watch = 0;
	}

	remove_default_agent(agent);

	g_hash_table_remove(agent_list, agent->owner);
}

struct agent *agent_ref(struct agent *agent)
{
	agent->ref++;

	DBG("%p: ref=%d", agent, agent->ref);

	return agent;
}

void agent_unref(struct agent *agent)
{
	agent->ref--;

	DBG("%p: ref=%d", agent, agent->ref);

	if (agent->ref > 0)
		return;

	if (agent->request) {
		DBusError err;
		agent_pincode_cb pincode_cb;
		agent_passkey_cb passkey_cb;
		agent_cb cb;

		dbus_error_init(&err);
		dbus_set_error_const(&err, ERROR_INTERFACE ".Failed",
								"Canceled");

		switch (agent->request->type) {
		case AGENT_REQUEST_PINCODE:
			pincode_cb = agent->request->cb;
			pincode_cb(agent, &err, NULL, agent->request->user_data);
			break;
		case AGENT_REQUEST_PASSKEY:
			passkey_cb = agent->request->cb;
			passkey_cb(agent, &err, 0, agent->request->user_data);
			break;
		case AGENT_REQUEST_CONFIRMATION:
		case AGENT_REQUEST_AUTHORIZATION:
		case AGENT_REQUEST_AUTHORIZE_SERVICE:
		case AGENT_REQUEST_DISPLAY_PINCODE:
		default:
			cb = agent->request->cb;
			cb(agent, &err, agent->request->user_data);
		}

		dbus_error_free(&err);

		agent_cancel(agent);
	}

	g_free(agent->owner);
	g_free(agent->path);

	g_free(agent);
}

struct agent *agent_get(const char *owner)
{
	struct agent *agent;

	if (owner) {
		agent = g_hash_table_lookup(agent_list, owner);
		if (agent)
			return agent_ref(agent);
	}

	if (!queue_isempty(default_agents))
		return agent_ref(queue_peek_head(default_agents));

	return NULL;
}

static struct agent *agent_create( const char *name, const char *path,
							uint8_t capability)
{
	struct agent *agent;

	agent = g_new0(struct agent, 1);

	agent->owner = g_strdup(name);
	agent->path = g_strdup(path);
	agent->capability = capability;

	agent->watch = g_dbus_add_disconnect_watch(btd_get_dbus_connection(),
							name, agent_disconnect,
							agent, NULL);

	return agent_ref(agent);
}

static struct agent_request *agent_request_new(struct agent *agent,
						agent_request_type_t type,
						void *cb,
						void *user_data,
						GDestroyNotify destroy)
{
	struct agent_request *req;

	req = g_new0(struct agent_request, 1);

	req->agent = agent;
	req->type = type;
	req->cb = cb;
	req->user_data = user_data;
	req->destroy = destroy;

	return req;
}

int agent_cancel(struct agent *agent)
{
	if (!agent->request)
		return -EINVAL;

	if (agent->request->call) {
		dbus_pending_call_cancel(agent->request->call);
		send_cancel_request(agent->request);
	}

	agent_request_free(agent->request, TRUE);
	agent->request = NULL;

	return 0;
}

static void simple_agent_reply(DBusPendingCall *call, void *user_data)
{
	struct agent_request *req = user_data;
	struct agent *agent = req->agent;
	DBusMessage *message;
	DBusError err;
	agent_cb cb = req->cb;

	/* steal_reply will always return non-NULL since the callback
	 * is only called after a reply has been received */
	message = dbus_pending_call_steal_reply(call);

	/* Protect from the callback freeing the agent */
	agent_ref(agent);

	dbus_error_init(&err);
	if (dbus_set_error_from_message(&err, message)) {
		DBG("agent error reply: %s, %s", err.name, err.message);

		cb(agent, &err, req->user_data);

		if (dbus_error_has_name(&err, DBUS_ERROR_NO_REPLY)) {
			error("Timed out waiting for reply from agent");
			agent_cancel(agent);
			dbus_message_unref(message);
			dbus_error_free(&err);
			agent_unref(agent);
			return;
		}

		dbus_error_free(&err);
		goto done;
	}

	if (!dbus_message_get_args(message, &err, DBUS_TYPE_INVALID)) {
		error("Wrong reply signature: %s", err.message);
		cb(agent, &err, req->user_data);
		dbus_error_free(&err);
		goto done;
	}

	cb(agent, NULL, req->user_data);
done:
	dbus_message_unref(message);

	agent->request = NULL;
	agent_request_free(req, TRUE);
	agent_unref(agent);
}

static int agent_call_authorize_service(struct agent_request *req,
						const char *device_path,
						const char *uuid)
{
	struct agent *agent = req->agent;

	req->msg = dbus_message_new_method_call(agent->owner, agent->path,
					AGENT_INTERFACE, "AuthorizeService");
	if (!req->msg) {
		error("Couldn't allocate D-Bus message");
		return -ENOMEM;
	}

	dbus_message_append_args(req->msg,
				DBUS_TYPE_OBJECT_PATH, &device_path,
				DBUS_TYPE_STRING, &uuid,
				DBUS_TYPE_INVALID);

	if (g_dbus_send_message_with_reply(btd_get_dbus_connection(),
						req->msg, &req->call,
						REQUEST_TIMEOUT) == FALSE) {
		error("D-Bus send failed");
		return -EIO;
	}

	dbus_pending_call_set_notify(req->call, simple_agent_reply, req, NULL);
	return 0;
}

int agent_authorize_service(struct agent *agent, const char *path,
				const char *uuid, agent_cb cb,
				void *user_data, GDestroyNotify destroy)
{
	struct agent_request *req;
	int err;

	if (agent->request)
		return -EBUSY;

	req = agent_request_new(agent, AGENT_REQUEST_AUTHORIZE_SERVICE, cb,
							user_data, destroy);

	err = agent_call_authorize_service(req, path, uuid);
	if (err < 0) {
		agent_request_free(req, FALSE);
		return -ENOMEM;
	}

	agent->request = req;

	DBG("authorize service request was sent for %s", path);

	return 0;
}

static void pincode_reply(DBusPendingCall *call, void *user_data)
{
	struct agent_request *req = user_data;
	struct agent *agent = req->agent;
	agent_pincode_cb cb = req->cb;
	DBusMessage *message;
	DBusError err;
	size_t len;
	char *pin;

	/* steal_reply will always return non-NULL since the callback
	 * is only called after a reply has been received */
	message = dbus_pending_call_steal_reply(call);

	/* Protect from the callback freeing the agent */
	agent_ref(agent);

	dbus_error_init(&err);
	if (dbus_set_error_from_message(&err, message)) {
		error("Agent %s replied with an error: %s, %s",
				agent->path, err.name, err.message);

		cb(agent, &err, NULL, req->user_data);
		dbus_error_free(&err);
		goto done;
	}

	if (!dbus_message_get_args(message, &err,
				DBUS_TYPE_STRING, &pin,
				DBUS_TYPE_INVALID)) {
		error("Wrong passkey reply signature: %s", err.message);
		cb(agent, &err, NULL, req->user_data);
		dbus_error_free(&err);
		goto done;
	}

	len = strlen(pin);

	if (len > 16 || len < 1) {
		error("Invalid PIN length (%zu) from agent", len);
		dbus_set_error_const(&err, ERROR_INTERFACE ".InvalidArgs",
					"Invalid passkey length");
		cb(agent, &err, NULL, req->user_data);
		dbus_error_free(&err);
		goto done;
	}

	cb(agent, NULL, pin, req->user_data);

done:
	if (message)
		dbus_message_unref(message);

	dbus_pending_call_cancel(req->call);
	agent->request = NULL;
	agent_request_free(req, TRUE);
	agent_unref(agent);
}

static int pincode_request_new(struct agent_request *req, const char *device_path,
				dbus_bool_t secure)
{
	struct agent *agent = req->agent;

	/* TODO: Add a new method or a new param to Agent interface to request
		secure pin. */

	req->msg = dbus_message_new_method_call(agent->owner, agent->path,
					AGENT_INTERFACE, "RequestPinCode");
	if (req->msg == NULL) {
		error("Couldn't allocate D-Bus message");
		return -ENOMEM;
	}

	dbus_message_append_args(req->msg, DBUS_TYPE_OBJECT_PATH, &device_path,
					DBUS_TYPE_INVALID);

	if (g_dbus_send_message_with_reply(btd_get_dbus_connection(), req->msg,
					&req->call, REQUEST_TIMEOUT) == FALSE) {
		error("D-Bus send failed");
		return -EIO;
	}

	dbus_pending_call_set_notify(req->call, pincode_reply, req, NULL);
	return 0;
}

int agent_request_pincode(struct agent *agent, struct btd_device *device,
				agent_pincode_cb cb, gboolean secure,
				void *user_data, GDestroyNotify destroy)
{
	struct agent_request *req;
	const char *dev_path = device_get_path(device);
	int err;

	if (agent->request)
		return -EBUSY;

	req = agent_request_new(agent, AGENT_REQUEST_PINCODE, cb,
							user_data, destroy);

	err = pincode_request_new(req, dev_path, secure);
	if (err < 0)
		goto failed;

	agent->request = req;

	return 0;

failed:
	agent_request_free(req, FALSE);
	return err;
}

static void passkey_reply(DBusPendingCall *call, void *user_data)
{
	struct agent_request *req = user_data;
	struct agent *agent = req->agent;
	agent_passkey_cb cb = req->cb;
	DBusMessage *message;
	DBusError err;
	uint32_t passkey;

	/* steal_reply will always return non-NULL since the callback
	 * is only called after a reply has been received */
	message = dbus_pending_call_steal_reply(call);

	dbus_error_init(&err);
	if (dbus_set_error_from_message(&err, message)) {
		error("Agent replied with an error: %s, %s",
						err.name, err.message);
		cb(agent, &err, 0, req->user_data);
		dbus_error_free(&err);
		goto done;
	}

	if (!dbus_message_get_args(message, &err,
				DBUS_TYPE_UINT32, &passkey,
				DBUS_TYPE_INVALID)) {
		error("Wrong passkey reply signature: %s", err.message);
		cb(agent, &err, 0, req->user_data);
		dbus_error_free(&err);
		goto done;
	}

	cb(agent, NULL, passkey, req->user_data);

done:
	if (message)
		dbus_message_unref(message);

	dbus_pending_call_cancel(req->call);
	agent->request = NULL;
	agent_request_free(req, TRUE);
}

static int passkey_request_new(struct agent_request *req,
				const char *device_path)
{
	struct agent *agent = req->agent;

	req->msg = dbus_message_new_method_call(agent->owner, agent->path,
					AGENT_INTERFACE, "RequestPasskey");
	if (req->msg == NULL) {
		error("Couldn't allocate D-Bus message");
		return -ENOMEM;
	}

	dbus_message_append_args(req->msg, DBUS_TYPE_OBJECT_PATH, &device_path,
					DBUS_TYPE_INVALID);

	if (g_dbus_send_message_with_reply(btd_get_dbus_connection(), req->msg,
					&req->call, REQUEST_TIMEOUT) == FALSE) {
		error("D-Bus send failed");
		return -EIO;
	}

	dbus_pending_call_set_notify(req->call, passkey_reply, req, NULL);
	return 0;
}

int agent_request_passkey(struct agent *agent, struct btd_device *device,
				agent_passkey_cb cb, void *user_data,
				GDestroyNotify destroy)
{
	struct agent_request *req;
	const char *dev_path = device_get_path(device);
	int err;

	if (agent->request)
		return -EBUSY;

	DBG("Calling Agent.RequestPasskey: name=%s, path=%s",
			agent->owner, agent->path);

	req = agent_request_new(agent, AGENT_REQUEST_PASSKEY, cb,
							user_data, destroy);

	err = passkey_request_new(req, dev_path);
	if (err < 0)
		goto failed;

	agent->request = req;

	return 0;

failed:
	agent_request_free(req, FALSE);
	return err;
}

static int confirmation_request_new(struct agent_request *req,
					const char *device_path,
					uint32_t passkey)
{
	struct agent *agent = req->agent;

	req->msg = dbus_message_new_method_call(agent->owner, agent->path,
				AGENT_INTERFACE, "RequestConfirmation");
	if (req->msg == NULL) {
		error("Couldn't allocate D-Bus message");
		return -ENOMEM;
	}

	dbus_message_append_args(req->msg,
				DBUS_TYPE_OBJECT_PATH, &device_path,
				DBUS_TYPE_UINT32, &passkey,
				DBUS_TYPE_INVALID);

	if (g_dbus_send_message_with_reply(btd_get_dbus_connection(), req->msg,
				&req->call, REQUEST_TIMEOUT) == FALSE) {
		error("D-Bus send failed");
		return -EIO;
	}

	dbus_pending_call_set_notify(req->call, simple_agent_reply, req, NULL);

	return 0;
}

int agent_request_confirmation(struct agent *agent, struct btd_device *device,
				uint32_t passkey, agent_cb cb,
				void *user_data, GDestroyNotify destroy)
{
	struct agent_request *req;
	const char *dev_path = device_get_path(device);
	int err;

	if (agent->request)
		return -EBUSY;

	DBG("Calling Agent.RequestConfirmation: name=%s, path=%s, passkey=%06u",
			agent->owner, agent->path, passkey);

	req = agent_request_new(agent, AGENT_REQUEST_CONFIRMATION, cb,
				user_data, destroy);

	err = confirmation_request_new(req, dev_path, passkey);
	if (err < 0)
		goto failed;

	agent->request = req;

	return 0;

failed:
	agent_request_free(req, FALSE);
	return err;
}

static int authorization_request_new(struct agent_request *req,
						const char *device_path)
{
	struct agent *agent = req->agent;

	req->msg = dbus_message_new_method_call(agent->owner, agent->path,
				AGENT_INTERFACE, "RequestAuthorization");
	if (req->msg == NULL) {
		error("Couldn't allocate D-Bus message");
		return -ENOMEM;
	}

	dbus_message_append_args(req->msg,
				DBUS_TYPE_OBJECT_PATH, &device_path,
				DBUS_TYPE_INVALID);

	if (g_dbus_send_message_with_reply(btd_get_dbus_connection(), req->msg,
				&req->call, REQUEST_TIMEOUT) == FALSE) {
		error("D-Bus send failed");
		return -EIO;
	}

	dbus_pending_call_set_notify(req->call, simple_agent_reply, req, NULL);

	return 0;
}

int agent_request_authorization(struct agent *agent, struct btd_device *device,
						agent_cb cb, void *user_data,
						GDestroyNotify destroy)
{
	struct agent_request *req;
	const char *dev_path = device_get_path(device);
	int err;

	if (agent->request)
		return -EBUSY;

	DBG("Calling Agent.RequestAuthorization: name=%s, path=%s",
						agent->owner, agent->path);

	req = agent_request_new(agent, AGENT_REQUEST_AUTHORIZATION, cb,
				user_data, destroy);

	err = authorization_request_new(req, dev_path);
	if (err < 0)
		goto failed;

	agent->request = req;

	return 0;

failed:
	agent_request_free(req, FALSE);
	return err;
}

int agent_display_passkey(struct agent *agent, struct btd_device *device,
				uint32_t passkey, uint16_t entered)
{
	DBusMessage *message;
	const char *dev_path = device_get_path(device);

	message = dbus_message_new_method_call(agent->owner, agent->path,
					AGENT_INTERFACE, "DisplayPasskey");
	if (!message) {
		error("Couldn't allocate D-Bus message");
		return -1;
	}

	dbus_message_append_args(message,
				DBUS_TYPE_OBJECT_PATH, &dev_path,
				DBUS_TYPE_UINT32, &passkey,
				DBUS_TYPE_UINT16, &entered,
				DBUS_TYPE_INVALID);

	if (!g_dbus_send_message(btd_get_dbus_connection(), message)) {
		error("D-Bus send failed");
		return -1;
	}

	return 0;
}

static void display_pincode_reply(DBusPendingCall *call, void *user_data)
{
	struct agent_request *req = user_data;
	struct agent *agent = req->agent;
	DBusMessage *message;
	DBusError err;
	agent_cb cb = req->cb;

	/* clear agent->request early; our callback will likely try
	 * another request */
	agent->request = NULL;

	/* steal_reply will always return non-NULL since the callback
	 * is only called after a reply has been received */
	message = dbus_pending_call_steal_reply(call);

	dbus_error_init(&err);
	if (dbus_set_error_from_message(&err, message)) {
		error("Agent replied with an error: %s, %s",
						err.name, err.message);

		cb(agent, &err, req->user_data);

		if (dbus_error_has_name(&err, DBUS_ERROR_NO_REPLY)) {
			agent_cancel(agent);
			dbus_message_unref(message);
			dbus_error_free(&err);
			return;
		}

		dbus_error_free(&err);
		goto done;
	}

	if (!dbus_message_get_args(message, &err, DBUS_TYPE_INVALID)) {
		error("Wrong reply signature: %s", err.message);
		cb(agent, &err, req->user_data);
		dbus_error_free(&err);
		goto done;
	}

	cb(agent, NULL, req->user_data);
done:
	dbus_message_unref(message);

	agent_request_free(req, TRUE);
}

static int display_pincode_request_new(struct agent_request *req,
					const char *device_path,
					const char *pincode)
{
	struct agent *agent = req->agent;

	req->msg = dbus_message_new_method_call(agent->owner, agent->path,
					AGENT_INTERFACE, "DisplayPinCode");
	if (req->msg == NULL) {
		error("Couldn't allocate D-Bus message");
		return -ENOMEM;
	}

	dbus_message_append_args(req->msg,
					DBUS_TYPE_OBJECT_PATH, &device_path,
					DBUS_TYPE_STRING, &pincode,
					DBUS_TYPE_INVALID);

	if (g_dbus_send_message_with_reply(btd_get_dbus_connection(), req->msg,
				&req->call, REQUEST_TIMEOUT) == FALSE) {
		error("D-Bus send failed");
		return -EIO;
	}

	dbus_pending_call_set_notify(req->call, display_pincode_reply,
								req, NULL);

	return 0;
}

int agent_display_pincode(struct agent *agent, struct btd_device *device,
				const char *pincode, agent_cb cb,
				void *user_data, GDestroyNotify destroy)
{
	struct agent_request *req;
	const char *dev_path = device_get_path(device);
	int err;

	if (agent->request)
		return -EBUSY;

	DBG("Calling Agent.DisplayPinCode: name=%s, path=%s, pincode=%s",
					agent->owner, agent->path, pincode);

	req = agent_request_new(agent, AGENT_REQUEST_DISPLAY_PINCODE, cb,
							user_data, destroy);

	err = display_pincode_request_new(req, dev_path, pincode);
	if (err < 0)
		goto failed;

	agent->request = req;

	return 0;

failed:
	agent_request_free(req, FALSE);
	return err;
}

uint8_t agent_get_io_capability(struct agent *agent)
{
	return agent->capability;
}

static void agent_destroy(gpointer data)
{
	struct agent *agent = data;

	DBG("agent %s", agent->owner);

	if (agent->watch > 0) {
		g_dbus_remove_watch(btd_get_dbus_connection(), agent->watch);
		agent->watch = 0;
		agent_release(agent);
	}

	remove_default_agent(agent);

	agent_unref(agent);
}

static uint8_t parse_io_capability(const char *capability)
{
	if (g_str_equal(capability, ""))
		return IO_CAPABILITY_KEYBOARDDISPLAY;
	if (g_str_equal(capability, "DisplayOnly"))
		return IO_CAPABILITY_DISPLAYONLY;
	if (g_str_equal(capability, "DisplayYesNo"))
		return IO_CAPABILITY_DISPLAYYESNO;
	if (g_str_equal(capability, "KeyboardOnly"))
		return IO_CAPABILITY_KEYBOARDONLY;
	if (g_str_equal(capability, "NoInputNoOutput"))
		return IO_CAPABILITY_NOINPUTNOOUTPUT;
	if (g_str_equal(capability, "KeyboardDisplay"))
		return IO_CAPABILITY_KEYBOARDDISPLAY;
	return IO_CAPABILITY_INVALID;
}

static DBusMessage *register_agent(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct agent *agent;
	const char *sender, *path, *capability;
	uint8_t cap;

	sender = dbus_message_get_sender(msg);

	agent = g_hash_table_lookup(agent_list, sender);
	if (agent)
		return btd_error_already_exists(msg);

	if (dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &path,
						DBUS_TYPE_STRING, &capability,
						DBUS_TYPE_INVALID) == FALSE)
		return btd_error_invalid_args(msg);

	cap = parse_io_capability(capability);
	if (cap == IO_CAPABILITY_INVALID)
		return btd_error_invalid_args(msg);

	agent = agent_create(sender, path, cap);
	if (!agent)
		return btd_error_invalid_args(msg);

	DBG("agent %s", agent->owner);

	g_hash_table_replace(agent_list, agent->owner, agent);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *unregister_agent(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct agent *agent;
	const char *sender, *path;

	sender = dbus_message_get_sender(msg);

	agent = g_hash_table_lookup(agent_list, sender);
	if (!agent)
		return btd_error_does_not_exist(msg);

	DBG("agent %s", agent->owner);

	if (dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &path,
						DBUS_TYPE_INVALID) == FALSE)
		return btd_error_invalid_args(msg);

	if (g_str_equal(path, agent->path) == FALSE)
		return btd_error_does_not_exist(msg);

	agent_disconnect(conn, agent);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *request_default(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	struct agent *agent;
	const char *sender, *path;

	sender = dbus_message_get_sender(msg);

	agent = g_hash_table_lookup(agent_list, sender);
	if (!agent)
		return btd_error_does_not_exist(msg);

	if (dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &path,
						DBUS_TYPE_INVALID) == FALSE)
		return btd_error_invalid_args(msg);

	if (g_str_equal(path, agent->path) == FALSE)
		return btd_error_does_not_exist(msg);

	if (!add_default_agent(agent))
		return btd_error_failed(msg, "Failed to set as default");

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable methods[] = {
	{ GDBUS_METHOD("RegisterAgent",
			GDBUS_ARGS({ "agent", "o"}, { "capability", "s" }),
			NULL, register_agent) },
	{ GDBUS_METHOD("UnregisterAgent", GDBUS_ARGS({ "agent", "o" }),
			NULL, unregister_agent) },
	{ GDBUS_METHOD("RequestDefaultAgent", GDBUS_ARGS({ "agent", "o" }),
			NULL, request_default ) },
	{ }
};

void btd_agent_init(void)
{
	agent_list = g_hash_table_new_full(g_str_hash, g_str_equal,
						NULL, agent_destroy);

	default_agents = queue_new();

	g_dbus_register_interface(btd_get_dbus_connection(),
				"/org/bluez", "org.bluez.AgentManager1",
				methods, NULL, NULL, NULL, NULL);
}

void btd_agent_cleanup(void)
{
	g_dbus_unregister_interface(btd_get_dbus_connection(),
				"/org/bluez", "org.bluez.AgentManager1");

	g_hash_table_destroy(agent_list);
	queue_destroy(default_agents, NULL);
}
