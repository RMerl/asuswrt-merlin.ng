// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Google Inc.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <dbus/dbus.h>
#include <gdbus/gdbus.h>

#include "lib/bluetooth.h"
#include "lib/mgmt.h"
#include "lib/sdp.h"

#include "adapter.h"
#include "dbus-common.h"
#include "error.h"
#include "log.h"
#include "eir.h"
#include "src/shared/ad.h"
#include "src/shared/mgmt.h"
#include "src/shared/queue.h"
#include "src/shared/util.h"
#include "advertising.h"

#define LE_ADVERTISING_MGR_IFACE "org.bluez.LEAdvertisingManager1"
#define LE_ADVERTISEMENT_IFACE "org.bluez.LEAdvertisement1"

struct btd_adv_manager {
	struct btd_adapter *adapter;
	struct queue *clients;
	struct mgmt *mgmt;
	uint16_t mgmt_index;
	uint8_t max_adv_len;
	uint8_t max_scan_rsp_len;
	uint8_t max_ads;
	uint32_t supported_flags;
	unsigned int instance_bitmap;
	bool extended_add_cmds;
	int8_t min_tx_power;
	int8_t max_tx_power;
};

#define AD_TYPE_BROADCAST 0
#define AD_TYPE_PERIPHERAL 1

/* BLUETOOTH SPECIFICATION Version 5.2 | Vol 4, Part E, page 2585
 * defines tx power value indicating no preference
 */
#define ADV_TX_POWER_NO_PREFERENCE 0x7F

struct btd_adv_client {
	struct btd_adv_manager *manager;
	char *owner;
	char *path;
	char *name;
	uint16_t appearance;
	uint16_t duration;
	uint16_t timeout;
	uint16_t discoverable_to;
	unsigned int to_id;
	unsigned int disc_to_id;
	unsigned int add_adv_id;
	GDBusClient *client;
	GDBusProxy *proxy;
	DBusMessage *reg;
	uint8_t type; /* Advertising type */
	uint32_t flags;
	struct bt_ad *data;
	struct bt_ad *scan;
	uint8_t instance;
	uint32_t min_interval;
	uint32_t max_interval;
	int8_t tx_power;
	mgmt_request_func_t refresh_done_func;
};

struct dbus_obj_match {
	const char *owner;
	const char *path;
};

static bool match_client(const void *a, const void *b)
{
	const struct btd_adv_client *client = a;
	const struct dbus_obj_match *match = b;

	if (match->owner && g_strcmp0(client->owner, match->owner))
		return false;

	if (match->path && g_strcmp0(client->path, match->path))
		return false;

	return true;
}

static void client_free(void *data)
{
	struct btd_adv_client *client = data;

	if (client->to_id > 0)
		g_source_remove(client->to_id);

	if (client->disc_to_id > 0)
		g_source_remove(client->disc_to_id);

	if (client->client) {
		g_dbus_client_set_disconnect_watch(client->client, NULL, NULL);
		g_dbus_client_unref(client->client);
	}

	if (client->reg) {
		DBusMessage *reply;

		reply = btd_error_failed(client->reg,
					"Failed to complete registration");
		g_dbus_send_message(btd_get_dbus_connection(), reply);
		dbus_message_unref(client->reg);
		client->reg = NULL;
	}

	if (client->add_adv_id)
		mgmt_cancel(client->manager->mgmt, client->add_adv_id);

	if (client->instance)
		util_clear_uid(&client->manager->instance_bitmap,
						client->instance);

	bt_ad_unref(client->data);
	bt_ad_unref(client->scan);

	g_dbus_proxy_unref(client->proxy);

	if (client->owner)
		g_free(client->owner);

	if (client->path)
		g_free(client->path);

	free(client->name);
	free(client);
}

static gboolean client_free_idle_cb(void *data)
{
	client_free(data);

	return FALSE;
}

static void client_release(void *data)
{
	struct btd_adv_client *client = data;

	DBG("Releasing advertisement %s, %s", client->owner, client->path);

	g_dbus_proxy_method_call(client->proxy, "Release", NULL, NULL, NULL,
									NULL);
}

static void client_destroy(void *data)
{
	client_release(data);
	client_free(data);
}

static void remove_advertising(struct btd_adv_manager *manager,
						uint8_t instance)
{
	struct mgmt_cp_remove_advertising cp;

	if (instance)
		DBG("instance %u", instance);
	else
		DBG("all instances");

	cp.instance = instance;

	mgmt_send(manager->mgmt, MGMT_OP_REMOVE_ADVERTISING,
			manager->mgmt_index, sizeof(cp), &cp, NULL, NULL, NULL);
}

static void client_remove(void *data)
{
	struct btd_adv_client *client = data;
	struct mgmt_cp_remove_advertising cp;

	g_dbus_client_set_proxy_handlers(client->client, NULL, NULL, NULL,
									client);
	g_dbus_client_set_disconnect_watch(client->client, NULL, NULL);

	cp.instance = client->instance;

	mgmt_send(client->manager->mgmt, MGMT_OP_REMOVE_ADVERTISING,
			client->manager->mgmt_index, sizeof(cp), &cp,
			NULL, NULL, NULL);

	queue_remove(client->manager->clients, client);

	g_idle_add(client_free_idle_cb, client);

	g_dbus_emit_property_changed(btd_get_dbus_connection(),
				adapter_get_path(client->manager->adapter),
				LE_ADVERTISING_MGR_IFACE, "SupportedInstances");

	g_dbus_emit_property_changed(btd_get_dbus_connection(),
				adapter_get_path(client->manager->adapter),
				LE_ADVERTISING_MGR_IFACE, "ActiveInstances");
}

static void client_disconnect_cb(DBusConnection *conn, void *user_data)
{
	DBG("Client disconnected");

	client_remove(user_data);
}

static bool parse_type(DBusMessageIter *iter, struct btd_adv_client *client)
{
	const char *msg_type;

	if (!iter)
		return true;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_STRING)
		return false;

	dbus_message_iter_get_basic(iter, &msg_type);

	if (!g_strcmp0(msg_type, "broadcast")) {
		client->type = AD_TYPE_BROADCAST;
		return true;
	}

	if (!g_strcmp0(msg_type, "peripheral")) {
		client->type = AD_TYPE_PERIPHERAL;
		return true;
	}

	return false;
}

