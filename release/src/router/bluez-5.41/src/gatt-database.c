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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "lib/uuid.h"
#include "btio/btio.h"
#include "gdbus/gdbus.h"
#include "src/shared/util.h"
#include "src/shared/queue.h"
#include "src/shared/att.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-server.h"
#include "src/shared/timeout.h"
#include "log.h"
#include "error.h"
#include "adapter.h"
#include "device.h"
#include "gatt-database.h"
#include "dbus-common.h"
#include "profile.h"
#include "service.h"

#ifndef ATT_CID
#define ATT_CID 4
#endif

#ifndef ATT_PSM
#define ATT_PSM 31
#endif

#define GATT_MANAGER_IFACE	"org.bluez.GattManager1"
#define GATT_PROFILE_IFACE	"org.bluez.GattProfile1"
#define GATT_SERVICE_IFACE	"org.bluez.GattService1"
#define GATT_CHRC_IFACE		"org.bluez.GattCharacteristic1"
#define GATT_DESC_IFACE		"org.bluez.GattDescriptor1"

#define UUID_GAP        0x1800
#define UUID_GATT       0x1801

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

struct gatt_app {
	struct btd_gatt_database *database;
	char *owner;
	char *path;
	DBusMessage *reg;
	GDBusClient *client;
	bool failed;
	struct queue *profiles;
	struct queue *services;
	struct queue *proxies;
};

struct external_service {
	struct gatt_app *app;
	char *path;	/* Path to GattService1 */
	GDBusProxy *proxy;
	struct gatt_db_attribute *attrib;
	uint16_t attr_cnt;
	struct queue *chrcs;
	struct queue *descs;
};

struct external_profile {
	struct gatt_app *app;
	GDBusProxy *proxy;
	struct queue *profiles; /* btd_profile list */
};

struct external_chrc {
	struct external_service *service;
	char *path;
	GDBusProxy *proxy;
	uint8_t props;
	uint8_t ext_props;
	uint32_t perm;
	struct gatt_db_attribute *attrib;
	struct gatt_db_attribute *ccc;
	struct queue *pending_reads;
	struct queue *pending_writes;
	unsigned int ntfy_cnt;
};

struct external_desc {
	struct external_service *service;
	char *chrc_path;
	GDBusProxy *proxy;
	uint32_t perm;
	struct gatt_db_attribute *attrib;
	bool handled;
	struct queue *pending_reads;
	struct queue *pending_writes;
};

struct pending_op {
	struct btd_device *device;
	unsigned int id;
	struct gatt_db_attribute *attrib;
	struct queue *owner_queue;
	struct iovec data;
};

struct device_state {
	bdaddr_t bdaddr;
	uint8_t bdaddr_type;
	struct queue *ccc_states;
};

struct ccc_state {
	uint16_t handle;
	uint8_t value[2];
};

struct ccc_cb_data {
	uint16_t handle;
	btd_gatt_database_ccc_write_t callback;
	btd_gatt_database_destroy_t destroy;
	void *user_data;
};

struct device_info {
	bdaddr_t bdaddr;
	uint8_t bdaddr_type;
};

static void ccc_cb_free(void *data)
{
	struct ccc_cb_data *ccc_cb = data;

	if (ccc_cb->destroy)
		ccc_cb->destroy(ccc_cb->user_data);

	free(ccc_cb);
}

static bool ccc_cb_match_service(const void *data, const void *match_data)
{
	const struct ccc_cb_data *ccc_cb = data;
	const struct gatt_db_attribute *attrib = match_data;
	uint16_t start, end;

	if (!gatt_db_attribute_get_service_handles(attrib, &start, &end))
		return false;

	return ccc_cb->handle >= start && ccc_cb->handle <= end;
}

static bool ccc_cb_match_handle(const void *data, const void *match_data)
{
	const struct ccc_cb_data *ccc_cb = data;
	uint16_t handle = PTR_TO_UINT(match_data);

	return ccc_cb->handle == handle;
}

static bool dev_state_match(const void *a, const void *b)
{
	const struct device_state *dev_state = a;
	const struct device_info *dev_info = b;

	return bacmp(&dev_state->bdaddr, &dev_info->bdaddr) == 0 &&
				dev_state->bdaddr_type == dev_info->bdaddr_type;
}

static struct device_state *
find_device_state(struct btd_gatt_database *database, bdaddr_t *bdaddr,
							uint8_t bdaddr_type)
{
	struct device_info dev_info;

	memset(&dev_info, 0, sizeof(dev_info));

	bacpy(&dev_info.bdaddr, bdaddr);
	dev_info.bdaddr_type = bdaddr_type;

	return queue_find(database->device_states, dev_state_match, &dev_info);
}

static bool ccc_state_match(const void *a, const void *b)
{
	const struct ccc_state *ccc = a;
	uint16_t handle = PTR_TO_UINT(b);

	return ccc->handle == handle;
}

static struct ccc_state *find_ccc_state(struct device_state *dev_state,
								uint16_t handle)
{
	return queue_find(dev_state->ccc_states, ccc_state_match,
							UINT_TO_PTR(handle));
}

static struct device_state *device_state_create(bdaddr_t *bdaddr,
							uint8_t bdaddr_type)
{
	struct device_state *dev_state;

	dev_state = new0(struct device_state, 1);
	dev_state->ccc_states = queue_new();
	bacpy(&dev_state->bdaddr, bdaddr);
	dev_state->bdaddr_type = bdaddr_type;

	return dev_state;
}

static struct device_state *get_device_state(struct btd_gatt_database *database,
							bdaddr_t *bdaddr,
							uint8_t bdaddr_type)
{
	struct device_state *dev_state;

	/*
	 * Find and return a device state. If a matching state doesn't exist,
	 * then create a new one.
	 */
	dev_state = find_device_state(database, bdaddr, bdaddr_type);
	if (dev_state)
		return dev_state;

	dev_state = device_state_create(bdaddr, bdaddr_type);

	queue_push_tail(database->device_states, dev_state);

	return dev_state;
}

static struct ccc_state *get_ccc_state(struct btd_gatt_database *database,
							bdaddr_t *bdaddr,
							uint8_t bdaddr_type,
							uint16_t handle)
{
	struct device_state *dev_state;
	struct ccc_state *ccc;

	dev_state = get_device_state(database, bdaddr, bdaddr_type);

	ccc = find_ccc_state(dev_state, handle);
	if (ccc)
		return ccc;

	ccc = new0(struct ccc_state, 1);
	ccc->handle = handle;
	queue_push_tail(dev_state->ccc_states, ccc);

	return ccc;
}

static void device_state_free(void *data)
{
	struct device_state *state = data;

	queue_destroy(state->ccc_states, free);
	free(state);
}

static void cancel_pending_read(void *data)
{
	struct pending_op *op = data;

	gatt_db_attribute_read_result(op->attrib, op->id,
					BT_ATT_ERROR_REQUEST_NOT_SUPPORTED,
					NULL, 0);
	op->owner_queue = NULL;
}

static void cancel_pending_write(void *data)
{
	struct pending_op *op = data;

	gatt_db_attribute_write_result(op->attrib, op->id,
					BT_ATT_ERROR_REQUEST_NOT_SUPPORTED);
	op->owner_queue = NULL;
}

static void chrc_free(void *data)
{
	struct external_chrc *chrc = data;

	queue_destroy(chrc->pending_reads, cancel_pending_read);
	queue_destroy(chrc->pending_writes, cancel_pending_write);

	g_free(chrc->path);

	g_dbus_proxy_set_property_watch(chrc->proxy, NULL, NULL);
	g_dbus_proxy_unref(chrc->proxy);

	free(chrc);
}

static void desc_free(void *data)
{
	struct external_desc *desc = data;

	queue_destroy(desc->pending_reads, cancel_pending_read);
	queue_destroy(desc->pending_writes, cancel_pending_write);

	g_dbus_proxy_unref(desc->proxy);
	g_free(desc->chrc_path);

	free(desc);
}

static void service_free(void *data)
{
	struct external_service *service = data;

	queue_destroy(service->chrcs, chrc_free);
	queue_destroy(service->descs, desc_free);

	if (service->attrib)
		gatt_db_remove_service(service->app->database->db,
							service->attrib);

	if (service->app->client)
		g_dbus_proxy_unref(service->proxy);

	g_free(service->path);

	free(service);
}

static void profile_remove(void *data)
{
	struct btd_profile *p = data;

	DBG("Removed \"%s\"", p->name);

	adapter_foreach(adapter_remove_profile, p);
	btd_profile_unregister(p);

	g_free((void *) p->name);
	g_free((void *) p->remote_uuid);

	free(p);
}

static void profile_release(struct external_profile *profile)
{
	DBG("Releasing \"%s\"", profile->app->owner);

	g_dbus_proxy_method_call(profile->proxy, "Release", NULL, NULL, NULL,
									NULL);
}

static void profile_free(void *data)
{
	struct external_profile *profile = data;

	queue_destroy(profile->profiles, profile_remove);

	profile_release(profile);

	g_dbus_proxy_unref(profile->proxy);

	free(profile);
}

