/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
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

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <glib.h>
#include <errno.h>
#include <sys/socket.h>

#include "ipc.h"
#include "ipc-common.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "lib/uuid.h"
#include "bluetooth.h"
#include "gatt.h"
#include "src/log.h"
#include "hal-msg.h"
#include "utils.h"
#include "src/shared/util.h"
#include "src/shared/queue.h"
#include "src/shared/att.h"
#include "src/shared/gatt-db.h"
#include "attrib/gattrib.h"
#include "attrib/att.h"
#include "attrib/gatt.h"
#include "btio/btio.h"

/* set according to Android bt_gatt_client.h */
#define GATT_MAX_ATTR_LEN 600

#define GATT_SUCCESS	0x00000000
#define GATT_FAILURE	0x00000101

#define BASE_UUID16_OFFSET     12

#define GATT_PERM_READ			0x00000001
#define GATT_PERM_READ_ENCRYPTED	0x00000002
#define GATT_PERM_READ_MITM		0x00000004
#define GATT_PERM_READ_AUTHORIZATION	0x00000008
#define GATT_PERM_WRITE			0x00000100
#define GATT_PERM_WRITE_ENCRYPTED	0x00000200
#define GATT_PERM_WRITE_MITM		0x00000400
#define GATT_PERM_WRITE_AUTHORIZATION	0x00000800
#define GATT_PERM_WRITE_SIGNED		0x00010000
#define GATT_PERM_WRITE_SIGNED_MITM	0x00020000
#define GATT_PERM_NONE			0x10000000

#define GATT_PAIR_CONN_TIMEOUT 30
#define GATT_CONN_TIMEOUT 2

static const uint8_t BLUETOOTH_UUID[] = {
	0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
	0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

typedef enum {
	DEVICE_DISCONNECTED = 0,
	DEVICE_CONNECT_INIT,		/* connection procedure initiated */
	DEVICE_CONNECT_READY,		/* dev found during LE scan */
	DEVICE_CONNECTED,		/* connection has been established */
} gatt_device_state_t;

static const char *device_state_str[] = {
	"DISCONNECTED",
	"CONNECT INIT",
	"CONNECT READY",
	"CONNECTED",
};

struct pending_trans_data {
	unsigned int id;
	uint8_t opcode;
	struct gatt_db_attribute *attrib;
	unsigned int serial_id;
};

struct gatt_app {
	int32_t id;
	uint8_t uuid[16];

	gatt_type_t type;

	/* Valid for client applications */
	struct queue *notifications;

	gatt_conn_cb_t func;
};

struct element_id {
	bt_uuid_t uuid;
	uint8_t instance;
};

struct descriptor {
	struct element_id id;
	uint16_t handle;
};

struct characteristic {
	struct element_id id;
	struct gatt_char ch;
	uint16_t end_handle;

	struct queue *descriptors;
};

struct service {
	struct element_id id;
	struct gatt_primary prim;
	struct gatt_included incl;

	bool primary;

	struct queue *chars;
	struct queue *included;	/* Valid only for primary services */
	bool incl_search_done;
};

struct notification_data {
	struct hal_gatt_srvc_id service;
	struct hal_gatt_gatt_id ch;
	struct app_connection *conn;
	guint notif_id;
	guint ind_id;
	int ref;
};

struct gatt_device {
	bdaddr_t bdaddr;

	gatt_device_state_t state;

	GAttrib *attrib;
	GIOChannel *att_io;
	struct queue *services;
	bool partial_srvc_search;

	guint watch_id;
	guint server_id;
	guint ind_id;

	int ref;

	struct queue *autoconnect_apps;

	struct queue *pending_requests;
};

struct app_connection {
	struct gatt_device *device;
	struct gatt_app *app;
	struct queue *transactions;
	int32_t id;

	guint timeout_id;

	bool wait_execute_write;
};

struct service_sdp {
	int32_t service_handle;
	uint32_t sdp_handle;
};

static struct ipc *hal_ipc = NULL;
static bdaddr_t adapter_addr;
static bool scanning = false;
static unsigned int advertising_cnt = 0;

static struct queue *gatt_apps = NULL;
static struct queue *gatt_devices = NULL;
static struct queue *app_connections = NULL;

static struct queue *services_sdp = NULL;

static struct queue *listen_apps = NULL;
static struct gatt_db *gatt_db = NULL;

static struct gatt_db_attribute *service_changed_attrib = NULL;

static GIOChannel *le_io = NULL;
static GIOChannel *bredr_io = NULL;

static uint32_t gatt_sdp_handle = 0;
static uint32_t gap_sdp_handle = 0;
static uint32_t dis_sdp_handle = 0;

static struct bt_crypto *crypto = NULL;

static int test_client_if = 0;
static const uint8_t TEST_UUID[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04
};

static bool is_bluetooth_uuid(const uint8_t *uuid)
{
	int i;

	for (i = 0; i < 16; i++) {
		/* ignore minimal uuid (16) value */
		if (i == 12 || i == 13)
			continue;

		if (uuid[i] != BLUETOOTH_UUID[i])
			return false;
	}

	return true;
}

static void android2uuid(const uint8_t *uuid, bt_uuid_t *dst)
{
	if (is_bluetooth_uuid(uuid)) {
		/* copy 16 bit uuid value from full android 128bit uuid */
		dst->type = BT_UUID16;
		dst->value.u16 = (uuid[13] << 8) + uuid[12];
	} else {
		int i;

		dst->type = BT_UUID128;
		for (i = 0; i < 16; i++)
			dst->value.u128.data[i] = uuid[15 - i];
	}
}

static void uuid2android(const bt_uuid_t *src, uint8_t *uuid)
{
	bt_uuid_t uu128;
	uint8_t i;

	if (src->type != BT_UUID128) {
		bt_uuid_to_uuid128(src, &uu128);
		src = &uu128;
	}

	for (i = 0; i < 16; i++)
		uuid[15 - i] = src->value.u128.data[i];
}

static void hal_srvc_id_to_element_id(const struct hal_gatt_srvc_id *from,
							struct element_id *to)
{
	to->instance = from->inst_id;
	android2uuid(from->uuid, &to->uuid);
}

static void element_id_to_hal_srvc_id(const struct element_id *from,
						uint8_t primary,
						struct hal_gatt_srvc_id *to)
{
	to->is_primary = primary;
	to->inst_id = from->instance;
	uuid2android(&from->uuid, to->uuid);
}

static void hal_gatt_id_to_element_id(const struct hal_gatt_gatt_id *from,
							struct element_id *to)
{
	to->instance = from->inst_id;
	android2uuid(from->uuid, &to->uuid);
}

static void element_id_to_hal_gatt_id(const struct element_id *from,
						struct hal_gatt_gatt_id *to)
{
	to->inst_id = from->instance;
	uuid2android(&from->uuid, to->uuid);
}

static void destroy_characteristic(void *data)
{
	struct characteristic *chars = data;

	if (!chars)
		return;

	queue_destroy(chars->descriptors, free);
	free(chars);
}

static void destroy_service(void *data)
{
	struct service *srvc = data;

	if (!srvc)
		return;

	queue_destroy(srvc->chars, destroy_characteristic);

	/*
	 * Included services we keep on two queues.
	 * 1. On the same queue with primary services.
	 * 2. On the queue inside primary service.
	 * So we need to free service memory only once but we need to destroy
	 * two queues
	 */
	queue_destroy(srvc->included, NULL);

	free(srvc);
}

static bool match_app_by_uuid(const void *data, const void *user_data)
{
	const uint8_t *exp_uuid = user_data;
	const struct gatt_app *client = data;

	return !memcmp(exp_uuid, client->uuid, sizeof(client->uuid));
}

static bool match_app_by_id(const void *data, const void *user_data)
{
	int32_t exp_id = PTR_TO_INT(user_data);
	const struct gatt_app *client = data;

	return client->id == exp_id;
}

static struct gatt_app *find_app_by_id(int32_t id)
{
	return queue_find(gatt_apps, match_app_by_id, INT_TO_PTR(id));
}

static bool match_device_by_bdaddr(const void *data, const void *user_data)
{
	const struct gatt_device *dev = data;
	const bdaddr_t *addr = user_data;

	return !bacmp(&dev->bdaddr, addr);
}

static bool match_device_by_state(const void *data, const void *user_data)
{
	const struct gatt_device *dev = data;

	if (dev->state != PTR_TO_UINT(user_data))
		return false;

	return true;
}

static bool match_pending_device(const void *data, const void *user_data)
{
	const struct gatt_device *dev = data;

	if ((dev->state == DEVICE_CONNECT_INIT) ||
					(dev->state == DEVICE_CONNECT_READY))
		return true;

	return false;
}

static bool match_connection_by_id(const void *data, const void *user_data)
{
	const struct app_connection *conn = data;
	const int32_t id = PTR_TO_INT(user_data);

	return conn->id == id;
}

static bool match_connection_by_device_and_app(const void *data,
							const void *user_data)
{
	const struct app_connection *conn = data;
	const struct app_connection *match = user_data;

	return conn->device == match->device && conn->app == match->app;
}

static struct app_connection *find_connection_by_id(int32_t conn_id)
{
	struct app_connection *conn;

	conn = queue_find(app_connections, match_connection_by_id,
							INT_TO_PTR(conn_id));
	if (conn && conn->device->state == DEVICE_CONNECTED)
		return conn;

	return NULL;
}

static bool match_connection_by_device(const void *data, const void *user_data)
{
	const struct app_connection *conn = data;
	const struct gatt_device *dev = user_data;

	return conn->device == dev;
}

static bool match_connection_by_app(const void *data, const void *user_data)
{
	const struct app_connection *conn = data;
	const struct gatt_app *app = user_data;

	return conn->app == app;
}

static struct gatt_device *find_device_by_addr(const bdaddr_t *addr)
{
	return queue_find(gatt_devices, match_device_by_bdaddr, addr);
}

static struct gatt_device *find_pending_device(void)
{
	return queue_find(gatt_devices, match_pending_device, NULL);
}

static struct gatt_device *find_device_by_state(uint32_t state)
{
	return queue_find(gatt_devices, match_device_by_state,
							UINT_TO_PTR(state));
}

static bool match_srvc_by_element_id(const void *data, const void *user_data)
{
	const struct element_id *exp_id = user_data;
	const struct service *service = data;

	if (service->id.instance == exp_id->instance)
		return !bt_uuid_cmp(&service->id.uuid, &exp_id->uuid);

	return false;
}

static bool match_srvc_by_higher_inst_id(const void *data,
							const void *user_data)
{
	const struct service *s = data;
	uint8_t inst_id = PTR_TO_INT(user_data);

	/* For now we match inst_id as it is unique */
	return inst_id < s->id.instance;
}

static bool match_srvc_by_bt_uuid(const void *data, const void *user_data)
{
	const bt_uuid_t *exp_uuid = user_data;
	const struct service *service = data;

	return !bt_uuid_cmp(exp_uuid, &service->id.uuid);
}

static bool match_srvc_by_range(const void *data, const void *user_data)
{
	const struct service *srvc = data;
	const struct att_range *range = user_data;

	return !memcmp(&srvc->prim.range, range, sizeof(srvc->prim.range));
}

static bool match_char_by_higher_inst_id(const void *data,
							const void *user_data)
{
	const struct characteristic *ch = data;
	uint8_t inst_id = PTR_TO_INT(user_data);

	/* For now we match inst_id as it is unique, we'll match uuids later */
	return inst_id < ch->id.instance;
}

static bool match_descr_by_element_id(const void *data, const void *user_data)
{
	const struct element_id *exp_id = user_data;
	const struct descriptor *descr = data;

	if (exp_id->instance == descr->id.instance)
		return !bt_uuid_cmp(&descr->id.uuid, &exp_id->uuid);

	return false;
}

static bool match_descr_by_higher_inst_id(const void *data,
							const void *user_data)
{
	const struct descriptor *descr = data;
	uint8_t instance = PTR_TO_INT(user_data);

	/* For now we match instance as it is unique */
	return instance < descr->id.instance;
}

static bool match_notification(const void *a, const void *b)
{
	const struct notification_data *a1 = a;
	const struct notification_data *b1 = b;

	if (a1->conn != b1->conn)
		return false;

	if (memcmp(&a1->ch, &b1->ch, sizeof(a1->ch)))
		return false;

	if (memcmp(&a1->service, &b1->service, sizeof(a1->service)))
		return false;

	return true;
}

static bool match_char_by_element_id(const void *data, const void *user_data)
{
	const struct element_id *exp_id = user_data;
	const struct characteristic *chars = data;

	if (exp_id->instance == chars->id.instance)
		return !bt_uuid_cmp(&chars->id.uuid, &exp_id->uuid);

	return false;
}

static void destroy_notification(void *data)
{
	struct notification_data *notification = data;
	struct gatt_app *app;

	if (!notification)
		return;

	if (--notification->ref)
		return;

	app = notification->conn->app;
	queue_remove_if(app->notifications, match_notification, notification);
	free(notification);
}

static void unregister_notification(void *data)
{
	struct notification_data *notification = data;
	struct gatt_device *dev = notification->conn->device;

	/*
	 * No device means it was already disconnected and client cleanup was
	 * triggered afterwards, but once client unregisters, device stays if
	 * used by others. Then just unregister single handle.
	 */
	if (!queue_find(gatt_devices, NULL, dev))
		return;

	if (notification->notif_id && dev)
		g_attrib_unregister(dev->attrib, notification->notif_id);

	if (notification->ind_id && dev)
		g_attrib_unregister(dev->attrib, notification->ind_id);
}

static void device_set_state(struct gatt_device *dev, uint32_t state)
{
	char bda[18];

	if (dev->state == state)
		return;

	ba2str(&dev->bdaddr, bda);
	DBG("gatt: Device %s state changed %s -> %s", bda,
			device_state_str[dev->state], device_state_str[state]);

	dev->state = state;
}

static bool auto_connect_le(struct gatt_device *dev)
{
	/*  For LE devices use auto connect feature if possible */
	if (bt_kernel_conn_control()) {
		if (!bt_auto_connect_add(bt_get_id_addr(&dev->bdaddr, NULL)))
			return false;
	} else {
		/* Trigger discovery if not already started */
		if (!scanning && !bt_le_discovery_start()) {
			error("gatt: Could not start scan");
			return false;
		}
	}

	device_set_state(dev, DEVICE_CONNECT_INIT);
	return true;
}

static void connection_cleanup(struct gatt_device *device)
{
	if (device->watch_id) {
		g_source_remove(device->watch_id);
		device->watch_id = 0;
	}

	if (device->att_io) {
		g_io_channel_shutdown(device->att_io, FALSE, NULL);
		g_io_channel_unref(device->att_io);
		device->att_io = NULL;
	}

	if (device->attrib) {
		GAttrib *attrib = device->attrib;

		if (device->server_id > 0)
			g_attrib_unregister(device->attrib, device->server_id);

		if (device->ind_id > 0)
			g_attrib_unregister(device->attrib, device->ind_id);

		device->attrib = NULL;
		g_attrib_cancel_all(attrib);
		g_attrib_unref(attrib);
	}

	/*
	 * If device was in connection_pending or connectable state we
	 * search device list if we should stop the scan.
	 */
	if (!scanning && (device->state == DEVICE_CONNECT_INIT ||
				device->state == DEVICE_CONNECT_READY)) {
		if (!find_pending_device())
			bt_le_discovery_stop(NULL);
	}

	/* If device is not bonded service cache should be refreshed */
	if (!bt_device_is_bonded(&device->bdaddr))
		queue_remove_all(device->services, NULL, NULL, destroy_service);

	device_set_state(device, DEVICE_DISCONNECTED);

	if (!queue_isempty(device->autoconnect_apps))
		auto_connect_le(device);
	else
		bt_auto_connect_remove(&device->bdaddr);
}

static void destroy_gatt_app(void *data)
{
	struct gatt_app *app = data;

	if (!app)
		return;

	/*
	 * First we want to get all notifications and unregister them.
	 * We don't pass unregister_notification to queue_destroy,
	 * because destroy notification performs operations on queue
	 * too. So remove all elements and then destroy queue.
	 */

	if (app->type == GATT_CLIENT)
		while (queue_peek_head(app->notifications)) {
			struct notification_data *notification;

			notification = queue_pop_head(app->notifications);
			unregister_notification(notification);
		}

	queue_destroy(app->notifications, free);

	free(app);
}

struct pending_request {
	struct gatt_db_attribute *attrib;
	int length;
	uint8_t *value;
	uint16_t offset;

	uint8_t *filter_value;
	uint16_t filter_vlen;

	bool completed;
	uint8_t error;
};

static void destroy_pending_request(void *data)
{
	struct pending_request *entry = data;

	if (!entry)
		return;

	free(entry->value);
	free(entry->filter_value);
	free(entry);
}

static void destroy_device(void *data)
{
	struct gatt_device *dev = data;

	if (!dev)
		return;

	queue_destroy(dev->services, destroy_service);
	queue_destroy(dev->pending_requests, destroy_pending_request);
	queue_destroy(dev->autoconnect_apps, NULL);

	bt_auto_connect_remove(&dev->bdaddr);

	free(dev);
}

static struct gatt_device *device_ref(struct gatt_device *device)
{
	if (!device)
		return NULL;

	device->ref++;

	return device;
}

static void device_unref(struct gatt_device *device)
{
	if (!device)
		return;

	if (--device->ref)
		return;

	destroy_device(device);
}

static struct gatt_device *create_device(const bdaddr_t *addr)
{
	struct gatt_device *dev;

	dev = new0(struct gatt_device, 1);

	bacpy(&dev->bdaddr, addr);

	dev->services = queue_new();
	dev->autoconnect_apps = queue_new();
	dev->pending_requests = queue_new();

	queue_push_head(gatt_devices, dev);

	return device_ref(dev);
}

static void send_client_connect_status_notify(struct app_connection *conn,
								int32_t status)
{
	struct hal_ev_gatt_client_connect ev;

	if (conn->app->func) {
		conn->app->func(&conn->device->bdaddr,
					status == GATT_SUCCESS ? 0 : -ENOTCONN,
					conn->device->attrib);
		return;
	}

	ev.client_if = conn->app->id;
	ev.conn_id = conn->id;
	ev.status = status;

	bdaddr2android(&conn->device->bdaddr, &ev.bda);

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT, HAL_EV_GATT_CLIENT_CONNECT,
							sizeof(ev), &ev);
}

static void send_server_connection_state_notify(struct app_connection *conn,
								bool connected)
{
	struct hal_ev_gatt_server_connection ev;

	if (conn->app->func) {
		conn->app->func(&conn->device->bdaddr,
					connected ? 0 : -ENOTCONN,
					conn->device->attrib);
		return;
	}

	ev.server_if = conn->app->id;
	ev.conn_id = conn->id;
	ev.connected = connected;

	bdaddr2android(&conn->device->bdaddr, &ev.bdaddr);

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_EV_GATT_SERVER_CONNECTION, sizeof(ev), &ev);
}

static void send_client_disconnect_status_notify(struct app_connection *conn,
								int32_t status)
{
	struct hal_ev_gatt_client_disconnect ev;

	if (conn->app->func) {
		conn->app->func(&conn->device->bdaddr, -ENOTCONN,
						conn->device->attrib);
		return;
	}

	ev.client_if = conn->app->id;
	ev.conn_id = conn->id;
	ev.status = status;

	bdaddr2android(&conn->device->bdaddr, &ev.bda);

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_EV_GATT_CLIENT_DISCONNECT, sizeof(ev), &ev);

}

static void notify_app_disconnect_status(struct app_connection *conn,
								int32_t status)
{
	if (!conn->app)
		return;

	if (conn->app->type == GATT_CLIENT)
		send_client_disconnect_status_notify(conn, status);
	else
		send_server_connection_state_notify(conn, !!status);
}

static void notify_app_connect_status(struct app_connection *conn,
								int32_t status)
{
	if (!conn->app)
		return;

	if (conn->app->type == GATT_CLIENT)
		send_client_connect_status_notify(conn, status);
	else
		send_server_connection_state_notify(conn, !status);
}

static void destroy_connection(void *data)
{
	struct app_connection *conn = data;

	if (!conn)
		return;

	if (conn->timeout_id > 0)
		g_source_remove(conn->timeout_id);

	switch (conn->device->state) {
	case DEVICE_CONNECTED:
		notify_app_disconnect_status(conn, GATT_SUCCESS);
		break;
	case DEVICE_CONNECT_INIT:
	case DEVICE_CONNECT_READY:
		notify_app_connect_status(conn, GATT_FAILURE);
		break;
	case DEVICE_DISCONNECTED:
		break;
	}

	if (!queue_find(app_connections, match_connection_by_device,
							conn->device))
		connection_cleanup(conn->device);

	queue_destroy(conn->transactions, free);
	device_unref(conn->device);
	free(conn);
}

static gboolean disconnected_cb(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct gatt_device *dev = user_data;
	int sock, err = 0;
	socklen_t len;

	sock = g_io_channel_unix_get_fd(io);
	len = sizeof(err);
	if (!getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len))
		DBG("%s (%d)", strerror(err), err);

	queue_remove_all(app_connections, match_connection_by_device, dev,
							destroy_connection);

	return FALSE;
}

static bool get_local_mtu(struct gatt_device *dev, uint16_t *mtu)
{
	GIOChannel *io;
	uint16_t imtu, omtu;

	io = g_attrib_get_channel(dev->attrib);

	if (!bt_io_get(io, NULL, BT_IO_OPT_IMTU, &imtu, BT_IO_OPT_OMTU, &omtu,
							BT_IO_OPT_INVALID)) {
		error("gatt: Failed to get local MTU");
		return false;
	}

	/*
	 * Limit MTU to  MIN(IMTU, OMTU). This is to avoid situation where
	 * local OMTU < MIN(remote MTU, IMTU)
	 */
	if (mtu)
		*mtu = MIN(imtu, omtu);

	return true;
}

static void notify_client_mtu_change(struct app_connection *conn, bool success)
{
	struct hal_ev_gatt_client_configure_mtu ev;
	size_t mtu;

	g_attrib_get_buffer(conn->device->attrib, &mtu);

	ev.conn_id = conn->id;
	ev.status = success ? GATT_SUCCESS : GATT_FAILURE;
	ev.mtu = mtu;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_CLIENT_CONFIGURE_MTU, sizeof(ev), &ev);
}

static void notify_server_mtu(struct app_connection *conn)
{
	struct hal_ev_gatt_server_mtu_changed ev;
	size_t mtu;

	g_attrib_get_buffer(conn->device->attrib, &mtu);

	ev.conn_id = conn->id;
	ev.mtu = mtu;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_SERVER_MTU_CHANGED, sizeof(ev), &ev);
}

static void notify_mtu_change(void *data, void *user_data)
{
	struct gatt_device *device = user_data;
	struct app_connection *conn = data;

	if (conn->device != device)
		return;

	if (!conn->app) {
		error("gatt: can't notify mtu - no app registered for conn");
		return;
	}

	switch (conn->app->type) {
	case GATT_CLIENT:
		notify_client_mtu_change(conn, true);
		break;
	case GATT_SERVER:
		notify_server_mtu(conn);
		break;
	default:
		break;
	}
}

static bool update_mtu(struct gatt_device *device, uint16_t rmtu)
{
	uint16_t mtu, lmtu;

	if (!get_local_mtu(device, &lmtu))
		return false;

	DBG("remote_mtu:%d local_mtu:%d", rmtu, lmtu);

	if (rmtu < ATT_DEFAULT_LE_MTU) {
		error("gatt: remote MTU invalid (%u bytes)", rmtu);
		return false;
	}

	mtu = MIN(lmtu, rmtu);

	if (mtu == ATT_DEFAULT_LE_MTU)
		return true;

	if (!g_attrib_set_mtu(device->attrib, mtu)) {
		error("gatt: Failed to set MTU");
		return false;
	}

	queue_foreach(app_connections, notify_mtu_change, device);

	return true;
}

static void att_handler(const uint8_t *ipdu, uint16_t len, gpointer user_data);

static void exchange_mtu_cb(guint8 status, const guint8 *pdu, guint16 plen,
							gpointer user_data)
{
	struct gatt_device *device = user_data;
	uint16_t rmtu;

	DBG("");

	if (status) {
		error("gatt: MTU exchange: %s", att_ecode2str(status));
		goto failed;
	}

	if (!dec_mtu_resp(pdu, plen, &rmtu)) {
		error("gatt: MTU exchange: protocol error");
		goto failed;
	}

	update_mtu(device, rmtu);

failed:
	device_unref(device);
}

static void send_exchange_mtu_request(struct gatt_device *device)
{
	uint16_t mtu;

	if (!get_local_mtu(device, &mtu))
		return;

	DBG("mtu %u", mtu);

	if (!gatt_exchange_mtu(device->attrib, mtu, exchange_mtu_cb,
							device_ref(device)))
		device_unref(device);
}

