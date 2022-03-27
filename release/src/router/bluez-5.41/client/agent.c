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

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>

#include "gdbus/gdbus.h"
#include "display.h"
#include "agent.h"

#define AGENT_PATH "/org/bluez/agent"
#define AGENT_INTERFACE "org.bluez.Agent1"

#define AGENT_PROMPT	COLOR_RED "[agent]" COLOR_OFF " "

static gboolean agent_registered = FALSE;
static const char *agent_capability = NULL;
static DBusMessage *pending_message = NULL;
static char *agent_saved_prompt = NULL;
static int agent_saved_point = 0;

static void agent_prompt(const char *msg)
{
	char *prompt;

	/* Normal use should not prompt for user input to the agent a second
	 * time before it releases the prompt, but we take a safe action. */
	if (agent_saved_prompt)
		return;

	agent_saved_point = rl_point;
	agent_saved_prompt = g_strdup(rl_prompt);

	rl_set_prompt("");
	rl_redisplay();

	prompt = g_strdup_printf(AGENT_PROMPT "%s", msg);
	rl_set_prompt(prompt);
	g_free(prompt);

	rl_replace_line("", 0);
	rl_redisplay();
}

static void agent_release_prompt(void)
{
	if (!agent_saved_prompt)
		return;

	/* This will cause rl_expand_prompt to re-run over the last prompt, but
	 * our prompt doesn't expand anyway. */
	rl_set_prompt(agent_saved_prompt);
	rl_replace_line("", 0);
	rl_point = agent_saved_point;
	rl_redisplay();

	g_free(agent_saved_prompt);
	agent_saved_prompt = NULL;
}

dbus_bool_t agent_completion(void)
{
	if (!pending_message)
		return FALSE;

	return TRUE;
}

static void pincode_response(DBusConnection *conn, const char *input)
{
	g_dbus_send_reply(conn, pending_message, DBUS_TYPE_STRING, &input,
							DBUS_TYPE_INVALID);
}

static void passkey_response(DBusConnection *conn, const char *input)
{
	dbus_uint32_t passkey;
	if (sscanf(input, "%u", &passkey) == 1)
		g_dbus_send_reply(conn, pending_message, DBUS_TYPE_UINT32,
						&passkey, DBUS_TYPE_INVALID);
	else if (!strcmp(input, "no"))
		g_dbus_send_error(conn, pending_message,
					"org.bluez.Error.Rejected", NULL);
	else
		g_dbus_send_error(conn, pending_message,
					"org.bluez.Error.Canceled", NULL);
}

static void confirm_response(DBusConnection *conn, const char *input)
{
	if (!strcmp(input, "yes"))
		g_dbus_send_reply(conn, pending_message, DBUS_TYPE_INVALID);
	else if (!strcmp(input, "no"))
		g_dbus_send_error(conn, pending_message,
					"org.bluez.Error.Rejected", NULL);
	else
		g_dbus_send_error(conn, pending_message,
					"org.bluez.Error.Canceled", NULL);
}

dbus_bool_t agent_input(DBusConnection *conn, const char *input)
{
	const char *member;

	if (!pending_message)
		return FALSE;

	agent_release_prompt();

	member = dbus_message_get_member(pending_message);

	if (!strcmp(member, "RequestPinCode"))
		pincode_response(conn, input);
	else if (!strcmp(member, "RequestPasskey"))
		passkey_response(conn, input);
	else if (!strcmp(member, "RequestConfirmation"))
		confirm_response(conn, input);
	else if (!strcmp(member, "RequestAuthorization"))
		confirm_response(conn, input);
	else if (!strcmp(member, "AuthorizeService"))
		confirm_response(conn, input);
	else
		g_dbus_send_error(conn, pending_message,
					"org.bluez.Error.Canceled", NULL);

	dbus_message_unref(pending_message);
	pending_message = NULL;

	return TRUE;
}