static void app_free(void *data)
{
	struct gatt_app *app = data;

	queue_destroy(app->profiles, profile_free);
	queue_destroy(app->services, service_free);
	queue_destroy(app->proxies, NULL);

	if (app->client) {
		g_dbus_client_set_disconnect_watch(app->client, NULL, NULL);
		g_dbus_client_set_proxy_handlers(app->client, NULL, NULL,
								NULL, NULL);
		g_dbus_client_set_ready_watch(app->client, NULL, NULL);
		g_dbus_client_unref(app->client);
	}

	if (app->reg)
		dbus_message_unref(app->reg);

	g_free(app->owner);
	g_free(app->path);

	free(app);
}

static void gatt_database_free(void *data)
{
	struct btd_gatt_database *database = data;

	if (database->le_io) {
		g_io_channel_shutdown(database->le_io, FALSE, NULL);
		g_io_channel_unref(database->le_io);
	}

	if (database->l2cap_io) {
		g_io_channel_shutdown(database->l2cap_io, FALSE, NULL);
		g_io_channel_unref(database->l2cap_io);
	}

	if (database->gatt_handle)
		adapter_service_remove(database->adapter, database->gatt_handle);

	if (database->gap_handle)
		adapter_service_remove(database->adapter, database->gap_handle);

	/* TODO: Persistently store CCC states before freeing them */
	gatt_db_unregister(database->db, database->db_id);

	queue_destroy(database->device_states, device_state_free);
	queue_destroy(database->apps, app_free);
	queue_destroy(database->profiles, profile_free);
	queue_destroy(database->ccc_callbacks, ccc_cb_free);
	database->device_states = NULL;
	database->ccc_callbacks = NULL;

	gatt_db_unref(database->db);

	btd_adapter_unref(database->adapter);
	free(database);
}

static void connect_cb(GIOChannel *io, GError *gerr, gpointer user_data)
{
	struct btd_adapter *adapter;
	struct btd_device *device;
	uint8_t dst_type;
	bdaddr_t src, dst;

	if (gerr) {
		error("%s", gerr->message);
		return;
	}

	bt_io_get(io, &gerr, BT_IO_OPT_SOURCE_BDADDR, &src,
						BT_IO_OPT_DEST_BDADDR, &dst,
						BT_IO_OPT_DEST_TYPE, &dst_type,
						BT_IO_OPT_INVALID);
	if (gerr) {
		error("bt_io_get: %s", gerr->message);
		g_error_free(gerr);
		return;
	}

	DBG("New incoming %s ATT connection", dst_type == BDADDR_BREDR ?
							"BR/EDR" : "LE");

	adapter = adapter_find(&src);
	if (!adapter)
		return;

	device = btd_adapter_get_device(adapter, &dst, dst_type);
	if (!device)
		return;

	device_attach_att(device, io);
}

static void gap_device_name_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	struct btd_gatt_database *database = user_data;
	uint8_t error = 0;
	size_t len = 0;
	const uint8_t *value = NULL;
	const char *device_name;

	DBG("GAP Device Name read request\n");

	device_name = btd_adapter_get_name(database->adapter);
	len = strlen(device_name);

	if (offset > len) {
		error = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	len -= offset;
	value = len ? (const uint8_t *) &device_name[offset] : NULL;

done:
	gatt_db_attribute_read_result(attrib, id, error, value, len);
}

static void gap_appearance_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	struct btd_gatt_database *database = user_data;
	uint8_t error = 0;
	size_t len = 2;
	const uint8_t *value = NULL;
	uint8_t appearance[2];
	uint32_t dev_class;

	DBG("GAP Appearance read request\n");

	dev_class = btd_adapter_get_class(database->adapter);

	if (offset > 2) {
		error = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	appearance[0] = dev_class & 0x00ff;
	appearance[1] = (dev_class >> 8) & 0x001f;

	len -= offset;
	value = len ? (uint8_t *)&appearance[offset] : NULL;

done:
	gatt_db_attribute_read_result(attrib, id, error, value, len);
}

static sdp_record_t *record_new(uuid_t *uuid, uint16_t start, uint16_t end)
{
	sdp_list_t *svclass_id, *apseq, *proto[2], *root, *aproto;
	uuid_t root_uuid, proto_uuid, l2cap;
	sdp_record_t *record;
	sdp_data_t *psm, *sh, *eh;
	uint16_t lp = ATT_PSM;

	if (uuid == NULL)
		return NULL;

	if (start > end)
		return NULL;

	record = sdp_record_alloc();
	if (record == NULL)
		return NULL;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(record, root);
	sdp_list_free(root, NULL);

	svclass_id = sdp_list_append(NULL, uuid);
	sdp_set_service_classes(record, svclass_id);
	sdp_list_free(svclass_id, NULL);

	sdp_uuid16_create(&l2cap, L2CAP_UUID);
	proto[0] = sdp_list_append(NULL, &l2cap);
	psm = sdp_data_alloc(SDP_UINT16, &lp);
	proto[0] = sdp_list_append(proto[0], psm);
	apseq = sdp_list_append(NULL, proto[0]);

	sdp_uuid16_create(&proto_uuid, ATT_UUID);
	proto[1] = sdp_list_append(NULL, &proto_uuid);
	sh = sdp_data_alloc(SDP_UINT16, &start);
	proto[1] = sdp_list_append(proto[1], sh);
	eh = sdp_data_alloc(SDP_UINT16, &end);
	proto[1] = sdp_list_append(proto[1], eh);
	apseq = sdp_list_append(apseq, proto[1]);

	aproto = sdp_list_append(NULL, apseq);
	sdp_set_access_protos(record, aproto);

	sdp_data_free(psm);
	sdp_data_free(sh);
	sdp_data_free(eh);
	sdp_list_free(proto[0], NULL);
	sdp_list_free(proto[1], NULL);
	sdp_list_free(apseq, NULL);
	sdp_list_free(aproto, NULL);

	return record;
}

uint32_t database_add_record(struct btd_gatt_database *database,
					uint16_t uuid,
					struct gatt_db_attribute *attr,
					const char *name)
{
	sdp_record_t *record;
	uint16_t start, end;
	uuid_t svc, gap_uuid;

	sdp_uuid16_create(&svc, uuid);
	gatt_db_attribute_get_service_handles(attr, &start, &end);

	record = record_new(&svc, start, end);
	if (!record)
		return 0;

	if (name != NULL)
		sdp_set_info_attr(record, name, "BlueZ", NULL);

	sdp_uuid16_create(&gap_uuid, UUID_GAP);
	if (sdp_uuid_cmp(&svc, &gap_uuid) == 0) {
		sdp_set_url_attr(record, "http://www.bluez.org/",
				"http://www.bluez.org/",
				"http://www.bluez.org/");
	}

	if (adapter_service_add(database->adapter, record) == 0)
		return record->handle;

	sdp_record_free(record);
	return 0;
}

static void populate_gap_service(struct btd_gatt_database *database)
{
	bt_uuid_t uuid;
	struct gatt_db_attribute *service;

	/* Add the GAP service */
	bt_uuid16_create(&uuid, UUID_GAP);
	service = gatt_db_add_service(database->db, &uuid, true, 5);
	database->gap_handle = database_add_record(database, UUID_GAP, service,
						"Generic Access Profile");

	/*
	 * Device Name characteristic.
	 */
	bt_uuid16_create(&uuid, GATT_CHARAC_DEVICE_NAME);
	gatt_db_service_add_characteristic(service, &uuid, BT_ATT_PERM_READ,
							BT_GATT_CHRC_PROP_READ,
							gap_device_name_read_cb,
							NULL, database);

	/*
	 * Device Appearance characteristic.
	 */
	bt_uuid16_create(&uuid, GATT_CHARAC_APPEARANCE);
	gatt_db_service_add_characteristic(service, &uuid, BT_ATT_PERM_READ,
							BT_GATT_CHRC_PROP_READ,
							gap_appearance_read_cb,
							NULL, database);

	gatt_db_service_set_active(service, true);
}

bool get_dst_info(struct bt_att *att, bdaddr_t *dst, uint8_t *dst_type)
{
	GIOChannel *io = NULL;
	GError *gerr = NULL;

	io = g_io_channel_unix_new(bt_att_get_fd(att));
	if (!io)
		return false;

	bt_io_get(io, &gerr, BT_IO_OPT_DEST_BDADDR, dst,
						BT_IO_OPT_DEST_TYPE, dst_type,
						BT_IO_OPT_INVALID);
	if (gerr) {
		error("gatt: bt_io_get: %s", gerr->message);
		g_error_free(gerr);
		g_io_channel_unref(io);
		return false;
	}

	g_io_channel_unref(io);
	return true;
}