static void ignore_confirmation_cb(guint8 status, const guint8 *pdu,
					guint16 len, gpointer user_data)
{
	/* Ignored. */
}

static void notify_att_range_change(struct gatt_device *dev,
							struct att_range *range)
{
	uint16_t handle;
	uint16_t length = 0;
	uint16_t ccc;
	uint8_t *pdu;
	size_t mtu;
	GAttribResultFunc confirmation_cb = NULL;

	handle = gatt_db_attribute_get_handle(service_changed_attrib);
	if (!handle)
		return;

	ccc = bt_get_gatt_ccc(&dev->bdaddr);
	if (!ccc)
		return;

	pdu = g_attrib_get_buffer(dev->attrib, &mtu);

	switch (ccc) {
	case 0x0001:
		length = enc_notification(handle, (uint8_t *) range,
						sizeof(*range), pdu, mtu);
		break;
	case 0x0002:
		length = enc_indication(handle, (uint8_t *) range,
						sizeof(*range), pdu, mtu);
		confirmation_cb = ignore_confirmation_cb;
		break;
	default:
		/* 0xfff4 reserved for future use */
		break;
	}

	g_attrib_send(dev->attrib, 0, pdu, length, confirmation_cb, NULL, NULL);
}

static struct app_connection *create_connection(struct gatt_device *device,
						struct gatt_app *app)
{
	struct app_connection *new_conn;
	static int32_t last_conn_id = 1;

	/* Check if already connected */
	new_conn = new0(struct app_connection, 1);

	/* Make connection id unique to connection record (app, device) pair */
	new_conn->app = app;
	new_conn->id = last_conn_id++;
	new_conn->transactions = queue_new();

	queue_push_head(app_connections, new_conn);

	new_conn->device = device_ref(device);

	return new_conn;
}

static struct service *create_service(uint8_t id, bool primary, char *uuid,
								void *data)
{
	struct service *s;

	s = new0(struct service, 1);

	if (bt_string_to_uuid(&s->id.uuid, uuid) < 0) {
		error("gatt: Cannot convert string to uuid");
		free(s);
		return NULL;
	}

	s->chars = queue_new();
	s->included = queue_new();
	s->id.instance = id;

	/* Put primary service to our local list */
	s->primary = primary;
	if (s->primary)
		memcpy(&s->prim, data, sizeof(s->prim));
	else
		memcpy(&s->incl, data, sizeof(s->incl));

	return s;
}

static void send_client_primary_notify(void *data, void *user_data)
{
	struct hal_ev_gatt_client_search_result ev;
	struct service *p = data;
	int32_t conn_id = PTR_TO_INT(user_data);

	/* In service queue we will have also included services */
	if (!p->primary)
		return;

	ev.conn_id  = conn_id;
	element_id_to_hal_srvc_id(&p->id, 1, &ev.srvc_id);

	uuid2android(&p->id.uuid, ev.srvc_id.uuid);

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_CLIENT_SEARCH_RESULT, sizeof(ev), &ev);
}

static void send_client_search_complete_notify(int32_t status, int32_t conn_id)
{
	struct hal_ev_gatt_client_search_complete ev;

	ev.status = status;
	ev.conn_id = conn_id;
	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_CLIENT_SEARCH_COMPLETE, sizeof(ev), &ev);
}

struct discover_srvc_data {
	bt_uuid_t uuid;
	struct app_connection *conn;
};

static void discover_srvc_by_uuid_cb(uint8_t status, GSList *ranges,
								void *user_data)
{
	struct discover_srvc_data *cb_data = user_data;
	struct gatt_primary prim;
	struct service *s;
	int32_t gatt_status;
	struct gatt_device *dev = cb_data->conn->device;
	uint8_t instance_id = queue_length(dev->services);

	DBG("Status %d", status);

	if (status) {
		error("gatt: Discover pri srvc filtered by uuid failed: %s",
							att_ecode2str(status));
		gatt_status = GATT_FAILURE;
		goto reply;
	}

	if (!ranges) {
		info("gatt: No primary services searched by uuid found");
		gatt_status = GATT_SUCCESS;
		goto reply;
	}

	bt_uuid_to_string(&cb_data->uuid, prim.uuid, sizeof(prim.uuid));

	for (; ranges; ranges = ranges->next) {
		memcpy(&prim.range, ranges->data, sizeof(prim.range));

		s = create_service(instance_id++, true, prim.uuid, &prim);
		if (!s) {
			gatt_status = GATT_FAILURE;
			goto reply;
		}

		queue_push_tail(dev->services, s);

		send_client_primary_notify(s, INT_TO_PTR(cb_data->conn->id));

		DBG("attr handle = 0x%04x, end grp handle = 0x%04x uuid: %s",
				prim.range.start, prim.range.end, prim.uuid);
	}

	/* Partial search service scanning was performed */
	dev->partial_srvc_search = true;
	gatt_status = GATT_SUCCESS;

reply:
	send_client_search_complete_notify(gatt_status, cb_data->conn->id);
	free(cb_data);
}

static void discover_srvc_all_cb(uint8_t status, GSList *services,
								void *user_data)
{
	struct discover_srvc_data *cb_data = user_data;
	struct gatt_device *dev = cb_data->conn->device;
	int32_t gatt_status;
	GSList *l;
	/*
	 * There might be multiply services with same uuid. Therefore make sure
	 * each primary service one has unique instance_id
	 */
	uint8_t instance_id = queue_length(dev->services);

	DBG("Status %d", status);

	if (status) {
		error("gatt: Discover all primary services failed: %s",
							att_ecode2str(status));
		gatt_status = GATT_FAILURE;
		goto reply;
	}

	if (!services) {
		info("gatt: No primary services found");
		gatt_status = GATT_SUCCESS;
		goto reply;
	}

	for (l = services; l; l = l->next) {
		struct gatt_primary *prim = l->data;
		struct service *p;

		if (queue_find(dev->services, match_srvc_by_range,
								&prim->range))
			continue;

		p = create_service(instance_id++, true, prim->uuid, prim);
		if (!p)
			continue;

		queue_push_tail(dev->services, p);

		DBG("attr handle = 0x%04x, end grp handle = 0x%04x uuid: %s",
			prim->range.start, prim->range.end, prim->uuid);
	}

	/*
	 * Send all found services notifications - first cache,
	 * then send notifies
	 */
	queue_foreach(dev->services, send_client_primary_notify,
						INT_TO_PTR(cb_data->conn->id));

	/* Full search service scanning was performed */
	dev->partial_srvc_search = false;
	gatt_status = GATT_SUCCESS;

reply:
	send_client_search_complete_notify(gatt_status, cb_data->conn->id);
	free(cb_data);
}

static gboolean connection_timeout(void *user_data)
{
	struct app_connection *conn = user_data;

	conn->timeout_id = 0;

	queue_remove(app_connections, conn);
	destroy_connection(conn);

	return FALSE;
}

static void discover_primary_cb(uint8_t status, GSList *services,
								void *user_data)
{
	struct discover_srvc_data *cb_data = user_data;
	struct app_connection *conn = cb_data->conn;
	struct gatt_device *dev = conn->device;
	GSList *l, *uuids = NULL;

	DBG("Status %d", status);

	if (status) {
		error("gatt: Discover all primary services failed: %s",
							att_ecode2str(status));
		free(cb_data);

		return;
	}

	if (!services) {
		info("gatt: No primary services found");
		free(cb_data);

		return;
	}

	for (l = services; l; l = l->next) {
		struct gatt_primary *prim = l->data;
		uint8_t *new_uuid;
		bt_uuid_t uuid, u128;

		DBG("uuid: %s", prim->uuid);

		if (bt_string_to_uuid(&uuid, prim->uuid) < 0) {
			error("gatt: Cannot convert string to uuid");
			continue;
		}

		bt_uuid_to_uuid128(&uuid, &u128);
		new_uuid = g_memdup(&u128.value.u128, sizeof(u128.value.u128));

		uuids = g_slist_prepend(uuids, new_uuid);
	}

	bt_device_set_uuids(&dev->bdaddr, uuids);

	free(cb_data);

	conn->timeout_id = g_timeout_add_seconds(GATT_CONN_TIMEOUT,
						connection_timeout, conn);
}

static guint search_dev_for_srvc(struct app_connection *conn, bt_uuid_t *uuid)
{
	struct discover_srvc_data *cb_data;

	cb_data = new0(struct discover_srvc_data, 1);
	cb_data->conn = conn;

	if (uuid) {
		memcpy(&cb_data->uuid, uuid, sizeof(cb_data->uuid));
		return gatt_discover_primary(conn->device->attrib, uuid,
					discover_srvc_by_uuid_cb, cb_data);
	}

	if (conn->app)
		return gatt_discover_primary(conn->device->attrib, NULL,
						discover_srvc_all_cb, cb_data);

	return gatt_discover_primary(conn->device->attrib, NULL,
						discover_primary_cb, cb_data);
}

struct connect_data {
	struct gatt_device *dev;
	int32_t status;
};

static void notify_app_connect_status_by_device(void *data, void *user_data)
{
	struct app_connection *conn = data;
	struct connect_data *con_data = user_data;

	if (conn->device == con_data->dev)
		notify_app_connect_status(conn, con_data->status);
}

static struct app_connection *find_conn_without_app(struct gatt_device *dev)
{
	struct app_connection conn_match;

	conn_match.device = dev;
	conn_match.app = NULL;

	return queue_find(app_connections, match_connection_by_device_and_app,
								&conn_match);
}

static struct app_connection *find_conn(const bdaddr_t *addr, int32_t app_id)
{
	struct app_connection conn_match;
	struct gatt_device *dev;
	struct gatt_app *app;

	/* Check if app is registered */
	app = find_app_by_id(app_id);
	if (!app) {
		error("gatt: Client id %d not found", app_id);
		return NULL;
	}

	/* Check if device is known */
	dev = find_device_by_addr(addr);
	if (!dev) {
		error("gatt: Client id %d not found", app_id);
		return NULL;
	}

	conn_match.device = dev;
	conn_match.app = app;

	return queue_find(app_connections, match_connection_by_device_and_app,
								&conn_match);
}

static void create_app_connection(void *data, void *user_data)
{
	struct gatt_device *dev = user_data;
	struct gatt_app *app;

	app = find_app_by_id(PTR_TO_INT(data));
	if (!app)
		return;

	DBG("Autoconnect application id=%d", app->id);

	if (!find_conn(&dev->bdaddr, PTR_TO_INT(data)))
		create_connection(dev, app);
}

static void ind_handler(const uint8_t *cmd, uint16_t cmd_len,
							gpointer user_data)
{
	struct gatt_device *dev = user_data;
	uint16_t resp_length = 0;
	size_t length;
	uint8_t *opdu = g_attrib_get_buffer(dev->attrib, &length);

	/*
	 * We have to send confirmation here. If some client is
	 * registered for this indication, event will be send in
	 * handle_notification
	 */

	resp_length = enc_confirmation(opdu, length);
	g_attrib_send(dev->attrib, 0, opdu, resp_length, NULL, NULL, NULL);
}

static void connect_cb(GIOChannel *io, GError *gerr, gpointer user_data)
{
	struct gatt_device *dev = user_data;
	struct connect_data data;
	struct att_range range;
	uint32_t status;
	GError *err = NULL;
	GAttrib *attrib;
	uint16_t mtu, cid;

	if (dev->state != DEVICE_CONNECT_READY) {
		error("gatt: Device not in a connecting state!?");
		g_io_channel_shutdown(io, TRUE, NULL);
		return;
	}

	if (dev->att_io) {
		g_io_channel_unref(dev->att_io);
		dev->att_io = NULL;
	}

	if (gerr) {
		error("gatt: connection failed %s", gerr->message);
		device_set_state(dev, DEVICE_DISCONNECTED);
		status = GATT_FAILURE;
		goto reply;
	}

	if (!bt_io_get(io, &err, BT_IO_OPT_IMTU, &mtu, BT_IO_OPT_CID, &cid,
							BT_IO_OPT_INVALID)) {
		error("gatt: Could not get imtu or cid: %s", err->message);
		device_set_state(dev, DEVICE_DISCONNECTED);
		status = GATT_FAILURE;
		g_error_free(err);
		goto reply;
	}

	/* on BR/EDR MTU must not be less then minimal allowed MTU */
	if (cid != ATT_CID && mtu < ATT_DEFAULT_L2CAP_MTU) {
		error("gatt: MTU too small (%u bytes)", mtu);
		device_set_state(dev, DEVICE_DISCONNECTED);
		status = GATT_FAILURE;
		goto reply;
	}

	DBG("mtu %u cid %u", mtu, cid);

	/* on LE we always start with default MTU */
	if (cid == ATT_CID)
		mtu = ATT_DEFAULT_LE_MTU;

	attrib = g_attrib_new(io, mtu, true);
	if (!attrib) {
		error("gatt: unable to create new GAttrib instance");
		device_set_state(dev, DEVICE_DISCONNECTED);
		status = GATT_FAILURE;
		goto reply;
	}

	dev->attrib = attrib;
	dev->watch_id = g_io_add_watch(io, G_IO_HUP | G_IO_ERR | G_IO_NVAL,
							disconnected_cb, dev);

	dev->server_id = g_attrib_register(attrib, GATTRIB_ALL_REQS,
						GATTRIB_ALL_HANDLES,
						att_handler, dev, NULL);
	dev->ind_id = g_attrib_register(attrib, ATT_OP_HANDLE_IND,
						GATTRIB_ALL_HANDLES,
						ind_handler, dev, NULL);
	if ((dev->server_id && dev->ind_id) == 0)
		error("gatt: Could not attach to server");

	device_set_state(dev, DEVICE_CONNECTED);

	/* Send exchange mtu request as we assume being client and server */
	/* TODO: Dont exchange mtu if no client apps */

	/* MTU exchange shall not be used on BR/EDR - Vol 3. Part G. 4.3.1 */
	if (cid == ATT_CID)
		send_exchange_mtu_request(dev);

	/*
	 * Service Changed Characteristic and CCC Descriptor handles
	 * should not change if there are bonded devices. We have them
	 * constant all the time, thus they should be excluded from
	 * range indicating changes.
	 */
	range.start = gatt_db_attribute_get_handle(service_changed_attrib) + 2;
	range.end = 0xffff;

	/*
	 * If there is ccc stored for that device we were acting as server for
	 * it, and as we dont have last connect and last services (de)activation
	 * timestamps we should always assume something has changed.
	 */
	notify_att_range_change(dev, &range);

	status = GATT_SUCCESS;

reply:
	/*
	 * Make sure there are app_connections for all apps interested in auto
	 * connect to that device
	 */
	queue_foreach(dev->autoconnect_apps, create_app_connection, dev);

	if (!queue_find(app_connections, match_connection_by_device, dev)) {
		struct app_connection *conn;

		if (!dev->attrib)
			return;

		conn = create_connection(dev, NULL);
		if (!conn)
			return;

		if (bt_is_pairing(&dev->bdaddr))
			/*
			 * If there is bonding ongoing lets wait for paired
			 * callback. Once we get that we can start search
			 * services
			 */
			conn->timeout_id = g_timeout_add_seconds(
						GATT_PAIR_CONN_TIMEOUT,
						connection_timeout, conn);
		else
			/*
			 * There is no ongoing bonding, lets search for primary
			 * services
			 */
			search_dev_for_srvc(conn, NULL);
	}

	data.dev = dev;
	data.status = status;
	queue_foreach(app_connections, notify_app_connect_status_by_device,
									&data);

	/* For BR/EDR notify about MTU since it is not negotiable*/
	if (cid != ATT_CID && status == GATT_SUCCESS)
		queue_foreach(app_connections, notify_mtu_change, dev);

	device_unref(dev);

	/* Check if we should restart scan */
	if (scanning)
		bt_le_discovery_start();

	/* FIXME: What to do if discovery won't start here. */
}

static int connect_le(struct gatt_device *dev)
{
	GIOChannel *io;
	GError *gerr = NULL;
	char addr[18];
	const bdaddr_t *bdaddr;
	uint8_t bdaddr_type;

	ba2str(&dev->bdaddr, addr);

	/* There is one connection attempt going on */
	if (dev->att_io) {
		info("gatt: connection to dev %s is ongoing", addr);
		return -EALREADY;
	}

	DBG("Connection attempt to: %s", addr);

	bdaddr = bt_get_id_addr(&dev->bdaddr, &bdaddr_type);

	/*
	 * This connection will help us catch any PDUs that comes before
	 * pairing finishes
	 */
	io = bt_io_connect(connect_cb, device_ref(dev), NULL, &gerr,
					BT_IO_OPT_SOURCE_BDADDR, &adapter_addr,
					BT_IO_OPT_SOURCE_TYPE, BDADDR_LE_PUBLIC,
					BT_IO_OPT_DEST_BDADDR, bdaddr,
					BT_IO_OPT_DEST_TYPE, bdaddr_type,
					BT_IO_OPT_CID, ATT_CID,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
					BT_IO_OPT_INVALID);
	if (!io) {
		error("gatt: Failed bt_io_connect(%s): %s", addr,
							gerr->message);
		g_error_free(gerr);
		return -EIO;
	}

	/* Keep this, so we can cancel the connection */
	dev->att_io = io;

	device_set_state(dev, DEVICE_CONNECT_READY);

	return 0;
}

static int connect_next_dev(void)
{
	struct gatt_device *dev;

	DBG("");

	dev = find_device_by_state(DEVICE_CONNECT_READY);
	if (!dev)
		return -ENODEV;

	return connect_le(dev);
}

static void bt_le_discovery_stop_cb(void)
{
	DBG("");

	/* Check now if there is any device ready to connect */
	if (connect_next_dev() < 0)
		bt_le_discovery_start();
}

static void le_device_found_handler(const bdaddr_t *addr, int rssi,
					uint16_t eir_len, const void *eir,
					bool connectable, bool bonded)
{
	uint8_t buf[IPC_MTU];
	struct hal_ev_gatt_client_scan_result *ev = (void *) buf;
	struct gatt_device *dev;
	char bda[18];

	if (!scanning)
		goto done;

	ba2str(addr, bda);
	DBG("LE Device found: %s, rssi: %d, adv_data: %d", bda, rssi, !!eir);

	bdaddr2android(addr, ev->bda);
	ev->rssi = rssi;
	ev->len = eir_len;

	memcpy(ev->adv_data, eir, ev->len);

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
						HAL_EV_GATT_CLIENT_SCAN_RESULT,
						sizeof(*ev) + ev->len, ev);

done:
	if (!connectable)
		return;

	/* We use auto connect feature from kernel if possible */
	if (bt_kernel_conn_control())
		return;

	dev = find_device_by_addr(addr);
	if (!dev) {
		if (!bonded)
			return;

		dev = create_device(addr);
	}

	if (dev->state != DEVICE_CONNECT_INIT)
		return;

	device_set_state(dev, DEVICE_CONNECT_READY);

	/*
	 * We are ok to perform connect now. Stop discovery
	 * and once it is stopped continue with creating ACL
	 */
	bt_le_discovery_stop(bt_le_discovery_stop_cb);
}

static struct gatt_app *register_app(const uint8_t *uuid, gatt_type_t type)
{
	static int32_t application_id = 1;
	struct gatt_app *app;

	if (queue_find(gatt_apps, match_app_by_uuid, uuid)) {
		error("gatt: app uuid is already on list");
		return NULL;
	}

	app = new0(struct gatt_app, 1);

	app->type = type;

	if (app->type == GATT_CLIENT)
		app->notifications = queue_new();

	memcpy(app->uuid, uuid, sizeof(app->uuid));

	app->id = application_id++;

	queue_push_head(gatt_apps, app);

	if (app->type == GATT_SERVER)
		queue_push_tail(listen_apps, INT_TO_PTR(app->id));

	return app;
}

static void handle_client_register(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_register *cmd = buf;
	struct hal_ev_gatt_client_register_client ev;
	struct gatt_app *app;

	DBG("");

	memset(&ev, 0, sizeof(ev));

	app = register_app(cmd->uuid, GATT_CLIENT);

	if (app) {
		ev.client_if = app->id;
		ev.status = GATT_SUCCESS;
	} else {
		ev.status = GATT_FAILURE;
	}

	/* We should send notification with given in cmd UUID */
	memcpy(ev.app_uuid, cmd->uuid, sizeof(ev.app_uuid));

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_CLIENT_REGISTER_CLIENT, sizeof(ev), &ev);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT, HAL_OP_GATT_CLIENT_REGISTER,
							HAL_STATUS_SUCCESS);
}

static void handle_client_scan(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_scan *cmd = buf;
	uint8_t status;

	DBG("new state %d", cmd->start);

	if (cmd->client_if != 0) {
		void *registered = find_app_by_id(cmd->client_if);

		if (!registered) {
			error("gatt: Client not registered");
			status = HAL_STATUS_FAILED;
			goto reply;
		}
	}

	/* Turn off scan */
	if (!cmd->start) {
		DBG("Stopping LE SCAN");

		if (scanning) {
			bt_le_discovery_stop(NULL);
			scanning = false;
		}

		status = HAL_STATUS_SUCCESS;
		goto reply;
	}

	/* Reply success if we already do scan */
	if (scanning) {
		status = HAL_STATUS_SUCCESS;
		goto reply;
	}

	/* Turn on scan */
	if (!bt_le_discovery_start()) {
		error("gatt: LE scan switch failed");
		status = HAL_STATUS_FAILED;
		goto reply;
	}

	scanning = true;
	status = HAL_STATUS_SUCCESS;

reply:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT, HAL_OP_GATT_CLIENT_SCAN,
									status);
}

static int connect_bredr(struct gatt_device *dev)
{
	BtIOSecLevel sec_level;
	GIOChannel *io;
	GError *gerr = NULL;
	char addr[18];

	ba2str(&dev->bdaddr, addr);

	/* There is one connection attempt going on */
	if (dev->att_io) {
		info("gatt: connection to dev %s is ongoing", addr);
		return -EALREADY;
	}

	DBG("Connection attempt to: %s", addr);

	sec_level = bt_device_is_bonded(&dev->bdaddr) ? BT_IO_SEC_MEDIUM :
								BT_IO_SEC_LOW;

	io = bt_io_connect(connect_cb, device_ref(dev), NULL, &gerr,
					BT_IO_OPT_SOURCE_BDADDR, &adapter_addr,
					BT_IO_OPT_SOURCE_TYPE, BDADDR_BREDR,
					BT_IO_OPT_DEST_BDADDR, &dev->bdaddr,
					BT_IO_OPT_DEST_TYPE, BDADDR_BREDR,
					BT_IO_OPT_PSM, ATT_PSM,
					BT_IO_OPT_SEC_LEVEL, sec_level,
					BT_IO_OPT_INVALID);
	if (!io) {
		error("gatt: Failed bt_io_connect(%s): %s", addr,
							gerr->message);
		g_error_free(gerr);
		return -EIO;
	}

	device_set_state(dev, DEVICE_CONNECT_READY);

	/* Keep this, so we can cancel the connection */
	dev->att_io = io;

	return 0;
}

static bool trigger_connection(struct app_connection *conn, bool direct)
{
	switch (conn->device->state) {
	case DEVICE_DISCONNECTED:
		/*
		 *  If device was last seen over BR/EDR connect over it.
		 *  Note: Connection state is handled in connect_bredr() func
		 */
		if (bt_device_last_seen_bearer(&conn->device->bdaddr) ==
								BDADDR_BREDR)
			return connect_bredr(conn->device) == 0;

		if (direct)
			return connect_le(conn->device) == 0;

		bt_gatt_add_autoconnect(conn->app->id, &conn->device->bdaddr);
		return auto_connect_le(conn->device);
	case DEVICE_CONNECTED:
		notify_app_connect_status(conn, GATT_SUCCESS);
		return true;
	case DEVICE_CONNECT_READY:
	case DEVICE_CONNECT_INIT:
	default:
		/* In those cases connection is already triggered. */
		return true;
	}
}

static void remove_autoconnect_device(struct gatt_device *dev)
{
	bt_auto_connect_remove(&dev->bdaddr);

	if (dev->state == DEVICE_CONNECT_INIT)
		device_set_state(dev, DEVICE_DISCONNECTED);

	device_unref(dev);
}

static void clear_autoconnect_devices(void *data, void *user_data)
{
	struct gatt_device *dev = data;

	if (queue_remove(dev->autoconnect_apps, user_data))
		if (queue_isempty(dev->autoconnect_apps))
			remove_autoconnect_device(dev);
}

