/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012 Tieto Poland
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

#include <errno.h>
#include <stdbool.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "src/plugin.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/profile.h"
#include "src/service.h"
#include "src/dbus-common.h"
#include "src/shared/util.h"
#include "src/error.h"
#include "attrib/gattrib.h"
#include "attrib/att.h"
#include "attrib/gatt.h"
#include "src/attio.h"
#include "src/log.h"

/* min length for ATT indication or notification: opcode (1b) + handle (2b) */
#define ATT_HDR_LEN 3

#define ATT_TIMEOUT 30

#define CYCLINGSPEED_INTERFACE		"org.bluez.CyclingSpeed1"
#define CYCLINGSPEED_MANAGER_INTERFACE	"org.bluez.CyclingSpeedManager1"
#define CYCLINGSPEED_WATCHER_INTERFACE	"org.bluez.CyclingSpeedWatcher1"

#define WHEEL_REV_SUPPORT		0x01
#define CRANK_REV_SUPPORT		0x02
#define MULTI_SENSOR_LOC_SUPPORT	0x04

#define WHEEL_REV_PRESENT	0x01
#define CRANK_REV_PRESENT	0x02

#define SET_CUMULATIVE_VALUE		0x01
#define START_SENSOR_CALIBRATION	0x02
#define UPDATE_SENSOR_LOC		0x03
#define REQUEST_SUPPORTED_SENSOR_LOC	0x04
#define RESPONSE_CODE			0x10

#define RSP_SUCCESS		0x01
#define RSP_NOT_SUPPORTED	0x02
#define RSP_INVALID_PARAM	0x03
#define RSP_FAILED		0x04

struct csc;

struct controlpoint_req {
	struct csc		*csc;
	uint8_t			opcode;
	guint			timeout;
	GDBusPendingReply	reply_id;
	DBusMessage		*msg;

	uint8_t			pending_location;
};

struct csc_adapter {
	struct btd_adapter	*adapter;
	GSList			*devices;	/* list of registered devices */
	GSList			*watchers;
};

struct csc {
	struct btd_device	*dev;
	struct csc_adapter	*cadapter;

	GAttrib			*attrib;
	guint			attioid;
	/* attio id for measurement characteristics value notifications */
	guint			attio_measurement_id;
	/* attio id for SC Control Point characteristics value indications */
	guint			attio_controlpoint_id;

	struct att_range	*svc_range;

	uint16_t		measurement_ccc_handle;
	uint16_t		controlpoint_val_handle;

	uint16_t		feature;
	gboolean		has_location;
	uint8_t			location;
	uint8_t			num_locations;
	uint8_t			*locations;

	struct controlpoint_req	*pending_req;
};

struct watcher {
	struct csc_adapter	*cadapter;
	guint			id;
	char			*srv;
	char			*path;
};

struct measurement {
	struct csc	*csc;

	bool		has_wheel_rev;
	uint32_t	wheel_rev;
	uint16_t	last_wheel_time;

	bool		has_crank_rev;
	uint16_t	crank_rev;
	uint16_t	last_crank_time;
};

struct characteristic {
	struct csc	*csc;
	char		uuid[MAX_LEN_UUID_STR + 1];
};

static GSList *csc_adapters = NULL;

static const char * const location_enum[] = {
	"other", "top-of-shoe", "in-shoe", "hip", "front-wheel", "left-crank",
	"right-crank", "left-pedal", "right-pedal", "front-hub",
	"rear-dropout", "chainstay", "rear-wheel", "rear-hub"
};

static const char *location2str(uint8_t value)
{
	if (value < G_N_ELEMENTS(location_enum))
		return location_enum[value];

	info("Body Sensor Location [%d] is RFU", value);

	return location_enum[0];
}

static int str2location(const char *location)
{
	size_t i;

	for (i = 0; i < G_N_ELEMENTS(location_enum); i++)
		if (!strcmp(location_enum[i], location))
			return i;

	return -1;
}

static int cmp_adapter(gconstpointer a, gconstpointer b)
{
	const struct csc_adapter *cadapter = a;
	const struct btd_adapter *adapter = b;

	if (adapter == cadapter->adapter)
		return 0;

	return -1;
}