static bool parse_service_uuids(DBusMessageIter *iter,
					struct btd_adv_client *client)
{
	DBusMessageIter ariter;

	if (!iter) {
		bt_ad_clear_service_uuid(client->data);
		return true;
	}

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return false;

	dbus_message_iter_recurse(iter, &ariter);

	bt_ad_clear_service_uuid(client->data);

	while (dbus_message_iter_get_arg_type(&ariter) == DBUS_TYPE_STRING) {
		const char *uuid_str;
		bt_uuid_t uuid;

		dbus_message_iter_get_basic(&ariter, &uuid_str);

		DBG("Adding ServiceUUID: %s", uuid_str);

		if (bt_string_to_uuid(&uuid, uuid_str) < 0)
			goto fail;

		if (!bt_ad_add_service_uuid(client->data, &uuid))
			goto fail;

		dbus_message_iter_next(&ariter);
	}

	return true;

fail:
	bt_ad_clear_service_uuid(client->data);
	return false;
}

static bool parse_solicit_uuids(DBusMessageIter *iter,
					struct btd_adv_client *client)
{
	DBusMessageIter ariter;

	if (!iter) {
		bt_ad_clear_solicit_uuid(client->data);
		return true;
	}

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return false;

	dbus_message_iter_recurse(iter, &ariter);

	bt_ad_clear_solicit_uuid(client->data);

	while (dbus_message_iter_get_arg_type(&ariter) == DBUS_TYPE_STRING) {
		const char *uuid_str;
		bt_uuid_t uuid;

		dbus_message_iter_get_basic(&ariter, &uuid_str);

		DBG("Adding SolicitUUID: %s", uuid_str);

		if (bt_string_to_uuid(&uuid, uuid_str) < 0)
			goto fail;

		if (!bt_ad_add_solicit_uuid(client->data, &uuid))
			goto fail;

		dbus_message_iter_next(&ariter);
	}

	return true;

fail:
	bt_ad_clear_solicit_uuid(client->data);
	return false;
}

static bool parse_manufacturer_data(DBusMessageIter *iter,
					struct btd_adv_client *client)
{
	DBusMessageIter entries;

	if (!iter) {
		bt_ad_clear_manufacturer_data(client->data);
		return true;
	}

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return false;

	dbus_message_iter_recurse(iter, &entries);

	bt_ad_clear_manufacturer_data(client->data);

	while (dbus_message_iter_get_arg_type(&entries)
						== DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter value, entry, array;
		uint16_t manuf_id;
		uint8_t *manuf_data;
		int len;

		dbus_message_iter_recurse(&entries, &entry);
		dbus_message_iter_get_basic(&entry, &manuf_id);

		dbus_message_iter_next(&entry);

		if (dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_VARIANT)
			goto fail;

		dbus_message_iter_recurse(&entry, &value);

		if (dbus_message_iter_get_arg_type(&value) != DBUS_TYPE_ARRAY)
			goto fail;

		dbus_message_iter_recurse(&value, &array);

		if (dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_BYTE)
			goto fail;

		dbus_message_iter_get_fixed_array(&array, &manuf_data, &len);

		DBG("Adding ManufacturerData for %04x", manuf_id);

		if (!bt_ad_add_manufacturer_data(client->data, manuf_id,
							manuf_data, len))
			goto fail;

		dbus_message_iter_next(&entries);
	}

	return true;

fail:
	bt_ad_clear_manufacturer_data(client->data);
	return false;
}

static bool parse_service_data(DBusMessageIter *iter,
					struct btd_adv_client *client)
{
	DBusMessageIter entries;

	if (!iter) {
		bt_ad_clear_service_data(client->data);
		return true;
	}

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return false;

	dbus_message_iter_recurse(iter, &entries);

	bt_ad_clear_service_data(client->data);

	while (dbus_message_iter_get_arg_type(&entries)
						== DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter value, entry, array;
		const char *uuid_str;
		bt_uuid_t uuid;
		uint8_t *service_data;
		int len;

		dbus_message_iter_recurse(&entries, &entry);
		dbus_message_iter_get_basic(&entry, &uuid_str);

		if (bt_string_to_uuid(&uuid, uuid_str) < 0)
			goto fail;

		dbus_message_iter_next(&entry);

		if (dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_VARIANT)
			goto fail;

		dbus_message_iter_recurse(&entry, &value);

		if (dbus_message_iter_get_arg_type(&value) != DBUS_TYPE_ARRAY)
			goto fail;

		dbus_message_iter_recurse(&value, &array);

		if (dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_BYTE)
			goto fail;

		dbus_message_iter_get_fixed_array(&array, &service_data, &len);

		DBG("Adding ServiceData for %s", uuid_str);

		if (!bt_ad_add_service_data(client->data, &uuid, service_data,
									len))
			goto fail;

		dbus_message_iter_next(&entries);
	}

	return true;

fail:
	bt_ad_clear_service_data(client->data);
	return false;
}

static struct adv_include {
	uint8_t flag;
	const char *name;
} includes[] = {
	{ MGMT_ADV_FLAG_TX_POWER, "tx-power" },
	{ MGMT_ADV_FLAG_APPEARANCE, "appearance" },
	{ MGMT_ADV_FLAG_LOCAL_NAME, "local-name" },
	{ },
};

static bool parse_includes(DBusMessageIter *iter,
					struct btd_adv_client *client)
{
	DBusMessageIter entries;

	if (!iter) {
		client->flags = 0;
		return true;
	}

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return false;

	dbus_message_iter_recurse(iter, &entries);

	/* Reset flags before parsing */
	client->flags = 0;

