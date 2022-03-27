/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011 GSyC/LibreSoft, Universidad Rey Juan Carlos.
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

#include <stdbool.h>
#include <errno.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "gdbus/gdbus.h"

#include "src/plugin.h"
#include "src/dbus-common.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/profile.h"
#include "src/service.h"
#include "src/shared/util.h"
#include "src/error.h"
#include "src/log.h"
#include "attrib/gattrib.h"
#include "src/attio.h"
#include "attrib/att.h"
#include "attrib/gatt.h"

#define THERMOMETER_INTERFACE		"org.bluez.Thermometer1"
#define THERMOMETER_MANAGER_INTERFACE	"org.bluez.ThermometerManager1"
#define THERMOMETER_WATCHER_INTERFACE	"org.bluez.ThermometerWatcher1"

/* Temperature measurement flag fields */
#define TEMP_UNITS		0x01
#define TEMP_TIME_STAMP		0x02
#define TEMP_TYPE		0x04

#define FLOAT_MAX_MANTISSA	16777216 /* 2^24 */

#define VALID_RANGE_DESC_SIZE	4
#define TEMPERATURE_TYPE_SIZE	1
#define MEASUREMENT_INTERVAL_SIZE	2

struct thermometer_adapter {
	struct btd_adapter	*adapter;
	GSList			*devices;
	GSList			*fwatchers;	/* Final measurements */
	GSList			*iwatchers;	/* Intermediate measurements */
};

struct thermometer {
	struct btd_device		*dev;		/* Device reference */
	struct thermometer_adapter	*tadapter;
	GAttrib				*attrib;	/* GATT connection */
	struct att_range		*svc_range;	/* Thermometer range */
	guint				attioid;	/* Att watcher id */
	/* attio id for Temperature Measurement value indications */
	guint				attio_measurement_id;
	/* attio id for Intermediate Temperature value notifications */
	guint				attio_intermediate_id;
	/* attio id for Measurement Interval value indications */
	guint				attio_interval_id;
	gboolean			intermediate;
	uint8_t				type;
	uint16_t			interval;
	uint16_t			max;
	uint16_t			min;
	gboolean			has_type;
	gboolean			has_interval;

	uint16_t			measurement_ccc_handle;
	uint16_t			intermediate_ccc_handle;
	uint16_t			interval_val_handle;
};

struct characteristic {
	struct thermometer	*t;	/* Thermometer where the char belongs */
	char			uuid[MAX_LEN_UUID_STR + 1];
};

struct watcher {
	struct thermometer_adapter	*tadapter;
	guint				id;
	char				*srv;
	char				*path;
};

struct measurement {
	struct thermometer	*t;
	int16_t			exp;
	int32_t			mant;
	uint64_t		time;
	gboolean		suptime;
	char			*unit;
	char			*type;
	char			*value;
};

struct tmp_interval_data {
	struct thermometer	*thermometer;
	uint16_t		interval;
};

static GSList *thermometer_adapters = NULL;

static const char * const temp_type[] = {
	"<reserved>",
	"armpit",
	"body",
	"ear",
	"finger",
	"intestines",
	"mouth",
	"rectum",
	"toe",
	"tympanum"
};

static const char *temptype2str(uint8_t value)
{
	 if (value > 0 && value < G_N_ELEMENTS(temp_type))
		return temp_type[value];

	error("Temperature type %d reserved for future use", value);
	return NULL;
}

static void destroy_watcher(gpointer user_data)
{
	struct watcher *watcher = user_data;

	g_free(watcher->path);
	g_free(watcher->srv);
	g_free(watcher);
}

static void remove_watcher(gpointer user_data)
{
	struct watcher *watcher = user_data;

	g_dbus_remove_watch(btd_get_dbus_connection(), watcher->id);
}

static void destroy_thermometer(gpointer user_data)
{
	struct thermometer *t = user_data;

	if (t->attioid > 0)
		btd_device_remove_attio_callback(t->dev, t->attioid);

	if (t->attrib != NULL) {
		g_attrib_unregister(t->attrib, t->attio_measurement_id);
		g_attrib_unregister(t->attrib, t->attio_intermediate_id);
		g_attrib_unregister(t->attrib, t->attio_interval_id);
		g_attrib_unref(t->attrib);
	}

	btd_device_unref(t->dev);
	g_free(t->svc_range);
	g_free(t);
}