static int cmp_device(gconstpointer a, gconstpointer b)
{
	const struct csc *csc = a;
	const struct btd_device *dev = b;

	if (dev == csc->dev)
		return 0;

	return -1;
}

static int cmp_watcher(gconstpointer a, gconstpointer b)
{
	const struct watcher *watcher = a;
	const struct watcher *match = b;
	int ret;

	ret = g_strcmp0(watcher->srv, match->srv);
	if (ret != 0)
		return ret;

	return g_strcmp0(watcher->path, match->path);
}

static struct csc_adapter *find_csc_adapter(struct btd_adapter *adapter)
{
	GSList *l = g_slist_find_custom(csc_adapters, adapter, cmp_adapter);

	if (!l)
		return NULL;

	return l->data;
}

static void destroy_watcher(gpointer user_data)
{
	struct watcher *watcher = user_data;

	g_free(watcher->path);
	g_free(watcher->srv);
	g_free(watcher);
}

static struct watcher *find_watcher(GSList *list, const char *sender,
							const char *path)
{
	struct watcher *match;
	GSList *l;

	match = g_new0(struct watcher, 1);
	match->srv = g_strdup(sender);
	match->path = g_strdup(path);

	l = g_slist_find_custom(list, match, cmp_watcher);
	destroy_watcher(match);

	if (l != NULL)
		return l->data;

	return NULL;
}

static void remove_watcher(gpointer user_data)
{
	struct watcher *watcher = user_data;

	g_dbus_remove_watch(btd_get_dbus_connection(), watcher->id);
}

static void destroy_csc_adapter(gpointer user_data)
{
	struct csc_adapter *cadapter = user_data;

	g_slist_free_full(cadapter->watchers, remove_watcher);

	g_free(cadapter);
}

static void destroy_csc(gpointer user_data)
{
	struct csc *csc = user_data;

	if (csc->attioid > 0)
		btd_device_remove_attio_callback(csc->dev, csc->attioid);

	if (csc->attrib != NULL) {
		g_attrib_unregister(csc->attrib, csc->attio_measurement_id);
		g_attrib_unregister(csc->attrib, csc->attio_controlpoint_id);
		g_attrib_unref(csc->attrib);
	}

	btd_device_unref(csc->dev);
	g_free(csc->svc_range);
	g_free(csc->locations);
	g_free(csc);
}

static void char_write_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	char *msg = user_data;

	if (status != 0)
		error("%s failed", msg);

	g_free(msg);
}

static gboolean controlpoint_timeout(gpointer user_data)
{
	struct controlpoint_req *req = user_data;

	if (req->opcode == UPDATE_SENSOR_LOC) {
		g_dbus_pending_property_error(req->reply_id,
						ERROR_INTERFACE ".Failed",
						"Operation failed (timeout)");
	} else if (req->opcode == SET_CUMULATIVE_VALUE) {
		DBusMessage *reply;

		reply = btd_error_failed(req->msg,
						"Operation failed (timeout)");

		g_dbus_send_message(btd_get_dbus_connection(), reply);

		dbus_message_unref(req->msg);
	}

	req->csc->pending_req = NULL;
	g_free(req);

	return FALSE;
}

static void controlpoint_write_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct controlpoint_req *req = user_data;

	if (status == 0) {
		req->timeout = g_timeout_add_seconds(ATT_TIMEOUT,
							controlpoint_timeout,
							req);
		return;
	}

	error("SC Control Point write failed (opcode=%d)", req->opcode);

	if (req->opcode == UPDATE_SENSOR_LOC) {
		g_dbus_pending_property_error(req->reply_id,
					ERROR_INTERFACE ".Failed",
					"Operation failed (%d)", status);
	} else if  (req->opcode == SET_CUMULATIVE_VALUE) {
		DBusMessage *reply;

		reply = btd_error_failed(req->msg, "Operation failed");

		g_dbus_send_message(btd_get_dbus_connection(), reply);

		dbus_message_unref(req->msg);
	}

	req->csc->pending_req = NULL;
	g_free(req);
}

