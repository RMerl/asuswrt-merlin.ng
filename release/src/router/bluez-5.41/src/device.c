/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
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
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>

#include <glib.h>
#include <dbus/dbus.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "log.h"
#include "src/shared/util.h"
#include "src/shared/att.h"
#include "src/shared/queue.h"
#include "src/shared/gatt-db.h"
#include "src/shared/gatt-client.h"
#include "src/shared/gatt-server.h"
#include "src/shared/ad.h"
#include "btio/btio.h"
#include "lib/mgmt.h"
#include "attrib/att.h"
#include "hcid.h"
#include "adapter.h"
#include "gatt-database.h"
#include "attrib/gattrib.h"
#include "attio.h"
#include "device.h"
#include "gatt-client.h"
#include "profile.h"
#include "service.h"
#include "dbus-common.h"
#include "error.h"
#include "uuid-helper.h"
#include "sdp-client.h"
#include "attrib/gatt.h"
#include "agent.h"
#include "textfile.h"
#include "storage.h"
#include "attrib-server.h"
#include "eir.h"

#define IO_CAPABILITY_NOINPUTNOOUTPUT	0x03

#define DISCONNECT_TIMER	2
#define DISCOVERY_TIMER		1

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define RSSI_THRESHOLD		8

#define GATT_PRIM_SVC_UUID_STR "2800"
#define GATT_SND_SVC_UUID_STR  "2801"
#define GATT_INCLUDE_UUID_STR "2802"
#define GATT_CHARAC_UUID_STR "2803"

static DBusConnection *dbus_conn = NULL;
static unsigned service_state_cb_id;

struct btd_disconnect_data {
	guint id;
	disconnect_watch watch;
	void *user_data;
	GDestroyNotify destroy;
};

struct bonding_req {
	DBusMessage *msg;
	guint listener_id;
	struct btd_device *device;
	uint8_t bdaddr_type;
	struct agent *agent;
	struct btd_adapter_pin_cb_iter *cb_iter;
	uint8_t status;
	guint retry_timer;
	struct timespec attempt_start_time;
	long last_attempt_duration_ms;
};

typedef enum {
	AUTH_TYPE_PINCODE,
	AUTH_TYPE_PASSKEY,
	AUTH_TYPE_CONFIRM,
	AUTH_TYPE_NOTIFY_PASSKEY,
	AUTH_TYPE_NOTIFY_PINCODE,
} auth_type_t;

struct authentication_req {
	auth_type_t type;
	struct agent *agent;
	struct btd_device *device;
	uint32_t passkey;
	char *pincode;
	gboolean secure;
};

struct browse_req {
	DBusMessage *msg;
	struct btd_device *device;
	GSList *match_uuids;
	GSList *profiles_added;
	sdp_list_t *records;
	int search_uuid;
	int reconnect_attempt;
	guint listener_id;
	uint16_t sdp_flags;
};

struct included_search {
	struct browse_req *req;
	GSList *services;
	GSList *current;
};

struct attio_data {
	guint id;
	attio_connect_cb cfunc;
	attio_disconnect_cb dcfunc;
	gpointer user_data;
};

struct svc_callback {
	unsigned int id;
	guint idle_id;
	struct btd_device *dev;
	device_svc_cb_t func;
	void *user_data;
};

typedef void (*attio_error_cb) (const GError *gerr, gpointer user_data);
typedef void (*attio_success_cb) (gpointer user_data);

struct att_callbacks {
	attio_error_cb err;		/* Callback for error */
	attio_success_cb success;	/* Callback for success */
	gpointer user_data;
};

/* Per-bearer (LE or BR/EDR) device state */
struct bearer_state {
	bool paired;
	bool bonded;
	bool connected;
	bool svc_resolved;
};

struct csrk_info {
	uint8_t key[16];
	uint32_t counter;
};

struct btd_device {
	int ref_count;

	bdaddr_t	bdaddr;
	uint8_t		bdaddr_type;
	char		*path;
	bool		bredr;
	bool		le;
	bool		pending_paired;		/* "Paired" waiting for SDP */
	bool		svc_refreshed;
	GSList		*svc_callbacks;
	GSList		*eir_uuids;
	struct bt_ad	*ad;
	char		name[MAX_NAME_LENGTH + 1];
	char		*alias;
	uint32_t	class;
	uint16_t	vendor_src;
	uint16_t	vendor;
	uint16_t	product;
	uint16_t	version;
	uint16_t	appearance;
	char		*modalias;
	struct btd_adapter	*adapter;
	GSList		*uuids;
	GSList		*primaries;		/* List of primary services */
	GSList		*services;		/* List of btd_service */
	GSList		*pending;		/* Pending services */
	GSList		*watches;		/* List of disconnect_data */
	bool		temporary;
	guint		disconn_timer;
	guint		discov_timer;
	struct browse_req *browse;		/* service discover request */
	struct bonding_req *bonding;
	struct authentication_req *authr;	/* authentication request */
	GSList		*disconnects;		/* disconnects message */
	DBusMessage	*connect;		/* connect message */
	DBusMessage	*disconnect;		/* disconnect message */
	GAttrib		*attrib;
	GSList		*attios;
	GSList		*attios_offline;

	struct bt_att *att;			/* The new ATT transport */
	uint16_t att_mtu;			/* The ATT MTU */
	unsigned int att_disconn_id;

	/*
	 * TODO: For now, device creates and owns the client-role gatt_db, but
	 * this needs to be persisted in a more central place so that proper
	 * attribute cache support can be built.
	 */
	struct gatt_db *db;			/* GATT db cache */
	struct bt_gatt_client *client;		/* GATT client instance */
	struct bt_gatt_server *server;		/* GATT server instance */

	struct btd_gatt_client *client_dbus;

	struct bearer_state bredr_state;
	struct bearer_state le_state;

	struct csrk_info *local_csrk;
	struct csrk_info *remote_csrk;

	sdp_list_t	*tmp_records;

	time_t		bredr_seen;
	time_t		le_seen;

	gboolean	trusted;
	gboolean	blocked;
	gboolean	auto_connect;
	gboolean	disable_auto_connect;
	gboolean	general_connect;

	bool		legacy;
	int8_t		rssi;
	int8_t		tx_power;

	GIOChannel	*att_io;
	guint		store_id;
};

static const uint16_t uuid_list[] = {
	L2CAP_UUID,
	PNP_INFO_SVCLASS_ID,
	PUBLIC_BROWSE_GROUP,
	0
};

static int device_browse_gatt(struct btd_device *device, DBusMessage *msg);
static int device_browse_sdp(struct btd_device *device, DBusMessage *msg);

static struct bearer_state *get_state(struct btd_device *dev,
							uint8_t bdaddr_type)
{
	if (bdaddr_type == BDADDR_BREDR)
		return &dev->bredr_state;
	else
		return &dev->le_state;
}

static GSList *find_service_with_profile(GSList *list, struct btd_profile *p)
{
	GSList *l;

	for (l = list; l != NULL; l = g_slist_next(l)) {
		struct btd_service *service = l->data;

		if (btd_service_get_profile(service) == p)
			return l;
	}

	return NULL;
}

static GSList *find_service_with_state(GSList *list,
						btd_service_state_t state)
{
	GSList *l;

	for (l = list; l != NULL; l = g_slist_next(l)) {
		struct btd_service *service = l->data;

		if (btd_service_get_state(service) == state)
			return l;
	}

	return NULL;
}

static GSList *find_service_with_uuid(GSList *list, char *uuid)
{
	GSList *l;

	for (l = list; l != NULL; l = g_slist_next(l)) {
		struct btd_service *service = l->data;
		struct btd_profile *profile = btd_service_get_profile(service);

		if (bt_uuid_strcmp(profile->remote_uuid, uuid) == 0)
			return l;
	}

	return NULL;
}

static void update_technologies(GKeyFile *file, struct btd_device *dev)
{
	const char *list[2];
	size_t len = 0;

	if (dev->bredr)
		list[len++] = "BR/EDR";

	if (dev->le) {
		const char *type;

		if (dev->bdaddr_type == BDADDR_LE_PUBLIC)
			type = "public";
		else
			type = "static";

		g_key_file_set_string(file, "General", "AddressType", type);

		list[len++] = "LE";
	}

	g_key_file_set_string_list(file, "General", "SupportedTechnologies",
								list, len);
}

static void store_csrk(struct csrk_info *csrk, GKeyFile *key_file,
							const char *group)
{
	char key[33];
	int i;

	for (i = 0; i < 16; i++)
		sprintf(key + (i * 2), "%2.2X", csrk->key[i]);

	g_key_file_set_string(key_file, group, "Key", key);
	g_key_file_set_integer(key_file, group, "Counter", csrk->counter);
}

static gboolean store_device_info_cb(gpointer user_data)
{
	struct btd_device *device = user_data;
	GKeyFile *key_file;
	char filename[PATH_MAX];
	char adapter_addr[18];
	char device_addr[18];
	char *str;
	char class[9];
	char **uuids = NULL;
	gsize length = 0;

	device->store_id = 0;

	ba2str(btd_adapter_get_address(device->adapter), adapter_addr);
	ba2str(&device->bdaddr, device_addr);
	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/info", adapter_addr,
			device_addr);

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	g_key_file_set_string(key_file, "General", "Name", device->name);

	if (device->alias != NULL)
		g_key_file_set_string(key_file, "General", "Alias",
								device->alias);
	else
		g_key_file_remove_key(key_file, "General", "Alias", NULL);

	if (device->class) {
		sprintf(class, "0x%6.6x", device->class);
		g_key_file_set_string(key_file, "General", "Class", class);
	} else {
		g_key_file_remove_key(key_file, "General", "Class", NULL);
	}

	if (device->appearance) {
		sprintf(class, "0x%4.4x", device->appearance);
		g_key_file_set_string(key_file, "General", "Appearance", class);
	} else {
		g_key_file_remove_key(key_file, "General", "Appearance", NULL);
	}

	update_technologies(key_file, device);

	g_key_file_set_boolean(key_file, "General", "Trusted",
							device->trusted);

	g_key_file_set_boolean(key_file, "General", "Blocked",
							device->blocked);

	if (device->uuids) {
		GSList *l;
		int i;

		uuids = g_new0(char *, g_slist_length(device->uuids) + 1);
		for (i = 0, l = device->uuids; l; l = g_slist_next(l), i++)
			uuids[i] = l->data;
		g_key_file_set_string_list(key_file, "General", "Services",
						(const char **)uuids, i);
	} else {
		g_key_file_remove_key(key_file, "General", "Services", NULL);
	}

	if (device->vendor_src) {
		g_key_file_set_integer(key_file, "DeviceID", "Source",
					device->vendor_src);
		g_key_file_set_integer(key_file, "DeviceID", "Vendor",
					device->vendor);
		g_key_file_set_integer(key_file, "DeviceID", "Product",
					device->product);
		g_key_file_set_integer(key_file, "DeviceID", "Version",
					device->version);
	} else {
		g_key_file_remove_group(key_file, "DeviceID", NULL);
	}

	if (device->local_csrk)
		store_csrk(device->local_csrk, key_file, "LocalSignatureKey");

	if (device->remote_csrk)
		store_csrk(device->remote_csrk, key_file, "RemoteSignatureKey");

	create_file(filename, S_IRUSR | S_IWUSR);

	str = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, str, length, NULL);
	g_free(str);

	g_key_file_free(key_file);
	g_free(uuids);

	return FALSE;
}

static bool device_address_is_private(struct btd_device *dev)
{
	if (dev->bdaddr_type != BDADDR_LE_RANDOM)
		return false;

	switch (dev->bdaddr.b[5] >> 6) {
	case 0x00:	/* Private non-resolvable */
	case 0x01:	/* Private resolvable */
		return true;
	default:
		return false;
	}
}

static void store_device_info(struct btd_device *device)
{
	if (device->temporary || device->store_id > 0)
		return;

	if (device_address_is_private(device)) {
		warn("Can't store info for private addressed device %s",
								device->path);
		return;
	}

	device->store_id = g_idle_add(store_device_info_cb, device);
}

void device_store_cached_name(struct btd_device *dev, const char *name)
{
	char filename[PATH_MAX];
	char s_addr[18], d_addr[18];
	GKeyFile *key_file;
	char *data;
	gsize length = 0;

	if (device_address_is_private(dev)) {
		warn("Can't store name for private addressed device %s",
								dev->path);
		return;
	}

	ba2str(btd_adapter_get_address(dev->adapter), s_addr);
	ba2str(&dev->bdaddr, d_addr);
	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/cache/%s", s_addr, d_addr);
	create_file(filename, S_IRUSR | S_IWUSR);

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);
	g_key_file_set_string(key_file, "General", "Name", name);

	data = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, data, length, NULL);
	g_free(data);

	g_key_file_free(key_file);
}

static void browse_request_free(struct browse_req *req)
{
	if (req->listener_id)
		g_dbus_remove_watch(dbus_conn, req->listener_id);
	if (req->msg)
		dbus_message_unref(req->msg);
	g_slist_free_full(req->profiles_added, g_free);
	if (req->records)
		sdp_list_free(req->records, (sdp_free_func_t) sdp_record_free);

	g_free(req);
}

static void gatt_client_cleanup(struct btd_device *device)
{
	if (!device->client)
		return;

	bt_gatt_client_set_service_changed(device->client, NULL, NULL, NULL);
	bt_gatt_client_set_ready_handler(device->client, NULL, NULL, NULL);
	bt_gatt_client_unref(device->client);
	device->client = NULL;
}

static void gatt_server_cleanup(struct btd_device *device)
{
	if (!device->server)
		return;

	bt_gatt_server_unref(device->server);
	device->server = NULL;
}

static void attio_cleanup(struct btd_device *device)
{
	if (device->att_disconn_id)
		bt_att_unregister_disconnect(device->att,
							device->att_disconn_id);

	if (device->att_io) {
		g_io_channel_shutdown(device->att_io, FALSE, NULL);
		g_io_channel_unref(device->att_io);
		device->att_io = NULL;
	}

	gatt_client_cleanup(device);
	gatt_server_cleanup(device);

	if (device->att) {
		bt_att_unref(device->att);
		device->att = NULL;
	}

	if (device->attrib) {
		GAttrib *attrib = device->attrib;

		device->attrib = NULL;
		g_attrib_cancel_all(attrib);
		g_attrib_unref(attrib);
	}
}

static void browse_request_cancel(struct browse_req *req)
{
	struct btd_device *device = req->device;
	struct btd_adapter *adapter = device->adapter;

	DBG("");

	bt_cancel_discovery(btd_adapter_get_address(adapter), &device->bdaddr);

	attio_cleanup(device);

	device->browse = NULL;
	browse_request_free(req);
}

static void svc_dev_remove(gpointer user_data)
{
	struct svc_callback *cb = user_data;

	if (cb->idle_id > 0)
		g_source_remove(cb->idle_id);

	cb->func(cb->dev, -ENODEV, cb->user_data);

	g_free(cb);
}

static void device_free(gpointer user_data)
{
	struct btd_device *device = user_data;

	btd_gatt_client_destroy(device->client_dbus);
	device->client_dbus = NULL;

	g_slist_free_full(device->uuids, g_free);
	g_slist_free_full(device->primaries, g_free);
	g_slist_free_full(device->attios, g_free);
	g_slist_free_full(device->attios_offline, g_free);
	g_slist_free_full(device->svc_callbacks, svc_dev_remove);

	/* Reset callbacks since the device is going to be freed */
	gatt_db_register(device->db, NULL, NULL, NULL, NULL);

	attio_cleanup(device);

	gatt_db_unref(device->db);

	bt_ad_unref(device->ad);

	if (device->tmp_records)
		sdp_list_free(device->tmp_records,
					(sdp_free_func_t) sdp_record_free);

	if (device->disconn_timer)
		g_source_remove(device->disconn_timer);

	if (device->discov_timer)
		g_source_remove(device->discov_timer);

	if (device->connect)
		dbus_message_unref(device->connect);

	if (device->disconnect)
		dbus_message_unref(device->disconnect);

	DBG("%p", device);

	if (device->authr) {
		if (device->authr->agent)
			agent_unref(device->authr->agent);
		g_free(device->authr->pincode);
		g_free(device->authr);
	}

	if (device->eir_uuids)
		g_slist_free_full(device->eir_uuids, g_free);

	g_free(device->local_csrk);
	g_free(device->remote_csrk);
	g_free(device->path);
	g_free(device->alias);
	free(device->modalias);
	g_free(device);
}

bool device_is_paired(struct btd_device *device, uint8_t bdaddr_type)
{
	struct bearer_state *state = get_state(device, bdaddr_type);

	return state->paired;
}

bool device_is_bonded(struct btd_device *device, uint8_t bdaddr_type)
{
	struct bearer_state *state = get_state(device, bdaddr_type);

	return state->bonded;
}

gboolean device_is_trusted(struct btd_device *device)
{
	return device->trusted;
}

static gboolean dev_property_get_address(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *device = data;
	char dstaddr[18];
	const char *ptr = dstaddr;

	ba2str(&device->bdaddr, dstaddr);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &ptr);

	return TRUE;
}

static gboolean dev_property_get_name(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *device = data;
	const char *ptr = device->name;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &ptr);

	return TRUE;
}

static gboolean dev_property_exists_name(const GDBusPropertyTable *property,
								void *data)
{
	struct btd_device *dev = data;

	return device_name_known(dev);
}

static gboolean dev_property_get_alias(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *device = data;
	char dstaddr[18];
	const char *ptr;

	/* Alias (fallback to name or address) */
	if (device->alias != NULL)
		ptr = device->alias;
	else if (strlen(device->name) > 0) {
		ptr = device->name;
	} else {
		ba2str(&device->bdaddr, dstaddr);
		g_strdelimit(dstaddr, ":", '-');
		ptr = dstaddr;
	}

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &ptr);

	return TRUE;
}

static void set_alias(GDBusPendingPropertySet id, const char *alias,
								void *data)
{
	struct btd_device *device = data;

	/* No change */
	if ((device->alias == NULL && g_str_equal(alias, "")) ||
					g_strcmp0(device->alias, alias) == 0) {
		g_dbus_pending_property_success(id);
		return;
	}

	g_free(device->alias);
	device->alias = g_str_equal(alias, "") ? NULL : g_strdup(alias);

	store_device_info(device);

	g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "Alias");

	g_dbus_pending_property_success(id);
}

static void dev_property_set_alias(const GDBusPropertyTable *property,
					DBusMessageIter *value,
					GDBusPendingPropertySet id, void *data)
{
	const char *alias;

	if (dbus_message_iter_get_arg_type(value) != DBUS_TYPE_STRING) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	dbus_message_iter_get_basic(value, &alias);

	set_alias(id, alias, data);
}

static gboolean dev_property_exists_class(const GDBusPropertyTable *property,
								void *data)
{
	struct btd_device *device = data;

	return device->class != 0;
}

static gboolean dev_property_get_class(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *device = data;

	if (device->class == 0)
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32, &device->class);

	return TRUE;
}

static gboolean get_appearance(const GDBusPropertyTable *property, void *data,
							uint16_t *appearance)
{
	struct btd_device *device = data;

	if (dev_property_exists_class(property, data))
		return FALSE;

	if (device->appearance) {
		*appearance = device->appearance;
		return TRUE;
	}

	return FALSE;
}

static gboolean dev_property_exists_appearance(
			const GDBusPropertyTable *property, void *data)
{
	uint16_t appearance;

	return get_appearance(property, data, &appearance);
}

static gboolean dev_property_get_appearance(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	uint16_t appearance;

	if (!get_appearance(property, data, &appearance))
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16, &appearance);

	return TRUE;
}

