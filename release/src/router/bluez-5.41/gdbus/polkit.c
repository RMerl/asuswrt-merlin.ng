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

#include <errno.h>

#include <dbus/dbus.h>

#include <glib.h>

int polkit_check_authorization(DBusConnection *conn,
				const char *action, gboolean interaction,
				void (*function) (dbus_bool_t authorized,
							void *user_data),
						void *user_data, int timeout);

static void add_dict_with_string_value(DBusMessageIter *iter,
					const char *key, const char *str)
{
	DBusMessageIter dict, entry, value;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
			DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
			DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
			DBUS_DICT_ENTRY_END_CHAR_AS_STRING, &dict);
	dbus_message_iter_open_container(&dict, DBUS_TYPE_DICT_ENTRY,
								NULL, &entry);

	dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &key);

	dbus_message_iter_open_container(&entry, DBUS_TYPE_VARIANT,
					DBUS_TYPE_STRING_AS_STRING, &value);
	dbus_message_iter_append_basic(&value, DBUS_TYPE_STRING, &str);
	dbus_message_iter_close_container(&entry, &value);

	dbus_message_iter_close_container(&dict, &entry);
	dbus_message_iter_close_container(iter, &dict);
}

static void add_empty_string_dict(DBusMessageIter *iter)
{
	DBusMessageIter dict;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
			DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
			DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_STRING_AS_STRING
			DBUS_DICT_ENTRY_END_CHAR_AS_STRING, &dict);

	dbus_message_iter_close_container(iter, &dict);
}

static void add_arguments(DBusConnection *conn, DBusMessageIter *iter,
				const char *action, dbus_uint32_t flags)
{
	const char *busname = dbus_bus_get_unique_name(conn);
	const char *kind = "system-bus-name";
	const char *cancel = "";
	DBusMessageIter subject;

	dbus_message_iter_open_container(iter, DBUS_TYPE_STRUCT,
							NULL, &subject);
	dbus_message_iter_append_basic(&subject, DBUS_TYPE_STRING, &kind);
	add_dict_with_string_value(&subject, "name", busname);
	dbus_message_iter_close_container(iter, &subject);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &action);
	add_empty_string_dict(iter);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32, &flags);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &cancel);
}

static dbus_bool_t parse_result(DBusMessageIter *iter)
{
	DBusMessageIter result;
	dbus_bool_t authorized, challenge;

	dbus_message_iter_recurse(iter, &result);

	dbus_message_iter_get_basic(&result, &authorized);
	dbus_message_iter_get_basic(&result, &challenge);

	return authorized;
}

struct authorization_data {
	void (*function) (dbus_bool_t authorized, void *user_data);
	void *user_data;
};

static void authorization_reply(DBusPendingCall *call, void *user_data)
{
	struct authorization_data *data = user_data;
	DBusMessage *reply;
	DBusMessageIter iter;
	dbus_bool_t authorized = FALSE;

	reply = dbus_pending_call_steal_reply(call);

	if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR)
		goto done;

	if (dbus_message_has_signature(reply, "(bba{ss})") == FALSE)
		goto done;

	dbus_message_iter_init(reply, &iter);

	authorized = parse_result(&iter);

done:
	if (data->function != NULL)
		data->function(authorized, data->user_data);

	dbus_message_unref(reply);

	dbus_pending_call_unref(call);
}

#define AUTHORITY_DBUS	"org.freedesktop.PolicyKit1"
#define AUTHORITY_INTF	"org.freedesktop.PolicyKit1.Authority"
#define AUTHORITY_PATH	"/org/freedesktop/PolicyKit1/Authority"

int polkit_check_authorization(DBusConnection *conn,
				const char *action, gboolean interaction,
				void (*function) (dbus_bool_t authorized,
							void *user_data),
						void *user_data, int timeout)
{
	struct authorization_data *data;
	DBusMessage *msg;
	DBusMessageIter iter;
	DBusPendingCall *call;
	dbus_uint32_t flags = 0x00000000;

	if (conn == NULL)
		return -EINVAL;

	data = dbus_malloc0(sizeof(*data));
	if (data == NULL)
		return -ENOMEM;

	msg = dbus_message_new_method_call(AUTHORITY_DBUS, AUTHORITY_PATH,
				AUTHORITY_INTF, "CheckAuthorization");
	if (msg == NULL) {
		dbus_free(data);
		return -ENOMEM;
	}

	if (interaction == TRUE)
		flags |= 0x00000001;

	if (action == NULL)
		action = "org.freedesktop.policykit.exec";

	dbus_message_iter_init_append(msg, &iter);
	add_arguments(conn, &iter, action, flags);

	if (dbus_connection_send_with_reply(conn, msg,
						&call, timeout) == FALSE) {
		dbus_message_unref(msg);
		dbus_free(data);
		return -EIO;
	}

	if (call == NULL) {
		dbus_message_unref(msg);
		dbus_free(data);
		return -EIO;
	}

	data->function = function;
	data->user_data = user_data;

	dbus_pending_call_set_notify(call, authorization_reply,
							data, dbus_free);

	dbus_message_unref(msg);

	return 0;
}