static void read_supported_locations(struct csc *csc)
{
	struct controlpoint_req *req;

	req = g_new0(struct controlpoint_req, 1);
	req->csc = csc;
	req->opcode = REQUEST_SUPPORTED_SENSOR_LOC;

	csc->pending_req = req;

	gatt_write_char(csc->attrib, csc->controlpoint_val_handle,
					&req->opcode, sizeof(req->opcode),
					controlpoint_write_cb, req);
}

static void read_feature_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct csc *csc = user_data;
	uint8_t value[2];
	ssize_t vlen;

	if (status) {
		error("CSC Feature read failed: %s", att_ecode2str(status));
		return;
	}

	vlen = dec_read_resp(pdu, len, value, sizeof(value));
	if (vlen < 0) {
		error("Protocol error");
		return;
	}

	if (vlen != sizeof(value)) {
		error("Invalid value length for CSC Feature");
		return;
	}

	csc->feature = get_le16(value);

	if ((csc->feature & MULTI_SENSOR_LOC_SUPPORT)
						&& (csc->locations == NULL))
		read_supported_locations(csc);
}

static void read_location_cb(guint8 status, const guint8 *pdu,
						guint16 len, gpointer user_data)
{
	struct csc *csc = user_data;
	uint8_t value;
	ssize_t vlen;

	if (status) {
		error("Sensor Location read failed: %s", att_ecode2str(status));
		return;
	}

	vlen = dec_read_resp(pdu, len, &value, sizeof(value));
	if (vlen < 0) {
		error("Protocol error");
		return;
	}

	if (vlen != sizeof(value)) {
		error("Invalid value length for Sensor Location");
		return;
	}

	csc->has_location = TRUE;
	csc->location = value;

	g_dbus_emit_property_changed(btd_get_dbus_connection(),
					device_get_path(csc->dev),
					CYCLINGSPEED_INTERFACE, "Location");
}

static void discover_desc_cb(guint8 status, GSList *descs, gpointer user_data)
{
	struct characteristic *ch = user_data;
	struct gatt_desc *desc;
	uint8_t attr_val[2];
	char *msg = NULL;

	if (status != 0) {
		error("Discover %s descriptors failed: %s", ch->uuid,
							att_ecode2str(status));
		goto done;
	}

	/* There will be only one descriptor on list and it will be CCC */
	desc = descs->data;

	if (g_strcmp0(ch->uuid, CSC_MEASUREMENT_UUID) == 0) {
		ch->csc->measurement_ccc_handle = desc->handle;

		if (g_slist_length(ch->csc->cadapter->watchers) == 0) {
			put_le16(0x0000, attr_val);
			msg = g_strdup("Disable measurement");
		} else {
			put_le16(GATT_CLIENT_CHARAC_CFG_NOTIF_BIT,
							attr_val);
			msg = g_strdup("Enable measurement");
		}
	} else if (g_strcmp0(ch->uuid, SC_CONTROL_POINT_UUID) == 0) {
		put_le16(GATT_CLIENT_CHARAC_CFG_IND_BIT, attr_val);
		msg = g_strdup("Enable SC Control Point indications");
	} else {
		goto done;
	}

	gatt_write_char(ch->csc->attrib, desc->handle, attr_val,
					sizeof(attr_val), char_write_cb, msg);

done:
	g_free(ch);
}

static void discover_desc(struct csc *csc, struct gatt_char *c,
						struct gatt_char *c_next)
{
	struct characteristic *ch;
	uint16_t start, end;
	bt_uuid_t uuid;

	start = c->value_handle + 1;

	if (c_next != NULL) {
		if (start == c_next->handle)
			return;
		end = c_next->handle - 1;
	} else if (c->value_handle != csc->svc_range->end) {
		end = csc->svc_range->end;
	} else {
		return;
	}

	ch = g_new0(struct characteristic, 1);
	ch->csc = csc;
	memcpy(ch->uuid, c->uuid, sizeof(c->uuid));

	bt_uuid16_create(&uuid, GATT_CLIENT_CHARAC_CFG_UUID);

	gatt_discover_desc(csc->attrib, start, end, &uuid, discover_desc_cb,
									ch);
}

