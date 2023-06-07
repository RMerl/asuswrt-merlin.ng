// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2017  Intel Corporation. All rights reserved.
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>
#include <signal.h>

#include <ell/ell.h>

#include "lib/bluetooth.h"
#include "src/shared/btp.h"

#define AD_PATH "/org/bluez/advertising"
#define AG_PATH "/org/bluez/agent"
#define AD_IFACE "org.bluez.LEAdvertisement1"
#define AG_IFACE "org.bluez.Agent1"

/* List of assigned numbers for advetising data and scan response */
#define AD_TYPE_FLAGS				0x01
#define AD_TYPE_INCOMPLETE_UUID16_SERVICE_LIST	0x02
#define AD_TYPE_SHORT_NAME			0x08
#define AD_TYPE_TX_POWER			0x0a
#define AD_TYPE_SOLICIT_UUID16_SERVICE_LIST	0x14
#define AD_TYPE_SERVICE_DATA_UUID16		0x16
#define AD_TYPE_APPEARANCE			0x19
#define AD_TYPE_MANUFACTURER_DATA		0xff

static void register_gap_service(void);

static struct l_dbus *dbus;

struct btp_adapter {
	struct l_dbus_proxy *proxy;
	struct l_dbus_proxy *ad_proxy;
	uint8_t index;
	uint32_t supported_settings;
	uint32_t current_settings;
	uint32_t default_settings;
	struct l_queue *devices;
};

struct btp_device {
	struct l_dbus_proxy *proxy;
	uint8_t address_type;
	bdaddr_t address;
};

static struct l_queue *adapters;
static char *socket_path;
static struct btp *btp;

static bool gap_service_registered;

struct ad_data {
	uint8_t data[25];
	uint8_t len;
};

struct service_data {
	char *uuid;
	struct ad_data data;
};

struct manufacturer_data {
	uint16_t id;
	struct ad_data data;
};

static struct ad {
	bool registered;
	char *type;
	char *local_name;
	uint16_t local_appearance;
	uint16_t duration;
	uint16_t timeout;
	struct l_queue *uuids;
	struct l_queue *services;
	struct l_queue *manufacturers;
	struct l_queue *solicits;
	bool tx_power;
	bool name;
	bool appearance;
} ad;

static struct btp_agent {
	bool registered;
	struct l_dbus_proxy *proxy;
	struct l_dbus_message *pending_req;
} ag;

static char *dupuuid2str(const uint8_t *uuid, uint8_t len)
{
	switch (len) {
	case 16:
		return l_strdup_printf("%hhx%hhx", uuid[0], uuid[1]);
	case 128:
		return l_strdup_printf("%hhx%hhx%hhx%hhx%hhx%hhx%hhx%hhx%hhx"
					"%hhx%hhx%hhx%hhx%hhx%hhx%hhx", uuid[0],
					uuid[1], uuid[2], uuid[3], uuid[4],
					uuid[5], uuid[6], uuid[6], uuid[8],
					uuid[7], uuid[10], uuid[11], uuid[12],
					uuid[13], uuid[14], uuid[15]);
	default:
		return NULL;
	}
}

static bool match_dev_addr_type(const char *addr_type_str, uint8_t addr_type)
{
	if (addr_type == BTP_GAP_ADDR_PUBLIC && strcmp(addr_type_str, "public"))
		return false;

	if (addr_type == BTP_GAP_ADDR_RANDOM && strcmp(addr_type_str, "random"))
		return false;

	return true;
}

static struct btp_adapter *find_adapter_by_proxy(struct l_dbus_proxy *proxy)
{
	const struct l_queue_entry *entry;

	for (entry = l_queue_get_entries(adapters); entry;
							entry = entry->next) {
		struct btp_adapter *adapter = entry->data;

		if (adapter->proxy == proxy)
			return adapter;
	}

	return NULL;
}

static struct btp_adapter *find_adapter_by_index(uint8_t index)
{
	const struct l_queue_entry *entry;

	for (entry = l_queue_get_entries(adapters); entry;
							entry = entry->next) {
		struct btp_adapter *adapter = entry->data;

		if (adapter->index == index)
			return adapter;
	}

	return NULL;
}

static struct btp_adapter *find_adapter_by_path(const char *path)
{
	const struct l_queue_entry *entry;

	for (entry = l_queue_get_entries(adapters); entry;
							entry = entry->next) {
		struct btp_adapter *adapter = entry->data;

		if (!strcmp(l_dbus_proxy_get_path(adapter->proxy), path))
			return adapter;
	}

	return NULL;
}

static struct btp_device *find_device_by_address(struct btp_adapter *adapter,
							const bdaddr_t *addr,
							uint8_t addr_type)
{
	const struct l_queue_entry *entry;
	const char *str;
	char addr_str[18];

	if (!ba2str(addr, addr_str))
		return NULL;

	for (entry = l_queue_get_entries(adapter->devices); entry;
							entry = entry->next) {
		struct btp_device *device = entry->data;

		l_dbus_proxy_get_property(device->proxy, "Address", "s", &str);
		if (strcmp(str, addr_str))
			continue;

		l_dbus_proxy_get_property(device->proxy, "AddressType", "s",
									&str);
		if (match_dev_addr_type(str, addr_type))
			return device;
	}

	return NULL;
}

static bool match_device_paths(const void *device, const void *path)
{
	const struct btp_device *dev = device;

	return !strcmp(l_dbus_proxy_get_path(dev->proxy), path);
}

static struct btp_device *find_device_by_path(const char *path)
{
	const struct l_queue_entry *entry;
	struct btp_device *device;

	for (entry = l_queue_get_entries(adapters); entry;
							entry = entry->next) {
		struct btp_adapter *adapter = entry->data;

		device = l_queue_find(adapter->devices, match_device_paths,
									path);
		if (device)
			return device;
	}

	return NULL;
}

static bool match_adapter_dev_proxy(const void *device, const void *proxy)
{
	const struct btp_device *d = device;

	return d->proxy == proxy;
}

static bool match_adapter_dev(const void *device_a, const void *device_b)
{
	return device_a == device_b;
}

static struct btp_adapter *find_adapter_by_device(struct btp_device *device)
{
	const struct l_queue_entry *entry;

	for (entry = l_queue_get_entries(adapters); entry;
							entry = entry->next) {
		struct btp_adapter *adapter = entry->data;

		if (l_queue_find(adapter->devices, match_adapter_dev, device))
			return adapter;
	}

	return NULL;
}

static struct btp_device *find_device_by_proxy(struct l_dbus_proxy *proxy)
{
	const struct l_queue_entry *entry;
	struct btp_device *device;

	for (entry = l_queue_get_entries(adapters); entry;
							entry = entry->next) {
		struct btp_adapter *adapter = entry->data;

		device = l_queue_find(adapter->devices, match_adapter_dev_proxy,
									proxy);

		if (device)
			return device;
	}

	return NULL;
}

static void btp_gap_read_commands(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	uint16_t commands = 0;

	if (index != BTP_INDEX_NON_CONTROLLER) {
		btp_send_error(btp, BTP_GAP_SERVICE, index,
						BTP_ERROR_INVALID_INDEX);
		return;
	}

	commands |= (1 << BTP_OP_GAP_READ_SUPPORTED_COMMANDS);
	commands |= (1 << BTP_OP_GAP_READ_CONTROLLER_INDEX_LIST);
	commands |= (1 << BTP_OP_GAP_READ_COTROLLER_INFO);
	commands |= (1 << BTP_OP_GAP_RESET);
	commands |= (1 << BTP_OP_GAP_SET_POWERED);
	commands |= (1 << BTP_OP_GAP_SET_CONNECTABLE);
	commands |= (1 << BTP_OP_GAP_SET_DISCOVERABLE);
	commands |= (1 << BTP_OP_GAP_SET_BONDABLE);
	commands |= (1 << BTP_OP_GAP_START_ADVERTISING);
	commands |= (1 << BTP_OP_GAP_STOP_ADVERTISING);
	commands |= (1 << BTP_OP_GAP_START_DISCOVERY);
	commands |= (1 << BTP_OP_GAP_STOP_DISCOVERY);
	commands |= (1 << BTP_OP_GAP_CONNECT);
	commands |= (1 << BTP_OP_GAP_DISCONNECT);
	commands |= (1 << BTP_OP_GAP_SET_IO_CAPA);
	commands |= (1 << BTP_OP_GAP_PAIR);
	commands |= (1 << BTP_OP_GAP_UNPAIR);

	commands = L_CPU_TO_LE16(commands);

	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_READ_SUPPORTED_COMMANDS,
			BTP_INDEX_NON_CONTROLLER, sizeof(commands), &commands);
}

static void btp_gap_read_controller_index(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	const struct l_queue_entry *entry;
	struct btp_gap_read_index_rp *rp;
	uint8_t cnt;
	int i;

	if (index != BTP_INDEX_NON_CONTROLLER) {
		btp_send_error(btp, BTP_GAP_SERVICE, index,
						BTP_ERROR_INVALID_INDEX);
		return;
	}

	cnt = l_queue_length(adapters);

	rp = l_malloc(sizeof(*rp) + cnt);

	rp->num = cnt;

	for (i = 0, entry = l_queue_get_entries(adapters); entry;
						i++, entry = entry->next) {
		struct btp_adapter *adapter = entry->data;

		rp->indexes[i] = adapter->index;
	}

	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_READ_CONTROLLER_INDEX_LIST,
			BTP_INDEX_NON_CONTROLLER, sizeof(*rp) + cnt, rp);
}

static void btp_gap_read_info(uint8_t index, const void *param, uint16_t length,
								void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	struct btp_gap_read_info_rp rp;
	const char *str;
	uint8_t status = BTP_ERROR_FAIL;

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	memset(&rp, 0, sizeof(rp));

	if (!l_dbus_proxy_get_property(adapter->proxy, "Address", "s", &str))
		goto failed;

	if (str2ba(str, &rp.address) < 0)
		goto failed;

	if (!l_dbus_proxy_get_property(adapter->proxy, "Name", "s", &str)) {
		goto failed;
	}

	snprintf((char *)rp.name, sizeof(rp.name), "%s", str);
	snprintf((char *)rp.short_name, sizeof(rp.short_name), "%s", str);
	rp.supported_settings = L_CPU_TO_LE32(adapter->supported_settings);
	rp.current_settings = L_CPU_TO_LE32(adapter->current_settings);

	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_READ_COTROLLER_INFO, index,
							sizeof(rp), &rp);

	return;
failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

static void remove_device_setup(struct l_dbus_message *message,
							void *user_data)
{
	struct btp_device *device = user_data;

	l_dbus_message_set_arguments(message, "o",
					l_dbus_proxy_get_path(device->proxy));
}

static void remove_device_reply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	struct btp_device *device = user_data;
	struct btp_adapter *adapter = find_adapter_by_proxy(proxy);

	if (!adapter)
		return;

	if (l_dbus_message_is_error(result)) {
		const char *name;

		l_dbus_message_get_error(result, &name, NULL);

		l_error("Failed to remove device %s (%s)",
					l_dbus_proxy_get_path(device->proxy),
					name);
		return;
	}

	l_queue_remove(adapter->devices, device);
}

