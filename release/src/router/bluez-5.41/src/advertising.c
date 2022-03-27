/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Google Inc.
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
 */

#include "advertising.h"

#include <stdint.h>
#include <stdbool.h>

#include <dbus/dbus.h>
#include <gdbus/gdbus.h>

#include "lib/bluetooth.h"
#include "lib/mgmt.h"
#include "lib/sdp.h"

#include "adapter.h"
#include "dbus-common.h"
#include "error.h"
#include "log.h"
#include "src/shared/ad.h"
#include "src/shared/mgmt.h"
#include "src/shared/queue.h"
#include "src/shared/util.h"

#define LE_ADVERTISING_MGR_IFACE "org.bluez.LEAdvertisingManager1"
#define LE_ADVERTISEMENT_IFACE "org.bluez.LEAdvertisement1"

struct btd_advertising {
	struct btd_adapter *adapter;
	struct queue *ads;
	struct mgmt *mgmt;
	uint16_t mgmt_index;
	uint8_t max_adv_len;
	uint8_t max_ads;
	unsigned int instance_bitmap;
};

#define AD_TYPE_BROADCAST 0
#define AD_TYPE_PERIPHERAL 1

struct advertisement {
	struct btd_advertising *manager;
	char *owner;
	char *path;
	GDBusClient *client;
	GDBusProxy *proxy;
	DBusMessage *reg;
	uint8_t type; /* Advertising type */
	bool include_tx_power;
	struct bt_ad *data;
	uint8_t instance;
};

struct dbus_obj_match {
	const char *owner;
	const char *path;
};

static bool match_advertisement(const void *a, const void *b)
{
	const struct advertisement *ad = a;
	const struct dbus_obj_match *match = b;

	if (match->owner && g_strcmp0(ad->owner, match->owner))
		return false;

	if (match->path && g_strcmp0(ad->path, match->path))
		return false;

	return true;
}

static void advertisement_free(void *data)
{
	struct advertisement *ad = data;

	if (ad->client) {
		g_dbus_client_set_disconnect_watch(ad->client, NULL, NULL);
		g_dbus_client_unref(ad->client);
	}

	bt_ad_unref(ad->data);

	g_dbus_proxy_unref(ad->proxy);

	if (ad->owner)
		g_free(ad->owner);

	if (ad->path)
		g_free(ad->path);

	free(ad);
}

static gboolean advertisement_free_idle_cb(void *data)
{
	advertisement_free(data);

	return FALSE;
}

static void advertisement_release(void *data)
{
	struct advertisement *ad = data;
	DBusMessage *message;

	DBG("Releasing advertisement %s, %s", ad->owner, ad->path);

	message = dbus_message_new_method_call(ad->owner, ad->path,
							LE_ADVERTISEMENT_IFACE,
							"Release");

	if (!message) {
		error("Couldn't allocate D-Bus message");
		return;
	}

	g_dbus_send_message(btd_get_dbus_connection(), message);
}

static void advertisement_destroy(void *data)
{
	advertisement_release(data);
	advertisement_free(data);
}

static void advertisement_remove(void *data)
{
	struct advertisement *ad = data;
	struct mgmt_cp_remove_advertising cp;

	g_dbus_client_set_disconnect_watch(ad->client, NULL, NULL);

	cp.instance = ad->instance;

	mgmt_send(ad->manager->mgmt, MGMT_OP_REMOVE_ADVERTISING,
			ad->manager->mgmt_index, sizeof(cp), &cp, NULL, NULL,
			NULL);

	queue_remove(ad->manager->ads, ad);

	util_clear_uid(&ad->manager->instance_bitmap, ad->instance);

	g_idle_add(advertisement_free_idle_cb, ad);
}

static void client_disconnect_cb(DBusConnection *conn, void *user_data)
{
	DBG("Client disconnected");

	advertisement_remove(user_data);
}

static bool parse_advertising_type(GDBusProxy *proxy, uint8_t *type)
{
	DBusMessageIter iter;
	const char *msg_type;

	if (!g_dbus_proxy_get_property(proxy, "Type", &iter))
		return false;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING)
		return false;

	dbus_message_iter_get_basic(&iter, &msg_type);

	if (!g_strcmp0(msg_type, "broadcast")) {
		*type = AD_TYPE_BROADCAST;
		return true;
	}

	if (!g_strcmp0(msg_type, "peripheral")) {
		*type = AD_TYPE_PERIPHERAL;
		return true;
	}

	return false;
}