static uint8_t unregister_app(int client_if)
{
	struct gatt_app *cl;

	/*
	 * Make sure that there is no devices in auto connect list for this
	 * application
	 */
	queue_foreach(gatt_devices, clear_autoconnect_devices,
							INT_TO_PTR(client_if));

	cl = queue_remove_if(gatt_apps, match_app_by_id, INT_TO_PTR(client_if));
	if (!cl) {
		error("gatt: client_if=%d not found", client_if);

		return HAL_STATUS_FAILED;
	}

	/* Destroy app connections with proper notifications for this app. */
	queue_remove_all(app_connections, match_connection_by_app, cl,
							destroy_connection);
	destroy_gatt_app(cl);

	return HAL_STATUS_SUCCESS;
}

static void send_client_listen_notify(int32_t id, int32_t status)
{
	struct hal_ev_gatt_client_listen ev;

	/* Server if because of typo in android headers */
	ev.server_if = id;
	ev.status = status;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT, HAL_EV_GATT_CLIENT_LISTEN,
							sizeof(ev), &ev);
}

struct listen_data {
	int32_t client_id;
	bool start;
};

static struct listen_data *create_listen_data(int32_t client_id, bool start)
{
	struct listen_data *d;

	d = new0(struct listen_data, 1);
	d->client_id = client_id;
	d->start = start;

	return d;
}

static void set_advertising_cb(uint8_t status, void *user_data)
{
	struct listen_data *l = user_data;

	send_client_listen_notify(l->client_id, status);

	/* In case of success update advertising state*/
	if (!status)
		advertising_cnt = l->start ? 1 : 0;

	/*
	 * Let's remove client from the list in two cases
	 * 1. Start failed
	 * 2. Stop succeed
	 */
	if ((l->start && status) || (!l->start && !status))
		queue_remove(listen_apps, INT_TO_PTR(l->client_id));

	free(l);
}

static void handle_client_unregister(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_unregister *cmd = buf;
	uint8_t status;
	void *listening_client;
	struct listen_data *data;

	DBG("");

	listening_client = queue_find(listen_apps, NULL,
						INT_TO_PTR(cmd->client_if));

	if (listening_client) {
		advertising_cnt--;
		queue_remove(listen_apps, INT_TO_PTR(cmd->client_if));
	} else {
		status = unregister_app(cmd->client_if);
		goto reply;
	}

	if (!advertising_cnt) {
		data = create_listen_data(cmd->client_if, false);

		if (!bt_le_set_advertising(data->start, set_advertising_cb,
								data)) {
			error("gatt: Could not set advertising");
			status = HAL_STATUS_FAILED;
			free(data);
			goto reply;
		}
	}

	status = unregister_app(cmd->client_if);

reply:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_UNREGISTER, status);
}

static uint8_t handle_connect(int32_t app_id, const bdaddr_t *addr, bool direct)
{
	struct app_connection conn_match;
	struct app_connection *conn;
	struct gatt_device *device;
	struct gatt_app *app;

	DBG("");

	app = find_app_by_id(app_id);
	if (!app)
		return HAL_STATUS_FAILED;

	device = find_device_by_addr(addr);
	if (!device)
		device = create_device(addr);

	conn_match.device = device;
	conn_match.app = app;

	conn = queue_find(app_connections, match_connection_by_device_and_app,
								&conn_match);
	if (!conn) {
		conn = create_connection(device, app);
		if (!conn)
			return HAL_STATUS_NOMEM;
	}

	if (!trigger_connection(conn, direct))
		return HAL_STATUS_FAILED;

	return HAL_STATUS_SUCCESS;
}

static void handle_client_connect(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_connect *cmd = buf;
	uint8_t status;
	bdaddr_t addr;

	DBG("is_direct:%u transport:%u", cmd->is_direct, cmd->transport);

	android2bdaddr(&cmd->bdaddr, &addr);

	/* TODO handle transport flag */

	status = handle_connect(cmd->client_if, &addr, cmd->is_direct);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT, HAL_OP_GATT_CLIENT_CONNECT,
								status);
}

static void handle_client_disconnect(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_disconnect *cmd = buf;
	struct app_connection *conn;
	uint8_t status;

	DBG("");

	/* TODO: should we care to match also bdaddr when conn_id is unique? */
	conn = queue_remove_if(app_connections, match_connection_by_id,
						INT_TO_PTR(cmd->conn_id));
	destroy_connection(conn);

	status = HAL_STATUS_SUCCESS;

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_DISCONNECT, status);
}

static void handle_client_listen(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_listen *cmd = buf;
	uint8_t status;
	struct listen_data *data;
	bool req_sent = false;
	void *listening_client;

	DBG("");

	if (!find_app_by_id(cmd->client_if)) {
		error("gatt: Client not registered");
		status = HAL_STATUS_FAILED;
		goto reply;
	}

	listening_client = queue_find(listen_apps, NULL,
						INT_TO_PTR(cmd->client_if));
	/* Start listening */
	if (cmd->start) {
		if (listening_client) {
			status = HAL_STATUS_SUCCESS;
			goto reply;
		}

		queue_push_tail(listen_apps, INT_TO_PTR(cmd->client_if));

		/* If listen is already on just return success*/
		if (advertising_cnt > 0) {
			advertising_cnt++;
			status = HAL_STATUS_SUCCESS;
			goto reply;
		}
	} else {
		/* Stop listening. Check if client was listening */
		if (!listening_client) {
			error("gatt: This client %d does not listen",
							cmd->client_if);
			status = HAL_STATUS_FAILED;
			goto reply;
		}

		/*
		 * In case there is more listening clients don't stop
		 * advertising
		 */
		if (advertising_cnt > 1) {
			advertising_cnt--;
			queue_remove(listen_apps, INT_TO_PTR(cmd->client_if));
			status = HAL_STATUS_SUCCESS;
			goto reply;
		}
	}

	data = create_listen_data(cmd->client_if, cmd->start);

	if (!bt_le_set_advertising(cmd->start, set_advertising_cb, data)) {
		error("gatt: Could not set advertising");
		status = HAL_STATUS_FAILED;
		free(data);
		goto reply;
	}

	/*
	 * Use this flag to keep in mind that we are waiting for callback with
	 * result
	 */
	req_sent = true;

	status = HAL_STATUS_SUCCESS;

reply:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT, HAL_OP_GATT_CLIENT_LISTEN,
							status);

	/* In case of early success or error, just send notification up */
	if (!req_sent) {
		int32_t gatt_status = status == HAL_STATUS_SUCCESS ?
						GATT_SUCCESS : GATT_FAILURE;
		send_client_listen_notify(cmd->client_if, gatt_status);
	}
}

static void handle_client_refresh(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_refresh *cmd = buf;
	struct gatt_device *dev;
	uint8_t status;
	bdaddr_t bda;

	/*
	 * This is Android's framework hidden API call. It seams that no
	 * notification is expected and Bluedroid silently updates device's
	 * cache under the hood. As we use lazy caching ,we can just clear the
	 * cache and we're done.
	 */

	DBG("");

	android2bdaddr(&cmd->bdaddr, &bda);
	dev = find_device_by_addr(&bda);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	queue_remove_all(dev->services, NULL, NULL, destroy_service);

	status = HAL_STATUS_SUCCESS;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT, HAL_OP_GATT_CLIENT_REFRESH,
									status);
}

static void handle_client_search_service(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_search_service *cmd = buf;
	struct app_connection *conn;
	uint8_t status;
	struct service *s;
	bt_uuid_t uuid;
	guint srvc_search_success;

	DBG("");

	if (len != sizeof(*cmd) + (cmd->filtered ? 16 : 0)) {
		error("Invalid search service size (%u bytes), terminating",
									len);
		raise(SIGTERM);
		return;
	}

	conn = find_connection_by_id(cmd->conn_id);
	if (!conn) {
		error("gatt: dev with conn_id=%d not found", cmd->conn_id);

		status = HAL_STATUS_FAILED;
		goto reply;
	}

	if (conn->device->state != DEVICE_CONNECTED) {
		char bda[18];

		ba2str(&conn->device->bdaddr, bda);
		error("gatt: device %s not connected", bda);

		status = HAL_STATUS_FAILED;
		goto reply;
	}

	if (cmd->filtered)
		android2uuid(cmd->filter_uuid, &uuid);

	/* Services not cached yet */
	if (queue_isempty(conn->device->services)) {
		if (cmd->filtered)
			srvc_search_success = search_dev_for_srvc(conn, &uuid);
		else
			srvc_search_success = search_dev_for_srvc(conn, NULL);

		if (!srvc_search_success) {
			status = HAL_STATUS_FAILED;
			goto reply;
		}

		status = HAL_STATUS_SUCCESS;
		goto reply;
	}

	/* Search in cached services for given service */
	if (cmd->filtered) {
		/* Search in cache for service by uuid */
		s = queue_find(conn->device->services, match_srvc_by_bt_uuid,
									&uuid);

		if (s) {
			send_client_primary_notify(s, INT_TO_PTR(conn->id));
		} else {
			if (!search_dev_for_srvc(conn, &uuid)) {
				status = HAL_STATUS_FAILED;
				goto reply;
			}

			status = HAL_STATUS_SUCCESS;
			goto reply;
		}
	} else {
		/* Refresh service cache if only partial search was performed */
		if (conn->device->partial_srvc_search) {
			srvc_search_success = search_dev_for_srvc(conn, NULL);
			if (!srvc_search_success) {
				status = HAL_STATUS_FAILED;
				goto reply;
			}
		} else
			queue_foreach(conn->device->services,
						send_client_primary_notify,
						INT_TO_PTR(cmd->conn_id));
	}

	send_client_search_complete_notify(GATT_SUCCESS, conn->id);

	status = HAL_STATUS_SUCCESS;

reply:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_OP_GATT_CLIENT_SEARCH_SERVICE, status);
}

static void send_client_incl_service_notify(const struct element_id *srvc_id,
						const struct service *incl,
						int32_t conn_id)
{
	struct hal_ev_gatt_client_get_inc_service ev;

	memset(&ev, 0, sizeof(ev));

	ev.conn_id = conn_id;

	element_id_to_hal_srvc_id(srvc_id, 1, &ev.srvc_id);

	if (incl) {
		element_id_to_hal_srvc_id(&incl->id, 0, &ev.incl_srvc_id);
		ev.status = GATT_SUCCESS;
	} else {
		ev.status = GATT_FAILURE;
	}

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT ,
					HAL_EV_GATT_CLIENT_GET_INC_SERVICE,
					sizeof(ev), &ev);
}

struct get_included_data {
	struct service *prim;
	struct app_connection *conn;
};

static int get_inst_id_of_prim_services(const struct gatt_device *dev)
{
	struct service *s = queue_peek_tail(dev->services);

	if (s)
		return s->id.instance;

	return -1;
}

static void get_included_cb(uint8_t status, GSList *included, void *user_data)
{
	struct get_included_data *data = user_data;
	struct app_connection *conn = data->conn;
	struct service *service = data->prim;
	struct service *incl = NULL;
	int instance_id;

	DBG("");

	free(data);

	if (status) {
		error("gatt: no included services found");
		goto failed;
	}

	/* Remember that we already search included services.*/
	service->incl_search_done = true;

	/*
	 * There might be multiply services with same uuid. Therefore make sure
	 * each service has unique instance id. Let's take the latest instance
	 * id of primary service and start iterate included services from this
	 * point.
	 */
	instance_id = get_inst_id_of_prim_services(conn->device);
	if (instance_id < 0)
		goto failed;

	for (; included; included = included->next) {
		struct gatt_included *included_service = included->data;

		incl = create_service(++instance_id, false,
							included_service->uuid,
							included_service);
		if (!incl)
			continue;

		/*
		 * Lets keep included service on two queues.
		 * 1. on services queue together with primary service
		 * 2. on special queue inside primary service
		 */
		queue_push_tail(service->included, incl);
		queue_push_tail(conn->device->services, incl);
	}

	/*
	 * Notify upper layer about first included service.
	 * Android framework will iterate for next one.
	 */
	incl = queue_peek_head(service->included);

failed:
	send_client_incl_service_notify(&service->id, incl, conn->id);
}

static void search_included_services(struct app_connection *conn,
							struct service *service)
{
	struct get_included_data *data;
	uint16_t start, end;

	data = new0(struct get_included_data, 1);
	data->prim = service;
	data->conn = conn;

	if (service->primary) {
		start = service->prim.range.start;
		end = service->prim.range.end;
	} else {
		start = service->incl.range.start;
		end = service->incl.range.end;
	}

	gatt_find_included(conn->device->attrib, start, end, get_included_cb,
									data);
}

static bool find_service(int32_t conn_id, struct element_id *service_id,
					struct app_connection **connection,
					struct service **service)
{
	struct service *srvc;
	struct app_connection *conn;

	conn = find_connection_by_id(conn_id);
	if (!conn) {
		error("gatt: conn_id=%d not found", conn_id);
		return false;
	}

	srvc = queue_find(conn->device->services, match_srvc_by_element_id,
								service_id);
	if (!srvc) {
		error("gatt: Service with inst_id: %d not found",
							service_id->instance);
		return false;
	}

	*connection = conn;
	*service = srvc;

	return true;
}

static void handle_client_get_included_service(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_get_included_service *cmd = buf;
	struct app_connection *conn;
	struct service *prim_service;
	struct service *incl_service = NULL;
	struct element_id match_id;
	struct element_id srvc_id;
	uint8_t status;

	DBG("");

	hal_srvc_id_to_element_id(&cmd->srvc_id, &srvc_id);

	if (len != sizeof(*cmd) +
			(cmd->continuation ? sizeof(cmd->incl_srvc_id[0]) : 0)) {
		error("Invalid get incl services size (%u bytes), terminating",
									len);
		raise(SIGTERM);
		return;
	}

	hal_srvc_id_to_element_id(&cmd->srvc_id, &match_id);
	if (!find_service(cmd->conn_id, &match_id, &conn, &prim_service)) {
		status = HAL_STATUS_FAILED;
		goto notify;
	}

	if (!prim_service->incl_search_done) {
		search_included_services(conn, prim_service);
		status = HAL_STATUS_SUCCESS;
		goto reply;
	}

	/* Try to use cache here */
	if (!cmd->continuation) {
		incl_service = queue_peek_head(prim_service->included);
	} else {
		uint8_t inst_id = cmd->incl_srvc_id[0].inst_id;

		incl_service = queue_find(prim_service->included,
						match_srvc_by_higher_inst_id,
						INT_TO_PTR(inst_id));
	}

	status = HAL_STATUS_SUCCESS;

notify:
	/*
	 * In case of error in handling request we need to send event with
	 * service id of cmd and gatt failure status.
	 */
	send_client_incl_service_notify(&srvc_id, incl_service, cmd->conn_id);

reply:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_OP_GATT_CLIENT_GET_INCLUDED_SERVICE, status);
}

static void send_client_char_notify(const struct hal_gatt_srvc_id *service,
					const struct hal_gatt_gatt_id *charac,
					int32_t char_prop, int32_t conn_id)
{
	struct hal_ev_gatt_client_get_characteristic ev;

	ev.conn_id = conn_id;

	if (charac) {
		memcpy(&ev.char_id, charac, sizeof(struct hal_gatt_gatt_id));
		ev.char_prop = char_prop;
		ev.status = GATT_SUCCESS;
	} else {
		memset(&ev.char_id, 0, sizeof(struct hal_gatt_gatt_id));
		ev.char_prop = 0;
		ev.status = GATT_FAILURE;
	}

	memcpy(&ev.srvc_id, service, sizeof(struct hal_gatt_srvc_id));

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_EV_GATT_CLIENT_GET_CHARACTERISTIC,
					sizeof(ev), &ev);
}

static void convert_send_client_char_notify(const struct characteristic *ch,
						int32_t conn_id,
						const struct service *service)
{
	struct hal_gatt_srvc_id srvc;
	struct hal_gatt_gatt_id charac;

	element_id_to_hal_srvc_id(&service->id, service->primary, &srvc);

	if (ch) {
		element_id_to_hal_gatt_id(&ch->id, &charac);
		send_client_char_notify(&srvc, &charac, ch->ch.properties,
								conn_id);
	} else {
		send_client_char_notify(&srvc, NULL, 0, conn_id);
	}
}

static void cache_all_srvc_chars(struct service *srvc, GSList *characteristics)
{
	uint16_t inst_id = 0;
	bt_uuid_t uuid;

	for (; characteristics; characteristics = characteristics->next) {
		struct characteristic *ch;

		ch = new0(struct characteristic, 1);
		ch->descriptors = queue_new();

		memcpy(&ch->ch, characteristics->data, sizeof(ch->ch));

		bt_string_to_uuid(&uuid, ch->ch.uuid);
		bt_uuid_to_uuid128(&uuid, &ch->id.uuid);

		/*
		 * For now we increment inst_id and use it as characteristic
		 * handle
		 */
		ch->id.instance = ++inst_id;

		/* Store end handle to use later for descriptors discovery */
		if (characteristics->next) {
			struct gatt_char *next = characteristics->next->data;

			ch->end_handle = next->handle - 1;
		} else {
			ch->end_handle = srvc->primary ? srvc->prim.range.end :
							srvc->incl.range.end;
		}

		DBG("attr handle = 0x%04x, end handle = 0x%04x uuid: %s",
				ch->ch.handle, ch->end_handle, ch->ch.uuid);

		queue_push_tail(srvc->chars, ch);
	}
}

struct discover_char_data {
	int32_t conn_id;
	struct service *service;
};

static void discover_char_cb(uint8_t status, GSList *characteristics,
								void *user_data)
{
	struct discover_char_data *data = user_data;
	struct service *srvc = data->service;

	if (status) {
		error("gatt: Failed to get characteristics: %s",
							att_ecode2str(status));
		convert_send_client_char_notify(NULL, data->conn_id, srvc);
		goto done;
	}

	if (queue_isempty(srvc->chars))
		cache_all_srvc_chars(srvc, characteristics);

	convert_send_client_char_notify(queue_peek_head(srvc->chars),
							data->conn_id, srvc);

done:
	free(data);
}

static void handle_client_get_characteristic(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_get_characteristic *cmd = buf;
	struct characteristic *ch;
	struct element_id match_id;
	struct app_connection *conn;
	struct service *srvc;
	uint8_t status;

	DBG("");

	if (len != sizeof(*cmd) + (cmd->continuation ? sizeof(cmd->char_id[0]) : 0)) {
		error("Invalid get characteristic size (%u bytes), terminating",
									len);
		raise(SIGTERM);
		return;
	}

	hal_srvc_id_to_element_id(&cmd->srvc_id, &match_id);
	if (!find_service(cmd->conn_id, &match_id, &conn, &srvc)) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	/* Discover all characteristics for services if not cached yet */
	if (queue_isempty(srvc->chars)) {
		struct discover_char_data *cb_data;
		struct att_range range;

		cb_data = new0(struct discover_char_data, 1);
		cb_data->service = srvc;
		cb_data->conn_id = conn->id;

		range = srvc->primary ? srvc->prim.range : srvc->incl.range;

		if (!gatt_discover_char(conn->device->attrib, range.start,
						range.end, NULL,
						discover_char_cb, cb_data)) {
			free(cb_data);

			status = HAL_STATUS_FAILED;
			goto done;
		}

		status = HAL_STATUS_SUCCESS;
		goto done;
	}

	if (cmd->continuation)
		ch = queue_find(srvc->chars, match_char_by_higher_inst_id,
					INT_TO_PTR(cmd->char_id[0].inst_id));
	else
		ch = queue_peek_head(srvc->chars);

	convert_send_client_char_notify(ch, conn->id, srvc);

	status = HAL_STATUS_SUCCESS;

done:
	if (status != HAL_STATUS_SUCCESS)
		send_client_char_notify(&cmd->srvc_id, NULL, 0, cmd->conn_id);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_CLIENT_GET_CHARACTERISTIC, status);
}

static void send_client_descr_notify(int32_t status, int32_t conn_id,
					bool primary,
					const struct element_id *srvc,
					const struct element_id *ch,
					const struct element_id *opt_descr)
{
	struct hal_ev_gatt_client_get_descriptor ev;

	memset(&ev, 0, sizeof(ev));

	ev.status = status;
	ev.conn_id = conn_id;

	element_id_to_hal_srvc_id(srvc, primary, &ev.srvc_id);
	element_id_to_hal_gatt_id(ch, &ev.char_id);

	if (opt_descr)
		element_id_to_hal_gatt_id(opt_descr, &ev.descr_id);

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_CLIENT_GET_DESCRIPTOR, sizeof(ev), &ev);
}

struct discover_desc_data {
	struct app_connection *conn;
	struct service *srvc;
	struct characteristic *ch;
};

static void gatt_discover_desc_cb(guint8 status, GSList *descs,
							gpointer user_data)
{
	struct discover_desc_data *data = user_data;
	struct app_connection *conn = data->conn;
	struct service *srvc = data->srvc;
	struct characteristic *ch = data->ch;
	struct descriptor *descr;
	int i = 0;

	if (status != 0) {
		error("Discover all characteristic descriptors failed [%s]: %s",
					ch->ch.uuid, att_ecode2str(status));
		goto reply;
	}

	for ( ; descs; descs = descs->next) {
		struct gatt_desc *desc = descs->data;
		bt_uuid_t uuid;

		descr = new0(struct descriptor, 1);

		bt_string_to_uuid(&uuid, desc->uuid);
		bt_uuid_to_uuid128(&uuid, &descr->id.uuid);

		descr->id.instance = ++i;
		descr->handle = desc->handle;

		DBG("attr handle = 0x%04x, uuid: %s", desc->handle, desc->uuid);

		queue_push_tail(ch->descriptors, descr);
	}

reply:
	descr = queue_peek_head(ch->descriptors);

	send_client_descr_notify(status ? GATT_FAILURE : GATT_SUCCESS, conn->id,
					srvc->primary, &srvc->id, &ch->id,
					descr ? &descr->id : NULL);

	free(data);
}

static bool build_descr_cache(struct app_connection *conn, struct service *srvc,
						struct characteristic *ch)
{
	struct discover_desc_data *cb_data;
	uint16_t start, end;

	/* Clip range to given characteristic */
	start = ch->ch.value_handle + 1;
	end = ch->end_handle;

	/* If there are no descriptors, notify with fail status. */
	if (start > end)
		return false;

	cb_data = new0(struct discover_desc_data, 1);
	cb_data->conn = conn;
	cb_data->srvc = srvc;
	cb_data->ch = ch;

	if (!gatt_discover_desc(conn->device->attrib, start, end, NULL,
					gatt_discover_desc_cb, cb_data)) {
		free(cb_data);
		return false;
	}

	return true;
}

static void handle_client_get_descriptor(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_get_descriptor *cmd = buf;
	struct descriptor *descr = NULL;
	struct characteristic *ch;
	struct service *srvc;
	struct element_id srvc_id;
	struct element_id char_id;
	struct app_connection *conn;
	int32_t conn_id;
	uint8_t primary;
	uint8_t status;

	DBG("");

	if (len != sizeof(*cmd) +
			(cmd->continuation ? sizeof(cmd->descr_id[0]) : 0)) {
		error("gatt: Invalid get descr command (%u bytes), terminating",
									len);

		raise(SIGTERM);
		return;
	}

	conn_id = cmd->conn_id;
	primary = cmd->srvc_id.is_primary;

	hal_srvc_id_to_element_id(&cmd->srvc_id, &srvc_id);
	hal_gatt_id_to_element_id(&cmd->char_id, &char_id);

	if (!find_service(conn_id, &srvc_id, &conn, &srvc)) {
		error("gatt: Get descr. could not find service");

		status = HAL_STATUS_FAILED;
		goto failed;
	}

	ch = queue_find(srvc->chars, match_char_by_element_id, &char_id);
	if (!ch) {
		error("gatt: Get descr. could not find characteristic");

		status = HAL_STATUS_FAILED;
		goto failed;
	}

	if (queue_isempty(ch->descriptors)) {
		if (build_descr_cache(conn, srvc, ch)) {
			ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_GET_DESCRIPTOR,
					HAL_STATUS_SUCCESS);
			return;
		}
	}

	status = HAL_STATUS_SUCCESS;

	/* Send from cache */
	if (cmd->continuation)
		descr = queue_find(ch->descriptors,
					match_descr_by_higher_inst_id,
					INT_TO_PTR(cmd->descr_id[0].inst_id));
	else
		descr = queue_peek_head(ch->descriptors);

failed:
	send_client_descr_notify(descr ? GATT_SUCCESS : GATT_FAILURE, conn_id,
						primary, &srvc_id, &char_id,
						&descr->id);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_CLIENT_GET_DESCRIPTOR, status);
}

struct char_op_data {
	int32_t conn_id;
	const struct element_id *srvc_id;
	const struct element_id *char_id;
	uint8_t primary;
};