static void unreg_advertising_setup(struct l_dbus_message *message,
								void *user_data)
{
	struct l_dbus_message_builder *builder;

	builder = l_dbus_message_builder_new(message);
	l_dbus_message_builder_append_basic(builder, 'o', AD_PATH);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static void ad_cleanup_service(void *service)
{
	struct service_data *s = service;

	l_free(s->uuid);
	l_free(s);
}

static void ad_cleanup(void)
{
	l_free(ad.local_name);
	l_queue_destroy(ad.uuids, l_free);
	l_queue_destroy(ad.services, ad_cleanup_service);
	l_queue_destroy(ad.manufacturers, l_free);
	l_queue_destroy(ad.solicits, l_free);

	memset(&ad, 0, sizeof(ad));
}

static void unreg_advertising_reply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	const char *path = l_dbus_proxy_get_path(proxy);
	struct btp_adapter *adapter = find_adapter_by_path(path);

	if (!adapter)
		return;

	if (l_dbus_message_is_error(result)) {
		const char *name;

		l_dbus_message_get_error(result, &name, NULL);

		l_error("Failed to stop advertising %s (%s)",
					l_dbus_proxy_get_path(proxy), name);
		return;
	}

	if (!l_dbus_object_remove_interface(dbus, AD_PATH, AD_IFACE))
		l_info("Unable to remove ad instance");
	if (!l_dbus_object_remove_interface(dbus, AD_PATH,
						L_DBUS_INTERFACE_PROPERTIES))
		l_info("Unable to remove propety instance");
	if (!l_dbus_unregister_interface(dbus, AD_IFACE))
		l_info("Unable to unregister ad interface");

	ad_cleanup();
}

static void unreg_agent_setup(struct l_dbus_message *message, void *user_data)
{
	struct l_dbus_message_builder *builder;

	builder = l_dbus_message_builder_new(message);

	l_dbus_message_builder_append_basic(builder, 'o', AG_PATH);

	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static void reset_unreg_agent_reply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	if (l_dbus_message_is_error(result)) {
		const char *name;

		l_dbus_message_get_error(result, &name, NULL);

		l_error("Failed to unregister agent %s (%s)",
					l_dbus_proxy_get_path(proxy), name);
		return;
	}

	if (!l_dbus_object_remove_interface(dbus, AG_PATH,
						L_DBUS_INTERFACE_PROPERTIES))
		l_info("Unable to remove propety instance");
	if (!l_dbus_object_remove_interface(dbus, AG_PATH, AG_IFACE))
		l_info("Unable to remove agent instance");
	if (!l_dbus_unregister_interface(dbus, AG_IFACE))
		l_info("Unable to unregister agent interface");

	ag.registered = false;
}

static void update_current_settings(struct btp_adapter *adapter,
							uint32_t new_settings)
{
	struct btp_new_settings_ev ev;

	adapter->current_settings = new_settings;

	ev.current_settings = L_CPU_TO_LE32(adapter->current_settings);

	btp_send(btp, BTP_GAP_SERVICE, BTP_EV_GAP_NEW_SETTINGS, adapter->index,
							sizeof(ev), &ev);
}

static void btp_gap_reset(uint8_t index, const void *param, uint16_t length,
								void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	const struct l_queue_entry *entry;
	uint8_t status;
	bool prop;
	uint32_t default_settings;

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	/* Adapter needs to be powered to be able to remove devices */
	if (!l_dbus_proxy_get_property(adapter->proxy, "Powered", "b", &prop) ||
									!prop) {
		status = BTP_ERROR_FAIL;
		goto failed;
	}

	for (entry = l_queue_get_entries(adapter->devices); entry;
							entry = entry->next) {
		struct btp_device *device = entry->data;

		l_dbus_proxy_method_call(adapter->proxy, "RemoveDevice",
						remove_device_setup,
						remove_device_reply, device,
						NULL);
	}

	if (adapter->ad_proxy && ad.registered)
		if (!l_dbus_proxy_method_call(adapter->ad_proxy,
						"UnregisterAdvertisement",
						unreg_advertising_setup,
						unreg_advertising_reply,
						NULL, NULL)) {
			status = BTP_ERROR_FAIL;
			goto failed;
		}

	if (ag.proxy && ag.registered)
		if (!l_dbus_proxy_method_call(ag.proxy, "UnregisterAgent",
						unreg_agent_setup,
						reset_unreg_agent_reply,
						NULL, NULL)) {
			status = BTP_ERROR_FAIL;
			goto failed;
		}

	default_settings = adapter->default_settings;

	update_current_settings(adapter, default_settings);

	/* TODO for we assume all went well */
	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_RESET, index,
				sizeof(default_settings), &default_settings);
	return;

failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

struct set_setting_data {
	struct btp_adapter *adapter;
	uint8_t opcode;
	uint32_t setting;
	bool value;
};

static void set_setting_reply(struct l_dbus_proxy *proxy,
				struct l_dbus_message *result, void *user_data)
{
	struct set_setting_data *data = user_data;
	struct btp_adapter *adapter = data->adapter;
	uint32_t settings;

	if (l_dbus_message_is_error(result)) {
		btp_send_error(btp, BTP_GAP_SERVICE, data->adapter->index,
								BTP_ERROR_FAIL);
		return;
	}

	if (data->value)
		adapter->current_settings |= data->setting;
	else
		adapter->current_settings &= ~data->setting;

	settings = L_CPU_TO_LE32(adapter->current_settings);

	btp_send(btp, BTP_GAP_SERVICE, data->opcode, adapter->index,
						sizeof(settings), &settings);
}