static void update_watcher(gpointer data, gpointer user_data)
{
	struct watcher *w = data;
	struct measurement *m = user_data;
	struct csc *csc = m->csc;
	const char *path = device_get_path(csc->dev);
	DBusMessageIter iter;
	DBusMessageIter dict;
	DBusMessage *msg;

	msg = dbus_message_new_method_call(w->srv, w->path,
			CYCLINGSPEED_WATCHER_INTERFACE, "MeasurementReceived");
	if (msg == NULL)
		return;

	dbus_message_iter_init_append(msg, &iter);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH , &path);

	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
			DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
			DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
			DBUS_DICT_ENTRY_END_CHAR_AS_STRING, &dict);

	if (m->has_wheel_rev) {
		dict_append_entry(&dict, "WheelRevolutions",
					DBUS_TYPE_UINT32, &m->wheel_rev);
		dict_append_entry(&dict, "LastWheelEventTime",
					DBUS_TYPE_UINT16, &m->last_wheel_time);
	}

	if (m->has_crank_rev) {
		dict_append_entry(&dict, "CrankRevolutions",
					DBUS_TYPE_UINT16, &m->crank_rev);
		dict_append_entry(&dict, "LastCrankEventTime",
					DBUS_TYPE_UINT16, &m->last_crank_time);
	}

	dbus_message_iter_close_container(&iter, &dict);

	dbus_message_set_no_reply(msg, TRUE);
	g_dbus_send_message(btd_get_dbus_connection(), msg);
}

static void process_measurement(struct csc *csc, const uint8_t *pdu,
								uint16_t len)
{
	struct measurement m;
	uint8_t flags;

	flags = *pdu;

	pdu++;
	len--;

	memset(&m, 0, sizeof(m));

	if ((flags & WHEEL_REV_PRESENT) && (csc->feature & WHEEL_REV_SUPPORT)) {
		if (len < 6) {
			error("Wheel revolutions data fields missing");
			return;
		}

		m.has_wheel_rev = true;
		m.wheel_rev = get_le32(pdu);
		m.last_wheel_time = get_le16(pdu + 4);
		pdu += 6;
		len -= 6;
	}

	if ((flags & CRANK_REV_PRESENT) && (csc->feature & CRANK_REV_SUPPORT)) {
		if (len < 4) {
			error("Crank revolutions data fields missing");
			return;
		}

		m.has_crank_rev = true;
		m.crank_rev = get_le16(pdu);
		m.last_crank_time = get_le16(pdu + 2);
		pdu += 4;
		len -= 4;
	}

	/* Notify all registered watchers */
	m.csc = csc;
	g_slist_foreach(csc->cadapter->watchers, update_watcher, &m);
}

static void measurement_notify_handler(const uint8_t *pdu, uint16_t len,
							gpointer user_data)
{
	struct csc *csc = user_data;

	/* should be at least opcode (1b) + handle (2b) */
	if (len < 3) {
		error("Invalid PDU received");
		return;
	}

	process_measurement(csc, pdu + 3, len - 3);
}

