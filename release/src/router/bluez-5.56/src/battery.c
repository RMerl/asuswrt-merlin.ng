// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2020  Google LLC
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdbool.h>
#include <stdint.h>
#include <glib.h>

#include "gdbus/gdbus.h"
#include "lib/bluetooth.h"
#include "src/shared/queue.h"
#include "src/shared/util.h"
#include "battery.h"
#include "dbus-common.h"
#include "adapter.h"
#include "device.h"
#include "log.h"
#include "error.h"

#define BATTERY_INTERFACE "org.bluez.Battery1"
#define BATTERY_PROVIDER_INTERFACE "org.bluez.BatteryProvider1"
#define BATTERY_PROVIDER_MANAGER_INTERFACE "org.bluez.BatteryProviderManager1"

#define BATTERY_MAX_PERCENTAGE 100

struct btd_battery {
	char *path; /* D-Bus object path */
	uint8_t percentage; /* valid between 0 to 100 inclusively */
	char *source; /* Descriptive source of the battery info */
	char *provider_path; /* The provider root path, if any */
};

struct btd_battery_provider_manager {
	struct btd_adapter *adapter; /* Does not own pointer */
	struct queue *battery_providers;
};

struct battery_provider {
	struct btd_battery_provider_manager *manager; /* Does not own pointer */

	char *owner; /* Owner D-Bus address */
	char *path; /* D-Bus object path */

	GDBusClient *client;
};

static struct queue *batteries = NULL;

static void provider_disconnect_cb(DBusConnection *conn, void *user_data);

static void battery_add(struct btd_battery *battery)
{
	if (!batteries)
		batteries = queue_new();

	queue_push_head(batteries, battery);
}

static void battery_remove(struct btd_battery *battery)
{
	queue_remove(batteries, battery);
	if (queue_isempty(batteries)) {
		queue_destroy(batteries, NULL);
		batteries = NULL;
	}
}

static bool match_path(const void *data, const void *user_data)
{
	const struct btd_battery *battery = data;
	const char *path = user_data;

	return g_strcmp0(battery->path, path) == 0;
}

static struct btd_battery *battery_new(const char *path, const char *source,
				       const char *provider_path)
{
	struct btd_battery *battery;

	battery = new0(struct btd_battery, 1);
	battery->path = g_strdup(path);
	battery->percentage = UINT8_MAX;
	if (source)
		battery->source = g_strdup(source);
	if (provider_path)
		battery->provider_path = g_strdup(provider_path);

	return battery;
}

static void battery_free(struct btd_battery *battery)
{
	if (battery->path)
		g_free(battery->path);

	if (battery->source)
		g_free(battery->source);

	free(battery);
}

static gboolean property_percentage_get(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_battery *battery = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BYTE,
				       &battery->percentage);

	return TRUE;
}

static gboolean property_percentage_exists(const GDBusPropertyTable *property,
					   void *data)
{
	struct btd_battery *battery = data;

	return battery->percentage <= BATTERY_MAX_PERCENTAGE;
}

static gboolean property_source_get(const GDBusPropertyTable *property,
				    DBusMessageIter *iter, void *data)
{
	struct btd_battery *battery = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING,
				       &battery->source);

	return TRUE;
}

static gboolean property_source_exists(const GDBusPropertyTable *property,
				       void *data)
{
	struct btd_battery *battery = data;

	return battery->source != NULL;
}

