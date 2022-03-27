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

#include <stdio.h>
#include <glib.h>
#include <dbus/dbus.h>

#include "gdbus.h"

#define METHOD_CALL_TIMEOUT (300 * 1000)

#ifndef DBUS_INTERFACE_OBJECT_MANAGER
#define DBUS_INTERFACE_OBJECT_MANAGER DBUS_INTERFACE_DBUS ".ObjectManager"
#endif

struct GDBusClient {
	int ref_count;
	DBusConnection *dbus_conn;
	char *service_name;
	char *base_path;
	char *root_path;
	guint watch;
	guint added_watch;
	guint removed_watch;
	GPtrArray *match_rules;
	DBusPendingCall *pending_call;
	DBusPendingCall *get_objects_call;
	GDBusWatchFunction connect_func;
	void *connect_data;
	GDBusWatchFunction disconn_func;
	gboolean connected;
	void *disconn_data;
	GDBusMessageFunction signal_func;
	void *signal_data;
	GDBusProxyFunction proxy_added;
	GDBusProxyFunction proxy_removed;
	GDBusClientFunction ready;
	void *ready_data;
	GDBusPropertyFunction property_changed;
	void *user_data;
	GList *proxy_list;
};

struct GDBusProxy {
	int ref_count;
	GDBusClient *client;
	char *obj_path;
	char *interface;
	GHashTable *prop_list;
	guint watch;
	GDBusPropertyFunction prop_func;
	void *prop_data;
	GDBusProxyFunction removed_func;
	void *removed_data;
};

struct prop_entry {
	char *name;
	int type;
	DBusMessage *msg;
};

static void modify_match_reply(DBusPendingCall *call, void *user_data)
{
	DBusMessage *reply = dbus_pending_call_steal_reply(call);
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, reply) == TRUE)
		dbus_error_free(&error);

	dbus_message_unref(reply);
}

static gboolean modify_match(DBusConnection *conn, const char *member,
							const char *rule)
{
	DBusMessage *msg;
	DBusPendingCall *call;

	msg = dbus_message_new_method_call(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS,
					DBUS_INTERFACE_DBUS, member);
	if (msg == NULL)
		return FALSE;

	dbus_message_append_args(msg, DBUS_TYPE_STRING, &rule,
						DBUS_TYPE_INVALID);

	if (g_dbus_send_message_with_reply(conn, msg, &call, -1) == FALSE) {
		dbus_message_unref(msg);
		return FALSE;
	}

	dbus_pending_call_set_notify(call, modify_match_reply, NULL, NULL);
	dbus_pending_call_unref(call);

	dbus_message_unref(msg);

	return TRUE;
}

static void iter_append_iter(DBusMessageIter *base, DBusMessageIter *iter)
{
	int type;

	type = dbus_message_iter_get_arg_type(iter);

	if (dbus_type_is_basic(type)) {
		const void *value;

		dbus_message_iter_get_basic(iter, &value);
		dbus_message_iter_append_basic(base, type, &value);
	} else if (dbus_type_is_container(type)) {
		DBusMessageIter iter_sub, base_sub;
		char *sig;

		dbus_message_iter_recurse(iter, &iter_sub);

		switch (type) {
		case DBUS_TYPE_ARRAY:
		case DBUS_TYPE_VARIANT:
			sig = dbus_message_iter_get_signature(&iter_sub);
			break;
		default:
			sig = NULL;
			break;
		}

		dbus_message_iter_open_container(base, type, sig, &base_sub);

		if (sig != NULL)
			dbus_free(sig);

		while (dbus_message_iter_get_arg_type(&iter_sub) !=
							DBUS_TYPE_INVALID) {
			iter_append_iter(&base_sub, &iter_sub);
			dbus_message_iter_next(&iter_sub);
		}

		dbus_message_iter_close_container(base, &base_sub);
	}
}

static void prop_entry_update(struct prop_entry *prop, DBusMessageIter *iter)
{
	DBusMessage *msg;
	DBusMessageIter base;

	msg = dbus_message_new(DBUS_MESSAGE_TYPE_METHOD_RETURN);
	if (msg == NULL)
		return;

	dbus_message_iter_init_append(msg, &base);
	iter_append_iter(&base, iter);

	if (prop->msg != NULL)
		dbus_message_unref(prop->msg);

	prop->msg = dbus_message_copy(msg);
	dbus_message_unref(msg);
}