static bool parse_advertising_service_uuids(GDBusProxy *proxy,
					struct bt_ad *data)
{
	DBusMessageIter iter, ariter;

	if (!g_dbus_proxy_get_property(proxy, "ServiceUUIDs", &iter))
		return true;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return false;

	dbus_message_iter_recurse(&iter, &ariter);

	bt_ad_clear_service_uuid(data);

	while (dbus_message_iter_get_arg_type(&ariter) == DBUS_TYPE_STRING) {
		const char *uuid_str;
		bt_uuid_t uuid;

		dbus_message_iter_get_basic(&ariter, &uuid_str);

		DBG("Adding ServiceUUID: %s", uuid_str);

		if (bt_string_to_uuid(&uuid, uuid_str) < 0)
			goto fail;

		if (!bt_ad_add_service_uuid(data, &uuid))
			goto fail;

		dbus_message_iter_next(&ariter);
	}

	return true;

fail:
	bt_ad_clear_service_uuid(data);
	return false;
}

static bool parse_advertising_solicit_uuids(GDBusProxy *proxy,
							struct bt_ad *data)
{
	DBusMessageIter iter, ariter;

	if (!g_dbus_proxy_get_property(proxy, "SolicitUUIDs", &iter))
		return true;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return false;

	dbus_message_iter_recurse(&iter, &ariter);

	bt_ad_clear_solicit_uuid(data);

	while (dbus_message_iter_get_arg_type(&ariter) == DBUS_TYPE_STRING) {
		const char *uuid_str;
		bt_uuid_t uuid;

		dbus_message_iter_get_basic(&ariter, &uuid_str);

		DBG("Adding SolicitUUID: %s", uuid_str);

		if (bt_string_to_uuid(&uuid, uuid_str) < 0)
			goto fail;

		if (!bt_ad_add_solicit_uuid(data, &uuid))
			goto fail;

		dbus_message_iter_next(&ariter);
	}

	return true;

fail:
	bt_ad_clear_solicit_uuid(data);
	return false;
}

static bool parse_advertising_manufacturer_data(GDBusProxy *proxy,
							struct bt_ad *data)
{
	DBusMessageIter iter, entries;

	if (!g_dbus_proxy_get_property(proxy, "ManufacturerData", &iter))
		return true;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return false;

	dbus_message_iter_recurse(&iter, &entries);

	bt_ad_clear_manufacturer_data(data);

	while (dbus_message_iter_get_arg_type(&entries)
						== DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter value, entry;
		uint16_t manuf_id;
		uint8_t *manuf_data;
		int len;

		dbus_message_iter_recurse(&entries, &entry);
		dbus_message_iter_get_basic(&entry, &manuf_id);

		dbus_message_iter_next(&entry);
		if (dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_ARRAY)
			goto fail;

		dbus_message_iter_recurse(&entry, &value);

		if (dbus_message_iter_get_arg_type(&value) != DBUS_TYPE_BYTE)
			goto fail;

		dbus_message_iter_get_fixed_array(&value, &manuf_data, &len);

		DBG("Adding ManufacturerData for %04x", manuf_id);

		if (!bt_ad_add_manufacturer_data(data, manuf_id, manuf_data,
									len))
			goto fail;

		dbus_message_iter_next(&entries);
	}

	return true;

fail:
	bt_ad_clear_manufacturer_data(data);
	return false;
}

static bool parse_advertising_service_data(GDBusProxy *proxy,
							struct bt_ad *data)
{
	DBusMessageIter iter, entries;

	if (!g_dbus_proxy_get_property(proxy, "ServiceData", &iter))
		return true;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return false;

	dbus_message_iter_recurse(&iter, &entries);

	bt_ad_clear_service_data(data);