static void controlpoint_property_reply(struct controlpoint_req *req,
								uint8_t code)
{
	switch (code) {
	case RSP_SUCCESS:
		g_dbus_pending_property_success(req->reply_id);
		break;

	case RSP_NOT_SUPPORTED:
		g_dbus_pending_property_error(req->reply_id,
					ERROR_INTERFACE ".NotSupported",
					"Feature is not supported");
		break;

	case RSP_INVALID_PARAM:
		g_dbus_pending_property_error(req->reply_id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		break;

	case RSP_FAILED:
		g_dbus_pending_property_error(req->reply_id,
					ERROR_INTERFACE ".Failed",
					"Operation failed");
		break;

	default:
		g_dbus_pending_property_error(req->reply_id,
					ERROR_INTERFACE ".Failed",
					"Operation failed (%d)", code);
		break;
	}
}

static void controlpoint_method_reply(struct controlpoint_req *req,
								uint8_t code)
{
	DBusMessage *reply;

	switch (code) {
	case RSP_SUCCESS:
		reply = dbus_message_new_method_return(req->msg);
		break;
	case RSP_NOT_SUPPORTED:
		reply = btd_error_not_supported(req->msg);
		break;
	case RSP_INVALID_PARAM:
		reply = btd_error_invalid_args(req->msg);
		break;
	case RSP_FAILED:
		reply = btd_error_failed(req->msg, "Failed");
		break;
	default:
		reply = btd_error_failed(req->msg, "Unknown error");
		break;
	}

	g_dbus_send_message(btd_get_dbus_connection(), reply);

	dbus_message_unref(req->msg);
}

static void controlpoint_ind_handler(const uint8_t *pdu, uint16_t len,
							gpointer user_data)
{
	struct csc *csc = user_data;
	struct controlpoint_req *req = csc->pending_req;
	uint8_t opcode;
	uint8_t req_opcode;
	uint8_t rsp_code;
	uint8_t *opdu;
	uint16_t olen;
	size_t plen;

	if (len < ATT_HDR_LEN) {
		error("Invalid PDU received");
		return;
	}

	/* skip ATT header */
	pdu += ATT_HDR_LEN;
	len -= ATT_HDR_LEN;

	if (len < 1) {
		error("Op Code missing");
		goto done;
	}

	opcode = *pdu;
	pdu++;
	len--;

	if (opcode != RESPONSE_CODE) {
		DBG("Unsupported Op Code received (%d)", opcode);
		goto done;
	}

	if (len < 2) {
		error("Invalid Response Code PDU received");
		goto done;
	}

	req_opcode = *pdu;
	rsp_code = *(pdu + 1);
	pdu += 2;
	len -= 2;

	if (req == NULL || req->opcode != req_opcode) {
		DBG("Indication received without pending request");
		goto done;
	}

	switch (req->opcode) {
	case SET_CUMULATIVE_VALUE:
		controlpoint_method_reply(req, rsp_code);
		break;

	case REQUEST_SUPPORTED_SENSOR_LOC:
		if (rsp_code == RSP_SUCCESS) {
			csc->num_locations = len;
			csc->locations = g_memdup(pdu, len);
		} else {
			error("Failed to read Supported Sendor Locations");
		}
		break;

	case UPDATE_SENSOR_LOC:
		csc->location = req->pending_location;

		controlpoint_property_reply(req, rsp_code);

		g_dbus_emit_property_changed(btd_get_dbus_connection(),
					device_get_path(csc->dev),
					CYCLINGSPEED_INTERFACE, "Location");
		break;
	}

	csc->pending_req = NULL;
	g_source_remove(req->timeout);
	g_free(req);

done:
	opdu = g_attrib_get_buffer(csc->attrib, &plen);
	olen = enc_confirmation(opdu, plen);
	if (olen > 0)
		g_attrib_send(csc->attrib, 0, opdu, olen, NULL, NULL, NULL);
}

static void discover_char_cb(uint8_t status, GSList *chars, void *user_data)
{
	struct csc *csc = user_data;
	uint16_t feature_val_handle = 0;

	if (status) {
		error("Discover CSCS characteristics: %s",
							att_ecode2str(status));
		return;
	}

	for (; chars; chars = chars->next) {
		struct gatt_char *c = chars->data;
		struct gatt_char *c_next =
				(chars->next ? chars->next->data : NULL);

		if (g_strcmp0(c->uuid, CSC_MEASUREMENT_UUID) == 0) {
			csc->attio_measurement_id =
				g_attrib_register(csc->attrib,
					ATT_OP_HANDLE_NOTIFY, c->value_handle,
					measurement_notify_handler, csc, NULL);

			discover_desc(csc, c, c_next);
		} else if (g_strcmp0(c->uuid, CSC_FEATURE_UUID) == 0) {
			feature_val_handle = c->value_handle;
		} else if (g_strcmp0(c->uuid, SENSOR_LOCATION_UUID) == 0) {
			DBG("Sensor Location supported");
			gatt_read_char(csc->attrib, c->value_handle,
							read_location_cb, csc);
		} else if (g_strcmp0(c->uuid, SC_CONTROL_POINT_UUID) == 0) {
			DBG("SC Control Point supported");
			csc->controlpoint_val_handle = c->value_handle;

			csc->attio_controlpoint_id = g_attrib_register(
					csc->attrib, ATT_OP_HANDLE_IND,
					c->value_handle,
					controlpoint_ind_handler, csc, NULL);

			discover_desc(csc, c, c_next);
		}
	}

	if (feature_val_handle > 0)
		gatt_read_char(csc->attrib, feature_val_handle,
							read_feature_cb, csc);
}

static void enable_measurement(gpointer data, gpointer user_data)
{
	struct csc *csc = data;
	uint16_t handle = csc->measurement_ccc_handle;
	uint8_t value[2];
	char *msg;

	if (csc->attrib == NULL || !handle)
		return;

	put_le16(GATT_CLIENT_CHARAC_CFG_NOTIF_BIT, value);
	msg = g_strdup("Enable measurement");

	gatt_write_char(csc->attrib, handle, value, sizeof(value),
							char_write_cb, msg);
}

static void disable_measurement(gpointer data, gpointer user_data)
{
	struct csc *csc = data;
	uint16_t handle = csc->measurement_ccc_handle;
	uint8_t value[2];
	char *msg;

	if (csc->attrib == NULL || !handle)
		return;

	put_le16(0x0000, value);
	msg = g_strdup("Disable measurement");

	gatt_write_char(csc->attrib, handle, value, sizeof(value),
							char_write_cb, msg);
}

static void attio_connected_cb(GAttrib *attrib, gpointer user_data)
{
	struct csc *csc = user_data;

	DBG("");

	csc->attrib = g_attrib_ref(attrib);

	gatt_discover_char(csc->attrib, csc->svc_range->start,
						csc->svc_range->end, NULL,
						discover_char_cb, csc);
}

static void attio_disconnected_cb(gpointer user_data)
{
	struct csc *csc = user_data;

	DBG("");

	if (csc->attio_measurement_id > 0) {
		g_attrib_unregister(csc->attrib, csc->attio_measurement_id);
		csc->attio_measurement_id = 0;
	}

	if (csc->attio_controlpoint_id > 0) {
		g_attrib_unregister(csc->attrib, csc->attio_controlpoint_id);
		csc->attio_controlpoint_id = 0;
	}

	g_attrib_unref(csc->attrib);
	csc->attrib = NULL;
}

static void watcher_exit_cb(DBusConnection *conn, void *user_data)
{
	struct watcher *watcher = user_data;
	struct csc_adapter *cadapter = watcher->cadapter;

	DBG("cycling watcher [%s] disconnected", watcher->path);

	cadapter->watchers = g_slist_remove(cadapter->watchers, watcher);
	g_dbus_remove_watch(conn, watcher->id);

	if (g_slist_length(cadapter->watchers) == 0)
		g_slist_foreach(cadapter->devices, disable_measurement, 0);
}

static DBusMessage *register_watcher(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	struct csc_adapter *cadapter = data;
	struct watcher *watcher;
	const char *sender = dbus_message_get_sender(msg);
	char *path;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &path,
							DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	watcher = find_watcher(cadapter->watchers, sender, path);
	if (watcher != NULL)
		return btd_error_already_exists(msg);

	watcher = g_new0(struct watcher, 1);
	watcher->cadapter = cadapter;
	watcher->id = g_dbus_add_disconnect_watch(conn, sender, watcher_exit_cb,
						watcher, destroy_watcher);
	watcher->srv = g_strdup(sender);
	watcher->path = g_strdup(path);

	if (g_slist_length(cadapter->watchers) == 0)
		g_slist_foreach(cadapter->devices, enable_measurement, 0);

	cadapter->watchers = g_slist_prepend(cadapter->watchers, watcher);

	DBG("cycling watcher [%s] registered", path);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *unregister_watcher(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	struct csc_adapter *cadapter = data;
	struct watcher *watcher;
	const char *sender = dbus_message_get_sender(msg);
	char *path;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &path,
							DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	watcher = find_watcher(cadapter->watchers, sender, path);
	if (watcher == NULL)
		return btd_error_does_not_exist(msg);

	cadapter->watchers = g_slist_remove(cadapter->watchers, watcher);
	g_dbus_remove_watch(conn, watcher->id);

	if (g_slist_length(cadapter->watchers) == 0)
		g_slist_foreach(cadapter->devices, disable_measurement, 0);

	DBG("cycling watcher [%s] unregistered", path);

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable cyclingspeed_manager_methods[] = {
	{ GDBUS_METHOD("RegisterWatcher",
			GDBUS_ARGS({ "agent", "o" }), NULL,
			register_watcher) },
	{ GDBUS_METHOD("UnregisterWatcher",
			GDBUS_ARGS({ "agent", "o" }), NULL,
			unregister_watcher) },
	{ }
};

static int csc_adapter_probe(struct btd_profile *p, struct btd_adapter *adapter)
{
	struct csc_adapter *cadapter;

	cadapter = g_new0(struct csc_adapter, 1);
	cadapter->adapter = adapter;

	if (!g_dbus_register_interface(btd_get_dbus_connection(),
						adapter_get_path(adapter),
						CYCLINGSPEED_MANAGER_INTERFACE,
						cyclingspeed_manager_methods,
						NULL, NULL, cadapter,
						destroy_csc_adapter)) {
		error("D-Bus failed to register %s interface",
						CYCLINGSPEED_MANAGER_INTERFACE);
		destroy_csc_adapter(cadapter);
		return -EIO;
	}

	csc_adapters = g_slist_prepend(csc_adapters, cadapter);

	return 0;
}

static void csc_adapter_remove(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	struct csc_adapter *cadapter;

	cadapter = find_csc_adapter(adapter);
	if (cadapter == NULL)
		return;

	csc_adapters = g_slist_remove(csc_adapters, cadapter);

	g_dbus_unregister_interface(btd_get_dbus_connection(),
					adapter_get_path(cadapter->adapter),
					CYCLINGSPEED_MANAGER_INTERFACE);
}

static gboolean property_get_location(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct csc *csc = data;
	const char *loc;

	if (!csc->has_location)
		return FALSE;

	loc = location2str(csc->location);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &loc);

	return TRUE;
}

static void property_set_location(const GDBusPropertyTable *property,
					DBusMessageIter *iter,
					GDBusPendingPropertySet id, void *data)
{
	struct csc *csc = data;
	char *loc;
	int loc_val;
	uint8_t att_val[2];
	struct controlpoint_req *req;

	if (csc->pending_req != NULL) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InProgress",
					"Operation already in progress");
		return;
	}

	if (!(csc->feature & MULTI_SENSOR_LOC_SUPPORT)) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".NotSupported",
					"Feature is not supported");
		return;
	}

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_STRING) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	dbus_message_iter_get_basic(iter, &loc);

	loc_val = str2location(loc);

	if (loc_val < 0) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	req = g_new(struct controlpoint_req, 1);
	req->csc = csc;
	req->reply_id = id;
	req->opcode = UPDATE_SENSOR_LOC;
	req->pending_location = loc_val;

	csc->pending_req = req;

	att_val[0] = UPDATE_SENSOR_LOC;
	att_val[1] = loc_val;

	gatt_write_char(csc->attrib, csc->controlpoint_val_handle, att_val,
				sizeof(att_val), controlpoint_write_cb, req);
}

