/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Google Inc.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#include <dbus/dbus.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "log.h"
#include "error.h"
#include "adapter.h"
#include "device.h"
#include "src/shared/queue.h"
#include "src/shared/att.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-client.h"
#include "src/shared/util.h"
#include "gatt-client.h"
#include "dbus-common.h"

#ifndef NELEM
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define GATT_SERVICE_IFACE		"org.bluez.GattService1"
#define GATT_CHARACTERISTIC_IFACE	"org.bluez.GattCharacteristic1"
#define GATT_DESCRIPTOR_IFACE		"org.bluez.GattDescriptor1"

struct btd_gatt_client {
	struct btd_device *device;
	bool ready;
	char devaddr[18];
	struct gatt_db *db;
	struct bt_gatt_client *gatt;

	struct queue *services;
	struct queue *all_notify_clients;
};

struct service {
	struct btd_gatt_client *client;
	bool primary;
	uint16_t start_handle;
	uint16_t end_handle;
	bt_uuid_t uuid;
	char *path;
	struct queue *chrcs;
};

typedef bool (*async_dbus_op_complete_t)(void *data);

struct async_dbus_op {
	int ref_count;
	unsigned int id;
	struct queue *msgs;
	void *data;
	uint16_t offset;
	async_dbus_op_complete_t complete;
};

struct characteristic {
	struct service *service;
	struct gatt_db_attribute *attr;
	uint16_t handle;
	uint16_t value_handle;
	uint8_t props;
	uint16_t ext_props;
	uint16_t ext_props_handle;
	bt_uuid_t uuid;
	char *path;

	struct async_dbus_op *read_op;
	struct async_dbus_op *write_op;

	struct queue *descs;

	bool notifying;
	struct queue *notify_clients;
};

struct descriptor {
	struct characteristic *chrc;
	struct gatt_db_attribute *attr;
	uint16_t handle;
	bt_uuid_t uuid;
	char *path;

	struct async_dbus_op *read_op;
	struct async_dbus_op *write_op;
};

static bool uuid_cmp(const bt_uuid_t *uuid, uint16_t u16)
{
	bt_uuid_t uuid16;

	bt_uuid16_create(&uuid16, u16);

	return bt_uuid_cmp(uuid, &uuid16) == 0;
}

static gboolean descriptor_get_uuid(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	char uuid[MAX_LEN_UUID_STR + 1];
	const char *ptr = uuid;
	struct descriptor *desc = data;

	bt_uuid_to_string(&desc->uuid, uuid, sizeof(uuid));
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &ptr);

	return TRUE;
}

static gboolean descriptor_get_characteristic(
					const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct descriptor *desc = data;
	const char *str = desc->chrc->path;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &str);

	return TRUE;
}

static void read_cb(struct gatt_db_attribute *attrib, int err,
				const uint8_t *value, size_t length,
				void *user_data)
{
	DBusMessageIter *array = user_data;

	if (err)
		return;

	dbus_message_iter_append_fixed_array(array, DBUS_TYPE_BYTE, &value,
								length);
}

static gboolean descriptor_get_value(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct descriptor *desc = data;
	DBusMessageIter array;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "y", &array);

	gatt_db_attribute_read(desc->attr, 0, 0, NULL, read_cb, &array);

	dbus_message_iter_close_container(iter, &array);

	return TRUE;
}

static void read_check_cb(struct gatt_db_attribute *attrib, int err,
				const uint8_t *value, size_t length,
				void *user_data)
{
	gboolean *ret = user_data;

	if (err || length == 0) {
		*ret = FALSE;
		return;
	}

	*ret = TRUE;
}

static gboolean descriptor_value_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct descriptor *desc = data;
	gboolean ret;

	gatt_db_attribute_read(desc->attr, 0, 0, NULL, read_check_cb, &ret);

	return ret;
}

static int parse_value_arg(DBusMessageIter *iter, uint8_t **value, int *len)
{
	DBusMessageIter array;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return -EINVAL;

	dbus_message_iter_recurse(iter, &array);
	dbus_message_iter_get_fixed_array(&array, value, len);

	return 0;
}

static void async_dbus_op_free(void *data)
{
	struct async_dbus_op *op = data;

	queue_destroy(op->msgs, (void *)dbus_message_unref);

	free(op);
}

static struct async_dbus_op *async_dbus_op_ref(struct async_dbus_op *op)
{
	__sync_fetch_and_add(&op->ref_count, 1);

	return op;
}

static void async_dbus_op_unref(void *data)
{
	struct async_dbus_op *op = data;

	if (__sync_sub_and_fetch(&op->ref_count, 1))
		return;

	async_dbus_op_free(op);
}

static void message_append_byte_array(DBusMessage *msg, const uint8_t *bytes,
								size_t len)
{
	DBusMessageIter iter, array;

	dbus_message_iter_init_append(msg, &iter);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "y", &array);
	dbus_message_iter_append_fixed_array(&array, DBUS_TYPE_BYTE, &bytes,
									len);
	dbus_message_iter_close_container(&iter, &array);
}

static DBusMessage *create_gatt_dbus_error(DBusMessage *msg, uint8_t att_ecode)
{
	switch (att_ecode) {
	case BT_ATT_ERROR_READ_NOT_PERMITTED:
		return btd_error_not_permitted(msg, "Read not permitted");
	case BT_ATT_ERROR_WRITE_NOT_PERMITTED:
		return btd_error_not_permitted(msg, "Write not permitted");
	case BT_ATT_ERROR_AUTHENTICATION:
	case BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION:
	case BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION_KEY_SIZE:
		return btd_error_not_permitted(msg, "Not paired");
	case BT_ATT_ERROR_INVALID_OFFSET:
		return btd_error_invalid_args_str(msg, "Invalid offset");
	case BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN:
		return btd_error_invalid_args_str(msg, "Invalid Length");
	case BT_ATT_ERROR_AUTHORIZATION:
		return btd_error_not_authorized(msg);
	case BT_ATT_ERROR_REQUEST_NOT_SUPPORTED:
		return btd_error_not_supported(msg);
	case 0:
		return btd_error_failed(msg, "Operation failed");
	default:
		return g_dbus_create_error(msg, ERROR_INTERFACE ".Failed",
				"Operation failed with ATT error: 0x%02x",
				att_ecode);
	}

	return NULL;
}