static void btp_gap_set_powered(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	const struct btp_gap_set_powered_cp *cp = param;
	uint8_t status = BTP_ERROR_FAIL;
	struct set_setting_data *data;

	if (length < sizeof(*cp))
		goto failed;

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	data = l_new(struct set_setting_data, 1);
	data->adapter = adapter;
	data->opcode = BTP_OP_GAP_SET_POWERED;
	data->setting = BTP_GAP_SETTING_POWERED;
	data->value = cp->powered;

	if (l_dbus_proxy_set_property(adapter->proxy, set_setting_reply,
					data, l_free, "Powered", "b",
					data->value))
		return;

	l_free(data);

failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

static void btp_gap_set_connectable(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	const struct btp_gap_set_connectable_cp *cp = param;
	uint8_t status = BTP_ERROR_FAIL;
	uint32_t new_settings;

	if (length < sizeof(*cp))
		goto failed;

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	new_settings = adapter->current_settings;

	if (cp->connectable)
		new_settings |= BTP_GAP_SETTING_CONNECTABLE;
	else
		new_settings &= ~BTP_GAP_SETTING_CONNECTABLE;

	update_current_settings(adapter, new_settings);

	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_SET_CONNECTABLE, index,
					sizeof(new_settings), &new_settings);

	return;

failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

static void btp_gap_set_discoverable(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	const struct btp_gap_set_discoverable_cp *cp = param;
	uint8_t status = BTP_ERROR_FAIL;
	struct set_setting_data *data;

	if (length < sizeof(*cp))
		goto failed;

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	data = l_new(struct set_setting_data, 1);
	data->adapter = adapter;
	data->opcode = BTP_OP_GAP_SET_DISCOVERABLE;
	data->setting = BTP_GAP_SETTING_DISCOVERABLE;
	data->value = cp->discoverable;

	if (l_dbus_proxy_set_property(adapter->proxy, set_setting_reply,
					data, l_free, "Discoverable", "b",
					data->value))
		return;

	l_free(data);

failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

static void btp_gap_set_bondable(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	const struct btp_gap_set_bondable_cp *cp = param;
	uint8_t status = BTP_ERROR_FAIL;
	struct set_setting_data *data;

	if (length < sizeof(*cp))
		goto failed;

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	data = l_new(struct set_setting_data, 1);
	data->adapter = adapter;
	data->opcode = BTP_OP_GAP_SET_BONDABLE;
	data->setting = BTP_GAP_SETTING_BONDABLE;
	data->value = cp->bondable;

	if (l_dbus_proxy_set_property(adapter->proxy, set_setting_reply,
					data, l_free, "Pairable", "b",
					data->value))
		return;

	l_free(data);

failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

static void ad_init(void)
{
	ad.uuids = l_queue_new();
	ad.services = l_queue_new();
	ad.manufacturers = l_queue_new();
	ad.solicits = l_queue_new();

	ad.local_appearance = UINT16_MAX;
}

static struct l_dbus_message *ad_release_call(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_message *reply;

	l_dbus_unregister_object(dbus, AD_PATH);
	l_dbus_unregister_interface(dbus, AD_IFACE);

	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");

	ad_cleanup();

	return reply;
}

static bool ad_type_getter(struct l_dbus *dbus, struct l_dbus_message *message,
				struct l_dbus_message_builder *builder,
				void *user_data)
{
	l_dbus_message_builder_append_basic(builder, 's', ad.type);

	return true;
}

static bool ad_serviceuuids_getter(struct l_dbus *dbus,
					struct l_dbus_message *message,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	const struct l_queue_entry *entry;

	if (l_queue_isempty(ad.uuids))
		return false;

	l_dbus_message_builder_enter_array(builder, "s");

	for (entry = l_queue_get_entries(ad.uuids); entry; entry = entry->next)
		l_dbus_message_builder_append_basic(builder, 's', entry->data);

	l_dbus_message_builder_leave_array(builder);

	return true;
}

static bool ad_servicedata_getter(struct l_dbus *dbus,
					struct l_dbus_message *message,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	const struct l_queue_entry *entry;
	size_t i;

	if (l_queue_isempty(ad.services))
		return false;

	l_dbus_message_builder_enter_array(builder, "{sv}");

	for (entry = l_queue_get_entries(ad.services); entry;
							entry = entry->next) {
		struct service_data *sd = entry->data;

		l_dbus_message_builder_enter_dict(builder, "sv");
		l_dbus_message_builder_append_basic(builder, 's', sd->uuid);
		l_dbus_message_builder_enter_variant(builder, "ay");
		l_dbus_message_builder_enter_array(builder, "y");

		for (i = 0; i < sd->data.len; i++)
			l_dbus_message_builder_append_basic(builder, 'y',
							&(sd->data.data[i]));

		l_dbus_message_builder_leave_array(builder);
		l_dbus_message_builder_leave_variant(builder);
		l_dbus_message_builder_leave_dict(builder);
	}
	l_dbus_message_builder_leave_array(builder);

	return true;
}

static bool ad_manufacturerdata_getter(struct l_dbus *dbus,
					struct l_dbus_message *message,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	const struct l_queue_entry *entry;
	size_t i;

	if (l_queue_isempty(ad.manufacturers))
		return false;

	l_dbus_message_builder_enter_array(builder, "{qv}");

	for (entry = l_queue_get_entries(ad.manufacturers); entry;
							entry = entry->next) {
		struct manufacturer_data *md = entry->data;

		l_dbus_message_builder_enter_dict(builder, "qv");
		l_dbus_message_builder_append_basic(builder, 'q', &md->id);
		l_dbus_message_builder_enter_variant(builder, "ay");
		l_dbus_message_builder_enter_array(builder, "y");

		for (i = 0; i < md->data.len; i++)
			l_dbus_message_builder_append_basic(builder, 'y',
							&(md->data.data[i]));

		l_dbus_message_builder_leave_array(builder);
		l_dbus_message_builder_leave_variant(builder);
		l_dbus_message_builder_leave_dict(builder);
	}
	l_dbus_message_builder_leave_array(builder);

	return true;
}

static bool ad_solicituuids_getter(struct l_dbus *dbus,
					struct l_dbus_message *message,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	const struct l_queue_entry *entry;

	if (l_queue_isempty(ad.solicits))
		return false;

	l_dbus_message_builder_enter_array(builder, "s");

	for (entry = l_queue_get_entries(ad.solicits); entry;
							entry = entry->next)
		l_dbus_message_builder_append_basic(builder, 's', entry->data);

	l_dbus_message_builder_leave_array(builder);

	return true;
}

static bool ad_includes_getter(struct l_dbus *dbus,
					struct l_dbus_message *message,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	l_dbus_message_builder_enter_array(builder, "s");

	if (!(ad.tx_power || ad.name || ad.appearance))
		return false;

	if (ad.tx_power) {
		const char *str = "tx-power";

		l_dbus_message_builder_append_basic(builder, 's', str);
	}

	if (ad.name) {
		const char *str = "local-name";

		l_dbus_message_builder_append_basic(builder, 's', str);
	}

	if (ad.appearance) {
		const char *str = "appearance";

		l_dbus_message_builder_append_basic(builder, 's', str);
	}

	l_dbus_message_builder_leave_array(builder);

	return true;
}

static bool ad_localname_getter(struct l_dbus *dbus,
					struct l_dbus_message *message,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	if (!ad.local_name)
		return false;

	l_dbus_message_builder_append_basic(builder, 's', ad.local_name);

	return true;
}

static bool ad_appearance_getter(struct l_dbus *dbus,
					struct l_dbus_message *message,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	if (!ad.local_appearance)
		return false;

	l_dbus_message_builder_append_basic(builder, 'q', &ad.local_appearance);

	return true;
}

static bool ad_duration_getter(struct l_dbus *dbus,
					struct l_dbus_message *message,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	if (!ad.duration)
		return false;

	l_dbus_message_builder_append_basic(builder, 'q', &ad.duration);

	return true;
}

static bool ad_timeout_getter(struct l_dbus *dbus,
					struct l_dbus_message *message,
					struct l_dbus_message_builder *builder,
					void *user_data)
{
	if (!ad.timeout)
		return false;

	l_dbus_message_builder_append_basic(builder, 'q', &ad.timeout);

	return true;
}

static void setup_ad_interface(struct l_dbus_interface *interface)
{
	l_dbus_interface_method(interface, "Release",
						L_DBUS_METHOD_FLAG_NOREPLY,
						ad_release_call, "", "");
	l_dbus_interface_property(interface, "Type", 0, "s", ad_type_getter,
									NULL);
	l_dbus_interface_property(interface, "ServiceUUIDs", 0, "as",
						ad_serviceuuids_getter, NULL);
	l_dbus_interface_property(interface, "ServiceData", 0, "a{sv}",
						ad_servicedata_getter, NULL);
	l_dbus_interface_property(interface, "ManufacturerData", 0,
					"a{qv}", ad_manufacturerdata_getter,
					NULL);
	l_dbus_interface_property(interface, "SolicitUUIDs", 0, "as",
						ad_solicituuids_getter, NULL);
	l_dbus_interface_property(interface, "Includes", 0, "as",
						ad_includes_getter, NULL);
	l_dbus_interface_property(interface, "LocalName", 0, "s",
						ad_localname_getter, NULL);
	l_dbus_interface_property(interface, "Appearance", 0, "q",
						ad_appearance_getter, NULL);
	l_dbus_interface_property(interface, "Duration", 0, "q",
						ad_duration_getter, NULL);
	l_dbus_interface_property(interface, "Timeout", 0, "q",
						ad_timeout_getter, NULL);
}

static void start_advertising_reply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	const char *path = l_dbus_proxy_get_path(proxy);
	struct btp_adapter *adapter = find_adapter_by_path(path);
	uint32_t new_settings;

	if (!adapter) {
		btp_send_error(btp, BTP_GAP_SERVICE, BTP_INDEX_NON_CONTROLLER,
								BTP_ERROR_FAIL);
		return;
	}

	if (l_dbus_message_is_error(result)) {
		const char *name, *desc;

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to start advertising (%s), %s", name, desc);

		btp_send_error(btp, BTP_GAP_SERVICE, adapter->index,
								BTP_ERROR_FAIL);
		return;
	}

	new_settings = adapter->current_settings;
	new_settings |= BTP_GAP_SETTING_ADVERTISING;
	update_current_settings(adapter, new_settings);

	ad.registered = true;

	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_START_ADVERTISING,
					adapter->index, sizeof(new_settings),
					&new_settings);
}

static void create_advertising_data(uint8_t adv_data_len, const uint8_t *data)
{
	const uint8_t *ad_data;
	uint8_t ad_type, ad_len;
	uint8_t remaining_data_len = adv_data_len;

	while (remaining_data_len) {
		ad_type = data[adv_data_len - remaining_data_len];
		ad_len = data[adv_data_len - remaining_data_len + 1];
		ad_data = &data[adv_data_len - remaining_data_len + 2];

		switch (ad_type) {
		case AD_TYPE_INCOMPLETE_UUID16_SERVICE_LIST:
		{
			char *uuid = dupuuid2str(ad_data, 16);

			l_queue_push_tail(ad.uuids, uuid);

			break;
		}
		case AD_TYPE_SHORT_NAME:
			ad.local_name = malloc(ad_len + 1);
			memcpy(ad.local_name, ad_data, ad_len);
			ad.local_name[ad_len] = '\0';

			break;
		case AD_TYPE_TX_POWER:
			ad.tx_power = true;

			/* XXX Value is ommited cause, stack fills it */

			break;
		case AD_TYPE_SERVICE_DATA_UUID16:
		{
			struct service_data *sd;

			sd = l_new(struct service_data, 1);
			sd->uuid = dupuuid2str(ad_data, 16);
			sd->data.len = ad_len - 2;
			memcpy(sd->data.data, ad_data + 2, sd->data.len);

			l_queue_push_tail(ad.services, sd);

			break;
		}
		case AD_TYPE_APPEARANCE:
			memcpy(&ad.local_appearance, ad_data, ad_len);

			break;
		case AD_TYPE_MANUFACTURER_DATA:
		{
			struct manufacturer_data *md;

			md = l_new(struct manufacturer_data, 1);
			/* The first 2 octets contain the Company Identifier
			 * Code followed by additional manufacturer specific
			 * data.
			 */
			memcpy(&md->id, ad_data, 2);
			md->data.len = ad_len - 2;
			memcpy(md->data.data, ad_data + 2, md->data.len);

			l_queue_push_tail(ad.manufacturers, md);

			break;
		}
		case AD_TYPE_SOLICIT_UUID16_SERVICE_LIST:
		{
			char *uuid = dupuuid2str(ad_data, 16);

			l_queue_push_tail(ad.solicits, uuid);

			break;
		}
		default:
			l_info("Unsupported advertising data type");

			break;
		}
		/* Advertising entity data len + advertising entity header
		 * (type, len)
		 */
		remaining_data_len -= ad_len + 2;
	}
}

static void create_scan_response(uint8_t scan_rsp_len, const uint8_t *data)
{
	/* TODO */
}

static void start_advertising_setup(struct l_dbus_message *message,
							void *user_data)
{
	struct l_dbus_message_builder *builder;

	builder = l_dbus_message_builder_new(message);
	l_dbus_message_builder_append_basic(builder, 'o', AD_PATH);
	l_dbus_message_builder_enter_array(builder, "{sv}");
	l_dbus_message_builder_enter_dict(builder, "sv");
	l_dbus_message_builder_leave_dict(builder);
	l_dbus_message_builder_leave_array(builder);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static void btp_gap_start_advertising(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	const struct btp_gap_start_adv_cp *cp = param;
	uint8_t status = BTP_ERROR_FAIL;
	bool prop;

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	/* Adapter needs to be powered to be able to advertise */
	if (!l_dbus_proxy_get_property(adapter->proxy, "Powered", "b", &prop) ||
							!prop || ad.registered)
		goto failed;

	if (!l_dbus_register_interface(dbus, AD_IFACE, setup_ad_interface, NULL,
								false)) {
		l_info("Unable to register ad interface");
		goto failed;
	}

	if (!l_dbus_object_add_interface(dbus, AD_PATH, AD_IFACE, NULL)) {
		l_info("Unable to instantiate ad interface");

		if (!l_dbus_unregister_interface(dbus, AD_IFACE))
			l_info("Unable to unregister ad interface");

		goto failed;
	}

	if (!l_dbus_object_add_interface(dbus, AD_PATH,
						L_DBUS_INTERFACE_PROPERTIES,
						NULL)) {
		l_info("Unable to instantiate the properties interface");

		if (!l_dbus_object_remove_interface(dbus, AD_PATH, AD_IFACE))
			l_info("Unable to remove ad instance");
		if (!l_dbus_unregister_interface(dbus, AD_IFACE))
			l_info("Unable to unregister ad interface");

		goto failed;
	}

	ad_init();

	if (adapter->current_settings & BTP_GAP_SETTING_CONNECTABLE)
		ad.type = "peripheral";
	else
		ad.type = "broadcast";

	if (cp->adv_data_len > 0)
		create_advertising_data(cp->adv_data_len, cp->data);
	if (cp->scan_rsp_len > 0)
		create_scan_response(cp->scan_rsp_len,
						cp->data + cp->scan_rsp_len);

	if (!l_dbus_proxy_method_call(adapter->ad_proxy,
							"RegisterAdvertisement",
							start_advertising_setup,
							start_advertising_reply,
							NULL, NULL)) {
		if (!l_dbus_object_remove_interface(dbus, AD_PATH, AD_IFACE))
			l_info("Unable to remove ad instance");
		if (!l_dbus_unregister_interface(dbus, AD_IFACE))
			l_info("Unable to unregister ad interface");

		goto failed;
	}

	return;

failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

static void stop_advertising_reply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	const char *path = l_dbus_proxy_get_path(proxy);
	struct btp_adapter *adapter = find_adapter_by_path(path);
	uint32_t new_settings;

	if (!adapter)
		return;

	if (l_dbus_message_is_error(result)) {
		const char *name;

		l_dbus_message_get_error(result, &name, NULL);

		l_error("Failed to stop advertising %s (%s)",
					l_dbus_proxy_get_path(proxy), name);
		return;
	}

	if (!l_dbus_object_remove_interface(dbus, AD_PATH, AD_IFACE))
		l_info("Unable to remove ad instance");
	if (!l_dbus_object_remove_interface(dbus, AD_PATH,
						L_DBUS_INTERFACE_PROPERTIES))
		l_info("Unable to remove propety instance");
	if (!l_dbus_unregister_interface(dbus, AD_IFACE))
		l_info("Unable to unregister ad interface");

	new_settings = adapter->current_settings;
	new_settings &= ~BTP_GAP_SETTING_ADVERTISING;
	update_current_settings(adapter, new_settings);

	ad_cleanup();

	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_STOP_ADVERTISING,
					adapter->index, sizeof(new_settings),
					&new_settings);
}

static void btp_gap_stop_advertising(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	uint8_t status = BTP_ERROR_FAIL;
	bool prop;

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	if (!l_dbus_proxy_get_property(adapter->proxy, "Powered", "b", &prop) ||
				!prop || !adapter->ad_proxy || !ad.registered)
		goto failed;

	if (!l_dbus_proxy_method_call(adapter->ad_proxy,
						"UnregisterAdvertisement",
						unreg_advertising_setup,
						stop_advertising_reply,
						NULL, NULL))
		goto failed;

	return;

failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

static void start_discovery_reply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_proxy(proxy);

	if (!adapter) {
		btp_send_error(btp, BTP_GAP_SERVICE, BTP_INDEX_NON_CONTROLLER,
								BTP_ERROR_FAIL);
		return;
	}

	if (l_dbus_message_is_error(result)) {
		const char *name, *desc;

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to start discovery (%s), %s", name, desc);

		btp_send_error(btp, BTP_GAP_SERVICE, adapter->index,
								BTP_ERROR_FAIL);
		return;
	}

	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_START_DISCOVERY,
						adapter->index, 0, NULL);
}

