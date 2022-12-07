// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013  Intel Corporation. All rights reserved.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <wordexp.h>
#include <ctype.h>

#include <glib.h>

#include "gdbus/gdbus.h"
#include "src/shared/shell.h"

/* String display constants */
#define COLORED_NEW	COLOR_GREEN "NEW" COLOR_OFF
#define COLORED_CHG	COLOR_YELLOW "CHG" COLOR_OFF
#define COLORED_DEL	COLOR_RED "DEL" COLOR_OFF

#define PROMPT_ON	COLOR_BLUE "[obex]" COLOR_OFF "# "
#define PROMPT_OFF	"[obex]# "

#define OBEX_SESSION_INTERFACE "org.bluez.obex.Session1"
#define OBEX_TRANSFER_INTERFACE "org.bluez.obex.Transfer1"
#define OBEX_CLIENT_INTERFACE "org.bluez.obex.Client1"
#define OBEX_OPP_INTERFACE "org.bluez.obex.ObjectPush1"
#define OBEX_FTP_INTERFACE "org.bluez.obex.FileTransfer1"
#define OBEX_PBAP_INTERFACE "org.bluez.obex.PhonebookAccess1"
#define OBEX_MAP_INTERFACE "org.bluez.obex.MessageAccess1"
#define OBEX_MSG_INTERFACE "org.bluez.obex.Message1"

static DBusConnection *dbus_conn;
static GDBusProxy *default_session;
static GList *sessions = NULL;
static GList *opps = NULL;
static GList *ftps = NULL;
static GList *pbaps = NULL;
static GList *maps = NULL;
static GList *msgs = NULL;
static GList *transfers = NULL;
static GDBusProxy *client = NULL;

struct transfer_data {
	uint64_t transferred;
	uint64_t size;
};

static void connect_handler(DBusConnection *connection, void *user_data)
{
	bt_shell_attach(fileno(stdin));
	bt_shell_set_prompt(PROMPT_ON);
}

static void disconnect_handler(DBusConnection *connection, void *user_data)
{
	bt_shell_detach();
	bt_shell_set_prompt(PROMPT_OFF);
}

static char *generic_generator(const char *text, int state, GList *source)
{
	static int index = 0;

	if (!state) {
		index = 0;
	}

	return g_dbus_proxy_path_lookup(source, &index, text);
}

static char *session_generator(const char *text, int state)
{
	return generic_generator(text, state, sessions);
}

static char *transfer_generator(const char *text, int state)
{
	return generic_generator(text, state, transfers);
}

static void connect_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to connect: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Connection successful\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

struct connect_args {
	char *dev;
	char *target;
};

static void connect_args_free(void *data)
{
	struct connect_args *args = data;

	g_free(args->dev);
	g_free(args->target);
	g_free(args);
}

static void connect_setup(DBusMessageIter *iter, void *user_data)
{
	struct connect_args *args = user_data;
	DBusMessageIter dict;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &args->dev);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	if (args->target == NULL)
		goto done;

	g_dbus_dict_append_entry(&dict, "Target",
					DBUS_TYPE_STRING, &args->target);

done:
	dbus_message_iter_close_container(iter, &dict);
}