static const char *get_icon(const GDBusPropertyTable *property, void *data)
{
	struct btd_device *device = data;
	const char *icon = NULL;
	uint16_t appearance;

	if (device->class != 0)
		icon = class_to_icon(device->class);
	else if (get_appearance(property, data, &appearance))
		icon = gap_appearance_to_icon(appearance);

	return icon;
}

static gboolean dev_property_exists_icon(
			const GDBusPropertyTable *property, void *data)
{
	return get_icon(property, data) != NULL;
}

static gboolean dev_property_get_icon(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	const char *icon;

	icon = get_icon(property, data);
	if (icon == NULL)
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &icon);

	return TRUE;
}

static gboolean dev_property_get_paired(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *dev = data;
	dbus_bool_t val;

	if (dev->bredr_state.paired || dev->le_state.paired)
		val = TRUE;
	else
		val = FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &val);

	return TRUE;
}

static gboolean dev_property_get_legacy(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *device = data;
	dbus_bool_t val = device->legacy;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &val);

	return TRUE;
}

static gboolean dev_property_get_rssi(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *dev = data;
	dbus_int16_t val = dev->rssi;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_INT16, &val);

	return TRUE;
}

static gboolean dev_property_exists_rssi(const GDBusPropertyTable *property,
								void *data)
{
	struct btd_device *dev = data;

	if (dev->rssi == 0)
		return FALSE;

	return TRUE;
}

static gboolean dev_property_get_tx_power(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *dev = data;
	dbus_int16_t val = dev->tx_power;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_INT16, &val);

	return TRUE;
}

static gboolean dev_property_exists_tx_power(const GDBusPropertyTable *property,
								void *data)
{
	struct btd_device *dev = data;

	if (dev->tx_power == 127)
		return FALSE;

	return TRUE;
}

static gboolean
dev_property_get_svc_resolved(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *device = data;
	gboolean val = device->svc_refreshed;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &val);

	return TRUE;
}

static gboolean dev_property_get_trusted(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *device = data;
	gboolean val = device_is_trusted(device);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &val);

	return TRUE;
}

static void set_trust(GDBusPendingPropertySet id, gboolean value, void *data)
{
	struct btd_device *device = data;

	btd_device_set_trusted(device, value);

	g_dbus_pending_property_success(id);
}

static void dev_property_set_trusted(const GDBusPropertyTable *property,
					DBusMessageIter *value,
					GDBusPendingPropertySet id, void *data)
{
	dbus_bool_t b;

	if (dbus_message_iter_get_arg_type(value) != DBUS_TYPE_BOOLEAN) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	dbus_message_iter_get_basic(value, &b);

	set_trust(id, b, data);
}

static gboolean dev_property_get_blocked(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *device = data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN,
							&device->blocked);

	return TRUE;
}

static void set_blocked(GDBusPendingPropertySet id, gboolean value, void *data)
{
	struct btd_device *device = data;
	int err;

	if (value)
		err = device_block(device, FALSE);
	else
		err = device_unblock(device, FALSE, FALSE);

	switch (-err) {
	case 0:
		g_dbus_pending_property_success(id);
		break;
	case EINVAL:
		g_dbus_pending_property_error(id, ERROR_INTERFACE ".Failed",
					"Kernel lacks blacklist support");
		break;
	default:
		g_dbus_pending_property_error(id, ERROR_INTERFACE ".Failed",
							strerror(-err));
		break;
	}
}


static void dev_property_set_blocked(const GDBusPropertyTable *property,
					DBusMessageIter *value,
					GDBusPendingPropertySet id, void *data)
{
	dbus_bool_t b;

	if (dbus_message_iter_get_arg_type(value) != DBUS_TYPE_BOOLEAN) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	dbus_message_iter_get_basic(value, &b);

	set_blocked(id, b, data);
}

static gboolean dev_property_get_connected(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *dev = data;
	dbus_bool_t connected;

	if (dev->bredr_state.connected || dev->le_state.connected)
		connected = TRUE;
	else
		connected = FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &connected);

	return TRUE;
}

static gboolean dev_property_get_uuids(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *dev = data;
	DBusMessageIter entry;
	GSList *l;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
				DBUS_TYPE_STRING_AS_STRING, &entry);

	if (dev->bredr_state.svc_resolved || dev->le_state.svc_resolved)
		l = dev->uuids;
	else if (dev->eir_uuids)
		l = dev->eir_uuids;
	else
		l = dev->uuids;

	for (; l != NULL; l = l->next)
		dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING,
							&l->data);

	dbus_message_iter_close_container(iter, &entry);

	return TRUE;
}

static gboolean dev_property_get_modalias(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *device = data;

	if (!device->modalias)
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING,
							&device->modalias);

	return TRUE;
}

static gboolean dev_property_exists_modalias(const GDBusPropertyTable *property,
								void *data)
{
	struct btd_device *device = data;

	return device->modalias ? TRUE : FALSE;
}

static gboolean dev_property_get_adapter(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *device = data;
	const char *str = adapter_get_path(device->adapter);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &str);

	return TRUE;
}

static void append_manufacturer_data(void *data, void *user_data)
{
	struct bt_ad_manufacturer_data *md = data;
	DBusMessageIter *dict = user_data;

	dict_append_basic_array(dict, DBUS_TYPE_UINT16, &md->manufacturer_id,
				DBUS_TYPE_BYTE, &md->data, md->len);
}

static gboolean
dev_property_get_manufacturer_data(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *device = data;
	DBusMessageIter dict;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_UINT16_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	bt_ad_foreach_manufacturer_data(device->ad, append_manufacturer_data,
									&dict);

	dbus_message_iter_close_container(iter, &dict);

	return TRUE;
}

static gboolean
dev_property_manufacturer_data_exist(const GDBusPropertyTable *property,
								void *data)
{
	struct btd_device *device = data;

	return bt_ad_has_manufacturer_data(device->ad, NULL);
}

static void append_service_data(void *data, void *user_data)
{
	struct bt_ad_service_data *sd = data;
	DBusMessageIter *dict = user_data;
	char uuid_str[MAX_LEN_UUID_STR];

	bt_uuid_to_string(&sd->uuid, uuid_str, sizeof(uuid_str));

	dict_append_array(dict, uuid_str, DBUS_TYPE_BYTE, &sd->data, sd->len);
}

static gboolean
dev_property_get_service_data(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct btd_device *device = data;
	DBusMessageIter dict;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
					DBUS_TYPE_STRING_AS_STRING
					DBUS_TYPE_VARIANT_AS_STRING
					DBUS_DICT_ENTRY_END_CHAR_AS_STRING,
					&dict);

	bt_ad_foreach_service_data(device->ad, append_service_data, &dict);

	dbus_message_iter_close_container(iter, &dict);

	return TRUE;
}

static gboolean
dev_property_service_data_exist(const GDBusPropertyTable *property,
								void *data)
{
	struct btd_device *device = data;

	return bt_ad_has_service_data(device->ad, NULL);
}

static gboolean disconnect_all(gpointer user_data)
{
	struct btd_device *device = user_data;

	device->disconn_timer = 0;

	if (device->bredr_state.connected)
		btd_adapter_disconnect_device(device->adapter, &device->bdaddr,
								BDADDR_BREDR);

	if (device->le_state.connected)
		btd_adapter_disconnect_device(device->adapter, &device->bdaddr,
							device->bdaddr_type);

	return FALSE;
}

int device_block(struct btd_device *device, gboolean update_only)
{
	int err = 0;

	if (device->blocked)
		return 0;

	if (device->disconn_timer > 0)
		g_source_remove(device->disconn_timer);

	disconnect_all(device);

	while (device->services != NULL) {
		struct btd_service *service = device->services->data;

		device->services = g_slist_remove(device->services, service);
		service_remove(service);
	}

	if (!update_only) {
		if (device->le)
			err = btd_adapter_block_address(device->adapter,
							&device->bdaddr,
							device->bdaddr_type);
		if (!err && device->bredr)
			err = btd_adapter_block_address(device->adapter,
							&device->bdaddr,
							BDADDR_BREDR);
	}

	if (err < 0)
		return err;

	device->blocked = TRUE;

	store_device_info(device);

	btd_device_set_temporary(device, false);

	g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "Blocked");

	return 0;
}

int device_unblock(struct btd_device *device, gboolean silent,
							gboolean update_only)
{
	int err = 0;

	if (!device->blocked)
		return 0;

	if (!update_only) {
		if (device->le)
			err = btd_adapter_unblock_address(device->adapter,
							&device->bdaddr,
							device->bdaddr_type);
		if (!err && device->bredr)
			err = btd_adapter_unblock_address(device->adapter,
							&device->bdaddr,
							BDADDR_BREDR);
	}

	if (err < 0)
		return err;

	device->blocked = FALSE;

	store_device_info(device);

	if (!silent) {
		g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "Blocked");
		device_probe_profiles(device, device->uuids);
	}

	return 0;
}

static void browse_request_exit(DBusConnection *conn, void *user_data)
{
	struct browse_req *req = user_data;

	DBG("Requestor exited");

	browse_request_cancel(req);
}

static void bonding_request_cancel(struct bonding_req *bonding)
{
	struct btd_device *device = bonding->device;
	struct btd_adapter *adapter = device->adapter;

	adapter_cancel_bonding(adapter, &device->bdaddr, device->bdaddr_type);
}

static void dev_disconn_service(gpointer a, gpointer b)
{
	btd_service_disconnect(a);
}

void device_request_disconnect(struct btd_device *device, DBusMessage *msg)
{
	if (device->bonding)
		bonding_request_cancel(device->bonding);

	if (device->browse)
		browse_request_cancel(device->browse);

	if (device->att_io) {
		g_io_channel_shutdown(device->att_io, FALSE, NULL);
		g_io_channel_unref(device->att_io);
		device->att_io = NULL;
	}

	if (device->connect) {
		DBusMessage *reply = btd_error_failed(device->connect,
								"Cancelled");
		g_dbus_send_message(dbus_conn, reply);
		dbus_message_unref(device->connect);
		device->connect = NULL;
	}

	if (btd_device_is_connected(device) && msg)
		device->disconnects = g_slist_append(device->disconnects,
						dbus_message_ref(msg));

	if (device->disconn_timer)
		return;

	g_slist_foreach(device->services, dev_disconn_service, NULL);

	g_slist_free(device->pending);
	device->pending = NULL;

	while (device->watches) {
		struct btd_disconnect_data *data = device->watches->data;

		if (data->watch)
			/* temporary is set if device is going to be removed */
			data->watch(device, device->temporary,
							data->user_data);

		/* Check if the watch has been removed by callback function */
		if (!g_slist_find(device->watches, data))
			continue;

		device->watches = g_slist_remove(device->watches, data);
		g_free(data);
	}

	if (!btd_device_is_connected(device)) {
		if (msg)
			g_dbus_send_reply(dbus_conn, msg, DBUS_TYPE_INVALID);
		return;
	}

	device->disconn_timer = g_timeout_add_seconds(DISCONNECT_TIMER,
							disconnect_all,
							device);
}

bool device_is_disconnecting(struct btd_device *device)
{
	return device->disconn_timer > 0;
}

static void device_set_auto_connect(struct btd_device *device, gboolean enable)
{
	char addr[18];

	if (!device || !device->le)
		return;

	ba2str(&device->bdaddr, addr);

	DBG("%s auto connect: %d", addr, enable);

	if (device->auto_connect == enable)
		return;

	device->auto_connect = enable;

	/* Disabling auto connect */
	if (enable == FALSE) {
		adapter_connect_list_remove(device->adapter, device);
		adapter_auto_connect_remove(device->adapter, device);
		return;
	}

	/* Enabling auto connect */
	adapter_auto_connect_add(device->adapter, device);

	if (device->attrib) {
		DBG("Already connected");
		return;
	}

	adapter_connect_list_add(device->adapter, device);
}

static DBusMessage *dev_disconnect(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	struct btd_device *device = user_data;

	/*
	 * Disable connections through passive scanning until
	 * Device1.Connect is called
	 */
	if (device->auto_connect) {
		device->disable_auto_connect = TRUE;
		device_set_auto_connect(device, FALSE);
	}

	device_request_disconnect(device, msg);

	return NULL;
}

static int connect_next(struct btd_device *dev)
{
	struct btd_service *service;
	int err = -ENOENT;

	while (dev->pending) {
		service = dev->pending->data;

		err = btd_service_connect(service);
		if (!err)
			return 0;

		dev->pending = g_slist_delete_link(dev->pending, dev->pending);
	}

	return err;
}

static void device_profile_connected(struct btd_device *dev,
					struct btd_profile *profile, int err)
{
	struct btd_service *pending;
	GSList *l;

	DBG("%s %s (%d)", profile->name, strerror(-err), -err);

	if (!err)
		btd_device_set_temporary(dev, false);

	if (dev->pending == NULL)
		goto done;

	if (!btd_device_is_connected(dev)) {
		switch (-err) {
		case EHOSTDOWN: /* page timeout */
		case EHOSTUNREACH: /* adapter not powered */
		case ECONNABORTED: /* adapter powered down */
			goto done;
		}
	}


	pending = dev->pending->data;
	l = find_service_with_profile(dev->pending, profile);
	if (l != NULL)
		dev->pending = g_slist_delete_link(dev->pending, l);

	/* Only continue connecting the next profile if it matches the first
	 * pending, otherwise it will trigger another connect to the same
	 * profile
	 */
	if (profile != btd_service_get_profile(pending))
		return;

	if (connect_next(dev) == 0)
		return;

done:
	g_slist_free(dev->pending);
	dev->pending = NULL;

	if (!dev->connect)
		return;

	if (!err && dbus_message_is_method_call(dev->connect, DEVICE_INTERFACE,
								"Connect"))
		dev->general_connect = TRUE;

	DBG("returning response to %s", dbus_message_get_sender(dev->connect));

	l = find_service_with_state(dev->services, BTD_SERVICE_STATE_CONNECTED);

	if (err && l == NULL)
		g_dbus_send_message(dbus_conn,
				btd_error_failed(dev->connect, strerror(-err)));
	else {
		/* Start passive SDP discovery to update known services */
		if (dev->bredr && !dev->svc_refreshed)
			device_browse_sdp(dev, NULL);
		g_dbus_send_reply(dbus_conn, dev->connect, DBUS_TYPE_INVALID);
	}

	dbus_message_unref(dev->connect);
	dev->connect = NULL;
}

void device_add_eir_uuids(struct btd_device *dev, GSList *uuids)
{
	GSList *l;
	bool added = false;

	if (dev->bredr_state.svc_resolved || dev->le_state.svc_resolved)
		return;

	for (l = uuids; l != NULL; l = l->next) {
		const char *str = l->data;
		if (g_slist_find_custom(dev->eir_uuids, str, bt_uuid_strcmp))
			continue;
		added = true;
		dev->eir_uuids = g_slist_append(dev->eir_uuids, g_strdup(str));
	}

	if (added)
		g_dbus_emit_property_changed(dbus_conn, dev->path,
						DEVICE_INTERFACE, "UUIDs");
}

static void add_manufacturer_data(void *data, void *user_data)
{
	struct eir_msd *msd = data;
	struct btd_device *dev = user_data;

	if (!bt_ad_add_manufacturer_data(dev->ad, msd->company, msd->data,
								msd->data_len))
		return;

	g_dbus_emit_property_changed(dbus_conn, dev->path,
					DEVICE_INTERFACE, "ManufacturerData");
}

void device_set_manufacturer_data(struct btd_device *dev, GSList *list)
{
	g_slist_foreach(list, add_manufacturer_data, dev);
}

static void add_service_data(void *data, void *user_data)
{
	struct eir_sd *sd = data;
	struct btd_device *dev = user_data;
	bt_uuid_t uuid;

	if (bt_string_to_uuid(&uuid, sd->uuid) < 0)
		return;

	if (!bt_ad_add_service_data(dev->ad, &uuid, sd->data, sd->data_len))
		return;

	g_dbus_emit_property_changed(dbus_conn, dev->path,
					DEVICE_INTERFACE, "ServiceData");
}

void device_set_service_data(struct btd_device *dev, GSList *list)
{
	g_slist_foreach(list, add_service_data, dev);
}

static struct btd_service *find_connectable_service(struct btd_device *dev,
							const char *uuid)
{
	GSList *l;

	for (l = dev->services; l != NULL; l = g_slist_next(l)) {
		struct btd_service *service = l->data;
		struct btd_profile *p = btd_service_get_profile(service);

		if (!p->connect || !p->remote_uuid)
			continue;

		if (strcasecmp(uuid, p->remote_uuid) == 0)
			return service;
	}

	return NULL;
}

static int service_prio_cmp(gconstpointer a, gconstpointer b)
{
	struct btd_profile *p1 = btd_service_get_profile(a);
	struct btd_profile *p2 = btd_service_get_profile(b);

	return p2->priority - p1->priority;
}

static GSList *create_pending_list(struct btd_device *dev, const char *uuid)
{
	struct btd_service *service;
	struct btd_profile *p;
	GSList *l;

	if (uuid) {
		service = find_connectable_service(dev, uuid);
		if (service)
			return g_slist_prepend(dev->pending, service);

		return dev->pending;
	}

	for (l = dev->services; l != NULL; l = g_slist_next(l)) {
		service = l->data;
		p = btd_service_get_profile(service);

		if (!p->auto_connect)
			continue;

		if (g_slist_find(dev->pending, service))
			continue;

		if (btd_service_get_state(service) !=
						BTD_SERVICE_STATE_DISCONNECTED)
			continue;

		dev->pending = g_slist_insert_sorted(dev->pending, service,
							service_prio_cmp);
	}

	return dev->pending;
}

int btd_device_connect_services(struct btd_device *dev, GSList *services)
{
	GSList *l;

	if (dev->pending || dev->connect || dev->browse)
		return -EBUSY;

	if (!btd_adapter_get_powered(dev->adapter))
		return -ENETDOWN;

	if (!dev->bredr_state.svc_resolved)
		return -ENOENT;

	for (l = services; l; l = g_slist_next(l)) {
		struct btd_service *service = l->data;

		dev->pending = g_slist_append(dev->pending, service);
	}

	return connect_next(dev);
}

static DBusMessage *connect_profiles(struct btd_device *dev, uint8_t bdaddr_type,
					DBusMessage *msg, const char *uuid)
{
	struct bearer_state *state = get_state(dev, bdaddr_type);
	int err;

	DBG("%s %s, client %s", dev->path, uuid ? uuid : "(all)",
						dbus_message_get_sender(msg));

	if (dev->pending || dev->connect || dev->browse)
		return btd_error_in_progress(msg);

	if (!btd_adapter_get_powered(dev->adapter))
		return btd_error_not_ready(msg);

	btd_device_set_temporary(dev, false);

	if (!state->svc_resolved)
		goto resolve_services;

	dev->pending = create_pending_list(dev, uuid);
	if (!dev->pending) {
		if (dev->svc_refreshed) {
			if (find_service_with_state(dev->services,
						BTD_SERVICE_STATE_CONNECTED))
				return dbus_message_new_method_return(msg);
			else
				return btd_error_not_available(msg);
		}

		goto resolve_services;
	}

	err = connect_next(dev);
	if (err < 0) {
		if (err == -EALREADY)
			return dbus_message_new_method_return(msg);
		return btd_error_failed(msg, strerror(-err));
	}

	dev->connect = dbus_message_ref(msg);

	return NULL;

resolve_services:
	DBG("Resolving services for %s", dev->path);

	if (bdaddr_type == BDADDR_BREDR)
		err = device_browse_sdp(dev, msg);
	else
		err = device_browse_gatt(dev, msg);
	if (err < 0)
		return btd_error_failed(msg, strerror(-err));

	return NULL;
}