	while (dbus_message_iter_get_arg_type(&entries)
						== DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter value, entry;
		const char *uuid_str;
		bt_uuid_t uuid;
		uint8_t *service_data;
		int len;

		dbus_message_iter_recurse(&entries, &entry);
		dbus_message_iter_get_basic(&entry, &uuid_str);

		if (bt_string_to_uuid(&uuid, uuid_str) < 0)
			goto fail;

		dbus_message_iter_next(&entry);
		if (dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_ARRAY)
			goto fail;

		dbus_message_iter_recurse(&entry, &value);

		if (dbus_message_iter_get_arg_type(&value) != DBUS_TYPE_BYTE)
			goto fail;

		dbus_message_iter_get_fixed_array(&value, &service_data, &len);

		DBG("Adding ServiceData for %s", uuid_str);

		if (!bt_ad_add_service_data(data, &uuid, service_data, len))
			goto fail;

		dbus_message_iter_next(&entries);
	}

	return true;

fail:
	bt_ad_clear_service_data(data);
	return false;
}

static bool parse_advertising_include_tx_power(GDBusProxy *proxy,
							bool *included)
{
	DBusMessageIter iter;
	dbus_bool_t b;

	if (!g_dbus_proxy_get_property(proxy, "IncludeTxPower", &iter))
		return true;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_BOOLEAN)
		return false;

	dbus_message_iter_get_basic(&iter, &b);

	*included = b;

	return true;
}

static void add_adverting_complete(struct advertisement *ad, uint8_t status)
{
	DBusMessage *reply;

	if (status) {
		error("Failed to add advertisement: %s (0x%02x)",
						mgmt_errstr(status), status);
		reply = btd_error_failed(ad->reg,
					"Failed to register advertisement");
		queue_remove(ad->manager->ads, ad);
		g_idle_add(advertisement_free_idle_cb, ad);

	} else
		reply = dbus_message_new_method_return(ad->reg);

	g_dbus_send_message(btd_get_dbus_connection(), reply);
	dbus_message_unref(ad->reg);
	ad->reg = NULL;
}

static void add_advertising_callback(uint8_t status, uint16_t length,
					  const void *param, void *user_data)
{
	struct advertisement *ad = user_data;
	const struct mgmt_rp_add_advertising *rp = param;

	if (status)
		goto done;

	if (!param || length < sizeof(*rp)) {
		status = MGMT_STATUS_FAILED;
		goto done;
	}

	ad->instance = rp->instance;

	g_dbus_client_set_disconnect_watch(ad->client, client_disconnect_cb,
									ad);
	DBG("Advertisement registered: %s", ad->path);

done:
	add_adverting_complete(ad, status);
}

static size_t calc_max_adv_len(struct advertisement *ad, uint32_t flags)
{
	size_t max = ad->manager->max_adv_len;

	/*
	 * Flags which reduce the amount of space available for advertising.
	 * See doc/mgmt-api.txt
	 */
	if (flags & MGMT_ADV_FLAG_TX_POWER)
		max -= 3;

	if (flags & (MGMT_ADV_FLAG_DISCOV | MGMT_ADV_FLAG_LIMITED_DISCOV |
						MGMT_ADV_FLAG_MANAGED_FLAGS))
		max -= 3;

	if (flags & MGMT_ADV_FLAG_APPEARANCE)
		max -= 4;

	return max;
}

static DBusMessage *refresh_advertisement(struct advertisement *ad)
{
	struct mgmt_cp_add_advertising *cp;
	uint8_t param_len;
	uint8_t *adv_data;
	size_t adv_data_len;
	uint32_t flags = 0;

	DBG("Refreshing advertisement: %s", ad->path);

	if (ad->type == AD_TYPE_PERIPHERAL)
		flags = MGMT_ADV_FLAG_CONNECTABLE | MGMT_ADV_FLAG_DISCOV;

	if (ad->include_tx_power)
		flags |= MGMT_ADV_FLAG_TX_POWER;

	adv_data = bt_ad_generate(ad->data, &adv_data_len);

	if (!adv_data || (adv_data_len > calc_max_adv_len(ad, flags))) {
		error("Advertising data too long or couldn't be generated.");

		return g_dbus_create_error(ad->reg, ERROR_INTERFACE
						".InvalidLength",
						"Advertising data too long.");
	}

	param_len = sizeof(struct mgmt_cp_add_advertising) + adv_data_len;

	cp = malloc0(param_len);

	if (!cp) {
		error("Couldn't allocate for MGMT!");

		free(adv_data);

		return btd_error_failed(ad->reg, "Failed");
	}

	cp->flags = htobl(flags);
	cp->instance = ad->instance;
	cp->adv_data_len = adv_data_len;
	memcpy(cp->data, adv_data, adv_data_len);

	free(adv_data);

	if (!mgmt_send(ad->manager->mgmt, MGMT_OP_ADD_ADVERTISING,
					ad->manager->mgmt_index, param_len, cp,
					add_advertising_callback, ad, NULL)) {
		error("Failed to add Advertising Data");

		free(cp);

		return btd_error_failed(ad->reg, "Failed");
	}

	free(cp);

	return NULL;
}