	while (dbus_message_iter_get_arg_type(&entries) == DBUS_TYPE_STRING) {
		const char *str;
		struct adv_include *inc;

		dbus_message_iter_get_basic(&entries, &str);

		for (inc = includes; inc && inc->name; inc++) {
			if (strcmp(str, inc->name))
				continue;

			if (!(client->manager->supported_flags & inc->flag))
				continue;

			DBG("Including Feature: %s", str);

			client->flags |= inc->flag;
		}

		dbus_message_iter_next(&entries);
	}

	return true;
}

static bool parse_local_name(DBusMessageIter *iter,
					struct btd_adv_client *client)
{
	const char *name;

	if (!iter) {
		free(client->name);
		client->name = NULL;
		return true;
	}

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_STRING)
		return false;

	if (client->flags & MGMT_ADV_FLAG_LOCAL_NAME) {
		error("Local name already included");
		return false;
	}

	dbus_message_iter_get_basic(iter, &name);

	free(client->name);
	client->name = strdup(name);

	return true;
}

static bool parse_appearance(DBusMessageIter *iter,
					struct btd_adv_client *client)
{
	if (!iter) {
		client->appearance = 0;
		return true;
	}

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_UINT16)
		return false;

	if (client->flags & MGMT_ADV_FLAG_APPEARANCE) {
		error("Appearance already included");
		return false;
	}

	dbus_message_iter_get_basic(iter, &client->appearance);

	return true;
}

static bool parse_duration(DBusMessageIter *iter,
					struct btd_adv_client *client)
{
	if (!iter) {
		client->duration = 0;
		return true;
	}

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_UINT16)
		return false;

	dbus_message_iter_get_basic(iter, &client->duration);

	return true;
}

static gboolean client_timeout(void *user_data)
{
	struct btd_adv_client *client = user_data;

	DBG("");

	client->to_id = 0;

	client_release(client);
	client_remove(client);

	return FALSE;
}

static bool parse_timeout(DBusMessageIter *iter,
					struct btd_adv_client *client)
{
	if (!iter) {
		client->timeout = 0;
		g_source_remove(client->to_id);
		client->to_id = 0;
		return true;
	}

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_UINT16)
		return false;

	dbus_message_iter_get_basic(iter, &client->timeout);

	if (client->to_id)
		g_source_remove(client->to_id);

	if (client->timeout > 0)
		client->to_id = g_timeout_add_seconds(client->timeout,
							client_timeout, client);

	return true;
}

static bool parse_data(DBusMessageIter *iter, struct btd_adv_client *client)
{
	DBusMessageIter entries;

	if (!iter) {
		bt_ad_clear_data(client->data);
		return true;
	}

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return false;

	dbus_message_iter_recurse(iter, &entries);

	bt_ad_clear_data(client->data);

	while (dbus_message_iter_get_arg_type(&entries)
						== DBUS_TYPE_DICT_ENTRY) {
		DBusMessageIter value, entry, array;
		uint8_t type;
		uint8_t *data;
		int len;

		dbus_message_iter_recurse(&entries, &entry);
		dbus_message_iter_get_basic(&entry, &type);

		dbus_message_iter_next(&entry);

		if (dbus_message_iter_get_arg_type(&entry) != DBUS_TYPE_VARIANT)
			goto fail;

		dbus_message_iter_recurse(&entry, &value);

		if (dbus_message_iter_get_arg_type(&value) != DBUS_TYPE_ARRAY)
			goto fail;

		dbus_message_iter_recurse(&value, &array);

		if (dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_BYTE)
			goto fail;

		dbus_message_iter_get_fixed_array(&array, &data, &len);

		DBG("Adding Data for type 0x%02x len %u", type, len);

		if (!bt_ad_add_data(client->data, type, data, len))
			goto fail;

		dbus_message_iter_next(&entries);
	}

	return true;

fail:
	bt_ad_clear_data(client->data);
	return false;
}

static bool set_flags(struct btd_adv_client *client, uint8_t flags)
{
	if (!flags) {
		bt_ad_clear_flags(client->data);
		return true;
	}

	/* Set BR/EDR Not Supported for LE only */
	if (!btd_adapter_get_bredr(client->manager->adapter))
		flags |= BT_AD_FLAG_NO_BREDR;

	/* Set BR/EDR Not Supported if adapter is not discoverable but the
	 * instance is.
	 */
	if ((flags & (BT_AD_FLAG_GENERAL | BT_AD_FLAG_LIMITED)) &&
			!btd_adapter_get_discoverable(client->manager->adapter))
		flags |= BT_AD_FLAG_NO_BREDR;

	if (!bt_ad_add_flags(client->data, &flags, 1))
		return false;

	return true;
}

static bool parse_discoverable(DBusMessageIter *iter,
				struct btd_adv_client *client)
{
	uint8_t flags;
	dbus_bool_t discoverable;

	if (!iter) {
		bt_ad_clear_flags(client->data);
		return true;
	}

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_BOOLEAN)
		return false;

	dbus_message_iter_get_basic(iter, &discoverable);

	if (discoverable)
		flags = BT_AD_FLAG_GENERAL;
	else
		flags = 0x00;

	if (!set_flags(client , flags))
		goto fail;

	DBG("Adding Flags 0x%02x", flags);

	return true;

fail:
	bt_ad_clear_flags(client->data);
	return false;
}