static void destroy_thermometer_adapter(gpointer user_data)
{
	struct thermometer_adapter *tadapter = user_data;

	if (tadapter->devices != NULL)
		g_slist_free_full(tadapter->devices, destroy_thermometer);

	if (tadapter->fwatchers != NULL)
		g_slist_free_full(tadapter->fwatchers, remove_watcher);

	g_free(tadapter);
}

static int cmp_adapter(gconstpointer a, gconstpointer b)
{
	const struct thermometer_adapter *tadapter = a;
	const struct btd_adapter *adapter = b;

	if (adapter == tadapter->adapter)
		return 0;

	return -1;
}

static int cmp_device(gconstpointer a, gconstpointer b)
{
	const struct thermometer *t = a;
	const struct btd_device *dev = b;

	if (dev == t->dev)
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

static struct thermometer_adapter *
find_thermometer_adapter(struct btd_adapter *adapter)
{
	GSList *l = g_slist_find_custom(thermometer_adapters, adapter,
								cmp_adapter);
	if (!l)
		return NULL;

	return l->data;
}

static void change_property(struct thermometer *t, const char *name,
							gpointer value) {
	if (g_strcmp0(name, "Intermediate") == 0) {
		gboolean *intermediate = value;
		if (t->intermediate == *intermediate)
			return;

		t->intermediate = *intermediate;
	} else if (g_strcmp0(name, "Interval") == 0) {
		uint16_t *interval = value;
		if (t->has_interval && t->interval == *interval)
			return;

		t->has_interval = TRUE;
		t->interval = *interval;
	} else if (g_strcmp0(name, "Maximum") == 0) {
		uint16_t *max = value;
		if (t->max == *max)
			return;

		t->max = *max;
	} else if (g_strcmp0(name, "Minimum") == 0) {
		uint16_t *min = value;
		if (t->min == *min)
			return;

		t->min = *min;
	} else {
		DBG("%s is not a thermometer property", name);
		return;
	}

	g_dbus_emit_property_changed(btd_get_dbus_connection(),
						device_get_path(t->dev),
						THERMOMETER_INTERFACE, name);
}

static void update_watcher(gpointer data, gpointer user_data)
{
	struct watcher *w = data;
	struct measurement *m = user_data;
	const char *path = device_get_path(m->t->dev);
	DBusMessageIter iter;
	DBusMessageIter dict;
	DBusMessage *msg;

	msg = dbus_message_new_method_call(w->srv, w->path,
				THERMOMETER_WATCHER_INTERFACE,
				"MeasurementReceived");
	if (msg == NULL)
		return;

	dbus_message_iter_init_append(msg, &iter);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_OBJECT_PATH , &path);

	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
			DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING
			DBUS_TYPE_STRING_AS_STRING DBUS_TYPE_VARIANT_AS_STRING
			DBUS_DICT_ENTRY_END_CHAR_AS_STRING, &dict);

	dict_append_entry(&dict, "Exponent", DBUS_TYPE_INT16, &m->exp);
	dict_append_entry(&dict, "Mantissa", DBUS_TYPE_INT32, &m->mant);
	dict_append_entry(&dict, "Unit", DBUS_TYPE_STRING, &m->unit);

	if (m->suptime)
		dict_append_entry(&dict, "Time", DBUS_TYPE_UINT64, &m->time);

	dict_append_entry(&dict, "Type", DBUS_TYPE_STRING, &m->type);
	dict_append_entry(&dict, "Measurement", DBUS_TYPE_STRING, &m->value);

	dbus_message_iter_close_container(&iter, &dict);

	dbus_message_set_no_reply(msg, TRUE);
	g_dbus_send_message(btd_get_dbus_connection(), msg);
}

static void recv_measurement(struct thermometer *t, struct measurement *m)
{
	GSList *wlist;

	m->t = t;

	if (g_strcmp0(m->value, "intermediate") == 0)
		wlist = t->tadapter->iwatchers;
	else
		wlist = t->tadapter->fwatchers;

	g_slist_foreach(wlist, update_watcher, m);
}