static gboolean property_exists_location(const GDBusPropertyTable *property,
								void *data)
{
	struct csc *csc = data;

	return csc->has_location;
}

static gboolean property_get_locations(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct csc *csc = data;
	DBusMessageIter entry;
	int i;

	if (!(csc->feature & MULTI_SENSOR_LOC_SUPPORT))
		return FALSE;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_STRING_AS_STRING, &entry);
	for (i = 0; i < csc->num_locations; i++) {
		char *loc = g_strdup(location2str(csc->locations[i]));
		dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &loc);
		g_free(loc);
	}

	dbus_message_iter_close_container(iter, &entry);

	return TRUE;
}

static gboolean property_exists_locations(const GDBusPropertyTable *property,
								void *data)
{
	struct csc *csc = data;

	return !!(csc->feature & MULTI_SENSOR_LOC_SUPPORT);
}

static gboolean property_get_wheel_rev_sup(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct csc *csc = data;
	dbus_bool_t val;

	val = !!(csc->feature & WHEEL_REV_SUPPORT);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &val);

	return TRUE;
}

static gboolean property_get_multi_loc_sup(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct csc *csc = data;
	dbus_bool_t val;

	val = !!(csc->feature & MULTI_SENSOR_LOC_SUPPORT);
	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &val);

	return TRUE;
}

