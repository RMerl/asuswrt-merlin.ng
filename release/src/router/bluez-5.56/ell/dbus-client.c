/*
 *
 *  Embedded Linux library
 *
 *  Copyright (C) 2011-2014  Intel Corporation. All rights reserved.
 *  Copyright (C) 2017  Codecoup. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "dbus.h"
#include "dbus-client.h"
#include "queue.h"
#include "private.h"

struct l_dbus_client {
	struct l_dbus *dbus;
	unsigned int watch;
	unsigned int added_watch;
	unsigned int removed_watch;
	char *service;
	uint32_t objects_call;

	l_dbus_watch_func_t connect_cb;
	void *connect_cb_data;
	l_dbus_destroy_func_t connect_cb_data_destroy;

	l_dbus_watch_func_t disconnect_cb;
	void *disconnect_cb_data;
	l_dbus_destroy_func_t disconnect_cb_data_destroy;

	l_dbus_client_ready_func_t ready_cb;
	void *ready_cb_data;
	l_dbus_destroy_func_t ready_cb_data_destroy;

	l_dbus_client_proxy_func_t proxy_added_cb;
	l_dbus_client_proxy_func_t proxy_removed_cb;
	l_dbus_client_property_function_t properties_changed_cb;
	void *proxy_cb_data;
	l_dbus_destroy_func_t proxy_cb_data_destroy;

	struct l_queue *proxies;
};

struct proxy_property {
	char *name;
	struct l_dbus_message *msg;
};

struct l_dbus_proxy {
	struct l_dbus_client *client;
	char *interface;
	char *path;
	uint32_t properties_watch;
	bool ready;

	struct l_queue *properties;
	struct l_queue *pending_calls;
};

LIB_EXPORT const char *l_dbus_proxy_get_path(struct l_dbus_proxy *proxy)
{
	if (unlikely(!proxy))
		return NULL;

	return proxy->path;
}

LIB_EXPORT const char *l_dbus_proxy_get_interface(struct l_dbus_proxy *proxy)
{
	if (unlikely(!proxy))
		return NULL;

	return proxy->interface;
}

static bool property_match_by_name(const void *a, const void *b)
{
	const struct proxy_property *prop = a;
	const char *name = b;

	return !strcmp(prop->name, name);
}

static struct proxy_property *find_property(struct l_dbus_proxy *proxy,
							const char *name)
{
	return l_queue_find(proxy->properties, property_match_by_name, name);
}

static struct proxy_property *get_property(struct l_dbus_proxy *proxy,
							const char *name)
{
	struct proxy_property *prop;

	prop = find_property(proxy, name);
	if (prop)
		return prop;

	prop = l_new(struct proxy_property, 1);
	prop->name = l_strdup(name);

	l_queue_push_tail(proxy->properties, prop);

	return prop;
}

LIB_EXPORT bool l_dbus_proxy_get_property(struct l_dbus_proxy *proxy,
						const char *name,
						const char *signature, ...)
{
	struct proxy_property *prop;
	va_list args;
	bool res;

	if (unlikely(!proxy))
		return false;

	prop = find_property(proxy, name);
	if (!prop)
		return false;

	va_start(args, signature);
	res = l_dbus_message_get_arguments_valist(prop->msg, signature, args);
	va_end(args);

	return res;
}

static void property_free(void *data)
{
	struct proxy_property *prop = data;

	if (prop->msg)
		l_dbus_message_unref(prop->msg);

	l_free(prop->name);
	l_free(prop);
}

static void cancel_pending_calls(struct l_dbus_proxy *proxy)
{
	const struct l_queue_entry *entry;

	for (entry = l_queue_get_entries(proxy->pending_calls); entry;
							entry = entry->next) {
		uint32_t call_id = L_PTR_TO_UINT(entry->data);

		l_dbus_cancel(proxy->client->dbus, call_id);
	}
}

static void dbus_proxy_destroy(struct l_dbus_proxy *proxy)
{
	if (unlikely(!proxy))
		return;

	if (proxy->properties_watch)
		l_dbus_remove_signal_watch(proxy->client->dbus,
						proxy->properties_watch);

	cancel_pending_calls(proxy);
	l_queue_destroy(proxy->pending_calls, NULL);
	l_queue_destroy(proxy->properties, property_free);
	l_free(proxy->interface);
	l_free(proxy->path);
	l_free(proxy);
}

struct method_call_request
{
	struct l_dbus_proxy *proxy;
	uint32_t call_id;
	l_dbus_message_func_t setup;
	l_dbus_client_proxy_result_func_t result;
	void *user_data;
	l_dbus_destroy_func_t destroy;
};

static void method_call_request_free(void *user_data)
{
	struct method_call_request *req = user_data;

	l_queue_remove(req->proxy->pending_calls, L_UINT_TO_PTR(req->call_id));

	if (req->destroy)
		req->destroy(req->user_data);

	l_free(req);
}

static void method_call_setup(struct l_dbus_message *message, void *user_data)
{
	struct method_call_request *req = user_data;

	if (req->setup)
		req->setup(message, req->user_data);
	else
		l_dbus_message_set_arguments(message, "");
}

static void method_call_reply(struct l_dbus_message *message, void *user_data)
{
	struct method_call_request *req = user_data;

	if (req->result)
		req->result(req->proxy, message, req->user_data);
}

LIB_EXPORT bool l_dbus_proxy_set_property(struct l_dbus_proxy *proxy,
				l_dbus_client_proxy_result_func_t result,
				void *user_data, l_dbus_destroy_func_t destroy,
				const char *name, const char *signature, ...)
{
	struct l_dbus_client *client = proxy->client;
	struct l_dbus_message_builder *builder;
	struct method_call_request *req;
	struct l_dbus_message *message;
	struct proxy_property *prop;
	va_list args;

	if (unlikely(!proxy))
		return false;

	prop = find_property(proxy, name);
	if (!prop)
		return false;

	if (strcmp(l_dbus_message_get_signature(prop->msg), signature))
		return false;

	message = l_dbus_message_new_method_call(client->dbus, client->service,
						proxy->path,
						L_DBUS_INTERFACE_PROPERTIES,
						"Set");
	if (!message)
		return false;

	builder = l_dbus_message_builder_new(message);
	if (!builder) {
		l_dbus_message_unref(message);
		return false;
	}

	l_dbus_message_builder_append_basic(builder, 's', proxy->interface);
	l_dbus_message_builder_append_basic(builder, 's', name);

	l_dbus_message_builder_enter_variant(builder, signature);

	va_start(args, signature);
	l_dbus_message_builder_append_from_valist(builder, signature, args);
	va_end(args);

	l_dbus_message_builder_leave_variant(builder);

	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	req = l_new(struct method_call_request, 1);
	req->proxy = proxy;
	req->result = result;
	req->user_data = user_data;
	req->destroy = destroy;

	req->call_id = l_dbus_send_with_reply(client->dbus, message,
						method_call_reply, req,
						method_call_request_free);
	if (!req->call_id) {
		l_free(req);
		return false;
	}

	l_queue_push_tail(proxy->pending_calls, L_UINT_TO_PTR(req->call_id));

	return true;
}

LIB_EXPORT uint32_t l_dbus_proxy_method_call(struct l_dbus_proxy *proxy,
					const char *method,
					l_dbus_message_func_t setup,
					l_dbus_client_proxy_result_func_t reply,
					void *user_data,
					l_dbus_destroy_func_t destroy)
{
	struct method_call_request *req;

	if (unlikely(!proxy))
		return 0;

	req = l_new(struct method_call_request, 1);
	req->proxy = proxy;
	req->setup = setup;
	req->result = reply;
	req->user_data = user_data;
	req->destroy = destroy;

	req->call_id = l_dbus_method_call(proxy->client->dbus,
						proxy->client->service,
						proxy->path, proxy->interface,
						method, method_call_setup,
						method_call_reply, req,
						method_call_request_free);
	if (!req->call_id) {
		l_free(req);
		return 0;
	}

	l_queue_push_tail(proxy->pending_calls, L_UINT_TO_PTR(req->call_id));

	return req->call_id;
}

static void proxy_update_property(struct l_dbus_proxy *proxy,
					const char *name,
					struct l_dbus_message_iter *property)
{
	struct l_dbus_message_builder *builder;
	struct proxy_property *prop = get_property(proxy, name);

	l_dbus_message_unref(prop->msg);

	if (!property) {
		prop->msg = NULL;
		goto done;
	}

	prop->msg = l_dbus_message_new_signal(proxy->client->dbus, proxy->path,
							proxy->interface, name);
	if (!prop->msg)
		return;

	builder = l_dbus_message_builder_new(prop->msg);
	l_dbus_message_builder_append_from_iter(builder, property);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

done:
	if (proxy->client->properties_changed_cb && proxy->ready)
		proxy->client->properties_changed_cb(proxy, name, prop->msg,
					proxy->client->proxy_cb_data);
}

static void proxy_invalidate_properties(struct l_dbus_proxy *proxy,
					struct l_dbus_message_iter* props)
{
	const char *name;

	while (l_dbus_message_iter_next_entry(props, &name))
		proxy_update_property(proxy, name, NULL);
}

static void proxy_update_properties(struct l_dbus_proxy *proxy,
					struct l_dbus_message_iter* props)
{
	struct l_dbus_message_iter variant;
	const char *name;

	while (l_dbus_message_iter_next_entry(props, &name, &variant))
		proxy_update_property(proxy, name, &variant);
}

static void properties_changed_callback(struct l_dbus_message *message,
								void *user_data)
{
	struct l_dbus_proxy *proxy = user_data;
	const char *interface;
	struct l_dbus_message_iter changed;
	struct l_dbus_message_iter invalidated;

	if (!l_dbus_message_get_arguments(message, "sa{sv}as", &interface,
							&changed, &invalidated))
		return;

	proxy_update_properties(proxy, &changed);
	proxy_invalidate_properties(proxy, &invalidated);
}

static struct l_dbus_proxy *dbus_proxy_new(struct l_dbus_client *client,
					const char *path, const char *interface)
{
	struct l_dbus_proxy *proxy = l_new(struct l_dbus_proxy, 1);

	proxy->properties_watch = l_dbus_add_signal_watch(client->dbus,
						client->service, path,
						L_DBUS_INTERFACE_PROPERTIES,
						"PropertiesChanged",
						L_DBUS_MATCH_ARGUMENT(0),
						interface, L_DBUS_MATCH_NONE,
						properties_changed_callback,
						proxy);
	if (!proxy->properties_watch) {
		l_free(proxy);
		return NULL;
	}

	proxy->client = client;
	proxy->interface = l_strdup(interface);
	proxy->path = l_strdup(path);
	proxy->properties = l_queue_new();
	proxy->pending_calls = l_queue_new();

	l_queue_push_tail(client->proxies, proxy);

	return proxy;
}

static bool is_ignorable(const char *interface)
{
	static const struct {
		const char *interface;
	} interfaces_to_ignore[] = {
		{ L_DBUS_INTERFACE_OBJECT_MANAGER },
		{ L_DBUS_INTERFACE_INTROSPECTABLE },
		{ L_DBUS_INTERFACE_PROPERTIES },
	};
	size_t i;

	for (i = 0; i < L_ARRAY_SIZE(interfaces_to_ignore); i++)
		if (!strcmp(interfaces_to_ignore[i].interface, interface))
			return true;

	return false;
}

static struct l_dbus_proxy *find_proxy(struct l_dbus_client *client,
					const char *path, const char *interface)
{
	const struct l_queue_entry *entry;

	for (entry = l_queue_get_entries(client->proxies); entry;
							entry = entry->next) {
		struct l_dbus_proxy *proxy = entry->data;

		if (!strcmp(proxy->interface, interface) &&
						!strcmp(proxy->path, path))
			return proxy;
	}

	return NULL;
}

static void parse_interface(struct l_dbus_client *client, const char *path,
					const char *interface,
					struct l_dbus_message_iter *properties)
{
	struct l_dbus_proxy *proxy;

	if (is_ignorable(interface))
		return;

	proxy = find_proxy(client, path, interface);
	if (!proxy)
		proxy = dbus_proxy_new(client, path, interface);

	if (!proxy)
		return;

	proxy_update_properties(proxy, properties);

	if (!proxy->ready) {
		proxy->ready = true;

		if (client->proxy_added_cb)
			client->proxy_added_cb(proxy, client->proxy_cb_data);
	}
}

static void parse_object(struct l_dbus_client *client, const char *path,
					struct l_dbus_message_iter *object)
{
	const char *interface;
	struct l_dbus_message_iter properties;

	if (!path)
		return;

	while (l_dbus_message_iter_next_entry(object, &interface, &properties))
		parse_interface(client, path, interface, &properties);
}

static void interfaces_added_callback(struct l_dbus_message *message,
								void *user_data)
{
	struct l_dbus_client *client = user_data;
	struct l_dbus_message_iter object;
	const char *path;

	if (!l_dbus_message_get_arguments(message, "oa{sa{sv}}", &path,
								&object))
		return;

	parse_object(client, path, &object);
}

static void interfaces_removed_callback(struct l_dbus_message *message,
								void *user_data)
{
	struct l_dbus_client *client = user_data;
	struct l_dbus_message_iter interfaces;
	const char *interface;
	const char *path;

	if (!l_dbus_message_get_arguments(message, "oas", &path, &interfaces))
		return;

	while (l_dbus_message_iter_next_entry(&interfaces, &interface)) {
		struct l_dbus_proxy *proxy;

		proxy = find_proxy(client, path, interface);
		if (!proxy)
			continue;

		l_queue_remove(proxy->client->proxies, proxy);

		if (client->proxy_removed_cb)
			client->proxy_removed_cb(proxy, client->proxy_cb_data);

		dbus_proxy_destroy(proxy);
	}
}

static void get_managed_objects_reply(struct l_dbus_message *message,
								void *user_data)
{
	struct l_dbus_client *client = user_data;
	struct l_dbus_message_iter objects;
	struct l_dbus_message_iter object;
	const char *path;

	client->objects_call = 0;

	if (l_dbus_message_is_error(message))
		return;

	if (!l_dbus_message_get_arguments(message, "a{oa{sa{sv}}}", &objects))
		return;

	while (l_dbus_message_iter_next_entry(&objects, &path, &object))
		parse_object(client, path, &object);

	client->added_watch = l_dbus_add_signal_watch(client->dbus,
						client->service, "/",
						L_DBUS_INTERFACE_OBJECT_MANAGER,
						"InterfacesAdded",
						L_DBUS_MATCH_NONE,
						interfaces_added_callback,
						client);

	client->removed_watch = l_dbus_add_signal_watch(client->dbus,
						client->service, "/",
						L_DBUS_INTERFACE_OBJECT_MANAGER,
						"InterfacesRemoved",
						L_DBUS_MATCH_NONE,
						interfaces_removed_callback,
						client);

	if (client->ready_cb)
		client->ready_cb(client, client->ready_cb_data);
}

static void service_appeared_callback(struct l_dbus *dbus, void *user_data)
{
	struct l_dbus_client *client = user_data;

	/* TODO should we allow to set different root? */
	client->objects_call = l_dbus_method_call(dbus, client->service, "/",
						L_DBUS_INTERFACE_OBJECT_MANAGER,
						"GetManagedObjects", NULL,
						get_managed_objects_reply,
						client, NULL);

	if (client->connect_cb)
		client->connect_cb(client->dbus, client->connect_cb_data);
}