static void proc_measurement(struct thermometer *t, const uint8_t *pdu,
						uint16_t len, gboolean final)
{
	struct measurement m;
	const char *type = NULL;
	uint8_t flags;
	uint32_t raw;

	/* skip opcode and handle */
	pdu += 3;
	len -= 3;

	if (len < 1) {
		DBG("Mandatory flags are not provided");
		return;
	}

	memset(&m, 0, sizeof(m));

	flags = *pdu;

	if (flags & TEMP_UNITS)
		m.unit = "fahrenheit";
	else
		m.unit = "celsius";

	pdu++;
	len--;

	if (len < 4) {
		DBG("Mandatory temperature measurement value is not provided");
		return;
	}

	raw = get_le32(pdu);
	m.mant = raw & 0x00FFFFFF;
	m.exp = ((int32_t) raw) >> 24;

	if (m.mant & 0x00800000) {
		/* convert to C2 negative value */
		m.mant = m.mant - FLOAT_MAX_MANTISSA;
	}

	pdu += 4;
	len -= 4;

	if (flags & TEMP_TIME_STAMP) {
		struct tm ts;
		time_t time;

		if (len < 7) {
			DBG("Time stamp is not provided");
			return;
		}

		ts.tm_year = get_le16(pdu) - 1900;
		ts.tm_mon = *(pdu + 2) - 1;
		ts.tm_mday = *(pdu + 3);
		ts.tm_hour = *(pdu + 4);
		ts.tm_min = *(pdu + 5);
		ts.tm_sec = *(pdu + 6);
		ts.tm_isdst = -1;

		time = mktime(&ts);
		m.time = (uint64_t) time;
		m.suptime = TRUE;

		pdu += 7;
		len -= 7;
	}

	if (flags & TEMP_TYPE) {
		if (len < 1) {
			DBG("Temperature type is not provided");
			return;
		}

		type = temptype2str(*pdu);
	} else if (t->has_type) {
		type = temptype2str(t->type);
	}

	m.type = g_strdup(type);
	m.value = final ? "final" : "intermediate";

	recv_measurement(t, &m);
	g_free(m.type);
}


static void measurement_ind_handler(const uint8_t *pdu, uint16_t len,
							gpointer user_data)
{
	struct thermometer *t = user_data;
	uint8_t *opdu;
	uint16_t olen;
	size_t plen;

	if (len < 3) {
		DBG("Bad pdu received");
		return;
	}

	proc_measurement(t, pdu, len, TRUE);

	opdu = g_attrib_get_buffer(t->attrib, &plen);
	olen = enc_confirmation(opdu, plen);

	if (olen > 0)
		g_attrib_send(t->attrib, 0, opdu, olen, NULL, NULL, NULL);
}

static void intermediate_notify_handler(const uint8_t *pdu, uint16_t len,
							gpointer user_data)
{
	struct thermometer *t = user_data;

	if (len < 3) {
		DBG("Bad pdu received");
		return;
	}

	proc_measurement(t, pdu, len, FALSE);
}

static void interval_ind_handler(const uint8_t *pdu, uint16_t len,
							gpointer user_data)
{
	struct thermometer *t = user_data;
	uint16_t interval;
	uint8_t *opdu;
	uint16_t olen;
	size_t plen;

	if (len < 5) {
		DBG("Bad pdu received");
		return;
	}

	interval = get_le16(pdu + 3);
	change_property(t, "Interval", &interval);

	opdu = g_attrib_get_buffer(t->attrib, &plen);
	olen = enc_confirmation(opdu, plen);

	if (olen > 0)
		g_attrib_send(t->attrib, 0, opdu, olen, NULL, NULL, NULL);
}

static void valid_range_desc_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct thermometer *t = user_data;
	uint8_t value[VALID_RANGE_DESC_SIZE];
	uint16_t max, min;
	ssize_t vlen;

	if (status != 0) {
		DBG("Valid Range descriptor read failed: %s",
							att_ecode2str(status));
		return;
	}

	vlen = dec_read_resp(pdu, len, value, sizeof(value));
	if (vlen < 0) {
		DBG("Protocol error\n");
		return;
	}

	if (vlen < 4) {
		DBG("Invalid range received");
		return;
	}

	min = get_le16(&value[0]);
	max = get_le16(&value[2]);

	if (min == 0 || min > max) {
		DBG("Invalid range");
		return;
	}

	change_property(t, "Maximum", &max);
	change_property(t, "Minimum", &min);
}

static void write_ccc_cb(guint8 status, const guint8 *pdu,
						guint16 len, gpointer user_data)
{
	char *msg = user_data;

	if (status != 0)
		error("%s failed", msg);

	g_free(msg);
}