static struct char_op_data *create_char_op_data(int32_t conn_id,
						const struct element_id *s_id,
						const struct element_id *ch_id,
						bool primary)
{
	struct char_op_data *d;

	d = new0(struct char_op_data, 1);
	d->conn_id = conn_id;
	d->srvc_id = s_id;
	d->char_id = ch_id;
	d->primary = primary;

	return d;
}

static void send_client_read_char_notify(int32_t status, const uint8_t *pdu,
						uint16_t len, int32_t conn_id,
						const struct element_id *s_id,
						const struct element_id *ch_id,
						uint8_t primary)
{
	uint8_t buf[IPC_MTU];
	struct hal_ev_gatt_client_read_characteristic *ev = (void *) buf;
	ssize_t vlen;

	memset(buf, 0, sizeof(buf));

	ev->conn_id = conn_id;
	ev->status = status;
	ev->data.status = status;

	element_id_to_hal_srvc_id(s_id, primary, &ev->data.srvc_id);
	element_id_to_hal_gatt_id(ch_id, &ev->data.char_id);

	if (status == 0 && pdu) {
		vlen = dec_read_resp(pdu, len, ev->data.value, sizeof(buf));
		if (vlen < 0) {
			error("gatt: Protocol error");
			ev->status = GATT_FAILURE;
		} else {
			ev->data.len = vlen;
		}
	}

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_EV_GATT_CLIENT_READ_CHARACTERISTIC,
					sizeof(*ev) + ev->data.len, ev);
}

static void read_char_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct char_op_data *data = user_data;

	send_client_read_char_notify(status, pdu, len, data->conn_id,
						data->srvc_id, data->char_id,
						data->primary);

	free(data);
}

static int get_cid(struct gatt_device *dev)
{
	GIOChannel *io;
	uint16_t cid;

	io = g_attrib_get_channel(dev->attrib);

	if (!bt_io_get(io, NULL, BT_IO_OPT_CID, &cid, BT_IO_OPT_INVALID)) {
		error("gatt: Failed to get CID");
		return -1;
	}

	return cid;
}

static int get_sec_level(struct gatt_device *dev)
{
	GIOChannel *io;
	int sec_level;

	io = g_attrib_get_channel(dev->attrib);

	if (!bt_io_get(io, NULL, BT_IO_OPT_SEC_LEVEL, &sec_level,
							BT_IO_OPT_INVALID)) {
		error("gatt: Failed to get sec_level");
		return -1;
	}

	return sec_level;
}

static bool set_security(struct gatt_device *device, int req_sec_level)
{
	int sec_level;
	GError *gerr = NULL;
	GIOChannel *io;

	sec_level = get_sec_level(device);
	if (sec_level < 0)
		return false;

	if (req_sec_level <= sec_level)
		return true;

	io = g_attrib_get_channel(device->attrib);
	if (!io)
		return false;

	bt_io_set(io, &gerr, BT_IO_OPT_SEC_LEVEL, req_sec_level,
							BT_IO_OPT_INVALID);
	if (gerr) {
		error("gatt: Failed to set security level: %s", gerr->message);
		g_error_free(gerr);
		return false;
	}

	return true;
}

bool bt_gatt_set_security(const bdaddr_t *bdaddr, int sec_level)
{
	struct gatt_device *device;

	device = find_device_by_addr(bdaddr);
	if (!device)
		return false;

	return set_security(device, sec_level);
}

static bool set_auth_type(struct gatt_device *device, int auth_type)
{
	int sec_level;

	switch (auth_type) {
	case HAL_GATT_AUTHENTICATION_MITM:
		sec_level = BT_SECURITY_HIGH;
		break;
	case HAL_GATT_AUTHENTICATION_NO_MITM:
		sec_level = BT_SECURITY_MEDIUM;
		break;
	case HAL_GATT_AUTHENTICATION_NONE:
		sec_level = BT_SECURITY_LOW;
		break;
	default:
		error("gatt: Invalid auth_type value: %d", auth_type);
		return false;
	}

	return set_security(device, sec_level);
}

static void handle_client_read_characteristic(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_read_characteristic *cmd = buf;
	struct char_op_data *cb_data;
	struct characteristic *ch;
	struct app_connection *conn;
	struct service *srvc;
	struct element_id srvc_id;
	struct element_id char_id;
	uint8_t status;

	DBG("");

	/* TODO authorization needs to be handled */

	hal_srvc_id_to_element_id(&cmd->srvc_id, &srvc_id);
	hal_gatt_id_to_element_id(&cmd->char_id, &char_id);

	if (!find_service(cmd->conn_id, &srvc_id, &conn, &srvc)) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	/* search characteristics by element id */
	ch = queue_find(srvc->chars, match_char_by_element_id, &char_id);
	if (!ch) {
		error("gatt: Characteristic with inst_id: %d not found",
							cmd->char_id.inst_id);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	cb_data = create_char_op_data(cmd->conn_id, &srvc->id, &ch->id,
						cmd->srvc_id.is_primary);

	if (!set_auth_type(conn->device, cmd->auth_req)) {
		error("gatt: Failed to set security %d", cmd->auth_req);
		status = HAL_STATUS_FAILED;
		free(cb_data);
		goto failed;
	}

	if (!gatt_read_char(conn->device->attrib, ch->ch.value_handle,
						read_char_cb, cb_data)) {
		error("gatt: Cannot read characteristic with inst_id: %d",
							cmd->char_id.inst_id);
		status = HAL_STATUS_FAILED;
		free(cb_data);
		goto failed;
	}

	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_CLIENT_READ_CHARACTERISTIC, status);

	/*
	 * We should send notification with service, characteristic id in case
	 * of errors.
	 */
	if (status != HAL_STATUS_SUCCESS)
		send_client_read_char_notify(GATT_FAILURE, NULL, 0,
						cmd->conn_id, &srvc_id,
						&char_id,
						cmd->srvc_id.is_primary);
}

static void send_client_write_char_notify(int32_t status, int32_t conn_id,
					const struct element_id *srvc_id,
					const struct element_id *char_id,
					uint8_t primary)
{
	struct hal_ev_gatt_client_write_characteristic ev;

	memset(&ev, 0, sizeof(ev));

	ev.conn_id = conn_id;
	ev.status = status;
	ev.data.status = status;

	element_id_to_hal_srvc_id(srvc_id, primary, &ev.data.srvc_id);
	element_id_to_hal_gatt_id(char_id, &ev.data.char_id);

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_EV_GATT_CLIENT_WRITE_CHARACTERISTIC,
					sizeof(ev), &ev);
}

static void write_char_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct char_op_data *data = user_data;

	send_client_write_char_notify(status, data->conn_id, data->srvc_id,
						data->char_id, data->primary);

	free(data);
}

static guint signed_write_cmd(struct gatt_device *dev, uint16_t handle,
					const uint8_t *value, uint16_t vlen)
{
	uint8_t csrk[16];
	uint32_t sign_cnt;
	guint res;

	memset(csrk, 0, 16);

	if (!bt_get_csrk(&dev->bdaddr, true, csrk, &sign_cnt, NULL)) {
		error("gatt: Could not get csrk key");
		return 0;
	}

	res = gatt_signed_write_cmd(dev->attrib, handle, value, vlen, crypto,
						csrk, sign_cnt, NULL, NULL);
	if (!res) {
		error("gatt: Signed write command failed");
		return 0;
	}

	bt_update_sign_counter(&dev->bdaddr, true, ++sign_cnt);

	return res;
}

static void handle_client_write_characteristic(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_write_characteristic *cmd = buf;
	struct char_op_data *cb_data = NULL;
	struct characteristic *ch;
	struct app_connection *conn;
	struct service *srvc;
	struct element_id srvc_id;
	struct element_id char_id;
	uint8_t status;
	guint res;

	DBG("");

	if (len != sizeof(*cmd) + cmd->len) {
		error("Invalid write char size (%u bytes), terminating", len);
		raise(SIGTERM);
		return;
	}

	hal_srvc_id_to_element_id(&cmd->srvc_id, &srvc_id);
	hal_gatt_id_to_element_id(&cmd->char_id, &char_id);

	if (!find_service(cmd->conn_id, &srvc_id, &conn, &srvc)) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	/* search characteristics by instance id */
	ch = queue_find(srvc->chars, match_char_by_element_id, &char_id);
	if (!ch) {
		error("gatt: Characteristic with inst_id: %d not found",
							cmd->char_id.inst_id);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	if (cmd->write_type == GATT_WRITE_TYPE_PREPARE ||
				cmd->write_type == GATT_WRITE_TYPE_DEFAULT) {
		cb_data = create_char_op_data(cmd->conn_id, &srvc->id, &ch->id,
						cmd->srvc_id.is_primary);
	}

	if (!set_auth_type(conn->device, cmd->auth_req)) {
		error("gatt: Failed to set security %d", cmd->auth_req);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	switch (cmd->write_type) {
	case GATT_WRITE_TYPE_NO_RESPONSE:
		res = gatt_write_cmd(conn->device->attrib, ch->ch.value_handle,
							cmd->value, cmd->len,
							NULL, NULL);
		break;
	case GATT_WRITE_TYPE_PREPARE:
		res = gatt_reliable_write_char(conn->device->attrib,
							ch->ch.value_handle,
							cmd->value, cmd->len,
							write_char_cb, cb_data);
		break;
	case GATT_WRITE_TYPE_DEFAULT:
		res = gatt_write_char(conn->device->attrib, ch->ch.value_handle,
							cmd->value, cmd->len,
							write_char_cb, cb_data);
		break;
	case GATT_WRITE_TYPE_SIGNED:
		if (get_cid(conn->device) != ATT_CID) {
			error("gatt: Cannot write signed on BR/EDR bearer");
			status = HAL_STATUS_FAILED;
			goto failed;
		}

		if (get_sec_level(conn->device) > BT_SECURITY_LOW)
			res = gatt_write_cmd(conn->device->attrib,
						ch->ch.value_handle, cmd->value,
						cmd->len, NULL, NULL);
		else
			res = signed_write_cmd(conn->device,
						ch->ch.value_handle, cmd->value,
						cmd->len);
		break;
	default:
		error("gatt: Write type %d unsupported", cmd->write_type);
		status = HAL_STATUS_UNSUPPORTED;
		goto failed;
	}

	if (!res) {
		error("gatt: Cannot write char. with inst_id: %d",
							cmd->char_id.inst_id);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_OP_GATT_CLIENT_WRITE_CHARACTERISTIC, status);

	/*
	 * We should send notification with service, characteristic id in case
	 * of error and write with no response
	 */
	if (status != HAL_STATUS_SUCCESS ||
			cmd->write_type == GATT_WRITE_TYPE_NO_RESPONSE ||
			cmd->write_type == GATT_WRITE_TYPE_SIGNED) {
		int32_t gatt_status = (status == HAL_STATUS_SUCCESS) ?
						GATT_SUCCESS : GATT_FAILURE;

		send_client_write_char_notify(gatt_status, cmd->conn_id,
						&srvc_id, &char_id,
						cmd->srvc_id.is_primary);
		free(cb_data);
	}
}

static void send_client_descr_read_notify(int32_t status, const uint8_t *pdu,
						guint16 len, int32_t conn_id,
						const struct element_id *srvc,
						const struct element_id *ch,
						const struct element_id *descr,
						uint8_t primary)
{
	uint8_t buf[IPC_MTU];
	struct hal_ev_gatt_client_read_descriptor *ev = (void *) buf;

	memset(buf, 0, sizeof(buf));

	ev->status = status;
	ev->conn_id = conn_id;
	ev->data.status = ev->status;

	element_id_to_hal_srvc_id(srvc, primary, &ev->data.srvc_id);
	element_id_to_hal_gatt_id(ch, &ev->data.char_id);
	element_id_to_hal_gatt_id(descr, &ev->data.descr_id);

	if (status == 0 && pdu) {
		ssize_t ret;

		ret = dec_read_resp(pdu, len, ev->data.value,
							GATT_MAX_ATTR_LEN);
		if (ret < 0) {
			error("gatt: Protocol error");
			ev->status = GATT_FAILURE;
		} else {
			ev->data.len = ret;
		}
	}

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_EV_GATT_CLIENT_READ_DESCRIPTOR,
					sizeof(*ev) + ev->data.len, ev);
}

struct desc_data {
	int32_t conn_id;
	const struct element_id *srvc_id;
	const struct element_id *char_id;
	const struct element_id *descr_id;
	uint8_t primary;
};

static void read_desc_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct desc_data *cb_data = user_data;

	if (status != 0)
		error("gatt: Discover all char descriptors failed: %s",
							att_ecode2str(status));

	send_client_descr_read_notify(status, pdu, len, cb_data->conn_id,
					cb_data->srvc_id, cb_data->char_id,
					cb_data->descr_id, cb_data->primary);

	free(cb_data);
}

static struct desc_data *create_desc_data(int32_t conn_id,
						const struct element_id *s_id,
						const struct element_id *ch_id,
						const struct element_id *d_id,
						uint8_t primary)
{
	struct desc_data *d;

	d = new0(struct desc_data, 1);
	d->conn_id = conn_id;
	d->srvc_id = s_id;
	d->char_id = ch_id;
	d->descr_id = d_id;
	d->primary = primary;

	return d;
}

static void handle_client_read_descriptor(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_read_descriptor *cmd = buf;
	struct desc_data *cb_data;
	struct characteristic *ch;
	struct descriptor *descr;
	struct service *srvc;
	struct element_id char_id;
	struct element_id descr_id;
	struct element_id srvc_id;
	struct app_connection *conn;
	int32_t conn_id = 0;
	uint8_t primary;
	uint8_t status;

	DBG("");

	conn_id = cmd->conn_id;
	primary = cmd->srvc_id.is_primary;

	hal_srvc_id_to_element_id(&cmd->srvc_id, &srvc_id);
	hal_gatt_id_to_element_id(&cmd->char_id, &char_id);
	hal_gatt_id_to_element_id(&cmd->descr_id, &descr_id);

	if (!find_service(conn_id, &srvc_id, &conn, &srvc)) {
		error("gatt: Read descr. could not find service");

		status = HAL_STATUS_FAILED;
		goto failed;
	}

	ch = queue_find(srvc->chars, match_char_by_element_id, &char_id);
	if (!ch) {
		error("gatt: Read descr. could not find characteristic");

		status = HAL_STATUS_FAILED;
		goto failed;
	}

	descr = queue_find(ch->descriptors, match_descr_by_element_id,
								&descr_id);
	if (!descr) {
		error("gatt: Read descr. could not find descriptor");

		status = HAL_STATUS_FAILED;
		goto failed;
	}

	cb_data = create_desc_data(conn_id, &srvc->id, &ch->id, &descr->id,
								primary);

	if (!set_auth_type(conn->device, cmd->auth_req)) {
		error("gatt: Failed to set security %d", cmd->auth_req);
		status = HAL_STATUS_FAILED;
		free(cb_data);
		goto failed;
	}

	if (!gatt_read_char(conn->device->attrib, descr->handle, read_desc_cb,
								cb_data)) {
		free(cb_data);

		status = HAL_STATUS_FAILED;
		goto failed;
	}

	status = HAL_STATUS_SUCCESS;

failed:
	if (status != HAL_STATUS_SUCCESS)
		send_client_descr_read_notify(GATT_FAILURE, NULL, 0, conn_id,
						&srvc_id, &char_id, &descr_id,
						primary);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_OP_GATT_CLIENT_READ_DESCRIPTOR, status);
}

static void send_client_descr_write_notify(int32_t status, int32_t conn_id,
						const struct element_id *srvc,
						const struct element_id *ch,
						const struct element_id *descr,
						uint8_t primary) {
	uint8_t buf[IPC_MTU];
	struct hal_ev_gatt_client_write_descriptor *ev = (void *) buf;

	memset(buf, 0, sizeof(buf));

	ev->status = status;
	ev->conn_id = conn_id;

	element_id_to_hal_srvc_id(srvc, primary, &ev->data.srvc_id);
	element_id_to_hal_gatt_id(ch, &ev->data.char_id);
	element_id_to_hal_gatt_id(descr, &ev->data.descr_id);
	ev->data.status = status;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_EV_GATT_CLIENT_WRITE_DESCRIPTOR,
					sizeof(*ev), ev);
}

static void write_descr_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct desc_data *cb_data = user_data;

	if (status)
		error("gatt: Write descriptors failed: %s",
							att_ecode2str(status));

	send_client_descr_write_notify(status, cb_data->conn_id,
					cb_data->srvc_id, cb_data->char_id,
					cb_data->descr_id, cb_data->primary);

	free(cb_data);
}

static void handle_client_write_descriptor(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_write_descriptor *cmd = buf;
	struct desc_data *cb_data = NULL;
	struct characteristic *ch;
	struct descriptor *descr;
	struct service *srvc;
	struct element_id srvc_id;
	struct element_id char_id;
	struct element_id descr_id;
	struct app_connection *conn;
	int32_t conn_id;
	uint8_t primary;
	uint8_t status;
	guint res;

	DBG("");

	if (len != sizeof(*cmd) + cmd->len) {
		error("Invalid write desriptor command (%u bytes), terminating",
									len);
		raise(SIGTERM);
		return;
	}

	primary = cmd->srvc_id.is_primary;
	conn_id = cmd->conn_id;

	hal_srvc_id_to_element_id(&cmd->srvc_id, &srvc_id);
	hal_gatt_id_to_element_id(&cmd->char_id, &char_id);
	hal_gatt_id_to_element_id(&cmd->descr_id, &descr_id);

	if (!find_service(cmd->conn_id, &srvc_id, &conn, &srvc)) {
		error("gatt: Write descr. could not find service");

		status = HAL_STATUS_FAILED;
		goto failed;
	}

	ch = queue_find(srvc->chars, match_char_by_element_id, &char_id);
	if (!ch) {
		error("gatt: Write descr. could not find characteristic");

		status = HAL_STATUS_FAILED;
		goto failed;
	}

	descr = queue_find(ch->descriptors, match_descr_by_element_id,
								&descr_id);
	if (!descr) {
		error("gatt: Write descr. could not find descriptor");

		status = HAL_STATUS_FAILED;
		goto failed;
	}

	if (cmd->write_type != GATT_WRITE_TYPE_NO_RESPONSE)
		cb_data = create_desc_data(conn_id, &srvc->id, &ch->id,
							&descr->id, primary);

	if (!set_auth_type(conn->device, cmd->auth_req)) {
		error("gatt: Failed to set security %d", cmd->auth_req);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	switch (cmd->write_type) {
	case GATT_WRITE_TYPE_NO_RESPONSE:
		res = gatt_write_cmd(conn->device->attrib, descr->handle,
					cmd->value, cmd->len, NULL , NULL);
		break;
	case GATT_WRITE_TYPE_PREPARE:
		res = gatt_reliable_write_char(conn->device->attrib,
						descr->handle, cmd->value,
						cmd->len, write_descr_cb,
						cb_data);
		break;
	case GATT_WRITE_TYPE_DEFAULT:
		res = gatt_write_char(conn->device->attrib, descr->handle,
						cmd->value, cmd->len,
						write_descr_cb, cb_data);
		break;
	default:
		error("gatt: Write type %d unsupported", cmd->write_type);
		status = HAL_STATUS_UNSUPPORTED;
		goto failed;
	}

	if (!res) {
		error("gatt: Write desc, could not write desc");
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	status = HAL_STATUS_SUCCESS;

failed:
	if (status != HAL_STATUS_SUCCESS ||
			cmd->write_type == GATT_WRITE_TYPE_NO_RESPONSE) {
		int32_t gatt_status = (status == HAL_STATUS_SUCCESS) ?
						GATT_SUCCESS : GATT_FAILURE;

		send_client_descr_write_notify(gatt_status, conn_id, &srvc_id,
						&char_id, &descr_id, primary);
		free(cb_data);
	}

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_CLIENT_WRITE_DESCRIPTOR, status);
}

static void send_client_write_execute_notify(int32_t id, int32_t status)
{
	struct hal_ev_gatt_client_exec_write ev;

	ev.conn_id = id;
	ev.status = status;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_EV_GATT_CLIENT_EXEC_WRITE,
					sizeof(ev), &ev);
}

static void write_execute_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	send_client_write_execute_notify(PTR_TO_INT(user_data), status);
}

static void handle_client_execute_write(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_execute_write *cmd = buf;
	struct app_connection *conn;
	uint8_t status;
	uint8_t flags;

	DBG("");

	conn = find_connection_by_id(cmd->conn_id);
	if (!conn) {
		status = HAL_STATUS_FAILED;
		goto reply;
	}

	flags = cmd->execute ? ATT_WRITE_ALL_PREP_WRITES :
						ATT_CANCEL_ALL_PREP_WRITES;

	if (!gatt_execute_write(conn->device->attrib, flags, write_execute_cb,
						INT_TO_PTR(cmd->conn_id))) {
		error("gatt: Could not send execute write");
		status = HAL_STATUS_FAILED;
		goto reply;
	}

	status = HAL_STATUS_SUCCESS;
reply:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_OP_GATT_CLIENT_EXECUTE_WRITE, status);

	/* In case of early error send also notification.*/
	if (status != HAL_STATUS_SUCCESS)
		send_client_write_execute_notify(cmd->conn_id, GATT_FAILURE);
}

static void handle_notification(const uint8_t *pdu, uint16_t len,
							gpointer user_data)
{
	uint8_t buf[IPC_MTU];
	struct hal_ev_gatt_client_notify *ev = (void *) buf;
	struct notification_data *notification = user_data;
	uint8_t data_offset = sizeof(uint8_t) + sizeof(uint16_t);

	if (len < data_offset)
		return;

	memcpy(&ev->char_id, &notification->ch, sizeof(ev->char_id));
	memcpy(&ev->srvc_id, &notification->service, sizeof(ev->srvc_id));
	bdaddr2android(&notification->conn->device->bdaddr, &ev->bda);
	ev->conn_id = notification->conn->id;
	ev->is_notify = pdu[0] == ATT_OP_HANDLE_NOTIFY;

	/* We have to cut opcode and handle from data */
	ev->len = len - data_offset;
	memcpy(ev->value, pdu + data_offset, len - data_offset);

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT, HAL_EV_GATT_CLIENT_NOTIFY,
						sizeof(*ev) + ev->len, ev);
}

static void send_register_for_notification_ev(int32_t id, int32_t registered,
					int32_t status,
					const struct hal_gatt_srvc_id *srvc,
					const struct hal_gatt_gatt_id *ch)
{
	struct hal_ev_gatt_client_reg_for_notif ev;

	ev.conn_id = id;
	ev.status = status;
	ev.registered = registered;
	memcpy(&ev.srvc_id, srvc, sizeof(ev.srvc_id));
	memcpy(&ev.char_id, ch, sizeof(ev.char_id));

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_CLIENT_REGISTER_FOR_NOTIF, sizeof(ev), &ev);
}

static void handle_client_register_for_notification(const void *buf,
								uint16_t len)
{
	const struct hal_cmd_gatt_client_register_for_notification *cmd = buf;
	struct notification_data *notification;
	struct characteristic *c;
	struct element_id match_id;
	struct app_connection *conn;
	int32_t conn_id = 0;
	struct service *service;
	uint8_t status;
	int32_t gatt_status;
	bdaddr_t addr;

	DBG("");

	android2bdaddr(&cmd->bdaddr, &addr);

	conn = find_conn(&addr, cmd->client_if);
	if (!conn) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	conn_id = conn->id;

	hal_srvc_id_to_element_id(&cmd->srvc_id, &match_id);
	service = queue_find(conn->device->services, match_srvc_by_element_id,
								&match_id);
	if (!service) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	hal_gatt_id_to_element_id(&cmd->char_id, &match_id);
	c = queue_find(service->chars, match_char_by_element_id, &match_id);
	if (!c) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	notification = new0(struct notification_data, 1);

	memcpy(&notification->ch, &cmd->char_id, sizeof(notification->ch));
	memcpy(&notification->service, &cmd->srvc_id,
						sizeof(notification->service));
	notification->conn = conn;

	if (queue_find(conn->app->notifications, match_notification,
								notification)) {
		free(notification);
		status = HAL_STATUS_SUCCESS;
		goto failed;
	}

	notification->notif_id = g_attrib_register(conn->device->attrib,
							ATT_OP_HANDLE_NOTIFY,
							c->ch.value_handle,
							handle_notification,
							notification,
							destroy_notification);
	if (!notification->notif_id) {
		free(notification);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	notification->ind_id = g_attrib_register(conn->device->attrib,
							ATT_OP_HANDLE_IND,
							c->ch.value_handle,
							handle_notification,
							notification,
							destroy_notification);
	if (!notification->ind_id) {
		g_attrib_unregister(conn->device->attrib,
							notification->notif_id);
		free(notification);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	/*
	 * Because same data - notification - is shared by two handlers, we
	 * introduce ref counter to be sure that data can be freed with no risk.
	 * Counter is decremented in destroy_notification.
	 */
	notification->ref = 2;

	queue_push_tail(conn->app->notifications, notification);

	status = HAL_STATUS_SUCCESS;

failed:
	gatt_status = status ? GATT_FAILURE : GATT_SUCCESS;
	send_register_for_notification_ev(conn_id, 1, gatt_status,
						&cmd->srvc_id, &cmd->char_id);
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_OP_GATT_CLIENT_REGISTER_FOR_NOTIFICATION, status);
}

static void handle_client_deregister_for_notification(const void *buf,
								uint16_t len)
{
	const struct hal_cmd_gatt_client_deregister_for_notification *cmd = buf;
	struct notification_data *notification, notif;
	struct app_connection *conn;
	int32_t conn_id = 0;
	uint8_t status;
	int32_t gatt_status;
	bdaddr_t addr;

	DBG("");

	android2bdaddr(&cmd->bdaddr, &addr);

	conn = find_conn(&addr, cmd->client_if);
	if (!conn) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	conn_id = conn->id;

	memcpy(&notif.ch, &cmd->char_id, sizeof(notif.ch));
	memcpy(&notif.service, &cmd->srvc_id, sizeof(notif.service));
	notif.conn = conn;

	notification = queue_find(conn->app->notifications,
						match_notification, &notif);
	if (!notification) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	unregister_notification(notification);

	status = HAL_STATUS_SUCCESS;

failed:
	gatt_status = status ? GATT_FAILURE : GATT_SUCCESS;
	send_register_for_notification_ev(conn_id, 0, gatt_status,
						&cmd->srvc_id, &cmd->char_id);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_OP_GATT_CLIENT_DEREGISTER_FOR_NOTIFICATION, status);
}