static struct prop_entry *prop_entry_new(const char *name,
						DBusMessageIter *iter)
{
	struct prop_entry *prop;

	prop = g_try_new0(struct prop_entry, 1);
	if (prop == NULL)
		return NULL;

	prop->name = g_strdup(name);
	prop->type = dbus_message_iter_get_arg_type(iter);

	prop_entry_update(prop, iter);

	return prop;
}

static void prop_entry_free(gpointer data)
{
	struct prop_entry *prop = data;

	if (prop->msg != NULL)
		dbus_message_unref(prop->msg);

	g_free(prop->name);

	g_free(prop);
}

static void add_property(GDBusProxy *proxy, const char *name,
				DBusMessageIter *iter, gboolean send_changed)
{
	GDBusClient *client = proxy->client;
	DBusMessageIter value;
	struct prop_entry *prop;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_VARIANT)
		return;

	dbus_message_iter_recurse(iter, &value);

	prop = g_hash_table_lookup(proxy->prop_list, name);
	if (prop != NULL) {
		prop_entry_update(prop, &value);
		goto done;
	}

	prop = prop_entry_new(name, &value);
	if (prop == NULL)
		return;

	g_hash_table_replace(proxy->prop_list, prop->name, prop);

done:
	if (proxy->prop_func)
		proxy->prop_func(proxy, name, &value, proxy->prop_data);

	if (client == NULL || send_changed == FALSE)
		return;

	if (client->property_changed)
		client->property_changed(proxy, name, &value,
							client->user_data);
}

static void update_properties(GDBusProxy *proxy, DBusMessageIter *iter,
							gboolean send_changed)
{
	DBusMessageIter dict;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return;

	dbus_message_iter_recurse(iter, &dict);

	while (dbus_message_iter_get_arg_type(&dict) == DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter entry;
		const char *name;

		dbus_message_iter_recurse(&dict, &entry);

		if (dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_STRING)
			break;

		dbus_message_iter_get_basic(&entry, &name);
		dbus_message_iter_next(&entry);

		add_property(proxy, name, &entry, send_changed);

		dbus_message_iter_next(&dict);
	}
}

static void get_all_properties_reply(DBusPendingCall *call, void *user_data)
{
	GDBusProxy *proxy = user_data;
	GDBusClient *client = proxy->client;
	DBusMessage *reply = dbus_pending_call_steal_reply(call);
	DBusMessageIter iter;
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, reply) == TRUE) {
		dbus_error_free(&error);
		goto done;
	}

	dbus_message_iter_init(reply, &iter);

	update_properties(proxy, &iter, FALSE);

done:
	if (g_list_find(client->proxy_list, proxy) == NULL) {
		if (client->proxy_added)
			client->proxy_added(proxy, client->user_data);

		client->proxy_list = g_list_append(client->proxy_list, proxy);
	}

	dbus_message_unref(reply);

	g_dbus_client_unref(client);
}

static void get_all_properties(GDBusProxy *proxy)
{
	GDBusClient *client = proxy->client;
	const char *service_name = client->service_name;
	DBusMessage *msg;
	DBusPendingCall *call;

	msg = dbus_message_new_method_call(service_name, proxy->obj_path,
					DBUS_INTERFACE_PROPERTIES, "GetAll");
	if (msg == NULL)
		return;

	dbus_message_append_args(msg, DBUS_TYPE_STRING, &proxy->interface,
							DBUS_TYPE_INVALID);

	if (g_dbus_send_message_with_reply(client->dbus_conn, msg,
							&call, -1) == FALSE) {
		dbus_message_unref(msg);
		return;
	}

	g_dbus_client_ref(client);

	dbus_pending_call_set_notify(call, get_all_properties_reply,
							proxy, NULL);
	dbus_pending_call_unref(call);

	dbus_message_unref(msg);
}

static GDBusProxy *proxy_lookup(GDBusClient *client, const char *path,
						const char *interface)
{
	GList *list;

	for (list = g_list_first(client->proxy_list); list;
						list = g_list_next(list)) {
		GDBusProxy *proxy = list->data;

		if (g_str_equal(proxy->interface, interface) == TRUE &&
				g_str_equal(proxy->obj_path, path) == TRUE)
			return proxy;
        }

	return NULL;
}