static size_t calc_max_adv_len(struct btd_adv_client *client, uint32_t flags)
{
	size_t max = client->manager->max_adv_len;

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

static uint8_t *generate_adv_data(struct btd_adv_client *client,
						uint32_t *flags, size_t *len)
{
	if ((*flags & MGMT_ADV_FLAG_APPEARANCE) ||
					client->appearance != UINT16_MAX) {
		uint16_t appearance;

		appearance = client->appearance;
		if (appearance == UINT16_MAX)
			/* TODO: Get the appearance from the adaptor once
			 * supported.
			 */
			appearance = 0x000;

		bt_ad_add_appearance(client->data, appearance);
	}

	return bt_ad_generate(client->data, len);
}

static uint8_t *generate_scan_rsp(struct btd_adv_client *client,
						uint32_t *flags, size_t *len)
{
	struct btd_adv_manager *manager = client->manager;
	const char *name;

	if (!(*flags & MGMT_ADV_FLAG_LOCAL_NAME) && !client->name) {
		*len = 0;
		return NULL;
	}

	*flags &= ~MGMT_ADV_FLAG_LOCAL_NAME;

	name = client->name;
	if (!name)
		name = btd_adapter_get_name(manager->adapter);

	bt_ad_add_name(client->scan, name);

	return bt_ad_generate(client->scan, len);
}

static int get_adv_flags(struct btd_adv_client *client)
{
	uint32_t flags = 0;

	if (client->type == AD_TYPE_PERIPHERAL) {
		flags = MGMT_ADV_FLAG_CONNECTABLE;

		if (btd_adapter_get_discoverable(client->manager->adapter) &&
				!(bt_ad_has_flags(client->data)))
			flags |= MGMT_ADV_FLAG_DISCOV;
	}

	flags |= client->flags;

	return flags;
}

static int refresh_legacy_adv(struct btd_adv_client *client,
				mgmt_request_func_t func,
				unsigned int *mgmt_id)
{
	struct mgmt_cp_add_advertising *cp;
	uint8_t param_len;
	uint8_t *adv_data;
	size_t adv_data_len;
	uint8_t *scan_rsp;
	size_t scan_rsp_len = -1;
	uint32_t flags = 0;
	unsigned int mgmt_ret;

	DBG("Refreshing advertisement: %s", client->path);

	flags = get_adv_flags(client);

	adv_data = generate_adv_data(client, &flags, &adv_data_len);
	if (!adv_data || (adv_data_len > calc_max_adv_len(client, flags))) {
		error("Advertising data too long or couldn't be generated.");
		return -EINVAL;
	}

	scan_rsp = generate_scan_rsp(client, &flags, &scan_rsp_len);
	if (!scan_rsp && scan_rsp_len) {
		error("Scan data couldn't be generated.");
		free(adv_data);
		return -EINVAL;
	}

	param_len = sizeof(struct mgmt_cp_add_advertising) + adv_data_len +
							scan_rsp_len;

	cp = malloc0(param_len);
	if (!cp) {
		error("Couldn't allocate for MGMT!");
		free(adv_data);
		free(scan_rsp);
		return -ENOMEM;
	}

	cp->flags = htobl(flags);
	cp->instance = client->instance;
	cp->duration = client->duration;
	cp->adv_data_len = adv_data_len;
	cp->scan_rsp_len = scan_rsp_len;
	memcpy(cp->data, adv_data, adv_data_len);
	memcpy(cp->data + adv_data_len, scan_rsp, scan_rsp_len);

	free(adv_data);
	free(scan_rsp);

	mgmt_ret = mgmt_send(client->manager->mgmt, MGMT_OP_ADD_ADVERTISING,
			client->manager->mgmt_index, param_len, cp,
			func, client, NULL);

	if (!mgmt_ret) {
		error("Failed to add Advertising Data");
		free(cp);
		return -EINVAL;
	}
	if (mgmt_id)
		*mgmt_id = mgmt_ret;

	free(cp);

	return 0;
}

static void add_adv_params_callback(uint8_t status, uint16_t length,
				    const void *param, void *user_data);

static int refresh_extended_adv(struct btd_adv_client *client,
				mgmt_request_func_t func, unsigned int *mgmt_id)
{
	struct mgmt_cp_add_ext_adv_params cp;
	uint32_t flags = 0;
	unsigned int mgmt_ret = 0;

	DBG("Refreshing advertisement parameters: %s", client->path);

	flags = get_adv_flags(client);

	memset(&cp, 0, sizeof(cp));
	cp.instance = client->instance;

	/* Not all advertising instances will use all possible parameters. The
	 * included_params bit field tells the kernel which parameters are
	 * relevant, and sensible defaults will be used for the rest
	 */

	if (client->duration) {
		cp.duration = client->duration;
		flags |= MGMT_ADV_PARAM_DURATION;
	}

	if (client->min_interval && client->max_interval) {
		cp.min_interval = client->min_interval;
		cp.max_interval = client->max_interval;
		flags |= MGMT_ADV_PARAM_INTERVALS;
	}

	if (client->tx_power != ADV_TX_POWER_NO_PREFERENCE) {
		cp.tx_power = client->tx_power;
		flags |= MGMT_ADV_PARAM_TX_POWER;
	}

	cp.flags = htobl(flags);

	mgmt_ret = mgmt_send(client->manager->mgmt, MGMT_OP_ADD_EXT_ADV_PARAMS,
			client->manager->mgmt_index, sizeof(cp), &cp,
			add_adv_params_callback, client, NULL);

	if (!mgmt_ret) {
		error("Failed to request extended advertising parameters");
		return -EINVAL;
	}

	/* Store callback, called after we set advertising data */
	client->refresh_done_func = func;

	if (mgmt_id)
		*mgmt_id = mgmt_ret;


	return 0;
}

static int refresh_advertisement(struct btd_adv_client *client,
			mgmt_request_func_t func, unsigned int *mgmt_id)
{
	if (client->manager->extended_add_cmds)
		return refresh_extended_adv(client, func, mgmt_id);

	return refresh_legacy_adv(client, func, mgmt_id);
}

static gboolean client_discoverable_timeout(void *user_data)
{
	struct btd_adv_client *client = user_data;

	DBG("");

	client->disc_to_id = 0;

	bt_ad_clear_flags(client->data);

	refresh_advertisement(client, NULL, NULL);

	return FALSE;
}

static bool parse_discoverable_timeout(DBusMessageIter *iter,
					struct btd_adv_client *client)
{
	if (!iter) {
		client->discoverable_to = 0;
		g_source_remove(client->disc_to_id);
		client->disc_to_id = 0;
		return true;
	}

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_UINT16)
		return false;

	dbus_message_iter_get_basic(iter, &client->discoverable_to);

	if (client->disc_to_id)
		g_source_remove(client->disc_to_id);

	client->disc_to_id = g_timeout_add_seconds(client->discoverable_to,
						client_discoverable_timeout,
						client);

	return true;
}