static void send_client_remote_rssi_notify(int32_t client_if,
						const bdaddr_t *addr,
						int32_t rssi, int32_t status)
{
	struct hal_ev_gatt_client_read_remote_rssi ev;

	ev.client_if = client_if;
	bdaddr2android(addr, &ev.address);
	ev.rssi = rssi;
	ev.status = status;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_CLIENT_READ_REMOTE_RSSI, sizeof(ev), &ev);
}

static void read_remote_rssi_cb(uint8_t status, const bdaddr_t *addr,
						int8_t rssi, void *user_data)
{
	int32_t client_if = PTR_TO_INT(user_data);
	int32_t gatt_status = status ? GATT_FAILURE : GATT_SUCCESS;

	send_client_remote_rssi_notify(client_if, addr, rssi, gatt_status);
}

static void handle_client_read_remote_rssi(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_read_remote_rssi *cmd = buf;
	uint8_t status;
	bdaddr_t bdaddr;

	DBG("");

	if (!find_app_by_id(cmd->client_if)) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	android2bdaddr(cmd->bdaddr, &bdaddr);
	if (!bt_read_device_rssi(&bdaddr, read_remote_rssi_cb,
						INT_TO_PTR(cmd->client_if))) {
		error("gatt: Could not read RSSI");
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_OP_GATT_CLIENT_READ_REMOTE_RSSI, status);

	if (status != HAL_STATUS_SUCCESS)
		send_client_remote_rssi_notify(cmd->client_if, &bdaddr, 0,
								GATT_FAILURE);
}

static void handle_client_get_device_type(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_get_device_type *cmd = buf;
	struct hal_rsp_gatt_client_get_device_type rsp;
	bdaddr_t bdaddr;

	DBG("");

	android2bdaddr(cmd->bdaddr, &bdaddr);

	rsp.type = bt_get_device_android_type(&bdaddr);

	ipc_send_rsp_full(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_GET_DEVICE_TYPE,
					sizeof(rsp), &rsp, -1);
}

static void handle_client_set_adv_data(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_set_adv_data *cmd = buf;
	uint8_t status;

	if (len != sizeof(*cmd) + cmd->manufacturer_len) {
		error("Invalid set adv data command (%u bytes), terminating",
									len);
		raise(SIGTERM);
		return;
	}

	DBG("scan_rsp=%u name=%u tx=%u min=%d max=%d app=%d",
		cmd->set_scan_rsp, cmd->include_name, cmd->include_txpower,
		cmd->min_interval, cmd->max_interval, cmd->appearance);

	DBG("manufacturer=%u service_data=%u service_uuid=%u",
				cmd->manufacturer_len, cmd->service_data_len,
				cmd->service_uuid_len);

	/* TODO This should be implemented when kernel supports it */
	if (cmd->manufacturer_len || cmd->service_data_len ||
							cmd->service_uuid_len) {
		error("gatt: Extra advertising data not supported");
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_CLIENT_SET_ADV_DATA, status);
}

static void test_command_result(guint8 status, const guint8 *pdu,
					guint16 len, gpointer user_data)
{
	DBG("status: %d", status);
}

static uint8_t test_read_write(bdaddr_t *bdaddr, bt_uuid_t *uuid, uint16_t op,
						uint16_t u2, uint16_t u3,
						uint16_t u4, uint16_t u5)
{
	guint16 length = 0;
	struct gatt_device *dev;
	uint8_t *pdu;
	size_t mtu;

	dev = find_device_by_addr(bdaddr);
	if (!dev || dev->state != DEVICE_CONNECTED)
		return HAL_STATUS_FAILED;

	pdu = g_attrib_get_buffer(dev->attrib, &mtu);
	if (!pdu)
		return HAL_STATUS_FAILED;

	switch (op) {
	case ATT_OP_READ_REQ:
		length = enc_read_req(u2, pdu, mtu);
		break;
	case ATT_OP_READ_BY_TYPE_REQ:
		length = enc_read_by_type_req(u2, u3, uuid, pdu, mtu);
		break;
	case ATT_OP_READ_BLOB_REQ:
		length = enc_read_blob_req(u2, u3, pdu, mtu);
		break;
	case ATT_OP_READ_BY_GROUP_REQ:
		length = enc_read_by_grp_req(u2, u3, uuid, pdu, mtu);
		break;
	case ATT_OP_READ_MULTI_REQ:
		return HAL_STATUS_UNSUPPORTED;
	case ATT_OP_WRITE_REQ:
		length = enc_write_req(u2, (uint8_t *) &u3, sizeof(u3), pdu,
									mtu);
		break;
	case ATT_OP_WRITE_CMD:
		length = enc_write_cmd(u2, (uint8_t *) &u3, sizeof(u3), pdu,
									mtu);
		break;
	case ATT_OP_PREP_WRITE_REQ:
		length = enc_prep_write_req(u2, u3, (uint8_t *) &u4, sizeof(u4),
								pdu, mtu);
		break;
	case ATT_OP_EXEC_WRITE_REQ:
		length = enc_exec_write_req(u2, pdu, mtu);
		break;
	case ATT_OP_SIGNED_WRITE_CMD:
		if (signed_write_cmd(dev, u2, (uint8_t *) &u3, sizeof(u3)))
			return HAL_STATUS_SUCCESS;
		else
			return HAL_STATUS_FAILED;
	default:
		error("gatt: Unknown operation type");

		return HAL_STATUS_UNSUPPORTED;
	}

	if (!g_attrib_send(dev->attrib, 0, pdu, length, test_command_result,
								NULL, NULL))
		return HAL_STATUS_FAILED;

	return HAL_STATUS_SUCCESS;
}

static uint8_t test_increase_security(bdaddr_t *bdaddr, uint16_t u1)
{
	struct gatt_device *device;

	device = find_device_by_addr(bdaddr);
	if (!device)
		return HAL_STATUS_FAILED;

	if (!set_auth_type(device, u1))
		return HAL_STATUS_FAILED;

	return HAL_STATUS_SUCCESS;
}

static void handle_client_test_command(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_test_command *cmd = buf;
	struct gatt_app *app;
	bdaddr_t bdaddr;
	bt_uuid_t uuid;
	uint8_t status;

	DBG("");

	android2bdaddr(cmd->bda1, &bdaddr);
	android2uuid(cmd->uuid1, &uuid);

	switch (cmd->command) {
	case GATT_CLIENT_TEST_CMD_ENABLE:
		if (cmd->u1) {
			if (!test_client_if) {
				app = register_app(TEST_UUID, GATT_CLIENT);
				if (app)
					test_client_if = app->id;
			}

			if (test_client_if)
				status = HAL_STATUS_SUCCESS;
			else
				status = HAL_STATUS_FAILED;
		} else {
			status = unregister_app(test_client_if);
			test_client_if = 0;
		}
		break;
	case GATT_CLIENT_TEST_CMD_CONNECT:
		/* TODO u1 holds device type, for now assume BLE */
		status = handle_connect(test_client_if, &bdaddr, false);
		break;
	case GATT_CLIENT_TEST_CMD_DISCONNECT:
		app = queue_find(gatt_apps, match_app_by_id,
						INT_TO_PTR(test_client_if));
		queue_remove_all(app_connections, match_connection_by_app, app,
							destroy_connection);

		status = HAL_STATUS_SUCCESS;
		break;
	case GATT_CLIENT_TEST_CMD_DISCOVER:
		status = HAL_STATUS_FAILED;
		break;
	case GATT_CLIENT_TEST_CMD_READ:
	case GATT_CLIENT_TEST_CMD_WRITE:
		status = test_read_write(&bdaddr, &uuid, cmd->u1, cmd->u2,
						cmd->u3, cmd->u4, cmd->u5);
		break;
	case GATT_CLIENT_TEST_CMD_INCREASE_SECURITY:
		status = test_increase_security(&bdaddr, cmd->u1);
		break;
	case GATT_CLIENT_TEST_CMD_PAIRING_CONFIG:
	default:
		status = HAL_STATUS_FAILED;
		break;
	}

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_CLIENT_TEST_COMMAND, status);
}

static void handle_server_register(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_server_register *cmd = buf;
	struct hal_ev_gatt_server_register ev;
	struct gatt_app *app;

	DBG("");

	memset(&ev, 0, sizeof(ev));

	app = register_app(cmd->uuid, GATT_SERVER);

	if (app) {
		ev.server_if = app->id;
		ev.status = GATT_SUCCESS;
	} else {
		ev.status = GATT_FAILURE;
	}

	memcpy(ev.uuid, cmd->uuid, sizeof(ev.uuid));

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_EV_GATT_SERVER_REGISTER, sizeof(ev), &ev);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT, HAL_OP_GATT_SERVER_REGISTER,
							HAL_STATUS_SUCCESS);
}

static void handle_server_unregister(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_server_unregister *cmd = buf;
	uint8_t status;

	DBG("");

	status = unregister_app(cmd->server_if);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_SERVER_UNREGISTER, status);
}

static void handle_server_connect(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_server_connect *cmd = buf;
	uint8_t status;
	bdaddr_t addr;

	DBG("");

	android2bdaddr(&cmd->bdaddr, &addr);

	/* TODO: Handle transport flag */

	status = handle_connect(cmd->server_if, &addr, cmd->is_direct);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT, HAL_OP_GATT_SERVER_CONNECT,
								status);
}

static void handle_server_disconnect(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_server_disconnect *cmd = buf;
	struct app_connection *conn;
	uint8_t status;

	DBG("");

	/* TODO: should we care to match also bdaddr when conn_id is unique? */
	conn = queue_remove_if(app_connections, match_connection_by_id,
						INT_TO_PTR(cmd->conn_id));
	destroy_connection(conn);

	status = HAL_STATUS_SUCCESS;

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_SERVER_DISCONNECT, status);
}

static void handle_server_add_service(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_server_add_service *cmd = buf;
	struct hal_ev_gatt_server_service_added ev;
	struct gatt_app *server;
	struct gatt_db_attribute *service;
	uint8_t status;
	bt_uuid_t uuid;

	DBG("");

	memset(&ev, 0, sizeof(ev));

	server = find_app_by_id(cmd->server_if);
	if (!server) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	android2uuid(cmd->srvc_id.uuid, &uuid);

	service = gatt_db_add_service(gatt_db, &uuid, cmd->srvc_id.is_primary,
							cmd->num_handles);
	if (!service) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	ev.srvc_handle = gatt_db_attribute_get_handle(service);
	if (!ev.srvc_handle) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	status = HAL_STATUS_SUCCESS;

failed:
	ev.status = status == HAL_STATUS_SUCCESS ? GATT_SUCCESS : GATT_FAILURE;
	ev.srvc_id = cmd->srvc_id;
	ev.server_if = cmd->server_if;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_SERVER_SERVICE_ADDED, sizeof(ev), &ev);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_SERVER_ADD_SERVICE, status);
}

static void handle_server_add_included_service(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_server_add_inc_service *cmd = buf;
	struct hal_ev_gatt_server_inc_srvc_added ev;
	struct gatt_app *server;
	struct gatt_db_attribute *service, *include;
	uint8_t status;

	DBG("");

	memset(&ev, 0, sizeof(ev));

	server = find_app_by_id(cmd->server_if);
	if (!server) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	service = gatt_db_get_attribute(gatt_db, cmd->service_handle);
	if (!service) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	include = gatt_db_get_attribute(gatt_db, cmd->included_handle);
	if (!include) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	service = gatt_db_service_add_included(service, include);
	if (!service) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	ev.incl_srvc_handle = gatt_db_attribute_get_handle(service);
	status = HAL_STATUS_SUCCESS;
failed:
	ev.srvc_handle = cmd->service_handle;
	ev.status = status;
	ev.server_if = cmd->server_if;
	ev.status = status == HAL_STATUS_SUCCESS ? GATT_SUCCESS : GATT_FAILURE;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_SERVER_INC_SRVC_ADDED, sizeof(ev), &ev);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_SERVER_ADD_INC_SERVICE, status);
}

static bool is_service(const bt_uuid_t *type)
{
	bt_uuid_t uuid;

	bt_uuid16_create(&uuid, GATT_PRIM_SVC_UUID);
	if (!bt_uuid_cmp(&uuid, type))
		return true;

	bt_uuid16_create(&uuid, GATT_SND_SVC_UUID);
	if (!bt_uuid_cmp(&uuid, type))
		return true;

	return false;
}

static bool match_pending_dev_request(const void *data, const void *user_data)
{
	const struct pending_request *pending_request = data;

	return !pending_request->completed;
}

static void send_dev_complete_response(struct gatt_device *device,
								uint8_t opcode)
{
	size_t mtu;
	uint8_t *rsp = g_attrib_get_buffer(device->attrib, &mtu);
	struct pending_request *val;
	uint16_t len = 0;
	uint8_t error = 0;

	if (queue_isempty(device->pending_requests))
		return;

	if (queue_find(device->pending_requests, match_pending_dev_request,
									NULL)) {
		DBG("Still pending requests");
		return;
	}

	val = queue_peek_head(device->pending_requests);
	if (!val) {
		error = ATT_ECODE_ATTR_NOT_FOUND;
		goto done;
	}

	if (val->error) {
		error = val->error;
		goto done;
	}

	switch (opcode) {
	case ATT_OP_READ_BY_TYPE_REQ: {
		struct att_data_list *adl;
		int iterator = 0;
		int length;
		struct queue *temp;

		temp = queue_new();

		val = queue_pop_head(device->pending_requests);
		if (!val) {
			queue_destroy(temp, NULL);
			error = ATT_ECODE_ATTR_NOT_FOUND;
			goto done;
		}

		if (val->error) {
			queue_destroy(temp, NULL);
			error = val->error;
			destroy_pending_request(val);
			goto done;
		}

		length = val->length;

		while (val && val->length == length && val->error == 0) {
			queue_push_tail(temp, val);
			val = queue_pop_head(device->pending_requests);
		}

		adl = att_data_list_alloc(queue_length(temp),
						sizeof(uint16_t) + length);

		destroy_pending_request(val);

		val = queue_pop_head(temp);
		while (val) {
			uint8_t *value = adl->data[iterator++];
			uint16_t handle;

			handle = gatt_db_attribute_get_handle(val->attrib);

			put_le16(handle, value);
			memcpy(&value[2], val->value, val->length);

			destroy_pending_request(val);
			val = queue_pop_head(temp);
		}

		len = enc_read_by_type_resp(adl, rsp, mtu);

		att_data_list_free(adl);
		queue_destroy(temp, destroy_pending_request);

		break;
	}
	case ATT_OP_READ_BLOB_REQ:
		len = enc_read_blob_resp(val->value, val->length, val->offset,
								rsp, mtu);
		break;
	case ATT_OP_READ_REQ:
		len = enc_read_resp(val->value, val->length, rsp, mtu);
		break;
	case ATT_OP_READ_BY_GROUP_REQ: {
		struct att_data_list *adl;
		int iterator = 0;
		int length;
		struct queue *temp;

		temp = queue_new();

		val = queue_pop_head(device->pending_requests);
		if (!val) {
			queue_destroy(temp, NULL);
			error = ATT_ECODE_ATTR_NOT_FOUND;
			goto done;
		}

		length = val->length;

		while (val && val->length == length) {
			queue_push_tail(temp, val);
			val = queue_pop_head(device->pending_requests);
		}

		adl = att_data_list_alloc(queue_length(temp),
						2 * sizeof(uint16_t) + length);

		val = queue_pop_head(temp);
		while (val) {
			uint8_t *value = adl->data[iterator++];
			uint16_t start_handle, end_handle;

			gatt_db_attribute_get_service_handles(val->attrib,
								&start_handle,
								&end_handle);

			put_le16(start_handle, value);
			put_le16(end_handle, &value[2]);
			memcpy(&value[4], val->value, val->length);

			destroy_pending_request(val);
			val = queue_pop_head(temp);
		}

		len = enc_read_by_grp_resp(adl, rsp, mtu);

		att_data_list_free(adl);
		queue_destroy(temp, destroy_pending_request);

		break;
	}
	case ATT_OP_FIND_BY_TYPE_REQ: {
		GSList *list = NULL;

		val = queue_pop_head(device->pending_requests);
		while (val) {
			struct att_range *range;
			const bt_uuid_t *type;

			/* Its find by type and value - filter by value here */
			if ((val->length != val->filter_vlen) ||
				memcmp(val->value, val->filter_value,
								val->length)) {

				destroy_pending_request(val);
				val = queue_pop_head(device->pending_requests);
				continue;
			}

			range = new0(struct att_range, 1);
			range->start = gatt_db_attribute_get_handle(
								val->attrib);

			type = gatt_db_attribute_get_type(val->attrib);
			if (is_service(type))
				gatt_db_attribute_get_service_handles(
								val->attrib,
								NULL,
								&range->end);
			else
				range->end = range->start;

			list = g_slist_append(list, range);

			destroy_pending_request(val);
			val = queue_pop_head(device->pending_requests);
		}

		if (list && !error)
			len = enc_find_by_type_resp(list, rsp, mtu);
		else
			error = ATT_ECODE_ATTR_NOT_FOUND;

		g_slist_free_full(list, free);

		break;
	}
	case ATT_OP_EXEC_WRITE_REQ:
		len = enc_exec_write_resp(rsp);
		break;
	case ATT_OP_WRITE_REQ:
		len = enc_write_resp(rsp);
		break;
	case ATT_OP_PREP_WRITE_REQ: {
		uint16_t handle;

		handle = gatt_db_attribute_get_handle(val->attrib);
		len = enc_prep_write_resp(handle, val->offset, val->value,
							val->length, rsp, mtu);
		break;
	}
	default:
		break;
	}

done:
	if (!len)
		len = enc_error_resp(opcode, 0x0000, error, rsp, mtu);

	g_attrib_send(device->attrib, 0, rsp, len, NULL, NULL, NULL);

	queue_remove_all(device->pending_requests, NULL, NULL,
						destroy_pending_request);
}

struct request_processing_data {
	uint8_t opcode;
	struct gatt_device *device;
};

static uint8_t check_device_permissions(struct gatt_device *device,
					uint8_t opcode, uint32_t permissions)
{
	GIOChannel *io;
	int sec_level;

	io = g_attrib_get_channel(device->attrib);

	if (!bt_io_get(io, NULL, BT_IO_OPT_SEC_LEVEL, &sec_level,
							BT_IO_OPT_INVALID))
		return ATT_ECODE_UNLIKELY;

	DBG("opcode 0x%02x permissions %u sec_level %u", opcode, permissions,
								sec_level);

	switch (opcode) {
	case ATT_OP_SIGNED_WRITE_CMD:
		if (!(permissions & GATT_PERM_WRITE_SIGNED))
				return ATT_ECODE_WRITE_NOT_PERM;

		if (permissions & GATT_PERM_WRITE_SIGNED_MITM) {
			bool auth;

			if (bt_get_csrk(&device->bdaddr, true, NULL, NULL,
					&auth) && auth)
				break;

			return ATT_ECODE_AUTHENTICATION;
		}
		break;
	case ATT_OP_READ_BY_TYPE_REQ:
	case ATT_OP_READ_REQ:
	case ATT_OP_READ_BLOB_REQ:
	case ATT_OP_READ_MULTI_REQ:
	case ATT_OP_READ_BY_GROUP_REQ:
	case ATT_OP_FIND_BY_TYPE_REQ:
	case ATT_OP_FIND_INFO_REQ:
		if (!(permissions & GATT_PERM_READ))
			return ATT_ECODE_READ_NOT_PERM;

		if ((permissions & GATT_PERM_READ_MITM) &&
						sec_level < BT_SECURITY_HIGH)
			return ATT_ECODE_AUTHENTICATION;

		if ((permissions & GATT_PERM_READ_ENCRYPTED) &&
						sec_level < BT_SECURITY_MEDIUM)
			return ATT_ECODE_INSUFF_ENC;

		if (permissions & GATT_PERM_READ_AUTHORIZATION)
			return ATT_ECODE_AUTHORIZATION;
		break;
	case ATT_OP_WRITE_REQ:
	case ATT_OP_WRITE_CMD:
	case ATT_OP_PREP_WRITE_REQ:
	case ATT_OP_EXEC_WRITE_REQ:
		if (!(permissions & GATT_PERM_WRITE))
			return ATT_ECODE_WRITE_NOT_PERM;

		if ((permissions & GATT_PERM_WRITE_MITM) &&
						sec_level < BT_SECURITY_HIGH)
			return ATT_ECODE_AUTHENTICATION;

		if ((permissions & GATT_PERM_WRITE_ENCRYPTED) &&
						sec_level < BT_SECURITY_MEDIUM)
			return ATT_ECODE_INSUFF_ENC;

		if (permissions & GATT_PERM_WRITE_AUTHORIZATION)
			return ATT_ECODE_AUTHORIZATION;
		break;
	default:
		return ATT_ECODE_UNLIKELY;
	}

	return 0;
}

static uint8_t err_to_att(int err)
{
	if (!err || (err > 0 && err < UINT8_MAX))
		return err;

	switch (err) {
	case -ENOENT:
		return ATT_ECODE_INVALID_HANDLE;
	case -ENOMEM:
		return ATT_ECODE_INSUFF_RESOURCES;
	default:
		return ATT_ECODE_UNLIKELY;
	}
}

static void attribute_read_cb(struct gatt_db_attribute *attrib, int err,
					const uint8_t *value, size_t length,
					void *user_data)
{
	struct pending_request *resp_data = user_data;
	uint8_t error = err_to_att(err);

	resp_data->attrib = attrib;
	resp_data->length = length;
	resp_data->error = error;

	resp_data->completed = true;

	if (!length)
		return;

	resp_data->value = malloc0(length);
	if (!resp_data->value) {
		resp_data->error = ATT_ECODE_INSUFF_RESOURCES;

		return;
	}

	memcpy(resp_data->value, value, length);
}

static void read_requested_attributes(void *data, void *user_data)
{
	struct pending_request *resp_data = data;
	struct request_processing_data *process_data = user_data;
	struct bt_att *att = g_attrib_get_att(process_data->device->attrib);
	struct gatt_db_attribute *attrib;
	uint32_t permissions;
	uint8_t error;

	attrib = resp_data->attrib;
	if (!attrib) {
		resp_data->error = ATT_ECODE_ATTR_NOT_FOUND;
		resp_data->completed = true;
		return;
	}

	permissions = gatt_db_attribute_get_permissions(attrib);

	/*
	 * Check if it is attribute we didn't declare permissions, like service
	 * declaration or included service. Set permissions to read only
	 */
	if (permissions == 0)
		permissions = GATT_PERM_READ;

	error = check_device_permissions(process_data->device,
							process_data->opcode,
							permissions);
	if (error != 0) {
		resp_data->error = error;
		resp_data->completed = true;
		return;
	}

	gatt_db_attribute_read(attrib, resp_data->offset, process_data->opcode,
					att, attribute_read_cb, resp_data);
}