static void process_thermometer_desc(struct characteristic *ch, uint16_t uuid,
								uint16_t handle)
{
	uint8_t atval[2];
	uint16_t val;
	char *msg;

	if (uuid == GATT_CHARAC_VALID_RANGE_UUID) {
		if (g_strcmp0(ch->uuid, MEASUREMENT_INTERVAL_UUID) == 0)
			gatt_read_char(ch->t->attrib, handle,
						valid_range_desc_cb, ch->t);
		return;
	}

	if (uuid != GATT_CLIENT_CHARAC_CFG_UUID)
		return;

	if (g_strcmp0(ch->uuid, TEMPERATURE_MEASUREMENT_UUID) == 0) {
		ch->t->measurement_ccc_handle = handle;

		if (g_slist_length(ch->t->tadapter->fwatchers) == 0) {
			val = 0x0000;
			msg = g_strdup("Disable Temperature Measurement ind");
		} else {
			val = GATT_CLIENT_CHARAC_CFG_IND_BIT;
			msg = g_strdup("Enable Temperature Measurement ind");
		}
	} else if (g_strcmp0(ch->uuid, INTERMEDIATE_TEMPERATURE_UUID) == 0) {
		ch->t->intermediate_ccc_handle = handle;

		if (g_slist_length(ch->t->tadapter->iwatchers) == 0) {
			val = 0x0000;
			msg = g_strdup("Disable Intermediate Temperature noti");
		} else {
			val = GATT_CLIENT_CHARAC_CFG_NOTIF_BIT;
			msg = g_strdup("Enable Intermediate Temperature noti");
		}
	} else if (g_strcmp0(ch->uuid, MEASUREMENT_INTERVAL_UUID) == 0) {
		val = GATT_CLIENT_CHARAC_CFG_IND_BIT;
		msg = g_strdup("Enable Measurement Interval indication");
	} else {
		return;
	}

	put_le16(val, atval);
	gatt_write_char(ch->t->attrib, handle, atval, sizeof(atval),
							write_ccc_cb, msg);
}

static void discover_desc_cb(guint8 status, GSList *descs, gpointer user_data)
{
	struct characteristic *ch = user_data;

	if (status != 0) {
		error("Discover all characteristic descriptors failed [%s]: %s",
					ch->uuid, att_ecode2str(status));
		goto done;
	}

	for ( ; descs; descs = descs->next) {
		struct gatt_desc *desc = descs->data;

		process_thermometer_desc(ch, desc->uuid16, desc->handle);
	}

done:
	g_free(ch);
}

static void discover_desc(struct thermometer *t, struct gatt_char *c,
						struct gatt_char *c_next)
{
	struct characteristic *ch;
	uint16_t start, end;

	start = c->value_handle + 1;

	if (c_next != NULL) {
		if (start == c_next->handle)
			return;
		end = c_next->handle - 1;
	} else if (c->value_handle != t->svc_range->end) {
		end = t->svc_range->end;
	} else {
		return;
	}

	ch = g_new0(struct characteristic, 1);
	ch->t = t;
	memcpy(ch->uuid, c->uuid, sizeof(c->uuid));

	gatt_discover_desc(t->attrib, start, end, NULL, discover_desc_cb, ch);
}

static void read_temp_type_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct thermometer *t = user_data;
	uint8_t value[TEMPERATURE_TYPE_SIZE];
	ssize_t vlen;

	if (status != 0) {
		DBG("Temperature Type value read failed: %s",
							att_ecode2str(status));
		return;
	}

	vlen = dec_read_resp(pdu, len, value, sizeof(value));
	if (vlen < 0) {
		DBG("Protocol error.");
		return;
	}

	if (vlen != 1) {
		DBG("Invalid length for Temperature type");
		return;
	}

	t->has_type = TRUE;
	t->type = value[0];
}

static void read_interval_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct thermometer *t = user_data;
	uint8_t value[MEASUREMENT_INTERVAL_SIZE];
	uint16_t interval;
	ssize_t vlen;

	if (status != 0) {
		DBG("Measurement Interval value read failed: %s",
							att_ecode2str(status));
		return;
	}

	vlen = dec_read_resp(pdu, len, value, sizeof(value));
	if (vlen < 0) {
		DBG("Protocol error\n");
		return;
	}

	if (vlen < 2) {
		DBG("Invalid Interval received");
		return;
	}

	interval = get_le16(&value[0]);
	change_property(t, "Interval", &interval);
}