static void write_descriptor_cb(struct gatt_db_attribute *attr, int err,
								void *user_data)
{
	struct descriptor *desc = user_data;

	if (err)
		return;

	g_dbus_emit_property_changed(btd_get_dbus_connection(), desc->path,
					GATT_DESCRIPTOR_IFACE, "Value");
}

static void async_dbus_op_reply(struct async_dbus_op *op, int err,
				const uint8_t *value, size_t length)
{
	const struct queue_entry *entry;
	DBusMessage *reply;

	op->id = 0;

	for (entry = queue_get_entries(op->msgs); entry; entry = entry->next) {
		DBusMessage *msg = entry->data;

		if (err) {
			reply = err > 0 ? create_gatt_dbus_error(msg, err) :
				btd_error_failed(msg, strerror(err));
			goto send_reply;
		}

		reply = g_dbus_create_reply(msg, DBUS_TYPE_INVALID);
		if (!reply) {
			error("Failed to allocate D-Bus message reply");
			return;
		}

		if (value)
			message_append_byte_array(reply, value, length);

send_reply:
		g_dbus_send_message(btd_get_dbus_connection(), reply);
	}
}

static void read_op_cb(struct gatt_db_attribute *attrib, int err,
				const uint8_t *value, size_t length,
				void *user_data)
{
	struct async_dbus_op *op = user_data;

	async_dbus_op_reply(op, err, value, length);
}

static void desc_read_cb(bool success, uint8_t att_ecode,
					const uint8_t *value, uint16_t length,
					void *user_data)
{
	struct async_dbus_op *op = user_data;
	struct descriptor *desc = op->data;

	if (!success)
		goto fail;

	if (!op->offset)
		gatt_db_attribute_reset(desc->attr);

	if (!gatt_db_attribute_write(desc->attr, op->offset, value, length, 0,
					NULL, write_descriptor_cb, desc)) {
		error("Failed to store attribute");
		att_ecode = BT_ATT_ERROR_UNLIKELY;
		goto fail;
	}

	/* Read the stored data from db */
	if (!gatt_db_attribute_read(desc->attr, 0, 0, NULL, read_op_cb, op)) {
		error("Failed to read database");
		att_ecode = BT_ATT_ERROR_UNLIKELY;
		goto fail;
	}

	desc->read_op = NULL;

	return;

fail:
	async_dbus_op_reply(op, att_ecode, NULL, 0);
	desc->read_op = NULL;
}

static int parse_options(DBusMessageIter *iter, uint16_t *offset)
{
	DBusMessageIter dict;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY)
		return -EINVAL;

	dbus_message_iter_recurse(iter, &dict);

	while (dbus_message_iter_get_arg_type(&dict) == DBUS_TYPE_DICT_ENTRY) {
		const char *key;
		DBusMessageIter value, entry;
		int var;

		dbus_message_iter_recurse(&dict, &entry);
		dbus_message_iter_get_basic(&entry, &key);

		dbus_message_iter_next(&entry);
		dbus_message_iter_recurse(&entry, &value);

		var = dbus_message_iter_get_arg_type(&value);
		if (strcasecmp(key, "offset") == 0) {
			if (var != DBUS_TYPE_UINT16)
				return -EINVAL;
			dbus_message_iter_get_basic(&value, offset);
		}

		dbus_message_iter_next(&dict);
	}

	return 0;
}

static struct async_dbus_op *async_dbus_op_new(DBusMessage *msg, void *data)
{
	struct async_dbus_op *op;

	op = new0(struct async_dbus_op, 1);
	op->msgs = queue_new();
	queue_push_tail(op->msgs, dbus_message_ref(msg));
	op->data = data;

	return op;
}

static struct async_dbus_op *read_value(struct bt_gatt_client *gatt,
					DBusMessage *msg, uint16_t handle,
					uint16_t offset,
					bt_gatt_client_read_callback_t callback,
					void *data)
{
	struct async_dbus_op *op;

	op = async_dbus_op_new(msg, data);
	op->offset = offset;

	op->id = bt_gatt_client_read_long_value(gatt, handle, offset, callback,
						async_dbus_op_ref(op),
						async_dbus_op_unref);
	if (op->id)
		return op;

	async_dbus_op_free(op);

	return NULL;
}

static DBusMessage *descriptor_read_value(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct descriptor *desc = user_data;
	struct bt_gatt_client *gatt = desc->chrc->service->client->gatt;
	DBusMessageIter iter;
	uint16_t offset = 0;

	if (!gatt)
		return btd_error_failed(msg, "Not connected");

	dbus_message_iter_init(msg, &iter);

	if (parse_options(&iter, &offset))
		return btd_error_invalid_args(msg);

	if (desc->read_op) {
		if (desc->read_op->offset != offset)
			return btd_error_in_progress(msg);
		queue_push_tail(desc->read_op->msgs, dbus_message_ref(msg));
		return NULL;
	}

	desc->read_op = read_value(gatt, msg, desc->handle, offset,
							desc_read_cb, desc);
	if (!desc->read_op)
		return btd_error_failed(msg, "Failed to send read request");

	return NULL;
}

static void write_result_cb(bool success, bool reliable_error,
					uint8_t att_ecode, void *user_data)
{
	struct async_dbus_op *op = user_data;
	int err = 0;

	if (op->complete && !op->complete(op->data)) {
		err = -EFAULT;
		goto done;
	}

	if (!success) {
		if (reliable_error)
			err = -EFAULT;
		else
			err = att_ecode;
	}

done:
	async_dbus_op_reply(op, err, NULL, 0);
}