static void gatt_ccc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	struct btd_gatt_database *database = user_data;
	struct ccc_state *ccc;
	uint16_t handle;
	uint8_t ecode = 0;
	const uint8_t *value = NULL;
	size_t len = 0;
	bdaddr_t bdaddr;
	uint8_t bdaddr_type;

	handle = gatt_db_attribute_get_handle(attrib);

	DBG("CCC read called for handle: 0x%04x", handle);

	if (offset > 2) {
		ecode = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	if (!get_dst_info(att, &bdaddr, &bdaddr_type)) {
		ecode = BT_ATT_ERROR_UNLIKELY;
		goto done;
	}

	ccc = get_ccc_state(database, &bdaddr, bdaddr_type, handle);

	len = 2 - offset;
	value = len ? &ccc->value[offset] : NULL;

done:
	gatt_db_attribute_read_result(attrib, id, ecode, value, len);
}

static void gatt_ccc_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	struct btd_gatt_database *database = user_data;
	struct ccc_state *ccc;
	struct ccc_cb_data *ccc_cb;
	uint16_t handle;
	uint8_t ecode = 0;
	bdaddr_t bdaddr;
	uint8_t bdaddr_type;

	handle = gatt_db_attribute_get_handle(attrib);

	DBG("CCC write called for handle: 0x%04x", handle);

	if (!value || len != 2) {
		ecode = BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN;
		goto done;
	}

	if (offset > 2) {
		ecode = BT_ATT_ERROR_INVALID_OFFSET;
		goto done;
	}

	if (!get_dst_info(att, &bdaddr, &bdaddr_type)) {
		ecode = BT_ATT_ERROR_UNLIKELY;
		goto done;
	}

	ccc = get_ccc_state(database, &bdaddr, bdaddr_type, handle);

	ccc_cb = queue_find(database->ccc_callbacks, ccc_cb_match_handle,
			UINT_TO_PTR(gatt_db_attribute_get_handle(attrib)));
	if (!ccc_cb) {
		ecode = BT_ATT_ERROR_UNLIKELY;
		goto done;
	}

	/* If value is identical, then just succeed */
	if (ccc->value[0] == value[0] && ccc->value[1] == value[1])
		goto done;

	if (ccc_cb->callback)
		ecode = ccc_cb->callback(get_le16(value), ccc_cb->user_data);

	if (!ecode) {
		ccc->value[0] = value[0];
		ccc->value[1] = value[1];
	}

done:
	gatt_db_attribute_write_result(attrib, id, ecode);
}

struct gatt_db_attribute *
service_add_ccc(struct gatt_db_attribute *service,
				struct btd_gatt_database *database,
				btd_gatt_database_ccc_write_t write_callback,
				void *user_data,
				btd_gatt_database_destroy_t destroy)
{
	struct gatt_db_attribute *ccc;
	struct ccc_cb_data *ccc_cb;
	bt_uuid_t uuid;

	ccc_cb = new0(struct ccc_cb_data, 1);

	bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);
	ccc = gatt_db_service_add_descriptor(service, &uuid,
				BT_ATT_PERM_READ | BT_ATT_PERM_WRITE,
				gatt_ccc_read_cb, gatt_ccc_write_cb, database);
	if (!ccc) {
		error("Failed to create CCC entry in database");
		free(ccc_cb);
		return NULL;
	}

	ccc_cb->handle = gatt_db_attribute_get_handle(ccc);
	ccc_cb->callback = write_callback;
	ccc_cb->destroy = destroy;
	ccc_cb->user_data = user_data;

	queue_push_tail(database->ccc_callbacks, ccc_cb);

	return ccc;
}

struct gatt_db_attribute *
btd_gatt_database_add_ccc(struct btd_gatt_database *database,
				uint16_t service_handle,
				btd_gatt_database_ccc_write_t write_callback,
				void *user_data,
				btd_gatt_database_destroy_t destroy)
{
	struct gatt_db_attribute *service;

	if (!database || !service_handle)
		return NULL;

	service = gatt_db_get_attribute(database->db, service_handle);
	if (!service) {
		error("No service exists with handle: 0x%04x", service_handle);
		return NULL;
	}

	return service_add_ccc(service, database, write_callback, user_data,
								destroy);
}

static void populate_gatt_service(struct btd_gatt_database *database)
{
	bt_uuid_t uuid;
	struct gatt_db_attribute *service;

	/* Add the GATT service */
	bt_uuid16_create(&uuid, UUID_GATT);
	service = gatt_db_add_service(database->db, &uuid, true, 4);
	database->gatt_handle = database_add_record(database, UUID_GATT,
						service,
						"Generic Attribute Profile");

	bt_uuid16_create(&uuid, GATT_CHARAC_SERVICE_CHANGED);
	database->svc_chngd = gatt_db_service_add_characteristic(service, &uuid,
				BT_ATT_PERM_READ, BT_GATT_CHRC_PROP_INDICATE,
				NULL, NULL, database);
	database->svc_chngd_ccc = service_add_ccc(service, database, NULL, NULL,
									NULL);

	gatt_db_service_set_active(service, true);
}

static void register_core_services(struct btd_gatt_database *database)
{
	populate_gap_service(database);
	populate_gatt_service(database);
}

struct notify {
	struct btd_gatt_database *database;
	uint16_t handle, ccc_handle;
	const uint8_t *value;
	uint16_t len;
	bool indicate;
};

static void conf_cb(void *user_data)
{
	DBG("GATT server received confirmation");
}

static void send_notification_to_device(void *data, void *user_data)
{
	struct device_state *device_state = data;
	struct notify *notify = user_data;
	struct ccc_state *ccc;
	struct btd_device *device;

	ccc = find_ccc_state(device_state, notify->ccc_handle);
	if (!ccc)
		return;

	if (!ccc->value[0] || (notify->indicate && !(ccc->value[0] & 0x02)))
		return;

	device = btd_adapter_get_device(notify->database->adapter,
						&device_state->bdaddr,
						device_state->bdaddr_type);
	if (!device)
		return;

	/*
	 * TODO: If the device is not connected but bonded, send the
	 * notification/indication when it becomes connected.
	 */
	if (!notify->indicate) {
		DBG("GATT server sending notification");
		bt_gatt_server_send_notification(
					btd_device_get_gatt_server(device),
					notify->handle, notify->value,
					notify->len);
		return;
	}

	DBG("GATT server sending indication");
	bt_gatt_server_send_indication(btd_device_get_gatt_server(device),
							notify->handle,
							notify->value,
							notify->len, conf_cb,
							NULL, NULL);
}

static void send_notification_to_devices(struct btd_gatt_database *database,
					uint16_t handle, const uint8_t *value,
					uint16_t len, uint16_t ccc_handle,
					bool indicate)
{
	struct notify notify;

	memset(&notify, 0, sizeof(notify));

	notify.database = database;
	notify.handle = handle;
	notify.ccc_handle = ccc_handle;
	notify.value = value;
	notify.len = len;
	notify.indicate = indicate;

	queue_foreach(database->device_states, send_notification_to_device,
								&notify);
}

static void send_service_changed(struct btd_gatt_database *database,
					struct gatt_db_attribute *attrib)
{
	uint16_t start, end;
	uint8_t value[4];
	uint16_t handle, ccc_handle;

	if (!gatt_db_attribute_get_service_handles(attrib, &start, &end)) {
		error("Failed to obtain changed service handles");
		return;
	}

	handle = gatt_db_attribute_get_handle(database->svc_chngd);
	ccc_handle = gatt_db_attribute_get_handle(database->svc_chngd_ccc);

	if (!handle || !ccc_handle) {
		error("Failed to obtain handles for \"Service Changed\""
							" characteristic");
		return;
	}

	put_le16(start, value);
	put_le16(end, value + 2);

	send_notification_to_devices(database, handle, value, sizeof(value),
							ccc_handle, true);
}

static void gatt_db_service_added(struct gatt_db_attribute *attrib,
								void *user_data)
{
	struct btd_gatt_database *database = user_data;

	DBG("GATT Service added to local database");

	send_service_changed(database, attrib);
}

static bool ccc_match_service(const void *data, const void *match_data)
{
	const struct ccc_state *ccc = data;
	const struct gatt_db_attribute *attrib = match_data;
	uint16_t start, end;

	if (!gatt_db_attribute_get_service_handles(attrib, &start, &end))
		return false;

	return ccc->handle >= start && ccc->handle <= end;
}

static void remove_device_ccc(void *data, void *user_data)
{
	struct device_state *state = data;

	queue_remove_all(state->ccc_states, ccc_match_service, user_data, free);
}

static void gatt_db_service_removed(struct gatt_db_attribute *attrib,
								void *user_data)
{
	struct btd_gatt_database *database = user_data;

	DBG("Local GATT service removed");

	send_service_changed(database, attrib);

	queue_foreach(database->device_states, remove_device_ccc, attrib);
	queue_remove_all(database->ccc_callbacks, ccc_cb_match_service, attrib,
								ccc_cb_free);
}

struct svc_match_data {
	const char *path;
	const char *sender;
};