static const GDBusPropertyTable battery_properties[] = {
	{ "Percentage", "y", property_percentage_get, NULL,
	  property_percentage_exists },
	{ "Source", "s", property_source_get, NULL, property_source_exists,
	  G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{}
};

struct btd_battery *btd_battery_register(const char *path, const char *source,
					 const char *provider_path)
{
	struct btd_battery *battery;

	DBG("path = %s", path);

	if (queue_find(batteries, match_path, path)) {
		error("error registering battery: path exists");
		return NULL;
	}

	if (!g_str_has_prefix(path, "/")) {
		error("error registering battery: invalid D-Bus object path");
		return NULL;
	}

	battery = battery_new(path, source, provider_path);
	battery_add(battery);

	if (!g_dbus_register_interface(btd_get_dbus_connection(), battery->path,
				       BATTERY_INTERFACE, NULL, NULL,
				       battery_properties, battery, NULL)) {
		error("error registering D-Bus interface for %s",
		      battery->path);

		battery_remove(battery);
		battery_free(battery);

		return NULL;
	}

	DBG("registered Battery object: %s", battery->path);

	return battery;
}

bool btd_battery_unregister(struct btd_battery *battery)
{
	DBG("path = %s", battery->path);

	if (!queue_find(batteries, NULL, battery)) {
		error("error unregistering battery: "
		      "battery %s is not registered",
		      battery->path);
		return false;
	}

	if (!g_dbus_unregister_interface(btd_get_dbus_connection(),
					 battery->path, BATTERY_INTERFACE)) {
		error("error unregistering battery %s from D-Bus interface",
		      battery->path);
		return false;
	}

	battery_remove(battery);
	battery_free(battery);

	return true;
}

bool btd_battery_update(struct btd_battery *battery, uint8_t percentage)
{
	DBG("path = %s", battery->path);

	if (!queue_find(batteries, NULL, battery)) {
		error("error updating battery: battery is not registered");
		return false;
	}

	if (percentage > BATTERY_MAX_PERCENTAGE) {
		error("error updating battery: percentage is not valid");
		return false;
	}

	if (battery->percentage == percentage)
		return true;

	battery->percentage = percentage;
	g_dbus_emit_property_changed(btd_get_dbus_connection(), battery->path,
				     BATTERY_INTERFACE, "Percentage");

	return true;
}

static struct btd_battery *find_battery_by_path(const char *path)
{
	return queue_find(batteries, match_path, path);
}

static void provided_battery_property_changed_cb(GDBusProxy *proxy,
						 const char *name,
						 DBusMessageIter *iter,
						 void *user_data)
{
	uint8_t percentage;
	const char *export_path;
	DBusMessageIter dev_iter;

	if (g_dbus_proxy_get_property(proxy, "Device", &dev_iter) == FALSE)
		return;

	dbus_message_iter_get_basic(&dev_iter, &export_path);

	if (strcmp(name, "Percentage") != 0)
		return;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_BYTE)
		return;

	dbus_message_iter_get_basic(iter, &percentage);

	DBG("battery percentage changed on %s, percentage = %d",
	    g_dbus_proxy_get_path(proxy), percentage);

	btd_battery_update(find_battery_by_path(export_path), percentage);
}

static void provided_battery_added_cb(GDBusProxy *proxy, void *user_data)
{
	struct battery_provider *provider = user_data;
	struct btd_battery *battery;
	struct btd_device *device;
	const char *path = g_dbus_proxy_get_path(proxy);
	const char *export_path;
	const char *source = NULL;
	uint8_t percentage;
	DBusMessageIter iter;

	if (g_dbus_proxy_get_property(proxy, "Device", &iter) == FALSE) {
		warn("Battery object %s does not specify device path", path);
		return;
	}

	dbus_message_iter_get_basic(&iter, &export_path);

	if (strcmp(g_dbus_proxy_get_interface(proxy),
		   BATTERY_PROVIDER_INTERFACE) != 0)
		return;

	device = btd_adapter_find_device_by_path(provider->manager->adapter,
						 export_path);
	if (!device || device_is_temporary(device)) {
		warn("Ignoring non-existent device path for battery %s",
		     export_path);
		return;
	}

	if (find_battery_by_path(export_path)) {
		DBG("Battery for %s is already provided, ignoring the new one",
		    export_path);
		return;
	}

	g_dbus_proxy_set_property_watch(
		proxy, provided_battery_property_changed_cb, provider);

	if (g_dbus_proxy_get_property(proxy, "Source", &iter) == TRUE)
		dbus_message_iter_get_basic(&iter, &source);

	battery = btd_battery_register(export_path, source, provider->path);

	DBG("provided battery added %s", path);

	/* Percentage property may not be immediately available, that's okay
	 * since we monitor changes to this property.
	 */
	if (g_dbus_proxy_get_property(proxy, "Percentage", &iter) == FALSE)
		return;

	dbus_message_iter_get_basic(&iter, &percentage);

	btd_battery_update(battery, percentage);
}