static void write_cb(bool success, uint8_t att_ecode, void *user_data)
{
	write_result_cb(success, false, att_ecode, user_data);
}

static struct async_dbus_op *start_long_write(DBusMessage *msg, uint16_t handle,
					struct bt_gatt_client *gatt,
					bool reliable, const uint8_t *value,
					size_t value_len, uint16_t offset,
					void *data,
					async_dbus_op_complete_t complete)
{
	struct async_dbus_op *op;

	op = async_dbus_op_new(msg, data);
	op->complete = complete;
	op->offset = offset;

	op->id = bt_gatt_client_write_long_value(gatt, reliable, handle, offset,
							value, value_len,
							write_result_cb, op,
							async_dbus_op_free);

	if (!op->id) {
		async_dbus_op_free(op);
		return NULL;
	}

	return op;
}

static struct async_dbus_op *start_write_request(DBusMessage *msg,
					uint16_t handle,
					struct bt_gatt_client *gatt,
					const uint8_t *value, size_t value_len,
					void *data,
					async_dbus_op_complete_t complete)
{
	struct async_dbus_op *op;

	op = async_dbus_op_new(msg, data);
	op->complete = complete;

	op->id = bt_gatt_client_write_value(gatt, handle, value, value_len,
							write_cb, op,
							async_dbus_op_free);
	if (!op->id) {
		async_dbus_op_free(op);
		return NULL;
	}

	return op;
}

static bool desc_write_complete(void *data)
{
	struct descriptor *desc = data;

	desc->write_op = NULL;

	/*
	 * The descriptor might have been unregistered during the read. Return
	 * failure.
	 */
	return !!desc->chrc;
}

static DBusMessage *descriptor_write_value(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct descriptor *desc = user_data;
	struct bt_gatt_client *gatt = desc->chrc->service->client->gatt;
	DBusMessageIter iter;
	uint8_t *value = NULL;
	int value_len = 0;
	uint16_t offset = 0;

	if (!gatt)
		return btd_error_failed(msg, "Not connected");

	if (desc->write_op)
		return btd_error_in_progress(msg);

	dbus_message_iter_init(msg, &iter);

	if (parse_value_arg(&iter, &value, &value_len))
		return btd_error_invalid_args(msg);

	if (parse_options(&iter, &offset))
		return btd_error_invalid_args(msg);

	/*
	 * Don't allow writing to Client Characteristic Configuration
	 * descriptors. We achieve this through the StartNotify and StopNotify
	 * methods on GattCharacteristic1.
	 */
	if (uuid_cmp(&desc->uuid, GATT_CLIENT_CHARAC_CFG_UUID))
		return btd_error_not_permitted(msg, "Write not permitted");

	/*
	 * Based on the value length and the MTU, either use a write or a long
	 * write.
	 */
	if (value_len <= bt_gatt_client_get_mtu(gatt) - 3 && !offset)
		desc->write_op = start_write_request(msg, desc->handle,
							gatt, value,
							value_len, desc,
							desc_write_complete);
	else
		desc->write_op = start_long_write(msg, desc->handle, gatt,
							false, value,
							value_len, offset, desc,
							desc_write_complete);

	if (!desc->write_op)
		return btd_error_failed(msg, "Failed to initiate write");

	return NULL;
}

static const GDBusPropertyTable descriptor_properties[] = {
	{ "UUID", "s", descriptor_get_uuid, NULL, NULL,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{ "Characteristic", "o", descriptor_get_characteristic, NULL, NULL,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{ "Value", "ay", descriptor_get_value, NULL, descriptor_value_exists,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{ }
};

static const GDBusMethodTable descriptor_methods[] = {
	{ GDBUS_EXPERIMENTAL_ASYNC_METHOD("ReadValue",
					GDBUS_ARGS({ "options", "a{sv}" }),
					GDBUS_ARGS({ "value", "ay" }),
					descriptor_read_value) },
	{ GDBUS_EXPERIMENTAL_ASYNC_METHOD("WriteValue",
					GDBUS_ARGS({ "value", "ay" },
						{ "options", "a{sv}" }),
					NULL,
					descriptor_write_value) },
	{ }
};

static void descriptor_free(void *data)
{
	struct descriptor *desc = data;

	g_free(desc->path);
	free(desc);
}

static struct descriptor *descriptor_create(struct gatt_db_attribute *attr,
						struct characteristic *chrc)
{
	struct descriptor *desc;

	desc = new0(struct descriptor, 1);
	desc->chrc = chrc;
	desc->attr = attr;
	desc->handle = gatt_db_attribute_get_handle(attr);

	bt_uuid_to_uuid128(gatt_db_attribute_get_type(attr), &desc->uuid);

	desc->path = g_strdup_printf("%s/desc%04x", chrc->path, desc->handle);

	if (!g_dbus_register_interface(btd_get_dbus_connection(), desc->path,
						GATT_DESCRIPTOR_IFACE,
						descriptor_methods, NULL,
						descriptor_properties,
						desc, descriptor_free)) {
		error("Unable to register GATT descriptor with handle 0x%04x",
								desc->handle);
		descriptor_free(desc);

		return NULL;
	}

	DBG("Exported GATT characteristic descriptor: %s", desc->path);

	if (uuid_cmp(&desc->uuid, GATT_CHARAC_EXT_PROPER_UUID))
		chrc->ext_props_handle = desc->handle;

	return desc;
}

static void unregister_descriptor(void *data)
{
	struct descriptor *desc = data;
	struct bt_gatt_client *gatt = desc->chrc->service->client->gatt;

	DBG("Removing GATT descriptor: %s", desc->path);

	if (desc->read_op)
		bt_gatt_client_cancel(gatt, desc->read_op->id);

	if (desc->write_op)
		bt_gatt_client_cancel(gatt, desc->write_op->id);

	desc->chrc = NULL;

	g_dbus_unregister_interface(btd_get_dbus_connection(), desc->path,
							GATT_DESCRIPTOR_IFACE);
}

static gboolean characteristic_get_uuid(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	char uuid[MAX_LEN_UUID_STR + 1];
	const char *ptr = uuid;
	struct characteristic *chrc = data;

	bt_uuid_to_string(&chrc->uuid, uuid, sizeof(uuid));
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &ptr);

	return TRUE;
}

static gboolean characteristic_get_service(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct characteristic *chrc = data;
	const char *str = chrc->service->path;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &str);

	return TRUE;
}

static gboolean characteristic_get_value(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct characteristic *chrc = data;
	DBusMessageIter array;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "y", &array);

	gatt_db_attribute_read(chrc->attr, 0, 0, NULL, read_cb, &array);

	dbus_message_iter_close_container(iter, &array);

	return TRUE;
}