static gboolean properties_changed(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	GDBusProxy *proxy = user_data;
	GDBusClient *client = proxy->client;
	DBusMessageIter iter, entry;
	const char *interface;

	if (dbus_message_iter_init(msg, &iter) == FALSE)
		return TRUE;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING)
		return TRUE;

	dbus_message_iter_get_basic(&iter, &interface);
	dbus_message_iter_next(&iter);

	update_properties(proxy, &iter, TRUE);

	dbus_message_iter_next(&iter);

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return TRUE;

	dbus_message_iter_recurse(&iter, &entry);

	while (dbus_message_iter_get_arg_type(&entry) == DBUS_TYPE_STRING) {
		const char *name;

		dbus_message_iter_get_basic(&entry, &name);

		g_hash_table_remove(proxy->prop_list, name);

		if (proxy->prop_func)
			proxy->prop_func(proxy, name, NULL, proxy->prop_data);

		if (client->property_changed)
			client->property_changed(proxy, name, NULL,
							client->user_data);

		dbus_message_iter_next(&entry);
	}

	return TRUE;
}

static GDBusProxy *proxy_new(GDBusClient *client, const char *path,
						const char *interface)
{
	GDBusProxy *proxy;

	proxy = g_try_new0(GDBusProxy, 1);
	if (proxy == NULL)
		return NULL;

	proxy->client = client;
	proxy->obj_path = g_strdup(path);
	proxy->interface = g_strdup(interface);

	proxy->prop_list = g_hash_table_new_full(g_str_hash, g_str_equal,
							NULL, prop_entry_free);
	proxy->watch = g_dbus_add_properties_watch(client->dbus_conn,
							client->service_name,
							proxy->obj_path,
							proxy->interface,
							properties_changed,
							proxy, NULL);

	return g_dbus_proxy_ref(proxy);
}

static void proxy_free(gpointer data)
{
	GDBusProxy *proxy = data;

	if (proxy->client) {
		GDBusClient *client = proxy->client;

		if (client->proxy_removed)
			client->proxy_removed(proxy, client->user_data);

		g_dbus_remove_watch(client->dbus_conn, proxy->watch);

		g_hash_table_remove_all(proxy->prop_list);

		proxy->client = NULL;
	}

	if (proxy->removed_func)
		proxy->removed_func(proxy, proxy->removed_data);

	g_dbus_proxy_unref(proxy);
}

static void proxy_remove(GDBusClient *client, const char *path,
						const char *interface)
{
	GList *list;

	for (list = g_list_first(client->proxy_list); list;
						list = g_list_next(list)) {
		GDBusProxy *proxy = list->data;

		if (g_str_equal(proxy->interface, interface) == TRUE &&
				g_str_equal(proxy->obj_path, path) == TRUE) {
			client->proxy_list =
				g_list_delete_link(client->proxy_list, list);
			proxy_free(proxy);
			break;
		}
	}
}

GDBusProxy *g_dbus_proxy_new(GDBusClient *client, const char *path,
							const char *interface)
{
	GDBusProxy *proxy;

	if (client == NULL)
		return NULL;

	proxy = proxy_lookup(client, path, interface);
	if (proxy)
		return g_dbus_proxy_ref(proxy);

	proxy = proxy_new(client, path, interface);
	if (proxy == NULL)
		return NULL;

	get_all_properties(proxy);

	return g_dbus_proxy_ref(proxy);
}

GDBusProxy *g_dbus_proxy_ref(GDBusProxy *proxy)
{
	if (proxy == NULL)
		return NULL;

	__sync_fetch_and_add(&proxy->ref_count, 1);

	return proxy;
}

void g_dbus_proxy_unref(GDBusProxy *proxy)
{
	if (proxy == NULL)
		return;

	if (__sync_sub_and_fetch(&proxy->ref_count, 1) > 0)
		return;

	g_hash_table_destroy(proxy->prop_list);

	g_free(proxy->obj_path);
	g_free(proxy->interface);

	g_free(proxy);
}

const char *g_dbus_proxy_get_path(GDBusProxy *proxy)
{
	if (proxy == NULL)
		return NULL;

	return proxy->obj_path;
}

const char *g_dbus_proxy_get_interface(GDBusProxy *proxy)
{
	if (proxy == NULL)
		return NULL;

	return proxy->interface;
}