static void process_dev_pending_requests(struct gatt_device *device,
							uint8_t att_opcode)
{
	struct request_processing_data process_data;

	if (queue_isempty(device->pending_requests))
		return;

	process_data.device = device;
	process_data.opcode = att_opcode;

	/* Process pending requests and prepare response */
	queue_foreach(device->pending_requests, read_requested_attributes,
								&process_data);

	send_dev_complete_response(device, att_opcode);
}

static struct pending_trans_data *conn_add_transact(struct app_connection *conn,
					uint8_t opcode,
					struct gatt_db_attribute *attrib,
					unsigned int serial_id)
{
	struct pending_trans_data *transaction;
	static int32_t trans_id = 1;

	transaction = new0(struct pending_trans_data, 1);
	transaction->id = trans_id++;
	transaction->opcode = opcode;
	transaction->attrib = attrib;
	transaction->serial_id = serial_id;

	queue_push_tail(conn->transactions, transaction);

	return transaction;
}

static bool get_dst_addr(struct bt_att *att, bdaddr_t *dst)
{
	GIOChannel *io = NULL;
	GError *gerr = NULL;

	io = g_io_channel_unix_new(bt_att_get_fd(att));
	if (!io)
		return false;

	bt_io_get(io, &gerr, BT_IO_OPT_DEST_BDADDR, dst, BT_IO_OPT_INVALID);
	if (gerr) {
		error("gatt: bt_io_get: %s", gerr->message);
		g_error_free(gerr);
		g_io_channel_unref(io);
		return false;
	}

	g_io_channel_unref(io);
	return true;
}

static void read_cb(struct gatt_db_attribute *attrib, unsigned int id,
			uint16_t offset, uint8_t opcode, struct bt_att *att,
			void *user_data)
{
	struct pending_trans_data *transaction;
	struct hal_ev_gatt_server_request_read ev;
	struct gatt_app *app;
	struct app_connection *conn;
	int32_t app_id = PTR_TO_INT(user_data);
	bdaddr_t bdaddr;

	DBG("id %u", id);

	app = find_app_by_id(app_id);
	if (!app) {
		error("gatt: read_cb, cound not found app id");
		goto failed;
	}

	if (!get_dst_addr(att, &bdaddr)) {
		error("gatt: read_cb, could not obtain dst BDADDR");
		goto failed;
	}

	conn = find_conn(&bdaddr, app->id);
	if (!conn) {
		error("gatt: read_cb, cound not found connection");
		goto failed;
	}

	memset(&ev, 0, sizeof(ev));

	/* Store the request data, complete callback and transaction id */
	transaction = conn_add_transact(conn, opcode, attrib, id);

	bdaddr2android(&bdaddr, ev.bdaddr);
	ev.conn_id = conn->id;
	ev.attr_handle = gatt_db_attribute_get_handle(attrib);
	ev.offset = offset;
	ev.is_long = opcode == ATT_OP_READ_BLOB_REQ;
	ev.trans_id = transaction->id;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_EV_GATT_SERVER_REQUEST_READ,
					sizeof(ev), &ev);

	return;

failed:
	gatt_db_attribute_read_result(attrib, id, -ENOENT, NULL, 0);
}

static void write_cb(struct gatt_db_attribute *attrib, unsigned int id,
			uint16_t offset, const uint8_t *value, size_t len,
			uint8_t opcode, struct bt_att *att, void *user_data)
{
	uint8_t buf[IPC_MTU];
	struct hal_ev_gatt_server_request_write *ev = (void *) buf;
	struct pending_trans_data *transaction;
	struct gatt_app *app;
	int32_t app_id = PTR_TO_INT(user_data);
	struct app_connection *conn;
	bdaddr_t bdaddr;

	DBG("id %u", id);

	app = find_app_by_id(app_id);
	if (!app) {
		error("gatt: write_cb could not found app id");
		goto failed;
	}

	if (!get_dst_addr(att, &bdaddr)) {
		error("gatt: write_cb, could not obtain dst BDADDR");
		goto failed;
	}

	conn = find_conn(&bdaddr, app->id);
	if (!conn) {
		error("gatt: write_cb could not found connection");
		goto failed;
	}

	/*
	 * Remember that this application has ongoing prep write
	 * Need it later to find out where to send execute write
	 */
	if (opcode == ATT_OP_PREP_WRITE_REQ)
		conn->wait_execute_write = true;

	/* Store the request data, complete callback and transaction id */
	transaction = conn_add_transact(conn, opcode, attrib, id);

	memset(ev, 0, sizeof(*ev));

	bdaddr2android(&bdaddr, &ev->bdaddr);
	ev->attr_handle = gatt_db_attribute_get_handle(attrib);
	ev->offset = offset;

	ev->conn_id = conn->id;
	ev->trans_id = transaction->id;

	ev->is_prep = opcode == ATT_OP_PREP_WRITE_REQ;

	if (opcode == ATT_OP_WRITE_REQ || opcode == ATT_OP_PREP_WRITE_REQ)
		ev->need_rsp = 0x01;
	else
		gatt_db_attribute_write_result(attrib, id, 0);

	ev->length = len;
	memcpy(ev->value, value, len);

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_SERVER_REQUEST_WRITE,
						sizeof(*ev) + ev->length , ev);
	return;

failed:
	gatt_db_attribute_write_result(attrib, id, ATT_ECODE_UNLIKELY);
}

static uint32_t android_to_gatt_permissions(int32_t hal_permissions)
{
	uint32_t permissions = 0;

	if (hal_permissions & HAL_GATT_PERMISSION_READ)
		permissions |= GATT_PERM_READ;

	if (hal_permissions & HAL_GATT_PERMISSION_READ_ENCRYPTED)
		permissions |= GATT_PERM_READ_ENCRYPTED | GATT_PERM_READ;

	if (hal_permissions & HAL_GATT_PERMISSION_READ_ENCRYPTED_MITM)
		permissions |= GATT_PERM_READ_MITM | GATT_PERM_READ_ENCRYPTED |
								GATT_PERM_READ;

	if (hal_permissions & HAL_GATT_PERMISSION_WRITE)
		permissions |= GATT_PERM_WRITE;

	if (hal_permissions & HAL_GATT_PERMISSION_WRITE_ENCRYPTED)
		permissions |= GATT_PERM_WRITE_ENCRYPTED | GATT_PERM_WRITE;

	if (hal_permissions & HAL_GATT_PERMISSION_WRITE_ENCRYPTED_MITM)
		permissions |= GATT_PERM_WRITE_MITM |
				GATT_PERM_WRITE_ENCRYPTED | GATT_PERM_WRITE;

	if (hal_permissions & HAL_GATT_PERMISSION_WRITE_SIGNED)
		permissions |= GATT_PERM_WRITE_SIGNED;

	if (hal_permissions & HAL_GATT_PERMISSION_WRITE_SIGNED_MITM)
		permissions |= GATT_PERM_WRITE_SIGNED_MITM |
							GATT_PERM_WRITE_SIGNED;

	return permissions;
}

static void handle_server_add_characteristic(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_server_add_characteristic *cmd = buf;
	struct hal_ev_gatt_server_characteristic_added ev;
	struct gatt_app *server;
	struct gatt_db_attribute *attrib;
	bt_uuid_t uuid;
	uint8_t status;
	uint32_t permissions;
	int32_t app_id = cmd->server_if;

	DBG("");

	memset(&ev, 0, sizeof(ev));

	server = find_app_by_id(app_id);
	if (!server) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	attrib = gatt_db_get_attribute(gatt_db, cmd->service_handle);
	if (!attrib) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	android2uuid(cmd->uuid, &uuid);
	permissions = android_to_gatt_permissions(cmd->permissions);

	attrib = gatt_db_service_add_characteristic(attrib,
							&uuid, permissions,
							cmd->properties,
							read_cb, write_cb,
							INT_TO_PTR(app_id));
	if (!attrib) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	ev.char_handle = gatt_db_attribute_get_handle(attrib);
	status = HAL_STATUS_SUCCESS;

failed:
	ev.srvc_handle = cmd->service_handle;
	ev.status = status;
	ev.server_if = app_id;
	ev.status = status == HAL_STATUS_SUCCESS ? GATT_SUCCESS : GATT_FAILURE;
	memcpy(ev.uuid, cmd->uuid, sizeof(cmd->uuid));

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_EV_GATT_SERVER_CHAR_ADDED, sizeof(ev), &ev);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_SERVER_ADD_CHARACTERISTIC, status);
}

static void handle_server_add_descriptor(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_server_add_descriptor *cmd = buf;
	struct hal_ev_gatt_server_descriptor_added ev;
	struct gatt_app *server;
	struct gatt_db_attribute *attrib;
	bt_uuid_t uuid;
	uint8_t status;
	uint32_t permissions;
	int32_t app_id = cmd->server_if;

	DBG("");

	memset(&ev, 0, sizeof(ev));

	server = find_app_by_id(app_id);
	if (!server) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	android2uuid(cmd->uuid, &uuid);
	permissions = android_to_gatt_permissions(cmd->permissions);

	attrib = gatt_db_get_attribute(gatt_db, cmd->service_handle);
	if (!attrib) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	attrib = gatt_db_service_add_descriptor(attrib, &uuid, permissions,
							read_cb, write_cb,
							INT_TO_PTR(app_id));
	if (!attrib) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	ev.descr_handle = gatt_db_attribute_get_handle(attrib);
	status = HAL_STATUS_SUCCESS;

failed:
	ev.server_if = app_id;
	ev.srvc_handle = cmd->service_handle;
	memcpy(ev.uuid, cmd->uuid, sizeof(cmd->uuid));
	ev.status = status == HAL_STATUS_SUCCESS ? GATT_SUCCESS : GATT_FAILURE;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_SERVER_DESCRIPTOR_ADDED, sizeof(ev), &ev);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_SERVER_ADD_DESCRIPTOR, status);
}

static void notify_service_change(void *data, void *user_data)
{
	struct att_range range;
	struct gatt_db_attribute *attrib = user_data;

	gatt_db_attribute_get_service_handles(attrib, &range.start, &range.end);

	/* In case of db error */
	if (!range.end)
		return;

	notify_att_range_change(data, &range);
}

static sdp_record_t *get_sdp_record(uuid_t *uuid, uint16_t start, uint16_t end,
							const char *name)
{
	sdp_list_t *svclass_id, *apseq, *proto[2], *root, *aproto;
	uuid_t root_uuid, proto_uuid, l2cap;
	sdp_record_t *record;
	sdp_data_t *psm, *sh, *eh;
	uint16_t lp = ATT_PSM;

	record = sdp_record_alloc();
	if (!record)
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

	if (name)
		sdp_set_info_attr(record, name, "BlueZ for Android", NULL);

	sdp_data_free(psm);
	sdp_data_free(sh);
	sdp_data_free(eh);
	sdp_list_free(proto[0], NULL);
	sdp_list_free(proto[1], NULL);
	sdp_list_free(apseq, NULL);
	sdp_list_free(aproto, NULL);

	return record;
}

static uint32_t add_sdp_record(const bt_uuid_t *uuid, uint16_t start,
						uint16_t end, const char *name)
{
	sdp_record_t *rec;
	uuid_t u, u32;

	switch (uuid->type) {
	case BT_UUID16:
		sdp_uuid16_create(&u, uuid->value.u16);
		break;
	case BT_UUID32:
		sdp_uuid32_create(&u32, uuid->value.u32);
		sdp_uuid32_to_uuid128(&u, &u32);
		break;
	case BT_UUID128:
		sdp_uuid128_create(&u, &uuid->value.u128);
		break;
	case BT_UUID_UNSPEC:
	default:
		return 0;
	}

	rec = get_sdp_record(&u, start, end, name);
	if (!rec)
		return 0;

	if (bt_adapter_add_record(rec, 0) < 0) {
		error("gatt: Failed to register SDP record");
		sdp_record_free(rec);
		return 0;
	}

	return rec->handle;
}

static bool match_service_sdp(const void *data, const void *user_data)
{
	const struct service_sdp *s = data;

	return s->service_handle == PTR_TO_INT(user_data);
}

static struct service_sdp *new_service_sdp_record(int32_t service_handle)
{
	bt_uuid_t uuid;
	struct service_sdp *s;
	struct gatt_db_attribute *attrib;
	uint16_t end_handle;

	attrib = gatt_db_get_attribute(gatt_db, service_handle);
	if (!attrib)
		return NULL;

	gatt_db_attribute_get_service_handles(attrib, NULL, &end_handle);
	if (!end_handle)
		return NULL;

	if (!gatt_db_attribute_get_service_uuid(attrib, &uuid))
		return NULL;

	s = new0(struct service_sdp, 1);
	s->service_handle = service_handle;
	s->sdp_handle = add_sdp_record(&uuid, service_handle, end_handle, NULL);
	if (!s->sdp_handle) {
		free(s);
		return NULL;
	}

	return s;
}

static void free_service_sdp_record(void *data)
{
	struct service_sdp *s = data;

	if (!s)
		return;

	bt_adapter_remove_record(s->sdp_handle);
	free(s);
}

static bool add_service_sdp_record(int32_t service_handle)
{
	struct service_sdp *s;

	s = queue_find(services_sdp, match_service_sdp,
						INT_TO_PTR(service_handle));
	if (s)
		return true;

	s = new_service_sdp_record(service_handle);
	if (!s)
		return false;

	queue_push_tail(services_sdp, s);

	return true;
}

static void remove_service_sdp_record(int32_t service_handle)
{
	struct service_sdp *s;

	s = queue_remove_if(services_sdp, match_service_sdp,
						INT_TO_PTR(service_handle));
	if (!s)
		return;

	free_service_sdp_record(s);
}

static void handle_server_start_service(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_server_start_service *cmd = buf;
	struct hal_ev_gatt_server_service_started ev;
	struct gatt_app *server;
	struct gatt_db_attribute *attrib;
	uint8_t status;

	DBG("transport 0x%02x", cmd->transport);

	memset(&ev, 0, sizeof(ev));

	if (cmd->transport == 0) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	server = find_app_by_id(cmd->server_if);
	if (!server) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	if (cmd->transport & GATT_SERVER_TRANSPORT_BREDR_BIT) {
		if (!add_service_sdp_record(cmd->service_handle)) {
			status = HAL_STATUS_FAILED;
			goto failed;
		}
	}
	/* TODO: Handle BREDR only */

	attrib = gatt_db_get_attribute(gatt_db, cmd->service_handle);
	if (!attrib) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	if (!gatt_db_service_set_active(attrib, true)) {
		/*
		 * no need to clean SDP since this can fail only if service
		 * handle is invalid in which case add_sdp_record() also fails
		 */
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	queue_foreach(gatt_devices, notify_service_change, attrib);

	status = HAL_STATUS_SUCCESS;

failed:
	ev.status = status == HAL_STATUS_SUCCESS ? GATT_SUCCESS : GATT_FAILURE;
	ev.server_if = cmd->server_if;
	ev.srvc_handle = cmd->service_handle;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_SERVER_SERVICE_STARTED, sizeof(ev), &ev);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_SERVER_START_SERVICE, status);
}

static void handle_server_stop_service(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_server_stop_service *cmd = buf;
	struct hal_ev_gatt_server_service_stopped ev;
	struct gatt_app *server;
	struct gatt_db_attribute *attrib;
	uint8_t status;

	DBG("");

	memset(&ev, 0, sizeof(ev));

	server = find_app_by_id(cmd->server_if);
	if (!server) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	attrib = gatt_db_get_attribute(gatt_db, cmd->service_handle);
	if (!attrib) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	if (!gatt_db_service_set_active(attrib, false)) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	remove_service_sdp_record(cmd->service_handle);

	status = HAL_STATUS_SUCCESS;

	queue_foreach(gatt_devices, notify_service_change, attrib);

failed:
	ev.status = status == HAL_STATUS_SUCCESS ? GATT_SUCCESS : GATT_FAILURE;
	ev.server_if = cmd->server_if;
	ev.srvc_handle = cmd->service_handle;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_SERVER_SERVICE_STOPPED, sizeof(ev), &ev);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_SERVER_STOP_SERVICE, status);
}

static void handle_server_delete_service(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_server_delete_service *cmd = buf;
	struct hal_ev_gatt_server_service_deleted ev;
	struct gatt_app *server;
	struct gatt_db_attribute *attrib;
	uint8_t status;

	DBG("");

	memset(&ev, 0, sizeof(ev));

	server = find_app_by_id(cmd->server_if);
	if (!server) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	attrib = gatt_db_get_attribute(gatt_db, cmd->service_handle);
	if (!attrib) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	if (!gatt_db_remove_service(gatt_db, attrib)) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	remove_service_sdp_record(cmd->service_handle);

	status = HAL_STATUS_SUCCESS;

failed:
	ev.status = status == HAL_STATUS_SUCCESS ? GATT_SUCCESS : GATT_FAILURE;
	ev.srvc_handle = cmd->service_handle;
	ev.server_if = cmd->server_if;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_SERVER_SERVICE_DELETED, sizeof(ev), &ev);

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_SERVER_DELETE_SERVICE, status);
}

static void indication_confirmation_cb(guint8 status, const guint8 *pdu,
						guint16 len, gpointer user_data)
{
	struct hal_ev_gatt_server_indication_sent ev;

	ev.status = status;
	ev.conn_id = PTR_TO_UINT(user_data);

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_SERVER_INDICATION_SENT, sizeof(ev), &ev);
}

static void handle_server_send_indication(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_server_send_indication *cmd = buf;
	struct app_connection *conn;
	uint8_t status;
	uint16_t length;
	uint8_t *pdu;
	size_t mtu;
	GAttribResultFunc confirmation_cb = NULL;

	DBG("");

	conn = find_connection_by_id(cmd->conn_id);
	if (!conn) {
		error("gatt: Could not find connection");
		status = HAL_STATUS_FAILED;
		goto reply;
	}

	pdu = g_attrib_get_buffer(conn->device->attrib, &mtu);

	if (cmd->confirm) {
		length = enc_indication(cmd->attribute_handle,
					(uint8_t *) cmd->value, cmd->len, pdu,
					mtu);
		confirmation_cb = indication_confirmation_cb;
	} else {
		length = enc_notification(cmd->attribute_handle,
						(uint8_t *) cmd->value,
						cmd->len, pdu, mtu);
	}

	if (!g_attrib_send(conn->device->attrib, 0, pdu, length,
				confirmation_cb, UINT_TO_PTR(conn->id), NULL)) {
		error("gatt: Failed to send indication");
		status = HAL_STATUS_FAILED;
	} else {
		status = HAL_STATUS_SUCCESS;
	}

	/* Here we confirm failed indications and all notifications */
	if (status || !confirmation_cb)
		indication_confirmation_cb(status, NULL, 0,
							UINT_TO_PTR(conn->id));

reply:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_SERVER_SEND_INDICATION, status);
}

static bool match_trans_id(const void *data, const void *user_data)
{
	const struct pending_trans_data *transaction = data;

	return transaction->id == PTR_TO_UINT(user_data);
}

static bool find_conn_waiting_exec_write(const void *data,
							const void *user_data)
{
	const struct app_connection *conn = data;

	return conn->wait_execute_write;
}

static bool pending_execute_write(void)
{
	return queue_find(app_connections, find_conn_waiting_exec_write, NULL);
}

static void handle_server_send_response(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_server_send_response *cmd = buf;
	struct pending_trans_data *transaction;
	struct app_connection *conn;
	uint8_t status;

	DBG("");

	conn = find_connection_by_id(cmd->conn_id);
	if (!conn) {
		error("gatt: could not found connection");
		status = HAL_STATUS_FAILED;
		goto reply;
	}

	transaction = queue_remove_if(conn->transactions, match_trans_id,
						UINT_TO_PTR(cmd->trans_id));
	if (!transaction) {
		error("gatt: transaction ID = %d not found", cmd->trans_id);
		status = HAL_STATUS_FAILED;
		goto reply;
	}

	if (transaction->opcode == ATT_OP_EXEC_WRITE_REQ) {
		struct pending_request *req;

		conn->wait_execute_write = false;

		/* Check for execute response from all server applications */
		if (pending_execute_write())
			goto done;

		/*
		 * This is usually done through db write callback but for
		 * execute write we dont have the attribute or handle to call
		 * gatt_db_attribute_write().
		 */
		req = queue_peek_head(conn->device->pending_requests);
		if (!req)
			goto done;

		/* Cast status to uint8_t, due to (byte) cast in java layer. */
		req->error = err_to_att((uint8_t) cmd->status);
		req->completed = true;

		/*
		 * FIXME: Handle situation when not all server applications
		 * respond with a success.
		 */
	}

	/* Cast status to uint8_t, due to (byte) cast in java layer. */
	if (transaction->opcode < ATT_OP_WRITE_REQ)
		gatt_db_attribute_read_result(transaction->attrib,
					transaction->serial_id,
					err_to_att((uint8_t) cmd->status),
					cmd->data, cmd->len);
	else
		gatt_db_attribute_write_result(transaction->attrib,
					transaction->serial_id,
					err_to_att((uint8_t) cmd->status));

	send_dev_complete_response(conn->device, transaction->opcode);

done:
	/* Clean request data */
	free(transaction);

	status = HAL_STATUS_SUCCESS;

reply:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_OP_GATT_SERVER_SEND_RESPONSE, status);
}

static void handle_client_scan_filter_setup(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_scan_filter_setup *cmd = buf;

	DBG("client_if %u", cmd->client_if);

	/* TODO */

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_SCAN_FILTER_SETUP,
					HAL_STATUS_UNSUPPORTED);
}

static void handle_client_scan_filter_add_remove(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_scan_filter_add_remove *cmd = buf;

	DBG("client_if %u", cmd->client_if);

	/* TODO */

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_CLIENT_SCAN_FILTER_ADD_REMOVE,
				HAL_STATUS_UNSUPPORTED);
}

static void handle_client_scan_filter_clear(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_scan_filter_clear *cmd = buf;

	DBG("client_if %u", cmd->client_if);

	/* TODO */

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_SCAN_FILTER_CLEAR,
					HAL_STATUS_UNSUPPORTED);
}

static void handle_client_scan_filter_enable(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_scan_filter_enable *cmd = buf;

	DBG("client_if %u", cmd->client_if);

	/* TODO */

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_SCAN_FILTER_ENABLE,
					HAL_STATUS_UNSUPPORTED);
}

static void handle_client_configure_mtu(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_configure_mtu *cmd = buf;
	static struct app_connection *conn;
	uint8_t status;

	DBG("conn_id %u mtu %d", cmd->conn_id, cmd->mtu);

	conn = find_connection_by_id(cmd->conn_id);
	if (!conn) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	/*
	 * currently MTU is always exchanged on connection, just report current
	 * value
	 *
	 * TODO figure out when send failed status in notification
	 * TODO should we fail for BR/EDR?
	 */
	notify_client_mtu_change(conn, false);
	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_CONFIGURE_MTU,
					status);
}

static void handle_client_conn_param_update(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_conn_param_update *cmd = buf;
	char address[18];
	bdaddr_t bdaddr;

	android2bdaddr(cmd->address, &bdaddr);
	ba2str(&bdaddr, address);

	DBG("%s", address);

	/* TODO */

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_CONN_PARAM_UPDATE,
					HAL_STATUS_UNSUPPORTED);
}

static void handle_client_set_scan_param(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_set_scan_param *cmd = buf;

	DBG("interval %d window %d", cmd->interval, cmd->window);

	/* TODO */

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_SET_SCAN_PARAM,
					HAL_STATUS_UNSUPPORTED);
}

static void handle_client_setup_multi_adv(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_setup_multi_adv *cmd = buf;

	DBG("client_if %d", cmd->client_if);

	/* TODO */

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_SETUP_MULTI_ADV,
					HAL_STATUS_UNSUPPORTED);
}

static void handle_client_update_multi_adv(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_update_multi_adv *cmd = buf;

	DBG("client_if %d", cmd->client_if);

	/* TODO */

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_UPDATE_MULTI_ADV,
					HAL_STATUS_UNSUPPORTED);
}

static void handle_client_setup_multi_adv_inst(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_setup_multi_adv_inst *cmd = buf;

	DBG("client_if %d", cmd->client_if);

	/* TODO */

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_SETUP_MULTI_ADV_INST,
					HAL_STATUS_UNSUPPORTED);
}

static void handle_client_disable_multi_adv_inst(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_disable_multi_adv_inst *cmd = buf;

	DBG("client_if %d", cmd->client_if);

	/* TODO */

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_CLIENT_DISABLE_MULTI_ADV_INST,
				HAL_STATUS_UNSUPPORTED);
}

static void handle_client_configure_batchscan(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_configure_batchscan *cmd = buf;

	DBG("client_if %d", cmd->client_if);

	/* TODO */

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_CONFIGURE_BATCHSCAN,
					HAL_STATUS_UNSUPPORTED);
}