static bool match_app(const void *a, const void *b)
{
	const struct gatt_app *app = a;
	const struct svc_match_data *data = b;

	return g_strcmp0(app->path, data->path) == 0 &&
				g_strcmp0(app->owner, data->sender) == 0;
}

static gboolean app_free_idle_cb(void *data)
{
	app_free(data);

	return FALSE;
}

static void client_disconnect_cb(DBusConnection *conn, void *user_data)
{
	struct gatt_app *app = user_data;
	struct btd_gatt_database *database = app->database;

	DBG("Client disconnected");

	if (queue_remove(database->apps, app))
		app_free(app);
}

static void remove_app(void *data)
{
	struct gatt_app *app = data;

	/*
	 * Set callback to NULL to avoid potential race condition
	 * when calling remove_app and GDBusClient unref.
	 */
	g_dbus_client_set_disconnect_watch(app->client, NULL, NULL);

	/*
	 * Set proxy handlers to NULL, so that this gets called only once when
	 * the first proxy that belongs to this service gets removed.
	 */
	g_dbus_client_set_proxy_handlers(app->client, NULL, NULL, NULL, NULL);


	queue_remove(app->database->apps, app);

	/*
	 * Do not run in the same loop, this may be a disconnect
	 * watch call and GDBusClient should not be destroyed.
	 */
	g_idle_add(app_free_idle_cb, app);
}

static bool match_service_by_path(const void *a, const void *b)
{
	const struct external_service *service = a;
	const char *path = b;

	return strcmp(service->path, path) == 0;
}

static bool parse_path(GDBusProxy *proxy, const char *name, const char **path)
{
	DBusMessageIter iter;

	if (!g_dbus_proxy_get_property(proxy, name, &iter))
		return false;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_OBJECT_PATH)
		return false;

	dbus_message_iter_get_basic(&iter, path);

	return true;
}

static bool incr_attr_count(struct external_service *service, uint16_t incr)
{
	if (service->attr_cnt > UINT16_MAX - incr)
		return false;

	service->attr_cnt += incr;

	return true;
}

static bool parse_chrc_flags(DBusMessageIter *array, uint8_t *props,
					uint8_t *ext_props, uint32_t *perm)
{
	const char *flag;

	*props = *ext_props = 0;

	do {
		if (dbus_message_iter_get_arg_type(array) != DBUS_TYPE_STRING)
			return false;

		dbus_message_iter_get_basic(array, &flag);

		if (!strcmp("broadcast", flag))
			*props |= BT_GATT_CHRC_PROP_BROADCAST;
		else if (!strcmp("read", flag)) {
			*props |= BT_GATT_CHRC_PROP_READ;
			*perm |= BT_ATT_PERM_READ;
		} else if (!strcmp("write-without-response", flag)) {
			*props |= BT_GATT_CHRC_PROP_WRITE_WITHOUT_RESP;
			*perm |= BT_ATT_PERM_WRITE;
		} else if (!strcmp("write", flag)) {
			*props |= BT_GATT_CHRC_PROP_WRITE;
			*perm |= BT_ATT_PERM_WRITE;
		} else if (!strcmp("notify", flag)) {
			*props |= BT_GATT_CHRC_PROP_NOTIFY;
		} else if (!strcmp("indicate", flag)) {
			*props |= BT_GATT_CHRC_PROP_INDICATE;
		} else if (!strcmp("authenticated-signed-writes", flag)) {
			*props |= BT_GATT_CHRC_PROP_AUTH;
			*perm |= BT_ATT_PERM_WRITE;
		} else if (!strcmp("reliable-write", flag)) {
			*ext_props |= BT_GATT_CHRC_EXT_PROP_RELIABLE_WRITE;
			*perm |= BT_ATT_PERM_WRITE;
		} else if (!strcmp("writable-auxiliaries", flag)) {
			*ext_props |= BT_GATT_CHRC_EXT_PROP_WRITABLE_AUX;
		} else if (!strcmp("encrypt-read", flag)) {
			*props |= BT_GATT_CHRC_PROP_READ;
			*ext_props |= BT_GATT_CHRC_EXT_PROP_ENC_READ;
			*perm |= BT_ATT_PERM_READ | BT_ATT_PERM_READ_ENCRYPT;
		} else if (!strcmp("encrypt-write", flag)) {
			*props |= BT_GATT_CHRC_PROP_WRITE;
			*ext_props |= BT_GATT_CHRC_EXT_PROP_ENC_WRITE;
			*perm |= BT_ATT_PERM_WRITE | BT_ATT_PERM_WRITE_ENCRYPT;
		} else if (!strcmp("encrypt-authenticated-read", flag)) {
			*props |= BT_GATT_CHRC_PROP_READ;
			*ext_props |= BT_GATT_CHRC_EXT_PROP_AUTH_READ;
			*perm |= BT_ATT_PERM_READ | BT_ATT_PERM_READ_AUTHEN;
		} else if (!strcmp("encrypt-authenticated-write", flag)) {
			*props |= BT_GATT_CHRC_PROP_WRITE;
			*ext_props |= BT_GATT_CHRC_EXT_PROP_AUTH_WRITE;
			*perm |= BT_ATT_PERM_WRITE | BT_ATT_PERM_WRITE_AUTHEN;
		} else if (!strcmp("secure-read", flag)) {
			*props |= BT_GATT_CHRC_PROP_READ;
			*ext_props |= BT_GATT_CHRC_EXT_PROP_AUTH_READ;
			*perm |= BT_ATT_PERM_READ | BT_ATT_PERM_READ_SECURE;
		} else if (!strcmp("secure-write", flag)) {
			*props |= BT_GATT_CHRC_PROP_WRITE;
			*ext_props |= BT_GATT_CHRC_EXT_PROP_AUTH_WRITE;
			*perm |= BT_ATT_PERM_WRITE | BT_ATT_PERM_WRITE_SECURE;
		} else {
			error("Invalid characteristic flag: %s", flag);
			return false;
		}
	} while (dbus_message_iter_next(array));

	if (*ext_props)
		*props |= BT_GATT_CHRC_PROP_EXT_PROP;

	return true;
}

static bool parse_desc_flags(DBusMessageIter *array, uint32_t *perm)
{
	const char *flag;

	*perm = 0;

	do {
		if (dbus_message_iter_get_arg_type(array) != DBUS_TYPE_STRING)
			return false;

		dbus_message_iter_get_basic(array, &flag);

		if (!strcmp("read", flag))
			*perm |= BT_ATT_PERM_READ;
		else if (!strcmp("write", flag))
			*perm |= BT_ATT_PERM_WRITE;
		else if (!strcmp("encrypt-read", flag))
			*perm |= BT_ATT_PERM_READ | BT_ATT_PERM_READ_ENCRYPT;
		else if (!strcmp("encrypt-write", flag))
			*perm |= BT_ATT_PERM_WRITE | BT_ATT_PERM_WRITE_ENCRYPT;
		else if (!strcmp("encrypt-authenticated-read", flag))
			*perm |= BT_ATT_PERM_READ | BT_ATT_PERM_READ_AUTHEN;
		else if (!strcmp("encrypt-authenticated-write", flag))
			*perm |= BT_ATT_PERM_WRITE | BT_ATT_PERM_WRITE_AUTHEN;
		else if (!strcmp("secure-read", flag))
			*perm |= BT_ATT_PERM_READ | BT_ATT_PERM_READ_SECURE;
		else if (!strcmp("secure-write", flag))
			*perm |= BT_ATT_PERM_WRITE | BT_ATT_PERM_WRITE_SECURE;
		else {
			error("Invalid descriptor flag: %s", flag);
			return false;
		}
	} while (dbus_message_iter_next(array));

	return true;
}

static bool parse_flags(GDBusProxy *proxy, uint8_t *props, uint8_t *ext_props,
								uint32_t *perm)
{
	DBusMessageIter iter, array;
	const char *iface;

	if (!g_dbus_proxy_get_property(proxy, "Flags", &iter))
		return false;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
		return false;

	dbus_message_iter_recurse(&iter, &array);

	iface = g_dbus_proxy_get_interface(proxy);
	if (!strcmp(iface, GATT_DESC_IFACE))
		return parse_desc_flags(&array, perm);

	return parse_chrc_flags(&array, props, ext_props, perm);
}