static const GDBusPropertyTable cyclingspeed_device_properties[] = {
	{ "Location", "s", property_get_location, property_set_location,
						property_exists_location },
	{ "SupportedLocations", "as", property_get_locations, NULL,
						property_exists_locations },
	{ "WheelRevolutionDataSupported", "b", property_get_wheel_rev_sup },
	{ "MultipleLocationsSupported", "b", property_get_multi_loc_sup },
	{ }
};

static DBusMessage *set_cumulative_wheel_rev(DBusConnection *conn,
						DBusMessage *msg, void *data)
{
	struct csc *csc = data;
	dbus_uint32_t value;
	struct controlpoint_req *req;
	uint8_t att_val[5]; /* uint8 opcode + uint32 value */

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_UINT32, &value,
							DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	if (csc->pending_req != NULL)
		return btd_error_in_progress(msg);

	req = g_new(struct controlpoint_req, 1);
	req->csc = csc;
	req->opcode = SET_CUMULATIVE_VALUE;
	req->msg = dbus_message_ref(msg);

	csc->pending_req = req;

	att_val[0] = SET_CUMULATIVE_VALUE;
	put_le32(value, att_val + 1);

	gatt_write_char(csc->attrib, csc->controlpoint_val_handle, att_val,
		sizeof(att_val), controlpoint_write_cb, req);

	return NULL;
}