gboolean g_dbus_proxy_get_property(GDBusProxy *proxy, const char *name,
                                                        DBusMessageIter *iter)
{
	struct prop_entry *prop;

	if (proxy == NULL || name == NULL)
		return FALSE;

	prop = g_hash_table_lookup(proxy->prop_list, name);
	if (prop == NULL)
		return FALSE;

	if (prop->msg == NULL)
		return FALSE;

	if (dbus_message_iter_init(prop->msg, iter) == FALSE)
		return FALSE;

	return TRUE;
}

struct refresh_property_data {
	GDBusProxy *proxy;
	char *name;
};

static void refresh_property_free(gpointer user_data)
{
	struct refresh_property_data *data = user_data;

	g_free(data->name);
	g_free(data);
}

static void refresh_property_reply(DBusPendingCall *call, void *user_data)
{
	struct refresh_property_data *data = user_data;
	DBusMessage *reply = dbus_pending_call_steal_reply(call);
	DBusError error;

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, reply) == FALSE) {
		DBusMessageIter iter;

		dbus_message_iter_init(reply, &iter);

		add_property(data->proxy, data->name, &iter, TRUE);
	} else
		dbus_error_free(&error);

	dbus_message_unref(reply);
}

gboolean g_dbus_proxy_refresh_property(GDBusProxy *proxy, const char *name)
{
	struct refresh_property_data *data;
	GDBusClient *client;
	DBusMessage *msg;
	DBusMessageIter iter;
	DBusPendingCall *call;

	if (proxy == NULL || name == NULL)
		return FALSE;

	client = proxy->client;
	if (client == NULL)
		return FALSE;

	data = g_try_new0(struct refresh_property_data, 1);
	if (data == NULL)
		return FALSE;

	data->proxy = proxy;
	data->name = g_strdup(name);

	msg = dbus_message_new_method_call(client->service_name,
			proxy->obj_path, DBUS_INTERFACE_PROPERTIES, "Get");
	if (msg == NULL) {
		refresh_property_free(data);
		return FALSE;
	}

	dbus_message_iter_init_append(msg, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING,
							&proxy->interface);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &name);

	if (g_dbus_send_message_with_reply(client->dbus_conn, msg,
							&call, -1) == FALSE) {
		dbus_message_unref(msg);
		refresh_property_free(data);
		return FALSE;
	}

	dbus_pending_call_set_notify(call, refresh_property_reply,
						data, refresh_property_free);
	dbus_pending_call_unref(call);

	dbus_message_unref(msg);

	return TRUE;
}

struct set_property_data {
	GDBusResultFunction function;
	void *user_data;
	GDBusDestroyFunction destroy;
};

static void set_property_reply(DBusPendingCall *call, void *user_data)
{
	struct set_property_data *data = user_data;
	DBusMessage *reply = dbus_pending_call_steal_reply(call);
	DBusError error;

	dbus_error_init(&error);

	dbus_set_error_from_message(&error, reply);

	if (data->function)
		data->function(&error, data->user_data);

	if (data->destroy)
		data->destroy(data->user_data);

	dbus_error_free(&error);

	dbus_message_unref(reply);
}

gboolean g_dbus_proxy_set_property_basic(GDBusProxy *proxy,
				const char *name, int type, const void *value,
				GDBusResultFunction function, void *user_data,
				GDBusDestroyFunction destroy)
{
	struct set_property_data *data;
	GDBusClient *client;
	DBusMessage *msg;
	DBusMessageIter iter, variant;
	DBusPendingCall *call;
	char type_as_str[2];

	if (proxy == NULL || name == NULL || value == NULL)
		return FALSE;

	if (dbus_type_is_basic(type) == FALSE)
		return FALSE;

	client = proxy->client;
	if (client == NULL)
		return FALSE;

	data = g_try_new0(struct set_property_data, 1);
	if (data == NULL)
		return FALSE;

	data->function = function;
	data->user_data = user_data;
	data->destroy = destroy;

	msg = dbus_message_new_method_call(client->service_name,
			proxy->obj_path, DBUS_INTERFACE_PROPERTIES, "Set");
	if (msg == NULL) {
		g_free(data);
		return FALSE;
	}

	type_as_str[0] = (char) type;
	type_as_str[1] = '\0';

	dbus_message_iter_init_append(msg, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING,
							&proxy->interface);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &name);

	dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT,
						type_as_str, &variant);
	dbus_message_iter_append_basic(&variant, type, value);
	dbus_message_iter_close_container(&iter, &variant);

	if (g_dbus_send_message_with_reply(client->dbus_conn, msg,
							&call, -1) == FALSE) {
		dbus_message_unref(msg);
		g_free(data);
		return FALSE;
	}

	dbus_pending_call_set_notify(call, set_property_reply, data, g_free);
	dbus_pending_call_unref(call);

	dbus_message_unref(msg);

	return TRUE;
}