static void process_thermometer_char(struct thermometer *t,
				struct gatt_char *c, struct gatt_char *c_next)
{
	if (g_strcmp0(c->uuid, INTERMEDIATE_TEMPERATURE_UUID) == 0) {
		gboolean intermediate = TRUE;
		change_property(t, "Intermediate", &intermediate);

		t->attio_intermediate_id = g_attrib_register(t->attrib,
					ATT_OP_HANDLE_NOTIFY, c->value_handle,
					intermediate_notify_handler, t, NULL);

		discover_desc(t, c, c_next);
	} else if (g_strcmp0(c->uuid, TEMPERATURE_MEASUREMENT_UUID) == 0) {

		t->attio_measurement_id = g_attrib_register(t->attrib,
					ATT_OP_HANDLE_IND, c->value_handle,
					measurement_ind_handler, t, NULL);

		discover_desc(t, c, c_next);
	} else if (g_strcmp0(c->uuid, TEMPERATURE_TYPE_UUID) == 0) {
		gatt_read_char(t->attrib, c->value_handle,
							read_temp_type_cb, t);
	} else if (g_strcmp0(c->uuid, MEASUREMENT_INTERVAL_UUID) == 0) {
		bool need_desc = false;

		gatt_read_char(t->attrib, c->value_handle, read_interval_cb, t);

		if (c->properties & GATT_CHR_PROP_WRITE) {
			t->interval_val_handle = c->value_handle;
			need_desc = true;
		}

		if (c->properties & GATT_CHR_PROP_INDICATE) {
			t->attio_interval_id = g_attrib_register(t->attrib,
					ATT_OP_HANDLE_IND, c->value_handle,
					interval_ind_handler, t, NULL);
			need_desc = true;
		}

		if (need_desc)
			discover_desc(t, c, c_next);
	}
}

static void configure_thermometer_cb(uint8_t status, GSList *characteristics,
								void *user_data)
{
	struct thermometer *t = user_data;
	GSList *l;

	if (status != 0) {
		error("Discover thermometer characteristics: %s",
							att_ecode2str(status));
		return;
	}

	for (l = characteristics; l; l = l->next) {
		struct gatt_char *c = l->data;
		struct gatt_char *c_next = (l->next ? l->next->data : NULL);

		process_thermometer_char(t, c, c_next);
	}
}

static void write_interval_cb(guint8 status, const guint8 *pdu, guint16 len,
							gpointer user_data)
{
	struct tmp_interval_data *data = user_data;

	if (status != 0) {
		error("Interval Write Request failed %s",
							att_ecode2str(status));
		goto done;
	}

	if (!dec_write_resp(pdu, len)) {
		error("Interval Write Request: protocol error");
		goto done;
	}

	change_property(data->thermometer, "Interval", &data->interval);

done:
	g_free(user_data);
}

static void enable_final_measurement(gpointer data, gpointer user_data)
{
	struct thermometer *t = data;
	uint16_t handle = t->measurement_ccc_handle;
	uint8_t value[2];
	char *msg;

	if (t->attrib == NULL || !handle)
		return;

	put_le16(GATT_CLIENT_CHARAC_CFG_IND_BIT, value);
	msg = g_strdup("Enable Temperature Measurement indications");

	gatt_write_char(t->attrib, handle, value, sizeof(value),
							write_ccc_cb, msg);
}

static void enable_intermediate_measurement(gpointer data, gpointer user_data)
{
	struct thermometer *t = data;
	uint16_t handle = t->intermediate_ccc_handle;
	uint8_t value[2];
	char *msg;

	if (t->attrib == NULL || !handle)
		return;

	put_le16(GATT_CLIENT_CHARAC_CFG_NOTIF_BIT, value);
	msg = g_strdup("Enable Intermediate Temperature notifications");

	gatt_write_char(t->attrib, handle, value, sizeof(value),
							write_ccc_cb, msg);
}

static void disable_final_measurement(gpointer data, gpointer user_data)
{
	struct thermometer *t = data;
	uint16_t handle = t->measurement_ccc_handle;
	uint8_t value[2];
	char *msg;

	if (t->attrib == NULL || !handle)
		return;

	put_le16(0x0000, value);
	msg = g_strdup("Disable Temperature Measurement indications");

	gatt_write_char(t->attrib, handle, value, sizeof(value),
							write_ccc_cb, msg);
}