static gboolean characteristic_value_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct characteristic *chrc = data;
	gboolean ret;

	gatt_db_attribute_read(chrc->attr, 0, 0, NULL, read_check_cb, &ret);

	return ret;
}

static gboolean characteristic_get_notifying(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct characteristic *chrc = data;
	dbus_bool_t notifying = chrc->notifying ? TRUE : FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &notifying);

	return TRUE;
}

static gboolean
characteristic_notifying_exists(const GDBusPropertyTable *property, void *data)
{
	struct characteristic *chrc = data;

	return (chrc->props & BT_GATT_CHRC_PROP_NOTIFY ||
				chrc->props & BT_GATT_CHRC_PROP_INDICATE);
}

struct chrc_prop_data {
	uint8_t prop;
	char *str;
};

static struct chrc_prop_data chrc_props[] = {
	/* Default Properties */
	{ BT_GATT_CHRC_PROP_BROADCAST,		"broadcast" },
	{ BT_GATT_CHRC_PROP_READ,		"read" },
	{ BT_GATT_CHRC_PROP_WRITE_WITHOUT_RESP,	"write-without-response" },
	{ BT_GATT_CHRC_PROP_WRITE,		"write" },
	{ BT_GATT_CHRC_PROP_NOTIFY,		"notify" },
	{ BT_GATT_CHRC_PROP_INDICATE,		"indicate" },
	{ BT_GATT_CHRC_PROP_AUTH,		"authenticated-signed-writes" },
	{ BT_GATT_CHRC_PROP_EXT_PROP,		"extended-properties" }
};

static struct chrc_prop_data chrc_ext_props[] = {
	/* Extended Properties */
	{ BT_GATT_CHRC_EXT_PROP_RELIABLE_WRITE,	"reliable-write" },
	{ BT_GATT_CHRC_EXT_PROP_WRITABLE_AUX,	"writable-auxiliaries" }
};

static gboolean characteristic_get_flags(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct characteristic *chrc = data;
	DBusMessageIter array;
	unsigned i;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "s", &array);

	for (i = 0; i < NELEM(chrc_props); i++) {
		if (chrc->props & chrc_props[i].prop)
			dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING,
							&chrc_props[i].str);
	}

	for (i = 0; i < NELEM(chrc_ext_props); i++) {
		if (chrc->ext_props & chrc_ext_props[i].prop)
			dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING,
							&chrc_ext_props[i].str);
	}

	dbus_message_iter_close_container(iter, &array);

	return TRUE;
}

static void write_characteristic_cb(struct gatt_db_attribute *attr, int err,
								void *user_data)
{
	struct characteristic *chrc = user_data;

	if (err)
		return;

	g_dbus_emit_property_changed_full(btd_get_dbus_connection(),
				chrc->path, GATT_CHARACTERISTIC_IFACE,
				"Value", G_DBUS_PROPERTY_CHANGED_FLAG_FLUSH);

}

static void chrc_read_cb(bool success, uint8_t att_ecode, const uint8_t *value,
					uint16_t length, void *user_data)
{
	struct async_dbus_op *op = user_data;
	struct characteristic *chrc = op->data;

	if (!success)
		goto fail;

	if (!op->offset)
		gatt_db_attribute_reset(chrc->attr);

	if (!gatt_db_attribute_write(chrc->attr, op->offset, value, length, 0,
					NULL, write_characteristic_cb, chrc)) {
		error("Failed to store attribute");
		att_ecode = BT_ATT_ERROR_UNLIKELY;
		goto fail;
	}

	/* Read the stored data from db */
	if (!gatt_db_attribute_read(chrc->attr, 0, 0, NULL, read_op_cb, op)) {
		error("Failed to read database");
		att_ecode = BT_ATT_ERROR_UNLIKELY;
		goto fail;
	}

	chrc->read_op = NULL;

	return;

fail:
	async_dbus_op_reply(op, att_ecode, NULL, 0);
	chrc->read_op = NULL;
}

static DBusMessage *characteristic_read_value(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct characteristic *chrc = user_data;
	struct bt_gatt_client *gatt = chrc->service->client->gatt;
	DBusMessageIter iter;
	uint16_t offset = 0;

	if (!gatt)
		return btd_error_failed(msg, "Not connected");

	dbus_message_iter_init(msg, &iter);

	if (parse_options(&iter, &offset))
		return btd_error_invalid_args(msg);

	if (chrc->read_op) {
		if (chrc->read_op->offset != offset)
			return btd_error_in_progress(msg);
		queue_push_tail(chrc->read_op->msgs, dbus_message_ref(msg));
		return NULL;
	}

	chrc->read_op = read_value(gatt, msg, chrc->value_handle, offset,
							chrc_read_cb, chrc);
	if (!chrc->read_op)
		return btd_error_failed(msg, "Failed to send read request");

	return NULL;
}