gboolean g_dbus_proxy_set_property_array(GDBusProxy *proxy,
				const char *name, int type, const void *value,
				size_t size, GDBusResultFunction function,
				void *user_data, GDBusDestroyFunction destroy)
{
	struct set_property_data *data;
	GDBusClient *client;
	DBusMessage *msg;
	DBusMessageIter iter, variant, array;
	DBusPendingCall *call;
	char array_sig[3];
	char type_sig[2];

	if (!proxy || !name || !value)
		return FALSE;

	if (!dbus_type_is_basic(type))
		return FALSE;

	client = proxy->client;
	if (!client)
		return FALSE;

	data = g_try_new0(struct set_property_data, 1);
	if (!data)
		return FALSE;

	data->function = function;
	data->user_data = user_data;
	data->destroy = destroy;

	msg = dbus_message_new_method_call(client->service_name,
						proxy->obj_path,
						DBUS_INTERFACE_PROPERTIES,
						"Set");
	if (!msg) {
		g_free(data);
		return FALSE;
	}

	array_sig[0] = DBUS_TYPE_ARRAY;
	array_sig[1] = (char) type;
	array_sig[2] = '\0';

	type_sig[0] = (char) type;
	type_sig[1] = '\0';

	dbus_message_iter_init_append(msg, &iter);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING,
							&proxy->interface);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &name);

	dbus_message_iter_open_container(&iter, DBUS_TYPE_VARIANT,
							array_sig, &variant);

	dbus_message_iter_open_container(&variant, DBUS_TYPE_ARRAY,
							type_sig, &array);

	if (dbus_type_is_fixed(type))
		dbus_message_iter_append_fixed_array(&array, type, &value,
									size);
	else if (type == DBUS_TYPE_STRING || type == DBUS_TYPE_OBJECT_PATH) {
		const char **str = (const char **) value;
		size_t i;

		for (i = 0; i < size; i++)
			dbus_message_iter_append_basic(&array, type, &str[i]);
	}

	dbus_message_iter_close_container(&variant, &array);
	dbus_message_iter_close_container(&iter, &variant);

	if (g_dbus_send_message_with_reply(client->dbus_conn, msg,
							&call, -1) == FALSE) {
		dbus_message_unref(msg);
		g_free(data);
		return FALSE;
	}

	dbus_pending_call_set_notify(call, set_property_reply, data, g_free);
	dbus_pending_call_unref(call);

	dbus_message_unref(msg);

	return TRUE;
}

struct method_call_data {
	GDBusReturnFunction function;
	void *user_data;
	GDBusDestroyFunction destroy;
};

static void method_call_reply(DBusPendingCall *call, void *user_data)
{
	struct method_call_data *data = user_data;
	DBusMessage *reply = dbus_pending_call_steal_reply(call);

	if (data->function)
		data->function(reply, data->user_data);

	if (data->destroy)
		data->destroy(data->user_data);

	dbus_message_unref(reply);
}

gboolean g_dbus_proxy_method_call(GDBusProxy *proxy, const char *method,
				GDBusSetupFunction setup,
				GDBusReturnFunction function, void *user_data,
				GDBusDestroyFunction destroy)
{
	struct method_call_data *data;
	GDBusClient *client;
	DBusMessage *msg;
	DBusPendingCall *call;

	if (proxy == NULL || method == NULL)
		return FALSE;

	client = proxy->client;
	if (client == NULL)
		return FALSE;

	msg = dbus_message_new_method_call(client->service_name,
				proxy->obj_path, proxy->interface, method);
	if (msg == NULL)
		return FALSE;

	if (setup) {
		DBusMessageIter iter;

		dbus_message_iter_init_append(msg, &iter);
		setup(&iter, user_data);
	}

	if (!function)
		return g_dbus_send_message(client->dbus_conn, msg);

	data = g_try_new0(struct method_call_data, 1);
	if (data == NULL)
		return FALSE;

	data->function = function;
	data->user_data = user_data;
	data->destroy = destroy;


	if (g_dbus_send_message_with_reply(client->dbus_conn, msg,
					&call, METHOD_CALL_TIMEOUT) == FALSE) {
		dbus_message_unref(msg);
		g_free(data);
		return FALSE;
	}

	dbus_pending_call_set_notify(call, method_call_reply, data, g_free);
	dbus_pending_call_unref(call);

	dbus_message_unref(msg);

	return TRUE;
}