static void set_discovery_filter_setup(struct l_dbus_message *message,
							void *user_data)
{
	uint8_t flags = L_PTR_TO_UINT(user_data);
	struct l_dbus_message_builder *builder;

	if (!(flags & (BTP_GAP_DISCOVERY_FLAG_LE |
					BTP_GAP_DISCOVERY_FLAG_BREDR))) {
		l_info("Failed to start discovery - no transport set");
		return;
	}

	builder = l_dbus_message_builder_new(message);

	l_dbus_message_builder_enter_array(builder, "{sv}");
	l_dbus_message_builder_enter_dict(builder, "sv");

	/* Be in observer mode or in general mode (default in Bluez) */
	if (flags & BTP_GAP_DISCOVERY_FLAG_OBSERVATION) {
		l_dbus_message_builder_append_basic(builder, 's', "Transport");
		l_dbus_message_builder_enter_variant(builder, "s");

		if (flags & (BTP_GAP_DISCOVERY_FLAG_LE |
						BTP_GAP_DISCOVERY_FLAG_BREDR))
			l_dbus_message_builder_append_basic(builder, 's',
									"auto");
		else if (flags & BTP_GAP_DISCOVERY_FLAG_LE)
			l_dbus_message_builder_append_basic(builder, 's', "le");
		else if (flags & BTP_GAP_DISCOVERY_FLAG_BREDR)
			l_dbus_message_builder_append_basic(builder, 's',
								"bredr");

		l_dbus_message_builder_leave_variant(builder);
	}

	l_dbus_message_builder_leave_dict(builder);
	l_dbus_message_builder_leave_array(builder);

	/* TODO add passive, limited discovery */
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static void set_discovery_filter_reply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_proxy(proxy);

	if (!adapter) {
		btp_send_error(btp, BTP_GAP_SERVICE, BTP_INDEX_NON_CONTROLLER,
								BTP_ERROR_FAIL);
		return;
	}

	if (l_dbus_message_is_error(result)) {
		const char *name, *desc;

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to set discovery filter (%s), %s", name, desc);

		btp_send_error(btp, BTP_GAP_SERVICE, adapter->index,
								BTP_ERROR_FAIL);
		return;
	}

	l_dbus_proxy_method_call(adapter->proxy, "StartDiscovery", NULL,
					start_discovery_reply, NULL, NULL);
}

static void btp_gap_start_discovery(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	const struct btp_gap_start_discovery_cp *cp = param;
	bool prop;

	if (!adapter) {
		btp_send_error(btp, BTP_GAP_SERVICE, index,
						BTP_ERROR_INVALID_INDEX);
		return;
	}

	/* Adapter needs to be powered to start discovery */
	if (!l_dbus_proxy_get_property(adapter->proxy, "Powered", "b", &prop) ||
									!prop) {
		btp_send_error(btp, BTP_GAP_SERVICE, index, BTP_ERROR_FAIL);
		return;
	}

	l_dbus_proxy_method_call(adapter->proxy, "SetDiscoveryFilter",
						set_discovery_filter_setup,
						set_discovery_filter_reply,
						L_UINT_TO_PTR(cp->flags), NULL);
}

static void clear_discovery_filter_setup(struct l_dbus_message *message,
							void *user_data)
{
	struct l_dbus_message_builder *builder;

	builder = l_dbus_message_builder_new(message);

	/* Clear discovery filter setup */
	l_dbus_message_builder_enter_array(builder, "{sv}");
	l_dbus_message_builder_enter_dict(builder, "sv");
	l_dbus_message_builder_leave_dict(builder);
	l_dbus_message_builder_leave_array(builder);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static void clear_discovery_filter_reaply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_proxy(proxy);

	if (!adapter) {
		btp_send_error(btp, BTP_GAP_SERVICE, BTP_INDEX_NON_CONTROLLER,
								BTP_ERROR_FAIL);
		return;
	}

	if (l_dbus_message_is_error(result)) {
		const char *name, *desc;

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to set discovery filter (%s), %s", name, desc);

		btp_send_error(btp, BTP_GAP_SERVICE, adapter->index,
								BTP_ERROR_FAIL);
		return;
	}

	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_STOP_DISCOVERY,
						adapter->index, 0, NULL);
}

static void stop_discovery_reply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_proxy(proxy);

	if (!adapter) {
		btp_send_error(btp, BTP_GAP_SERVICE, BTP_INDEX_NON_CONTROLLER,
								BTP_ERROR_FAIL);
		return;
	}

	if (l_dbus_message_is_error(result)) {
		const char *name;

		l_dbus_message_get_error(result, &name, NULL);
		l_error("Failed to stop discovery (%s)", name);

		btp_send_error(btp, BTP_GAP_SERVICE, adapter->index,
								BTP_ERROR_FAIL);
		return;
	}

	l_dbus_proxy_method_call(adapter->proxy, "SetDiscoveryFilter",
						clear_discovery_filter_setup,
						clear_discovery_filter_reaply,
						NULL, NULL);
}

static void btp_gap_stop_discovery(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	bool prop;

	if (!adapter) {
		btp_send_error(btp, BTP_GAP_SERVICE, index,
						BTP_ERROR_INVALID_INDEX);
		return;
	}

	/* Adapter needs to be powered to be able to remove devices */
	if (!l_dbus_proxy_get_property(adapter->proxy, "Powered", "b", &prop) ||
									!prop) {
		btp_send_error(btp, BTP_GAP_SERVICE, index, BTP_ERROR_FAIL);
		return;
	}

	l_dbus_proxy_method_call(adapter->proxy, "StopDiscovery", NULL,
					stop_discovery_reply, NULL, NULL);
}

static void connect_reply(struct l_dbus_proxy *proxy,
				struct l_dbus_message *result, void *user_data)
{
	uint8_t adapter_index = L_PTR_TO_UINT(user_data);
	struct btp_adapter *adapter = find_adapter_by_index(adapter_index);

	if (!adapter) {
		btp_send_error(btp, BTP_GAP_SERVICE, BTP_INDEX_NON_CONTROLLER,
								BTP_ERROR_FAIL);
		return;
	}

	if (l_dbus_message_is_error(result)) {
		const char *name, *desc;

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to connect (%s), %s", name, desc);

		btp_send_error(btp, BTP_GAP_SERVICE, adapter_index,
								BTP_ERROR_FAIL);
		return;
	}

	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_CONNECT, adapter_index, 0,
									NULL);
}

struct connect_device_data {
	bdaddr_t addr;
	uint8_t addr_type;
};

static void connect_device_destroy(void *connect_device_data)
{
	l_free(connect_device_data);
}

static void connect_device_reply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_proxy(proxy);

	if (!adapter) {
		btp_send_error(btp, BTP_GAP_SERVICE, BTP_INDEX_NON_CONTROLLER,
								BTP_ERROR_FAIL);
		return;
	}

	if (l_dbus_message_is_error(result)) {
		const char *name, *desc;

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to connect device (%s), %s", name, desc);

		return;
	}
}

static void connect_device_setup(struct l_dbus_message *message,
								void *user_data)
{
	struct connect_device_data *cdd = user_data;
	struct l_dbus_message_builder *builder;
	char str_addr[18];

	ba2str(&cdd->addr, str_addr);

	builder = l_dbus_message_builder_new(message);

	l_dbus_message_builder_enter_array(builder, "{sv}");

	l_dbus_message_builder_enter_dict(builder, "sv");
	l_dbus_message_builder_append_basic(builder, 's', "Address");
	l_dbus_message_builder_enter_variant(builder, "s");
	l_dbus_message_builder_append_basic(builder, 's', str_addr);
	l_dbus_message_builder_leave_variant(builder);
	l_dbus_message_builder_leave_dict(builder);

	l_dbus_message_builder_enter_dict(builder, "sv");
	l_dbus_message_builder_append_basic(builder, 's', "AddressType");
	l_dbus_message_builder_enter_variant(builder, "s");
	if (cdd->addr_type == BTP_GAP_ADDR_RANDOM)
		l_dbus_message_builder_append_basic(builder, 's', "random");
	else
		l_dbus_message_builder_append_basic(builder, 's', "public");
	l_dbus_message_builder_leave_variant(builder);
	l_dbus_message_builder_leave_dict(builder);

	l_dbus_message_builder_leave_array(builder);

	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static void btp_gap_connect(uint8_t index, const void *param, uint16_t length,
								void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	const struct btp_gap_connect_cp *cp = param;
	struct btp_device *device;
	struct connect_device_data *cdd;
	bool prop;
	uint8_t status = BTP_ERROR_FAIL;

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	/* Adapter needs to be powered to be able to connect */
	if (!l_dbus_proxy_get_property(adapter->proxy, "Powered", "b", &prop) ||
									!prop)
		goto failed;

	device = find_device_by_address(adapter, &cp->address,
							cp->address_type);

	if (!device) {
		cdd = l_new(struct connect_device_data, 1);
		memcpy(&cdd->addr, &cp->address, sizeof(cdd->addr));
		cdd->addr_type = cp->address_type;

		l_dbus_proxy_method_call(adapter->proxy, "ConnectDevice",
							connect_device_setup,
							connect_device_reply,
							cdd,
							connect_device_destroy);

		btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_CONNECT,
						adapter->index, 0, NULL);
		return;
	}

	l_dbus_proxy_method_call(device->proxy, "Connect", NULL, connect_reply,
					L_UINT_TO_PTR(adapter->index), NULL);

	return;

failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

static void disconnect_reply(struct l_dbus_proxy *proxy,
				struct l_dbus_message *result, void *user_data)
{
	uint8_t adapter_index = L_PTR_TO_UINT(user_data);
	struct btp_adapter *adapter = find_adapter_by_index(adapter_index);

	if (!adapter) {
		btp_send_error(btp, BTP_GAP_SERVICE, BTP_INDEX_NON_CONTROLLER,
								BTP_ERROR_FAIL);
		return;
	}

	if (l_dbus_message_is_error(result)) {
		const char *name, *desc;

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to disconnect (%s), %s", name, desc);

		btp_send_error(btp, BTP_GAP_SERVICE, adapter_index,
								BTP_ERROR_FAIL);
		return;
	}

	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_DISCONNECT, adapter_index, 0,
									NULL);
}