static bool chrc_write_complete(void *data)
{
	struct characteristic *chrc = data;

	chrc->write_op = NULL;

	/*
	 * The characteristic might have been unregistered during the read.
	 * Return failure.
	 */
	return !!chrc->service;
}

static DBusMessage *characteristic_write_value(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct characteristic *chrc = user_data;
	struct bt_gatt_client *gatt = chrc->service->client->gatt;
	DBusMessageIter iter;
	uint8_t *value = NULL;
	int value_len = 0;
	bool supported = false;
	uint16_t offset = 0;

	if (!gatt)
		return btd_error_failed(msg, "Not connected");

	if (chrc->write_op)
		return btd_error_in_progress(msg);

	dbus_message_iter_init(msg, &iter);

	if (parse_value_arg(&iter, &value, &value_len))
		return btd_error_invalid_args(msg);

	if (parse_options(&iter, &offset))
		return btd_error_invalid_args(msg);

	/*
	 * Decide which write to use based on characteristic properties. For now
	 * we don't perform signed writes since gatt-client doesn't support them
	 * and the user can always encrypt the through pairing. The procedure to
	 * use is determined based on the following priority:
	 *
	 *   * "reliable-write" property set -> reliable long-write.
	 *   * "write" property set -> write request.
	 *     - If value is larger than MTU - 3: long-write
	 *   * "write-without-response" property set -> write command.
	 */
	if ((chrc->ext_props & BT_GATT_CHRC_EXT_PROP_RELIABLE_WRITE)) {
		supported = true;
		chrc->write_op = start_long_write(msg, chrc->value_handle, gatt,
						true, value, value_len, offset,
						chrc, chrc_write_complete);
		if (chrc->write_op)
			return NULL;
	}

	if (chrc->props & BT_GATT_CHRC_PROP_WRITE) {
		uint16_t mtu;

		supported = true;
		mtu = bt_gatt_client_get_mtu(gatt);
		if (!mtu)
			return btd_error_failed(msg, "No ATT transport");

		if (value_len <= mtu - 3 && !offset)
			chrc->write_op = start_write_request(msg,
						chrc->value_handle,
						gatt, value, value_len,
						chrc, chrc_write_complete);
		else
			chrc->write_op = start_long_write(msg,
						chrc->value_handle, gatt,
						false, value, value_len, offset,
						chrc, chrc_write_complete);

		if (chrc->write_op)
			return NULL;
	}

	if (!(chrc->props & BT_GATT_CHRC_PROP_WRITE_WITHOUT_RESP))
		goto fail;

	supported = true;

	if (bt_gatt_client_write_without_response(gatt,
					chrc->value_handle,
					chrc->props & BT_GATT_CHRC_PROP_AUTH,
					value, value_len))
		return dbus_message_new_method_return(msg);

fail:
	if (supported)
		return btd_error_failed(msg, "Failed to initiate write");

	return btd_error_not_supported(msg);
}

struct notify_client {
	struct characteristic *chrc;
	int ref_count;
	char *owner;
	guint watch;
	unsigned int notify_id;
};

static void notify_client_free(struct notify_client *client)
{
	DBG("owner %s", client->owner);

	g_dbus_remove_watch(btd_get_dbus_connection(), client->watch);
	bt_gatt_client_unregister_notify(client->chrc->service->client->gatt,
							client->notify_id);
	free(client->owner);
	free(client);
}

static void notify_client_unref(void *data)
{
	struct notify_client *client = data;

	DBG("owner %s", client->owner);

	if (__sync_sub_and_fetch(&client->ref_count, 1))
		return;

	notify_client_free(client);
}

static struct notify_client *notify_client_ref(struct notify_client *client)
{
	DBG("owner %s", client->owner);

	__sync_fetch_and_add(&client->ref_count, 1);

	return client;
}

static bool match_notifying(const void *a, const void *b)
{
	const struct notify_client *client = a;

	return !!client->notify_id;
}

static void update_notifying(struct characteristic *chrc)
{
	if (!chrc->notifying)
		return;

	if (queue_find(chrc->notify_clients, match_notifying, NULL))
		return;

	chrc->notifying = false;

	g_dbus_emit_property_changed(btd_get_dbus_connection(), chrc->path,
						GATT_CHARACTERISTIC_IFACE,
						"Notifying");
}

static void notify_client_disconnect(DBusConnection *conn, void *user_data)
{
	struct notify_client *client = user_data;
	struct characteristic *chrc = client->chrc;

	DBG("owner %s", client->owner);

	queue_remove(chrc->notify_clients, client);
	queue_remove(chrc->service->client->all_notify_clients, client);

	update_notifying(chrc);

	notify_client_unref(client);
}

static struct notify_client *notify_client_create(struct characteristic *chrc,
							const char *owner)
{
	struct notify_client *client;

	client = new0(struct notify_client, 1);
	client->chrc = chrc;
	client->owner = strdup(owner);
	if (!client->owner) {
		free(client);
		return NULL;
	}

	client->watch = g_dbus_add_disconnect_watch(btd_get_dbus_connection(),
						owner, notify_client_disconnect,
						client, NULL);
	if (!client->watch) {
		free(client->owner);
		free(client);
		return NULL;
	}

	return notify_client_ref(client);
}

static bool match_notify_sender(const void *a, const void *b)
{
	const struct notify_client *client = a;
	const char *sender = b;

	return strcmp(client->owner, sender) == 0;
}

static void notify_cb(uint16_t value_handle, const uint8_t *value,
					uint16_t length, void *user_data)
{
	struct async_dbus_op *op = user_data;
	struct notify_client *client = op->data;
	struct characteristic *chrc = client->chrc;

	/*
	 * Even if the value didn't change, we want to send a PropertiesChanged
	 * signal so that we propagate the notification/indication to
	 * applications.
	 */
	gatt_db_attribute_reset(chrc->attr);
	gatt_db_attribute_write(chrc->attr, 0, value, length, 0, NULL,
						write_characteristic_cb, chrc);
}