static void agent_release(DBusConnection *conn)
{
	agent_registered = FALSE;
	agent_capability = NULL;

	if (pending_message) {
		dbus_message_unref(pending_message);
		pending_message = NULL;
	}

	agent_release_prompt();

	g_dbus_unregister_interface(conn, AGENT_PATH, AGENT_INTERFACE);
}

static DBusMessage *release_agent(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	rl_printf("Agent released\n");

	agent_release(conn);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *request_pincode(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	const char *device;

	rl_printf("Request PIN code\n");

	dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &device,
							DBUS_TYPE_INVALID);

	agent_prompt("Enter PIN code: ");

	pending_message = dbus_message_ref(msg);

	return NULL;
}

static DBusMessage *display_pincode(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	const char *device;
	const char *pincode;

	dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &device,
				DBUS_TYPE_STRING, &pincode, DBUS_TYPE_INVALID);

	rl_printf(AGENT_PROMPT "PIN code: %s\n", pincode);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *request_passkey(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	const char *device;

	rl_printf("Request passkey\n");

	dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &device,
							DBUS_TYPE_INVALID);

	agent_prompt("Enter passkey (number in 0-999999): ");

	pending_message = dbus_message_ref(msg);

	return NULL;
}

static DBusMessage *display_passkey(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	const char *device;
	dbus_uint32_t passkey;
	dbus_uint16_t entered;
	char passkey_full[7];

	dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &device,
			DBUS_TYPE_UINT32, &passkey, DBUS_TYPE_UINT16, &entered,
							DBUS_TYPE_INVALID);

	snprintf(passkey_full, sizeof(passkey_full), "%.6u", passkey);
	passkey_full[6] = '\0';

	if (entered > strlen(passkey_full))
		entered = strlen(passkey_full);

	rl_printf(AGENT_PROMPT "Passkey: "
			COLOR_BOLDGRAY "%.*s" COLOR_BOLDWHITE "%s\n" COLOR_OFF,
				entered, passkey_full, passkey_full + entered);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *request_confirmation(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	const char *device;
	dbus_uint32_t passkey;
	char *str;

	rl_printf("Request confirmation\n");

	dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &device,
				DBUS_TYPE_UINT32, &passkey, DBUS_TYPE_INVALID);

	str = g_strdup_printf("Confirm passkey %06u (yes/no): ", passkey);
	agent_prompt(str);
	g_free(str);

	pending_message = dbus_message_ref(msg);

	return NULL;
}

static DBusMessage *request_authorization(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	const char *device;

	rl_printf("Request authorization\n");

	dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &device,
							DBUS_TYPE_INVALID);

	agent_prompt("Accept pairing (yes/no): ");

	pending_message = dbus_message_ref(msg);

	return NULL;
}

static DBusMessage *authorize_service(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	const char *device, *uuid;
	char *str;

	rl_printf("Authorize service\n");

	dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &device,
				DBUS_TYPE_STRING, &uuid, DBUS_TYPE_INVALID);

	str = g_strdup_printf("Authorize service %s (yes/no): ", uuid);
	agent_prompt(str);
	g_free(str);

	pending_message = dbus_message_ref(msg);

	return NULL;
}