static void disable_intermediate_measurement(gpointer data, gpointer user_data)
{
	struct thermometer *t = data;
	uint16_t handle = t->intermediate_ccc_handle;
	uint8_t value[2];
	char *msg;

	if (t->attrib == NULL || !handle)
		return;

	put_le16(0x0000, value);
	msg = g_strdup("Disable Intermediate Temperature notifications");

	gatt_write_char(t->attrib, handle, value, sizeof(value),
							write_ccc_cb, msg);
}

static void remove_int_watcher(struct thermometer_adapter *tadapter,
							struct watcher *w)
{
	if (!g_slist_find(tadapter->iwatchers, w))
		return;

	tadapter->iwatchers = g_slist_remove(tadapter->iwatchers, w);

	if (g_slist_length(tadapter->iwatchers) == 0)
		g_slist_foreach(tadapter->devices,
					disable_intermediate_measurement, 0);
}

static void watcher_exit(DBusConnection *conn, void *user_data)
{
	struct watcher *watcher = user_data;
	struct thermometer_adapter *tadapter = watcher->tadapter;

	DBG("Thermometer watcher %s disconnected", watcher->path);

	remove_int_watcher(tadapter, watcher);

	tadapter->fwatchers = g_slist_remove(tadapter->fwatchers, watcher);
	g_dbus_remove_watch(btd_get_dbus_connection(), watcher->id);

	if (g_slist_length(tadapter->fwatchers) == 0)
		g_slist_foreach(tadapter->devices,
					disable_final_measurement, 0);
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

static DBusMessage *register_watcher(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	const char *sender = dbus_message_get_sender(msg);
	struct thermometer_adapter *tadapter = data;
	struct watcher *watcher;
	char *path;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &path,
							DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	watcher = find_watcher(tadapter->fwatchers, sender, path);
	if (watcher != NULL)
		return btd_error_already_exists(msg);

	DBG("Thermometer watcher %s registered", path);

	watcher = g_new0(struct watcher, 1);
	watcher->srv = g_strdup(sender);
	watcher->path = g_strdup(path);
	watcher->tadapter = tadapter;
	watcher->id = g_dbus_add_disconnect_watch(conn, sender, watcher_exit,
						watcher, destroy_watcher);

	if (g_slist_length(tadapter->fwatchers) == 0)
		g_slist_foreach(tadapter->devices, enable_final_measurement, 0);

	tadapter->fwatchers = g_slist_prepend(tadapter->fwatchers, watcher);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *unregister_watcher(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	const char *sender = dbus_message_get_sender(msg);
	struct thermometer_adapter *tadapter = data;
	struct watcher *watcher;
	char *path;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &path,
							DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	watcher = find_watcher(tadapter->fwatchers, sender, path);
	if (watcher == NULL)
		return btd_error_does_not_exist(msg);

	DBG("Thermometer watcher %s unregistered", path);

	remove_int_watcher(tadapter, watcher);

	tadapter->fwatchers = g_slist_remove(tadapter->fwatchers, watcher);
	g_dbus_remove_watch(btd_get_dbus_connection(), watcher->id);

	if (g_slist_length(tadapter->fwatchers) == 0)
		g_slist_foreach(tadapter->devices,
					disable_final_measurement, 0);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *enable_intermediate(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	const char *sender = dbus_message_get_sender(msg);
	struct thermometer_adapter *ta = data;
	struct watcher *watcher;
	char *path;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &path,
							DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	watcher = find_watcher(ta->fwatchers, sender, path);
	if (watcher == NULL)
		return btd_error_does_not_exist(msg);

	if (find_watcher(ta->iwatchers, sender, path))
		return btd_error_already_exists(msg);

	DBG("Intermediate measurement watcher %s registered", path);

	if (g_slist_length(ta->iwatchers) == 0)
		g_slist_foreach(ta->devices,
					enable_intermediate_measurement, 0);

	ta->iwatchers = g_slist_prepend(ta->iwatchers, watcher);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *disable_intermediate(DBusConnection *conn, DBusMessage *msg,
								void *data)
{
	const char *sender = dbus_message_get_sender(msg);
	struct thermometer_adapter *ta = data;
	struct watcher *watcher;
	char *path;

	if (!dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &path,
							DBUS_TYPE_INVALID))
		return btd_error_invalid_args(msg);

	watcher = find_watcher(ta->iwatchers, sender, path);
	if (watcher == NULL)
		return btd_error_does_not_exist(msg);

	DBG("Intermediate measurement %s unregistered", path);

	remove_int_watcher(ta, watcher);

	return dbus_message_new_method_return(msg);
}

static gboolean property_get_intermediate(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct thermometer *t = data;
	dbus_bool_t val;

	val = !!t->intermediate;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &val);

	return TRUE;
}

static gboolean property_get_interval(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct thermometer *t = data;

	if (!t->has_interval)
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16, &t->interval);

	return TRUE;
}

static void property_set_interval(const GDBusPropertyTable *property,
					DBusMessageIter *iter,
					GDBusPendingPropertySet id, void *data)
{
	struct thermometer *t = data;
	struct tmp_interval_data *interval_data;
	uint16_t val;
	uint8_t atval[2];

	if (t->interval_val_handle == 0) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".NotSupported",
					"Operation is not supported");
		return;
	}

	if (dbus_message_iter_get_arg_type(iter) != DBUS_TYPE_UINT16) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	dbus_message_iter_get_basic(iter, &val);

	if (val < t->min || val > t->max) {
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
		return;
	}

	put_le16(val, &atval[0]);

	interval_data = g_new0(struct tmp_interval_data, 1);
	interval_data->thermometer = t;
	interval_data->interval = val;
	gatt_write_char(t->attrib, t->interval_val_handle, atval, sizeof(atval),
					write_interval_cb, interval_data);

	g_dbus_pending_property_success(id);
}