static void handle_client_enable_batchscan(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_enable_batchscan *cmd = buf;

	DBG("client_if %d", cmd->client_if);

	/* TODO */

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_ENABLE_BATCHSCAN,
					HAL_STATUS_UNSUPPORTED);
}

static void handle_client_disable_batchscan(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_disable_batchscan *cmd = buf;

	DBG("client_if %d", cmd->client_if);

	/* TODO */

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
					HAL_OP_GATT_CLIENT_DISABLE_BATCHSCAN,
					HAL_STATUS_UNSUPPORTED);
}

static void handle_client_read_batchscan_reports(const void *buf, uint16_t len)
{
	const struct hal_cmd_gatt_client_read_batchscan_reports *cmd = buf;

	DBG("client_if %d", cmd->client_if);

	/* TODO */

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_GATT,
				HAL_OP_GATT_CLIENT_READ_BATCHSCAN_REPORTS,
				HAL_STATUS_UNSUPPORTED);
}

static const struct ipc_handler cmd_handlers[] = {
	/* HAL_OP_GATT_CLIENT_REGISTER */
	{ handle_client_register, false,
		sizeof(struct hal_cmd_gatt_client_register) },
	/* HAL_OP_GATT_CLIENT_UNREGISTER */
	{ handle_client_unregister, false,
		sizeof(struct hal_cmd_gatt_client_unregister) },
	/* HAL_OP_GATT_CLIENT_SCAN */
	{ handle_client_scan, false,
		sizeof(struct hal_cmd_gatt_client_scan) },
	/* HAL_OP_GATT_CLIENT_CONNECT */
	{ handle_client_connect, false,
		sizeof(struct hal_cmd_gatt_client_connect) },
	/* HAL_OP_GATT_CLIENT_DISCONNECT */
	{ handle_client_disconnect, false,
		sizeof(struct hal_cmd_gatt_client_disconnect) },
	/* HAL_OP_GATT_CLIENT_LISTEN */
	{ handle_client_listen, false,
		sizeof(struct hal_cmd_gatt_client_listen) },
	/* HAL_OP_GATT_CLIENT_REFRESH */
	{ handle_client_refresh, false,
		sizeof(struct hal_cmd_gatt_client_refresh) },
	/* HAL_OP_GATT_CLIENT_SEARCH_SERVICE */
	{ handle_client_search_service, true,
		sizeof(struct hal_cmd_gatt_client_search_service) },
	/* HAL_OP_GATT_CLIENT_GET_INCLUDED_SERVICE */
	{ handle_client_get_included_service, true,
		sizeof(struct hal_cmd_gatt_client_get_included_service) },
	/* HAL_OP_GATT_CLIENT_GET_CHARACTERISTIC */
	{ handle_client_get_characteristic, true,
		sizeof(struct hal_cmd_gatt_client_get_characteristic) },
	/* HAL_OP_GATT_CLIENT_GET_DESCRIPTOR */
	{ handle_client_get_descriptor, true,
		sizeof(struct hal_cmd_gatt_client_get_descriptor) },
	/* HAL_OP_GATT_CLIENT_READ_CHARACTERISTIC */
	{ handle_client_read_characteristic, false,
		sizeof(struct hal_cmd_gatt_client_read_characteristic) },
	/* HAL_OP_GATT_CLIENT_WRITE_CHARACTERISTIC */
	{ handle_client_write_characteristic, true,
		sizeof(struct hal_cmd_gatt_client_write_characteristic) },
	/* HAL_OP_GATT_CLIENT_READ_DESCRIPTOR */
	{ handle_client_read_descriptor, false,
		sizeof(struct hal_cmd_gatt_client_read_descriptor) },
	/* HAL_OP_GATT_CLIENT_WRITE_DESCRIPTOR */
	{ handle_client_write_descriptor, true,
		sizeof(struct hal_cmd_gatt_client_write_descriptor) },
	/* HAL_OP_GATT_CLIENT_EXECUTE_WRITE */
	{ handle_client_execute_write, false,
		sizeof(struct hal_cmd_gatt_client_execute_write)},
	/* HAL_OP_GATT_CLIENT_REGISTER_FOR_NOTIFICATION */
	{ handle_client_register_for_notification, false,
		sizeof(struct hal_cmd_gatt_client_register_for_notification) },
	/* HAL_OP_GATT_CLIENT_DEREGISTER_FOR_NOTIFICATION */
	{ handle_client_deregister_for_notification, false,
		sizeof(struct hal_cmd_gatt_client_deregister_for_notification) },
	/* HAL_OP_GATT_CLIENT_READ_REMOTE_RSSI */
	{ handle_client_read_remote_rssi, false,
		sizeof(struct hal_cmd_gatt_client_read_remote_rssi) },
	/* HAL_OP_GATT_CLIENT_GET_DEVICE_TYPE */
	{ handle_client_get_device_type, false,
		sizeof(struct hal_cmd_gatt_client_get_device_type) },
	/* HAL_OP_GATT_CLIENT_SET_ADV_DATA */
	{ handle_client_set_adv_data, true,
		sizeof(struct hal_cmd_gatt_client_set_adv_data) },
	/* HAL_OP_GATT_CLIENT_TEST_COMMAND */
	{ handle_client_test_command, false,
		sizeof(struct hal_cmd_gatt_client_test_command) },
	/* HAL_OP_GATT_SERVER_REGISTER */
	{ handle_server_register, false,
		sizeof(struct hal_cmd_gatt_server_register) },
	/* HAL_OP_GATT_SERVER_UNREGISTER */
	{ handle_server_unregister, false,
		sizeof(struct hal_cmd_gatt_server_unregister) },
	/* HAL_OP_GATT_SERVER_CONNECT */
	{ handle_server_connect, false,
		sizeof(struct hal_cmd_gatt_server_connect) },
	/* HAL_OP_GATT_SERVER_DISCONNECT */
	{ handle_server_disconnect, false,
		sizeof(struct hal_cmd_gatt_server_disconnect) },
	/* HAL_OP_GATT_SERVER_ADD_SERVICE */
	{ handle_server_add_service, false,
		sizeof(struct hal_cmd_gatt_server_add_service) },
	/* HAL_OP_GATT_SERVER_ADD_INC_SERVICE */
	{ handle_server_add_included_service, false,
		sizeof(struct hal_cmd_gatt_server_add_inc_service) },
	/* HAL_OP_GATT_SERVER_ADD_CHARACTERISTIC */
	{ handle_server_add_characteristic, false,
		sizeof(struct hal_cmd_gatt_server_add_characteristic) },
	/* HAL_OP_GATT_SERVER_ADD_DESCRIPTOR */
	{ handle_server_add_descriptor, false,
		sizeof(struct hal_cmd_gatt_server_add_descriptor) },
	/* HAL_OP_GATT_SERVER_START_SERVICE */
	{ handle_server_start_service, false,
		sizeof(struct hal_cmd_gatt_server_start_service) },
	/* HAL_OP_GATT_SERVER_STOP_SERVICE */
	{ handle_server_stop_service, false,
		sizeof(struct hal_cmd_gatt_server_stop_service) },
	/* HAL_OP_GATT_SERVER_DELETE_SERVICE */
	{ handle_server_delete_service, false,
		sizeof(struct hal_cmd_gatt_server_delete_service) },
	/* HAL_OP_GATT_SERVER_SEND_INDICATION */
	{ handle_server_send_indication, true,
		sizeof(struct hal_cmd_gatt_server_send_indication) },
	/* HAL_OP_GATT_SERVER_SEND_RESPONSE */
	{ handle_server_send_response, true,
		sizeof(struct hal_cmd_gatt_server_send_response) },
	/* HAL_OP_GATT_CLIENT_SCAN_FILTER_SETUP */
	{ handle_client_scan_filter_setup, false,
		sizeof(struct hal_cmd_gatt_client_scan_filter_setup) },
	/* HAL_OP_GATT_CLIENT_SCAN_FILTER_ADD_REMOVE */
	{ handle_client_scan_filter_add_remove, true,
		sizeof(struct hal_cmd_gatt_client_scan_filter_add_remove) },
	/* HAL_OP_GATT_CLIENT_SCAN_FILTER_CLEAR */
	{ handle_client_scan_filter_clear, false,
		sizeof(struct hal_cmd_gatt_client_scan_filter_clear) },
	/* HAL_OP_GATT_CLIENT_SCAN_FILTER_ENABLE */
	{ handle_client_scan_filter_enable, false,
		sizeof(struct hal_cmd_gatt_client_scan_filter_enable) },
	/* HAL_OP_GATT_CLIENT_CONFIGURE_MTU */
	{ handle_client_configure_mtu, false,
		sizeof(struct hal_cmd_gatt_client_configure_mtu) },
	/* HAL_OP_GATT_CLIENT_CONN_PARAM_UPDATE */
	{ handle_client_conn_param_update, false,
		sizeof(struct hal_cmd_gatt_client_conn_param_update) },
	/* HAL_OP_GATT_CLIENT_SET_SCAN_PARAM */
	{ handle_client_set_scan_param, false,
		sizeof(struct hal_cmd_gatt_client_set_scan_param) },
	/* HAL_OP_GATT_CLIENT_SETUP_MULTI_ADV */
	{ handle_client_setup_multi_adv, false,
		sizeof(struct hal_cmd_gatt_client_setup_multi_adv) },
	/* HAL_OP_GATT_CLIENT_UPDATE_MULTI_ADV */
	{ handle_client_update_multi_adv, false,
		sizeof(struct hal_cmd_gatt_client_update_multi_adv) },
	/* HAL_OP_GATT_CLIENT_SETUP_MULTI_ADV_INST */
	{ handle_client_setup_multi_adv_inst, false,
		sizeof(struct hal_cmd_gatt_client_setup_multi_adv_inst) },
	/* HAL_OP_GATT_CLIENT_DISABLE_MULTI_ADV_INST */
	{ handle_client_disable_multi_adv_inst, false,
		sizeof(struct hal_cmd_gatt_client_disable_multi_adv_inst) },
	/* HAL_OP_GATT_CLIENT_CONFIGURE_BATCHSCAN */
	{ handle_client_configure_batchscan, false,
		sizeof(struct hal_cmd_gatt_client_configure_batchscan) },
	/* HAL_OP_GATT_CLIENT_ENABLE_BATCHSCAN */
	{ handle_client_enable_batchscan, false,
		sizeof(struct hal_cmd_gatt_client_enable_batchscan) },
	/* HAL_OP_GATT_CLIENT_DISABLE_BATCHSCAN */
	{ handle_client_disable_batchscan, false,
		sizeof(struct hal_cmd_gatt_client_disable_batchscan) },
	/* HAL_OP_GATT_CLIENT_READ_BATCHSCAN_REPORTS */
	{ handle_client_read_batchscan_reports, false,
		sizeof(struct hal_cmd_gatt_client_read_batchscan_reports) },
};

static uint8_t read_by_type(const uint8_t *cmd, uint16_t cmd_len,
						struct gatt_device *device)
{
	uint16_t start, end;
	uint16_t len = 0;
	bt_uuid_t uuid;
	struct queue *q;

	DBG("");

	switch (cmd[0]) {
	case ATT_OP_READ_BY_TYPE_REQ:
		len = dec_read_by_type_req(cmd, cmd_len, &start, &end, &uuid);
		break;
	case ATT_OP_READ_BY_GROUP_REQ:
		len = dec_read_by_grp_req(cmd, cmd_len, &start, &end, &uuid);
		break;
	default:
		break;
	}

	if (!len)
		return ATT_ECODE_INVALID_PDU;

	if (start > end || start == 0)
		return ATT_ECODE_INVALID_HANDLE;

	q = queue_new();

	switch (cmd[0]) {
	case ATT_OP_READ_BY_TYPE_REQ:
		gatt_db_read_by_type(gatt_db, start, end, uuid, q);
		break;
	case ATT_OP_READ_BY_GROUP_REQ:
		gatt_db_read_by_group_type(gatt_db, start, end, uuid, q);
		break;
	default:
		break;
	}

	if (queue_isempty(q)) {
		queue_destroy(q, NULL);
		return ATT_ECODE_ATTR_NOT_FOUND;
	}

	while (queue_peek_head(q)) {
		struct pending_request *data;
		struct gatt_db_attribute *attrib = queue_pop_head(q);

		data = new0(struct pending_request, 1);
		data->attrib = attrib;
		queue_push_tail(device->pending_requests, data);
	}

	queue_destroy(q, NULL);
	process_dev_pending_requests(device, cmd[0]);

	return 0;
}

static uint8_t read_request(const uint8_t *cmd, uint16_t cmd_len,
							struct gatt_device *dev)
{
	struct gatt_db_attribute *attrib;
	uint16_t handle;
	uint16_t len;
	uint16_t offset;
	struct pending_request *data;

	DBG("");

	switch (cmd[0]) {
	case ATT_OP_READ_BLOB_REQ:
		len = dec_read_blob_req(cmd, cmd_len, &handle, &offset);
		if (!len)
			return ATT_ECODE_INVALID_PDU;
		break;
	case ATT_OP_READ_REQ:
		len = dec_read_req(cmd, cmd_len, &handle);
		if (!len)
			return ATT_ECODE_INVALID_PDU;
		offset = 0;
		break;
	default:
		error("gatt: Unexpected read type 0x%02x", cmd[0]);
		return ATT_ECODE_REQ_NOT_SUPP;
	}

	attrib = gatt_db_get_attribute(gatt_db, handle);
	if (attrib == 0)
		return ATT_ECODE_INVALID_HANDLE;

	data = new0(struct pending_request, 1);
	data->offset = offset;
	data->attrib = attrib;
	queue_push_tail(dev->pending_requests, data);

	process_dev_pending_requests(dev, cmd[0]);

	return 0;
}

static uint8_t mtu_att_handle(const uint8_t *cmd, uint16_t cmd_len,
							struct gatt_device *dev)
{
	uint16_t rmtu, mtu, len;
	size_t length;
	uint8_t *rsp;

	DBG("");

	len = dec_mtu_req(cmd, cmd_len, &rmtu);
	if (!len)
		return ATT_ECODE_INVALID_PDU;

	/* MTU exchange shall not be used on BR/EDR - Vol 3. Part G. 4.3.1 */
	if (get_cid(dev) != ATT_CID)
		return ATT_ECODE_UNLIKELY;

	if (!get_local_mtu(dev, &mtu))
		return ATT_ECODE_UNLIKELY;

	if (!update_mtu(dev, rmtu))
		return ATT_ECODE_UNLIKELY;

	rsp = g_attrib_get_buffer(dev->attrib, &length);

	/* Respond with our MTU */
	len = enc_mtu_resp(mtu, rsp, length);
	if (!g_attrib_send(dev->attrib, 0, rsp, len, NULL, NULL, NULL))
		return ATT_ECODE_UNLIKELY;

	return 0;
}

static uint8_t find_info_handle(const uint8_t *cmd, uint16_t cmd_len,
				uint8_t *rsp, size_t rsp_size, uint16_t *length)
{
	struct gatt_db_attribute *attrib;
	struct queue *q, *temp;
	struct att_data_list *adl;
	int iterator = 0;
	uint16_t start, end;
	uint16_t len, queue_len;
	uint8_t format;
	uint8_t ret = 0;

	DBG("");

	len = dec_find_info_req(cmd, cmd_len, &start, &end);
	if (!len)
		return ATT_ECODE_INVALID_PDU;

	if (start > end || start == 0)
		return ATT_ECODE_INVALID_HANDLE;

	q = queue_new();

	gatt_db_find_information(gatt_db, start, end, q);

	if (queue_isempty(q)) {
		queue_destroy(q, NULL);
		return ATT_ECODE_ATTR_NOT_FOUND;
	}

	temp = queue_new();

	attrib = queue_peek_head(q);
	/* UUIDS can be only 128 bit and 16 bit */
	len = bt_uuid_len(gatt_db_attribute_get_type(attrib));
	if (len != 2 && len != 16) {
		queue_destroy(q, NULL);
		queue_destroy(temp, NULL);
		return ATT_ECODE_UNLIKELY;
	}

	while (attrib) {
		const bt_uuid_t *type;

		type = gatt_db_attribute_get_type(attrib);
		if (bt_uuid_len(type) != len)
			break;

		queue_push_tail(temp, queue_pop_head(q));
		attrib = queue_peek_head(q);
	}

	queue_destroy(q, NULL);

	queue_len = queue_length(temp);
	adl = att_data_list_alloc(queue_len, len + sizeof(uint16_t));
	if (!adl) {
		queue_destroy(temp, NULL);
		return ATT_ECODE_INSUFF_RESOURCES;
	}

	while (queue_peek_head(temp)) {
		uint8_t *value;
		const bt_uuid_t *type;
		struct gatt_db_attribute *attrib = queue_pop_head(temp);
		uint16_t handle;

		type = gatt_db_attribute_get_type(attrib);
		if (!type)
			break;

		value = adl->data[iterator++];

		handle = gatt_db_attribute_get_handle(attrib);
		put_le16(handle, value);
		memcpy(&value[2], &type->value, len);
	}

	if (len == 2)
		format = ATT_FIND_INFO_RESP_FMT_16BIT;
	else
		format = ATT_FIND_INFO_RESP_FMT_128BIT;

	len = enc_find_info_resp(format, adl, rsp, rsp_size);
	if (!len)
		ret = ATT_ECODE_UNLIKELY;

	*length = len;
	att_data_list_free(adl);
	queue_destroy(temp, NULL);

	return ret;
}

struct find_by_type_request_data {
	struct gatt_device *device;
	uint8_t *search_value;
	size_t search_vlen;
	uint8_t error;
};

static void find_by_type_request_cb(struct gatt_db_attribute *attrib,
								void *user_data)
{
	struct find_by_type_request_data *find_data = user_data;
	struct pending_request *request_data;

	if (find_data->error)
		return;

	request_data = new0(struct pending_request, 1);
	request_data->filter_value = malloc0(find_data->search_vlen);
	if (!request_data->filter_value) {
		destroy_pending_request(request_data);
		find_data->error = ATT_ECODE_INSUFF_RESOURCES;
		return;
	}

	request_data->attrib = attrib;
	request_data->filter_vlen = find_data->search_vlen;
	memcpy(request_data->filter_value, find_data->search_value,
							find_data->search_vlen);

	queue_push_tail(find_data->device->pending_requests, request_data);
}

static uint8_t find_by_type_request(const uint8_t *cmd, uint16_t cmd_len,
						struct gatt_device *device)
{
	uint8_t search_value[cmd_len];
	size_t search_vlen;
	uint16_t start, end;
	bt_uuid_t uuid;
	uint16_t len;
	struct find_by_type_request_data data;

	DBG("");

	len = dec_find_by_type_req(cmd, cmd_len, &start, &end, &uuid,
						search_value, &search_vlen);
	if (!len)
		return ATT_ECODE_INVALID_PDU;

	if (start > end || start == 0)
		return ATT_ECODE_INVALID_HANDLE;

	data.error = 0;
	data.search_vlen = search_vlen;
	data.search_value = search_value;
	data.device = device;

	if (gatt_db_find_by_type(gatt_db, start, end, &uuid,
					find_by_type_request_cb, &data) == 0) {
		size_t mtu;
		uint8_t *rsp = g_attrib_get_buffer(device->attrib, &mtu);

		len = enc_error_resp(ATT_OP_FIND_BY_TYPE_REQ, start,
					ATT_ECODE_ATTR_NOT_FOUND, rsp, mtu);
		g_attrib_send(device->attrib, 0, rsp, len, NULL, NULL, NULL);
		return 0;
	}

	if (!data.error)
		process_dev_pending_requests(device, ATT_OP_FIND_BY_TYPE_REQ);

	return data.error;
}

static void write_confirm(struct gatt_db_attribute *attrib,
						int err, void *user_data)
{
	if (!err)
		return;

	error("Error writting attribute %p", attrib);
}

static void write_cmd_request(const uint8_t *cmd, uint16_t cmd_len,
						struct gatt_device *dev)
{
	uint8_t value[cmd_len];
	struct gatt_db_attribute *attrib;
	uint32_t permissions;
	uint16_t handle;
	uint16_t len;
	size_t vlen;

	len = dec_write_cmd(cmd, cmd_len, &handle, value, &vlen);
	if (!len)
		return;

	if (handle == 0)
		return;

	attrib = gatt_db_get_attribute(gatt_db, handle);
	if (!attrib)
		return;

	permissions = gatt_db_attribute_get_permissions(attrib);

	if (check_device_permissions(dev, cmd[0], permissions))
		return;

	gatt_db_attribute_write(attrib, 0, value, vlen, cmd[0],
						g_attrib_get_att(dev->attrib),
						write_confirm, NULL);
}

static void write_signed_cmd_request(const uint8_t *cmd, uint16_t cmd_len,
						struct gatt_device *dev)
{
	uint8_t value[cmd_len];
	uint8_t s[ATT_SIGNATURE_LEN];
	struct gatt_db_attribute *attrib;
	uint32_t permissions;
	uint16_t handle;
	uint16_t len;
	size_t vlen;
	uint8_t csrk[16];
	uint32_t sign_cnt;

	if (get_cid(dev) != ATT_CID) {
		error("gatt: Remote tries write signed on BR/EDR bearer");
		connection_cleanup(dev);
		return;
	}

	if (get_sec_level(dev) != BT_SECURITY_LOW) {
		error("gatt: Remote tries write signed on encrypted link");
		connection_cleanup(dev);
		return;
	}

	if (!bt_get_csrk(&dev->bdaddr, false, csrk, &sign_cnt, NULL)) {
		error("gatt: No valid csrk from remote device");
		return;
	}

	len = dec_signed_write_cmd(cmd, cmd_len, &handle, value, &vlen, s);

	if (handle == 0)
		return;

	attrib = gatt_db_get_attribute(gatt_db, handle);
	if (!attrib)
		return;

	permissions = gatt_db_attribute_get_permissions(attrib);

	if (check_device_permissions(dev, cmd[0], permissions))
		return;

	if (len) {
		uint8_t t[ATT_SIGNATURE_LEN];
		uint32_t r_sign_cnt = get_le32(s);

		if (r_sign_cnt < sign_cnt) {
			error("gatt: Invalid sign counter (%d<%d)",
							r_sign_cnt, sign_cnt);
			return;
		}

		/* Generate signature and verify it */
		if (!bt_crypto_sign_att(crypto, csrk, cmd,
						cmd_len - ATT_SIGNATURE_LEN,
						r_sign_cnt, t)) {
			error("gatt: Error when generating att signature");
			return;
		}

		if (memcmp(t, s, ATT_SIGNATURE_LEN)) {
			error("gatt: signature does not match");
			return;
		}
		/* Signature OK, proceed with write */
		bt_update_sign_counter(&dev->bdaddr, false, r_sign_cnt);
		gatt_db_attribute_write(attrib, 0, value, vlen, cmd[0],
						g_attrib_get_att(dev->attrib),
						write_confirm, NULL);
	}
}

static void attribute_write_cb(struct gatt_db_attribute *attrib, int err,
								void *user_data)
{
	struct pending_request *data = user_data;
	uint8_t error = err_to_att(err);

	DBG("");

	data->attrib = attrib;
	data->error = error;
	data->completed = true;
}

static uint8_t write_req_request(const uint8_t *cmd, uint16_t cmd_len,
						struct gatt_device *dev)
{
	uint8_t value[cmd_len];
	struct pending_request *data;
	struct gatt_db_attribute *attrib;
	uint32_t permissions;
	uint16_t handle;
	uint16_t len;
	uint8_t error;
	size_t vlen;

	len = dec_write_req(cmd, cmd_len, &handle, value, &vlen);
	if (!len)
		return ATT_ECODE_INVALID_PDU;

	if (handle == 0)
		return ATT_ECODE_INVALID_HANDLE;

	attrib = gatt_db_get_attribute(gatt_db, handle);
	if (!attrib)
		return ATT_ECODE_ATTR_NOT_FOUND;

	permissions = gatt_db_attribute_get_permissions(attrib);

	error = check_device_permissions(dev, cmd[0], permissions);
	if (error)
		return error;

	data = new0(struct pending_request, 1);
	data->attrib = attrib;

	queue_push_tail(dev->pending_requests, data);

	if (!gatt_db_attribute_write(attrib, 0, value, vlen, cmd[0],
						g_attrib_get_att(dev->attrib),
						attribute_write_cb, data)) {
		queue_remove(dev->pending_requests, data);
		free(data);
		return ATT_ECODE_UNLIKELY;
	}

	send_dev_complete_response(dev, cmd[0]);

	return 0;
}

static uint8_t write_prep_request(const uint8_t *cmd, uint16_t cmd_len,
						struct gatt_device *dev)
{
	uint8_t value[cmd_len];
	struct pending_request *data;
	struct gatt_db_attribute *attrib;
	uint32_t permissions;
	uint16_t handle;
	uint16_t offset;
	uint8_t error;
	uint16_t len;
	size_t vlen;