static void btp_gap_disconnect(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	const struct btp_gap_disconnect_cp *cp = param;
	uint8_t status = BTP_ERROR_FAIL;
	struct btp_device *device;
	bool prop;

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	/* Adapter needs to be powered to be able to connect */
	if (!l_dbus_proxy_get_property(adapter->proxy, "Powered", "b", &prop) ||
									!prop)
		goto failed;

	device = find_device_by_address(adapter, &cp->address,
							cp->address_type);

	if (!device)
		goto failed;

	l_dbus_proxy_method_call(device->proxy, "Disconnect", NULL,
					disconnect_reply,
					L_UINT_TO_PTR(adapter->index), NULL);

	return;

failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

static struct l_dbus_message *ag_release_call(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_message *reply;

	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");

	return reply;
}

static struct l_dbus_message *ag_request_passkey_call(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct btp_gap_passkey_req_ev ev;
	struct btp_device *device;
	struct btp_adapter *adapter;
	const char *path, *str_addr, *str_addr_type;

	l_dbus_message_get_arguments(message, "o", &path);

	device = find_device_by_path(path);

	if (!l_dbus_proxy_get_property(device->proxy, "Address", "s", &str_addr)
		|| !l_dbus_proxy_get_property(device->proxy, "AddressType", "s",
		&str_addr_type)) {
		l_info("Cannot get device properties");

		return NULL;
	}

	ev.address_type = strcmp(str_addr_type, "public") ?
							BTP_GAP_ADDR_RANDOM :
							BTP_GAP_ADDR_PUBLIC;
	if (!str2ba(str_addr, &ev.address))
		return NULL;

	adapter = find_adapter_by_device(device);

	ag.pending_req = l_dbus_message_ref(message);

	btp_send(btp, BTP_GAP_SERVICE, BTP_EV_GAP_PASSKEY_REQUEST,
					adapter->index, sizeof(ev), &ev);

	return NULL;
}

static struct l_dbus_message *ag_display_passkey_call(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct btp_gap_passkey_display_ev ev;
	struct btp_device *device;
	struct btp_adapter *adapter;
	struct l_dbus_message *reply;
	const char *path, *str_addr, *str_addr_type;
	uint32_t passkey;
	uint16_t entered;

	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");

	l_dbus_message_get_arguments(message, "ouq", &path, &passkey, &entered);

	device = find_device_by_path(path);

	if (!l_dbus_proxy_get_property(device->proxy, "Address", "s", &str_addr)
		|| !l_dbus_proxy_get_property(device->proxy, "AddressType", "s",
		&str_addr_type)) {
		l_info("Cannot get device properties");

		return reply;
	}

	ev.passkey = L_CPU_TO_LE32(passkey);
	ev.address_type = strcmp(str_addr_type, "public") ?
							BTP_GAP_ADDR_RANDOM :
							BTP_GAP_ADDR_PUBLIC;
	if (str2ba(str_addr, &ev.address) < 0) {
		l_info("Incorrect device addres");

		return reply;
	}

	adapter = find_adapter_by_device(device);

	btp_send(btp, BTP_GAP_SERVICE, BTP_EV_GAP_PASSKEY_DISPLAY,
					adapter->index, sizeof(ev), &ev);

	return reply;
}

static struct l_dbus_message *ag_request_confirmation_call(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct btp_gap_passkey_confirm_ev ev;
	struct btp_device *device;
	struct btp_adapter *adapter;
	const char *path, *str_addr, *str_addr_type;
	uint32_t passkey;

	l_dbus_message_get_arguments(message, "ou", &path, &passkey);

	device = find_device_by_path(path);

	if (!l_dbus_proxy_get_property(device->proxy, "Address", "s", &str_addr)
		|| !l_dbus_proxy_get_property(device->proxy, "AddressType", "s",
		&str_addr_type)) {
		l_info("Cannot get device properties");

		return NULL;
	}

	ev.passkey = L_CPU_TO_LE32(passkey);
	ev.address_type = strcmp(str_addr_type, "public") ?
							BTP_GAP_ADDR_RANDOM :
							BTP_GAP_ADDR_PUBLIC;
	if (str2ba(str_addr, &ev.address) < 0) {
		l_info("Incorrect device address");

		return NULL;
	}

	adapter = find_adapter_by_device(device);

	ag.pending_req = l_dbus_message_ref(message);

	btp_send(btp, BTP_GAP_SERVICE, BTP_EV_GAP_PASSKEY_CONFIRM,
					adapter->index, sizeof(ev), &ev);

	return NULL;
}

static struct l_dbus_message *ag_request_authorization_call(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_message *reply;

	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");

	return reply;
}

static struct l_dbus_message *ag_authorize_service_call(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_message *reply;

	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");

	return reply;
}

static struct l_dbus_message *ag_cancel_call(struct l_dbus *dbus,
						struct l_dbus_message *message,
						void *user_data)
{
	struct l_dbus_message *reply;

	reply = l_dbus_message_new_method_return(message);
	l_dbus_message_set_arguments(reply, "");

	return reply;
}

static void setup_ag_interface(struct l_dbus_interface *iface)
{
	l_dbus_interface_method(iface, "Release", 0, ag_release_call, "", "");
	l_dbus_interface_method(iface, "RequestPasskey", 0,
					ag_request_passkey_call, "u", "o",
					"passkey", "device");
	l_dbus_interface_method(iface, "DisplayPasskey", 0,
					ag_display_passkey_call, "", "ouq",
					"device", "passkey", "entered");
	l_dbus_interface_method(iface, "RequestConfirmation", 0,
					ag_request_confirmation_call, "", "ou",
					"device", "passkey");
	l_dbus_interface_method(iface, "RequestAuthorization", 0,
					ag_request_authorization_call, "", "o",
					"device");
	l_dbus_interface_method(iface, "AuthorizeService", 0,
					ag_authorize_service_call, "", "os",
					"device", "uuid");
	l_dbus_interface_method(iface, "Cancel", 0, ag_cancel_call, "", "");
}

struct set_io_capabilities_data {
	uint8_t capa;
	struct btp_adapter *adapter;
};

static void set_io_capabilities_setup(struct l_dbus_message *message,
								void *user_data)
{
	struct set_io_capabilities_data *sicd = user_data;
	struct l_dbus_message_builder *builder;
	char *capa_str;

	builder = l_dbus_message_builder_new(message);

	l_dbus_message_builder_append_basic(builder, 'o', AG_PATH);

	switch (sicd->capa) {
	case BTP_GAP_IOCAPA_DISPLAY_ONLY:
		capa_str = "DisplayOnly";
		break;
	case BTP_GAP_IOCAPA_DISPLAY_YESNO:
		capa_str = "DisplayYesNo";
		break;
	case BTP_GAP_IOCAPA_KEYBOARD_ONLY:
		capa_str = "KeyboardOnly";
		break;
	case BTP_GAP_IOCAPA_KEYBOARD_DISPLAY:
		capa_str = "KeyboardDisplay";
		break;
	case BTP_GAP_IOCAPA_NO_INPUT_NO_OUTPUT:
	default:
		capa_str = "NoInputNoOutput";
		break;
	}

	l_dbus_message_builder_append_basic(builder, 's', capa_str);

	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static void reg_def_req_default_agent_reply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	if (l_dbus_message_is_error(result)) {
		const char *name, *desc;

		if (!l_dbus_object_remove_interface(dbus, AG_PATH, AG_IFACE))
			l_info("Unable to remove agent instance");
		if (!l_dbus_unregister_interface(dbus, AG_IFACE))
			l_info("Unable to unregister agent interface");

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to request default agent (%s), %s", name, desc);

		btp_send_error(btp, BTP_CORE_SERVICE, BTP_INDEX_NON_CONTROLLER,
								BTP_ERROR_FAIL);
		return;
	}

	register_gap_service();
	gap_service_registered = true;

	ag.registered = true;

	btp_send(btp, BTP_CORE_SERVICE, BTP_OP_CORE_REGISTER,
					BTP_INDEX_NON_CONTROLLER, 0, NULL);
}

static void set_io_req_default_agent_reply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	struct btp_adapter *adapter = user_data;

	if (!adapter) {
		btp_send_error(btp, BTP_GAP_SERVICE, BTP_INDEX_NON_CONTROLLER,
								BTP_ERROR_FAIL);
		goto failed;
	}

	if (l_dbus_message_is_error(result)) {
		const char *name, *desc;

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to set io capabilities (%s), %s", name, desc);

		btp_send_error(btp, BTP_GAP_SERVICE, adapter->index,
								BTP_ERROR_FAIL);
		goto failed;
	}

	ag.registered = true;

	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_SET_IO_CAPA,
						adapter->index, 0, NULL);

	return;

failed:
	if (!l_dbus_object_remove_interface(dbus, AG_PATH, AG_IFACE))
		l_info("Unable to remove agent instance");
	if (!l_dbus_unregister_interface(dbus, AG_IFACE))
		l_info("Unable to unregister agent interface");
}

static void request_default_agent_setup(struct l_dbus_message *message,
								void *user_data)
{
	struct l_dbus_message_builder *builder;

	builder = l_dbus_message_builder_new(message);

	l_dbus_message_builder_append_basic(builder, 'o', AG_PATH);

	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static void set_io_capabilities_reply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	struct set_io_capabilities_data *sicd = user_data;

	if (!sicd->adapter) {
		btp_send_error(btp, BTP_GAP_SERVICE, BTP_INDEX_NON_CONTROLLER,
								BTP_ERROR_FAIL);
		goto failed;
	}

	if (l_dbus_message_is_error(result)) {
		const char *name, *desc;

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to set io capabilities (%s), %s", name, desc);

		btp_send_error(btp, BTP_GAP_SERVICE, sicd->adapter->index,
								BTP_ERROR_FAIL);
		goto failed;
	}

	if (l_dbus_proxy_method_call(ag.proxy, "RequestDefaultAgent",
						request_default_agent_setup,
						set_io_req_default_agent_reply,
						sicd->adapter, NULL))
		return;

failed:
	if (!l_dbus_object_remove_interface(dbus, AG_PATH, AG_IFACE))
		l_info("Unable to remove agent instance");
	if (!l_dbus_unregister_interface(dbus, AG_IFACE))
		l_info("Unable to unregister agent interface");
}

static void register_default_agent_reply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	const char *name, *desc;

	if (l_dbus_message_is_error(result)) {
		if (!l_dbus_object_remove_interface(dbus, AG_PATH, AG_IFACE))
			l_info("Unable to remove agent instance");
		if (!l_dbus_unregister_interface(dbus, AG_IFACE))
			l_info("Unable to unregister agent interface");

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to register default agent (%s), %s", name,
									desc);
		return;
	}

	if (!l_dbus_proxy_method_call(ag.proxy, "RequestDefaultAgent",
						request_default_agent_setup,
						reg_def_req_default_agent_reply,
						NULL, NULL)) {
		if (!l_dbus_object_remove_interface(dbus, AG_PATH, AG_IFACE))
			l_info("Unable to remove agent instance");
		if (!l_dbus_unregister_interface(dbus, AG_IFACE))
			l_info("Unable to unregister agent interface");
	}
}