static struct external_chrc *chrc_create(struct gatt_app *app,
							GDBusProxy *proxy,
							const char *path)
{
	struct external_service *service;
	struct external_chrc *chrc;
	const char *service_path;

	if (!parse_path(proxy, "Service", &service_path)) {
		error("Failed to obtain service path for characteristic");
		return NULL;
	}

	service = queue_find(app->services, match_service_by_path,
								service_path);
	if (!service) {
		error("Unable to find service for characteristic: %s", path);
		return NULL;
	}

	chrc = new0(struct external_chrc, 1);
	chrc->pending_reads = queue_new();
	chrc->pending_writes = queue_new();

	chrc->path = g_strdup(path);
	if (!chrc->path)
		goto fail;

	chrc->service = service;
	chrc->proxy = g_dbus_proxy_ref(proxy);

	/*
	 * Add 2 for the characteristic declaration and the value
	 * attribute.
	 */
	if (!incr_attr_count(chrc->service, 2)) {
		error("Failed to increment attribute count");
		goto fail;
	}

	/*
	 * Parse characteristic flags (i.e. properties) here since they
	 * are used to determine if any special descriptors should be
	 * created.
	 */
	if (!parse_flags(proxy, &chrc->props, &chrc->ext_props, &chrc->perm)) {
		error("Failed to parse characteristic properties");
		goto fail;
	}

	if ((chrc->props & BT_GATT_CHRC_PROP_NOTIFY ||
				chrc->props & BT_GATT_CHRC_PROP_INDICATE) &&
				!incr_attr_count(chrc->service, 1)) {
		error("Failed to increment attribute count for CCC");
		goto fail;
	}

	if (chrc->ext_props && !incr_attr_count(chrc->service, 1)) {
		error("Failed to increment attribute count for CEP");
		goto fail;
	}

	queue_push_tail(chrc->service->chrcs, chrc);

	return chrc;

fail:
	chrc_free(chrc);
	return NULL;
}

static bool match_chrc(const void *a, const void *b)
{
	const struct external_chrc *chrc = a;
	const char *path = b;

	return strcmp(chrc->path, path) == 0;
}

static bool match_service_by_chrc(const void *a, const void *b)
{
	const struct external_service *service = a;
	const char *path = b;

	return queue_find(service->chrcs, match_chrc, path);
}

static struct external_desc *desc_create(struct gatt_app *app,
							GDBusProxy *proxy)
{
	struct external_service *service;
	struct external_desc *desc;
	const char *chrc_path;

	if (!parse_path(proxy, "Characteristic", &chrc_path)) {
		error("Failed to obtain characteristic path for descriptor");
		return NULL;
	}

	service = queue_find(app->services, match_service_by_chrc, chrc_path);
	if (!service) {
		error("Unable to find service for characteristic: %s",
								chrc_path);
		return NULL;
	}

	desc = new0(struct external_desc, 1);
	desc->pending_reads = queue_new();
	desc->pending_writes = queue_new();

	desc->chrc_path = g_strdup(chrc_path);
	if (!desc->chrc_path)
		goto fail;

	desc->service = service;
	desc->proxy = g_dbus_proxy_ref(proxy);

	/* Add 1 for the descriptor attribute */
	if (!incr_attr_count(desc->service, 1)) {
		error("Failed to increment attribute count");
		goto fail;
	}

	/*
	 * Parse descriptors flags here since they are used to
	 * determine the permission the descriptor should have
	 */
	if (!parse_flags(proxy, NULL, NULL, &desc->perm)) {
		error("Failed to parse characteristic properties");
		goto fail;
	}

	queue_push_tail(desc->service->descs, desc);

	return desc;

fail:
	desc_free(desc);
	return NULL;
}

static bool check_service_path(GDBusProxy *proxy,
					struct external_service *service)
{
	const char *service_path;

	if (!parse_path(proxy, "Service", &service_path))
		return false;

	return g_strcmp0(service_path, service->path) == 0;
}

static struct external_service *create_service(struct gatt_app *app,
						GDBusProxy *proxy,
						const char *path)
{
	struct external_service *service;

	if (!path || !g_str_has_prefix(path, "/"))
		return NULL;

	service = queue_find(app->services, match_service_by_path, path);
	if (service) {
		error("Duplicated service: %s", path);
		return NULL;
	}

	service = new0(struct external_service, 1);

	service->app = app;

	service->path = g_strdup(path);
	if (!service->path)
		goto fail;

	service->proxy = g_dbus_proxy_ref(proxy);
	service->chrcs = queue_new();
	service->descs = queue_new();

	/* Add 1 for the service declaration */
	if (!incr_attr_count(service, 1)) {
		error("Failed to increment attribute count");
		goto fail;
	}

	queue_push_tail(app->services, service);

	return service;

fail:
	service_free(service);
	return NULL;
}

static void proxy_added_cb(GDBusProxy *proxy, void *user_data)
{
	struct gatt_app *app = user_data;
	const char *iface, *path;

	if (app->failed)
		return;

	queue_push_tail(app->proxies, proxy);

	iface = g_dbus_proxy_get_interface(proxy);
	path = g_dbus_proxy_get_path(proxy);

	DBG("Object received: %s, iface: %s", path, iface);
}

static void proxy_removed_cb(GDBusProxy *proxy, void *user_data)
{
	struct gatt_app *app = user_data;
	struct external_service *service;
	const char *path;

	path = g_dbus_proxy_get_path(proxy);

	service = queue_remove_if(app->services, match_service_by_path,
							(void *) path);
	if (!service)
		return;

	DBG("Proxy removed - removing service: %s", service->path);

	service_free(service);
}

static bool parse_uuid(GDBusProxy *proxy, bt_uuid_t *uuid)
{
	DBusMessageIter iter;
	bt_uuid_t tmp;
	const char *uuidstr;

	if (!g_dbus_proxy_get_property(proxy, "UUID", &iter))
		return false;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING)
		return false;

	dbus_message_iter_get_basic(&iter, &uuidstr);

	if (bt_string_to_uuid(uuid, uuidstr) < 0)
		return false;

	/* GAP & GATT services are created and managed by BlueZ */
	bt_uuid16_create(&tmp, UUID_GAP);
	if (!bt_uuid_cmp(&tmp, uuid)) {
		error("GAP service must be handled by BlueZ");
		return false;
	}

	bt_uuid16_create(&tmp, UUID_GATT);
	if (!bt_uuid_cmp(&tmp, uuid)) {
		error("GATT service must be handled by BlueZ");
		return false;
	}

	return true;
}

static bool parse_primary(GDBusProxy *proxy, bool *primary)
{
	DBusMessageIter iter;
	dbus_bool_t val;

	if (!g_dbus_proxy_get_property(proxy, "Primary", &iter))
		return false;

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_BOOLEAN)
		return false;

	dbus_message_iter_get_basic(&iter, &val);

	*primary = val;

	return true;
}

static uint8_t dbus_error_to_att_ecode(const char *error_name)
{

	if (strcmp(error_name, "org.bluez.Error.Failed") == 0)
		return 0x80;  /* For now return this "application error" */

	if (strcmp(error_name, "org.bluez.Error.NotSupported") == 0)
		return BT_ATT_ERROR_REQUEST_NOT_SUPPORTED;

	if (strcmp(error_name, "org.bluez.Error.NotAuthorized") == 0)
		return BT_ATT_ERROR_AUTHORIZATION;

	if (strcmp(error_name, "org.bluez.Error.InvalidValueLength") == 0)
		return BT_ATT_ERROR_INVALID_ATTRIBUTE_VALUE_LEN;

	if (strcmp(error_name, "org.bluez.Error.InProgress") == 0)
		return BT_ERROR_ALREADY_IN_PROGRESS;

	return 0;
}

static void read_reply_cb(DBusMessage *message, void *user_data)
{
	struct pending_op *op = user_data;
	DBusError err;
	DBusMessageIter iter, array;
	uint8_t ecode = 0;
	uint8_t *value = NULL;
	int len = 0;

	if (!op->owner_queue) {
		DBG("Pending read was canceled when object got removed");
		return;
	}

	dbus_error_init(&err);

	if (dbus_set_error_from_message(&err, message) == TRUE) {
		DBG("Failed to read value: %s: %s", err.name, err.message);
		ecode = dbus_error_to_att_ecode(err.name);
		ecode = ecode ? ecode : BT_ATT_ERROR_READ_NOT_PERMITTED;
		dbus_error_free(&err);
		goto done;
	}

	dbus_message_iter_init(message, &iter);

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY) {
		/*
		 * Return not supported for this, as the external app basically
		 * doesn't properly support reading from this characteristic.
		 */
		ecode = BT_ATT_ERROR_REQUEST_NOT_SUPPORTED;
		error("Invalid return value received for \"ReadValue\"");
		goto done;
	}

	dbus_message_iter_recurse(&iter, &array);
	dbus_message_iter_get_fixed_array(&array, &value, &len);

	if (len < 0) {
		ecode = BT_ATT_ERROR_REQUEST_NOT_SUPPORTED;
		value = NULL;
		len = 0;
		goto done;
	}

	/* Truncate the value if it's too large */
	len = MIN(BT_ATT_MAX_VALUE_LEN, len);
	value = len ? value : NULL;

done:
	gatt_db_attribute_read_result(op->attrib, op->id, ecode, value, len);
}

static void pending_op_free(void *data)
{
	struct pending_op *op = data;

	if (op->owner_queue)
		queue_remove(op->owner_queue, op);

	free(op);
}

static struct pending_op *pending_read_new(struct btd_device *device,
					struct queue *owner_queue,
					struct gatt_db_attribute *attrib,
					unsigned int id)
{
	struct pending_op *op;

	op = new0(struct pending_op, 1);

	op->owner_queue = owner_queue;
	op->device = device;
	op->attrib = attrib;
	op->id = id;
	queue_push_tail(owner_queue, op);

	return op;
}