gboolean g_dbus_proxy_set_property_watch(GDBusProxy *proxy,
			GDBusPropertyFunction function, void *user_data)
{
	if (proxy == NULL)
		return FALSE;

	proxy->prop_func = function;
	proxy->prop_data = user_data;

	return TRUE;
}

gboolean g_dbus_proxy_set_removed_watch(GDBusProxy *proxy,
				GDBusProxyFunction function, void *user_data)
{
	if (proxy == NULL)
		return FALSE;

	proxy->removed_func = function;
	proxy->removed_data = user_data;

	return TRUE;
}

static void refresh_properties(GDBusClient *client)
{
	GList *list;

	for (list = g_list_first(client->proxy_list); list;
						list = g_list_next(list)) {
		GDBusProxy *proxy = list->data;

		get_all_properties(proxy);
        }
}

static void parse_properties(GDBusClient *client, const char *path,
				const char *interface, DBusMessageIter *iter)
{
	GDBusProxy *proxy;

	if (g_str_equal(interface, DBUS_INTERFACE_INTROSPECTABLE) == TRUE)
		return;

	if (g_str_equal(interface, DBUS_INTERFACE_PROPERTIES) == TRUE)
		return;

	proxy = proxy_lookup(client, path, interface);
	if (proxy) {
		update_properties(proxy, iter, FALSE);
		return;
	}

	proxy = proxy_new(client, path, interface);
	if (proxy == NULL)
		return;

	update_properties(proxy, iter, FALSE);

	if (client->proxy_added)
		client->proxy_added(proxy, client->user_data);

	client->proxy_list = g_list_append(client->proxy_list, proxy);
}

static void parse_interfaces(GDBusClient *client, const char *path,
						DBusMessageIter *iter)
{
	DBusMessageIter dict;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return;

	dbus_message_iter_recurse(iter, &dict);

	while (dbus_message_iter_get_arg_type(&dict) == DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter entry;
		const char *interface;

		dbus_message_iter_recurse(&dict, &entry);

		if (dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_STRING)
			break;

		dbus_message_iter_get_basic(&entry, &interface);
		dbus_message_iter_next(&entry);

		parse_properties(client, path, interface, &entry);

		dbus_message_iter_next(&dict);
	}
}

static gboolean interfaces_added(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	GDBusClient *client = user_data;
	DBusMessageIter iter;
	const char *path;

	if (dbus_message_iter_init(msg, &iter) == FALSE)
		return TRUE;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_OBJECT_PATH)
		return TRUE;

	dbus_message_iter_get_basic(&iter, &path);
	dbus_message_iter_next(&iter);

	g_dbus_client_ref(client);

	parse_interfaces(client, path, &iter);

	g_dbus_client_unref(client);

	return TRUE;
}

static gboolean interfaces_removed(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	GDBusClient *client = user_data;
	DBusMessageIter iter, entry;
	const char *path;

	if (dbus_message_iter_init(msg, &iter) == FALSE)
		return TRUE;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_OBJECT_PATH)
		return TRUE;

	dbus_message_iter_get_basic(&iter, &path);
	dbus_message_iter_next(&iter);

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return TRUE;

	dbus_message_iter_recurse(&iter, &entry);

	g_dbus_client_ref(client);

	while (dbus_message_iter_get_arg_type(&entry) == DBUS_TYPE_STRING) {
		const char *interface;

		dbus_message_iter_get_basic(&entry, &interface);
		proxy_remove(client, path, interface);
		dbus_message_iter_next(&entry);
	}

	g_dbus_client_unref(client);

	return TRUE;
}

static void parse_managed_objects(GDBusClient *client, DBusMessage *msg)
{
	DBusMessageIter iter, dict;

	if (dbus_message_iter_init(msg, &iter) == FALSE)
		return;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return;

	dbus_message_iter_recurse(&iter, &dict);

	while (dbus_message_iter_get_arg_type(&dict) == DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter entry;
		const char *path;

		dbus_message_iter_recurse(&dict, &entry);

		if (dbus_message_iter_get_arg_type(&entry) !=
							DBUS_TYPE_OBJECT_PATH)
			break;

		dbus_message_iter_get_basic(&entry, &path);
		dbus_message_iter_next(&entry);

		parse_interfaces(client, path, &entry);

		dbus_message_iter_next(&dict);
	}
}