#define NVAL_TIME ((time_t) -1)
#define SEEN_TRESHHOLD 300

static uint8_t select_conn_bearer(struct btd_device *dev)
{
	time_t bredr_last = NVAL_TIME, le_last = NVAL_TIME;
	time_t current = time(NULL);

	if (dev->bredr_seen) {
		bredr_last = current - dev->bredr_seen;
		if (bredr_last > SEEN_TRESHHOLD)
			bredr_last = NVAL_TIME;
	}

	if (dev->le_seen) {
		le_last = current - dev->le_seen;
		if (le_last > SEEN_TRESHHOLD)
			le_last = NVAL_TIME;
	}

	if (le_last == NVAL_TIME && bredr_last == NVAL_TIME)
		return dev->bdaddr_type;

	if (dev->bredr && (!dev->le || le_last == NVAL_TIME))
		return BDADDR_BREDR;

	if (dev->le && (!dev->bredr || bredr_last == NVAL_TIME))
		return dev->bdaddr_type;

	if (bredr_last < le_last)
		return BDADDR_BREDR;

	return dev->bdaddr_type;
}

static DBusMessage *dev_connect(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	struct btd_device *dev = user_data;
	uint8_t bdaddr_type;

	if (dev->bredr_state.connected)
		bdaddr_type = dev->bdaddr_type;
	else if (dev->le_state.connected && dev->bredr)
		bdaddr_type = BDADDR_BREDR;
	else
		bdaddr_type = select_conn_bearer(dev);

	if (bdaddr_type != BDADDR_BREDR) {
		int err;

		if (dev->le_state.connected)
			return dbus_message_new_method_return(msg);

		btd_device_set_temporary(dev, false);

		if (dev->disable_auto_connect) {
			dev->disable_auto_connect = FALSE;
			device_set_auto_connect(dev, TRUE);
		}

		err = device_connect_le(dev);
		if (err < 0)
			return btd_error_failed(msg, strerror(-err));

		dev->connect = dbus_message_ref(msg);

		return NULL;
	}

	return connect_profiles(dev, bdaddr_type, msg, NULL);
}

static DBusMessage *connect_profile(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	struct btd_device *dev = user_data;
	const char *pattern;
	char *uuid;
	DBusMessage *reply;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &pattern,
							DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	uuid = bt_name2string(pattern);
	reply = connect_profiles(dev, BDADDR_BREDR, msg, uuid);
	free(uuid);

	return reply;
}

static void device_profile_disconnected(struct btd_device *dev,
					struct btd_profile *profile, int err)
{
	if (!dev->disconnect)
		return;

	if (err)
		g_dbus_send_message(dbus_conn,
					btd_error_failed(dev->disconnect,
							strerror(-err)));
	else
		g_dbus_send_reply(dbus_conn, dev->disconnect,
							DBUS_TYPE_INVALID);

	dbus_message_unref(dev->disconnect);
	dev->disconnect = NULL;
}

static DBusMessage *disconnect_profile(DBusConnection *conn, DBusMessage *msg,
							void *user_data)
{
	struct btd_device *dev = user_data;
	struct btd_service *service;
	const char *pattern;
	char *uuid;
	int err;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_STRING, &pattern,
							DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	uuid = bt_name2string(pattern);
	if (uuid == NULL)
		return btd_error_invalid_args(msg);

	service = find_connectable_service(dev, uuid);
	free(uuid);

	if (!service)
		return btd_error_invalid_args(msg);

	if (dev->disconnect)
		return btd_error_in_progress(msg);

	dev->disconnect = dbus_message_ref(msg);

	err = btd_service_disconnect(service);
	if (err == 0)
		return NULL;

	dbus_message_unref(dev->disconnect);
	dev->disconnect = NULL;

	if (err == -ENOTSUP)
		return btd_error_not_supported(msg);

	return btd_error_failed(msg, strerror(-err));
}

static void store_services(struct btd_device *device)
{
	struct btd_adapter *adapter = device->adapter;
	char filename[PATH_MAX];
	char src_addr[18], dst_addr[18];
	uuid_t uuid;
	char *prim_uuid;
	GKeyFile *key_file;
	GSList *l;
	char *data;
	gsize length = 0;

	if (device_address_is_private(device)) {
		warn("Can't store services for private addressed device %s",
								device->path);
		return;
	}

	sdp_uuid16_create(&uuid, GATT_PRIM_SVC_UUID);
	prim_uuid = bt_uuid2string(&uuid);
	if (prim_uuid == NULL)
		return;

	ba2str(btd_adapter_get_address(adapter), src_addr);
	ba2str(&device->bdaddr, dst_addr);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/attributes", src_addr,
								dst_addr);
	key_file = g_key_file_new();

	for (l = device->primaries; l; l = l->next) {
		struct gatt_primary *primary = l->data;
		char handle[6], uuid_str[33];
		int i;

		sprintf(handle, "%hu", primary->range.start);

		bt_string2uuid(&uuid, primary->uuid);
		sdp_uuid128_to_uuid(&uuid);

		switch (uuid.type) {
		case SDP_UUID16:
			sprintf(uuid_str, "%4.4X", uuid.value.uuid16);
			break;
		case SDP_UUID32:
			sprintf(uuid_str, "%8.8X", uuid.value.uuid32);
			break;
		case SDP_UUID128:
			for (i = 0; i < 16; i++)
				sprintf(uuid_str + (i * 2), "%2.2X",
						uuid.value.uuid128.data[i]);
			break;
		default:
			uuid_str[0] = '\0';
		}

		g_key_file_set_string(key_file, handle, "UUID", prim_uuid);
		g_key_file_set_string(key_file, handle, "Value", uuid_str);
		g_key_file_set_integer(key_file, handle, "EndGroupHandle",
					primary->range.end);
	}

	data = g_key_file_to_data(key_file, &length, NULL);
	if (length > 0) {
		create_file(filename, S_IRUSR | S_IWUSR);
		g_file_set_contents(filename, data, length, NULL);
	}

	free(prim_uuid);
	g_free(data);
	g_key_file_free(key_file);
}

struct gatt_saver {
	struct btd_device *device;
	uint16_t ext_props;
	GKeyFile *key_file;
};

static void store_desc(struct gatt_db_attribute *attr, void *user_data)
{
	struct gatt_saver *saver = user_data;
	GKeyFile *key_file = saver->key_file;
	char handle[6], value[100], uuid_str[MAX_LEN_UUID_STR];
	const bt_uuid_t *uuid;
	bt_uuid_t ext_uuid;
	uint16_t handle_num;

	handle_num = gatt_db_attribute_get_handle(attr);
	sprintf(handle, "%04hx", handle_num);

	uuid = gatt_db_attribute_get_type(attr);
	bt_uuid_to_string(uuid, uuid_str, sizeof(uuid_str));

	bt_uuid16_create(&ext_uuid, GATT_CHARAC_EXT_PROPER_UUID);
	if (!bt_uuid_cmp(uuid, &ext_uuid) && saver->ext_props)
		sprintf(value, "%04hx:%s", saver->ext_props, uuid_str);
	else
		sprintf(value, "%s", uuid_str);

	g_key_file_set_string(key_file, "Attributes", handle, value);
}

static void store_chrc(struct gatt_db_attribute *attr, void *user_data)
{
	struct gatt_saver *saver = user_data;
	GKeyFile *key_file = saver->key_file;
	char handle[6], value[100], uuid_str[MAX_LEN_UUID_STR];
	uint16_t handle_num, value_handle;
	uint8_t properties;
	bt_uuid_t uuid;

	if (!gatt_db_attribute_get_char_data(attr, &handle_num, &value_handle,
						&properties, &saver->ext_props,
						&uuid)) {
		warn("Error storing characteristic - can't get data");
		return;
	}

	sprintf(handle, "%04hx", handle_num);
	bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));
	sprintf(value, GATT_CHARAC_UUID_STR ":%04hx:%02hhx:%s", value_handle,
							properties, uuid_str);
	g_key_file_set_string(key_file, "Attributes", handle, value);

	gatt_db_service_foreach_desc(attr, store_desc, saver);
}

static void store_incl(struct gatt_db_attribute *attr, void *user_data)
{
	struct gatt_saver *saver = user_data;
	GKeyFile *key_file = saver->key_file;
	struct gatt_db_attribute *service;
	char handle[6], value[100], uuid_str[MAX_LEN_UUID_STR];
	uint16_t handle_num, start, end;
	bt_uuid_t uuid;

	if (!gatt_db_attribute_get_incl_data(attr, &handle_num, &start, &end)) {
		warn("Error storing included service - can't get data");
		return;
	}

	service = gatt_db_get_attribute(saver->device->db, start);
	if (!service) {
		warn("Error storing included service - can't find it");
		return;
	}

	sprintf(handle, "%04hx", handle_num);

	bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));
	sprintf(value, GATT_INCLUDE_UUID_STR ":%04hx:%04hx:%s", start,
								end, uuid_str);

	g_key_file_set_string(key_file, "Attributes", handle, value);
}

static void store_service(struct gatt_db_attribute *attr, void *user_data)
{
	struct gatt_saver *saver = user_data;
	GKeyFile *key_file = saver->key_file;
	char uuid_str[MAX_LEN_UUID_STR], handle[6], value[256];
	uint16_t start, end;
	bt_uuid_t uuid;
	bool primary;
	char *type;

	if (!gatt_db_attribute_get_service_data(attr, &start, &end, &primary,
								&uuid)) {
		warn("Error storing service - can't get data");
		return;
	}

	sprintf(handle, "%04hx", start);

	bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));

	if (primary)
		type = GATT_PRIM_SVC_UUID_STR;
	else
		type = GATT_SND_SVC_UUID_STR;

	sprintf(value, "%s:%04hx:%s", type, end, uuid_str);

	g_key_file_set_string(key_file, "Attributes", handle, value);

	gatt_db_service_foreach_incl(attr, store_incl, saver);
	gatt_db_service_foreach_char(attr, store_chrc, saver);
}

static void store_gatt_db(struct btd_device *device)
{
	struct btd_adapter *adapter = device->adapter;
	char filename[PATH_MAX];
	char src_addr[18], dst_addr[18];
	GKeyFile *key_file;
	char *data;
	gsize length = 0;
	struct gatt_saver saver;

	if (device_address_is_private(device)) {
		warn("Can't store GATT db for private addressed device %s",
								device->path);
		return;
	}

	ba2str(btd_adapter_get_address(adapter), src_addr);
	ba2str(&device->bdaddr, dst_addr);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/cache/%s", src_addr,
								dst_addr);
	create_file(filename, S_IRUSR | S_IWUSR);

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	/* Remove current attributes since it might have changed */
	g_key_file_remove_group(key_file, "Attributes", NULL);

	saver.key_file = key_file;
	saver.device = device;

	gatt_db_foreach_service(device->db, NULL, store_service, &saver);

	data = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, data, length, NULL);

	g_free(data);
	g_key_file_free(key_file);
}


static void browse_request_complete(struct browse_req *req, uint8_t bdaddr_type,
									int err)
{
	struct btd_device *dev = req->device;
	DBusMessage *reply = NULL;

	if (!req->msg)
		goto done;

	if (dbus_message_is_method_call(req->msg, DEVICE_INTERFACE, "Pair")) {
		if (!device_is_paired(dev, bdaddr_type)) {
			reply = btd_error_failed(req->msg, "Not paired");
			goto done;
		}

		if (dev->pending_paired) {
			g_dbus_emit_property_changed(dbus_conn, dev->path,
						DEVICE_INTERFACE, "Paired");
			dev->pending_paired = false;
		}

		/* Disregard browse errors in case of Pair */
		reply = g_dbus_create_reply(req->msg, DBUS_TYPE_INVALID);
		goto done;
	}

	if (err) {
		reply = btd_error_failed(req->msg, strerror(-err));
		goto done;
	}

	if (dbus_message_is_method_call(req->msg, DEVICE_INTERFACE, "Connect"))
		reply = dev_connect(dbus_conn, req->msg, dev);
	else if (dbus_message_is_method_call(req->msg, DEVICE_INTERFACE,
							"ConnectProfile"))
		reply = connect_profile(dbus_conn, req->msg, dev);
	else
		reply = g_dbus_create_reply(req->msg, DBUS_TYPE_INVALID);

done:
	if (reply)
		g_dbus_send_message(dbus_conn, reply);

	browse_request_free(req);
}

static void device_set_svc_refreshed(struct btd_device *device, bool value)
{
	if (device->svc_refreshed == value)
		return;

	device->svc_refreshed = value;

	g_dbus_emit_property_changed(dbus_conn, device->path,
					DEVICE_INTERFACE, "ServicesResolved");
}

static void device_svc_resolved(struct btd_device *dev, uint8_t bdaddr_type,
								int err)
{
	struct bearer_state *state = get_state(dev, bdaddr_type);
	struct browse_req *req = dev->browse;

	DBG("%s err %d", dev->path, err);
	if (WORKAROUND) usleep(200*1000);

	state->svc_resolved = true;

	/* Disconnection notification can happen before this function
	 * gets called, so don't set svc_refreshed for a disconnected
	 * device.
	 */
	if (state->connected)
		device_set_svc_refreshed(dev, true);

	g_slist_free_full(dev->eir_uuids, g_free);
	dev->eir_uuids = NULL;

	if (dev->pending_paired) {
		g_dbus_emit_property_changed(dbus_conn, dev->path,
						DEVICE_INTERFACE, "Paired");
		dev->pending_paired = false;
	}

	while (dev->svc_callbacks) {
		struct svc_callback *cb = dev->svc_callbacks->data;

		if (cb->idle_id > 0)
			g_source_remove(cb->idle_id);

		cb->func(dev, err, cb->user_data);

		dev->svc_callbacks = g_slist_delete_link(dev->svc_callbacks,
							dev->svc_callbacks);
		g_free(cb);
	}

	if (!dev->temporary)
		store_device_info(dev);

	if (bdaddr_type != BDADDR_BREDR && err == 0)
		store_services(dev);

	if (!req)
		return;

	dev->browse = NULL;
	browse_request_complete(req, bdaddr_type, err);
}

static struct bonding_req *bonding_request_new(DBusMessage *msg,
						struct btd_device *device,
						uint8_t bdaddr_type,
						struct agent *agent)
{
	struct bonding_req *bonding;
	char addr[18];

	ba2str(&device->bdaddr, addr);
	DBG("Requesting bonding for %s", addr);

	bonding = g_new0(struct bonding_req, 1);

	bonding->msg = dbus_message_ref(msg);
	bonding->bdaddr_type = bdaddr_type;

	bonding->cb_iter = btd_adapter_pin_cb_iter_new(device->adapter);

	/* Marks the bonding start time for the first attempt on request
	 * construction. The following attempts will be updated on
	 * device_bonding_retry. */
	clock_gettime(CLOCK_MONOTONIC, &bonding->attempt_start_time);

	if (agent)
		bonding->agent = agent_ref(agent);

	return bonding;
}

void device_bonding_restart_timer(struct btd_device *device)
{
	if (!device || !device->bonding)
		return;

	clock_gettime(CLOCK_MONOTONIC, &device->bonding->attempt_start_time);
}

static void bonding_request_stop_timer(struct bonding_req *bonding)
{
	struct timespec current;

	clock_gettime(CLOCK_MONOTONIC, &current);

	/* Compute the time difference in ms. */
	bonding->last_attempt_duration_ms =
		(current.tv_sec - bonding->attempt_start_time.tv_sec) * 1000L +
		(current.tv_nsec - bonding->attempt_start_time.tv_nsec)
								/ 1000000L;
}

/* Returns the duration of the last bonding attempt in milliseconds. The
 * duration is measured starting from the latest of the following three
 * events and finishing when the Command complete event is received for the
 * authentication request:
 *  - MGMT_OP_PAIR_DEVICE is sent,
 *  - MGMT_OP_PIN_CODE_REPLY is sent and
 *  - Command complete event is received for the sent MGMT_OP_PIN_CODE_REPLY.
 */
long device_bonding_last_duration(struct btd_device *device)
{
	struct bonding_req *bonding = device->bonding;

	if (!bonding)
		return 0;

	return bonding->last_attempt_duration_ms;
}

static void create_bond_req_exit(DBusConnection *conn, void *user_data)
{
	struct btd_device *device = user_data;
	char addr[18];

	ba2str(&device->bdaddr, addr);
	DBG("%s: requestor exited before bonding was completed", addr);

	if (device->authr)
		device_cancel_authentication(device, FALSE);

	if (device->bonding) {
		device->bonding->listener_id = 0;
		device_request_disconnect(device, NULL);
	}
}

static DBusMessage *pair_device(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	struct btd_device *device = data;
	struct btd_adapter *adapter = device->adapter;
	struct bearer_state *state;
	uint8_t bdaddr_type;
	const char *sender;
	struct agent *agent;
	struct bonding_req *bonding;
	uint8_t io_cap;
	int err;

	btd_device_set_temporary(device, false);

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	if (device->bonding)
		return btd_error_in_progress(msg);

	if (device->bredr_state.bonded)
		bdaddr_type = device->bdaddr_type;
	else if (device->le_state.bonded)
		bdaddr_type = BDADDR_BREDR;
	else
		bdaddr_type = select_conn_bearer(device);

	state = get_state(device, bdaddr_type);

	if (state->bonded)
		return btd_error_already_exists(msg);

	sender = dbus_message_get_sender(msg);

	agent = agent_get(sender);
	if (agent)
		io_cap = agent_get_io_capability(agent);
	else
		io_cap = IO_CAPABILITY_NOINPUTNOOUTPUT;

	bonding = bonding_request_new(msg, device, bdaddr_type, agent);

	if (agent)
		agent_unref(agent);

	bonding->listener_id = g_dbus_add_disconnect_watch(dbus_conn,
						sender, create_bond_req_exit,
						device, NULL);

	device->bonding = bonding;
	bonding->device = device;

	/* Due to a bug in the kernel we might loose out on ATT commands
	 * that arrive during the SMP procedure, so connect the ATT
	 * channel first and only then start pairing (there's code for
	 * this in the ATT connect callback)
	 */
	if (bdaddr_type != BDADDR_BREDR) {
		if (!state->connected && btd_le_connect_before_pairing())
			err = device_connect_le(device);
		else
			err = adapter_create_bonding(adapter, &device->bdaddr,
							device->bdaddr_type,
							io_cap);
	} else {
		err = adapter_create_bonding(adapter, &device->bdaddr,
							BDADDR_BREDR, io_cap);
	}

	if (err < 0)
		return btd_error_failed(msg, strerror(-err));

	return NULL;
}

static DBusMessage *new_authentication_return(DBusMessage *msg, uint8_t status)
{
	switch (status) {
	case MGMT_STATUS_SUCCESS:
		return dbus_message_new_method_return(msg);

	case MGMT_STATUS_CONNECT_FAILED:
		return dbus_message_new_error(msg,
				ERROR_INTERFACE ".ConnectionAttemptFailed",
				"Page Timeout");
	case MGMT_STATUS_TIMEOUT:
		return dbus_message_new_error(msg,
				ERROR_INTERFACE ".AuthenticationTimeout",
				"Authentication Timeout");
	case MGMT_STATUS_BUSY:
	case MGMT_STATUS_REJECTED:
		return dbus_message_new_error(msg,
				ERROR_INTERFACE ".AuthenticationRejected",
				"Authentication Rejected");
	case MGMT_STATUS_CANCELLED:
	case MGMT_STATUS_NO_RESOURCES:
	case MGMT_STATUS_DISCONNECTED:
		return dbus_message_new_error(msg,
				ERROR_INTERFACE ".AuthenticationCanceled",
				"Authentication Canceled");
	case MGMT_STATUS_ALREADY_PAIRED:
		return dbus_message_new_error(msg,
				ERROR_INTERFACE ".AlreadyExists",
				"Already Paired");
	default:
		return dbus_message_new_error(msg,
				ERROR_INTERFACE ".AuthenticationFailed",
				"Authentication Failed");
	}
}