static void append_options(DBusMessageIter *iter, void *user_data)
{
	struct pending_op *op = user_data;
	const char *path = device_get_path(op->device);

	dict_append_entry(iter, "device", DBUS_TYPE_OBJECT_PATH, &path);
}

static void read_setup_cb(DBusMessageIter *iter, void *user_data)
{
	struct pending_op *op = user_data;
	DBusMessageIter dict;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	append_options(&dict, op);

	dbus_message_iter_close_container(iter, &dict);
}

static struct pending_op *send_read(struct btd_device *device,
					struct gatt_db_attribute *attrib,
					GDBusProxy *proxy,
					struct queue *owner_queue,
					unsigned int id)
{
	struct pending_op *op;

	op = pending_read_new(device, owner_queue, attrib, id);

	if (g_dbus_proxy_method_call(proxy, "ReadValue", read_setup_cb,
				read_reply_cb, op, pending_op_free) == TRUE)
		return op;

	pending_op_free(op);

	return NULL;
}

static void write_setup_cb(DBusMessageIter *iter, void *user_data)
{
	struct pending_op *op = user_data;
	DBusMessageIter array, dict;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "y", &array);
	dbus_message_iter_append_fixed_array(&array, DBUS_TYPE_BYTE,
					&op->data.iov_base, op->data.iov_len);
	dbus_message_iter_close_container(iter, &array);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	append_options(&dict, op);

	dbus_message_iter_close_container(iter, &dict);

	if (!op->owner_queue) {
		gatt_db_attribute_write_result(op->attrib, op->id, 0);
		pending_op_free(op);
	}
}

static void write_reply_cb(DBusMessage *message, void *user_data)
{
	struct pending_op *op = user_data;
	DBusError err;
	DBusMessageIter iter;
	uint8_t ecode = 0;

	if (!op->owner_queue) {
		DBG("Pending write was canceled when object got removed");
		return;
	}

	dbus_error_init(&err);

	if (dbus_set_error_from_message(&err, message) == TRUE) {
		DBG("Failed to write value: %s: %s", err.name, err.message);
		ecode = dbus_error_to_att_ecode(err.name);
		ecode = ecode ? ecode : BT_ATT_ERROR_WRITE_NOT_PERMITTED;
		dbus_error_free(&err);
		goto done;
	}

	dbus_message_iter_init(message, &iter);
	if (dbus_message_iter_has_next(&iter)) {
		/*
		 * Return not supported for this, as the external app basically
		 * doesn't properly support the "WriteValue" API.
		 */
		ecode = BT_ATT_ERROR_REQUEST_NOT_SUPPORTED;
		error("Invalid return value received for \"WriteValue\"");
	}

done:
	gatt_db_attribute_write_result(op->attrib, op->id, ecode);
}

static struct pending_op *pending_write_new(struct btd_device *device,
					struct queue *owner_queue,
					struct gatt_db_attribute *attrib,
					unsigned int id,
					const uint8_t *value,
					size_t len)
{
	struct pending_op *op;

	op = new0(struct pending_op, 1);

	op->data.iov_base = (uint8_t *) value;
	op->data.iov_len = len;

	op->device = device;
	op->owner_queue = owner_queue;
	op->attrib = attrib;
	op->id = id;
	queue_push_tail(owner_queue, op);

	return op;
}

static struct pending_op *send_write(struct btd_device *device,
					struct gatt_db_attribute *attrib,
					GDBusProxy *proxy,
					struct queue *owner_queue,
					unsigned int id,
					const uint8_t *value, size_t len)
{
	struct pending_op *op;

	op = pending_write_new(device, owner_queue, attrib, id, value, len);

	if (g_dbus_proxy_method_call(proxy, "WriteValue", write_setup_cb,
					owner_queue ? write_reply_cb : NULL,
					op, pending_op_free) == TRUE)
		return op;

	pending_op_free(op);

	return NULL;
}

static uint8_t ccc_write_cb(uint16_t value, void *user_data)
{
	struct external_chrc *chrc = user_data;

	DBG("External CCC write received with value: 0x%04x", value);

	/* Notifications/indications disabled */
	if (!value) {
		if (!chrc->ntfy_cnt)
			return 0;

		if (__sync_sub_and_fetch(&chrc->ntfy_cnt, 1))
			return 0;

		/*
		 * Send request to stop notifying. This is best-effort
		 * operation, so simply ignore the return the value.
		 */
		g_dbus_proxy_method_call(chrc->proxy, "StopNotify", NULL,
							NULL, NULL, NULL);
		return 0;
	}

	if (chrc->ntfy_cnt == UINT_MAX) {
		/* Maximum number of per-device CCC descriptors configured */
		return BT_ATT_ERROR_INSUFFICIENT_RESOURCES;
	}

	/* Don't support undefined CCC values yet */
	if (value > 2 ||
		(value == 1 && !(chrc->props & BT_GATT_CHRC_PROP_NOTIFY)) ||
		(value == 2 && !(chrc->props & BT_GATT_CHRC_PROP_INDICATE)))
		return BT_ERROR_CCC_IMPROPERLY_CONFIGURED;

	/*
	 * Always call StartNotify for an incoming enable and ignore the return
	 * value for now.
	 */
	if (g_dbus_proxy_method_call(chrc->proxy,
						"StartNotify", NULL, NULL,
						NULL, NULL) == FALSE)
		return BT_ATT_ERROR_UNLIKELY;

	__sync_fetch_and_add(&chrc->ntfy_cnt, 1);

	return 0;
}

static void property_changed_cb(GDBusProxy *proxy, const char *name,
					DBusMessageIter *iter, void *user_data)
{
	struct external_chrc *chrc = user_data;
	DBusMessageIter array;
	uint8_t *value = NULL;
	int len = 0;

	if (strcmp(name, "Value"))
		return;

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_ARRAY) {
		DBG("Malformed \"Value\" property received");
		return;
	}

	dbus_message_iter_recurse(iter, &array);
	dbus_message_iter_get_fixed_array(&array, &value, &len);

	if (len < 0) {
		DBG("Malformed \"Value\" property received");
		return;
	}

	/* Truncate the value if it's too large */
	len = MIN(BT_ATT_MAX_VALUE_LEN, len);
	value = len ? value : NULL;

	send_notification_to_devices(chrc->service->app->database,
				gatt_db_attribute_get_handle(chrc->attrib),
				value, len,
				gatt_db_attribute_get_handle(chrc->ccc),
				chrc->props & BT_GATT_CHRC_PROP_INDICATE);
}

static bool database_add_ccc(struct external_service *service,
						struct external_chrc *chrc)
{
	if (!(chrc->props & BT_GATT_CHRC_PROP_NOTIFY) &&
				!(chrc->props & BT_GATT_CHRC_PROP_INDICATE))
		return true;

	chrc->ccc = service_add_ccc(service->attrib, service->app->database,
						ccc_write_cb, chrc, NULL);
	if (!chrc->ccc) {
		error("Failed to create CCC entry for characteristic");
		return false;
	}

	if (g_dbus_proxy_set_property_watch(chrc->proxy, property_changed_cb,
							chrc) == FALSE) {
		error("Failed to set up property watch for characteristic");
		return false;
	}

	DBG("Created CCC entry for characteristic");

	return true;
}

static void cep_write_cb(struct gatt_db_attribute *attrib, int err,
								void *user_data)
{
	if (err)
		DBG("Failed to store CEP value in the database");
	else
		DBG("Stored CEP value in the database");
}

static bool database_add_cep(struct external_service *service,
						struct external_chrc *chrc)
{
	struct gatt_db_attribute *cep;
	bt_uuid_t uuid;
	uint8_t value[2];

	if (!chrc->ext_props)
		return true;

	bt_uuid16_create(&uuid, GATT_CHARAC_EXT_PROPER_UUID);
	cep = gatt_db_service_add_descriptor(service->attrib, &uuid,
							BT_ATT_PERM_READ,
							NULL, NULL, NULL);
	if (!cep) {
		error("Failed to create CEP entry for characteristic");
		return false;
	}

	memset(value, 0, sizeof(value));
	value[0] = chrc->ext_props;

	if (!gatt_db_attribute_write(cep, 0, value, sizeof(value), 0, NULL,
							cep_write_cb, NULL)) {
		DBG("Failed to store CEP value in the database");
		return false;
	}

	DBG("Created CEP entry for characteristic");

	return true;
}

static struct btd_device *att_get_device(struct bt_att *att)
{
	GIOChannel *io = NULL;
	GError *gerr = NULL;
	bdaddr_t src, dst;
	uint8_t dst_type;
	struct btd_adapter *adapter;

	io = g_io_channel_unix_new(bt_att_get_fd(att));
	if (!io)
		return NULL;