static void service_disappeared_callback(struct l_dbus *dbus, void *user_data)
{
	struct l_dbus_client *client = user_data;

	if (client->disconnect_cb)
		client->disconnect_cb(client->dbus, client->disconnect_cb_data);

	l_queue_clear(client->proxies,
				(l_queue_destroy_func_t)dbus_proxy_destroy);
}

LIB_EXPORT struct l_dbus_client *l_dbus_client_new(struct l_dbus *dbus,
					const char *service, const char *path)
{
	struct l_dbus_client *client = l_new(struct l_dbus_client, 1);

	client->dbus = dbus;

	client->watch = l_dbus_add_service_watch(dbus, service,
						service_appeared_callback,
						service_disappeared_callback,
						client, NULL);

	if (!client->watch) {
		l_free(client);
		return NULL;
	}

	client->service = l_strdup(service);
	client->proxies = l_queue_new();

	return client;
}

LIB_EXPORT void l_dbus_client_destroy(struct l_dbus_client *client)
{
	if (unlikely(!client))
		return;

	if (client->watch)
		l_dbus_remove_signal_watch(client->dbus, client->watch);

	if (client->added_watch)
		l_dbus_remove_signal_watch(client->dbus, client->added_watch);

	if (client->removed_watch)
		l_dbus_remove_signal_watch(client->dbus, client->removed_watch);

	if (client->connect_cb_data_destroy)
		client->connect_cb_data_destroy(client->connect_cb_data);

	if (client->disconnect_cb_data_destroy)
		client->disconnect_cb_data_destroy(client->disconnect_cb_data);

	if (client->ready_cb_data_destroy)
		client->ready_cb_data_destroy(client->ready_cb_data);

	if (client->proxy_cb_data_destroy)
		client->proxy_cb_data_destroy(client->proxy_cb_data);

	if (client->objects_call)
		l_dbus_cancel(client->dbus, client->objects_call);

	l_queue_destroy(client->proxies,
				(l_queue_destroy_func_t)dbus_proxy_destroy);

	l_free(client->service);
	l_free(client);
}