static gboolean property_exists_interval(const GDBusPropertyTable *property,
								void *data)
{
	struct thermometer *t = data;

	return t->has_interval;
}

static gboolean property_get_maximum(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct thermometer *t = data;

	if (!t->has_interval)
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16, &t->max);

	return TRUE;
}

static gboolean property_get_minimum(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *data)
{
	struct thermometer *t = data;

	if (!t->has_interval)
		return FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16, &t->min);

	return TRUE;
}

static const GDBusPropertyTable thermometer_properties[] = {
	{ "Intermediate", "b", property_get_intermediate },
	{ "Interval", "q", property_get_interval, property_set_interval,
						property_exists_interval },
	{ "Maximum", "q", property_get_maximum, NULL,
						property_exists_interval },
	{ "Minimum", "q", property_get_minimum, NULL,
						property_exists_interval },
	{ }
};

static void attio_connected_cb(GAttrib *attrib, gpointer user_data)
{
	struct thermometer *t = user_data;

	t->attrib = g_attrib_ref(attrib);

	gatt_discover_char(t->attrib, t->svc_range->start, t->svc_range->end,
					NULL, configure_thermometer_cb, t);
}

static void attio_disconnected_cb(gpointer user_data)
{
	struct thermometer *t = user_data;

	DBG("GATT Disconnected");

	if (t->attio_measurement_id > 0) {
		g_attrib_unregister(t->attrib, t->attio_measurement_id);
		t->attio_measurement_id = 0;
	}

	if (t->attio_intermediate_id > 0) {
		g_attrib_unregister(t->attrib, t->attio_intermediate_id);
		t->attio_intermediate_id = 0;
	}

	if (t->attio_interval_id > 0) {
		g_attrib_unregister(t->attrib, t->attio_interval_id);
		t->attio_interval_id = 0;
	}

	g_attrib_unref(t->attrib);
	t->attrib = NULL;
}

static int thermometer_register(struct btd_device *device,
						struct gatt_primary *tattr)
{
	const char *path = device_get_path(device);
	struct thermometer *t;
	struct btd_adapter *adapter;
	struct thermometer_adapter *tadapter;

	adapter = device_get_adapter(device);

	tadapter = find_thermometer_adapter(adapter);

	if (tadapter == NULL)
		return -1;

	t = g_new0(struct thermometer, 1);
	t->dev = btd_device_ref(device);
	t->tadapter = tadapter;
	t->svc_range = g_new0(struct att_range, 1);
	t->svc_range->start = tattr->range.start;
	t->svc_range->end = tattr->range.end;

	tadapter->devices = g_slist_prepend(tadapter->devices, t);

	if (!g_dbus_register_interface(btd_get_dbus_connection(),
				path, THERMOMETER_INTERFACE,
				NULL, NULL, thermometer_properties,
				t, destroy_thermometer)) {
		error("D-Bus failed to register %s interface",
							THERMOMETER_INTERFACE);
		destroy_thermometer(t);
		return -EIO;
	}