static struct adv_secondary {
	int flag;
	const char *name;
} secondary[] = {
	{ MGMT_ADV_FLAG_SEC_1M, "1M" },
	{ MGMT_ADV_FLAG_SEC_2M, "2M" },
	{ MGMT_ADV_FLAG_SEC_CODED, "Coded" },
	{ },
};

static bool parse_secondary(DBusMessageIter *iter,
					struct btd_adv_client *client)
{
	const char *str;
	struct adv_secondary *sec;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_STRING)
		return false;

	/* Reset secondary channels before parsing */
	client->flags &= 0xfe00;

	dbus_message_iter_get_basic(iter, &str);

	for (sec = secondary; sec && sec->name; sec++) {
		if (strcmp(str, sec->name))
			continue;

		if (!(client->manager->supported_flags & sec->flag))
			return false;

		DBG("Secondary Channel: %s", str);

		client->flags |= sec->flag;

		return true;
	}

	return false;
}

static bool parse_min_interval(DBusMessageIter *iter,
					struct btd_adv_client *client)
{
	uint32_t min_interval_ms;

	/* Only consider this property if experimental setting is applied */
	if (!(g_dbus_get_flags() & G_DBUS_FLAG_ENABLE_EXPERIMENTAL))
		return true;

	if (!iter)
		return false;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_UINT32)
		return false;

	dbus_message_iter_get_basic(iter, &min_interval_ms);

	/* Convert ms to jiffies to be used in adv request */
	client->min_interval = min_interval_ms / 0.625;

	/* BLUETOOTH SPECIFICATION Version 5.2 | Vol 4, Part E, page 2584
	 * defines acceptable interval range
	 */
	if (client->min_interval < 0x20 || client->min_interval > 0xFFFFFF) {
		client->min_interval = 0;
		return false;
	}

	return true;
}

static bool parse_max_interval(DBusMessageIter *iter,
					struct btd_adv_client *client)
{
	uint32_t max_interval_ms;

	/* Only consider this property if experimental setting is applied */
	if (!(g_dbus_get_flags() & G_DBUS_FLAG_ENABLE_EXPERIMENTAL))
		return true;

	if (!iter)
		return false;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_UINT32)
		return false;

	dbus_message_iter_get_basic(iter, &max_interval_ms);

	/* Convert ms to jiffies to be used in adv request */
	client->max_interval = max_interval_ms / 0.625;

	/* BLUETOOTH SPECIFICATION Version 5.2 | Vol 4, Part E, page 2584
	 * defines acceptable interval range
	 */
	if (client->max_interval < 0x20 || client->max_interval > 0xFFFFFF) {
		client->max_interval = 0;
		return false;
	}

	return true;
}

static bool parse_tx_power(DBusMessageIter *iter,
					struct btd_adv_client *client)
{
	int16_t val;

	/* Only consider this property if experimental setting is applied */
	if (!(g_dbus_get_flags() & G_DBUS_FLAG_ENABLE_EXPERIMENTAL))
		return true;

	if (!iter)
		return false;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_INT16)
		return false;

	dbus_message_iter_get_basic(iter, &val);

	/* BLUETOOTH SPECIFICATION Version 5.2 | Vol 4, Part E, page 2585
	 * defines acceptable tx power range
	 */
	if (val < -127 || val > 20)
		return false;

	client->tx_power = val;

	return true;
}

static struct adv_parser {
	const char *name;
	bool (*func)(DBusMessageIter *iter, struct btd_adv_client *client);
} parsers[] = {
	{ "Type", parse_type },
	{ "ServiceUUIDs", parse_service_uuids },
	{ "SolicitUUIDs", parse_solicit_uuids },
	{ "ManufacturerData", parse_manufacturer_data },
	{ "ServiceData", parse_service_data },
	{ "Includes", parse_includes },
	{ "LocalName", parse_local_name },
	{ "Appearance", parse_appearance },
	{ "Duration", parse_duration },
	{ "Timeout", parse_timeout },
	{ "Data", parse_data },
	{ "Discoverable", parse_discoverable },
	{ "DiscoverableTimeout", parse_discoverable_timeout },
	{ "SecondaryChannel", parse_secondary },
	{ "MinInterval", parse_min_interval },
	{ "MaxInterval", parse_max_interval },
	{ "TxPower", parse_tx_power },
	{ },
};

static void properties_changed(GDBusProxy *proxy, const char *name,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adv_client *client = user_data;
	struct adv_parser *parser;

	for (parser = parsers; parser && parser->name; parser++) {
		if (strcmp(parser->name, name))
			continue;

		if (parser->func(iter, client)) {
			refresh_advertisement(client, NULL, NULL);

			break;
		}
	}
}

static void add_client_complete(struct btd_adv_client *client, uint8_t status)
{
	DBusMessage *reply;

	if (status) {
		error("Failed to add advertisement: %s (0x%02x)",
						mgmt_errstr(status), status);
		reply = btd_error_failed(client->reg,
					"Failed to register advertisement");
		queue_remove(client->manager->clients, client);
		g_idle_add(client_free_idle_cb, client);

	} else
		reply = dbus_message_new_method_return(client->reg);

	g_dbus_send_message(btd_get_dbus_connection(), reply);
	dbus_message_unref(client->reg);
	client->reg = NULL;
}

static void add_adv_callback(uint8_t status, uint16_t length,
					  const void *param, void *user_data)
{
	struct btd_adv_client *client = user_data;
	const struct mgmt_rp_add_advertising *rp = param;

	client->add_adv_id = 0;

	if (status)
		goto done;

	if (!param || length < sizeof(*rp)) {
		status = MGMT_STATUS_FAILED;
		goto done;
	}

	client->instance = rp->instance;

	g_dbus_client_set_disconnect_watch(client->client, client_disconnect_cb,
									client);
	DBG("Advertisement registered: %s", client->path);

	g_dbus_emit_property_changed(btd_get_dbus_connection(),
				adapter_get_path(client->manager->adapter),
				LE_ADVERTISING_MGR_IFACE, "SupportedInstances");

	g_dbus_emit_property_changed(btd_get_dbus_connection(),
				adapter_get_path(client->manager->adapter),
				LE_ADVERTISING_MGR_IFACE, "ActiveInstances");

	g_dbus_proxy_set_property_watch(client->proxy, properties_changed,
								client);

done:
	add_client_complete(client, status);
}