static void bonding_request_free(struct bonding_req *bonding)
{
	if (!bonding)
		return;

	if (bonding->listener_id)
		g_dbus_remove_watch(dbus_conn, bonding->listener_id);

	if (bonding->msg)
		dbus_message_unref(bonding->msg);

	if (bonding->cb_iter)
		g_free(bonding->cb_iter);

	if (bonding->agent) {
		agent_cancel(bonding->agent);
		agent_unref(bonding->agent);
		bonding->agent = NULL;
	}

	if (bonding->retry_timer)
		g_source_remove(bonding->retry_timer);

	if (bonding->device)
		bonding->device->bonding = NULL;

	g_free(bonding);
}

static void device_cancel_bonding(struct btd_device *device, uint8_t status)
{
	struct bonding_req *bonding = device->bonding;
	DBusMessage *reply;
	char addr[18];

	if (!bonding)
		return;

	ba2str(&device->bdaddr, addr);
	DBG("Canceling bonding request for %s", addr);

	if (device->authr)
		device_cancel_authentication(device, FALSE);

	reply = new_authentication_return(bonding->msg, status);
	g_dbus_send_message(dbus_conn, reply);

	bonding_request_cancel(bonding);
	bonding_request_free(bonding);
}

static DBusMessage *cancel_pairing(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	struct btd_device *device = data;
	struct bonding_req *req = device->bonding;

	DBG("");

	if (!req)
		return btd_error_does_not_exist(msg);

	device_cancel_bonding(device, MGMT_STATUS_CANCELLED);

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable device_methods[] = {
	{ GDBUS_ASYNC_METHOD("Disconnect", NULL, NULL, dev_disconnect) },
	{ GDBUS_ASYNC_METHOD("Connect", NULL, NULL, dev_connect) },
	{ GDBUS_ASYNC_METHOD("ConnectProfile", GDBUS_ARGS({ "UUID", "s" }),
						NULL, connect_profile) },
	{ GDBUS_ASYNC_METHOD("DisconnectProfile", GDBUS_ARGS({ "UUID", "s" }),
						NULL, disconnect_profile) },
	{ GDBUS_ASYNC_METHOD("Pair", NULL, NULL, pair_device) },
	{ GDBUS_METHOD("CancelPairing", NULL, NULL, cancel_pairing) },
	{ }
};

static const GDBusPropertyTable device_properties[] = {
	{ "Address", "s", dev_property_get_address },
	{ "Name", "s", dev_property_get_name, NULL, dev_property_exists_name },
	{ "Alias", "s", dev_property_get_alias, dev_property_set_alias },
	{ "Class", "u", dev_property_get_class, NULL,
					dev_property_exists_class },
	{ "Appearance", "q", dev_property_get_appearance, NULL,
					dev_property_exists_appearance },
	{ "Icon", "s", dev_property_get_icon, NULL,
					dev_property_exists_icon },
	{ "Paired", "b", dev_property_get_paired },
	{ "Trusted", "b", dev_property_get_trusted, dev_property_set_trusted },
	{ "Blocked", "b", dev_property_get_blocked, dev_property_set_blocked },
	{ "LegacyPairing", "b", dev_property_get_legacy },
	{ "RSSI", "n", dev_property_get_rssi, NULL, dev_property_exists_rssi },
	{ "Connected", "b", dev_property_get_connected },
	{ "UUIDs", "as", dev_property_get_uuids },
	{ "Modalias", "s", dev_property_get_modalias, NULL,
						dev_property_exists_modalias },
	{ "Adapter", "o", dev_property_get_adapter },
	{ "ManufacturerData", "a{qv}", dev_property_get_manufacturer_data,
				NULL, dev_property_manufacturer_data_exist,
				G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{ "ServiceData", "a{sv}", dev_property_get_service_data,
				NULL, dev_property_service_data_exist,
				G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{ "TxPower", "n", dev_property_get_tx_power, NULL,
					dev_property_exists_tx_power,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },
	{ "ServicesResolved", "b", dev_property_get_svc_resolved, NULL, NULL,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL },

	{ }
};

uint8_t btd_device_get_bdaddr_type(struct btd_device *dev)
{
	return dev->bdaddr_type;
}

bool btd_device_is_connected(struct btd_device *dev)
{
	return dev->bredr_state.connected || dev->le_state.connected;
}

void device_add_connection(struct btd_device *dev, uint8_t bdaddr_type)
{
	struct bearer_state *state = get_state(dev, bdaddr_type);

	device_update_last_seen(dev, bdaddr_type);
	DBG("First connection: %s", state->connected?"No":"Yes");

	if (state->connected) {
		char addr[18];
		ba2str(&dev->bdaddr, addr);
		error("Device %s is already connected", addr);
		return;
	}

	/* If this is the first connection over this bearer */
	if (bdaddr_type == BDADDR_BREDR)
		device_set_bredr_support(dev);
	else
		device_set_le_support(dev, bdaddr_type);

	state->connected = true;

	if (dev->le_state.connected && dev->bredr_state.connected)
		return;

	g_dbus_emit_property_changed(dbus_conn, dev->path, DEVICE_INTERFACE,
								"Connected");
}

void device_remove_connection(struct btd_device *device, uint8_t bdaddr_type)
{
	struct bearer_state *state = get_state(device, bdaddr_type);

	if (!state->connected)
		return;

	state->connected = false;
	device->general_connect = FALSE;

	device_set_svc_refreshed(device, false);

	if (device->disconn_timer > 0) {
		g_source_remove(device->disconn_timer);
		device->disconn_timer = 0;
	}

	while (device->disconnects) {
		DBusMessage *msg = device->disconnects->data;

		g_dbus_send_reply(dbus_conn, msg, DBUS_TYPE_INVALID);
		device->disconnects = g_slist_remove(device->disconnects, msg);
		dbus_message_unref(msg);
	}

	if (state->paired && !state->bonded)
		btd_adapter_remove_bonding(device->adapter, &device->bdaddr,
								bdaddr_type);

	if (device->bredr_state.connected || device->le_state.connected)
		return;

	g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "Connected");
}

guint device_add_disconnect_watch(struct btd_device *device,
				disconnect_watch watch, void *user_data,
				GDestroyNotify destroy)
{
	struct btd_disconnect_data *data;
	static guint id = 0;

	data = g_new0(struct btd_disconnect_data, 1);
	data->id = ++id;
	data->watch = watch;
	data->user_data = user_data;
	data->destroy = destroy;

	device->watches = g_slist_append(device->watches, data);

	return data->id;
}

void device_remove_disconnect_watch(struct btd_device *device, guint id)
{
	GSList *l;

	for (l = device->watches; l; l = l->next) {
		struct btd_disconnect_data *data = l->data;

		if (data->id == id) {
			device->watches = g_slist_remove(device->watches,
							data);
			if (data->destroy)
				data->destroy(data->user_data);
			g_free(data);
			return;
		}
	}
}

static char *load_cached_name(struct btd_device *device, const char *local,
				const char *peer)
{
	char filename[PATH_MAX];
	GKeyFile *key_file;
	char *str = NULL;
	int len;

	if (device_address_is_private(device))
		return NULL;

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/cache/%s", local, peer);

	key_file = g_key_file_new();

	if (!g_key_file_load_from_file(key_file, filename, 0, NULL))
		goto failed;

	str = g_key_file_get_string(key_file, "General", "Name", NULL);
	if (str) {
		len = strlen(str);
		if (len > HCI_MAX_NAME_LENGTH)
			str[HCI_MAX_NAME_LENGTH] = '\0';
	}

failed:
	g_key_file_free(key_file);

	return str;
}

static struct csrk_info *load_csrk(GKeyFile *key_file, const char *group)
{
	struct csrk_info *csrk;
	char *str;
	int i;

	str = g_key_file_get_string(key_file, group, "Key", NULL);
	if (!str)
		return NULL;

	csrk = g_new0(struct csrk_info, 1);

	for (i = 0; i < 16; i++) {
		if (sscanf(str + (i * 2), "%2hhx", &csrk->key[i]) != 1)
			goto fail;
	}

	/*
	 * In case of older storage this will return 0 which is fine since it
	 * didn't support signing at that point the counter should never have
	 * been used.
	 */
	csrk->counter = g_key_file_get_integer(key_file, group, "Counter",
									NULL);
	g_free(str);

	return csrk;

fail:
	g_free(str);
	g_free(csrk);
	return NULL;
}

static void load_services(struct btd_device *device, char **uuids)
{
	char **uuid;

	for (uuid = uuids; *uuid; uuid++) {
		if (g_slist_find_custom(device->uuids, *uuid, bt_uuid_strcmp))
			continue;

		device->uuids = g_slist_insert_sorted(device->uuids,
							g_strdup(*uuid),
							bt_uuid_strcmp);
	}

	g_strfreev(uuids);
}

static void convert_info(struct btd_device *device, GKeyFile *key_file)
{
	char filename[PATH_MAX];
	char adapter_addr[18];
	char device_addr[18];
	char **uuids;
	char *str;
	gsize length = 0;

	/* Load device profile list from legacy properties */
	uuids = g_key_file_get_string_list(key_file, "General", "SDPServices",
								NULL, NULL);
	if (uuids)
		load_services(device, uuids);

	uuids = g_key_file_get_string_list(key_file, "General", "GATTServices",
								NULL, NULL);
	if (uuids)
		load_services(device, uuids);

	if (!device->uuids)
		return;

	/* Remove old entries so they are not loaded again */
	g_key_file_remove_key(key_file, "General", "SDPServices", NULL);
	g_key_file_remove_key(key_file, "General", "GATTServices", NULL);

	ba2str(btd_adapter_get_address(device->adapter), adapter_addr);
	ba2str(&device->bdaddr, device_addr);
	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/info", adapter_addr,
			device_addr);

	str = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, str, length, NULL);
	g_free(str);

	store_device_info(device);
}

static void load_info(struct btd_device *device, const char *local,
			const char *peer, GKeyFile *key_file)
{
	char *str;
	gboolean store_needed = FALSE;
	gboolean blocked;
	char **uuids;
	int source, vendor, product, version;
	char **techno, **t;

	/* Load device name from storage info file, if that fails fall back to
	 * the cache.
	 */
	str = g_key_file_get_string(key_file, "General", "Name", NULL);
	if (str == NULL) {
		str = load_cached_name(device, local, peer);
		if (str)
			store_needed = TRUE;
	}

	if (str) {
		strcpy(device->name, str);
		g_free(str);
	}

	/* Load alias */
	device->alias = g_key_file_get_string(key_file, "General", "Alias",
									NULL);

	/* Load class */
	str = g_key_file_get_string(key_file, "General", "Class", NULL);
	if (str) {
		uint32_t class;

		if (sscanf(str, "%x", &class) == 1)
			device->class = class;
		g_free(str);
	}

	/* Load appearance */
	str = g_key_file_get_string(key_file, "General", "Appearance", NULL);
	if (str) {
		device->appearance = strtol(str, NULL, 16);
		g_free(str);
	}

	/* Load device technology */
	techno = g_key_file_get_string_list(key_file, "General",
					"SupportedTechnologies", NULL, NULL);
	if (!techno)
		goto next;

	for (t = techno; *t; t++) {
		if (g_str_equal(*t, "BR/EDR"))
			device->bredr = true;
		else if (g_str_equal(*t, "LE"))
			device->le = true;
		else
			error("Unknown device technology");
	}

	if (!device->le) {
		device->bdaddr_type = BDADDR_BREDR;
	} else {
		str = g_key_file_get_string(key_file, "General",
						"AddressType", NULL);

		if (str && g_str_equal(str, "public"))
			device->bdaddr_type = BDADDR_LE_PUBLIC;
		else if (str && g_str_equal(str, "static"))
			device->bdaddr_type = BDADDR_LE_RANDOM;
		else
			error("Unknown LE device technology");

		g_free(str);

		device->local_csrk = load_csrk(key_file, "LocalSignatureKey");
		device->remote_csrk = load_csrk(key_file, "RemoteSignatureKey");
	}

	g_strfreev(techno);

next:
	/* Load trust */
	device->trusted = g_key_file_get_boolean(key_file, "General",
							"Trusted", NULL);

	/* Load device blocked */
	blocked = g_key_file_get_boolean(key_file, "General", "Blocked", NULL);
	if (blocked)
		device_block(device, FALSE);

	/* Load device profile list */
	uuids = g_key_file_get_string_list(key_file, "General", "Services",
						NULL, NULL);
	if (uuids) {
		load_services(device, uuids);

		/* Discovered services restored from storage */
		device->bredr_state.svc_resolved = true;
	}

	/* Load device id */
	source = g_key_file_get_integer(key_file, "DeviceID", "Source", NULL);
	if (source) {
		vendor = g_key_file_get_integer(key_file, "DeviceID",
							"Vendor", NULL);

		product = g_key_file_get_integer(key_file, "DeviceID",
							"Product", NULL);

		version = g_key_file_get_integer(key_file, "DeviceID",
							"Version", NULL);

		btd_device_set_pnpid(device, source, vendor, product, version);
	}

	if (store_needed)
		store_device_info(device);
}

static void load_att_info(struct btd_device *device, const char *local,
				const char *peer)
{
	char filename[PATH_MAX];
	GKeyFile *key_file;
	char *prim_uuid, *str;
	char **groups, **handle, *service_uuid;
	struct gatt_primary *prim;
	uuid_t uuid;
	char tmp[3];
	int i;

	sdp_uuid16_create(&uuid, GATT_PRIM_SVC_UUID);
	prim_uuid = bt_uuid2string(&uuid);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/attributes", local,
			peer);

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);
	groups = g_key_file_get_groups(key_file, NULL);

	for (handle = groups; *handle; handle++) {
		gboolean uuid_ok;
		int end;

		str = g_key_file_get_string(key_file, *handle, "UUID", NULL);
		if (!str)
			continue;

		uuid_ok = g_str_equal(str, prim_uuid);
		g_free(str);

		if (!uuid_ok)
			continue;

		str = g_key_file_get_string(key_file, *handle, "Value", NULL);
		if (!str)
			continue;

		end = g_key_file_get_integer(key_file, *handle,
						"EndGroupHandle", NULL);
		if (end == 0) {
			g_free(str);
			continue;
		}

		prim = g_new0(struct gatt_primary, 1);
		prim->range.start = atoi(*handle);
		prim->range.end = end;

		switch (strlen(str)) {
		case 4:
			uuid.type = SDP_UUID16;
			sscanf(str, "%04hx", &uuid.value.uuid16);
		break;
		case 8:
			uuid.type = SDP_UUID32;
			sscanf(str, "%08x", &uuid.value.uuid32);
			break;
		case 32:
			uuid.type = SDP_UUID128;
			memset(tmp, 0, sizeof(tmp));
			for (i = 0; i < 16; i++) {
				memcpy(tmp, str + (i * 2), 2);
				uuid.value.uuid128.data[i] =
						(uint8_t) strtol(tmp, NULL, 16);
			}
			break;
		default:
			g_free(str);
			g_free(prim);
			continue;
		}

		service_uuid = bt_uuid2string(&uuid);
		memcpy(prim->uuid, service_uuid, MAX_LEN_UUID_STR);
		free(service_uuid);
		g_free(str);

		device->primaries = g_slist_append(device->primaries, prim);
	}

	g_strfreev(groups);
	g_key_file_free(key_file);
	free(prim_uuid);
}

static void device_register_primaries(struct btd_device *device,
						GSList *prim_list, int psm)
{
	device->primaries = g_slist_concat(device->primaries, prim_list);
}

static void add_primary(struct gatt_db_attribute *attr, void *user_data)
{
	GSList **new_services = user_data;
	struct gatt_primary *prim;
	bt_uuid_t uuid;

	prim = g_new0(struct gatt_primary, 1);
	if (!prim) {
		DBG("Failed to allocate gatt_primary structure");
		return;
	}

	gatt_db_attribute_get_service_handles(attr, &prim->range.start,
							&prim->range.end);
	gatt_db_attribute_get_service_uuid(attr, &uuid);
	bt_uuid_to_string(&uuid, prim->uuid, sizeof(prim->uuid));

	*new_services = g_slist_append(*new_services, prim);
}

static void load_desc_value(struct gatt_db_attribute *attrib,
						int err, void *user_data)
{
	if (err)
		warn("loading descriptor value to db failed");
}

static int load_desc(char *handle, char *value,
					struct gatt_db_attribute *service)
{
	char uuid_str[MAX_LEN_UUID_STR];
	struct gatt_db_attribute *att;
	uint16_t handle_int;
	uint16_t val;
	bt_uuid_t uuid, ext_uuid;

	if (sscanf(handle, "%04hx", &handle_int) != 1)
		return -EIO;

	/* Check if there is any value stored, otherwise it is just the UUID */
	if (sscanf(value, "%04hx:%s", &val, uuid_str) != 2) {
		if (sscanf(value, "%s", uuid_str) != 1)
			return -EIO;
		val = 0;
	}

	DBG("loading descriptor handle: 0x%04x, value: 0x%04x, uuid: %s",
				handle_int, val, uuid_str);

	bt_string_to_uuid(&uuid, uuid_str);
	bt_uuid16_create(&ext_uuid, GATT_CHARAC_EXT_PROPER_UUID);

	/* If it is CEP then it must contain the value */
	if (!bt_uuid_cmp(&uuid, &ext_uuid) && !val) {
		warn("cannot load CEP descriptor without value");
		return -EIO;
	}

	att = gatt_db_service_insert_descriptor(service, handle_int, &uuid,
							0, NULL, NULL, NULL);
	if (!att || gatt_db_attribute_get_handle(att) != handle_int) {
		warn("loading descriptor to db failed");
		return -EIO;
	}

	if (val) {
		if (!gatt_db_attribute_write(att, 0, (uint8_t *)&val,
						sizeof(val), 0, NULL,
						load_desc_value, NULL))
			return -EIO;
	}

	return 0;
}

static int load_chrc(char *handle, char *value,
					struct gatt_db_attribute *service)
{
	uint16_t properties, value_handle, handle_int;
	char uuid_str[MAX_LEN_UUID_STR];
	struct gatt_db_attribute *att;
	bt_uuid_t uuid;

	if (sscanf(handle, "%04hx", &handle_int) != 1)
		return -EIO;

	if (sscanf(value, GATT_CHARAC_UUID_STR ":%04hx:%02hx:%s", &value_handle,
						&properties, uuid_str) != 3)
		return -EIO;

	bt_string_to_uuid(&uuid, uuid_str);

	/* Log debug message. */
	DBG("loading characteristic handle: 0x%04x, value handle: 0x%04x,"
				" properties 0x%04x uuid: %s", handle_int,
				value_handle, properties, uuid_str);

	att = gatt_db_service_insert_characteristic(service, value_handle,
							&uuid, 0, properties,
							NULL, NULL, NULL);
	if (!att || gatt_db_attribute_get_handle(att) != value_handle) {
		warn("loading characteristic to db failed");
		return -EIO;
	}

	return 0;
}

static int load_incl(struct gatt_db *db, char *handle, char *value,
					struct gatt_db_attribute *service)
{
	char uuid_str[MAX_LEN_UUID_STR];
	struct gatt_db_attribute *att;
	uint16_t start, end;

	if (sscanf(handle, "%04hx", &start) != 1)
		return -EIO;