static DBusMessage *parse_advertisement(struct advertisement *ad)
{
	if (!parse_advertising_type(ad->proxy, &ad->type)) {
		error("Failed to read \"Type\" property of advertisement");
		goto fail;
	}

	if (!parse_advertising_service_uuids(ad->proxy, ad->data)) {
		error("Property \"ServiceUUIDs\" failed to parse");
		goto fail;
	}

	if (!parse_advertising_solicit_uuids(ad->proxy, ad->data)) {
		error("Property \"SolicitUUIDs\" failed to parse");
		goto fail;
	}

	if (!parse_advertising_manufacturer_data(ad->proxy, ad->data)) {
		error("Property \"ManufacturerData\" failed to parse");
		goto fail;
	}

	if (!parse_advertising_service_data(ad->proxy, ad->data)) {
		error("Property \"ServiceData\" failed to parse");
		goto fail;
	}

	if (!parse_advertising_include_tx_power(ad->proxy,
						&ad->include_tx_power)) {
		error("Property \"IncludeTxPower\" failed to parse");
		goto fail;
	}

	return refresh_advertisement(ad);

fail:
	return btd_error_failed(ad->reg, "Failed to parse advertisement.");
}

static void advertisement_proxy_added(GDBusProxy *proxy, void *data)
{
	struct advertisement *ad = data;
	DBusMessage *reply;

	reply = parse_advertisement(ad);
	if (!reply)
		return;

	/* Failed to publish for some reason, remove. */
	queue_remove(ad->manager->ads, ad);

	g_idle_add(advertisement_free_idle_cb, ad);

	g_dbus_send_message(btd_get_dbus_connection(), reply);

	dbus_message_unref(ad->reg);
	ad->reg = NULL;
}

static struct advertisement *advertisement_create(DBusConnection *conn,
					DBusMessage *msg, const char *path)
{
	struct advertisement *ad;
	const char *sender = dbus_message_get_sender(msg);

	if (!path || !g_str_has_prefix(path, "/"))
		return NULL;

	ad = new0(struct advertisement, 1);
	ad->client = g_dbus_client_new_full(conn, sender, path, path);
	if (!ad->client)
		goto fail;

	ad->owner = g_strdup(sender);
	if (!ad->owner)
		goto fail;

	ad->path = g_strdup(path);
	if (!ad->path)
		goto fail;

	DBG("Adding proxy for %s", path);
	ad->proxy = g_dbus_proxy_new(ad->client, path, LE_ADVERTISEMENT_IFACE);
	if (!ad->proxy)
		goto fail;

	g_dbus_client_set_proxy_handlers(ad->client, advertisement_proxy_added,
								NULL, NULL, ad);

	ad->reg = dbus_message_ref(msg);

	ad->data = bt_ad_new();
	if (!ad->data)
		goto fail;

	return ad;

fail:
	advertisement_free(ad);
	return NULL;
}

static DBusMessage *register_advertisement(DBusConnection *conn,
						DBusMessage *msg,
						void *user_data)
{
	struct btd_advertising *manager = user_data;
	DBusMessageIter args;
	struct advertisement *ad;
	struct dbus_obj_match match;
	uint8_t instance;

	DBG("RegisterAdvertisement");

	if (!dbus_message_iter_init(msg, &args))
		return btd_error_invalid_args(msg);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_OBJECT_PATH)
		return btd_error_invalid_args(msg);

	dbus_message_iter_get_basic(&args, &match.path);

	match.owner = dbus_message_get_sender(msg);

	if (queue_find(manager->ads, match_advertisement, &match))
		return btd_error_already_exists(msg);

	instance = util_get_uid(&manager->instance_bitmap, manager->max_ads);
	if (!instance)
		return btd_error_failed(msg, "Maximum advertisements reached");

	dbus_message_iter_next(&args);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY)
		return btd_error_invalid_args(msg);

	ad = advertisement_create(conn, msg, match.path);
	if (!ad)
		return btd_error_failed(msg,
					"Failed to register advertisement");

	DBG("Registered advertisement at path %s", match.path);

	ad->instance = instance;
	ad->manager = manager;

	queue_push_tail(manager->ads, ad);

	return NULL;
}