static void add_adv_params_callback(uint8_t status, uint16_t length,
					  const void *param, void *user_data)
{
	struct btd_adv_client *client = user_data;
	const struct mgmt_rp_add_ext_adv_params *rp = param;
	struct mgmt_cp_add_ext_adv_data *cp = NULL;
	uint8_t param_len;
	uint8_t *adv_data = NULL;
	size_t adv_data_len;
	uint8_t *scan_rsp = NULL;
	size_t scan_rsp_len = -1;
	uint32_t flags = 0;
	unsigned int mgmt_ret;
	dbus_int16_t tx_power;

	if (status)
		goto fail;

	if (!param || length < sizeof(*rp)) {
		status = MGMT_STATUS_FAILED;
		goto fail;
	}

	DBG("Refreshing advertisement data: %s", client->path);

	/* Update tx power held by client */
	tx_power = rp->tx_power;
	if (tx_power != ADV_TX_POWER_NO_PREFERENCE)
		g_dbus_proxy_set_property_basic(client->proxy, "TxPower",
				DBUS_TYPE_INT16, &tx_power, NULL, NULL, NULL);

	client->instance = rp->instance;

	flags = get_adv_flags(client);

	adv_data = generate_adv_data(client, &flags, &adv_data_len);
	if (!adv_data || (adv_data_len > rp->max_adv_data_len)) {
		error("Advertising data too long or couldn't be generated.");
		goto fail;
	}

	scan_rsp = generate_scan_rsp(client, &flags, &scan_rsp_len);
	if ((!scan_rsp && scan_rsp_len) ||
			scan_rsp_len > rp->max_scan_rsp_len) {
		error("Scan data couldn't be generated.");
		goto fail;
	}

	param_len = sizeof(struct mgmt_cp_add_advertising) + adv_data_len +
							scan_rsp_len;

	cp = malloc0(param_len);
	if (!cp) {
		error("Couldn't allocate for MGMT!");
		goto fail;
	}

	cp->instance = client->instance;
	cp->adv_data_len = adv_data_len;
	cp->scan_rsp_len = scan_rsp_len;
	memcpy(cp->data, adv_data, adv_data_len);
	memcpy(cp->data + adv_data_len, scan_rsp, scan_rsp_len);

	free(adv_data);
	free(scan_rsp);
	adv_data = NULL;
	scan_rsp = NULL;

	/* Submit request to update instance data */
	mgmt_ret = mgmt_send(client->manager->mgmt, MGMT_OP_ADD_EXT_ADV_DATA,
			     client->manager->mgmt_index, param_len, cp,
			     client->refresh_done_func, client, NULL);

	/* Clear the callback */
	client->refresh_done_func = NULL;

	if (!mgmt_ret) {
		error("Failed to add Advertising Data");
		goto fail;
	}

	if (client->add_adv_id)
		client->add_adv_id = mgmt_ret;

	free(cp);
	cp = NULL;

	return;

fail:
	if (adv_data)
		free(adv_data);

	if (scan_rsp)
		free(scan_rsp);

	if (cp)
		free(cp);

	if (!status)
		status = -EINVAL;

	/* Failure for any reason ends this advertising request */
	add_client_complete(client, status);
}

static DBusMessage *parse_advertisement(struct btd_adv_client *client)
{
	struct adv_parser *parser;
	int err;

	for (parser = parsers; parser && parser->name; parser++) {
		DBusMessageIter iter;

		if (!g_dbus_proxy_get_property(client->proxy, parser->name,
								&iter))
			continue;

		if (!parser->func(&iter, client)) {
			error("Error parsing %s property", parser->name);
			goto fail;
		}
	}

	if (bt_ad_has_flags(client->data)) {
		/* BLUETOOTH SPECIFICATION Version 5.0 | Vol 3, Part C
		 * page 2042:
		 * A device in the broadcast mode shall not set the
		 * ‘LE General Discoverable Mode’ flag or the
		 * ‘LE Limited Discoverable Mode’ flag in the Flags AD Type
		 * as defined in [Core Specification Supplement], Part A,
		 * Section 1.3.
		 */
		if (client->type == AD_TYPE_BROADCAST) {
			error("Broadcast cannot set flags");
			goto fail;
		}

		/* Set Limited Discoverable if DiscoverableTimeout is set */
		if (client->disc_to_id &&
				!set_flags(client, BT_AD_FLAG_LIMITED)) {
			error("Failed to set Limited Discoverable Flag");
			goto fail;
		}
	} else if (client->disc_to_id) {
		/* Ignore DiscoverableTimeout if not discoverable */
		g_source_remove(client->disc_to_id);
		client->disc_to_id = 0;
		client->discoverable_to = 0;
	}

	if (client->timeout && client->timeout < client->discoverable_to) {
		/* DiscoverableTimeout must not be bigger than Timeout */
		error("DiscoverableTimeout > Timeout");
		goto fail;
	}

	if (client->min_interval > client->max_interval) {
		/* Min interval must not be bigger than max interval */
		error("MinInterval must be less than MaxInterval (%u > %u)",
				client->min_interval, client->max_interval);
		goto fail;
	}

	err = refresh_advertisement(client, add_adv_callback,
					&client->add_adv_id);

	if (!err)
		return NULL;

fail:
	return btd_error_failed(client->reg, "Failed to parse advertisement.");
}

static void client_proxy_added(GDBusProxy *proxy, void *data)
{
	struct btd_adv_client *client = data;
	DBusMessage *reply;
	const char *interface;

	interface = g_dbus_proxy_get_interface(proxy);
	if (g_str_equal(interface, LE_ADVERTISEMENT_IFACE) == FALSE)
		return;

	reply = parse_advertisement(client);
	if (!reply)
		return;

	/* Failed to publish for some reason, remove. */
	queue_remove(client->manager->clients, client);

	g_idle_add(client_free_idle_cb, client);

	g_dbus_send_message(btd_get_dbus_connection(), reply);

	dbus_message_unref(client->reg);
	client->reg = NULL;
}