static void get_managed_objects_reply(DBusPendingCall *call, void *user_data)
{
	GDBusClient *client = user_data;
	DBusMessage *reply = dbus_pending_call_steal_reply(call);
	DBusError error;

	g_dbus_client_ref(client);

	dbus_error_init(&error);

	if (dbus_set_error_from_message(&error, reply) == TRUE) {
		dbus_error_free(&error);
		goto done;
	}

	parse_managed_objects(client, reply);

done:
	if (client->ready)
		client->ready(client, client->ready_data);

	dbus_message_unref(reply);

	dbus_pending_call_unref(client->get_objects_call);
	client->get_objects_call = NULL;

	g_dbus_client_unref(client);
}

static void get_managed_objects(GDBusClient *client)
{
	DBusMessage *msg;

	if (!client->connected)
		return;

	if ((!client->proxy_added && !client->proxy_removed) ||
							!client->root_path) {
		refresh_properties(client);
		return;
	}

	if (client->get_objects_call != NULL)
		return;

	msg = dbus_message_new_method_call(client->service_name,
						client->root_path,
						DBUS_INTERFACE_OBJECT_MANAGER,
						"GetManagedObjects");
	if (msg == NULL)
		return;

	dbus_message_append_args(msg, DBUS_TYPE_INVALID);

	if (g_dbus_send_message_with_reply(client->dbus_conn, msg,
				&client->get_objects_call, -1) == FALSE) {
		dbus_message_unref(msg);
		return;
	}

	dbus_pending_call_set_notify(client->get_objects_call,
						get_managed_objects_reply,
						client, NULL);

	dbus_message_unref(msg);
}

static void service_connect(DBusConnection *conn, void *user_data)
{
	GDBusClient *client = user_data;

	g_dbus_client_ref(client);

	client->connected = TRUE;

	if (client->connect_func)
		client->connect_func(conn, client->connect_data);

	get_managed_objects(client);

	g_dbus_client_unref(client);
}

static void service_disconnect(DBusConnection *conn, void *user_data)
{
	GDBusClient *client = user_data;

	client->connected = FALSE;

	g_list_free_full(client->proxy_list, proxy_free);
	client->proxy_list = NULL;

	if (client->disconn_func)
		client->disconn_func(conn, client->disconn_data);
}