static void set_io_capabilities_destroy(void *user_data)
{
	l_free(user_data);
}

static bool register_default_agent(struct btp_adapter *adapter, uint8_t capa,
				l_dbus_client_proxy_result_func_t set_io_cb)
{
	struct set_io_capabilities_data *data;

	if (!l_dbus_register_interface(dbus, AG_IFACE, setup_ag_interface, NULL,
								false)) {
		l_info("Unable to register agent interface");
		return false;
	}

	if (!l_dbus_object_add_interface(dbus, AG_PATH, AG_IFACE, NULL)) {
		l_info("Unable to instantiate agent interface");

		if (!l_dbus_unregister_interface(dbus, AG_IFACE))
			l_info("Unable to unregister agent interface");

		return false;
	}

	if (!l_dbus_object_add_interface(dbus, AG_PATH,
						L_DBUS_INTERFACE_PROPERTIES,
						NULL)) {
		l_info("Unable to instantiate the ag properties interface");

		if (!l_dbus_object_remove_interface(dbus, AG_PATH, AG_IFACE))
			l_info("Unable to remove agent instance");
		if (!l_dbus_unregister_interface(dbus, AG_IFACE))
			l_info("Unable to unregister agent interface");

		return false;
	}

	data = l_new(struct set_io_capabilities_data, 1);
	data->adapter = adapter;
	data->capa = capa;

	if (!l_dbus_proxy_method_call(ag.proxy, "RegisterAgent",
					set_io_capabilities_setup, set_io_cb,
					data, set_io_capabilities_destroy)) {
		if (!l_dbus_object_remove_interface(dbus, AG_PATH, AG_IFACE))
			l_info("Unable to remove agent instance");
		if (!l_dbus_unregister_interface(dbus, AG_IFACE))
			l_info("Unable to unregister agent interface");

		return false;
	}

	return true;
}

struct rereg_unreg_agent_data {
	struct btp_adapter *adapter;
	l_dbus_client_proxy_result_func_t cb;
	uint8_t capa;
};

static void rereg_unreg_agent_reply(struct l_dbus_proxy *proxy,
						struct l_dbus_message *result,
						void *user_data)
{
	struct rereg_unreg_agent_data *ruad = user_data;

	if (l_dbus_message_is_error(result)) {
		const char *name;

		l_dbus_message_get_error(result, &name, NULL);

		l_error("Failed to unregister agent %s (%s)",
					l_dbus_proxy_get_path(proxy), name);
		return;
	}

	if (!l_dbus_object_remove_interface(dbus, AG_PATH,
						L_DBUS_INTERFACE_PROPERTIES))
		l_info("Unable to remove propety instance");
	if (!l_dbus_object_remove_interface(dbus, AG_PATH, AG_IFACE))
		l_info("Unable to remove agent instance");
	if (!l_dbus_unregister_interface(dbus, AG_IFACE))
		l_info("Unable to unregister agent interface");

	ag.registered = false;

	if (!register_default_agent(ruad->adapter, ruad->capa, ruad->cb))
		btp_send_error(btp, BTP_GAP_SERVICE, ruad->adapter->index,
								BTP_ERROR_FAIL);
}

static void rereg_unreg_agent_destroy(void *rereg_unreg_agent_data)
{
	l_free(rereg_unreg_agent_data);
}

static void btp_gap_set_io_capabilities(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	const struct btp_gap_set_io_capa_cp *cp = param;
	uint8_t status = BTP_ERROR_FAIL;
	struct rereg_unreg_agent_data *data;
	bool prop;

	switch (cp->capa) {
	case BTP_GAP_IOCAPA_DISPLAY_ONLY:
	case BTP_GAP_IOCAPA_DISPLAY_YESNO:
	case BTP_GAP_IOCAPA_KEYBOARD_ONLY:
	case BTP_GAP_IOCAPA_NO_INPUT_NO_OUTPUT:
	case BTP_GAP_IOCAPA_KEYBOARD_DISPLAY:
		break;
	default:
		l_error("Wrong iocapa given!");

		goto failed;
	}

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	/* Adapter needs to be powered to be able to set io cap */
	if (!l_dbus_proxy_get_property(adapter->proxy, "Powered", "b", &prop) ||
									!prop)
		goto failed;

	if (ag.registered) {
		data = l_new(struct rereg_unreg_agent_data, 1);
		data->adapter = adapter;
		data->capa = cp->capa;
		data->cb = set_io_capabilities_reply;

		if (!l_dbus_proxy_method_call(ag.proxy, "UnregisterAgent",
						unreg_agent_setup,
						rereg_unreg_agent_reply, data,
						rereg_unreg_agent_destroy))
			goto failed;

		return;
	}

	if (!register_default_agent(adapter, cp->capa,
						set_io_capabilities_reply))
		goto failed;

	return;

failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

static void pair_reply(struct l_dbus_proxy *proxy,
				struct l_dbus_message *result, void *user_data)
{
	uint8_t adapter_index = L_PTR_TO_UINT(user_data);
	struct btp_adapter *adapter = find_adapter_by_index(adapter_index);

	if (!adapter)
		return;

	if (l_dbus_message_is_error(result)) {
		const char *name, *desc;

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to pair (%s), %s", name, desc);

		return;
	}
}

static void btp_gap_pair(uint8_t index, const void *param, uint16_t length,
								void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	const struct btp_gap_pair_cp *cp = param;
	uint8_t status = BTP_ERROR_FAIL;
	struct btp_device *device;
	bool prop;

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	/* Adapter needs to be powered to be able to pair */
	if (!l_dbus_proxy_get_property(adapter->proxy, "Powered", "b", &prop) ||
									!prop)
		goto failed;

	device = find_device_by_address(adapter, &cp->address,
							cp->address_type);

	if (!device)
		goto failed;

	/* This command is asynchronous, send reply immediatelly to not block
	 * pairing process eg. passkey request.
	 */
	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_PAIR, adapter->index, 0,
									NULL);

	l_dbus_proxy_method_call(device->proxy, "Pair", NULL, pair_reply,
					L_UINT_TO_PTR(adapter->index), NULL);

	return;

failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

static void unpair_reply(struct l_dbus_proxy *proxy,
				struct l_dbus_message *result, void *user_data)
{
	struct btp_device *device = user_data;
	struct btp_adapter *adapter = find_adapter_by_device(device);

	if (!adapter) {
		btp_send_error(btp, BTP_GAP_SERVICE, BTP_INDEX_NON_CONTROLLER,
								BTP_ERROR_FAIL);
		return;
	}

	if (l_dbus_message_is_error(result)) {
		const char *name, *desc;

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to unpair (%s), %s", name, desc);

		btp_send_error(btp, BTP_GAP_SERVICE, adapter->index,
								BTP_ERROR_FAIL);
		return;
	}

	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_UNPAIR, adapter->index, 0,
									NULL);
}

static void unpair_setup(struct l_dbus_message *message, void *user_data)
{
	struct btp_device *device = user_data;
	const char *path = l_dbus_proxy_get_path(device->proxy);
	struct l_dbus_message_builder *builder;

	builder = l_dbus_message_builder_new(message);

	l_dbus_message_builder_append_basic(builder, 'o', path);

	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);
}

static void btp_gap_unpair(uint8_t index, const void *param, uint16_t length,
								void *user_data)
{
	struct btp_adapter *adapter = find_adapter_by_index(index);
	const struct btp_gap_pair_cp *cp = param;
	uint8_t status = BTP_ERROR_FAIL;
	struct btp_device *device;
	bool prop;

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	/* Adapter needs to be powered to be able to unpair */
	if (!l_dbus_proxy_get_property(adapter->proxy, "Powered", "b", &prop) ||
									!prop)
		goto failed;

	device = find_device_by_address(adapter, &cp->address,
							cp->address_type);

	if (!device)
		goto failed;

	/* There is no direct unpair method, removing device will clear pairing
	 * information.
	 */
	l_dbus_proxy_method_call(adapter->proxy, "RemoveDevice", unpair_setup,
						unpair_reply, device, NULL);

	return;

failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

static void passkey_entry_rsp_reply(struct l_dbus_message *result,
								void *user_data)
{
	struct btp_adapter *adapter = user_data;

	if (l_dbus_message_is_error(result)) {
		const char *name, *desc;

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to reply with passkey (%s), %s", name, desc);

		btp_send_error(btp, BTP_GAP_SERVICE, adapter->index,
								BTP_ERROR_FAIL);
		return;
	}

	l_dbus_message_unref(ag.pending_req);

	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_PASSKEY_ENTRY_RSP,
						adapter->index, 0, NULL);
}