static void create_notify_reply(struct async_dbus_op *op, bool success,
							uint8_t att_ecode)
{
	int err;

	if (success)
		err = 0;
	else
		err = att_ecode ? att_ecode : -ENOENT;

	async_dbus_op_reply(op, err, NULL, 0);
}

static void register_notify_cb(uint16_t att_ecode, void *user_data)
{
	struct async_dbus_op *op = user_data;
	struct notify_client *client = op->data;
	struct characteristic *chrc = client->chrc;

	if (att_ecode) {
		queue_remove(chrc->notify_clients, client);
		queue_remove(chrc->service->client->all_notify_clients, client);
		notify_client_free(client);

		create_notify_reply(op, false, att_ecode);

		return;
	}

	if (!chrc->notifying) {
		chrc->notifying = true;
		g_dbus_emit_property_changed(btd_get_dbus_connection(),
					chrc->path, GATT_CHARACTERISTIC_IFACE,
					"Notifying");
	}

	create_notify_reply(op, true, 0);
}

static DBusMessage *characteristic_start_notify(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct characteristic *chrc = user_data;
	struct bt_gatt_client *gatt = chrc->service->client->gatt;
	const char *sender = dbus_message_get_sender(msg);
	struct async_dbus_op *op;
	struct notify_client *client;

	if (!(chrc->props & BT_GATT_CHRC_PROP_NOTIFY ||
				chrc->props & BT_GATT_CHRC_PROP_INDICATE))
		return btd_error_not_supported(msg);

	/* Each client can only have one active notify session. */
	client = queue_find(chrc->notify_clients, match_notify_sender, sender);
	if (client)
		return client->notify_id ?
				btd_error_failed(msg, "Already notifying") :
				btd_error_in_progress(msg);

	client = notify_client_create(chrc, sender);
	if (!client)
		return btd_error_failed(msg, "Failed allocate notify session");

	queue_push_tail(chrc->notify_clients, client);
	queue_push_tail(chrc->service->client->all_notify_clients, client);

	/*
	 * If the device is currently not connected, return success. We will
	 * automatically try and register all clients when a GATT client becomes
	 * ready.
	 */
	if (!gatt) {
		DBusMessage *reply;

		reply = g_dbus_create_reply(msg, DBUS_TYPE_INVALID);
		if (reply)
			return reply;

		/*
		 * Clean up and respond with an error instead of timing out to
		 * avoid any ambiguities.
		 */
		error("Failed to construct D-Bus message reply");
		goto fail;
	}

	op = async_dbus_op_new(msg, client);

	client->notify_id = bt_gatt_client_register_notify(gatt,
						chrc->value_handle,
						register_notify_cb, notify_cb,
						op, async_dbus_op_free);
	if (client->notify_id)
		return NULL;

	async_dbus_op_free(op);

fail:
	queue_remove(chrc->notify_clients, client);
	queue_remove(chrc->service->client->all_notify_clients, client);

	/* Directly free the client */
	notify_client_free(client);

	return btd_error_failed(msg, "Failed to register notify session");
}

static DBusMessage *characteristic_stop_notify(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct characteristic *chrc = user_data;
	struct bt_gatt_client *gatt = chrc->service->client->gatt;
	const char *sender = dbus_message_get_sender(msg);
	struct notify_client *client;

	client = queue_remove_if(chrc->notify_clients, match_notify_sender,
							(void *) sender);
	if (!client)
		return btd_error_failed(msg, "No notify session started");

	queue_remove(chrc->service->client->all_notify_clients, client);
	bt_gatt_client_unregister_notify(gatt, client->notify_id);
	update_notifying(chrc);

	notify_client_unref(client);

	return dbus_message_new_method_return(msg);
}