	if (sscanf(value, GATT_INCLUDE_UUID_STR ":%04hx:%04hx:%s", &start, &end,
								uuid_str) != 3)
		return -EIO;

	/* Log debug message. */
	DBG("loading included service: 0x%04x, end: 0x%04x, uuid: %s", start,
								end, uuid_str);

	att = gatt_db_get_attribute(db, start);
	if (!att) {
		warn("loading included service to db failed - no such service");
		return -EIO;
	}

	att = gatt_db_service_add_included(service, att);
	if (!att) {
		warn("loading included service to db failed");
		return -EIO;
	}

	return 0;
}

static int load_service(struct gatt_db *db, char *handle, char *value)
{
	struct gatt_db_attribute *att;
	uint16_t start, end;
	char type[MAX_LEN_UUID_STR], uuid_str[MAX_LEN_UUID_STR];
	bt_uuid_t uuid;
	bool primary;

	if (sscanf(handle, "%04hx", &start) != 1)
		return -EIO;

	if (sscanf(value, "%[^:]:%04hx:%s", type, &end, uuid_str) != 3)
		return -EIO;

	if (g_str_equal(type, GATT_PRIM_SVC_UUID_STR))
		primary = true;
	else if (g_str_equal(type, GATT_SND_SVC_UUID_STR))
		primary = false;
	else
		return -EIO;

	bt_string_to_uuid(&uuid, uuid_str);

	/* Log debug message. */
	DBG("loading service: 0x%04x, end: 0x%04x, uuid: %s",
							start, end, uuid_str);

	att = gatt_db_insert_service(db, start, &uuid, primary,
							end - start + 1);
	if (!att) {
		error("Unable load service into db!");
		return -EIO;
	}

	return 0;
}

static int load_gatt_db_impl(GKeyFile *key_file, char **keys,
							struct gatt_db *db)
{
	struct gatt_db_attribute *current_service;
	char **handle, *value, type[MAX_LEN_UUID_STR];
	int ret;

	/* first load service definitions */
	for (handle = keys; *handle; handle++) {
		value = g_key_file_get_string(key_file, "Attributes", *handle,
									NULL);

		if (sscanf(value, "%[^:]:", type) != 1) {
			warn("Missing Type in attribute definition");
			g_free(value);
			return -EIO;
		}

		if (g_str_equal(type, GATT_PRIM_SVC_UUID_STR) ||
				g_str_equal(type, GATT_SND_SVC_UUID_STR)) {
			ret = load_service(db, *handle, value);
			if (ret) {
				g_free(value);
				return ret;
			}
		}

		g_free(value);
	}

	current_service = NULL;
	/* then fill them with data*/
	for (handle = keys; *handle; handle++) {
		value = g_key_file_get_string(key_file, "Attributes", *handle,
									NULL);

		if (sscanf(value, "%[^:]:", type) != 1) {
			warn("Missing Type in attribute definition");
			g_free(value);
			return -EIO;
		}

		if (g_str_equal(type, GATT_PRIM_SVC_UUID_STR) ||
				g_str_equal(type, GATT_SND_SVC_UUID_STR)) {
			uint16_t tmp;
			uint16_t start, end;
			bool primary;
			bt_uuid_t uuid;
			char uuid_str[MAX_LEN_UUID_STR];

			if (sscanf(*handle, "%04hx", &tmp) != 1) {
				warn("Unable to parse attribute handle");
				g_free(value);
				return -EIO;
			}

			if (current_service)
				gatt_db_service_set_active(current_service,
									true);

			current_service = gatt_db_get_attribute(db, tmp);

			gatt_db_attribute_get_service_data(current_service,
							&start, &end,
							&primary, &uuid);

			bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));
		} else if (g_str_equal(type, GATT_INCLUDE_UUID_STR)) {
			ret = load_incl(db, *handle, value, current_service);
		} else if (g_str_equal(type, GATT_CHARAC_UUID_STR)) {
			ret = load_chrc(*handle, value, current_service);
		} else {
			ret = load_desc(*handle, value, current_service);
		}

		g_free(value);
		if (ret) {
			gatt_db_clear(db);
			return ret;
		}
	}

	if (current_service)
		gatt_db_service_set_active(current_service, true);

	return 0;
}

static void load_gatt_db(struct btd_device *device, const char *local,
							const char *peer)
{
	char **keys, filename[PATH_MAX];
	GKeyFile *key_file;

	DBG("Restoring %s gatt database from file", peer);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/cache/%s", local, peer);

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);
	keys = g_key_file_get_keys(key_file, "Attributes", NULL, NULL);

	if (!keys) {
		warn("No cache for %s", peer);
		g_key_file_free(key_file);
		return;
	}

	if (load_gatt_db_impl(key_file, keys, device->db))
		warn("Unable to load gatt db from file for %s", peer);

	g_strfreev(keys);
	g_key_file_free(key_file);

	g_slist_free_full(device->primaries, g_free);
	device->primaries = NULL;
	gatt_db_foreach_service(device->db, NULL, add_primary,
							&device->primaries);
}

static void device_add_uuids(struct btd_device *device, GSList *uuids)
{
	GSList *l;
	bool changed = false;

	for (l = uuids; l != NULL; l = g_slist_next(l)) {
		GSList *match = g_slist_find_custom(device->uuids, l->data,
							bt_uuid_strcmp);
		if (match)
			continue;

		changed = true;
		device->uuids = g_slist_insert_sorted(device->uuids,
						g_strdup(l->data),
						bt_uuid_strcmp);
	}

	if (changed)
		g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "UUIDs");
}

static bool device_match_profile(struct btd_device *device,
					struct btd_profile *profile,
					GSList *uuids)
{
	if (profile->remote_uuid == NULL)
		return false;

	if (g_slist_find_custom(uuids, profile->remote_uuid,
							bt_uuid_strcmp) == NULL)
		return false;

	return true;
}

static void add_gatt_service(struct gatt_db_attribute *attr, void *user_data)
{
	struct btd_device *device = user_data;
	struct btd_service *service;
	struct btd_profile *profile;
	bt_uuid_t uuid;
	char uuid_str[MAX_LEN_UUID_STR];
	GSList *l;

	gatt_db_attribute_get_service_uuid(attr, &uuid);
	bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));

	/* Check if service was already probed */
	l = find_service_with_uuid(device->services, uuid_str);
	if (l)
		goto done;

	/* Add UUID and probe service */
	btd_device_add_uuid(device, uuid_str);

	/* Check if service was probed */
	l = find_service_with_uuid(device->services, uuid_str);
	if (!l)
		return;

done:
	/* Mark service as active to skip discovering it again */
	gatt_db_service_set_active(attr, true);

	service = l->data;
	profile = btd_service_get_profile(service);

	/* Claim attributes of internal profiles */
	if (!profile->external) {
		/* Mark the service as claimed by the existing profile. */
		gatt_db_service_set_claimed(attr, true);
	}

	/* Notify driver about the new connection */
	service_accept(service);
}

static void device_add_gatt_services(struct btd_device *device)
{
	char addr[18];

	ba2str(&device->bdaddr, addr);

	if (device->blocked) {
		DBG("Skipping profiles for blocked device %s", addr);
		return;
	}

	gatt_db_foreach_service(device->db, NULL, add_gatt_service, device);
}

static void device_accept_gatt_profiles(struct btd_device *device)
{
	GSList *l;

	for (l = device->services; l != NULL; l = g_slist_next(l))
		service_accept(l->data);
}

static void device_remove_gatt_service(struct btd_device *device,
						struct gatt_db_attribute *attr)
{
	struct btd_service *service;
	bt_uuid_t uuid;
	char uuid_str[MAX_LEN_UUID_STR];
	GSList *l;

	gatt_db_attribute_get_service_uuid(attr, &uuid);
	bt_uuid_to_string(&uuid, uuid_str, sizeof(uuid_str));

	l = find_service_with_uuid(device->services, uuid_str);
	if (!l)
		return;

	service = l->data;
	device->services = g_slist_delete_link(device->services, l);
	device->pending = g_slist_remove(device->pending, service);
	service_remove(service);
}

static gboolean gatt_services_changed(gpointer user_data)
{
	struct btd_device *device = user_data;

	store_gatt_db(device);

	return FALSE;
}

static void gatt_service_added(struct gatt_db_attribute *attr, void *user_data)
{
	struct btd_device *device = user_data;
	GSList *new_service = NULL;
	uint16_t start, end;

	if (!bt_gatt_client_is_ready(device->client))
		return;

	gatt_db_attribute_get_service_data(attr, &start, &end, NULL, NULL);

	DBG("start: 0x%04x, end: 0x%04x", start, end);

	/*
	 * TODO: Remove the primaries list entirely once all profiles use
	 * shared/gatt.
	 */
	add_primary(attr, &new_service);
	if (!new_service)
		return;

	device_register_primaries(device, new_service, -1);

	add_gatt_service(attr, device);

	btd_gatt_client_service_added(device->client_dbus, attr);

	gatt_services_changed(device);
}

static gint prim_attr_cmp(gconstpointer a, gconstpointer b)
{
	const struct gatt_primary *prim = a;
	const struct gatt_db_attribute *attr = b;
	uint16_t start, end;

	gatt_db_attribute_get_service_handles(attr, &start, &end);

	return !(prim->range.start == start && prim->range.end == end);
}

static gint prim_uuid_cmp(gconstpointer a, gconstpointer b)
{
	const struct gatt_primary *prim = a;
	const char *uuid = b;

	return bt_uuid_strcmp(prim->uuid, uuid);
}

static void gatt_service_removed(struct gatt_db_attribute *attr,
								void *user_data)
{
	struct btd_device *device = user_data;
	GSList *l;
	struct gatt_primary *prim;
	uint16_t start, end;

	/*
	 * NOTE: shared/gatt-client clears the database in case of failure. This
	 * triggers the service_removed callback for all affected services.
	 * Hence, this function will be called in the following cases:
	 *
	 *    1. When a GATT service gets removed due to "Service Changed".
	 *
	 *    2. When a GATT service gets removed when the database get cleared
	 *       upon disconnection with a non-bonded device.
	 *
	 *    3. When a GATT service gets removed when the database get cleared
	 *       by shared/gatt-client when its initialization procedure fails,
	 *       e.g. due to an ATT protocol error or an unexpected disconnect.
	 *       In this case the gatt-client will not be ready.
	 */

	gatt_db_attribute_get_service_handles(attr, &start, &end);

	DBG("start: 0x%04x, end: 0x%04x", start, end);

	/* Remove the corresponding gatt_primary */
	l = g_slist_find_custom(device->primaries, attr, prim_attr_cmp);
	if (!l)
		return;

	prim = l->data;
	device->primaries = g_slist_delete_link(device->primaries, l);

	/*
	 * Remove the corresponding UUIDs entry and profile, only if this is
	 * the last service with this UUID.
	 */
	l = g_slist_find_custom(device->uuids, prim->uuid, bt_uuid_strcmp);

	if (l && !g_slist_find_custom(device->primaries, prim->uuid,
							prim_uuid_cmp)) {
		/*
		 * If this happend since the db was cleared for a non-bonded
		 * device, then don't remove the btd_service just yet. We do
		 * this so that we can avoid re-probing the profile if the same
		 * GATT service is found on the device on re-connection.
		 * However, if the device is marked as temporary, then we
		 * remove it anyway.
		 */
		if (device->client || device->temporary == TRUE)
			device_remove_gatt_service(device, attr);

		g_free(l->data);
		device->uuids = g_slist_delete_link(device->uuids, l);
		g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "UUIDs");
	}

	g_free(prim);

	store_device_info(device);

	btd_gatt_client_service_removed(device->client_dbus, attr);

	gatt_services_changed(device);
}

static struct btd_device *device_new(struct btd_adapter *adapter,
				const char *address)
{
	char *address_up;
	struct btd_device *device;
	const char *adapter_path = adapter_get_path(adapter);

	DBG("address %s", address);

	device = g_try_malloc0(sizeof(struct btd_device));
	if (device == NULL)
		return NULL;

	device->tx_power = 127;

	device->db = gatt_db_new();
	if (!device->db) {
		g_free(device);
		return NULL;
	}

	device->ad = bt_ad_new();
	if (!device->ad) {
		device_free(device);
		return NULL;
	}

	address_up = g_ascii_strup(address, -1);
	device->path = g_strdup_printf("%s/dev_%s", adapter_path, address_up);
	g_strdelimit(device->path, ":", '_');
	g_free(address_up);

	str2ba(address, &device->bdaddr);

	device->client_dbus = btd_gatt_client_new(device);
	if (!device->client_dbus) {
		error("Failed to create btd_gatt_client");
		device_free(device);
		return NULL;
	}

	DBG("Creating device %s", device->path);

	if (g_dbus_register_interface(dbus_conn,
					device->path, DEVICE_INTERFACE,
					device_methods, NULL,
					device_properties, device,
					device_free) == FALSE) {
		error("Unable to register device interface for %s", address);
		device_free(device);
		return NULL;
	}

	device->adapter = adapter;
	device->temporary = true;

	gatt_db_register(device->db, gatt_service_added, gatt_service_removed,
								device, NULL);

	return btd_device_ref(device);
}

struct btd_device *device_create_from_storage(struct btd_adapter *adapter,
				const char *address, GKeyFile *key_file)
{
	struct btd_device *device;
	const bdaddr_t *src;
	char srcaddr[18];

	DBG("address %s", address);

	device = device_new(adapter, address);
	if (device == NULL)
		return NULL;

	src = btd_adapter_get_address(adapter);
	ba2str(src, srcaddr);

	convert_info(device, key_file);

	load_info(device, srcaddr, address, key_file);
	load_att_info(device, srcaddr, address);

	return device;
}

struct btd_device *device_create(struct btd_adapter *adapter,
				const bdaddr_t *bdaddr, uint8_t bdaddr_type)
{
	struct btd_device *device;
	const bdaddr_t *sba;
	char src[18], dst[18];
	char *str;

	ba2str(bdaddr, dst);
	DBG("dst %s", dst);

	device = device_new(adapter, dst);
	if (device == NULL)
		return NULL;

	device->bdaddr_type = bdaddr_type;

	if (bdaddr_type == BDADDR_BREDR)
		device->bredr = true;
	else
		device->le = true;

	sba = btd_adapter_get_address(adapter);
	ba2str(sba, src);

	str = load_cached_name(device, src, dst);
	if (str) {
		strcpy(device->name, str);
		g_free(str);
	}

	return device;
}

char *btd_device_get_storage_path(struct btd_device *device,
				const char *filename)
{
	char srcaddr[18], dstaddr[18];

	if (device_address_is_private(device)) {
		warn("Refusing storage path for private addressed device %s",
								device->path);
		return NULL;
	}

	ba2str(btd_adapter_get_address(device->adapter), srcaddr);
	ba2str(&device->bdaddr, dstaddr);

	if (!filename)
		return g_strdup_printf(STORAGEDIR "/%s/%s", srcaddr, dstaddr);

	return g_strdup_printf(STORAGEDIR "/%s/%s/%s", srcaddr, dstaddr,
							filename);
}

void btd_device_device_set_name(struct btd_device *device, const char *name)
{
	if (strncmp(name, device->name, MAX_NAME_LENGTH) == 0)
		return;

	DBG("%s %s", device->path, name);

	strncpy(device->name, name, MAX_NAME_LENGTH);

	store_device_info(device);

	g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "Name");

	if (device->alias != NULL)
		return;

	g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "Alias");
}

void device_get_name(struct btd_device *device, char *name, size_t len)
{
	if (name != NULL && len > 0) {
		strncpy(name, device->name, len - 1);
		name[len - 1] = '\0';
	}
}

bool device_name_known(struct btd_device *device)
{
	return device->name[0] != '\0';
}

void device_set_class(struct btd_device *device, uint32_t class)
{
	if (device->class == class)
		return;

	DBG("%s 0x%06X", device->path, class);

	device->class = class;

	store_device_info(device);

	g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "Class");
	g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "Icon");
}

void device_update_addr(struct btd_device *device, const bdaddr_t *bdaddr,
							uint8_t bdaddr_type)
{
	if (!bacmp(bdaddr, &device->bdaddr) &&
					bdaddr_type == device->bdaddr_type)
		return;

	/* Since this function is only used for LE SMP Identity
	 * Resolving purposes we can now assume LE is supported.
	 */
	device->le = true;

	bacpy(&device->bdaddr, bdaddr);
	device->bdaddr_type = bdaddr_type;

	store_device_info(device);

	g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "Address");
}

void device_set_bredr_support(struct btd_device *device)
{
	if (device->bredr)
		return;

	device->bredr = true;
	store_device_info(device);
}

void device_set_le_support(struct btd_device *device, uint8_t bdaddr_type)
{
	if (device->le)
		return;

	device->le = true;
	device->bdaddr_type = bdaddr_type;

	store_device_info(device);
}

void device_update_last_seen(struct btd_device *device, uint8_t bdaddr_type)
{
	if (bdaddr_type == BDADDR_BREDR)
		device->bredr_seen = time(NULL);
	else
		device->le_seen = time(NULL);
}

/* It is possible that we have two device objects for the same device in
 * case it has first been discovered over BR/EDR and has a private
 * address when discovered over LE for the first time. In such a case we
 * need to inherit critical values from the duplicate so that we don't
 * ovewrite them when writing to storage. The next time bluetoothd
 * starts the device will show up as a single instance.
 */
void device_merge_duplicate(struct btd_device *dev, struct btd_device *dup)
{
	GSList *l;

	DBG("");

	dev->bredr = dup->bredr;

	dev->trusted = dup->trusted;
	dev->blocked = dup->blocked;

	for (l = dup->uuids; l; l = g_slist_next(l))
		dev->uuids = g_slist_append(dev->uuids, g_strdup(l->data));

	if (dev->name[0] == '\0')
		strcpy(dev->name, dup->name);

	if (!dev->alias)
		dev->alias = g_strdup(dup->alias);

	dev->class = dup->class;

	dev->vendor_src = dup->vendor_src;
	dev->vendor = dup->vendor;
	dev->product = dup->product;
	dev->version = dup->version;
}

uint32_t btd_device_get_class(struct btd_device *device)
{
	return device->class;
}

uint16_t btd_device_get_vendor(struct btd_device *device)
{
	return device->vendor;
}

uint16_t btd_device_get_vendor_src(struct btd_device *device)
{
	return device->vendor_src;
}

uint16_t btd_device_get_product(struct btd_device *device)
{
	return device->product;
}

uint16_t btd_device_get_version(struct btd_device *device)
{
	return device->version;
}

static void delete_folder_tree(const char *dirname)
{
	DIR *dir;
	struct dirent *entry;
	char filename[PATH_MAX];

	dir = opendir(dirname);
	if (dir == NULL)
		return;

	while ((entry = readdir(dir)) != NULL) {
		if (g_str_equal(entry->d_name, ".") ||
				g_str_equal(entry->d_name, ".."))
			continue;

		if (entry->d_type == DT_UNKNOWN)
			entry->d_type = util_get_dt(dirname, entry->d_name);

		snprintf(filename, PATH_MAX, "%s/%s", dirname, entry->d_name);

		if (entry->d_type == DT_DIR)
			delete_folder_tree(filename);
		else
			unlink(filename);
	}
	closedir(dir);

	rmdir(dirname);
}