static void provided_battery_removed_cb(GDBusProxy *proxy, void *user_data)
{
	struct battery_provider *provider = user_data;
	struct btd_battery *battery;
	const char *export_path;
	DBusMessageIter iter;

	if (g_dbus_proxy_get_property(proxy, "Device", &iter) == FALSE)
		return;

	dbus_message_iter_get_basic(&iter, &export_path);

	if (strcmp(g_dbus_proxy_get_interface(proxy),
		   BATTERY_PROVIDER_INTERFACE) != 0)
		return;

	DBG("provided battery removed %s", g_dbus_proxy_get_path(proxy));

	battery = find_battery_by_path(export_path);
	if (!battery)
		return;

	if (g_strcmp0(battery->provider_path, provider->path) != 0)
		return;

	g_dbus_proxy_set_property_watch(proxy, NULL, NULL);

	btd_battery_unregister(battery);
}

static bool match_provider_path(const void *data, const void *user_data)
{
	const struct battery_provider *provider = data;
	const char *path = user_data;

	return strcmp(provider->path, path) == 0;
}

static void unregister_if_path_has_prefix(void *data, void *user_data)
{
	struct btd_battery *battery = data;
	struct battery_provider *provider = user_data;

	if (g_strcmp0(battery->provider_path, provider->path) == 0)
		btd_battery_unregister(battery);
}

static void battery_provider_free(gpointer data)
{
	struct battery_provider *provider = data;

	/* Unregister batteries under the root path of provider->path */
	queue_foreach(batteries, unregister_if_path_has_prefix, provider);

	if (provider->owner)
		g_free(provider->owner);

	if (provider->path)
		g_free(provider->path);

	if (provider->client) {
		g_dbus_client_set_disconnect_watch(provider->client, NULL,
						   NULL);
		g_dbus_client_set_proxy_handlers(provider->client, NULL, NULL,
						 NULL, NULL);
		g_dbus_client_unref(provider->client);
	}

	free(provider);
}

static struct battery_provider *
battery_provider_new(DBusConnection *conn,
		     struct btd_battery_provider_manager *manager,
		     const char *path, const char *sender)
{
	struct battery_provider *provider;

	provider = new0(struct battery_provider, 1);
	provider->manager = manager;
	provider->owner = g_strdup(sender);
	provider->path = g_strdup(path);

	provider->client = g_dbus_client_new_full(conn, sender, path, path);

	if (!provider->client) {
		error("error creating D-Bus client %s", path);
		battery_provider_free(provider);
		return NULL;
	}

	g_dbus_client_set_disconnect_watch(provider->client,
					   provider_disconnect_cb, provider);

	g_dbus_client_set_proxy_handlers(provider->client,
					 provided_battery_added_cb,
					 provided_battery_removed_cb, NULL,
					 provider);

	return provider;
}

static void provider_disconnect_cb(DBusConnection *conn, void *user_data)
{
	struct battery_provider *provider = user_data;
	struct btd_battery_provider_manager *manager = provider->manager;

	DBG("battery provider client disconnected %s root path %s",
	    provider->owner, provider->path);

	if (!queue_find(manager->battery_providers, NULL, provider)) {
		warn("Disconnection on a non-existing provider %s",
		     provider->path);
		return;
	}

	queue_remove(manager->battery_providers, provider);
	battery_provider_free(provider);
}