static struct btd_adv_client *client_create(struct btd_adv_manager *manager,
					DBusConnection *conn,
					DBusMessage *msg, const char *path)
{
	struct btd_adv_client *client;
	const char *sender = dbus_message_get_sender(msg);

	if (!path || !g_str_has_prefix(path, "/"))
		return NULL;

	client = new0(struct btd_adv_client, 1);
	client->client = g_dbus_client_new_full(conn, sender, path, path);
	if (!client->client)
		goto fail;

	client->owner = g_strdup(sender);
	if (!client->owner)
		goto fail;

	client->path = g_strdup(path);
	if (!client->path)
		goto fail;

	DBG("Adding proxy for %s", path);
	client->proxy = g_dbus_proxy_new(client->client, path,
						LE_ADVERTISEMENT_IFACE);
	if (!client->proxy)
		goto fail;

	g_dbus_client_set_proxy_handlers(client->client, client_proxy_added,
							NULL, NULL, client);

	client->data = bt_ad_new();
	if (!client->data)
		goto fail;

	client->scan = bt_ad_new();
	if (!client->scan)
		goto fail;

	client->manager = manager;
	client->appearance = UINT16_MAX;
	client->tx_power = ADV_TX_POWER_NO_PREFERENCE;
	client->min_interval = 0;
	client->max_interval = 0;

	client->refresh_done_func = NULL;

	return client;

fail:
	client_free(client);
	return NULL;
}

static DBusMessage *register_advertisement(DBusConnection *conn,
						DBusMessage *msg,
						void *user_data)
{
	struct btd_adv_manager *manager = user_data;
	DBusMessageIter args;
	struct btd_adv_client *client;
	struct dbus_obj_match match;

	DBG("RegisterAdvertisement");

	if (!dbus_message_iter_init(msg, &args))
		return btd_error_invalid_args(msg);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_OBJECT_PATH)
		return btd_error_invalid_args(msg);

	dbus_message_iter_get_basic(&args, &match.path);

	match.owner = dbus_message_get_sender(msg);

	if (queue_find(manager->clients, match_client, &match))
		return btd_error_already_exists(msg);

	dbus_message_iter_next(&args);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY)
		return btd_error_invalid_args(msg);

	client = client_create(manager, conn, msg, match.path);
	if (!client)
		return btd_error_failed(msg,
					"Failed to register advertisement");

	client->instance = util_get_uid(&manager->instance_bitmap,
							manager->max_ads);
	if (!client->instance) {
		client_free(client);
		return btd_error_not_permitted(msg,
					"Maximum advertisements reached");
	}

	DBG("Registered advertisement at path %s", match.path);

	client->reg = dbus_message_ref(msg);

	queue_push_tail(manager->clients, client);

	return NULL;
}

static DBusMessage *unregister_advertisement(DBusConnection *conn,
						DBusMessage *msg,
						void *user_data)
{
	struct btd_adv_manager *manager = user_data;
	DBusMessageIter args;
	struct btd_adv_client *client;
	struct dbus_obj_match match;

	DBG("UnregisterAdvertisement");

	if (!dbus_message_iter_init(msg, &args))
		return btd_error_invalid_args(msg);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_OBJECT_PATH)
		return btd_error_invalid_args(msg);

	dbus_message_iter_get_basic(&args, &match.path);

	match.owner = dbus_message_get_sender(msg);

	client = queue_find(manager->clients, match_client, &match);
	if (!client)
		return btd_error_does_not_exist(msg);

	client_remove(client);

	return dbus_message_new_method_return(msg);
}

static gboolean get_instances(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_adv_manager *manager = data;
	uint8_t instances;

	instances = manager->max_ads - queue_length(manager->clients);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BYTE, &instances);

	return TRUE;
}

static gboolean get_active_instances(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_adv_manager *manager = data;
	uint8_t instances;

	instances = queue_length(manager->clients);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BYTE, &instances);

	return TRUE;
}

static void append_include(struct btd_adv_manager *manager,
						DBusMessageIter *iter)
{
	struct adv_include *inc;

	for (inc = includes; inc && inc->name; inc++) {
		if (manager->supported_flags & inc->flag)
			dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING,
								&inc->name);
	}
}

static gboolean get_supported_includes(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_adv_manager *manager = data;
	DBusMessageIter entry;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_STRING_AS_STRING, &entry);

	append_include(manager, &entry);

	dbus_message_iter_close_container(iter, &entry);

	return TRUE;
}

static void append_secondary(struct btd_adv_manager *manager,
						DBusMessageIter *iter)
{
	struct adv_secondary *sec;

	for (sec = secondary; sec && sec->name; sec++) {
		if (manager->supported_flags & sec->flag)
			dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING,
								&sec->name);
	}
}

static gboolean secondary_exits(const GDBusPropertyTable *property, void *data)
{
	struct btd_adv_manager *manager = data;

	/* 1M PHY shall always be supported if ext_adv is supported */
	return manager->supported_flags & MGMT_ADV_FLAG_SEC_1M;
}

static gboolean get_supported_secondary(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_adv_manager *manager = data;
	DBusMessageIter entry;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_STRING_AS_STRING, &entry);

	append_secondary(manager, &entry);

	dbus_message_iter_close_container(iter, &entry);

	return TRUE;
}

static gboolean get_supported_cap(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_adv_manager *manager = data;
	DBusMessageIter dict;
	int16_t min_tx_power = manager->min_tx_power;
	int16_t max_tx_power = manager->max_tx_power;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	if (min_tx_power != ADV_TX_POWER_NO_PREFERENCE)
		dict_append_entry(&dict, "MinTxPower", DBUS_TYPE_INT16,
				&min_tx_power);

	if (max_tx_power != ADV_TX_POWER_NO_PREFERENCE)
		dict_append_entry(&dict, "MaxTxPower", DBUS_TYPE_INT16,
				&max_tx_power);

	dict_append_entry(&dict, "MaxAdvLen", DBUS_TYPE_BYTE,
			&manager->max_adv_len);
	dict_append_entry(&dict, "MaxScnRspLen", DBUS_TYPE_BYTE,
			&manager->max_scan_rsp_len);

	dbus_message_iter_close_container(iter, &dict);

	return TRUE;
}