	bt_io_get(io, &gerr, BT_IO_OPT_SOURCE_BDADDR, &src,
					BT_IO_OPT_DEST_BDADDR, &dst,
					BT_IO_OPT_DEST_TYPE, &dst_type,
					BT_IO_OPT_INVALID);
	if (gerr) {
		error("bt_io_get: %s", gerr->message);
		g_error_free(gerr);
		g_io_channel_unref(io);
		return NULL;
	}

	g_io_channel_unref(io);

	adapter = adapter_find(&src);
	if (!adapter) {
		error("Unable to find adapter object");
		return NULL;
	}

	return btd_adapter_find_device(adapter, &dst, dst_type);
}

static void desc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	struct external_desc *desc = user_data;
	struct btd_device *device;

	if (desc->attrib != attrib) {
		error("Read callback called with incorrect attribute");
		goto fail;
	}

	device = att_get_device(att);
	if (!device) {
		error("Unable to find device object");
		goto fail;
	}

	if (send_read(device, attrib, desc->proxy, desc->pending_reads, id))
		return;

fail:
	gatt_db_attribute_read_result(attrib, id, BT_ATT_ERROR_UNLIKELY,
								NULL, 0);
}

static void desc_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	struct external_desc *desc = user_data;
	struct btd_device *device;

	if (desc->attrib != attrib) {
		error("Read callback called with incorrect attribute");
		goto fail;
	}

	device = att_get_device(att);
	if (!device) {
		error("Unable to find device object");
		goto fail;
	}

	if (send_write(device, attrib, desc->proxy, desc->pending_writes, id,
							value, len))
		return;

fail:
	gatt_db_attribute_read_result(attrib, id, BT_ATT_ERROR_UNLIKELY,
								NULL, 0);
}

static bool database_add_desc(struct external_service *service,
						struct external_desc *desc)
{
	bt_uuid_t uuid;

	if (!parse_uuid(desc->proxy, &uuid)) {
		error("Failed to read \"UUID\" property of descriptor");
		return false;
	}

	desc->attrib = gatt_db_service_add_descriptor(service->attrib, &uuid,
							desc->perm,
							desc_read_cb,
							desc_write_cb, desc);
	if (!desc->attrib) {
		error("Failed to create descriptor entry in database");
		return false;
	}

	desc->handled = true;

	return true;
}

static void chrc_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	struct external_chrc *chrc = user_data;
	struct btd_device *device;

	if (chrc->attrib != attrib) {
		error("Read callback called with incorrect attribute");
		goto fail;
	}

	device = att_get_device(att);
	if (!device) {
		error("Unable to find device object");
		goto fail;
	}

	if (send_read(device, attrib, chrc->proxy, chrc->pending_reads, id))
		return;

fail:
	gatt_db_attribute_read_result(attrib, id, BT_ATT_ERROR_UNLIKELY,
								NULL, 0);
}

static void chrc_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	struct external_chrc *chrc = user_data;
	struct btd_device *device;
	struct queue *queue;

	if (chrc->attrib != attrib) {
		error("Write callback called with incorrect attribute");
		goto fail;
	}

	device = att_get_device(att);
	if (!device) {
		error("Unable to find device object");
		goto fail;
	}

	if (!(chrc->props & BT_GATT_CHRC_PROP_WRITE_WITHOUT_RESP))
		queue = chrc->pending_writes;
	else
		queue = NULL;

	if (send_write(device, attrib, chrc->proxy, queue, id, value, len))
		return;

fail:
	gatt_db_attribute_read_result(attrib, id, BT_ATT_ERROR_UNLIKELY,
								NULL, 0);
}

static bool database_add_chrc(struct external_service *service,
						struct external_chrc *chrc)
{
	bt_uuid_t uuid;
	const struct queue_entry *entry;

	if (!parse_uuid(chrc->proxy, &uuid)) {
		error("Failed to read \"UUID\" property of characteristic");
		return false;
	}

	if (!check_service_path(chrc->proxy, service)) {
		error("Invalid service path for characteristic");
		return false;
	}

	chrc->attrib = gatt_db_service_add_characteristic(service->attrib,
						&uuid, chrc->perm,
						chrc->props, chrc_read_cb,
						chrc_write_cb, chrc);
	if (!chrc->attrib) {
		error("Failed to create characteristic entry in database");
		return false;
	}

	if (!database_add_ccc(service, chrc))
		return false;

	if (!database_add_cep(service, chrc))
		return false;

	/* Handle the descriptors that belong to this characteristic. */
	for (entry = queue_get_entries(service->descs); entry;
							entry = entry->next) {
		struct external_desc *desc = entry->data;

		if (desc->handled || g_strcmp0(desc->chrc_path, chrc->path))
			continue;

		if (!database_add_desc(service, desc)) {
			chrc->attrib = NULL;
			error("Failed to create descriptor entry");
			return false;
		}
	}

	return true;
}

static bool match_desc_unhandled(const void *a, const void *b)
{
	const struct external_desc *desc = a;

	return !desc->handled;
}

static bool database_add_service(struct external_service *service)
{
	bt_uuid_t uuid;
	bool primary;
	const struct queue_entry *entry;

	if (!parse_uuid(service->proxy, &uuid)) {
		error("Failed to read \"UUID\" property of service");
		return false;
	}

	if (!parse_primary(service->proxy, &primary)) {
		error("Failed to read \"Primary\" property of service");
		return false;
	}

	service->attrib = gatt_db_add_service(service->app->database->db, &uuid,
						primary, service->attr_cnt);
	if (!service->attrib)
		return false;

	entry = queue_get_entries(service->chrcs);
	while (entry) {
		struct external_chrc *chrc = entry->data;

		if (!database_add_chrc(service, chrc)) {
			error("Failed to add characteristic");
			goto fail;
		}

		entry = entry->next;
	}

	/* If there are any unhandled descriptors, return an error */
	if (queue_find(service->descs, match_desc_unhandled, NULL)) {
		error("Found descriptor with no matching characteristic!");
		goto fail;
	}

	gatt_db_service_set_active(service->attrib, true);

	return true;

fail:
	gatt_db_remove_service(service->app->database->db, service->attrib);
	service->attrib = NULL;

	return false;
}

static bool database_add_app(struct gatt_app *app)
{
	const struct queue_entry *entry;

	entry = queue_get_entries(app->services);
	while (entry) {
		if (!database_add_service(entry->data)) {
			error("Failed to add service");
			return false;
		}

		entry = entry->next;
	}

	return true;
}

static int profile_device_probe(struct btd_service *service)
{
	struct btd_profile *p = btd_service_get_profile(service);

	DBG("%s probed", p->name);

	return 0;
}

static void profile_device_remove(struct btd_service *service)
{
	struct btd_profile *p = btd_service_get_profile(service);

	DBG("%s removed", p->name);
}

static int profile_add(struct external_profile *profile, const char *uuid)
{
	struct btd_profile *p;

	p = new0(struct btd_profile, 1);

	/* Assign directly to avoid having extra fields */
	p->name = (const void *) g_strdup_printf("%s%s/%s", profile->app->owner,
				g_dbus_proxy_get_path(profile->proxy), uuid);
	if (!p->name) {
		free(p);
		return -ENOMEM;
	}

	p->remote_uuid = (const void *) g_strdup(uuid);
	if (!p->remote_uuid) {
		g_free((void *) p->name);
		free(p);
		return -ENOMEM;
	}

	p->device_probe = profile_device_probe;
	p->device_remove = profile_device_remove;
	p->auto_connect = true;
	p->external = true;

	queue_push_tail(profile->profiles, p);

	DBG("Added \"%s\"", p->name);

	return 0;
}

static void add_profile(void *data, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	btd_profile_register(data);
	adapter_add_profile(adapter, data);
}

static struct external_profile *create_profile(struct gatt_app *app,
						GDBusProxy *proxy,
						const char *path)
{
	struct external_profile *profile;
	DBusMessageIter iter, array;

	if (!path || !g_str_has_prefix(path, "/"))
		return NULL;

	profile = new0(struct external_profile, 1);

	profile->app = app;
	profile->proxy = g_dbus_proxy_ref(proxy);
	profile->profiles = queue_new();

	if (!g_dbus_proxy_get_property(proxy, "UUIDs", &iter)) {
		DBG("UUIDs property not found");
		goto fail;
	}

	dbus_message_iter_recurse(&iter, &array);

	while (dbus_message_iter_get_arg_type(&array) == DBUS_TYPE_STRING) {
		const char *uuid;

		dbus_message_iter_get_basic(&array, &uuid);

		if (profile_add(profile, uuid) < 0)
			goto fail;

		dbus_message_iter_next(&array);
	}

	if (queue_isempty(profile->profiles))
		goto fail;

	queue_foreach(profile->profiles, add_profile, app->database->adapter);
	queue_push_tail(app->profiles, profile);

	return profile;

fail:
	profile_free(profile);
	return NULL;
}