static DBusMessage *register_battery_provider(DBusConnection *conn,
					      DBusMessage *msg, void *user_data)
{
	struct btd_battery_provider_manager *manager = user_data;
	const char *sender = dbus_message_get_sender(msg);
	DBusMessageIter args;
	const char *path;
	struct battery_provider *provider;

	if (!dbus_message_iter_init(msg, &args))
		return btd_error_invalid_args(msg);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_OBJECT_PATH)
		return btd_error_invalid_args(msg);

	dbus_message_iter_get_basic(&args, &path);

	DBG("register battery provider path = %s", path);

	if (!g_str_has_prefix(path, "/"))
		return btd_error_invalid_args(msg);

	if (queue_find(manager->battery_providers, match_provider_path, path)) {
		return dbus_message_new_error(msg,
					      ERROR_INTERFACE ".AlreadyExists",
					      "Provider already exists");
	}

	provider = battery_provider_new(conn, manager, path, sender);
	queue_push_head(manager->battery_providers, provider);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *unregister_battery_provider(DBusConnection *conn,
						DBusMessage *msg,
						void *user_data)
{
	struct btd_battery_provider_manager *manager = user_data;
	const char *sender = dbus_message_get_sender(msg);
	DBusMessageIter args;
	const char *path;
	struct battery_provider *provider;

	if (!dbus_message_iter_init(msg, &args))
		return btd_error_invalid_args(msg);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_OBJECT_PATH)
		return btd_error_invalid_args(msg);

	dbus_message_iter_get_basic(&args, &path);

	DBG("unregister battery provider path = %s", path);

	provider = queue_find(manager->battery_providers, match_provider_path,
			      path);
	if (!provider || strcmp(provider->owner, sender) != 0) {
		return dbus_message_new_error(msg,
					      ERROR_INTERFACE ".DoesNotExist",
					      "Provider does not exist");
	}

	queue_remove(manager->battery_providers, provider);
	battery_provider_free(provider);

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable methods[] = {
	{ GDBUS_EXPERIMENTAL_METHOD("RegisterBatteryProvider",
				    GDBUS_ARGS({ "provider", "o" }), NULL,
				    register_battery_provider) },
	{ GDBUS_EXPERIMENTAL_METHOD("UnregisterBatteryProvider",
				    GDBUS_ARGS({ "provider", "o" }), NULL,
				    unregister_battery_provider) },
	{}
};

static struct btd_battery_provider_manager *
manager_new(struct btd_adapter *adapter)
{
	struct btd_battery_provider_manager *manager;

	DBG("");

	manager = new0(struct btd_battery_provider_manager, 1);
	manager->adapter = adapter;
	manager->battery_providers = queue_new();

	return manager;
}

static void manager_free(struct btd_battery_provider_manager *manager)
{
	if (!manager)
		return;

	DBG("");

	queue_destroy(manager->battery_providers, battery_provider_free);

	free(manager);
}

struct btd_battery_provider_manager *
btd_battery_provider_manager_create(struct btd_adapter *adapter)
{
	struct btd_battery_provider_manager *manager;

	if (!adapter)
		return NULL;

	manager = manager_new(adapter);
	if (!manager)
		return NULL;

	if (!g_dbus_register_interface(btd_get_dbus_connection(),
				       adapter_get_path(manager->adapter),
				       BATTERY_PROVIDER_MANAGER_INTERFACE,
				       methods, NULL, NULL, manager, NULL)) {
		error("error registering " BATTERY_PROVIDER_MANAGER_INTERFACE
		      " interface");
		manager_free(manager);
		return NULL;
	}

	info("Battery Provider Manager created");

	return manager;
}

void btd_battery_provider_manager_destroy(
	struct btd_battery_provider_manager *manager)
{
	if (!manager)
		return;

	g_dbus_unregister_interface(btd_get_dbus_connection(),
				    adapter_get_path(manager->adapter),
				    BATTERY_PROVIDER_MANAGER_INTERFACE);

	info("Battery Provider Manager destroyed");

	manager_free(manager);
}