static DBusMessage *unregister_advertisement(DBusConnection *conn,
						DBusMessage *msg,
						void *user_data)
{
	struct btd_advertising *manager = user_data;
	DBusMessageIter args;
	struct advertisement *ad;
	struct dbus_obj_match match;

	DBG("UnregisterAdvertisement");

	if (!dbus_message_iter_init(msg, &args))
		return btd_error_invalid_args(msg);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_OBJECT_PATH)
		return btd_error_invalid_args(msg);

	dbus_message_iter_get_basic(&args, &match.path);

	match.owner = dbus_message_get_sender(msg);

	ad = queue_find(manager->ads, match_advertisement, &match);
	if (!ad)
		return btd_error_does_not_exist(msg);

	advertisement_remove(ad);

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable methods[] = {
	{ GDBUS_EXPERIMENTAL_ASYNC_METHOD("RegisterAdvertisement",
					GDBUS_ARGS({ "advertisement", "o" },
							{ "options", "a{sv}" }),
					NULL, register_advertisement) },
	{ GDBUS_EXPERIMENTAL_ASYNC_METHOD("UnregisterAdvertisement",
						GDBUS_ARGS({ "service", "o" }),
						NULL,
						unregister_advertisement) },
	{ }
};

static void advertising_manager_destroy(void *user_data)
{
	struct btd_advertising *manager = user_data;

	queue_destroy(manager->ads, advertisement_destroy);

	mgmt_unref(manager->mgmt);

	free(manager);
}

static void read_adv_features_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_advertising *manager = user_data;
	const struct mgmt_rp_read_adv_features *feat = param;

	if (status || !param) {
		error("Failed to read advertising features: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	if (length < sizeof(*feat)) {
		error("Wrong size of read adv features response");
		return;
	}

	manager->max_adv_len = feat->max_adv_data_len;
	manager->max_ads = feat->max_instances;

	if (manager->max_ads == 0)
		return;

	if (!g_dbus_register_interface(btd_get_dbus_connection(),
					adapter_get_path(manager->adapter),
					LE_ADVERTISING_MGR_IFACE,
					methods, NULL, NULL, manager, NULL))
		error("Failed to register " LE_ADVERTISING_MGR_IFACE);
}

static struct btd_advertising *
advertising_manager_create(struct btd_adapter *adapter)
{
	struct btd_advertising *manager;

	manager = new0(struct btd_advertising, 1);
	manager->adapter = adapter;

	manager->mgmt = mgmt_new_default();

	if (!manager->mgmt) {
		error("Failed to access management interface");
		free(manager);
		return NULL;
	}

	manager->mgmt_index = btd_adapter_get_index(adapter);

	if (!mgmt_send(manager->mgmt, MGMT_OP_READ_ADV_FEATURES,
				manager->mgmt_index, 0, NULL,
				read_adv_features_callback, manager, NULL)) {
		error("Failed to read advertising features");
		advertising_manager_destroy(manager);
		return NULL;
	}

	manager->ads = queue_new();

	return manager;
}

struct btd_advertising *
btd_advertising_manager_new(struct btd_adapter *adapter)
{
	struct btd_advertising *manager;

	if (!adapter)
		return NULL;

	manager = advertising_manager_create(adapter);
	if (!manager)
		return NULL;

	DBG("LE Advertising Manager created for adapter: %s",
						adapter_get_path(adapter));

	return manager;
}

void btd_advertising_manager_destroy(struct btd_advertising *manager)
{
	if (!manager)
		return;

	g_dbus_unregister_interface(btd_get_dbus_connection(),
					adapter_get_path(manager->adapter),
					LE_ADVERTISING_MGR_IFACE);

	advertising_manager_destroy(manager);
}