static void device_remove_stored(struct btd_device *device)
{
	const bdaddr_t *src = btd_adapter_get_address(device->adapter);
	char adapter_addr[18];
	char device_addr[18];
	char filename[PATH_MAX];
	GKeyFile *key_file;
	char *data;
	gsize length = 0;

	if (device->bredr_state.bonded) {
		device->bredr_state.bonded = false;
		btd_adapter_remove_bonding(device->adapter, &device->bdaddr,
								BDADDR_BREDR);
	}

	if (device->le_state.bonded) {
		device->le_state.bonded = false;
		btd_adapter_remove_bonding(device->adapter, &device->bdaddr,
							device->bdaddr_type);
	}

	device->bredr_state.paired = false;
	device->le_state.paired = false;

	if (device->blocked)
		device_unblock(device, TRUE, FALSE);

	ba2str(src, adapter_addr);
	ba2str(&device->bdaddr, device_addr);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s", adapter_addr,
			device_addr);
	delete_folder_tree(filename);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/cache/%s", adapter_addr,
			device_addr);

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);
	g_key_file_remove_group(key_file, "ServiceRecords", NULL);

	data = g_key_file_to_data(key_file, &length, NULL);
	if (length > 0) {
		create_file(filename, S_IRUSR | S_IWUSR);
		g_file_set_contents(filename, data, length, NULL);
	}

	g_free(data);
	g_key_file_free(key_file);
}

void device_remove(struct btd_device *device, gboolean remove_stored)
{
	DBG("Removing device %s", device->path);

	if (device->bonding) {
		uint8_t status;

		if (device->bredr_state.connected)
			status = MGMT_STATUS_DISCONNECTED;
		else
			status = MGMT_STATUS_CONNECT_FAILED;

		device_cancel_bonding(device, status);
	}

	if (device->browse)
		browse_request_cancel(device->browse);

	while (device->services != NULL) {
		struct btd_service *service = device->services->data;

		device->services = g_slist_remove(device->services, service);
		service_remove(service);
	}

	g_slist_free(device->pending);
	device->pending = NULL;

	if (btd_device_is_connected(device)) {
		g_source_remove(device->disconn_timer);
		disconnect_all(device);
	}

	if (device->store_id > 0) {
		g_source_remove(device->store_id);
		device->store_id = 0;

		if (!remove_stored)
			store_device_info_cb(device);
	}

	if (remove_stored)
		device_remove_stored(device);

	btd_device_unref(device);
}

int device_address_cmp(gconstpointer a, gconstpointer b)
{
	const struct btd_device *device = a;
	const char *address = b;
	char addr[18];

	ba2str(&device->bdaddr, addr);
	return strcasecmp(addr, address);
}

int device_bdaddr_cmp(gconstpointer a, gconstpointer b)
{
	const struct btd_device *device = a;
	const bdaddr_t *bdaddr = b;

	return bacmp(&device->bdaddr, bdaddr);
}

static bool addr_is_public(uint8_t addr_type)
{
	if (addr_type == BDADDR_BREDR || addr_type == BDADDR_LE_PUBLIC)
		return true;

	return false;
}

int device_addr_type_cmp(gconstpointer a, gconstpointer b)
{
	const struct btd_device *dev = a;
	const struct device_addr_type *addr = b;
	int cmp;

	cmp = bacmp(&dev->bdaddr, &addr->bdaddr);

	/*
	 * Address matches and both old and new are public addresses
	 * (doesn't matter whether LE or BR/EDR, then consider this a
	 * match.
	 */
	if (!cmp 
#ifndef RTCONFIG_LANTIQ
		&& addr_is_public(addr->bdaddr_type) && addr_is_public(dev->bdaddr_type)
#endif
	)
		return 0;

	if (addr->bdaddr_type == BDADDR_BREDR) {
		if (!dev->bredr)
			return -1;

		return cmp;
	}

	if (!dev->le)
		return -1;

	if (addr->bdaddr_type != dev->bdaddr_type)
		return -1;

	return cmp;
}

static gboolean record_has_uuid(const sdp_record_t *rec,
				const char *profile_uuid)
{
	sdp_list_t *pat;

	for (pat = rec->pattern; pat != NULL; pat = pat->next) {
		char *uuid;
		int ret;

		uuid = bt_uuid2string(pat->data);
		if (!uuid)
			continue;

		ret = strcasecmp(uuid, profile_uuid);

		free(uuid);

		if (ret == 0)
			return TRUE;
	}

	return FALSE;
}

GSList *btd_device_get_uuids(struct btd_device *device)
{
	return device->uuids;
}

struct probe_data {
	struct btd_device *dev;
	GSList *uuids;
};

static struct btd_service *probe_service(struct btd_device *device,
						struct btd_profile *profile,
						GSList *uuids)
{
	struct btd_service *service;

	if (profile->device_probe == NULL)
		return NULL;

	if (!device_match_profile(device, profile, uuids))
		return NULL;

	service = service_create(device, profile);

	if (service_probe(service)) {
		btd_service_unref(service);
		return NULL;
	}

	if (profile->auto_connect)
		device_set_auto_connect(device, TRUE);

	return service;
}

static void dev_probe(struct btd_profile *p, void *user_data)
{
	struct probe_data *d = user_data;
	struct btd_service *service;

	service = probe_service(d->dev, p, d->uuids);
	if (!service)
		return;

	d->dev->services = g_slist_append(d->dev->services, service);
}

void device_probe_profile(gpointer a, gpointer b)
{
	struct btd_device *device = a;
	struct btd_profile *profile = b;
	struct btd_service *service;

	service = probe_service(device, profile, device->uuids);
	if (!service)
		return;

	device->services = g_slist_append(device->services, service);

	if (!profile->auto_connect || !device->general_connect)
		return;

	device->pending = g_slist_append(device->pending, service);

	if (g_slist_length(device->pending) == 1)
		connect_next(device);
}

void device_remove_profile(gpointer a, gpointer b)
{
	struct btd_device *device = a;
	struct btd_profile *profile = b;
	struct btd_service *service;
	GSList *l;

	l = find_service_with_profile(device->services, profile);
	if (l == NULL)
		return;

	service = l->data;
	device->services = g_slist_delete_link(device->services, l);
	device->pending = g_slist_remove(device->pending, service);
	service_remove(service);
}

void device_probe_profiles(struct btd_device *device, GSList *uuids)
{
	struct probe_data d = { device, uuids };
	char addr[18];

	ba2str(&device->bdaddr, addr);

	if (device->blocked) {
		DBG("Skipping profiles for blocked device %s", addr);
		goto add_uuids;
	}

	DBG("Probing profiles for device %s", addr);

	btd_profile_foreach(dev_probe, &d);

add_uuids:
	device_add_uuids(device, uuids);
}

static void store_sdp_record(GKeyFile *key_file, sdp_record_t *rec)
{
	char handle_str[11];
	sdp_buf_t buf;
	int size, i;
	char *str;

	sprintf(handle_str, "0x%8.8X", rec->handle);

	if (sdp_gen_record_pdu(rec, &buf) < 0)
		return;

	size = buf.data_size;

	str = g_malloc0(size*2+1);

	for (i = 0; i < size; i++)
		sprintf(str + (i * 2), "%02X", buf.data[i]);

	g_key_file_set_string(key_file, "ServiceRecords", handle_str, str);

	free(buf.data);
	g_free(str);
}

static void store_primaries_from_sdp_record(GKeyFile *key_file,
						sdp_record_t *rec)
{
	uuid_t uuid;
	char *att_uuid, *prim_uuid;
	uint16_t start = 0, end = 0, psm = 0;
	char handle[6], uuid_str[33];
	int i;

	sdp_uuid16_create(&uuid, ATT_UUID);
	att_uuid = bt_uuid2string(&uuid);

	sdp_uuid16_create(&uuid, GATT_PRIM_SVC_UUID);
	prim_uuid = bt_uuid2string(&uuid);

	if (!record_has_uuid(rec, att_uuid))
		goto done;

	if (!gatt_parse_record(rec, &uuid, &psm, &start, &end))
		goto done;

	sprintf(handle, "%hu", start);
	switch (uuid.type) {
	case SDP_UUID16:
		sprintf(uuid_str, "%4.4X", uuid.value.uuid16);
		break;
	case SDP_UUID32:
		sprintf(uuid_str, "%8.8X", uuid.value.uuid32);
		break;
	case SDP_UUID128:
		for (i = 0; i < 16; i++)
			sprintf(uuid_str + (i * 2), "%2.2X",
					uuid.value.uuid128.data[i]);
		break;
	default:
		uuid_str[0] = '\0';
	}

	g_key_file_set_string(key_file, handle, "UUID", prim_uuid);
	g_key_file_set_string(key_file, handle, "Value", uuid_str);
	g_key_file_set_integer(key_file, handle, "EndGroupHandle", end);

done:
	free(prim_uuid);
	free(att_uuid);
}

static int rec_cmp(const void *a, const void *b)
{
	const sdp_record_t *r1 = a;
	const sdp_record_t *r2 = b;

	return r1->handle - r2->handle;
}

static int update_record(struct browse_req *req, const char *uuid,
							sdp_record_t *rec)
{
	GSList *l;

	/* Check for duplicates */
	if (sdp_list_find(req->records, rec, rec_cmp))
		return -EALREADY;

	/* Copy record */
	req->records = sdp_list_append(req->records, sdp_copy_record(rec));

	/* Check if UUID is duplicated */
	l = g_slist_find_custom(req->device->uuids, uuid, bt_uuid_strcmp);
	if (l == NULL) {
		l = g_slist_find_custom(req->profiles_added, uuid,
							bt_uuid_strcmp);
		if (l != NULL)
			return 0;
		req->profiles_added = g_slist_append(req->profiles_added,
							g_strdup(uuid));
	}

	return 0;
}

static void update_bredr_services(struct browse_req *req, sdp_list_t *recs)
{
	struct btd_device *device = req->device;
	sdp_list_t *seq;
	char srcaddr[18], dstaddr[18];
	char sdp_file[PATH_MAX];
	char att_file[PATH_MAX];
	GKeyFile *sdp_key_file;
	GKeyFile *att_key_file;
	char *data;
	gsize length = 0;

	ba2str(btd_adapter_get_address(device->adapter), srcaddr);
	ba2str(&device->bdaddr, dstaddr);

	snprintf(sdp_file, PATH_MAX, STORAGEDIR "/%s/cache/%s", srcaddr,
								dstaddr);

	sdp_key_file = g_key_file_new();
	g_key_file_load_from_file(sdp_key_file, sdp_file, 0, NULL);

	snprintf(att_file, PATH_MAX, STORAGEDIR "/%s/%s/attributes", srcaddr,
								dstaddr);

	att_key_file = g_key_file_new();
	g_key_file_load_from_file(att_key_file, att_file, 0, NULL);

	for (seq = recs; seq; seq = seq->next) {
		sdp_record_t *rec = (sdp_record_t *) seq->data;
		sdp_list_t *svcclass = NULL;
		char *profile_uuid;

		if (!rec)
			break;

		if (sdp_get_service_classes(rec, &svcclass) < 0)
			continue;

		/* Check for empty service classes list */
		if (svcclass == NULL) {
			DBG("Skipping record with no service classes");
			continue;
		}

		/* Extract the first element and skip the remainning */
		profile_uuid = bt_uuid2string(svcclass->data);
		if (!profile_uuid) {
			sdp_list_free(svcclass, free);
			continue;
		}

		if (bt_uuid_strcmp(profile_uuid, PNP_UUID) == 0) {
			uint16_t source, vendor, product, version;
			sdp_data_t *pdlist;

			pdlist = sdp_data_get(rec, SDP_ATTR_VENDOR_ID_SOURCE);
			source = pdlist ? pdlist->val.uint16 : 0x0000;

			pdlist = sdp_data_get(rec, SDP_ATTR_VENDOR_ID);
			vendor = pdlist ? pdlist->val.uint16 : 0x0000;

			pdlist = sdp_data_get(rec, SDP_ATTR_PRODUCT_ID);
			product = pdlist ? pdlist->val.uint16 : 0x0000;

			pdlist = sdp_data_get(rec, SDP_ATTR_VERSION);
			version = pdlist ? pdlist->val.uint16 : 0x0000;

			if (source || vendor || product || version)
				btd_device_set_pnpid(device, source, vendor,
							product, version);
		}

		if (update_record(req, profile_uuid, rec) < 0)
			goto next;

		if (sdp_key_file)
			store_sdp_record(sdp_key_file, rec);

		if (att_key_file)
			store_primaries_from_sdp_record(att_key_file, rec);

next:
		free(profile_uuid);
		sdp_list_free(svcclass, free);
	}

	if (sdp_key_file) {
		data = g_key_file_to_data(sdp_key_file, &length, NULL);
		if (length > 0) {
			create_file(sdp_file, S_IRUSR | S_IWUSR);
			g_file_set_contents(sdp_file, data, length, NULL);
		}

		g_free(data);
		g_key_file_free(sdp_key_file);
	}

	if (att_key_file) {
		data = g_key_file_to_data(att_key_file, &length, NULL);
		if (length > 0) {
			create_file(att_file, S_IRUSR | S_IWUSR);
			g_file_set_contents(att_file, data, length, NULL);
		}

		g_free(data);
		g_key_file_free(att_key_file);
	}
}

static int primary_cmp(gconstpointer a, gconstpointer b)
{
	return memcmp(a, b, sizeof(struct gatt_primary));
}

static void update_gatt_uuids(struct browse_req *req, GSList *current,
								GSList *found)
{
	GSList *l, *lmatch;

	/* Added Profiles */
	for (l = found; l; l = g_slist_next(l)) {
		struct gatt_primary *prim = l->data;

		/* Entry found ? */
		lmatch = g_slist_find_custom(current, prim, primary_cmp);
		if (lmatch)
			continue;

		/* New entry */
		req->profiles_added = g_slist_append(req->profiles_added,
							g_strdup(prim->uuid));

		DBG("UUID Added: %s", prim->uuid);
	}
}

static GSList *device_services_from_record(struct btd_device *device,
							GSList *profiles)
{
	GSList *l, *prim_list = NULL;
	char *att_uuid;
	uuid_t proto_uuid;

	sdp_uuid16_create(&proto_uuid, ATT_UUID);
	att_uuid = bt_uuid2string(&proto_uuid);

	for (l = profiles; l; l = l->next) {
		const char *profile_uuid = l->data;
		const sdp_record_t *rec;
		struct gatt_primary *prim;
		uint16_t start = 0, end = 0, psm = 0;
		uuid_t prim_uuid;

		rec = btd_device_get_record(device, profile_uuid);
		if (!rec)
			continue;

		if (!record_has_uuid(rec, att_uuid))
			continue;

		if (!gatt_parse_record(rec, &prim_uuid, &psm, &start, &end))
			continue;

		prim = g_new0(struct gatt_primary, 1);
		prim->range.start = start;
		prim->range.end = end;
		sdp_uuid2strn(&prim_uuid, prim->uuid, sizeof(prim->uuid));

		prim_list = g_slist_append(prim_list, prim);
	}

	free(att_uuid);

	return prim_list;
}

static void search_cb(sdp_list_t *recs, int err, gpointer user_data)
{
	struct browse_req *req = user_data;
	struct btd_device *device = req->device;
	GSList *primaries;
	char addr[18];

	ba2str(&device->bdaddr, addr);

	if (err < 0) {
		error("%s: error updating services: %s (%d)",
				addr, strerror(-err), -err);
		goto send_reply;
	}

	update_bredr_services(req, recs);

	if (device->tmp_records)
		sdp_list_free(device->tmp_records,
					(sdp_free_func_t) sdp_record_free);

	device->tmp_records = req->records;
	req->records = NULL;

	if (!req->profiles_added) {
		DBG("%s: No service update", addr);
		goto send_reply;
	}

	primaries = device_services_from_record(device, req->profiles_added);
	if (primaries)
		device_register_primaries(device, primaries, ATT_PSM);

	/*
	 * TODO: The btd_service instances for GATT services need to be
	 * initialized with the service handles. Eventually this code should
	 * perform ATT protocol service discovery over the ATT PSM to obtain
	 * the full list of services and populate a client-role gatt_db over
	 * BR/EDR.
	 */
	device_probe_profiles(device, req->profiles_added);

	/* Propagate services changes */
	g_dbus_emit_property_changed(dbus_conn, req->device->path,
						DEVICE_INTERFACE, "UUIDs");

send_reply:
	device_svc_resolved(device, BDADDR_BREDR, err);
}

static void browse_cb(sdp_list_t *recs, int err, gpointer user_data)
{
	struct browse_req *req = user_data;
	struct btd_device *device = req->device;
	struct btd_adapter *adapter = device->adapter;
	uuid_t uuid;

	/* If we have a valid response and req->search_uuid == 2, then L2CAP
	 * UUID & PNP searching was successful -- we are done */
	if (err < 0 || (req->search_uuid == 2 && req->records)) {
		if (err == -ECONNRESET && req->reconnect_attempt < 1) {
			req->search_uuid--;
			req->reconnect_attempt++;
		} else
			goto done;
	}

	update_bredr_services(req, recs);

	/* Search for mandatory uuids */
	if (uuid_list[req->search_uuid]) {
		sdp_uuid16_create(&uuid, uuid_list[req->search_uuid++]);
		bt_search_service(btd_adapter_get_address(adapter),
						&device->bdaddr, &uuid,
						browse_cb, user_data, NULL,
						req->sdp_flags);
		return;
	}

done:
	search_cb(recs, err, user_data);
}

static bool device_get_auto_connect(struct btd_device *device)
{
	if (device->disable_auto_connect)
		return false;

	return device->auto_connect;
}

static void attio_connected(gpointer data, gpointer user_data)
{
	struct attio_data *attio = data;
	GAttrib *attrib = user_data;

	DBG("");

	if (attio->cfunc)
		attio->cfunc(attrib, attio->user_data);
}

static void attio_disconnected(gpointer data, gpointer user_data)
{
	struct attio_data *attio = data;

	DBG("");

	if (attio->dcfunc)
		attio->dcfunc(attio->user_data);
}

static void att_disconnected_cb(int err, void *user_data)
{
	struct btd_device *device = user_data;

	DBG("");

	if (device->browse)
		goto done;

	DBG("%s (%d)", strerror(err), err);

	g_slist_foreach(device->attios, attio_disconnected, NULL);

	btd_gatt_client_disconnected(device->client_dbus);

	if (!device_get_auto_connect(device)) {
		DBG("Automatic connection disabled");
		goto done;
	}

	/*
	 * Keep scanning/re-connection active if disconnection reason
	 * is connection timeout, remote user terminated connection or local
	 * initiated disconnection.
	 */
	if (err == ETIMEDOUT || err == ECONNRESET || err == ECONNABORTED)
		adapter_connect_list_add(device->adapter, device);

done:
	attio_cleanup(device);
}

static void register_gatt_services(struct btd_device *device)
{
	struct browse_req *req = device->browse;
	GSList *services = NULL;

	if (!bt_gatt_client_is_ready(device->client))
		return;

	/*
	 * TODO: Remove the primaries list entirely once all profiles use
	 * shared/gatt.
	 */
	gatt_db_foreach_service(device->db, NULL, add_primary, &services);

	btd_device_set_temporary(device, false);

	if (req)
		update_gatt_uuids(req, device->primaries, services);

	g_slist_free_full(device->primaries, g_free);
	device->primaries = NULL;

	device_register_primaries(device, services, -1);

	device_add_gatt_services(device);
}

static void gatt_client_init(struct btd_device *device);

static void gatt_client_ready_cb(bool success, uint8_t att_ecode,
								void *user_data)
{
	struct btd_device *device = user_data;

	DBG("status: %s, error: %u", success ? "success" : "failed", att_ecode);

	if (!success) {
		device_svc_resolved(device, device->bdaddr_type, -EIO);
		return;
	}

	register_gatt_services(device);

	btd_gatt_client_ready(device->client_dbus);

	device_svc_resolved(device, device->bdaddr_type, 0);

	store_gatt_db(device);
}