static void register_profile(void *data, void *user_data)
{
	struct gatt_app *app = user_data;
	GDBusProxy *proxy = data;
	const char *iface = g_dbus_proxy_get_interface(proxy);
	const char *path = g_dbus_proxy_get_path(proxy);

	if (app->failed)
		return;

	if (g_strcmp0(iface, GATT_PROFILE_IFACE) == 0) {
		struct external_profile *profile;

		profile = create_profile(app, proxy, path);
		if (!profile) {
			app->failed = true;
			return;
		}
	}
}

static void register_service(void *data, void *user_data)
{
	struct gatt_app *app = user_data;
	GDBusProxy *proxy = data;
	const char *iface = g_dbus_proxy_get_interface(proxy);
	const char *path = g_dbus_proxy_get_path(proxy);

	if (app->failed)
		return;

	if (g_strcmp0(iface, GATT_SERVICE_IFACE) == 0) {
		struct external_service *service;

		service = create_service(app, proxy, path);
		if (!service) {
			app->failed = true;
			return;
		}
	}
}

static void register_characteristic(void *data, void *user_data)
{
	struct gatt_app *app = user_data;
	GDBusProxy *proxy = data;
	const char *iface = g_dbus_proxy_get_interface(proxy);
	const char *path = g_dbus_proxy_get_path(proxy);

	if (app->failed)
		return;

	iface = g_dbus_proxy_get_interface(proxy);
	path = g_dbus_proxy_get_path(proxy);

	if (g_strcmp0(iface, GATT_CHRC_IFACE) == 0) {
		struct external_chrc *chrc;

		chrc = chrc_create(app, proxy, path);
		if (!chrc) {
			app->failed = true;
			return;
		}
	}
}

static void register_descriptor(void *data, void *user_data)
{
	struct gatt_app *app = user_data;
	GDBusProxy *proxy = data;
	const char *iface = g_dbus_proxy_get_interface(proxy);

	if (app->failed)
		return;

	if (g_strcmp0(iface, GATT_DESC_IFACE) == 0) {
		struct external_desc *desc;

		desc = desc_create(app, proxy);
		if (!desc) {
			app->failed = true;
			return;
		}
	}
}

static void client_ready_cb(GDBusClient *client, void *user_data)
{
	struct gatt_app *app = user_data;
	DBusMessage *reply;
	bool fail = false;

	/*
	 * Process received objects
	 */
	if (queue_isempty(app->proxies)) {
		error("No object received");
		fail = true;
		reply = btd_error_failed(app->reg,
					"No object received");
		goto reply;
	}

	queue_foreach(app->proxies, register_profile, app);
	queue_foreach(app->proxies, register_service, app);
	queue_foreach(app->proxies, register_characteristic, app);
	queue_foreach(app->proxies, register_descriptor, app);

	if ((queue_isempty(app->services) && queue_isempty(app->profiles)) ||
							app->failed) {
		error("No valid external GATT objects found");
		fail = true;
		reply = btd_error_failed(app->reg,
					"No valid service object found");
		goto reply;
	}

	if (!database_add_app(app)) {
		error("Failed to create GATT service entry in local database");
		fail = true;
		reply = btd_error_failed(app->reg,
					"Failed to create entry in database");
		goto reply;
	}

	DBG("GATT application registered: %s:%s", app->owner, app->path);

	reply = dbus_message_new_method_return(app->reg);

reply:
	g_dbus_send_message(btd_get_dbus_connection(), reply);
	dbus_message_unref(app->reg);
	app->reg = NULL;

	if (fail)
		remove_app(app);
}

static struct gatt_app *create_app(DBusConnection *conn, DBusMessage *msg,
							const char *path)
{
	struct gatt_app *app;
	const char *sender = dbus_message_get_sender(msg);

	if (!path || !g_str_has_prefix(path, "/"))
		return NULL;

	app = new0(struct gatt_app, 1);

	app->client = g_dbus_client_new_full(conn, sender, path, path);
	if (!app->client)
		goto fail;

	app->owner = g_strdup(sender);
	if (!app->owner)
		goto fail;

	app->path = g_strdup(path);
	if (!app->path)
		goto fail;

	app->services = queue_new();
	app->profiles = queue_new();
	app->proxies = queue_new();
	app->reg = dbus_message_ref(msg);

	g_dbus_client_set_disconnect_watch(app->client, client_disconnect_cb,
									app);
	g_dbus_client_set_proxy_handlers(app->client, proxy_added_cb,
					proxy_removed_cb, NULL, app);
	g_dbus_client_set_ready_watch(app->client, client_ready_cb, app);

	return app;

fail:
	app_free(app);
	return NULL;
}

static DBusMessage *manager_register_app(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct btd_gatt_database *database = user_data;
	const char *sender = dbus_message_get_sender(msg);
	DBusMessageIter args;
	const char *path;
	struct gatt_app *app;
	struct svc_match_data match_data;

	if (!dbus_message_iter_init(msg, &args))
		return btd_error_invalid_args(msg);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_OBJECT_PATH)
		return btd_error_invalid_args(msg);

	dbus_message_iter_get_basic(&args, &path);

	match_data.path = path;
	match_data.sender = sender;

	if (queue_find(database->apps, match_app, &match_data))
		return btd_error_already_exists(msg);

	dbus_message_iter_next(&args);
	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY)
		return btd_error_invalid_args(msg);

	app = create_app(conn, msg, path);
	if (!app)
		return btd_error_failed(msg, "Failed to register application");

	DBG("Registering application: %s:%s", sender, path);

	app->database = database;
	queue_push_tail(database->apps, app);

	return NULL;
}

static DBusMessage *manager_unregister_app(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct btd_gatt_database *database = user_data;
	const char *sender = dbus_message_get_sender(msg);
	const char *path;
	DBusMessageIter args;
	struct gatt_app *app;
	struct svc_match_data match_data;

	if (!dbus_message_iter_init(msg, &args))
		return btd_error_invalid_args(msg);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_OBJECT_PATH)
		return btd_error_invalid_args(msg);

	dbus_message_iter_get_basic(&args, &path);

	match_data.path = path;
	match_data.sender = sender;

	app = queue_remove_if(database->apps, match_app, &match_data);
	if (!app)
		return btd_error_does_not_exist(msg);

	app_free(app);

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable manager_methods[] = {
	{ GDBUS_EXPERIMENTAL_ASYNC_METHOD("RegisterApplication",
			GDBUS_ARGS({ "application", "o" },
			{ "options", "a{sv}" }), NULL,
			manager_register_app) },
	{ GDBUS_EXPERIMENTAL_ASYNC_METHOD("UnregisterApplication",
					GDBUS_ARGS({ "application", "o" }),
					NULL, manager_unregister_app) },
	{ }
};

struct btd_gatt_database *btd_gatt_database_new(struct btd_adapter *adapter)
{
	struct btd_gatt_database *database;
	GError *gerr = NULL;
	const bdaddr_t *addr;

	if (!adapter)
		return NULL;

	database = new0(struct btd_gatt_database, 1);
	database->adapter = btd_adapter_ref(adapter);
	database->db = gatt_db_new();
	database->device_states = queue_new();
	database->apps = queue_new();
	database->profiles = queue_new();
	database->ccc_callbacks = queue_new();

	database->db_id = gatt_db_register(database->db, gatt_db_service_added,
							gatt_db_service_removed,
							database, NULL);
	if (!database->db_id)
		goto fail;

	addr = btd_adapter_get_address(adapter);
	database->le_io = bt_io_listen(connect_cb, NULL, NULL, NULL, &gerr,
					BT_IO_OPT_SOURCE_BDADDR, addr,
					BT_IO_OPT_SOURCE_TYPE, BDADDR_LE_PUBLIC,
					BT_IO_OPT_CID, ATT_CID,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
					BT_IO_OPT_INVALID);
	if (!database->le_io) {
		error("Failed to start listening: %s", gerr->message);
		g_error_free(gerr);
		goto fail;
	}

	/* BR/EDR socket */
	database->l2cap_io = bt_io_listen(connect_cb, NULL, NULL, NULL, &gerr,
					BT_IO_OPT_SOURCE_BDADDR, addr,
					BT_IO_OPT_PSM, ATT_PSM,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
					BT_IO_OPT_INVALID);
	if (database->l2cap_io == NULL) {
		error("Failed to start listening: %s", gerr->message);
		g_error_free(gerr);
		goto fail;
	}

	if (g_dbus_register_interface(btd_get_dbus_connection(),
						adapter_get_path(adapter),
						GATT_MANAGER_IFACE,
						manager_methods, NULL, NULL,
						database, NULL))
		DBG("GATT Manager registered for adapter: %s",
						adapter_get_path(adapter));

	register_core_services(database);

	return database;

fail:
	gatt_database_free(database);

	return NULL;
}

void btd_gatt_database_destroy(struct btd_gatt_database *database)
{
	if (!database)
		return;

	g_dbus_unregister_interface(btd_get_dbus_connection(),
					adapter_get_path(database->adapter),
					GATT_MANAGER_IFACE);

	gatt_database_free(database);
}

struct gatt_db *btd_gatt_database_get_db(struct btd_gatt_database *database)
{
	if (!database)
		return NULL;

	return database->db;
}