static const GDBusPropertyTable properties[] = {
	{ "ActiveInstances", "y", get_active_instances, NULL, NULL },
	{ "SupportedInstances", "y", get_instances, NULL, NULL },
	{ "SupportedIncludes", "as", get_supported_includes, NULL, NULL },
	{ "SupportedSecondaryChannels", "as", get_supported_secondary, NULL,
							secondary_exits },
	{ "SupportedCapabilities", "a{sv}", get_supported_cap, NULL, NULL,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL},
	{ }
};

static const GDBusMethodTable methods[] = {
	{ GDBUS_ASYNC_METHOD("RegisterAdvertisement",
					GDBUS_ARGS({ "advertisement", "o" },
							{ "options", "a{sv}" }),
					NULL, register_advertisement) },
	{ GDBUS_ASYNC_METHOD("UnregisterAdvertisement",
						GDBUS_ARGS({ "service", "o" }),
						NULL,
						unregister_advertisement) },
	{ }
};

static void manager_destroy(void *user_data)
{
	struct btd_adv_manager *manager = user_data;

	queue_destroy(manager->clients, client_destroy);

	mgmt_unref(manager->mgmt);

	free(manager);
}

static void read_adv_features_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adv_manager *manager = user_data;
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
	manager->max_scan_rsp_len = feat->max_scan_rsp_len;
	manager->max_ads = feat->max_instances;
	manager->supported_flags |= feat->supported_flags;

	if (manager->max_ads == 0)
		return;

	/* Reset existing instances */
	if (feat->num_instances)
		remove_advertising(manager, 0);
}

static void read_controller_cap_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adv_manager *manager = user_data;
	const struct mgmt_rp_read_controller_cap *rp = param;
	const uint8_t *ptr = rp->cap;
	size_t offset = 0;
	uint8_t tag_len;
	uint8_t tag_type;

	if (status || !param) {
		error("Failed to read advertising features: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	if (sizeof(rp->cap_len) + rp->cap_len != length) {
		error("Controller capabilities malformed, size %zu != %u",
				sizeof(rp->cap_len) + rp->cap_len, length);
		return;
	}

	while (offset < rp->cap_len) {
		tag_len = ptr[offset++];
		tag_type = ptr[offset++];

		switch (tag_type) {
		case MGMT_CAP_LE_TX_PWR:
			if ((tag_len - sizeof(tag_type)) !=
					2*sizeof(manager->min_tx_power)) {
				error("TX power had unexpected length %d",
					tag_len);
				break;
			}
			memcpy(&manager->min_tx_power, &ptr[offset], tag_len);
			memcpy(&manager->max_tx_power, &ptr[offset+1], tag_len);
		}

		/* Step to the next entry */
		offset += (tag_len - sizeof(tag_type));
	}
}

static struct btd_adv_manager *manager_create(struct btd_adapter *adapter,
						struct mgmt *mgmt)
{
	struct btd_adv_manager *manager;

	manager = new0(struct btd_adv_manager, 1);
	manager->adapter = adapter;

	manager->mgmt = mgmt_ref(mgmt);

	if (!manager->mgmt) {
		error("Failed to access management interface");
		free(manager);
		return NULL;
	}

	manager->mgmt_index = btd_adapter_get_index(adapter);
	manager->clients = queue_new();
	manager->supported_flags = MGMT_ADV_FLAG_LOCAL_NAME;
	manager->extended_add_cmds =
			btd_has_kernel_features(KERNEL_HAS_EXT_ADV_ADD_CMDS);
	manager->min_tx_power = ADV_TX_POWER_NO_PREFERENCE;
	manager->max_tx_power = ADV_TX_POWER_NO_PREFERENCE;

	if (!g_dbus_register_interface(btd_get_dbus_connection(),
					adapter_get_path(manager->adapter),
					LE_ADVERTISING_MGR_IFACE, methods,
					NULL, properties, manager, NULL)) {
		error("Failed to register " LE_ADVERTISING_MGR_IFACE);
		goto fail;
	}

	if (!mgmt_send(manager->mgmt, MGMT_OP_READ_ADV_FEATURES,
				manager->mgmt_index, 0, NULL,
				read_adv_features_callback, manager, NULL)) {
		error("Failed to read advertising features");
		goto fail;
	}

	/* Query controller capabilities. This will be used to display valid
	 * advertising tx power range to the client.
	 */
	if (g_dbus_get_flags() & G_DBUS_FLAG_ENABLE_EXPERIMENTAL &&
			btd_has_kernel_features(KERNEL_HAS_CONTROLLER_CAP_CMD))
		mgmt_send(manager->mgmt, MGMT_OP_READ_CONTROLLER_CAP,
			manager->mgmt_index, 0, NULL,
			read_controller_cap_complete, manager, NULL);

	return manager;

fail:
	manager_destroy(manager);
	return NULL;
}

struct btd_adv_manager *btd_adv_manager_new(struct btd_adapter *adapter,
							struct mgmt *mgmt)
{
	struct btd_adv_manager *manager;

	if (!adapter || !mgmt)
		return NULL;

	manager = manager_create(adapter, mgmt);
	if (!manager)
		return NULL;

	DBG("LE Advertising Manager created for adapter: %s",
						adapter_get_path(adapter));

	return manager;
}

void btd_adv_manager_destroy(struct btd_adv_manager *manager)
{
	if (!manager)
		return;

	g_dbus_unregister_interface(btd_get_dbus_connection(),
					adapter_get_path(manager->adapter),
					LE_ADVERTISING_MGR_IFACE);

	manager_destroy(manager);
}

static void manager_refresh(void *data, void *user_data)
{
	struct btd_adv_client *client = data;

	refresh_advertisement(client, user_data, NULL);
}

void btd_adv_manager_refresh(struct btd_adv_manager *manager)
{
	if (!manager)
		return;

	queue_foreach(manager->clients, manager_refresh, NULL);
}