static void gatt_client_service_changed(uint16_t start_handle,
							uint16_t end_handle,
							void *user_data)
{
	DBG("start 0x%04x, end: 0x%04x", start_handle, end_handle);
}

static void gatt_debug(const char *str, void *user_data)
{
	DBG("%s", str);
}

static void gatt_client_init(struct btd_device *device)
{
	gatt_client_cleanup(device);

	device->client = bt_gatt_client_new(device->db, device->att,
							device->att_mtu);
	if (!device->client) {
		DBG("Failed to initialize");
		return;
	}

	bt_gatt_client_set_debug(device->client, gatt_debug, NULL, NULL);

	/* Notify attio so it can react to notifications */
	g_slist_foreach(device->attios, attio_connected, device->attrib);

	/*
	 * Notify notify existing service about the new connection so they can
	 * react to notifications while discovering services
	 */
	device_accept_gatt_profiles(device);

	if (!bt_gatt_client_set_ready_handler(device->client,
							gatt_client_ready_cb,
							device, NULL)) {
		DBG("Failed to set ready handler");
		gatt_client_cleanup(device);
		return;
	}

	if (!bt_gatt_client_set_service_changed(device->client,
						gatt_client_service_changed,
						device, NULL)) {
		DBG("Failed to set service changed handler");
		gatt_client_cleanup(device);
		return;
	}

	btd_gatt_client_connected(device->client_dbus);
}

static void gatt_server_init(struct btd_device *device, struct gatt_db *db)
{
	if (!db) {
		error("No local GATT database exists for this adapter");
		return;
	}

	gatt_server_cleanup(device);

	device->server = bt_gatt_server_new(db, device->att, device->att_mtu);
	if (!device->server)
		error("Failed to initialize bt_gatt_server");

	bt_gatt_server_set_debug(device->server, gatt_debug, NULL, NULL);
}

static bool local_counter(uint32_t *sign_cnt, void *user_data)
{
	struct btd_device *dev = user_data;

	if (!dev->local_csrk)
		return false;

	*sign_cnt = dev->local_csrk->counter++;

	store_device_info(dev);

	return true;
}

static bool remote_counter(uint32_t *sign_cnt, void *user_data)
{
	struct btd_device *dev = user_data;

	if (!dev->remote_csrk || *sign_cnt < dev->remote_csrk->counter)
		return false;

	dev->remote_csrk->counter = *sign_cnt;

	store_device_info(dev);

	return true;
}

bool device_attach_att(struct btd_device *dev, GIOChannel *io)
{
	GError *gerr = NULL;
	GAttrib *attrib;
	BtIOSecLevel sec_level;
	uint16_t mtu;
	uint16_t cid;
	struct btd_gatt_database *database;
	const bdaddr_t *src, *dst;
	char srcaddr[18], dstaddr[18];

	bt_io_get(io, &gerr, BT_IO_OPT_SEC_LEVEL, &sec_level,
						BT_IO_OPT_IMTU, &mtu,
						BT_IO_OPT_CID, &cid,
						BT_IO_OPT_INVALID);

	if (gerr) {
		error("bt_io_get: %s", gerr->message);
		g_error_free(gerr);
		return false;
	}

	if (sec_level == BT_IO_SEC_LOW && dev->le_state.paired) {
		DBG("Elevating security level since LTK is available");

		sec_level = BT_IO_SEC_MEDIUM;
		bt_io_set(io, &gerr, BT_IO_OPT_SEC_LEVEL, sec_level,
							BT_IO_OPT_INVALID);
		if (gerr) {
			error("bt_io_set: %s", gerr->message);
			g_error_free(gerr);
			return false;
		}
	}

	dev->att_mtu = MIN(mtu, BT_ATT_MAX_LE_MTU);
	attrib = g_attrib_new(io, dev->att_mtu, false);
	if (!attrib) {
		error("Unable to create new GAttrib instance");
		return false;
	}

	dev->attrib = attrib;
	dev->att = g_attrib_get_att(attrib);

	bt_att_ref(dev->att);

	dev->att_disconn_id = bt_att_register_disconnect(dev->att,
						att_disconnected_cb, dev, NULL);
	bt_att_set_close_on_unref(dev->att, true);

	if (dev->local_csrk)
		bt_att_set_local_key(dev->att, dev->local_csrk->key,
							local_counter, dev);

	if (dev->remote_csrk)
		bt_att_set_remote_key(dev->att, dev->remote_csrk->key,
							remote_counter, dev);

	database = btd_adapter_get_database(dev->adapter);

	src = btd_adapter_get_address(dev->adapter);
	ba2str(src, srcaddr);

	dst = device_get_address(dev);
	ba2str(dst, dstaddr);

#if !defined(RTCONFIG_BT_CONN)
	gatt_client_init(dev);
#endif
	gatt_server_init(dev, btd_gatt_database_get_db(database));

	if (gatt_db_isempty(dev->db))
		load_gatt_db(dev, srcaddr, dstaddr);

	/*
	 * Remove the device from the connect_list and give the passive
	 * scanning another chance to be restarted in case there are
	 * other devices in the connect_list.
	 */
	adapter_connect_list_remove(dev->adapter, dev);

	return true;
}

static void att_connect_cb(GIOChannel *io, GError *gerr, gpointer user_data)
{
	struct att_callbacks *attcb = user_data;
	struct btd_device *device = attcb->user_data;
	DBusMessage *reply;
	uint8_t io_cap;
	int err = 0;

	g_io_channel_unref(device->att_io);
	device->att_io = NULL;

	if (gerr) {
		DBG("%s", gerr->message);

		if (attcb->err)
			attcb->err(gerr, user_data);

		err = -ECONNABORTED;
		goto done;
	}

	if (!device_attach_att(device, io))
		goto done;

	if (attcb->success)
		attcb->success(user_data);

	if (!device->bonding)
		goto done;

	if (device->bonding->agent)
		io_cap = agent_get_io_capability(device->bonding->agent);
	else
		io_cap = IO_CAPABILITY_NOINPUTNOOUTPUT;

	err = adapter_create_bonding(device->adapter, &device->bdaddr,
					device->bdaddr_type, io_cap);
done:
	if (device->bonding && err < 0) {
		reply = btd_error_failed(device->bonding->msg, strerror(-err));
		g_dbus_send_message(dbus_conn, reply);
		bonding_request_cancel(device->bonding);
		bonding_request_free(device->bonding);
	}

	if (!err)
		device_browse_gatt(device, NULL);

	if (device->connect) {
		if (err < 0)
			reply = btd_error_failed(device->connect,
							strerror(-err));
		else
			reply = dbus_message_new_method_return(device->connect);

		g_dbus_send_message(dbus_conn, reply);
		dbus_message_unref(device->connect);
		device->connect = NULL;
	}

	g_free(attcb);
}

static void att_error_cb(const GError *gerr, gpointer user_data)
{
	struct att_callbacks *attcb = user_data;
	struct btd_device *device = attcb->user_data;

	if (g_error_matches(gerr, BT_IO_ERROR, ECONNABORTED))
		return;

	if (device_get_auto_connect(device)) {
		DBG("Enabling automatic connections");
		adapter_connect_list_add(device->adapter, device);
	}
}

int device_connect_le(struct btd_device *dev)
{
	struct btd_adapter *adapter = dev->adapter;
	struct att_callbacks *attcb;
	BtIOSecLevel sec_level;
	GIOChannel *io;
	GError *gerr = NULL;
	char addr[18];

	/* There is one connection attempt going on */
	if (dev->att_io)
		return -EALREADY;

	ba2str(&dev->bdaddr, addr);

	DBG("Connection attempt to: %s", addr);

	attcb = g_new0(struct att_callbacks, 1);
	attcb->err = att_error_cb;
	attcb->user_data = dev;

	if (dev->le_state.paired)
		sec_level = BT_IO_SEC_MEDIUM;
	else
		sec_level = BT_IO_SEC_LOW;

	/*
	 * This connection will help us catch any PDUs that comes before
	 * pairing finishes
	 */
	io = bt_io_connect(att_connect_cb, attcb, NULL, &gerr,
			BT_IO_OPT_SOURCE_BDADDR,
			btd_adapter_get_address(adapter),
			BT_IO_OPT_SOURCE_TYPE, BDADDR_LE_PUBLIC,
			BT_IO_OPT_DEST_BDADDR, &dev->bdaddr,
			BT_IO_OPT_DEST_TYPE, dev->bdaddr_type,
			BT_IO_OPT_CID, ATT_CID,
			BT_IO_OPT_SEC_LEVEL, sec_level,
			BT_IO_OPT_INVALID);

	if (io == NULL) {
		if (dev->bonding) {
			DBusMessage *reply = btd_error_failed(
					dev->bonding->msg, gerr->message);

			g_dbus_send_message(dbus_conn, reply);
			bonding_request_cancel(dev->bonding);
			bonding_request_free(dev->bonding);
		}

		error("ATT bt_io_connect(%s): %s", addr, gerr->message);
		g_error_free(gerr);
		g_free(attcb);
		return -EIO;
	}

	/* Keep this, so we can cancel the connection */
	dev->att_io = io;

	return 0;
}

static void att_browse_error_cb(const GError *gerr, gpointer user_data)
{
	struct att_callbacks *attcb = user_data;
	struct btd_device *device = attcb->user_data;
	struct browse_req *req = device->browse;

	device->browse = NULL;
	browse_request_complete(req, device->bdaddr_type, -ECONNABORTED);
}

static void att_browse_cb(gpointer user_data)
{
	DBG("ATT connection successful");
}

static struct browse_req *browse_request_new(struct btd_device *device,
							DBusMessage *msg)
{
	struct browse_req *req;

	if (device->browse)
		return NULL;

	req = g_new0(struct browse_req, 1);
	req->device = device;

	device->browse = req;

	if (!msg)
		return req;

	req->msg = dbus_message_ref(msg);

	/*
	 * Track the request owner to cancel it automatically if the owner
	 * exits
	 */
	req->listener_id = g_dbus_add_disconnect_watch(dbus_conn,
						dbus_message_get_sender(msg),
						browse_request_exit,
						req, NULL);

	return req;
}

static int device_browse_gatt(struct btd_device *device, DBusMessage *msg)
{
	struct btd_adapter *adapter = device->adapter;
	struct att_callbacks *attcb;
	struct browse_req *req;

	req = browse_request_new(device, msg);
	if (!req)
		return -EBUSY;

	if (device->client) {
		/*
		 * If discovery has not yet completed, then wait for gatt-client
		 * to become ready.
		 */
		if (!bt_gatt_client_is_ready(device->client))
			return 0;

		/*
		 * Services have already been discovered, so signal this browse
		 * request as resolved.
		 */
		device_svc_resolved(device, device->bdaddr_type, 0);
		return 0;
	}

	attcb = g_new0(struct att_callbacks, 1);
	attcb->err = att_browse_error_cb;
	attcb->success = att_browse_cb;
	attcb->user_data = device;

	device->att_io = bt_io_connect(att_connect_cb,
				attcb, NULL, NULL,
				BT_IO_OPT_SOURCE_BDADDR,
				btd_adapter_get_address(adapter),
				BT_IO_OPT_SOURCE_TYPE, BDADDR_LE_PUBLIC,
				BT_IO_OPT_DEST_BDADDR, &device->bdaddr,
				BT_IO_OPT_DEST_TYPE, device->bdaddr_type,
				BT_IO_OPT_CID, ATT_CID,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
				BT_IO_OPT_INVALID);

	if (device->att_io == NULL) {
		device->browse = NULL;
		browse_request_free(req);
		g_free(attcb);
		return -EIO;
	}

	return 0;
}

static uint16_t get_sdp_flags(struct btd_device *device)
{
	uint16_t vid, pid;

	vid = btd_device_get_vendor(device);
	pid = btd_device_get_product(device);

	/* Sony DualShock 4 is not respecting negotiated L2CAP MTU. This might
	 * results in SDP response being dropped by kernel. Workaround this by
	 * forcing SDP code to use bigger MTU while connecting.
	 */
	if (vid == 0x054c && pid == 0x05c4)
		return SDP_LARGE_MTU;

	if (btd_adapter_ssp_enabled(device->adapter))
		return 0;

	/* if no EIR try matching Sony DualShock 4 with name and class */
	if (!strncmp(device->name, "Wireless Controller", MAX_NAME_LENGTH) &&
			device->class == 0x2508)
		return SDP_LARGE_MTU;

	return 0;
}

static int device_browse_sdp(struct btd_device *device, DBusMessage *msg)
{
	struct btd_adapter *adapter = device->adapter;
	struct browse_req *req;
	uuid_t uuid;
	int err;

	req = browse_request_new(device, msg);
	if (!req)
		return -EBUSY;

	sdp_uuid16_create(&uuid, uuid_list[req->search_uuid++]);

	req->sdp_flags = get_sdp_flags(device);

	err = bt_search_service(btd_adapter_get_address(adapter),
				&device->bdaddr, &uuid, browse_cb, req, NULL,
				req->sdp_flags);
	if (err < 0) {
		browse_request_free(req);
		return err;
	}

	return err;
}

int device_discover_services(struct btd_device *device)
{
	int err;

	if (device->bredr)
		err = device_browse_sdp(device, NULL);
	else
		err = device_browse_gatt(device, NULL);

	if (err == 0 && device->discov_timer) {
		g_source_remove(device->discov_timer);
		device->discov_timer = 0;
	}

	return err;
}

struct btd_adapter *device_get_adapter(struct btd_device *device)
{
	if (!device)
		return NULL;

	return device->adapter;
}

const bdaddr_t *device_get_address(struct btd_device *device)
{
	return &device->bdaddr;
}

const char *device_get_path(const struct btd_device *device)
{
	if (!device)
		return NULL;

	return device->path;
}

gboolean device_is_temporary(struct btd_device *device)
{
	return device->temporary;
}

void btd_device_set_temporary(struct btd_device *device, bool temporary)
{
	if (!device)
		return;

	if (device->temporary == temporary)
		return;

	if (device_address_is_private(device))
		return;

	DBG("temporary %d", temporary);

	device->temporary = temporary;

	if (temporary) {
		if (device->bredr)
			adapter_whitelist_remove(device->adapter, device);
		adapter_connect_list_remove(device->adapter, device);
		return;
	}

	if (device->bredr)
		adapter_whitelist_add(device->adapter, device);

	store_device_info(device);
}

void btd_device_set_trusted(struct btd_device *device, gboolean trusted)
{
	if (!device)
		return;

	if (device->trusted == trusted)
		return;

	DBG("trusted %d", trusted);

	device->trusted = trusted;

	store_device_info(device);

	g_dbus_emit_property_changed(dbus_conn, device->path,
					DEVICE_INTERFACE, "Trusted");
}

void device_set_bonded(struct btd_device *device, uint8_t bdaddr_type)
{
	if (!device)
		return;

	DBG("");

	if (bdaddr_type == BDADDR_BREDR)
		device->bredr_state.bonded = true;
	else
		device->le_state.bonded = true;

	btd_device_set_temporary(device, false);
}

void device_set_legacy(struct btd_device *device, bool legacy)
{
	if (!device)
		return;

	DBG("legacy %d", legacy);

	if (device->legacy == legacy)
		return;

	device->legacy = legacy;

	g_dbus_emit_property_changed(dbus_conn, device->path,
					DEVICE_INTERFACE, "LegacyPairing");
}

void device_set_rssi_with_delta(struct btd_device *device, int8_t rssi,
							int8_t delta_threshold)
{
	if (!device)
		return;

	if (rssi == 0 || device->rssi == 0) {
		if (device->rssi == rssi)
			return;

		DBG("rssi %d", rssi);

		device->rssi = rssi;
	} else {
		int delta;

		if (device->rssi > rssi)
			delta = device->rssi - rssi;
		else
			delta = rssi - device->rssi;

		/* only report changes of delta_threshold dBm or more */
		if (delta < delta_threshold)
			return;

		DBG("rssi %d delta %d", rssi, delta);

		device->rssi = rssi;
	}

	g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "RSSI");
}

void device_set_rssi(struct btd_device *device, int8_t rssi)
{
	device_set_rssi_with_delta(device, rssi, RSSI_THRESHOLD);
}

void device_set_tx_power(struct btd_device *device, int8_t tx_power)
{
	if (!device)
		return;

	if (device->tx_power == tx_power)
		return;

	DBG("tx_power %d", tx_power);

	device->tx_power = tx_power;

	g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "TxPower");
}

static gboolean start_discovery(gpointer user_data)
{
	struct btd_device *device = user_data;

	if (device->bredr)
		device_browse_sdp(device, NULL);
	else
		device_browse_gatt(device, NULL);

	device->discov_timer = 0;

	return FALSE;
}

void device_set_paired(struct btd_device *dev, uint8_t bdaddr_type)
{
	struct bearer_state *state = get_state(dev, bdaddr_type);

	if (state->paired)
		return;

	state->paired = true;

	/* If the other bearer state was alraedy true we don't need to
	 * send any property signals.
	 */
	if (dev->bredr_state.paired == dev->le_state.paired)
		return;

	if (!state->svc_resolved) {
		dev->pending_paired = true;
		return;
	}

	g_dbus_emit_property_changed(dbus_conn, dev->path,
						DEVICE_INTERFACE, "Paired");
}

void device_set_unpaired(struct btd_device *dev, uint8_t bdaddr_type)
{
	struct bearer_state *state = get_state(dev, bdaddr_type);

	if (!state->paired)
		return;

	state->paired = false;

	/*
	 * If the other bearer state is still true we don't need to
	 * send any property signals or remove device.
	 */
	if (dev->bredr_state.paired != dev->le_state.paired) {
		/* TODO disconnect only unpaired bearer */
		if (state->connected)
			device_request_disconnect(dev, NULL);

		return;
	}

	g_dbus_emit_property_changed(dbus_conn, dev->path,
						DEVICE_INTERFACE, "Paired");

	btd_device_set_temporary(dev, true);

	if (btd_device_is_connected(dev))
		device_request_disconnect(dev, NULL);
	else
		btd_adapter_remove_device(dev->adapter, dev);
}

static void device_auth_req_free(struct btd_device *device)
{
	struct authentication_req *authr = device->authr;

	if (!authr)
		return;

	if (authr->agent)
		agent_unref(authr->agent);

	g_free(authr->pincode);
	g_free(authr);

	device->authr = NULL;
}

bool device_is_retrying(struct btd_device *device)
{
	struct bonding_req *bonding = device->bonding;

	return bonding && bonding->retry_timer > 0;
}

void device_bonding_complete(struct btd_device *device, uint8_t bdaddr_type,
								uint8_t status)
{
	struct bonding_req *bonding = device->bonding;
	struct authentication_req *auth = device->authr;
	struct bearer_state *state = get_state(device, bdaddr_type);

	DBG("bonding %p status 0x%02x", bonding, status);

	if (auth && auth->agent)
		agent_cancel(auth->agent);

	if (status) {
		device_cancel_authentication(device, TRUE);
		device_bonding_failed(device, status);
		return;
	}

	device_auth_req_free(device);

	/* If we're already paired nothing more is needed */
	if (state->paired)
		return;

	device_set_paired(device, bdaddr_type);

	/* If services are already resolved just reply to the pairing
	 * request
	 */
	if (state->svc_resolved && bonding) {
		/* Attept to store services for this device failed because it
		 * was not paired. Now that we're paired retry. */
		store_gatt_db(device);

		g_dbus_send_reply(dbus_conn, bonding->msg, DBUS_TYPE_INVALID);
		bonding_request_free(bonding);
		return;
	}

	/* If we were initiators start service discovery immediately.
	 * However if the other end was the initator wait a few seconds
	 * before SDP. This is due to potential IOP issues if the other
	 * end starts doing SDP at the same time as us */
	if (bonding) {
		DBG("Proceeding with service discovery");
		/* If we are initiators remove any discovery timer and just
		 * start discovering services directly */
		if (device->discov_timer) {
			g_source_remove(device->discov_timer);
			device->discov_timer = 0;
		}

		if (bdaddr_type == BDADDR_BREDR)
			device_browse_sdp(device, bonding->msg);
		else
			device_browse_gatt(device, bonding->msg);

		bonding_request_free(bonding);
	} else if (!state->svc_resolved) {
#if 0 /* disable reverse discovery */
		if (!device->browse && !device->discov_timer &&
				main_opts.reverse_sdp) {
			/* If we are not initiators and there is no currently
			 * active discovery or discovery timer, set discovery
			 * timer */
			DBG("setting timer for reverse service discovery");
			device->discov_timer = g_timeout_add_seconds(
							DISCOVERY_TIMER,
							start_discovery,
							device);
		}
#endif
	}
}