static const GDBusPropertyTable characteristic_properties[] = {
	{ "UUID", "s", characteristic_get_uuid, NULL, NULL,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{ "Service", "o", characteristic_get_service, NULL, NULL,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{ "Value", "ay", characteristic_get_value, NULL,
					characteristic_value_exists,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{ "Notifying", "b", characteristic_get_notifying, NULL,
					characteristic_notifying_exists,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{ "Flags", "as", characteristic_get_flags, NULL, NULL,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{ }
};

static const GDBusMethodTable characteristic_methods[] = {
	{ GDBUS_EXPERIMENTAL_ASYNC_METHOD("ReadValue",
					GDBUS_ARGS({ "options", "a{sv}" }),
					GDBUS_ARGS({ "value", "ay" }),
					characteristic_read_value) },
	{ GDBUS_EXPERIMENTAL_ASYNC_METHOD("WriteValue",
					GDBUS_ARGS({ "value", "ay" },
						{ "options", "a{sv}" }),
					NULL,
					characteristic_write_value) },
	{ GDBUS_EXPERIMENTAL_ASYNC_METHOD("StartNotify", NULL, NULL,
					characteristic_start_notify) },
	{ GDBUS_EXPERIMENTAL_METHOD("StopNotify", NULL, NULL,
					characteristic_stop_notify) },
	{ }
};

static void characteristic_free(void *data)
{
	struct characteristic *chrc = data;

	/* List should be empty here */
	queue_destroy(chrc->descs, NULL);
	queue_destroy(chrc->notify_clients, NULL);

	g_free(chrc->path);
	free(chrc);
}

static struct characteristic *characteristic_create(
						struct gatt_db_attribute *attr,
						struct service *service)
{
	struct characteristic *chrc;
	bt_uuid_t uuid;

	chrc = new0(struct characteristic, 1);
	chrc->descs = queue_new();
	chrc->notify_clients = queue_new();
	chrc->service = service;

	gatt_db_attribute_get_char_data(attr, &chrc->handle,
							&chrc->value_handle,
							&chrc->props,
							&chrc->ext_props,
							&uuid);

	chrc->attr = gatt_db_get_attribute(service->client->db,
							chrc->value_handle);
	if (!chrc->attr) {
		error("Attribute 0x%04x not found", chrc->value_handle);
		characteristic_free(chrc);
		return NULL;
	}

	bt_uuid_to_uuid128(&uuid, &chrc->uuid);

	chrc->path = g_strdup_printf("%s/char%04x", service->path,
								chrc->handle);

	if (!g_dbus_register_interface(btd_get_dbus_connection(), chrc->path,
						GATT_CHARACTERISTIC_IFACE,
						characteristic_methods, NULL,
						characteristic_properties,
						chrc, characteristic_free)) {
		error("Unable to register GATT characteristic with handle "
							"0x%04x", chrc->handle);
		characteristic_free(chrc);

		return NULL;
	}

	DBG("Exported GATT characteristic: %s", chrc->path);

	return chrc;
}

static void remove_client(void *data)
{
	struct notify_client *ntfy_client = data;
	struct btd_gatt_client *client = ntfy_client->chrc->service->client;

	queue_remove(client->all_notify_clients, ntfy_client);

	notify_client_unref(ntfy_client);
}

static void unregister_characteristic(void *data)
{
	struct characteristic *chrc = data;
	struct bt_gatt_client *gatt = chrc->service->client->gatt;

	DBG("Removing GATT characteristic: %s", chrc->path);

	if (chrc->read_op)
		bt_gatt_client_cancel(gatt, chrc->read_op->id);

	if (chrc->write_op)
		bt_gatt_client_cancel(gatt, chrc->write_op->id);

	queue_remove_all(chrc->notify_clients, NULL, NULL, remove_client);
	queue_remove_all(chrc->descs, NULL, NULL, unregister_descriptor);

	g_dbus_unregister_interface(btd_get_dbus_connection(), chrc->path,
						GATT_CHARACTERISTIC_IFACE);
}

static gboolean service_get_uuid(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	char uuid[MAX_LEN_UUID_STR + 1];
	const char *ptr = uuid;
	struct service *service = data;

	bt_uuid_to_string(&service->uuid, uuid, sizeof(uuid));
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &ptr);

	return TRUE;
}

static gboolean service_get_device(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct service *service = data;
	const char *str = device_get_path(service->client->device);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &str);

	return TRUE;
}

static gboolean service_get_primary(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct service *service = data;
	dbus_bool_t primary;

	primary = service->primary ? TRUE : FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &primary);

	return TRUE;
}

static const GDBusPropertyTable service_properties[] = {
	{ "UUID", "s", service_get_uuid, NULL, NULL,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{ "Device", "o", service_get_device, NULL, NULL,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{ "Primary", "b", service_get_primary, NULL, NULL,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{ }
};

static void service_free(void *data)
{
	struct service *service = data;

	queue_destroy(service->chrcs, NULL);  /* List should be empty here */
	g_free(service->path);
	free(service);
}

static struct service *service_create(struct gatt_db_attribute *attr,
						struct btd_gatt_client *client)
{
	struct service *service;
	const char *device_path = device_get_path(client->device);
	bt_uuid_t uuid;

	service = new0(struct service, 1);
	service->chrcs = queue_new();
	service->client = client;

	gatt_db_attribute_get_service_data(attr, &service->start_handle,
							&service->end_handle,
							&service->primary,
							&uuid);
	bt_uuid_to_uuid128(&uuid, &service->uuid);

	service->path = g_strdup_printf("%s/service%04x", device_path,
							service->start_handle);

	if (!g_dbus_register_interface(btd_get_dbus_connection(), service->path,
						GATT_SERVICE_IFACE,
						NULL, NULL,
						service_properties,
						service, service_free)) {
		error("Unable to register GATT service with handle 0x%04x for "
							"device %s",
							service->start_handle,
							client->devaddr);
		service_free(service);

		return NULL;
	}

	DBG("Exported GATT service: %s", service->path);

	/* Set service active so we can skip discovering next time */
	gatt_db_service_set_active(attr, true);

	/* Mark the service as claimed since it going to be exported */
	gatt_db_service_set_claimed(attr, true);

	return service;
}

static void unregister_service(void *data)
{
	struct service *service = data;

	DBG("Removing GATT service: %s", service->path);

	queue_remove_all(service->chrcs, NULL, NULL, unregister_characteristic);

	g_dbus_unregister_interface(btd_get_dbus_connection(), service->path,
							GATT_SERVICE_IFACE);
}

struct export_data {
	void *root;
	bool failed;
};

static void export_desc(struct gatt_db_attribute *attr, void *user_data)
{
	struct descriptor *desc;
	struct export_data *data = user_data;
	struct characteristic *charac = data->root;

	if (data->failed)
		return;

	desc = descriptor_create(attr, charac);
	if (!desc) {
		data->failed = true;
		return;
	}

	queue_push_tail(charac->descs, desc);
}

static bool create_descriptors(struct gatt_db_attribute *attr,
					struct characteristic *charac)
{
	struct export_data data;

	data.root = charac;
	data.failed = false;

	gatt_db_service_foreach_desc(attr, export_desc, &data);

	return !data.failed;
}

static void export_char(struct gatt_db_attribute *attr, void *user_data)
{
	struct characteristic *charac;
	struct export_data *data = user_data;
	struct service *service = data->root;

	if (data->failed)
		return;

	charac = characteristic_create(attr, service);
	if (!charac)
		goto fail;

	if (!create_descriptors(attr, charac)) {
		unregister_characteristic(charac);
		goto fail;
	}

	queue_push_tail(service->chrcs, charac);

	return;

fail:
	data->failed = true;
}

static bool create_characteristics(struct gatt_db_attribute *attr,
						struct service *service)
{
	struct export_data data;

	data.root = service;
	data.failed = false;

	gatt_db_service_foreach_char(attr, export_char, &data);

	return !data.failed;
}

static void export_service(struct gatt_db_attribute *attr, void *user_data)
{
	struct btd_gatt_client *client = user_data;
	struct service *service;

	if (gatt_db_service_get_claimed(attr))
		return;

	service = service_create(attr, client);
	if (!service)
		return;

	if (!create_characteristics(attr, service)) {
		error("Exporting characteristics failed");
		unregister_service(service);
		return;
	}

	queue_push_tail(client->services, service);
}

static void create_services(struct btd_gatt_client *client)
{
	/* Don't attempt to create any objects if experimental is disabled */
	if (!(g_dbus_get_flags() & G_DBUS_FLAG_ENABLE_EXPERIMENTAL)) {
		info("GATT service objects disabled");
		return;
	}

	DBG("Exporting objects for GATT services: %s", client->devaddr);

	gatt_db_foreach_service(client->db, NULL, export_service, client);
}

struct btd_gatt_client *btd_gatt_client_new(struct btd_device *device)
{
	struct btd_gatt_client *client;
	struct gatt_db *db;

	if (!device)
		return NULL;

	db = btd_device_get_gatt_db(device);
	if (!db)
		return NULL;

	client = new0(struct btd_gatt_client, 1);
	client->services = queue_new();
	client->all_notify_clients = queue_new();
	client->device = device;
	ba2str(device_get_address(device), client->devaddr);

	client->db = gatt_db_ref(db);

	return client;
}

void btd_gatt_client_destroy(struct btd_gatt_client *client)
{
	if (!client)
		return;

	queue_destroy(client->services, unregister_service);
	queue_destroy(client->all_notify_clients, NULL);
	bt_gatt_client_unref(client->gatt);
	gatt_db_unref(client->db);
	free(client);
}

static void register_notify(void *data, void *user_data)
{
	struct notify_client *notify_client = data;
	struct btd_gatt_client *client = user_data;
	struct async_dbus_op *op;

	DBG("Re-register subscribed notification client");

	op = new0(struct async_dbus_op, 1);
	op->data = notify_client;

	notify_client->notify_id = bt_gatt_client_register_notify(client->gatt,
					notify_client->chrc->value_handle,
					register_notify_cb, notify_cb,
					op, async_dbus_op_free);
	if (notify_client->notify_id)
		return;

	async_dbus_op_free(op);

	DBG("Failed to re-register notification client");

	queue_remove(notify_client->chrc->notify_clients, notify_client);
	queue_remove(client->all_notify_clients, notify_client);

	notify_client_free(notify_client);
}

void btd_gatt_client_ready(struct btd_gatt_client *client)
{
	if (!client)
		return;

	if (!client->gatt) {
		struct bt_gatt_client *gatt;

		gatt = btd_device_get_gatt_client(client->device);
		client->gatt = bt_gatt_client_clone(gatt);
		if (!client->gatt) {
			error("GATT client not initialized");
			return;
		}
	}

	client->ready = true;

	DBG("GATT client ready");

	create_services(client);
}

void btd_gatt_client_connected(struct btd_gatt_client *client)
{
	struct bt_gatt_client *gatt;

	gatt = btd_device_get_gatt_client(client->device);
	if (!gatt) {
		error("GATT client not initialized");
		return;
	}

	DBG("Device connected.");

	bt_gatt_client_unref(client->gatt);
	client->gatt = bt_gatt_client_clone(gatt);

	/*
	 * Services have already been created before. Re-enable notifications
	 * for any pre-registered notification sessions.
	 */
	queue_foreach(client->all_notify_clients, register_notify, client);
}

void btd_gatt_client_service_added(struct btd_gatt_client *client,
					struct gatt_db_attribute *attrib)
{
	if (!client || !attrib || !client->ready)
		return;

	export_service(attrib, client);
}

static bool match_service_handle(const void *a, const void *b)
{
	const struct service *service = a;
	uint16_t start_handle = PTR_TO_UINT(b);

	return service->start_handle == start_handle;
}

void btd_gatt_client_service_removed(struct btd_gatt_client *client,
					struct gatt_db_attribute *attrib)
{
	uint16_t start_handle, end_handle;

	if (!client || !attrib || !client->ready)
		return;

	gatt_db_attribute_get_service_handles(attrib, &start_handle,
								&end_handle);

	DBG("GATT Services Removed - start: 0x%04x, end: 0x%04x", start_handle,
								end_handle);
	queue_remove_all(client->services, match_service_handle,
						UINT_TO_PTR(start_handle),
						unregister_service);
}

static void clear_notify_id(void *data, void *user_data)
{
	struct notify_client *client = data;

	client->notify_id = 0;
}

void btd_gatt_client_disconnected(struct btd_gatt_client *client)
{
	if (!client || !client->gatt)
		return;

	DBG("Device disconnected. Cleaning up.");

	/*
	 * TODO: Once GATT over BR/EDR is properly supported, we should pass the
	 * correct bdaddr_type based on the transport over which GATT is being
	 * done.
	 */
	queue_foreach(client->all_notify_clients, clear_notify_id, NULL);

	bt_gatt_client_unref(client->gatt);
	client->gatt = NULL;
}

struct foreach_service_data {
	btd_gatt_client_service_path_t func;
	void *user_data;
};

static void client_service_foreach(void *data, void *user_data)
{
	struct service *service = data;
	struct foreach_service_data *foreach_data = user_data;

	foreach_data->func(service->path, foreach_data->user_data);
}

void btd_gatt_client_foreach_service(struct btd_gatt_client *client,
					btd_gatt_client_service_path_t func,
					void *user_data)
{
	struct foreach_service_data data;

	if (!client)
		return;

	data.func = func;
	data.user_data = user_data;

	queue_foreach(client->services, client_service_foreach, &data);
}