static const GDBusMethodTable cyclingspeed_device_methods[] = {
	{ GDBUS_ASYNC_METHOD("SetCumulativeWheelRevolutions",
				GDBUS_ARGS({ "value", "u" }), NULL,
						set_cumulative_wheel_rev) },
	{ }
};

static int csc_device_probe(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct btd_adapter *adapter;
	struct csc_adapter *cadapter;
	struct csc *csc;
	struct gatt_primary *prim;

	prim = btd_device_get_primary(device, CYCLING_SC_UUID);
	if (prim == NULL)
		return -EINVAL;

	adapter = device_get_adapter(device);

	cadapter = find_csc_adapter(adapter);
	if (cadapter == NULL)
		return -1;

	csc = g_new0(struct csc, 1);
	csc->dev = btd_device_ref(device);
	csc->cadapter = cadapter;

	if (!g_dbus_register_interface(btd_get_dbus_connection(),
						device_get_path(device),
						CYCLINGSPEED_INTERFACE,
						cyclingspeed_device_methods,
						NULL,
						cyclingspeed_device_properties,
						csc, destroy_csc)) {
		error("D-Bus failed to register %s interface",
						CYCLINGSPEED_INTERFACE);
		destroy_csc(csc);
		return -EIO;
	}

	csc->svc_range = g_new0(struct att_range, 1);
	csc->svc_range->start = prim->range.start;
	csc->svc_range->end = prim->range.end;

	cadapter->devices = g_slist_prepend(cadapter->devices, csc);

	csc->attioid = btd_device_add_attio_callback(device, attio_connected_cb,
						attio_disconnected_cb, csc);

	return 0;
}

static void csc_device_remove(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct btd_adapter *adapter;
	struct csc_adapter *cadapter;
	struct csc *csc;
	GSList *l;

	adapter = device_get_adapter(device);

	cadapter = find_csc_adapter(adapter);
	if (cadapter == NULL)
		return;

	l = g_slist_find_custom(cadapter->devices, device, cmp_device);
	if (l == NULL)
		return;

	csc = l->data;

	cadapter->devices = g_slist_remove(cadapter->devices, csc);

	g_dbus_unregister_interface(btd_get_dbus_connection(),
						device_get_path(device),
						CYCLINGSPEED_INTERFACE);
}

static struct btd_profile cscp_profile = {
	.name		= "Cycling Speed and Cadence GATT Driver",
	.remote_uuid	= CYCLING_SC_UUID,

	.adapter_probe	= csc_adapter_probe,
	.adapter_remove	= csc_adapter_remove,

	.device_probe	= csc_device_probe,
	.device_remove	= csc_device_remove,
};

static int cyclingspeed_init(void)
{
	return btd_profile_register(&cscp_profile);
}

static void cyclingspeed_exit(void)
{
	btd_profile_unregister(&cscp_profile);
}

BLUETOOTH_PLUGIN_DEFINE(cyclingspeed, VERSION,
					BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
					cyclingspeed_init, cyclingspeed_exit)