	len = dec_prep_write_req(cmd, cmd_len, &handle, &offset,
						value, &vlen);
	if (!len)
		return ATT_ECODE_INVALID_PDU;

	if (handle == 0)
		return ATT_ECODE_INVALID_HANDLE;

	attrib = gatt_db_get_attribute(gatt_db, handle);
	if (!attrib)
		return ATT_ECODE_ATTR_NOT_FOUND;

	permissions = gatt_db_attribute_get_permissions(attrib);

	error = check_device_permissions(dev, cmd[0], permissions);
	if (error)
		return error;

	data = new0(struct pending_request, 1);
	data->attrib = attrib;
	data->offset = offset;

	queue_push_tail(dev->pending_requests, data);

	data->value = g_memdup(value, vlen);
	data->length = vlen;

	if (!gatt_db_attribute_write(attrib, offset, value, vlen, cmd[0],
						g_attrib_get_att(dev->attrib),
						attribute_write_cb, data)) {
		queue_remove(dev->pending_requests, data);
		g_free(data->value);
		free(data);

		return ATT_ECODE_UNLIKELY;
	}

	send_dev_complete_response(dev, cmd[0]);

	return 0;
}

static void send_server_write_execute_notify(void *data, void *user_data)
{
	struct hal_ev_gatt_server_request_exec_write *ev = user_data;
	struct pending_trans_data *transaction;
	struct app_connection *conn = data;

	if (!conn->wait_execute_write)
		return;

	ev->conn_id = conn->id;

	transaction = conn_add_transact(conn, ATT_OP_EXEC_WRITE_REQ, NULL, 0);

	ev->trans_id = transaction->id;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_GATT,
			HAL_EV_GATT_SERVER_REQUEST_EXEC_WRITE, sizeof(*ev), ev);
}

static uint8_t write_execute_request(const uint8_t *cmd, uint16_t cmd_len,
						struct gatt_device *dev)
{
	struct hal_ev_gatt_server_request_exec_write ev;
	uint8_t value;
	struct pending_request *data;

	/*
	 * Check if there was any write prep before.
	 * TODO: Try to find better error code if possible
	 */
	if (!pending_execute_write())
		return ATT_ECODE_UNLIKELY;

	if (!dec_exec_write_req(cmd, cmd_len, &value))
		return ATT_ECODE_INVALID_PDU;

	memset(&ev, 0, sizeof(ev));
	bdaddr2android(&dev->bdaddr, &ev.bdaddr);
	ev.exec_write = value;

	data = new0(struct pending_request, 1);

	queue_push_tail(dev->pending_requests, data);

	queue_foreach(app_connections, send_server_write_execute_notify, &ev);
	send_dev_complete_response(dev, cmd[0]);

	return 0;
}

static void att_handler(const uint8_t *ipdu, uint16_t len, gpointer user_data)
{
	struct gatt_device *dev = user_data;
	uint8_t status;
	uint16_t resp_length = 0;
	size_t length;
	uint8_t *opdu = g_attrib_get_buffer(dev->attrib, &length);

	DBG("op 0x%02x", ipdu[0]);

	if (len > length) {
		error("gatt: Too much data on ATT socket %p", opdu);
		status = ATT_ECODE_INVALID_PDU;
		goto done;
	}

	switch (ipdu[0]) {
	case ATT_OP_READ_BY_GROUP_REQ:
	case ATT_OP_READ_BY_TYPE_REQ:
		status = read_by_type(ipdu, len, dev);
		break;
	case ATT_OP_READ_REQ:
	case ATT_OP_READ_BLOB_REQ:
		status = read_request(ipdu, len, dev);
		break;
	case ATT_OP_MTU_REQ:
		status = mtu_att_handle(ipdu, len, dev);
		break;
	case ATT_OP_FIND_INFO_REQ:
		status = find_info_handle(ipdu, len, opdu, length,
								&resp_length);
		break;
	case ATT_OP_WRITE_REQ:
		status = write_req_request(ipdu, len, dev);
		break;
	case ATT_OP_WRITE_CMD:
		write_cmd_request(ipdu, len, dev);
		/* No response on write cmd */
		return;
	case ATT_OP_SIGNED_WRITE_CMD:
		write_signed_cmd_request(ipdu, len, dev);
		/* No response on write signed cmd */
		return;
	case ATT_OP_PREP_WRITE_REQ:
		status = write_prep_request(ipdu, len, dev);
		break;
	case ATT_OP_FIND_BY_TYPE_REQ:
		status = find_by_type_request(ipdu, len, dev);
		break;
	case ATT_OP_EXEC_WRITE_REQ:
		status = write_execute_request(ipdu, len, dev);
		break;
	case ATT_OP_READ_MULTI_REQ:
	default:
		DBG("Unsupported request 0x%02x", ipdu[0]);
		status = ATT_ECODE_REQ_NOT_SUPP;
		break;
	}

done:
	if (status)
		resp_length = enc_error_resp(ipdu[0], 0x0000, status, opdu,
									length);

	g_attrib_send(dev->attrib, 0, opdu, resp_length, NULL, NULL, NULL);
}

static void connect_confirm(GIOChannel *io, void *user_data)
{
	struct gatt_device *dev;
	bdaddr_t dst;
	GError *gerr = NULL;

	DBG("");

	bt_io_get(io, &gerr, BT_IO_OPT_DEST_BDADDR, &dst, BT_IO_OPT_INVALID);
	if (gerr) {
		error("gatt: bt_io_get: %s", gerr->message);
		g_error_free(gerr);
		return;
	}

	/* TODO Handle collision */
	dev = find_device_by_addr(&dst);
	if (!dev) {
		dev = create_device(&dst);
	} else {
		if ((dev->state != DEVICE_DISCONNECTED) &&
					!(dev->state == DEVICE_CONNECT_INIT &&
					bt_kernel_conn_control())) {
			char addr[18];

			ba2str(&dst, addr);
			info("gatt: Rejecting incoming connection from %s",
									addr);
			goto drop;
		}
	}

	if (!bt_io_accept(io, connect_cb, device_ref(dev), NULL, NULL)) {
		error("gatt: failed to accept connection");
		device_unref(dev);
		goto drop;
	}

	queue_foreach(listen_apps, create_app_connection, dev);
	device_set_state(dev, DEVICE_CONNECT_READY);

	return;

drop:
	g_io_channel_shutdown(io, TRUE, NULL);
}

struct gap_srvc_handles {
	struct gatt_db_attribute *srvc;

	/* Characteristics */
	struct gatt_db_attribute *dev_name;
	struct gatt_db_attribute *appear;
	struct gatt_db_attribute *priv;
};

static struct gap_srvc_handles gap_srvc_data;

#define APPEARANCE_GENERIC_PHONE 0x0040
#define PERIPHERAL_PRIVACY_DISABLE 0x00

static void device_name_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	const char *name = bt_get_adapter_name();

	gatt_db_attribute_read_result(attrib, id, 0, (void *) name,
								strlen(name));
}

static void register_gap_service(void)
{
	uint16_t start, end;
	bt_uuid_t uuid;

	/* GAP UUID */
	bt_uuid16_create(&uuid, 0x1800);
	gap_srvc_data.srvc = gatt_db_add_service(gatt_db, &uuid, true, 7);

	/* Device name characteristic */
	bt_uuid16_create(&uuid, GATT_CHARAC_DEVICE_NAME);
	gap_srvc_data.dev_name =
			gatt_db_service_add_characteristic(gap_srvc_data.srvc,
							&uuid, GATT_PERM_READ,
							GATT_CHR_PROP_READ,
							device_name_read_cb,
							NULL, NULL);

	/* Appearance */
	bt_uuid16_create(&uuid, GATT_CHARAC_APPEARANCE);

	gap_srvc_data.appear =
			gatt_db_service_add_characteristic(gap_srvc_data.srvc,
							&uuid, GATT_PERM_READ,
							GATT_CHR_PROP_READ,
							NULL, NULL, NULL);
	if (gap_srvc_data.appear) {
		uint16_t value;
		/* Store appearance into db */
		value = cpu_to_le16(APPEARANCE_GENERIC_PHONE);
		gatt_db_attribute_write(gap_srvc_data.appear, 0,
						(void *) &value, sizeof(value),
						ATT_OP_WRITE_REQ, NULL,
						write_confirm, NULL);
	}

	/* Pripheral privacy flag */
	bt_uuid16_create(&uuid, GATT_CHARAC_PERIPHERAL_PRIV_FLAG);
	gap_srvc_data.priv =
			gatt_db_service_add_characteristic(gap_srvc_data.srvc,
							&uuid, GATT_PERM_READ,
							GATT_CHR_PROP_READ,
							NULL, NULL, NULL);
	if (gap_srvc_data.priv) {
		uint8_t value;

		/* Store privacy into db */
		value = PERIPHERAL_PRIVACY_DISABLE;
		gatt_db_attribute_write(gap_srvc_data.priv, 0,
						&value, sizeof(value),
						ATT_OP_WRITE_REQ, NULL,
						write_confirm, NULL);
	}

	gatt_db_service_set_active(gap_srvc_data.srvc , true);

	/* SDP */
	bt_uuid16_create(&uuid, 0x1800);
	gatt_db_attribute_get_service_handles(gap_srvc_data.srvc, &start, &end);
	gap_sdp_handle = add_sdp_record(&uuid, start, end,
						"Generic Access Profile");
	if (!gap_sdp_handle)
		error("gatt: Failed to register GAP SDP record");
}

static void device_info_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	char *buf = user_data;

	gatt_db_attribute_read_result(attrib, id, 0, user_data, strlen(buf));
}

static void device_info_read_system_id_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	uint8_t pdu[8];

	put_le64(bt_config_get_system_id(), pdu);

	gatt_db_attribute_read_result(attrib, id, 0, pdu, sizeof(pdu));
}

static void device_info_read_pnp_id_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	uint8_t pdu[7];

	pdu[0] = bt_config_get_pnp_source();
	put_le16(bt_config_get_pnp_vendor(), &pdu[1]);
	put_le16(bt_config_get_pnp_product(), &pdu[3]);
	put_le16(bt_config_get_pnp_version(), &pdu[5]);

	gatt_db_attribute_read_result(attrib, id, 0, pdu, sizeof(pdu));
}

static void register_device_info_service(void)
{
	bt_uuid_t uuid;
	struct gatt_db_attribute *service;
	uint16_t start_handle, end_handle;
	const char *data;
	uint32_t enc_perm = GATT_PERM_READ | GATT_PERM_READ_ENCRYPTED;

	DBG("");

	/* Device Information Service */
	bt_uuid16_create(&uuid, 0x180a);
	service = gatt_db_add_service(gatt_db, &uuid, true, 17);

	/* User data are not const hence (void *) cast is used */
	data = bt_config_get_name();
	if (data) {
		bt_uuid16_create(&uuid, GATT_CHARAC_MODEL_NUMBER_STRING);
		gatt_db_service_add_characteristic(service, &uuid,
						GATT_PERM_READ,
						GATT_CHR_PROP_READ,
						device_info_read_cb, NULL,
						(void *) data);
	}

	data = bt_config_get_serial();
	if (data) {
		bt_uuid16_create(&uuid, GATT_CHARAC_SERIAL_NUMBER_STRING);
		gatt_db_service_add_characteristic(service, &uuid,
						enc_perm, GATT_CHR_PROP_READ,
						device_info_read_cb, NULL,
						(void *) data);
	}

	if (bt_config_get_system_id()) {
		bt_uuid16_create(&uuid, GATT_CHARAC_SYSTEM_ID);
		gatt_db_service_add_characteristic(service, &uuid,
						enc_perm, GATT_CHR_PROP_READ,
						device_info_read_system_id_cb,
						NULL, NULL);
	}

	data = bt_config_get_fw_rev();
	if (data) {
		bt_uuid16_create(&uuid, GATT_CHARAC_FIRMWARE_REVISION_STRING);
		gatt_db_service_add_characteristic(service, &uuid,
						GATT_PERM_READ,
						GATT_CHR_PROP_READ,
						device_info_read_cb, NULL,
						(void *) data);
	}

	data = bt_config_get_hw_rev();
	if (data) {
		bt_uuid16_create(&uuid, GATT_CHARAC_HARDWARE_REVISION_STRING);
		gatt_db_service_add_characteristic(service, &uuid,
						GATT_PERM_READ,
						GATT_CHR_PROP_READ,
						device_info_read_cb, NULL,
						(void *) data);
	}

	bt_uuid16_create(&uuid, GATT_CHARAC_SOFTWARE_REVISION_STRING);
	gatt_db_service_add_characteristic(service, &uuid, GATT_PERM_READ,
					GATT_CHR_PROP_READ, device_info_read_cb,
					NULL, VERSION);

	data = bt_config_get_vendor();
	if (data) {
		bt_uuid16_create(&uuid, GATT_CHARAC_MANUFACTURER_NAME_STRING);
		gatt_db_service_add_characteristic(service, &uuid,
						GATT_PERM_READ,
						GATT_CHR_PROP_READ,
						device_info_read_cb, NULL,
						(void *) data);
	}

	if (bt_config_get_pnp_source()) {
		bt_uuid16_create(&uuid, GATT_CHARAC_PNP_ID);
		gatt_db_service_add_characteristic(service, &uuid,
						GATT_PERM_READ,
						GATT_CHR_PROP_READ,
						device_info_read_pnp_id_cb,
						NULL, NULL);
	}

	gatt_db_service_set_active(service, true);

	/* SDP */
	bt_uuid16_create(&uuid, 0x180a);
	gatt_db_attribute_get_service_handles(service, &start_handle,
								&end_handle);
	dis_sdp_handle = add_sdp_record(&uuid, start_handle, end_handle,
						"Device Information Service");
	if (!dis_sdp_handle)
		error("gatt: Failed to register DIS SDP record");
}

static void gatt_srvc_change_write_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					const uint8_t *value, size_t len,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	struct gatt_device *dev;
	bdaddr_t bdaddr;

	if (!get_dst_addr(att, &bdaddr)) {
		error("gatt: srvc_change_write_cb, could not obtain BDADDR");
		return;
	}

	dev = find_device_by_addr(&bdaddr);
	if (!dev) {
		error("gatt: Could not find device ?!");
		return;
	}

	if (!bt_device_is_bonded(&bdaddr)) {
		gatt_db_attribute_write_result(attrib, id,
						ATT_ECODE_AUTHORIZATION);
		return;
	}

	/* 2 octets are expected as CCC value */
	if (len != 2) {
		gatt_db_attribute_write_result(attrib, id,
						ATT_ECODE_INVAL_ATTR_VALUE_LEN);
		return;
	}

	/* Set services changed indication value */
	bt_store_gatt_ccc(&bdaddr, get_le16(value));

	gatt_db_attribute_write_result(attrib, id, 0);
}

static void gatt_srvc_change_read_cb(struct gatt_db_attribute *attrib,
					unsigned int id, uint16_t offset,
					uint8_t opcode, struct bt_att *att,
					void *user_data)
{
	struct gatt_device *dev;
	uint8_t pdu[2];
	bdaddr_t bdaddr;

	if (!get_dst_addr(att, &bdaddr)) {
		error("gatt: srvc_change_read_cb, could not obtain BDADDR");
		return;
	}

	dev = find_device_by_addr(&bdaddr);
	if (!dev) {
		error("gatt: Could not find device ?!");
		return;
	}

	put_le16(bt_get_gatt_ccc(&dev->bdaddr), pdu);

	gatt_db_attribute_read_result(attrib, id, 0, pdu, sizeof(pdu));
}

static void register_gatt_service(void)
{
	struct gatt_db_attribute *service;
	uint16_t start_handle, end_handle;
	bt_uuid_t uuid;

	DBG("");

	bt_uuid16_create(&uuid, 0x1801);
	service = gatt_db_add_service(gatt_db, &uuid, true, 4);

	bt_uuid16_create(&uuid, GATT_CHARAC_SERVICE_CHANGED);
	service_changed_attrib = gatt_db_service_add_characteristic(service,
							&uuid, GATT_PERM_NONE,
							GATT_CHR_PROP_INDICATE,
							NULL, NULL, NULL);

	bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);
	gatt_db_service_add_descriptor(service, &uuid,
					GATT_PERM_READ | GATT_PERM_WRITE,
					gatt_srvc_change_read_cb,
					gatt_srvc_change_write_cb, NULL);

	gatt_db_service_set_active(service, true);

	/* SDP */
	bt_uuid16_create(&uuid, 0x1801);
	gatt_db_attribute_get_service_handles(service, &start_handle,
								&end_handle);
	gatt_sdp_handle = add_sdp_record(&uuid, start_handle, end_handle,
						"Generic Attribute Profile");

	if (!gatt_sdp_handle)
		error("gatt: Failed to register GATT SDP record");
}

static bool start_listening(void)
{
	/* BR/EDR socket */
	bredr_io = bt_io_listen(NULL, connect_confirm, NULL, NULL, NULL,
					BT_IO_OPT_SOURCE_TYPE, BDADDR_BREDR,
					BT_IO_OPT_PSM, ATT_PSM,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
					BT_IO_OPT_INVALID);

	/* LE socket */
	le_io = bt_io_listen(NULL, connect_confirm, NULL, NULL, NULL,
					BT_IO_OPT_SOURCE_TYPE, BDADDR_LE_PUBLIC,
					BT_IO_OPT_CID, ATT_CID,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
					BT_IO_OPT_INVALID);

	if (!le_io && !bredr_io) {
		error("gatt: Failed to start listening IO");
		return false;
	}

	return true;
}

static void gatt_paired_cb(const bdaddr_t *addr)
{
	struct gatt_device *dev;
	char address[18];
	struct app_connection *conn;

	dev = find_device_by_addr(addr);
	if (!dev)
		return;

	ba2str(addr, address);
	DBG("Paired device %s", address);

	/* conn without app is internal one used for search primary services */
	conn = find_conn_without_app(dev);
	if (!conn)
		return;

	if (conn->timeout_id > 0) {
		g_source_remove(conn->timeout_id);
		conn->timeout_id = 0;
	}

	search_dev_for_srvc(conn, NULL);
}

static void gatt_unpaired_cb(const bdaddr_t *addr)
{
	struct gatt_device *dev;
	char address[18];

	dev = find_device_by_addr(addr);
	if (!dev)
		return;

	ba2str(addr, address);
	DBG("Unpaired device %s", address);

	queue_remove(gatt_devices, dev);
	destroy_device(dev);
}

bool bt_gatt_register(struct ipc *ipc, const bdaddr_t *addr)
{
	DBG("");

	if (!bt_paired_register(gatt_paired_cb)) {
		error("gatt: Could not register paired callback");
		return false;
	}

	if (!bt_unpaired_register(gatt_unpaired_cb)) {
		error("gatt: Could not register unpaired callback");
		return false;
	}

	if (!start_listening())
		return false;

	crypto = bt_crypto_new();
	if (!crypto) {
		error("gatt: Failed to setup crypto");
		goto failed;
	}

	gatt_devices = queue_new();
	gatt_apps = queue_new();
	app_connections = queue_new();
	listen_apps = queue_new();
	services_sdp = queue_new();
	gatt_db = gatt_db_new();

	if (!gatt_db) {
		error("gatt: Failed to allocate memory for database");
		goto failed;
	}

	if (!bt_le_register(le_device_found_handler)) {
		error("gatt: bt_le_register failed");
		goto failed;
	}

	bacpy(&adapter_addr, addr);

	hal_ipc = ipc;

	ipc_register(hal_ipc, HAL_SERVICE_ID_GATT, cmd_handlers,
						G_N_ELEMENTS(cmd_handlers));

	register_gap_service();
	register_device_info_service();
	register_gatt_service();

	info("gatt: LE: %s BR/EDR: %s", le_io ? "enabled" : "disabled",
					bredr_io ? "enabled" : "disabled");

	return true;

failed:

	bt_paired_unregister(gatt_paired_cb);
	bt_unpaired_unregister(gatt_unpaired_cb);

	queue_destroy(gatt_apps, NULL);
	gatt_apps = NULL;

	queue_destroy(gatt_devices, NULL);
	gatt_devices = NULL;

	queue_destroy(app_connections, NULL);
	app_connections = NULL;

	queue_destroy(listen_apps, NULL);
	listen_apps = NULL;

	queue_destroy(services_sdp, NULL);
	services_sdp = NULL;

	gatt_db_unref(gatt_db);
	gatt_db = NULL;

	bt_crypto_unref(crypto);
	crypto = NULL;

	if (le_io) {
		g_io_channel_unref(le_io);
		le_io = NULL;
	}

	if (bredr_io) {
		g_io_channel_unref(bredr_io);
		bredr_io = NULL;
	}

	return false;
}

void bt_gatt_unregister(void)
{
	DBG("");

	ipc_unregister(hal_ipc, HAL_SERVICE_ID_GATT);
	hal_ipc = NULL;

	queue_destroy(app_connections, destroy_connection);
	app_connections = NULL;

	queue_destroy(gatt_apps, destroy_gatt_app);
	gatt_apps = NULL;

	queue_destroy(gatt_devices, destroy_device);
	gatt_devices = NULL;

	queue_destroy(services_sdp, free_service_sdp_record);
	services_sdp = NULL;

	queue_destroy(listen_apps, NULL);
	listen_apps = NULL;

	gatt_db_unref(gatt_db);
	gatt_db = NULL;

	if (le_io) {
		g_io_channel_unref(le_io);
		le_io = NULL;
	}

	if (bredr_io) {
		g_io_channel_unref(bredr_io);
		bredr_io = NULL;
	}

	if (gap_sdp_handle) {
		bt_adapter_remove_record(gap_sdp_handle);
		gap_sdp_handle = 0;
	}

	if (gatt_sdp_handle) {
		bt_adapter_remove_record(gatt_sdp_handle);
		gatt_sdp_handle = 0;
	}

	if (dis_sdp_handle) {
		bt_adapter_remove_record(dis_sdp_handle);
		dis_sdp_handle = 0;
	}

	bt_crypto_unref(crypto);
	crypto = NULL;

	bt_le_unregister();
	bt_unpaired_unregister(gatt_unpaired_cb);
}

unsigned int bt_gatt_register_app(const char *uuid, gatt_type_t type,
							gatt_conn_cb_t func)
{
	struct gatt_app *app;
	bt_uuid_t u, u128;

	bt_string_to_uuid(&u, uuid);
	bt_uuid_to_uuid128(&u, &u128);
	app = register_app((void *) &u128.value.u128, type);
	if (!app)
		return 0;

	app->func = func;

	return app->id;
}

bool bt_gatt_unregister_app(unsigned int id)
{
	uint8_t status;

	status = unregister_app(id);

	return status != HAL_STATUS_FAILED;
}

bool bt_gatt_connect_app(unsigned int id, const bdaddr_t *addr)
{
	uint8_t status;

	status = handle_connect(id, addr, false);

	return status != HAL_STATUS_FAILED;
}

bool bt_gatt_disconnect_app(unsigned int id, const bdaddr_t *addr)
{
	struct app_connection match;
	struct app_connection *conn;
	struct gatt_device *device;
	struct gatt_app *app;

	app = find_app_by_id(id);
	if (!app)
		return false;

	device = find_device_by_addr(addr);
	if (!device)
		return false;

	match.device = device;
	match.app = app;

	conn = queue_remove_if(app_connections,
				match_connection_by_device_and_app, &match);
	if (!conn)
		return false;

	destroy_connection(conn);

	return true;
}

bool bt_gatt_add_autoconnect(unsigned int id, const bdaddr_t *addr)
{
	struct gatt_device *dev;
	struct gatt_app *app;

	DBG("");

	app = find_app_by_id(id);
	if (!app) {
		error("gatt: App ID=%d not found", id);
		return false;
	}

	dev = find_device_by_addr(addr);
	if (!dev) {
		error("gatt: Device not found");
		return false;
	}

	/* Take reference of device for auto connect purpose */
	if (queue_isempty(dev->autoconnect_apps))
		device_ref(dev);

	if (!queue_find(dev->autoconnect_apps, NULL, INT_TO_PTR(id)))
		return queue_push_head(dev->autoconnect_apps, INT_TO_PTR(id));

	return true;
}

void bt_gatt_remove_autoconnect(unsigned int id, const bdaddr_t *addr)
{
	struct gatt_device *dev;

	DBG("");

	dev = find_device_by_addr(addr);
	if (!dev) {
		error("gatt: Device not found");
		return;
	}

	queue_remove(dev->autoconnect_apps, INT_TO_PTR(id));

	if (queue_isempty(dev->autoconnect_apps))
		remove_autoconnect_device(dev);
}