static void btp_gap_passkey_entry_rsp(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	const struct btp_gap_passkey_entry_rsp_cp *cp = param;
	struct btp_adapter *adapter = find_adapter_by_index(index);
	struct l_dbus_message_builder *builder;
	uint8_t status = BTP_ERROR_FAIL;
	uint32_t passkey = L_CPU_TO_LE32(cp->passkey);
	bool prop;

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	/* Adapter needs to be powered to be able to response with passkey */
	if (!l_dbus_proxy_get_property(adapter->proxy, "Powered", "b", &prop) ||
						!prop || !ag.pending_req)
		goto failed;

	builder = l_dbus_message_builder_new(ag.pending_req);
	l_dbus_message_builder_append_basic(builder, 'u', &passkey);
	l_dbus_message_builder_finalize(builder);
	l_dbus_message_builder_destroy(builder);

	l_dbus_send_with_reply(dbus, ag.pending_req, passkey_entry_rsp_reply,
								adapter, NULL);

	return;

failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

static void passkey_confirm_rsp_reply(struct l_dbus_message *result,
								void *user_data)
{
	struct btp_adapter *adapter = user_data;

	if (l_dbus_message_is_error(result)) {
		const char *name, *desc;

		l_dbus_message_get_error(result, &name, &desc);
		l_error("Failed to confirm passkey (%s), %s", name, desc);

		btp_send_error(btp, BTP_GAP_SERVICE, adapter->index,
								BTP_ERROR_FAIL);
		return;
	}

	l_dbus_message_unref(ag.pending_req);

	btp_send(btp, BTP_GAP_SERVICE, BTP_OP_GAP_PASSKEY_CONFIRM_RSP,
						adapter->index, 0, NULL);
}

static void btp_gap_confirm_entry_rsp(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	const struct btp_gap_passkey_confirm_rsp_cp *cp = param;
	struct btp_adapter *adapter = find_adapter_by_index(index);
	struct l_dbus_message *reply;
	uint8_t status = BTP_ERROR_FAIL;
	bool prop;

	if (!adapter) {
		status = BTP_ERROR_INVALID_INDEX;
		goto failed;
	}

	/* Adapter needs to be powered to be able to confirm passkey */
	if (!l_dbus_proxy_get_property(adapter->proxy, "Powered", "b", &prop) ||
						!prop || !ag.pending_req)
		goto failed;

	if (cp->match) {
		reply = l_dbus_message_new_method_return(ag.pending_req);
		l_dbus_message_set_arguments(reply, "");
	} else {
		reply = l_dbus_message_new_error(ag.pending_req,
						"org.bluez.Error.Rejected",
						"Passkey missmatch");
	}

	l_dbus_send_with_reply(dbus, ag.pending_req, passkey_confirm_rsp_reply,
								adapter, NULL);

	return;

failed:
	btp_send_error(btp, BTP_GAP_SERVICE, index, status);
}

static void btp_gap_device_found_ev(struct l_dbus_proxy *proxy)
{
	struct btp_device *device = find_device_by_proxy(proxy);
	struct btp_adapter *adapter = find_adapter_by_device(device);
	struct btp_device_found_ev ev;
	struct btp_gap_device_connected_ev ev_conn;
	const char *str, *addr_str;
	int16_t rssi;
	uint8_t address_type;
	bool connected;

	if (!l_dbus_proxy_get_property(proxy, "Address", "s", &addr_str) ||
					str2ba(addr_str, &ev.address) < 0)
		return;

	if (!l_dbus_proxy_get_property(proxy, "AddressType", "s", &str))
		return;

	address_type = strcmp(str, "public") ? BTP_GAP_ADDR_RANDOM :
							BTP_GAP_ADDR_PUBLIC;
	ev.address_type = address_type;

	if (!l_dbus_proxy_get_property(proxy, "RSSI", "n", &rssi))
		ev.rssi = 0x81;
	else
		ev.rssi = rssi;

	/* TODO Temporary set all flags */
	ev.flags = (BTP_EV_GAP_DEVICE_FOUND_FLAG_RSSI |
					BTP_EV_GAP_DEVICE_FOUND_FLAG_AD |
					BTP_EV_GAP_DEVICE_FOUND_FLAG_SR);

	/* TODO Add eir to device found event */
	ev.eir_len = 0;

	btp_send(btp, BTP_GAP_SERVICE, BTP_EV_GAP_DEVICE_FOUND, adapter->index,
						sizeof(ev) + ev.eir_len, &ev);

	if (l_dbus_proxy_get_property(proxy, "Connected", "b", &connected) &&
								connected) {
		ev_conn.address_type = address_type;
		str2ba(addr_str, &ev_conn.address);

		btp_send(btp, BTP_GAP_SERVICE, BTP_EV_GAP_DEVICE_CONNECTED,
				adapter->index, sizeof(ev_conn), &ev_conn);
	}
}

static void btp_gap_device_connection_ev(struct l_dbus_proxy *proxy,
								bool connected)
{
	struct btp_adapter *adapter;
	struct btp_device *device;
	const char *str_addr, *str_addr_type;
	uint8_t address_type;

	device = find_device_by_proxy(proxy);
	adapter = find_adapter_by_device(device);

	if (!device || !adapter)
		return;

	if (!l_dbus_proxy_get_property(proxy, "Address", "s", &str_addr))
		return;

	if (!l_dbus_proxy_get_property(proxy, "AddressType", "s",
								&str_addr_type))
		return;

	address_type = strcmp(str_addr_type, "public") ? BTP_GAP_ADDR_RANDOM :
							BTP_GAP_ADDR_PUBLIC;

	if (connected) {
		struct btp_gap_device_connected_ev ev;

		str2ba(str_addr, &ev.address);
		ev.address_type = address_type;

		btp_send(btp, BTP_GAP_SERVICE, BTP_EV_GAP_DEVICE_CONNECTED,
					adapter->index, sizeof(ev), &ev);
	} else {
		struct btp_gap_device_disconnected_ev ev;

		str2ba(str_addr, &ev.address);
		ev.address_type = address_type;

		btp_send(btp, BTP_GAP_SERVICE, BTP_EV_GAP_DEVICE_DISCONNECTED,
					adapter->index, sizeof(ev), &ev);
	}
}

static void btp_identity_resolved_ev(struct l_dbus_proxy *proxy)
{
	struct btp_device *dev = find_device_by_proxy(proxy);
	struct btp_adapter *adapter = find_adapter_by_device(dev);
	struct btp_gap_identity_resolved_ev ev;
	char *str_addr, *str_addr_type;
	uint8_t identity_address_type;

	if (!l_dbus_proxy_get_property(proxy, "Address", "s", &str_addr))
		return;

	if (!l_dbus_proxy_get_property(proxy, "AddressType", "s",
								&str_addr_type))
		return;

	identity_address_type = strcmp(str_addr_type, "public") ?
				BTP_GAP_ADDR_RANDOM : BTP_GAP_ADDR_PUBLIC;

	str2ba(str_addr, &ev.identity_address);
	ev.identity_address_type = identity_address_type;

	memcpy(&ev.address, &dev->address, sizeof(ev.address));
	ev.address_type = dev->address_type;

	btp_send(btp, BTP_GAP_SERVICE, BTP_EV_GAP_IDENTITY_RESOLVED,
					adapter->index, sizeof(ev), &ev);
}

static void register_gap_service(void)
{
	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_READ_SUPPORTED_COMMANDS,
					btp_gap_read_commands, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE,
				BTP_OP_GAP_READ_CONTROLLER_INDEX_LIST,
				btp_gap_read_controller_index, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_READ_COTROLLER_INFO,
						btp_gap_read_info, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_RESET,
						btp_gap_reset, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_SET_POWERED,
					btp_gap_set_powered, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_SET_CONNECTABLE,
					btp_gap_set_connectable, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_SET_DISCOVERABLE,
					btp_gap_set_discoverable, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_SET_BONDABLE,
					btp_gap_set_bondable, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_START_ADVERTISING,
					btp_gap_start_advertising, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_STOP_ADVERTISING,
					btp_gap_stop_advertising, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_START_DISCOVERY,
					btp_gap_start_discovery, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_STOP_DISCOVERY,
					btp_gap_stop_discovery, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_CONNECT, btp_gap_connect,
								NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_DISCONNECT,
						btp_gap_disconnect, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_SET_IO_CAPA,
				btp_gap_set_io_capabilities, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_PAIR, btp_gap_pair, NULL,
									NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_UNPAIR, btp_gap_unpair,
								NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_PASSKEY_ENTRY_RSP,
					btp_gap_passkey_entry_rsp, NULL, NULL);

	btp_register(btp, BTP_GAP_SERVICE, BTP_OP_GAP_PASSKEY_CONFIRM_RSP,
					btp_gap_confirm_entry_rsp, NULL, NULL);
}

static void btp_core_read_commands(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	uint8_t commands = 0;

	if (index != BTP_INDEX_NON_CONTROLLER) {
		btp_send_error(btp, BTP_CORE_SERVICE, index,
						BTP_ERROR_INVALID_INDEX);
		return;
	}

	commands |= (1 << BTP_OP_CORE_READ_SUPPORTED_COMMANDS);
	commands |= (1 << BTP_OP_CORE_READ_SUPPORTED_SERVICES);
	commands |= (1 << BTP_OP_CORE_REGISTER);
	commands |= (1 << BTP_OP_CORE_UNREGISTER);

	btp_send(btp, BTP_CORE_SERVICE, BTP_OP_CORE_READ_SUPPORTED_COMMANDS,
			BTP_INDEX_NON_CONTROLLER, sizeof(commands), &commands);
}

static void btp_core_read_services(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	uint8_t services = 0;

	if (index != BTP_INDEX_NON_CONTROLLER) {
		btp_send_error(btp, BTP_CORE_SERVICE, index,
						BTP_ERROR_INVALID_INDEX);
		return;
	}

	services |= (1 << BTP_CORE_SERVICE);
	services |= (1 << BTP_GAP_SERVICE);

	btp_send(btp, BTP_CORE_SERVICE, BTP_OP_CORE_READ_SUPPORTED_SERVICES,
			BTP_INDEX_NON_CONTROLLER, sizeof(services), &services);
}

static void btp_core_register(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	const struct btp_core_register_cp  *cp = param;

	if (length < sizeof(*cp))
		goto failed;

	if (index != BTP_INDEX_NON_CONTROLLER) {
		btp_send_error(btp, BTP_CORE_SERVICE, index,
						BTP_ERROR_INVALID_INDEX);
		return;
	}

	switch (cp->service_id) {
	case BTP_GAP_SERVICE:
		if (gap_service_registered)
			goto failed;

		if (!register_default_agent(NULL,
					BTP_GAP_IOCAPA_NO_INPUT_NO_OUTPUT,
					register_default_agent_reply))
			goto failed;

		return;
	case BTP_GATT_SERVICE:
	case BTP_L2CAP_SERVICE:
	case BTP_MESH_NODE_SERVICE:
	case BTP_CORE_SERVICE:
	default:
		goto failed;
	}

	btp_send(btp, BTP_CORE_SERVICE, BTP_OP_CORE_REGISTER,
					BTP_INDEX_NON_CONTROLLER, 0, NULL);
	return;

failed:
	btp_send_error(btp, BTP_CORE_SERVICE, index, BTP_ERROR_FAIL);
}

static void btp_core_unregister(uint8_t index, const void *param,
					uint16_t length, void *user_data)
{
	const struct btp_core_unregister_cp  *cp = param;

	if (length < sizeof(*cp))
		goto failed;

	if (index != BTP_INDEX_NON_CONTROLLER) {
		btp_send_error(btp, BTP_CORE_SERVICE, index,
						BTP_ERROR_INVALID_INDEX);
		return;
	}

	switch (cp->service_id) {
	case BTP_GAP_SERVICE:
		if (!gap_service_registered)
			goto failed;

		btp_unregister_service(btp, BTP_GAP_SERVICE);
		gap_service_registered = false;
		break;
	case BTP_GATT_SERVICE:
	case BTP_L2CAP_SERVICE:
	case BTP_MESH_NODE_SERVICE:
	case BTP_CORE_SERVICE:
	default:
		goto failed;
	}

	btp_send(btp, BTP_CORE_SERVICE, BTP_OP_CORE_UNREGISTER,
					BTP_INDEX_NON_CONTROLLER, 0, NULL);
	return;

failed:
	btp_send_error(btp, BTP_CORE_SERVICE, index, BTP_ERROR_FAIL);
}

static void register_core_service(void)
{
	btp_register(btp, BTP_CORE_SERVICE,
					BTP_OP_CORE_READ_SUPPORTED_COMMANDS,
					btp_core_read_commands, NULL, NULL);

	btp_register(btp, BTP_CORE_SERVICE,
					BTP_OP_CORE_READ_SUPPORTED_SERVICES,
					btp_core_read_services, NULL, NULL);

	btp_register(btp, BTP_CORE_SERVICE, BTP_OP_CORE_REGISTER,
						btp_core_register, NULL, NULL);

	btp_register(btp, BTP_CORE_SERVICE, BTP_OP_CORE_UNREGISTER,
					btp_core_unregister, NULL, NULL);
}

static void signal_handler(uint32_t signo, void *user_data)
{
	switch (signo) {
	case SIGINT:
	case SIGTERM:
		l_info("Terminating");
		l_main_quit();
		break;
	}
}

static void btp_device_free(struct btp_device *device)
{
	l_free(device);
}