static DBusHandlerResult message_filter(DBusConnection *connection,
					DBusMessage *message, void *user_data)
{
	GDBusClient *client = user_data;
	const char *sender, *path, *interface;

	if (dbus_message_get_type(message) != DBUS_MESSAGE_TYPE_SIGNAL)
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	sender = dbus_message_get_sender(message);
	if (sender == NULL)
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	path = dbus_message_get_path(message);
	interface = dbus_message_get_interface(message);

	if (g_str_has_prefix(path, client->base_path) == FALSE)
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	if (g_str_equal(interface, DBUS_INTERFACE_PROPERTIES) == TRUE)
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	if (client->signal_func)
		client->signal_func(connection, message, client->signal_data);

	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

GDBusClient *g_dbus_client_new(DBusConnection *connection,
					const char *service, const char *path)
{
	return g_dbus_client_new_full(connection, service, path, "/");
}

GDBusClient *g_dbus_client_new_full(DBusConnection *connection,
							const char *service,
							const char *path,
							const char *root_path)
{
	GDBusClient *client;
	unsigned int i;

	if (!connection || !service)
		return NULL;

	client = g_try_new0(GDBusClient, 1);
	if (client == NULL)
		return NULL;

	if (dbus_connection_add_filter(connection, message_filter,
						client, NULL) == FALSE) {
		g_free(client);
		return NULL;
	}

	client->dbus_conn = dbus_connection_ref(connection);
	client->service_name = g_strdup(service);
	client->base_path = g_strdup(path);
	client->root_path = g_strdup(root_path);
	client->connected = FALSE;

	client->match_rules = g_ptr_array_sized_new(1);
	g_ptr_array_set_free_func(client->match_rules, g_free);

	client->watch = g_dbus_add_service_watch(connection, service,
						service_connect,
						service_disconnect,
						client, NULL);

	if (!root_path)
		return g_dbus_client_ref(client);

	client->added_watch = g_dbus_add_signal_watch(connection, service,
						client->root_path,
						DBUS_INTERFACE_OBJECT_MANAGER,
						"InterfacesAdded",
						interfaces_added,
						client, NULL);
	client->removed_watch = g_dbus_add_signal_watch(connection, service,
						client->root_path,
						DBUS_INTERFACE_OBJECT_MANAGER,
						"InterfacesRemoved",
						interfaces_removed,
						client, NULL);
	g_ptr_array_add(client->match_rules, g_strdup_printf("type='signal',"
				"sender='%s',path_namespace='%s'",
				client->service_name, client->base_path));

	for (i = 0; i < client->match_rules->len; i++) {
		modify_match(client->dbus_conn, "AddMatch",
				g_ptr_array_index(client->match_rules, i));
	}

	return g_dbus_client_ref(client);
}

GDBusClient *g_dbus_client_ref(GDBusClient *client)
{
	if (client == NULL)
		return NULL;

	__sync_fetch_and_add(&client->ref_count, 1);

	return client;
}

void g_dbus_client_unref(GDBusClient *client)
{
	unsigned int i;

	if (client == NULL)
		return;

	if (__sync_sub_and_fetch(&client->ref_count, 1) > 0)
		return;

	if (client->pending_call != NULL) {
		dbus_pending_call_cancel(client->pending_call);
		dbus_pending_call_unref(client->pending_call);
	}

	if (client->get_objects_call != NULL) {
		dbus_pending_call_cancel(client->get_objects_call);
		dbus_pending_call_unref(client->get_objects_call);
	}

	for (i = 0; i < client->match_rules->len; i++) {
		modify_match(client->dbus_conn, "RemoveMatch",
				g_ptr_array_index(client->match_rules, i));
	}

	g_ptr_array_free(client->match_rules, TRUE);

	dbus_connection_remove_filter(client->dbus_conn,
						message_filter, client);

	g_list_free_full(client->proxy_list, proxy_free);

	/*
	 * Don't call disconn_func twice if disconnection
	 * was previously reported.
	 */
	if (client->disconn_func && client->connected)
		client->disconn_func(client->dbus_conn, client->disconn_data);

	g_dbus_remove_watch(client->dbus_conn, client->watch);
	g_dbus_remove_watch(client->dbus_conn, client->added_watch);
	g_dbus_remove_watch(client->dbus_conn, client->removed_watch);

	dbus_connection_unref(client->dbus_conn);

	g_free(client->service_name);
	g_free(client->base_path);
	g_free(client->root_path);

	g_free(client);
}

gboolean g_dbus_client_set_connect_watch(GDBusClient *client,
				GDBusWatchFunction function, void *user_data)
{
	if (client == NULL)
		return FALSE;

	client->connect_func = function;
	client->connect_data = user_data;

	return TRUE;
}

gboolean g_dbus_client_set_disconnect_watch(GDBusClient *client,
				GDBusWatchFunction function, void *user_data)
{
	if (client == NULL)
		return FALSE;

	client->disconn_func = function;
	client->disconn_data = user_data;

	return TRUE;
}

gboolean g_dbus_client_set_signal_watch(GDBusClient *client,
				GDBusMessageFunction function, void *user_data)
{
	if (client == NULL)
		return FALSE;

	client->signal_func = function;
	client->signal_data = user_data;

	return TRUE;
}

gboolean g_dbus_client_set_ready_watch(GDBusClient *client,
				GDBusClientFunction ready, void *user_data)
{
	if (client == NULL)
		return FALSE;

	client->ready = ready;
	client->ready_data = user_data;

	return TRUE;
}

gboolean g_dbus_client_set_proxy_handlers(GDBusClient *client,
					GDBusProxyFunction proxy_added,
					GDBusProxyFunction proxy_removed,
					GDBusPropertyFunction property_changed,
					void *user_data)
{
	if (client == NULL)
		return FALSE;

	client->proxy_added = proxy_added;
	client->proxy_removed = proxy_removed;
	client->property_changed = property_changed;
	client->user_data = user_data;

	if (proxy_added || proxy_removed || property_changed)
		get_managed_objects(client);

	return TRUE;
}