static gboolean svc_idle_cb(gpointer user_data)
{
	struct svc_callback *cb = user_data;
	struct btd_device *dev = cb->dev;

	dev->svc_callbacks = g_slist_remove(dev->svc_callbacks, cb);

	cb->func(cb->dev, 0, cb->user_data);

	g_free(cb);

	return FALSE;
}

unsigned int device_wait_for_svc_complete(struct btd_device *dev,
							device_svc_cb_t func,
							void *user_data)
{
	/* This API is only used for BR/EDR (for now) */
	struct bearer_state *state = &dev->bredr_state;
	static unsigned int id = 0;
	struct svc_callback *cb;

	cb = g_new0(struct svc_callback, 1);
	cb->func = func;
	cb->user_data = user_data;
	cb->dev = dev;
	cb->id = ++id;

	dev->svc_callbacks = g_slist_prepend(dev->svc_callbacks, cb);

	if (state->svc_resolved || !main_opts.reverse_sdp)
		cb->idle_id = g_idle_add(svc_idle_cb, cb);
	else if (dev->discov_timer > 0) {
		g_source_remove(dev->discov_timer);
		dev->discov_timer = g_idle_add(start_discovery, dev);
	}

	return cb->id;
}

bool device_remove_svc_complete_callback(struct btd_device *dev,
							unsigned int id)
{
	GSList *l;

	for (l = dev->svc_callbacks; l != NULL; l = g_slist_next(l)) {
		struct svc_callback *cb = l->data;

		if (cb->id != id)
			continue;

		if (cb->idle_id > 0)
			g_source_remove(cb->idle_id);

		dev->svc_callbacks = g_slist_remove(dev->svc_callbacks, cb);
		g_free(cb);

		return true;
	}

	return false;
}

gboolean device_is_bonding(struct btd_device *device, const char *sender)
{
	struct bonding_req *bonding = device->bonding;

	if (!device->bonding)
		return FALSE;

	if (!sender)
		return TRUE;

	return g_str_equal(sender, dbus_message_get_sender(bonding->msg));
}

static gboolean device_bonding_retry(gpointer data)
{
	struct btd_device *device = data;
	struct btd_adapter *adapter = device_get_adapter(device);
	struct bonding_req *bonding = device->bonding;
	uint8_t io_cap;
	int err;

	if (!bonding)
		return FALSE;

	DBG("retrying bonding");
	bonding->retry_timer = 0;

	/* Restart the bonding timer to the begining of the pairing. If not
	 * pincode request/reply occurs during this retry,
	 * device_bonding_last_duration() will return a consistent value from
	 * this point. */
	device_bonding_restart_timer(device);

	if (bonding->agent)
		io_cap = agent_get_io_capability(bonding->agent);
	else
		io_cap = IO_CAPABILITY_NOINPUTNOOUTPUT;

	err = adapter_bonding_attempt(adapter, &device->bdaddr,
				device->bdaddr_type, io_cap);
	if (err < 0)
		device_bonding_complete(device, bonding->bdaddr_type,
							bonding->status);

	return FALSE;
}

int device_bonding_attempt_retry(struct btd_device *device)
{
	struct bonding_req *bonding = device->bonding;

	/* Ignore other failure events while retrying */
	if (device_is_retrying(device))
		return 0;

	if (!bonding)
		return -EINVAL;

	/* Mark the end of a bonding attempt to compute the delta for the
	 * retry. */
	bonding_request_stop_timer(bonding);

	if (btd_adapter_pin_cb_iter_end(bonding->cb_iter))
		return -EINVAL;

	DBG("scheduling retry");
	bonding->retry_timer = g_timeout_add(3000,
						device_bonding_retry, device);
	return 0;
}

void device_bonding_failed(struct btd_device *device, uint8_t status)
{
	struct bonding_req *bonding = device->bonding;
	DBusMessage *reply;

	DBG("status %u", status);

	if (!bonding)
		return;

	if (device->authr)
		device_cancel_authentication(device, FALSE);

	reply = new_authentication_return(bonding->msg, status);
	g_dbus_send_message(dbus_conn, reply);

	bonding_request_free(bonding);
}

struct btd_adapter_pin_cb_iter *device_bonding_iter(struct btd_device *device)
{
	if (device->bonding == NULL)
		return NULL;

	return device->bonding->cb_iter;
}

static void pincode_cb(struct agent *agent, DBusError *err, const char *pin,
								void *data)
{
	struct authentication_req *auth = data;
	struct btd_device *device = auth->device;

	/* No need to reply anything if the authentication already failed */
	if (auth->agent == NULL)
		return;

	btd_adapter_pincode_reply(device->adapter, &device->bdaddr,
						pin, pin ? strlen(pin) : 0);

	agent_unref(device->authr->agent);
	device->authr->agent = NULL;
}

static void confirm_cb(struct agent *agent, DBusError *err, void *data)
{
	struct authentication_req *auth = data;
	struct btd_device *device = auth->device;

	/* No need to reply anything if the authentication already failed */
	if (auth->agent == NULL)
		return;

	btd_adapter_confirm_reply(device->adapter, &device->bdaddr,
							device->bdaddr_type,
							err ? FALSE : TRUE);

	agent_unref(device->authr->agent);
	device->authr->agent = NULL;
}

static void passkey_cb(struct agent *agent, DBusError *err,
						uint32_t passkey, void *data)
{
	struct authentication_req *auth = data;
	struct btd_device *device = auth->device;

	/* No need to reply anything if the authentication already failed */
	if (auth->agent == NULL)
		return;

	if (err)
		passkey = INVALID_PASSKEY;

	btd_adapter_passkey_reply(device->adapter, &device->bdaddr,
						device->bdaddr_type, passkey);

	agent_unref(device->authr->agent);
	device->authr->agent = NULL;
}

static void display_pincode_cb(struct agent *agent, DBusError *err, void *data)
{
	struct authentication_req *auth = data;
	struct btd_device *device = auth->device;

	pincode_cb(agent, err, auth->pincode, auth);

	g_free(device->authr->pincode);
	device->authr->pincode = NULL;
}

static struct authentication_req *new_auth(struct btd_device *device,
					auth_type_t type, gboolean secure)
{
	struct authentication_req *auth;
	struct agent *agent;
	char addr[18];

	ba2str(&device->bdaddr, addr);
	DBG("Requesting agent authentication for %s", addr);

	if (device->authr) {
		error("Authentication already requested for %s", addr);
		return NULL;
	}

	if (device->bonding && device->bonding->agent)
		agent = agent_ref(device->bonding->agent);
	else
		agent = agent_get(NULL);

	if (!agent) {
		error("No agent available for request type %d", type);
		return NULL;
	}

	auth = g_new0(struct authentication_req, 1);
	auth->agent = agent;
	auth->device = device;
	auth->type = type;
	auth->secure = secure;
	device->authr = auth;

	return auth;
}

int device_request_pincode(struct btd_device *device, gboolean secure)
{
	struct authentication_req *auth;
	int err;

	auth = new_auth(device, AUTH_TYPE_PINCODE, secure);
	if (!auth)
		return -EPERM;

	err = agent_request_pincode(auth->agent, device, pincode_cb, secure,
								auth, NULL);
	if (err < 0) {
		error("Failed requesting authentication");
		device_auth_req_free(device);
	}

	return err;
}

int device_request_passkey(struct btd_device *device)
{
	struct authentication_req *auth;
	int err;

	auth = new_auth(device, AUTH_TYPE_PASSKEY, FALSE);
	if (!auth)
		return -EPERM;

	err = agent_request_passkey(auth->agent, device, passkey_cb, auth,
									NULL);
	if (err < 0) {
		error("Failed requesting authentication");
		device_auth_req_free(device);
	}

	return err;
}

int device_confirm_passkey(struct btd_device *device, uint32_t passkey,
							uint8_t confirm_hint)

{
	struct authentication_req *auth;
	int err;

	auth = new_auth(device, AUTH_TYPE_CONFIRM, FALSE);
	if (!auth)
		return -EPERM;

	auth->passkey = passkey;

	if (confirm_hint)
		err = agent_request_authorization(auth->agent, device,
						confirm_cb, auth, NULL);
	else
		err = agent_request_confirmation(auth->agent, device, passkey,
						confirm_cb, auth, NULL);

	if (err < 0) {
		error("Failed requesting authentication");
		device_auth_req_free(device);
	}

	return err;
}

int device_notify_passkey(struct btd_device *device, uint32_t passkey,
							uint8_t entered)
{
	struct authentication_req *auth;
	int err;

	if (device->authr) {
		auth = device->authr;
		if (auth->type != AUTH_TYPE_NOTIFY_PASSKEY)
			return -EPERM;
	} else {
		auth = new_auth(device, AUTH_TYPE_NOTIFY_PASSKEY, FALSE);
		if (!auth)
			return -EPERM;
	}

	err = agent_display_passkey(auth->agent, device, passkey, entered);
	if (err < 0) {
		error("Failed requesting authentication");
		device_auth_req_free(device);
	}

	return err;
}

int device_notify_pincode(struct btd_device *device, gboolean secure,
							const char *pincode)
{
	struct authentication_req *auth;
	int err;

	auth = new_auth(device, AUTH_TYPE_NOTIFY_PINCODE, secure);
	if (!auth)
		return -EPERM;

	auth->pincode = g_strdup(pincode);

	err = agent_display_pincode(auth->agent, device, pincode,
					display_pincode_cb, auth, NULL);
	if (err < 0) {
		error("Failed requesting authentication");
		device_auth_req_free(device);
	}

	return err;
}

static void cancel_authentication(struct authentication_req *auth)
{
	struct agent *agent;
	DBusError err;

	if (!auth || !auth->agent)
		return;

	agent = auth->agent;
	auth->agent = NULL;

	dbus_error_init(&err);
	dbus_set_error_const(&err, ERROR_INTERFACE ".Canceled", NULL);

	switch (auth->type) {
	case AUTH_TYPE_PINCODE:
		pincode_cb(agent, &err, NULL, auth);
		break;
	case AUTH_TYPE_CONFIRM:
		confirm_cb(agent, &err, auth);
		break;
	case AUTH_TYPE_PASSKEY:
		passkey_cb(agent, &err, 0, auth);
		break;
	case AUTH_TYPE_NOTIFY_PASSKEY:
		/* User Notify doesn't require any reply */
		break;
	case AUTH_TYPE_NOTIFY_PINCODE:
		pincode_cb(agent, &err, NULL, auth);
		break;
	}

	dbus_error_free(&err);
}

void device_cancel_authentication(struct btd_device *device, gboolean aborted)
{
	struct authentication_req *auth = device->authr;
	char addr[18];

	if (!auth)
		return;

	ba2str(&device->bdaddr, addr);
	DBG("Canceling authentication request for %s", addr);

	if (auth->agent)
		agent_cancel(auth->agent);

	if (!aborted)
		cancel_authentication(auth);

	device_auth_req_free(device);
}

gboolean device_is_authenticating(struct btd_device *device)
{
	return (device->authr != NULL);
}

struct gatt_primary *btd_device_get_primary(struct btd_device *device,
							const char *uuid)
{
	GSList *match;

	match = g_slist_find_custom(device->primaries, uuid, bt_uuid_strcmp);
	if (match)
		return match->data;

	return NULL;
}

GSList *btd_device_get_primaries(struct btd_device *device)
{
	return device->primaries;
}

struct gatt_db *btd_device_get_gatt_db(struct btd_device *device)
{
	if (!device)
		return NULL;

	return device->db;
}

struct bt_gatt_client *btd_device_get_gatt_client(struct btd_device *device)
{
	if (!device)
		return NULL;

	return device->client;
}

struct bt_gatt_server *btd_device_get_gatt_server(struct btd_device *device)
{
	if (!device)
		return NULL;

	return device->server;
}

void btd_device_gatt_set_service_changed(struct btd_device *device,
						uint16_t start, uint16_t end)
{
	/*
	 * TODO: Remove this function and handle service changed via
	 * gatt-client.
	 */
}

void btd_device_add_uuid(struct btd_device *device, const char *uuid)
{
	GSList *uuid_list;
	char *new_uuid;

	if (g_slist_find_custom(device->uuids, uuid, bt_uuid_strcmp))
		return;

	new_uuid = g_strdup(uuid);
	uuid_list = g_slist_append(NULL, new_uuid);

	device_probe_profiles(device, uuid_list);

	g_free(new_uuid);
	g_slist_free(uuid_list);

	store_device_info(device);

	g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "UUIDs");
}

static sdp_list_t *read_device_records(struct btd_device *device)
{
	char local[18], peer[18];
	char filename[PATH_MAX];
	GKeyFile *key_file;
	char **keys, **handle;
	char *str;
	sdp_list_t *recs = NULL;
	sdp_record_t *rec;

	ba2str(btd_adapter_get_address(device->adapter), local);
	ba2str(&device->bdaddr, peer);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/cache/%s", local, peer);

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);
	keys = g_key_file_get_keys(key_file, "ServiceRecords", NULL, NULL);

	for (handle = keys; handle && *handle; handle++) {
		str = g_key_file_get_string(key_file, "ServiceRecords",
						*handle, NULL);
		if (!str)
			continue;

		rec = record_from_string(str);
		recs = sdp_list_append(recs, rec);
		g_free(str);
	}

	g_strfreev(keys);
	g_key_file_free(key_file);

	return recs;
}

const sdp_record_t *btd_device_get_record(struct btd_device *device,
							const char *uuid)
{
	/* Load records from storage if there is nothing in cache */
	if (!device->tmp_records) {
		device->tmp_records = read_device_records(device);
		if (!device->tmp_records)
			return NULL;
	}

	return find_record_in_list(device->tmp_records, uuid);
}

struct btd_device *btd_device_ref(struct btd_device *device)
{
	__sync_fetch_and_add(&device->ref_count, 1);

	return device;
}

void btd_device_unref(struct btd_device *device)
{
	if (__sync_sub_and_fetch(&device->ref_count, 1))
		return;

	if (!device->path) {
		error("freeing device without an object path");
		return;
	}

	DBG("Freeing device %s", device->path);

	g_dbus_unregister_interface(dbus_conn, device->path, DEVICE_INTERFACE);
}

int device_get_appearance(struct btd_device *device, uint16_t *value)
{
	if (device->appearance == 0)
		return -1;

	if (value)
		*value = device->appearance;

	return 0;
}

void device_set_appearance(struct btd_device *device, uint16_t value)
{
	const char *icon = gap_appearance_to_icon(value);

	if (device->appearance == value)
		return;

	g_dbus_emit_property_changed(dbus_conn, device->path,
					DEVICE_INTERFACE, "Appearance");

	if (icon)
		g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "Icon");

	device->appearance = value;
	store_device_info(device);
}

static gboolean notify_attios(gpointer user_data)
{
	struct btd_device *device = user_data;

	DBG("");

	if (device->attrib == NULL)
		return FALSE;

	g_slist_foreach(device->attios_offline, attio_connected, device->attrib);
	device->attios = g_slist_concat(device->attios, device->attios_offline);
	device->attios_offline = NULL;

	return FALSE;
}

guint btd_device_add_attio_callback(struct btd_device *device,
						attio_connect_cb cfunc,
						attio_disconnect_cb dcfunc,
						gpointer user_data)
{
	struct attio_data *attio;
	static guint attio_id = 0;

	DBG("%p registered ATT connection callback", device);

	attio = g_new0(struct attio_data, 1);
	attio->id = ++attio_id;
	attio->cfunc = cfunc;
	attio->dcfunc = dcfunc;
	attio->user_data = user_data;

	device_set_auto_connect(device, TRUE);

	/* Check if there is no GAttrib associated to the device created by a
	 * incoming connection */
	if (!device->attrib)
		device->attrib = attrib_from_device(device);

	if (device->attrib && cfunc) {
		device->attios_offline = g_slist_append(device->attios_offline,
									attio);
		g_idle_add(notify_attios, device);
		return attio->id;
	}

	device->attios = g_slist_append(device->attios, attio);

	return attio->id;
}

static int attio_id_cmp(gconstpointer a, gconstpointer b)
{
	const struct attio_data *attio = a;
	guint id = GPOINTER_TO_UINT(b);

	return attio->id - id;
}

gboolean btd_device_remove_attio_callback(struct btd_device *device, guint id)
{
	struct attio_data *attio;
	GSList *l;

	l = g_slist_find_custom(device->attios, GUINT_TO_POINTER(id),
								attio_id_cmp);
	if (l) {
		attio = l->data;
		device->attios = g_slist_remove(device->attios, attio);
	} else {
		l = g_slist_find_custom(device->attios_offline,
					GUINT_TO_POINTER(id), attio_id_cmp);
		if (!l)
			return FALSE;

		attio = l->data;
		device->attios_offline = g_slist_remove(device->attios_offline,
									attio);
	}

	g_free(attio);

	return TRUE;
}

void btd_device_set_pnpid(struct btd_device *device, uint16_t source,
			uint16_t vendor, uint16_t product, uint16_t version)
{
	if (device->vendor_src == source && device->version == version &&
			device->vendor == vendor && device->product == product)
		return;

	device->vendor_src = source;
	device->vendor = vendor;
	device->product = product;
	device->version = version;

	free(device->modalias);
	device->modalias = bt_modalias(source, vendor, product, version);

	g_dbus_emit_property_changed(dbus_conn, device->path,
						DEVICE_INTERFACE, "Modalias");

	store_device_info(device);
}

static void service_state_changed(struct btd_service *service,
						btd_service_state_t old_state,
						btd_service_state_t new_state,
						void *user_data)
{
	struct btd_profile *profile = btd_service_get_profile(service);
	struct btd_device *device = btd_service_get_device(service);
	int err = btd_service_get_error(service);

	if (new_state == BTD_SERVICE_STATE_CONNECTING ||
				new_state == BTD_SERVICE_STATE_DISCONNECTING)
		return;

	if (old_state == BTD_SERVICE_STATE_CONNECTING)
		device_profile_connected(device, profile, err);
	else if (old_state == BTD_SERVICE_STATE_DISCONNECTING)
		device_profile_disconnected(device, profile, err);
}

struct btd_service *btd_device_get_service(struct btd_device *dev,
						const char *remote_uuid)
{
	GSList *l;

	for (l = dev->services; l != NULL; l = g_slist_next(l)) {
		struct btd_service *service = l->data;
		struct btd_profile *p = btd_service_get_profile(service);

		if (g_str_equal(p->remote_uuid, remote_uuid))
			return service;
	}

	return NULL;
}

void btd_device_init(void)
{
	dbus_conn = btd_get_dbus_connection();
	service_state_cb_id = btd_service_add_state_cb(
						service_state_changed, NULL);
}

void btd_device_cleanup(void)
{
	btd_service_remove_state_cb(service_state_cb_id);
}