static DBusMessage *cancel_request(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	rl_printf("Request canceled\n");

	agent_release_prompt();
	dbus_message_unref(pending_message);
	pending_message = NULL;

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable methods[] = {
	{ GDBUS_METHOD("Release", NULL, NULL, release_agent) },
	{ GDBUS_ASYNC_METHOD("RequestPinCode",
			GDBUS_ARGS({ "device", "o" }),
			GDBUS_ARGS({ "pincode", "s" }), request_pincode) },
	{ GDBUS_METHOD("DisplayPinCode",
			GDBUS_ARGS({ "device", "o" }, { "pincode", "s" }),
			NULL, display_pincode) },
	{ GDBUS_ASYNC_METHOD("RequestPasskey",
			GDBUS_ARGS({ "device", "o" }),
			GDBUS_ARGS({ "passkey", "u" }), request_passkey) },
	{ GDBUS_METHOD("DisplayPasskey",
			GDBUS_ARGS({ "device", "o" }, { "passkey", "u" },
							{ "entered", "q" }),
			NULL, display_passkey) },
	{ GDBUS_ASYNC_METHOD("RequestConfirmation",
			GDBUS_ARGS({ "device", "o" }, { "passkey", "u" }),
			NULL, request_confirmation) },
	{ GDBUS_ASYNC_METHOD("RequestAuthorization",
			GDBUS_ARGS({ "device", "o" }),
			NULL, request_authorization) },
	{ GDBUS_ASYNC_METHOD("AuthorizeService",
			GDBUS_ARGS({ "device", "o" }, { "uuid", "s" }),
			NULL,  authorize_service) },
	{ GDBUS_METHOD("Cancel", NULL, NULL, cancel_request) },
	{ }
};

static void register_agent_setup(DBusMessageIter *iter, void *user_data)
{
	const char *path = AGENT_PATH;
	const char *capability = agent_capability;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &capability);
}

static void register_agent_reply(DBusMessage *message, void *user_data)
{
	DBusConnection *conn = user_data;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == FALSE) {
		agent_registered = TRUE;
		rl_printf("Agent registered\n");
	} else {
		rl_printf("Failed to register agent: %s\n", error.name);
		dbus_error_free(&error);

		if (g_dbus_unregister_interface(conn, AGENT_PATH,
						AGENT_INTERFACE) == FALSE)
			rl_printf("Failed to unregister agent object\n");
	}
}

void agent_register(DBusConnection *conn, GDBusProxy *manager,
						const char *capability)

{
	if (agent_registered == TRUE) {
		rl_printf("Agent is already registered\n");
		return;
	}

	agent_capability = capability;

	if (g_dbus_register_interface(conn, AGENT_PATH,
					AGENT_INTERFACE, methods,
					NULL, NULL, NULL, NULL) == FALSE) {
		rl_printf("Failed to register agent object\n");
		return;
	}

	if (g_dbus_proxy_method_call(manager, "RegisterAgent",
						register_agent_setup,
						register_agent_reply,
						conn, NULL) == FALSE) {
		rl_printf("Failed to call register agent method\n");
		return;
	}

	agent_capability = NULL;
}

static void unregister_agent_setup(DBusMessageIter *iter, void *user_data)
{
	const char *path = AGENT_PATH;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
}

static void unregister_agent_reply(DBusMessage *message, void *user_data)
{
	DBusConnection *conn = user_data;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == FALSE) {
		rl_printf("Agent unregistered\n");
		agent_release(conn);
	} else {
		rl_printf("Failed to unregister agent: %s\n", error.name);
		dbus_error_free(&error);
	}
}

void agent_unregister(DBusConnection *conn, GDBusProxy *manager)
{
	if (agent_registered == FALSE) {
		rl_printf("No agent is registered\n");
		return;
	}

	if (!manager) {
		rl_printf("Agent unregistered\n");
		agent_release(conn);
		return;
	}

	if (g_dbus_proxy_method_call(manager, "UnregisterAgent",
						unregister_agent_setup,
						unregister_agent_reply,
						conn, NULL) == FALSE) {
		rl_printf("Failed to call unregister agent method\n");
		return;
	}
}

static void request_default_setup(DBusMessageIter *iter, void *user_data)
{
	const char *path = AGENT_PATH;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
}

static void request_default_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		rl_printf("Failed to request default agent: %s\n", error.name);
		dbus_error_free(&error);
		return;
	}

	rl_printf("Default agent request successful\n");
}

void agent_default(DBusConnection *conn, GDBusProxy *manager)
{
	if (agent_registered == FALSE) {
		rl_printf("No agent is registered\n");
		return;
	}

	if (g_dbus_proxy_method_call(manager, "RequestDefaultAgent",
						request_default_setup,
						request_default_reply,
						NULL, NULL) == FALSE) {
		rl_printf("Failed to call request default agent method\n");
		return;
	}
}