static void btp_adapter_free(struct btp_adapter *adapter)
{
	l_queue_destroy(adapter->devices,
				(l_queue_destroy_func_t)btp_device_free);
	l_free(adapter);
}

static void extract_settings(struct l_dbus_proxy *proxy, uint32_t *current,
						uint32_t *supported)
{
	bool prop;

	*supported = 0;
	*current = 0;

	/* TODO not all info is available via D-Bus API */
	*supported |=  BTP_GAP_SETTING_POWERED;
	*supported |=  BTP_GAP_SETTING_CONNECTABLE;
	*supported |=  BTP_GAP_SETTING_DISCOVERABLE;
	*supported |=  BTP_GAP_SETTING_BONDABLE;
	*supported |=  BTP_GAP_SETTING_SSP;
	*supported |=  BTP_GAP_SETTING_BREDR;
	*supported |=  BTP_GAP_SETTING_LE;
	*supported |=  BTP_GAP_SETTING_ADVERTISING;
	*supported |=  BTP_GAP_SETTING_SC;
	*supported |=  BTP_GAP_SETTING_PRIVACY;
	/* *supported |=  BTP_GAP_SETTING_STATIC_ADDRESS; */

	/* TODO not all info is availbe via D-Bus API so some are assumed to be
	 * enabled by bluetoothd or simply hardcoded until API is extended
	 */
	*current |=  BTP_GAP_SETTING_CONNECTABLE;
	*current |=  BTP_GAP_SETTING_SSP;
	*current |=  BTP_GAP_SETTING_BREDR;
	*current |=  BTP_GAP_SETTING_LE;
	*current |=  BTP_GAP_SETTING_PRIVACY;
	*current |=  BTP_GAP_SETTING_SC;
	/* *supported |=  BTP_GAP_SETTING_STATIC_ADDRESS; */

	if (l_dbus_proxy_get_property(proxy, "Powered", "b", &prop) && prop)
		*current |=  BTP_GAP_SETTING_POWERED;

	if (l_dbus_proxy_get_property(proxy, "Discoverable", "b", &prop) &&
									prop)
		*current |=  BTP_GAP_SETTING_DISCOVERABLE;

	if (l_dbus_proxy_get_property(proxy, "Pairable", "b", &prop) && prop)
		*current |=  BTP_GAP_SETTING_BONDABLE;
}

static void proxy_added(struct l_dbus_proxy *proxy, void *user_data)
{
	const char *interface = l_dbus_proxy_get_interface(proxy);
	const char *path = l_dbus_proxy_get_path(proxy);

	l_info("Proxy added: %s (%s)", interface, path);

	if (!strcmp(interface, "org.bluez.Adapter1")) {
		struct btp_adapter *adapter;

		adapter = l_new(struct btp_adapter, 1);
		adapter->proxy = proxy;
		adapter->index = l_queue_length(adapters);
		adapter->devices = l_queue_new();

		extract_settings(proxy, &adapter->current_settings,
						&adapter->supported_settings);

		adapter->default_settings = adapter->current_settings;

		l_queue_push_tail(adapters, adapter);
		return;
	}

	if (!strcmp(interface, "org.bluez.Device1")) {
		struct btp_adapter *adapter;
		struct btp_device *device;
		char *str, *str_addr, *str_addr_type;

		if (!l_dbus_proxy_get_property(proxy, "Adapter", "o", &str))
			return;

		adapter = find_adapter_by_path(str);
		if (!adapter)
			return;

		device = l_new(struct btp_device, 1);
		device->proxy = proxy;

		l_queue_push_tail(adapter->devices, device);

		btp_gap_device_found_ev(proxy);

		if (!l_dbus_proxy_get_property(proxy, "Address", "s",
								&str_addr))
			return;

		if (!l_dbus_proxy_get_property(proxy, "AddressType", "s",
								&str_addr_type))
			return;

		device->address_type = strcmp(str_addr_type, "public") ?
							BTP_GAP_ADDR_RANDOM :
							BTP_GAP_ADDR_PUBLIC;
		if (!str2ba(str_addr, &device->address))
			return;

		return;
	}

	if (!strcmp(interface, "org.bluez.LEAdvertisingManager1")) {
		struct btp_adapter *adapter;

		adapter = find_adapter_by_path(path);
		if (!adapter)
			return;

		adapter->ad_proxy = proxy;

		return;
	}

	if (!strcmp(interface, "org.bluez.AgentManager1")) {
		ag.proxy = proxy;

		return;
	}
}

static bool device_match_by_proxy(const void *a, const void *b)
{
	const struct btp_device *device = a;
	const struct l_dbus_proxy *proxy = b;

	return device->proxy == proxy;
}

static void proxy_removed(struct l_dbus_proxy *proxy, void *user_data)
{
	const char *interface = l_dbus_proxy_get_interface(proxy);
	const char *path = l_dbus_proxy_get_path(proxy);

	l_info("Proxy removed: %s (%s)", interface, path);

	if (!strcmp(interface, "org.bluez.Adapter1")) {
		l_info("Adapter removed, terminating.");
		l_main_quit();
		return;
	}

	if (!strcmp(interface, "org.bluez.Device1")) {
		struct btp_adapter *adapter;
		char *str;

		if (!l_dbus_proxy_get_property(proxy, "Adapter", "o", &str))
			return;

		adapter = find_adapter_by_path(str);
		if (!adapter)
			return;

		l_queue_remove_if(adapter->devices, device_match_by_proxy,
									proxy);

		return;
	}
}

static void property_changed(struct l_dbus_proxy *proxy, const char *name,
				struct l_dbus_message *msg, void *user_data)
{
	const char *interface = l_dbus_proxy_get_interface(proxy);
	const char *path = l_dbus_proxy_get_path(proxy);

	l_info("property_changed %s %s %s", name, path, interface);

	if (!strcmp(interface, "org.bluez.Adapter1")) {
		struct btp_adapter *adapter = find_adapter_by_proxy(proxy);
		uint32_t new_settings;

		if (!adapter)
			return;

		new_settings = adapter->current_settings;

		if (!strcmp(name, "Powered")) {
			bool prop;

			if (!l_dbus_message_get_arguments(msg, "b", &prop))
				return;

			if (prop)
				new_settings |= BTP_GAP_SETTING_POWERED;
			else
				new_settings &= ~BTP_GAP_SETTING_POWERED;
		} else if (!strcmp(name, "Discoverable")) {
			bool prop;

			if (!l_dbus_message_get_arguments(msg, "b", &prop))
				return;

			if (prop)
				new_settings |= BTP_GAP_SETTING_DISCOVERABLE;
			else
				new_settings &= ~BTP_GAP_SETTING_DISCOVERABLE;
		}

		if (!strcmp(name, "Pairable")) {
			bool prop;

			if (!l_dbus_message_get_arguments(msg, "b", &prop))
				return;

			if (prop)
				new_settings |= BTP_GAP_SETTING_BONDABLE;
			else
				new_settings &= ~BTP_GAP_SETTING_BONDABLE;
		}

		if (new_settings != adapter->current_settings)
			update_current_settings(adapter, new_settings);

		return;
	} else if (!strcmp(interface, "org.bluez.Device1")) {
		if (!strcmp(name, "RSSI")) {
			int16_t rssi;

			if (!l_dbus_message_get_arguments(msg, "n", &rssi))
				return;

			btp_gap_device_found_ev(proxy);
		} else if (!strcmp(name, "Connected")) {
			bool prop;

			if (!l_dbus_message_get_arguments(msg, "b", &prop))
				return;

			btp_gap_device_connection_ev(proxy, prop);
		} else if (!strcmp(name, "AddressType")) {
			/* Addres property change came first along with address
			 * type.
			 */
			btp_identity_resolved_ev(proxy);
		}
	}
}

static void client_connected(struct l_dbus *dbus, void *user_data)
{
	l_info("D-Bus client connected");
}

static void client_disconnected(struct l_dbus *dbus, void *user_data)
{
	l_info("D-Bus client disconnected, terminated");
	l_main_quit();
}

static void btp_disconnect_handler(struct btp *btp, void *user_data)
{
	l_info("btp disconnected");
	l_main_quit();
}

static void client_ready(struct l_dbus_client *client, void *user_data)
{
	l_info("D-Bus client ready, connecting BTP");

	btp = btp_new(socket_path);
	if (!btp) {
		l_error("Failed to connect BTP, terminating");
		l_main_quit();
		return;
	}

	btp_set_disconnect_handler(btp, btp_disconnect_handler, NULL, NULL);

	register_core_service();

	btp_send(btp, BTP_CORE_SERVICE, BTP_EV_CORE_READY,
					BTP_INDEX_NON_CONTROLLER, 0, NULL);
}

static void ready_callback(void *user_data)
{
	if (!l_dbus_object_manager_enable(dbus, "/"))
		l_info("Unable to register the ObjectManager");
}

static void usage(void)
{
	l_info("btpclient - Bluetooth tester");
	l_info("Usage:");
	l_info("\tbtpclient [options]");
	l_info("options:\n"
	"\t-s, --socket <socket>  Socket to use for BTP\n"
	"\t-q, --quiet            Don't emit any logs\n"
	"\t-v, --version          Show version\n"
	"\t-h, --help             Show help options");
}

static const struct option options[] = {
	{ "socket",	1, 0, 's' },
	{ "quiet",	0, 0, 'q' },
	{ "version",	0, 0, 'v' },
	{ "help",	0, 0, 'h' },
	{ 0, 0, 0, 0 }
};

int main(int argc, char *argv[])
{
	struct l_dbus_client *client;
	int opt;

	l_log_set_stderr();

	while ((opt = getopt_long(argc, argv, "+hs:vq", options, NULL)) != -1) {
		switch (opt) {
		case 's':
			socket_path = l_strdup(optarg);
			break;
		case 'q':
			l_log_set_null();
			break;
		case 'd':
			break;
		case 'v':
			l_info("%s", VERSION);
			return EXIT_SUCCESS;
		case 'h':
		default:
			usage();
			return EXIT_SUCCESS;
		}
	}

	if (!socket_path) {
		l_info("Socket option is required");
		l_info("Type --help for usage");
		return EXIT_FAILURE;
	}

	if (!l_main_init())
		return EXIT_FAILURE;


	adapters = l_queue_new();

	dbus = l_dbus_new_default(L_DBUS_SYSTEM_BUS);
	l_dbus_set_ready_handler(dbus, ready_callback, NULL, NULL);
	client = l_dbus_client_new(dbus, "org.bluez", "/org/bluez");

	l_dbus_client_set_connect_handler(client, client_connected, NULL, NULL);
	l_dbus_client_set_disconnect_handler(client, client_disconnected, NULL,
									NULL);

	l_dbus_client_set_proxy_handlers(client, proxy_added, proxy_removed,
						property_changed, NULL, NULL);

	l_dbus_client_set_ready_handler(client, client_ready, NULL, NULL);

	l_main_run_with_signal(signal_handler, NULL);

	l_dbus_client_destroy(client);
	l_dbus_destroy(dbus);
	btp_cleanup(btp);

	l_queue_destroy(adapters, (l_queue_destroy_func_t)btp_adapter_free);

	l_free(socket_path);

	l_main_exit();

	return EXIT_SUCCESS;
}