LIB_EXPORT bool l_dbus_client_set_connect_handler(struct l_dbus_client *client,
						l_dbus_watch_func_t function,
						void *user_data,
						l_dbus_destroy_func_t destroy)
{
	if (unlikely(!client))
		return false;

	if (client->connect_cb_data_destroy)
		client->connect_cb_data_destroy(client->connect_cb_data);

	client->connect_cb = function;
	client->connect_cb_data = user_data;
	client->connect_cb_data_destroy = destroy;

	return true;
}

LIB_EXPORT bool l_dbus_client_set_disconnect_handler(struct l_dbus_client *client,
						l_dbus_watch_func_t function,
						void *user_data,
						l_dbus_destroy_func_t destroy)
{
	if (unlikely(!client))
		return false;

	if(client->disconnect_cb_data_destroy)
		client->disconnect_cb_data_destroy(client->disconnect_cb_data);

	client->disconnect_cb = function;
	client->disconnect_cb_data = user_data;
	client->disconnect_cb_data_destroy = destroy;

	return true;
}

LIB_EXPORT bool l_dbus_client_set_ready_handler(struct l_dbus_client *client,
					l_dbus_client_ready_func_t function,
					void *user_data,
					l_dbus_destroy_func_t destroy)
{
	if (unlikely(!client))
		return false;

	if (client->ready_cb_data_destroy)
		client->ready_cb_data_destroy(client->ready_cb_data);

	client->ready_cb = function;
	client->ready_cb_data = user_data;
	client->ready_cb_data_destroy = destroy;

	return true;
}

LIB_EXPORT bool l_dbus_client_set_proxy_handlers(struct l_dbus_client *client,
			l_dbus_client_proxy_func_t proxy_added,
			l_dbus_client_proxy_func_t proxy_removed,
			l_dbus_client_property_function_t property_changed,
			void *user_data, l_dbus_destroy_func_t destroy)
{
	if (unlikely(!client))
		return false;

	if (client->proxy_cb_data_destroy)
		client->proxy_cb_data_destroy(client->proxy_cb_data);

	client->proxy_added_cb = proxy_added;
	client->proxy_removed_cb = proxy_removed;
	client->properties_changed_cb = property_changed;
	client->proxy_cb_data = user_data;
	client->proxy_cb_data_destroy = destroy;

	return true;
}