	t->attioid = btd_device_add_attio_callback(device, attio_connected_cb,
						attio_disconnected_cb, t);
	return 0;
}

static void thermometer_unregister(struct btd_device *device)
{
	struct thermometer *t;
	struct btd_adapter *adapter;
	struct thermometer_adapter *tadapter;
	GSList *l;

	adapter = device_get_adapter(device);

	tadapter = find_thermometer_adapter(adapter);

	if (tadapter == NULL)
		return;

	l = g_slist_find_custom(tadapter->devices, device, cmp_device);
	if (l == NULL)
		return;

	t = l->data;

	tadapter->devices = g_slist_remove(tadapter->devices, t);

	g_dbus_unregister_interface(btd_get_dbus_connection(),
				device_get_path(t->dev), THERMOMETER_INTERFACE);
}

static const GDBusMethodTable thermometer_manager_methods[] = {
	{ GDBUS_METHOD("RegisterWatcher",
			GDBUS_ARGS({ "agent", "o" }), NULL,
			register_watcher) },
	{ GDBUS_METHOD("UnregisterWatcher",
			GDBUS_ARGS({ "agent", "o" }), NULL,
			unregister_watcher) },
	{ GDBUS_METHOD("EnableIntermediateMeasurement",
			GDBUS_ARGS({ "agent", "o" }), NULL,
			enable_intermediate) },
	{ GDBUS_METHOD("DisableIntermediateMeasurement",
			GDBUS_ARGS({ "agent", "o" }), NULL,
			disable_intermediate) },
	{ }
};

static int thermometer_adapter_register(struct btd_adapter *adapter)
{
	struct thermometer_adapter *tadapter;

	tadapter = g_new0(struct thermometer_adapter, 1);
	tadapter->adapter = adapter;

	if (!g_dbus_register_interface(btd_get_dbus_connection(),
						adapter_get_path(adapter),
						THERMOMETER_MANAGER_INTERFACE,
						thermometer_manager_methods,
						NULL, NULL, tadapter,
						destroy_thermometer_adapter)) {
		error("D-Bus failed to register %s interface",
						THERMOMETER_MANAGER_INTERFACE);
		destroy_thermometer_adapter(tadapter);
		return -EIO;
	}

	thermometer_adapters = g_slist_prepend(thermometer_adapters, tadapter);

	return 0;
}

static void thermometer_adapter_unregister(struct btd_adapter *adapter)
{
	struct thermometer_adapter *tadapter;

	tadapter = find_thermometer_adapter(adapter);
	if (tadapter == NULL)
		return;

	thermometer_adapters = g_slist_remove(thermometer_adapters, tadapter);

	g_dbus_unregister_interface(btd_get_dbus_connection(),
					adapter_get_path(tadapter->adapter),
					THERMOMETER_MANAGER_INTERFACE);
}

static int thermometer_device_probe(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);
	struct gatt_primary *tattr;

	tattr = btd_device_get_primary(device, HEALTH_THERMOMETER_UUID);
	if (tattr == NULL)
		return -EINVAL;

	return thermometer_register(device, tattr);
}

static void thermometer_device_remove(struct btd_service *service)
{
	struct btd_device *device = btd_service_get_device(service);

	thermometer_unregister(device);
}

static int thermometer_adapter_probe(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	return thermometer_adapter_register(adapter);
}

static void thermometer_adapter_remove(struct btd_profile *p,
						struct btd_adapter *adapter)
{
	thermometer_adapter_unregister(adapter);
}

static struct btd_profile thermometer_profile = {
	.name		= "Health Thermometer GATT driver",
	.remote_uuid	= HEALTH_THERMOMETER_UUID,
	.device_probe	= thermometer_device_probe,
	.device_remove	= thermometer_device_remove,
	.adapter_probe	= thermometer_adapter_probe,
	.adapter_remove	= thermometer_adapter_remove
};

static int thermometer_init(void)
{
	return btd_profile_register(&thermometer_profile);
}

static void thermometer_exit(void)
{
	btd_profile_unregister(&thermometer_profile);
}

BLUETOOTH_PLUGIN_DEFINE(thermometer, VERSION, BLUETOOTH_PLUGIN_PRIORITY_DEFAULT,
					thermometer_init, thermometer_exit)