static void cmd_connect(int argc, char *argv[])
{
	struct connect_args *args;
	const char *target = "opp";

	if (!client) {
		bt_shell_printf("Client proxy not available\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (argc > 2)
		target = argv[2];

	args = g_new0(struct connect_args, 1);
	args->dev = g_strdup(argv[1]);
	args->target = g_strdup(target);

	if (g_dbus_proxy_method_call(client, "CreateSession", connect_setup,
			connect_reply, args, connect_args_free) == FALSE) {
		bt_shell_printf("Failed to connect\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to connect to %s\n", argv[1]);
}

static void disconnect_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to disconnect: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Disconnection successful\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void disconnect_setup(DBusMessageIter *iter, void *user_data)
{
	GDBusProxy *proxy = user_data;
	const char *path;

	path = g_dbus_proxy_get_path(proxy);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
}

static void cmd_disconnect(int argc, char *argv[])
{
	GDBusProxy *proxy;

	if (argc > 1)
		proxy = g_dbus_proxy_lookup(sessions, NULL, argv[1],
						OBEX_SESSION_INTERFACE);
	else
		proxy = default_session;

	if (proxy == NULL) {
		bt_shell_printf("Session not available\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (g_dbus_proxy_method_call(client, "RemoveSession", disconnect_setup,
				disconnect_reply, proxy, NULL) == FALSE) {
		bt_shell_printf("Failed to disconnect\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to disconnect to %s\n",
						g_dbus_proxy_get_path(proxy));
}

static char *proxy_description(GDBusProxy *proxy, const char *title,
						const char *description)
{
	const char *path;

	path = g_dbus_proxy_get_path(proxy);

	return g_strdup_printf("%s%s%s%s %s ",
					description ? "[" : "",
					description ? : "",
					description ? "] " : "",
					title, path);
}

static void print_proxy(GDBusProxy *proxy, const char *title,
							const char *description)
{
	char *str;

	str = proxy_description(proxy, title, description);

	bt_shell_printf("%s%s\n", str,
			default_session == proxy ? "[default]" : "");

	g_free(str);
}

static void cmd_list(int argc, char *arg[])
{
	GList *l;

	for (l = sessions; l; l = g_list_next(l)) {
		GDBusProxy *proxy = l->data;
		print_proxy(proxy, "Session", NULL);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static bool check_default_session(void)
{
	if (!default_session) {
		bt_shell_printf("No default session available\n");
		return FALSE;
	}

	return TRUE;
}

static void print_iter(const char *label, const char *name,
						DBusMessageIter *iter)
{
	dbus_bool_t valbool;
	dbus_uint64_t valu64;
	dbus_uint32_t valu32;
	dbus_uint16_t valu16;
	dbus_int16_t vals16;
	const char *valstr;
	DBusMessageIter subiter;

	if (iter == NULL) {
		bt_shell_printf("%s%s is nil\n", label, name);
		return;
	}

	switch (dbus_message_iter_get_arg_type(iter)) {
	case DBUS_TYPE_INVALID:
		bt_shell_printf("%s%s is invalid\n", label, name);
		break;
	case DBUS_TYPE_STRING:
	case DBUS_TYPE_OBJECT_PATH:
		dbus_message_iter_get_basic(iter, &valstr);
		bt_shell_printf("%s%s: %s\n", label, name, valstr);
		break;
	case DBUS_TYPE_BOOLEAN:
		dbus_message_iter_get_basic(iter, &valbool);
		bt_shell_printf("%s%s: %s\n", label, name,
					valbool == TRUE ? "yes" : "no");
		break;
	case DBUS_TYPE_UINT64:
		dbus_message_iter_get_basic(iter, &valu64);
		bt_shell_printf("%s%s: %" PRIu64 "\n", label, name, valu64);
		break;
	case DBUS_TYPE_UINT32:
		dbus_message_iter_get_basic(iter, &valu32);
		bt_shell_printf("%s%s: 0x%08x\n", label, name, valu32);
		break;
	case DBUS_TYPE_UINT16:
		dbus_message_iter_get_basic(iter, &valu16);
		bt_shell_printf("%s%s: 0x%04x\n", label, name, valu16);
		break;
	case DBUS_TYPE_INT16:
		dbus_message_iter_get_basic(iter, &vals16);
		bt_shell_printf("%s%s: %d\n", label, name, vals16);
		break;
	case DBUS_TYPE_VARIANT:
		dbus_message_iter_recurse(iter, &subiter);
		print_iter(label, name, &subiter);
		break;
	case DBUS_TYPE_ARRAY:
		dbus_message_iter_recurse(iter, &subiter);
		while (dbus_message_iter_get_arg_type(&subiter) !=
							DBUS_TYPE_INVALID) {
			print_iter(label, name, &subiter);
			dbus_message_iter_next(&subiter);
		}
		break;
	case DBUS_TYPE_DICT_ENTRY:
		dbus_message_iter_recurse(iter, &subiter);
		dbus_message_iter_get_basic(&subiter, &valstr);
		dbus_message_iter_next(&subiter);
		print_iter(label, valstr, &subiter);
		break;
	default:
		bt_shell_printf("%s%s has unsupported type\n", label, name);
		break;
	}
}

static void print_property(GDBusProxy *proxy, const char *name)
{
	DBusMessageIter iter;

	if (g_dbus_proxy_get_property(proxy, name, &iter) == FALSE)
		return;

	print_iter("\t", name, &iter);
}

static void cmd_show(int argc, char *argv[])
{
	GDBusProxy *proxy;

	if (argc < 2) {
		if (check_default_session() == FALSE)
			return bt_shell_noninteractive_quit(EXIT_FAILURE);

		proxy = default_session;
	} else {
		proxy = g_dbus_proxy_lookup(sessions, NULL, argv[1],
						OBEX_SESSION_INTERFACE);
		if (!proxy) {
			bt_shell_printf("Session %s not available\n", argv[1]);
			return bt_shell_noninteractive_quit(EXIT_FAILURE);
		}
	}

	bt_shell_printf("Session %s\n", g_dbus_proxy_get_path(proxy));

	print_property(proxy, "Destination");
	print_property(proxy, "Target");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void set_default_session(GDBusProxy *proxy)
{
	char *desc;
	DBusMessageIter iter;

	default_session = proxy;

	if (!g_dbus_proxy_get_property(proxy, "Destination", &iter)) {
		desc = g_strdup(PROMPT_ON);
		goto done;
	}

	dbus_message_iter_get_basic(&iter, &desc);
	desc = g_strdup_printf(COLOR_BLUE "[%s]" COLOR_OFF "# ", desc);

done:
	bt_shell_set_prompt(desc);
	g_free(desc);
}

static void cmd_select(int argc, char *argv[])
{
	GDBusProxy *proxy;

	proxy = g_dbus_proxy_lookup(sessions, NULL, argv[1],
						OBEX_SESSION_INTERFACE);
	if (proxy == NULL) {
		bt_shell_printf("Session %s not available\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (default_session == proxy)
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);

	set_default_session(proxy);

	print_proxy(proxy, "Session", NULL);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void transfer_info(GDBusProxy *proxy, int argc, char *argv[])
{
	bt_shell_printf("Transfer %s\n", g_dbus_proxy_get_path(proxy));

	print_property(proxy, "Session");
	print_property(proxy, "Name");
	print_property(proxy, "Type");
	print_property(proxy, "Status");
	print_property(proxy, "Time");
	print_property(proxy, "Size");
	print_property(proxy, "Transferred");
	print_property(proxy, "Filename");
}

static void message_info(GDBusProxy *proxy, int argc, char *argv[])
{
	bt_shell_printf("Message %s\n", g_dbus_proxy_get_path(proxy));

	print_property(proxy, "Folder");
	print_property(proxy, "Subject");
	print_property(proxy, "Timestamp");
	print_property(proxy, "Sender");
	print_property(proxy, "SenderAddress");
	print_property(proxy, "ReplyTo");
	print_property(proxy, "Recipient");
	print_property(proxy, "RecipientAddress");
	print_property(proxy, "Type");
	print_property(proxy, "Size");
	print_property(proxy, "Status");
	print_property(proxy, "Priority");
	print_property(proxy, "Read");
	print_property(proxy, "Deleted");
	print_property(proxy, "Sent");
	print_property(proxy, "Protected");
}

static void cmd_info(int argc, char *argv[])
{
	GDBusProxy *proxy;

	proxy = g_dbus_proxy_lookup(transfers, NULL, argv[1],
						OBEX_TRANSFER_INTERFACE);
	if (proxy) {
		transfer_info(proxy, argc, argv);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	proxy = g_dbus_proxy_lookup(msgs, NULL, argv[1], OBEX_MSG_INTERFACE);
	if (proxy) {
		message_info(proxy, argc, argv);
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	bt_shell_printf("Object %s not available\n", argv[1]);

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void cancel_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to cancel: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Cancel successful\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_cancel(int argc, char *argv[])
{
	GDBusProxy *proxy;

	proxy = g_dbus_proxy_lookup(transfers, NULL, argv[1],
						OBEX_TRANSFER_INTERFACE);
	if (!proxy) {
		bt_shell_printf("Transfer %s not available\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (g_dbus_proxy_method_call(proxy, "Cancel", NULL, cancel_reply, NULL,
							NULL) == FALSE) {
		bt_shell_printf("Failed to cancel transfer\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to cancel transfer %s\n",
						g_dbus_proxy_get_path(proxy));
}

static void suspend_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to suspend: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Suspend successful\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_suspend(int argc, char *argv[])
{
	GDBusProxy *proxy;

	proxy = g_dbus_proxy_lookup(transfers, NULL, argv[1],
						OBEX_TRANSFER_INTERFACE);
	if (!proxy) {
		bt_shell_printf("Transfer %s not available\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (g_dbus_proxy_method_call(proxy, "Suspend", NULL, suspend_reply,
						NULL, NULL) == FALSE) {
		bt_shell_printf("Failed to suspend transfer\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to suspend transfer %s\n",
						g_dbus_proxy_get_path(proxy));
}

static void resume_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to resume: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Resume successful\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_resume(int argc, char *argv[])
{
	GDBusProxy *proxy;

	proxy = g_dbus_proxy_lookup(transfers, NULL, argv[1],
						OBEX_TRANSFER_INTERFACE);
	if (!proxy) {
		bt_shell_printf("Transfer %s not available\n", argv[1]);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (g_dbus_proxy_method_call(proxy, "Resume", NULL, resume_reply,
						NULL, NULL) == FALSE) {
		bt_shell_printf("Failed to resume transfer\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to resume transfer %s\n",
						g_dbus_proxy_get_path(proxy));
}

static void print_dict_iter(DBusMessageIter *iter)
{
	DBusMessageIter dict;
	int ctype;

	ctype = dbus_message_iter_get_arg_type(iter);
	if (ctype != DBUS_TYPE_ARRAY)
		return;

	dbus_message_iter_recurse(iter, &dict);

	while ((ctype = dbus_message_iter_get_arg_type(&dict)) !=
							DBUS_TYPE_INVALID) {
		DBusMessageIter entry;
		const char *key;

		if (ctype != DBUS_TYPE_DICT_ENTRY)
			return;

		dbus_message_iter_recurse(&dict, &entry);
		if (dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_STRING)
			return;

		dbus_message_iter_get_basic(&entry, &key);
		dbus_message_iter_next(&entry);

		print_iter("\t", key, &entry);

		dbus_message_iter_next(&dict);
	}
}

static void print_transfer_iter(DBusMessageIter *iter)
{
	const char *path;

	dbus_message_iter_get_basic(iter, &path);

	bt_shell_printf("Transfer %s\n", path);

	dbus_message_iter_next(iter);

	print_dict_iter(iter);
}

static void send_reply(DBusMessage *message, void *user_data)
{
	DBusMessageIter iter;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to send/pull: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	dbus_message_iter_init(message, &iter);

	print_transfer_iter(&iter);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void send_setup(DBusMessageIter *iter, void *user_data)
{
	const char *file = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &file);
}

static void opp_send(GDBusProxy *proxy, int argc, char *argv[])
{
	if (g_dbus_proxy_method_call(proxy, "SendFile", send_setup, send_reply,
					g_strdup(argv[1]), g_free) == FALSE) {
		bt_shell_printf("Failed to send\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to send %s to %s\n", argv[1],
						g_dbus_proxy_get_path(proxy));
}

static void opp_pull(GDBusProxy *proxy, int argc, char *argv[])
{
	if (g_dbus_proxy_method_call(proxy, "PullBusinessCard", send_setup,
			send_reply, g_strdup(argv[1]), g_free) == FALSE) {
		bt_shell_printf("Failed to pull\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to pull %s from %s\n", argv[1],
						g_dbus_proxy_get_path(proxy));
}

static void push_reply(DBusMessage *message, void *user_data)
{
	DBusMessageIter iter;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to PushMessage: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	dbus_message_iter_init(message, &iter);

	print_transfer_iter(&iter);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void push_setup(DBusMessageIter *iter, void *user_data)
{
	const char *file = user_data;
	const char *folder = "";
	DBusMessageIter dict;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &file);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &folder);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	dbus_message_iter_close_container(iter, &dict);
}

static void map_send(GDBusProxy *proxy, int argc, char *argv[])
{
	if (g_dbus_proxy_method_call(proxy, "PushMessage", push_setup,
					push_reply, g_strdup(argv[1]),
					g_free) == FALSE) {
		bt_shell_printf("Failed to send\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to send %s to %s\n", argv[1],
						g_dbus_proxy_get_path(proxy));
}

static void cmd_send(int argc, char *argv[])
{
	const char *path = g_dbus_proxy_get_path(default_session);
	GDBusProxy *proxy;

	if (!check_default_session())
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	proxy = g_dbus_proxy_lookup(opps, NULL, path, OBEX_OPP_INTERFACE);
	if (proxy) {
		opp_send(proxy, argc, argv);
		return;
	}

	proxy = g_dbus_proxy_lookup(maps, NULL, path, OBEX_MAP_INTERFACE);
	if (proxy) {
		map_send(proxy, argc, argv);
		return;
	}

	bt_shell_printf("Command not supported\n");

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void cmd_pull(int argc, char *argv[])
{
	const char *path = g_dbus_proxy_get_path(default_session);
	GDBusProxy *proxy;

	if (!check_default_session())
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	proxy = g_dbus_proxy_lookup(opps, NULL, path, OBEX_OPP_INTERFACE);
	if (proxy) {
		opp_pull(proxy, argc, argv);
		return;
	}

	bt_shell_printf("Command not supported\n");

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void change_folder_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to ChangeFolder: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("ChangeFolder successful\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void change_folder_setup(DBusMessageIter *iter, void *user_data)
{
	const char *folder = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &folder);
}

static void select_reply(DBusMessage *message, void *user_data)
{
	DBusMessageIter iter;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to Select: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	dbus_message_iter_init(message, &iter);

	bt_shell_printf("Select successful\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void select_setup(DBusMessageIter *iter, void *user_data)
{
	const char *folder = user_data;
	const char *location = "int";

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &location);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &folder);
}

static void setfolder_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to SetFolder: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("SetFolder successful\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void setfolder_setup(DBusMessageIter *iter, void *user_data)
{
	const char *folder = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &folder);
}

static void ftp_cd(GDBusProxy *proxy, int argc, char *argv[])
{
	if (g_dbus_proxy_method_call(proxy, "ChangeFolder", change_folder_setup,
					change_folder_reply, g_strdup(argv[1]),
					g_free) == FALSE) {
		bt_shell_printf("Failed to ChangeFolder\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to ChangeFolder to %s\n", argv[1]);
}

static void pbap_cd(GDBusProxy *proxy, int argc, char *argv[])
{
	if (g_dbus_proxy_method_call(proxy, "Select", select_setup,
					select_reply, g_strdup(argv[1]),
					g_free) == FALSE) {
		bt_shell_printf("Failed to Select\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to Select to %s\n", argv[1]);
}

static void map_cd(GDBusProxy *proxy, int argc, char *argv[])
{
	if (g_dbus_proxy_method_call(proxy, "SetFolder", setfolder_setup,
					setfolder_reply, g_strdup(argv[1]),
					g_free) == FALSE) {
		bt_shell_printf("Failed to SetFolder\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to SetFolder to %s\n", argv[1]);
}

static void cmd_cd(int argc, char *argv[])
{
	const char *path = g_dbus_proxy_get_path(default_session);
	GDBusProxy *proxy;

	if (!check_default_session())
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	proxy = g_dbus_proxy_lookup(ftps, NULL, path, OBEX_FTP_INTERFACE);
	if (proxy) {
		ftp_cd(proxy, argc, argv);
		return;
	}

	proxy = g_dbus_proxy_lookup(pbaps, NULL, path, OBEX_PBAP_INTERFACE);
	if (proxy) {
		pbap_cd(proxy, argc, argv);
		return;
	}

	proxy = g_dbus_proxy_lookup(maps, NULL, path, OBEX_MAP_INTERFACE);
	if (proxy) {
		map_cd(proxy, argc, argv);
		return;
	}

	bt_shell_printf("Command not supported\n");

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void list_folder_reply(DBusMessage *message, void *user_data)
{
	DBusMessageIter iter, array;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to ListFolder: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	dbus_message_iter_init(message, &iter);

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);

	dbus_message_iter_recurse(&iter, &array);

	while (dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_INVALID) {
		print_dict_iter(&array);
		dbus_message_iter_next(&array);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void ftp_ls(GDBusProxy *proxy, int argc, char *argv[])
{
	if (g_dbus_proxy_method_call(proxy, "ListFolder", NULL,
						list_folder_reply, NULL,
						NULL) == FALSE) {
		bt_shell_printf("Failed to ls\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to ListFolder\n");
}

static void parse_list_reply(DBusMessage *message)
{
	DBusMessageIter iter, array;

	dbus_message_iter_init(message, &iter);

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	dbus_message_iter_recurse(&iter, &array);

	while (dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_INVALID) {
		DBusMessageIter entry;
		const char *vcard;

		if (dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_STRUCT)
			return;

		dbus_message_iter_recurse(&array, &entry);

		dbus_message_iter_get_basic(&entry, &vcard);
		dbus_message_iter_next(&entry);
		print_iter("\t", vcard, &entry);
		dbus_message_iter_next(&array);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void list_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to List: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	parse_list_reply(message);
}

static void list_setup(DBusMessageIter *iter, void *user_data)
{
	DBusMessageIter dict;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	dbus_message_iter_close_container(iter, &dict);
}

static void search_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to Search: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	parse_list_reply(message);
}

static void search_setup(DBusMessageIter *iter, void *user_data)
{
	const char *value = user_data;
	const char *field;
	DBusMessageIter dict;

	field = isalpha(value[0]) ? "name" : "number";

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &field);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &value);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	dbus_message_iter_close_container(iter, &dict);
}

static void pbap_search(GDBusProxy *proxy, int argc, char *argv[])
{
	if (g_dbus_proxy_method_call(proxy, "Search", search_setup,
					search_reply, g_strdup(argv[1]),
					g_free) == FALSE) {
		bt_shell_printf("Failed to Search\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to Search\n");
}

static void list_folders_reply(DBusMessage *message, void *user_data)
{
	DBusError error;
	DBusMessageIter iter, array;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to ListFolders: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	dbus_message_iter_init(message, &iter);

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	dbus_message_iter_recurse(&iter, &array);

	while (dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_INVALID) {
		print_dict_iter(&array);
		dbus_message_iter_next(&array);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void list_folders_setup(DBusMessageIter *iter, void *user_data)
{
	DBusMessageIter dict;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	dbus_message_iter_close_container(iter, &dict);
}

static void list_messages_reply(DBusMessage *message, void *user_data)
{
	DBusError error;
	DBusMessageIter iter, array;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to ListFolders: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	dbus_message_iter_init(message, &iter);

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	dbus_message_iter_recurse(&iter, &array);

	while ((dbus_message_iter_get_arg_type(&array)) ==
						DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter entry;
		const char *obj;

		dbus_message_iter_recurse(&array, &entry);
		dbus_message_iter_get_basic(&entry, &obj);
		bt_shell_printf("\t%s\n", obj);
		dbus_message_iter_next(&array);
	}

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void list_messages_setup(DBusMessageIter *iter, void *user_data)
{
	const char *folder = user_data;
	DBusMessageIter dict;

	if (strcmp(folder, "*") == 0)
		folder = "";

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &folder);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	dbus_message_iter_close_container(iter, &dict);
}

static void pbap_list(GDBusProxy *proxy, int argc, char *argv[])
{
	if (g_dbus_proxy_method_call(proxy, "List", list_setup, list_reply,
						NULL, NULL) == FALSE) {
		bt_shell_printf("Failed to List\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to List\n");
}

static void get_size_reply(DBusMessage *message, void *user_data)
{
	GDBusProxy *proxy = user_data;
	DBusError error;
	DBusMessageIter iter;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to GetSize: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	dbus_message_iter_init(message, &iter);

	print_iter("\t", "Size", &iter);

	pbap_list(proxy, 0, NULL);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void pbap_get_size(GDBusProxy *proxy, int argc, char *argv[])
{
	if (g_dbus_proxy_method_call(proxy, "GetSize", NULL, get_size_reply,
						proxy, NULL) == FALSE) {
		bt_shell_printf("Failed to GetSize\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to GetSize\n");
}

static void pbap_ls(GDBusProxy *proxy, int argc, char *argv[])
{
	if (argc > 1) {
		if (strcmp("-l", argv[1]))
			pbap_search(proxy, argc, argv);
		else
			pbap_get_size(proxy, argc, argv);
		return;
	}

	pbap_list(proxy, argc, argv);
}

static void map_ls_messages(GDBusProxy *proxy, int argc, char *argv[])
{
	if (g_dbus_proxy_method_call(proxy, "ListMessages", list_messages_setup,
					list_messages_reply, g_strdup(argv[1]),
					g_free) == FALSE) {
		bt_shell_printf("Failed to ListMessages\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to ListMessages\n");
}

static void map_ls(GDBusProxy *proxy, int argc, char *argv[])
{
	if (argc > 1) {
		map_ls_messages(proxy, argc, argv);
		return;
	}

	if (g_dbus_proxy_method_call(proxy, "ListFolders", list_folders_setup,
						list_folders_reply, NULL,
						NULL) == FALSE) {
		bt_shell_printf("Failed to ListFolders\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to ListFolders\n");
}

static void cmd_ls(int argc, char *argv[])
{
	const char *path = g_dbus_proxy_get_path(default_session);
	GDBusProxy *proxy;

	if (!check_default_session())
		return;

	proxy = g_dbus_proxy_lookup(ftps, NULL, path, OBEX_FTP_INTERFACE);
	if (proxy) {
		ftp_ls(proxy, argc, argv);
		return;
	}

	proxy = g_dbus_proxy_lookup(pbaps, NULL, path, OBEX_PBAP_INTERFACE);
	if (proxy) {
		pbap_ls(proxy, argc, argv);
		return;
	}

	proxy = g_dbus_proxy_lookup(maps, NULL, path, OBEX_MAP_INTERFACE);
	if (proxy) {
		map_ls(proxy, argc, argv);
		return;
	}

	bt_shell_printf("Command not supported\n");

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

struct cp_args {
	char *source;
	char *target;
};

static void cp_free(void *data)
{
	struct cp_args *args = data;

	g_free(args->source);
	g_free(args->target);
	g_free(args);
}

static struct cp_args *cp_new(char *argv[])
{
	struct cp_args *args;
	const char *source;
	const char *target;

	source = rindex(argv[1], ':');
	if (source == NULL)
		source = argv[1];
	else
		source++;

	target = rindex(argv[2], ':');
	if (target == NULL)
		target = argv[2];
	else
		target++;

	args = g_new0(struct cp_args, 1);
	args->source = g_strdup(source);
	args->target = g_strdup(target);

	return args;
}

static void cp_setup(DBusMessageIter *iter, void *user_data)
{
	struct cp_args *args = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &args->source);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &args->target);
}

static void copy_file_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to CopyFile: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("CopyFile successful\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void ftp_copy(GDBusProxy *proxy, int argc, char *argv[])
{
	struct cp_args *args;

	args = cp_new(argv);

	if (g_dbus_proxy_method_call(proxy, "CopyFile", cp_setup,
				copy_file_reply, args, cp_free) == FALSE) {
		bt_shell_printf("Failed to CopyFile\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to CopyFile\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void get_file_reply(DBusMessage *message, void *user_data)
{
	DBusError error;
	DBusMessageIter iter;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to GetFile: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	dbus_message_iter_init(message, &iter);

	print_transfer_iter(&iter);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void get_file_setup(DBusMessageIter *iter, void *user_data)
{
	struct cp_args *args = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &args->target);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &args->source);
}

static void ftp_get(GDBusProxy *proxy, int argc, char *argv[])
{
	struct cp_args *args;

	if (rindex(argv[2], ':') == NULL)
		return ftp_copy(proxy, argc, argv);

	args = cp_new(argv);

	if (g_dbus_proxy_method_call(proxy, "GetFile", get_file_setup,
				get_file_reply, args, cp_free) == FALSE) {
		bt_shell_printf("Failed to GetFile\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to GetFile\n");
}

static void put_file_reply(DBusMessage *message, void *user_data)
{
	DBusError error;
	DBusMessageIter iter;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to PutFile: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	dbus_message_iter_init(message, &iter);

	print_transfer_iter(&iter);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void ftp_put(GDBusProxy *proxy, int argc, char *argv[])
{
	struct cp_args *args;

	if (rindex(argv[2], ':') != NULL) {
		bt_shell_printf("Invalid target file argument\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	args = cp_new(argv);

	if (g_dbus_proxy_method_call(proxy, "PutFile", cp_setup, put_file_reply,
						args, cp_free) == FALSE) {
		bt_shell_printf("Failed to PutFile\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to PutFile\n");
}

static void ftp_cp(GDBusProxy *proxy, int argc, char *argv[])
{
	if (rindex(argv[1], ':') == NULL)
		return ftp_get(proxy, argc, argv);

	return ftp_put(proxy, argc, argv);
}

static void pull_all_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to PullAll: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}


	bt_shell_printf("PullAll successful\n");
}

static void pull_all_setup(DBusMessageIter *iter, void *user_data)
{
	const char *file = user_data;
	DBusMessageIter dict;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &file);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	dbus_message_iter_close_container(iter, &dict);
}

static void pbap_pull_all(GDBusProxy *proxy, int argc, char *argv[])
{
	if (g_dbus_proxy_method_call(proxy, "PullAll", pull_all_setup,
					pull_all_reply, g_strdup(argv[2]),
					g_free) == FALSE) {
		bt_shell_printf("Failed to PullAll\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to PullAll\n");
}

static void pull_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to Pull: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}


	bt_shell_printf("Pull successful\n");
}

static void pull_setup(DBusMessageIter *iter, void *user_data)
{
	struct cp_args *args = user_data;
	DBusMessageIter dict;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &args->source);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &args->target);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	dbus_message_iter_close_container(iter, &dict);
}

static void pbap_pull(GDBusProxy *proxy, int argc, char *argv[])
{
	struct cp_args *args;

	args = cp_new(argv);

	if (g_dbus_proxy_method_call(proxy, "Pull", pull_setup, pull_reply,
						args, cp_free) == FALSE) {
		bt_shell_printf("Failed to Pull\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to Pull\n");
}

static void pbap_cp(GDBusProxy *proxy, int argc, char *argv[])
{
	if (strcmp(argv[1], "*") == 0 || strcmp(argv[1], "*.vcf") == 0)
		return pbap_pull_all(proxy, argc, argv);

	return pbap_pull(proxy, argc, argv);
}

static void get_reply(DBusMessage *message, void *user_data)
{
	DBusMessageIter iter;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to Get: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	dbus_message_iter_init(message, &iter);

	print_transfer_iter(&iter);

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void get_setup(DBusMessageIter *iter, void *user_data)
{
	const char *file = user_data;
	dbus_bool_t attachment = TRUE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &file);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &attachment);
}

static void map_cp(GDBusProxy *proxy, int argc, char *argv[])
{
	GDBusProxy *obj;

	obj = g_dbus_proxy_lookup(msgs, NULL, argv[1], OBEX_MSG_INTERFACE);
	if (obj == NULL) {
		bt_shell_printf("Invalid message argument\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (g_dbus_proxy_method_call(obj, "Get", get_setup, get_reply,
					g_strdup(argv[2]), g_free) == FALSE) {
		bt_shell_printf("Failed to Get\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to Get\n");
}

static void cmd_cp(int argc, char *argv[])
{
	const char *path = g_dbus_proxy_get_path(default_session);
	GDBusProxy *proxy;

	if (!check_default_session())
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	proxy = g_dbus_proxy_lookup(ftps, NULL, path, OBEX_FTP_INTERFACE);
	if (proxy) {
		ftp_cp(proxy, argc, argv);
		return;
	}

	proxy = g_dbus_proxy_lookup(pbaps, NULL, path, OBEX_PBAP_INTERFACE);
	if (proxy) {
		pbap_cp(proxy, argc, argv);
		return;
	}

	proxy = g_dbus_proxy_lookup(maps, NULL, path, OBEX_MAP_INTERFACE);
	if (proxy) {
		map_cp(proxy, argc, argv);
		return;
	}

	bt_shell_printf("Command not supported\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void move_file_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to MoveFile: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("MoveFile successful\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void cmd_mv(int argc, char *argv[])
{
	const char *path = g_dbus_proxy_get_path(default_session);
	GDBusProxy *proxy;
	struct cp_args *args;

	if (!check_default_session())
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	proxy = g_dbus_proxy_lookup(ftps, NULL, path, OBEX_FTP_INTERFACE);
	if (proxy == NULL) {
		bt_shell_printf("Command not supported\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	args = cp_new(argv);

	if (g_dbus_proxy_method_call(proxy, "MoveFile", cp_setup,
				move_file_reply, args, cp_free) == FALSE) {
		bt_shell_printf("Failed to MoveFile\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to MoveFile\n");
}

static void delete_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to Delete: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Delete successful\n");

	return bt_shell_noninteractive_quit(EXIT_SUCCESS);
}

static void delete_setup(DBusMessageIter *iter, void *user_data)
{
	const char *file = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &file);
}

static void ftp_rm(GDBusProxy *proxy, int argc, char *argv[])
{
	if (g_dbus_proxy_method_call(proxy, "Delete", delete_setup,
					delete_reply, g_strdup(argv[1]),
					g_free) == FALSE) {
		bt_shell_printf("Failed to Delete\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to Delete\n");
}

static void set_delete_reply(const DBusError *error, void *user_data)
{
	if (dbus_error_is_set(error)) {
		bt_shell_printf("Failed to set Deleted: %s\n", error->name);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	} else {
		bt_shell_printf("Set Deleted successful\n");
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}
}

static void map_rm(GDBusProxy *proxy, int argc, char *argv[])
{
	GDBusProxy *msg;
	dbus_bool_t value = TRUE;

	msg = g_dbus_proxy_lookup(msgs, NULL, argv[1], OBEX_MSG_INTERFACE);
	if (msg == NULL) {
		bt_shell_printf("Invalid message argument\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (g_dbus_proxy_set_property_basic(msg, "Deleted", DBUS_TYPE_BOOLEAN,
						&value, set_delete_reply,
						NULL, NULL) == FALSE) {
		bt_shell_printf("Failed to set Deleted\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to set Deleted\n");
}

static void cmd_rm(int argc, char *argv[])
{
	const char *path = g_dbus_proxy_get_path(default_session);
	GDBusProxy *proxy;

	if (!check_default_session())
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	proxy = g_dbus_proxy_lookup(ftps, NULL, path, OBEX_FTP_INTERFACE);
	if (proxy) {
		ftp_rm(proxy, argc, argv);
		return;
	}

	proxy = g_dbus_proxy_lookup(maps, NULL, path, OBEX_MAP_INTERFACE);
	if (proxy) {
		map_rm(proxy, argc, argv);
		return;
	}

	bt_shell_printf("Command not supported\n");

	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void create_folder_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, message) == TRUE) {
		bt_shell_printf("Failed to CreateFolder: %s\n", error.name);
		dbus_error_free(&error);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("CreateFolder successful\n");
}

static void create_folder_setup(DBusMessageIter *iter, void *user_data)
{
	const char *folder = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &folder);
}

static void cmd_mkdir(int argc, char *argv[])
{
	const char *path = g_dbus_proxy_get_path(default_session);
	GDBusProxy *proxy;

	if (!check_default_session())
		return bt_shell_noninteractive_quit(EXIT_FAILURE);

	proxy = g_dbus_proxy_lookup(ftps, NULL, path, OBEX_FTP_INTERFACE);
	if (proxy == NULL) {
		bt_shell_printf("Command not supported\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	if (g_dbus_proxy_method_call(proxy, "CreateFolder", create_folder_setup,
					create_folder_reply, g_strdup(argv[1]),
					g_free) == FALSE) {
		bt_shell_printf("Failed to CreateFolder\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	bt_shell_printf("Attempting to CreateFolder\n");
}

static const struct bt_shell_menu main_menu = {
	.name = "main",
	.entries = {
	{ "connect",      "<dev> [uuid]", cmd_connect, "Connect session" },
	{ "disconnect",   "[session]", cmd_disconnect, "Disconnect session",
						session_generator },
	{ "list",         NULL,       cmd_list, "List available sessions" },
	{ "show",         "[session]", cmd_show, "Session information",
						session_generator },
	{ "select",       "<session>", cmd_select, "Select default session",
						session_generator },
	{ "info",         "<object>", cmd_info, "Object information",
						transfer_generator },
	{ "cancel",       "<transfer>", cmd_cancel, "Cancel transfer",
						transfer_generator },
	{ "suspend",      "<transfer>", cmd_suspend, "Suspend transfer",
						transfer_generator },
	{ "resume",       "<transfer>", cmd_resume, "Resume transfer",
						transfer_generator },
	{ "send",         "<file>",   cmd_send, "Send file" },
	{ "pull",	  "<file>",   cmd_pull,
					"Pull Vobject & stores in file" },
	{ "cd",           "<path>",   cmd_cd, "Change current folder" },
	{ "ls",           "<options>", cmd_ls, "List current folder" },
	{ "cp",          "<source file> <destination file>",   cmd_cp,
				"Copy source file to destination file" },
	{ "mv",          "<source file> <destination file>",   cmd_mv,
				"Move source file to destination file" },
	{ "rm",          "<file>",    cmd_rm, "Delete file" },
	{ "mkdir",       "<folder>",    cmd_mkdir, "Create folder" },
	{} },
};

static void client_added(GDBusProxy *proxy)
{
	if (client == NULL)
		client = proxy;

	print_proxy(proxy, "Client", COLORED_NEW);
}

static void session_added(GDBusProxy *proxy)
{
	sessions = g_list_append(sessions, proxy);

	if (default_session == NULL)
		set_default_session(proxy);

	print_proxy(proxy, "Session", COLORED_NEW);
}

static void print_transferred(struct transfer_data *data, const char *str,
							DBusMessageIter *iter)
{
	dbus_uint64_t valu64;
	uint64_t speed;
	int seconds, minutes;

	dbus_message_iter_get_basic(iter, &valu64);

	/*
	 * Use the file size to output the proper size/speed since obexd resets
	 * the current transferred size to zero on completion of transfer.
	 */
	if (valu64 == 0)
		valu64 = data->size;

	speed = valu64 - data->transferred;
	data->transferred = valu64;

	if (data->size == 0) {
		bt_shell_printf("%sTransferred: %" PRIu64 " (@%" PRIu64
				"KB/s)\n", str, valu64, speed / 1000);
		return;
	}

	seconds = (data->size - data->transferred) / speed;
	minutes = seconds / 60;
	seconds %= 60;
	bt_shell_printf("%sTransferred: %" PRIu64 " (@%" PRIu64
			"KB/s %02u:%02u)\n", str, valu64, speed / 1000,
			minutes, seconds);
}

static void transfer_property_changed(GDBusProxy *proxy, const char *name,
					DBusMessageIter *iter, void *user_data)
{
	struct transfer_data *data = user_data;
	char *str;

	if (iter == NULL)
		return;

	str = proxy_description(proxy, "Transfer", COLORED_CHG);

	if (strcmp(name, "Transferred") == 0) {
		print_transferred(data, str, iter);
		goto done;
	}

	if (strcmp(name, "Size") == 0)
		dbus_message_iter_get_basic(iter, &data->size);

	print_iter(str, name, iter);

done:
	g_free(str);
}

static void transfer_destroy(GDBusProxy *proxy, void *user_data)
{
	struct transfer_data *data = user_data;

	g_free(data);
}

static void transfer_added(GDBusProxy *proxy)
{
	struct transfer_data *data;
	DBusMessageIter iter;

	transfers = g_list_append(transfers, proxy);

	print_proxy(proxy, "Transfer", COLORED_NEW);

	data = g_new0(struct transfer_data, 1);

	if (g_dbus_proxy_get_property(proxy, "Transferred", &iter))
		dbus_message_iter_get_basic(&iter, &data->transferred);

	if (g_dbus_proxy_get_property(proxy, "Size", &iter))
		dbus_message_iter_get_basic(&iter, &data->size);

	g_dbus_proxy_set_property_watch(proxy, transfer_property_changed, data);
	g_dbus_proxy_set_removed_watch(proxy, transfer_destroy, data);
}

static void opp_added(GDBusProxy *proxy)
{
	opps = g_list_append(opps, proxy);

	print_proxy(proxy, "ObjectPush", COLORED_NEW);
}

static void ftp_added(GDBusProxy *proxy)
{
	ftps = g_list_append(ftps, proxy);

	print_proxy(proxy, "FileTransfer", COLORED_NEW);
}

static void pbap_added(GDBusProxy *proxy)
{
	pbaps = g_list_append(pbaps, proxy);

	print_proxy(proxy, "PhonebookAccess", COLORED_NEW);
}

static void map_added(GDBusProxy *proxy)
{
	maps = g_list_append(maps, proxy);

	print_proxy(proxy, "MessageAccess", COLORED_NEW);
}

static void msg_added(GDBusProxy *proxy)
{
	msgs = g_list_append(msgs, proxy);

	print_proxy(proxy, "Message", COLORED_NEW);
}

static void proxy_added(GDBusProxy *proxy, void *user_data)
{
	const char *interface;

	interface = g_dbus_proxy_get_interface(proxy);

	if (!strcmp(interface, OBEX_CLIENT_INTERFACE))
		client_added(proxy);
	else if (!strcmp(interface, OBEX_SESSION_INTERFACE))
		session_added(proxy);
	else if (!strcmp(interface, OBEX_TRANSFER_INTERFACE))
		transfer_added(proxy);
	else if (!strcmp(interface, OBEX_OPP_INTERFACE))
		opp_added(proxy);
	else if (!strcmp(interface, OBEX_FTP_INTERFACE))
		ftp_added(proxy);
	else if (!strcmp(interface, OBEX_PBAP_INTERFACE))
		pbap_added(proxy);
	else if (!strcmp(interface, OBEX_MAP_INTERFACE))
		map_added(proxy);
	else if (!strcmp(interface, OBEX_MSG_INTERFACE))
		msg_added(proxy);
}

static void client_removed(GDBusProxy *proxy)
{
	print_proxy(proxy, "Client", COLORED_DEL);

	if (client == proxy)
		client = NULL;
}

static void session_removed(GDBusProxy *proxy)
{
	print_proxy(proxy, "Session", COLORED_DEL);

	if (default_session == proxy)
		set_default_session(NULL);

	sessions = g_list_remove(sessions, proxy);
}

static void transfer_removed(GDBusProxy *proxy)
{
	print_proxy(proxy, "Transfer", COLORED_DEL);

	transfers = g_list_remove(transfers, proxy);
}

static void opp_removed(GDBusProxy *proxy)
{
	print_proxy(proxy, "ObjectPush", COLORED_DEL);

	opps = g_list_remove(opps, proxy);
}

static void ftp_removed(GDBusProxy *proxy)
{
	print_proxy(proxy, "FileTransfer", COLORED_DEL);

	ftps = g_list_remove(ftps, proxy);
}

static void pbap_removed(GDBusProxy *proxy)
{
	print_proxy(proxy, "PhonebookAccess", COLORED_DEL);

	pbaps = g_list_remove(pbaps, proxy);
}

static void map_removed(GDBusProxy *proxy)
{
	print_proxy(proxy, "MessageAccess", COLORED_DEL);

	maps = g_list_remove(maps, proxy);
}

static void msg_removed(GDBusProxy *proxy)
{
	print_proxy(proxy, "Message", COLORED_DEL);

	msgs = g_list_remove(msgs, proxy);
}

static void proxy_removed(GDBusProxy *proxy, void *user_data)
{
	const char *interface;

	interface = g_dbus_proxy_get_interface(proxy);

	if (!strcmp(interface, OBEX_CLIENT_INTERFACE))
		client_removed(proxy);
	else if (!strcmp(interface, OBEX_SESSION_INTERFACE))
		session_removed(proxy);
	else if (!strcmp(interface, OBEX_TRANSFER_INTERFACE))
		transfer_removed(proxy);
	else if (!strcmp(interface, OBEX_OPP_INTERFACE))
		opp_removed(proxy);
	else if (!strcmp(interface, OBEX_FTP_INTERFACE))
		ftp_removed(proxy);
	else if (!strcmp(interface, OBEX_PBAP_INTERFACE))
		pbap_removed(proxy);
	else if (!strcmp(interface, OBEX_MAP_INTERFACE))
		map_removed(proxy);
	else if (!strcmp(interface, OBEX_MSG_INTERFACE))
		msg_removed(proxy);
}

static void session_property_changed(GDBusProxy *proxy, const char *name,
						DBusMessageIter *iter)
{
	char *str;

	str = proxy_description(proxy, "Session", COLORED_CHG);
	print_iter(str, name, iter);
	g_free(str);
}

static void property_changed(GDBusProxy *proxy, const char *name,
					DBusMessageIter *iter, void *user_data)
{
	const char *interface;

	interface = g_dbus_proxy_get_interface(proxy);

	if (!strcmp(interface, OBEX_SESSION_INTERFACE))
		session_property_changed(proxy, name, iter);
}

int main(int argc, char *argv[])
{
	GDBusClient *client;
	int status;

	bt_shell_init(argc, argv, NULL);
	bt_shell_set_menu(&main_menu);
	bt_shell_set_prompt(PROMPT_OFF);

	dbus_conn = g_dbus_setup_bus(DBUS_BUS_SESSION, NULL, NULL);

	client = g_dbus_client_new(dbus_conn, "org.bluez.obex",
							"/org/bluez/obex");

	g_dbus_client_set_connect_watch(client, connect_handler, NULL);
	g_dbus_client_set_disconnect_watch(client, disconnect_handler, NULL);

	g_dbus_client_set_proxy_handlers(client, proxy_added, proxy_removed,
							property_changed, NULL);

	status = bt_shell_run();

	g_dbus_client_unref(client);

	dbus_connection_unref(dbus_conn);

	return status;
}
