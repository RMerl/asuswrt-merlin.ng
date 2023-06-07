// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2020 Google LLC
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <glib.h>
#include <dbus/dbus.h>
#include <gdbus/gdbus.h>

#include "lib/bluetooth.h"
#include "lib/mgmt.h"

#include "adapter.h"
#include "dbus-common.h"
#include "device.h"
#include "log.h"
#include "src/error.h"
#include "src/shared/mgmt.h"
#include "src/shared/queue.h"
#include "src/shared/util.h"

#include "adv_monitor.h"

#define ADV_MONITOR_INTERFACE		"org.bluez.AdvertisementMonitor1"
#define ADV_MONITOR_MGR_INTERFACE	"org.bluez.AdvertisementMonitorManager1"

#define ADV_MONITOR_UNSET_RSSI		127	/* dBm */
#define ADV_MONITOR_MAX_RSSI		20	/* dBm */
#define ADV_MONITOR_MIN_RSSI		-127	/* dBm */
#define ADV_MONITOR_UNSET_TIMEOUT	0	/* second */
#define ADV_MONITOR_MIN_TIMEOUT		1	/* second */
#define ADV_MONITOR_MAX_TIMEOUT		300	/* second */
#define ADV_MONITOR_DEFAULT_LOW_TIMEOUT	5	/* second */
#define ADV_MONITOR_DEFAULT_HIGH_TIMEOUT 10	/* second */
#define ADV_MONITOR_UNSET_SAMPLING_PERIOD 256	/* 100 ms */
#define ADV_MONITOR_MAX_SAMPLING_PERIOD	255	/* 100 ms */
#define ADV_MONITOR_DEFAULT_SAMPLING_PERIOD 0	/* 100 ms */

struct btd_adv_monitor_manager {
	struct btd_adapter *adapter;
	struct mgmt *mgmt;
	uint16_t adapter_id;

	uint32_t supported_features;	/* MGMT_ADV_MONITOR_FEATURE_MASK_* */
	uint32_t enabled_features;	/* MGMT_ADV_MONITOR_FEATURE_MASK_* */
	uint16_t max_num_monitors;
	uint8_t max_num_patterns;

	struct queue *apps;	/* apps who registered for Adv monitoring */
};

struct adv_monitor_app {
	struct btd_adv_monitor_manager *manager;
	char *owner;
	char *path;

	DBusMessage *reg;
	GDBusClient *client;

	struct queue *monitors;
};

enum monitor_type {
	MONITOR_TYPE_NONE,
	MONITOR_TYPE_OR_PATTERNS,
};

enum monitor_state {
	MONITOR_STATE_NEW,	/* New but not yet init'ed with actual values */
	MONITOR_STATE_FAILED,	/* Failed to be init'ed */
	MONITOR_STATE_INITED,	/* Init'ed but not yet sent to kernel */
	MONITOR_STATE_ACTIVE,	/* Accepted by kernel */
	MONITOR_STATE_REMOVED,	/* Removed from kernel */
	MONITOR_STATE_RELEASED,	/* Dbus Object removed by app */
};

struct adv_monitor {
	struct adv_monitor_app *app;
	GDBusProxy *proxy;
	char *path;

	enum monitor_state state;	/* MONITOR_STATE_* */
	uint16_t monitor_handle;	/* Kernel Monitor Handle */

	int8_t high_rssi;		/* High RSSI threshold */
	uint16_t high_rssi_timeout;	/* High RSSI threshold timeout */
	int8_t low_rssi;		/* Low RSSI threshold */
	uint16_t low_rssi_timeout;	/* Low RSSI threshold timeout */
	uint16_t sampling_period;	/* Merge packets in the same timeslot.
					 * Currenly unimplemented in user space.
					 * Used only to pass data to kernel.
					 */
	struct queue *devices;		/* List of adv_monitor_device objects */

	enum monitor_type type;		/* MONITOR_TYPE_* */
	struct queue *patterns;		/* List of bt_ad_pattern objects */
};

/* Some data like last_seen, timer/timeout values need to be maintained
 * per device. struct adv_monitor_device maintains such data.
 */
struct adv_monitor_device {
	struct adv_monitor *monitor;
	struct btd_device *device;

	time_t high_rssi_first_seen;	/* Start time when RSSI climbs above
					 * the high RSSI threshold
					 */
	time_t low_rssi_first_seen;	/* Start time when RSSI drops below
					 * the low RSSI threshold
					 */
	time_t last_seen;		/* Time when last Adv was received */
	bool found;			/* State of the device - lost/found */
	guint lost_timer;		/* Timer to track if the device goes
					 * offline/out-of-range
					 */
};

struct app_match_data {
	const char *owner;
	const char *path;
};

struct adv_content_filter_info {
	struct bt_ad *ad;
	struct queue *matched_monitors;	/* List of matched monitors */
};

struct adv_rssi_filter_info {
	struct btd_device *device;
	int8_t rssi;
};

static void monitor_device_free(void *data);
static void adv_monitor_filter_rssi(struct adv_monitor *monitor,
					struct btd_device *device, int8_t rssi);

const struct adv_monitor_type {
	enum monitor_type type;
	const char *name;
} supported_types[] = {
	{ MONITOR_TYPE_OR_PATTERNS, "or_patterns" },
	{ },
};

/* Replies to an app's D-Bus message and unref it */
static void app_reply_msg(struct adv_monitor_app *app, DBusMessage *reply)
{
	if (!app || !app->reg || !reply)
		return;

	g_dbus_send_message(btd_get_dbus_connection(), reply);
	dbus_message_unref(app->reg);
	app->reg = NULL;
}

/* Frees a pattern */
static void pattern_free(void *data)
{
	struct bt_ad_pattern *pattern = data;

	free(pattern);
}

/* Frees a monitor object */
static void monitor_free(struct adv_monitor *monitor)
{
	g_dbus_proxy_unref(monitor->proxy);
	g_free(monitor->path);

	queue_destroy(monitor->devices, monitor_device_free);
	monitor->devices = NULL;

	queue_destroy(monitor->patterns, pattern_free);

	free(monitor);
}

/* Calls Release() method of the remote Adv Monitor */
static void monitor_release(struct adv_monitor *monitor)
{
	/* Release() method on a monitor can be called when -
	 * 1. monitor initialization failed
	 * 2. app calls UnregisterMonitor and monitors held by app are released
	 * 3. monitor is removed by kernel
	 */
	if (monitor->state != MONITOR_STATE_FAILED &&
	    monitor->state != MONITOR_STATE_ACTIVE &&
	    monitor->state != MONITOR_STATE_REMOVED) {
		return;
	}

	DBG("Calling Release() on Adv Monitor of owner %s at path %s",
		monitor->app->owner, monitor->path);

	g_dbus_proxy_method_call(monitor->proxy, "Release", NULL, NULL, NULL,
					NULL);
}

/* Handles the callback of Remove Adv Monitor command */
static void remove_adv_monitor_cb(uint8_t status, uint16_t length,
				const void *param, void *user_data)
{
	const struct mgmt_rp_remove_adv_monitor *rp = param;

	if (status != MGMT_STATUS_SUCCESS || !param) {
		error("Failed to Remove Adv Monitor with status 0x%02x",
				status);
		return;
	}

	if (length < sizeof(*rp)) {
		error("Wrong size of Remove Adv Monitor response");
		return;
	}

	DBG("Adv monitor with handle:0x%04x removed from kernel",
		le16_to_cpu(rp->monitor_handle));
}

/* Sends Remove Adv Monitor command to the kernel */
static void monitor_remove(struct adv_monitor *monitor)
{
	struct adv_monitor_app *app = monitor->app;
	uint16_t adapter_id = app->manager->adapter_id;
	struct mgmt_cp_remove_adv_monitor cp;

	/* Monitor from kernel can be removed when -
	 * 1. already activated monitor object is deleted by app
	 * 2. app is destroyed and monitors held by app are marked as released
	 */
	if (monitor->state != MONITOR_STATE_ACTIVE &&
	    monitor->state != MONITOR_STATE_RELEASED) {
		return;
	}

	monitor->state = MONITOR_STATE_REMOVED;

	cp.monitor_handle = cpu_to_le16(monitor->monitor_handle);

	if (!mgmt_send(app->manager->mgmt, MGMT_OP_REMOVE_ADV_MONITOR,
			adapter_id, sizeof(cp), &cp, remove_adv_monitor_cb,
			app->manager, NULL)) {
		btd_error(adapter_id,
				"Unable to send Remove Advt Monitor command");
	}
}

/* Destroys monitor object */
static void monitor_destroy(void *data)
{
	struct adv_monitor *monitor = data;

	if (!monitor)
		return;

	queue_remove(monitor->app->monitors, monitor);

	monitor_release(monitor);
	monitor_remove(monitor);
	monitor_free(monitor);
}

/* Destroys an app object along with related D-Bus handlers */
static void app_destroy(void *data)
{
	struct adv_monitor_app *app = data;

	if (!app)
		return;

	DBG("Destroy Adv Monitor app %s at path %s", app->owner, app->path);

	queue_destroy(app->monitors, monitor_destroy);

	if (app->reg) {
		app_reply_msg(app, btd_error_failed(app->reg,
						"Adv Monitor app destroyed"));
	}

	if (app->client) {
		g_dbus_client_set_disconnect_watch(app->client, NULL, NULL);
		g_dbus_client_set_proxy_handlers(app->client, NULL, NULL, NULL,
							NULL);
		g_dbus_client_set_ready_watch(app->client, NULL, NULL);
		g_dbus_client_unref(app->client);
	}

	g_free(app->owner);
	g_free(app->path);

	free(app);
}

/* Updates monitor state to 'released' */
static void monitor_state_released(void *data, void *user_data)
{
	struct adv_monitor *monitor = data;

	if (!monitor && monitor->state != MONITOR_STATE_ACTIVE)
		return;

	monitor->state = MONITOR_STATE_RELEASED;
}

/* Handles a D-Bus disconnection event of an app */
static void app_disconnect_cb(DBusConnection *conn, void *user_data)
{
	struct adv_monitor_app *app = user_data;

	if (!app) {
		error("Unexpected NULL app object upon app disconnect");
		return;
	}

	btd_info(app->manager->adapter_id,
			"Adv Monitor app %s disconnected from D-Bus",
			app->owner);

	if (queue_remove(app->manager->apps, app)) {
		queue_foreach(app->monitors, monitor_state_released, NULL);
		app_destroy(app);
	}
}

/* Handles the ready signal of Adv Monitor app */
static void app_ready_cb(GDBusClient *client, void *user_data)
{
	struct adv_monitor_app *app = user_data;
	uint16_t adapter_id = app->manager->adapter_id;

	btd_info(adapter_id, "Path %s reserved for Adv Monitor app %s",
			app->path, app->owner);

	app_reply_msg(app, dbus_message_new_method_return(app->reg));
}

/* Allocates an Adv Monitor */
static struct adv_monitor *monitor_new(struct adv_monitor_app *app,
						GDBusProxy *proxy)
{
	struct adv_monitor *monitor;

	if (!app || !proxy)
		return NULL;

	monitor = new0(struct adv_monitor, 1);
	if (!monitor)
		return NULL;

	monitor->app = app;
	monitor->proxy = g_dbus_proxy_ref(proxy);
	monitor->path = g_strdup(g_dbus_proxy_get_path(proxy));

	monitor->state = MONITOR_STATE_NEW;

	monitor->high_rssi = ADV_MONITOR_UNSET_RSSI;
	monitor->high_rssi_timeout = ADV_MONITOR_UNSET_TIMEOUT;
	monitor->low_rssi = ADV_MONITOR_UNSET_RSSI;
	monitor->low_rssi_timeout = ADV_MONITOR_UNSET_TIMEOUT;
	monitor->sampling_period = ADV_MONITOR_UNSET_SAMPLING_PERIOD;
	monitor->devices = queue_new();

	monitor->type = MONITOR_TYPE_NONE;
	monitor->patterns = NULL;

	return monitor;
}

/* Matches a monitor based on its D-Bus path */
static bool monitor_match(const void *a, const void *b)
{
	const GDBusProxy *proxy = b;
	const struct adv_monitor *monitor = a;

	if (!proxy || !monitor)
		return false;

	if (g_strcmp0(g_dbus_proxy_get_path(proxy), monitor->path) != 0)
		return false;

	return true;
}

/* Retrieves Type from the remote Adv Monitor object, verifies the value and
 * update the local Adv Monitor
 */
static bool parse_monitor_type(struct adv_monitor *monitor, const char *path)
{
	DBusMessageIter iter;
	const struct adv_monitor_type *t;
	const char *type_str;
	uint16_t adapter_id = monitor->app->manager->adapter_id;

	if (!g_dbus_proxy_get_property(monitor->proxy, "Type", &iter)) {
		btd_error(adapter_id,
				"Failed to retrieve property Type from the "
				"Adv Monitor at path %s", path);
		return false;
	}

	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING)
		goto failed;

	dbus_message_iter_get_basic(&iter, &type_str);

	for (t = supported_types; t->name; t++) {
		if (strcmp(t->name, type_str) == 0) {
			monitor->type = t->type;
			return true;
		}
	}

failed:
	btd_error(adapter_id,
			"Invalid argument of property Type of the Adv Monitor "
			"at path %s", path);

	return false;
}

/* Retrieves RSSI thresholds and timeouts from the remote Adv Monitor object,
 * verifies the values and update the local Adv Monitor
 */
static bool parse_rssi_and_timeout(struct adv_monitor *monitor,
					const char *path)
{
	DBusMessageIter iter;
	GDBusProxy *proxy = monitor->proxy;
	int16_t h_rssi = ADV_MONITOR_UNSET_RSSI;
	int16_t l_rssi = ADV_MONITOR_UNSET_RSSI;
	uint16_t h_rssi_timeout = ADV_MONITOR_UNSET_TIMEOUT;
	uint16_t l_rssi_timeout = ADV_MONITOR_UNSET_TIMEOUT;
	int16_t sampling_period = ADV_MONITOR_UNSET_SAMPLING_PERIOD;
	uint16_t adapter_id = monitor->app->manager->adapter_id;

	/* Extract RSSIHighThreshold */
	if (g_dbus_proxy_get_property(proxy, "RSSIHighThreshold", &iter)) {
		if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_INT16)
			goto failed;
		dbus_message_iter_get_basic(&iter, &h_rssi);
	}

	/* Extract RSSIHighTimeout */
	if (g_dbus_proxy_get_property(proxy, "RSSIHighTimeout", &iter)) {
		if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_UINT16)
			goto failed;
		dbus_message_iter_get_basic(&iter, &h_rssi_timeout);
	}

	/* Extract RSSILowThreshold */
	if (g_dbus_proxy_get_property(proxy, "RSSILowThreshold", &iter)) {
		if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_INT16)
			goto failed;
		dbus_message_iter_get_basic(&iter, &l_rssi);
	}

	/* Extract RSSILowTimeout */
	if (g_dbus_proxy_get_property(proxy, "RSSILowTimeout", &iter)) {
		if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_UINT16)
			goto failed;
		dbus_message_iter_get_basic(&iter, &l_rssi_timeout);
	}

	/* Extract RSSISamplingPeriod */
	if (g_dbus_proxy_get_property(proxy, "RSSISamplingPeriod", &iter)) {
		if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_UINT16)
			goto failed;
		dbus_message_iter_get_basic(&iter, &sampling_period);
	}

	/* Verify the values of RSSIs and their timeouts. All fields should be
	 * either set to the unset values or are set within valid ranges.
	 * If the fields are only partially set, we would try our best to fill
	 * in with some sane values.
	 */
	if (h_rssi == ADV_MONITOR_UNSET_RSSI &&
		l_rssi == ADV_MONITOR_UNSET_RSSI &&
		h_rssi_timeout == ADV_MONITOR_UNSET_TIMEOUT &&
		l_rssi_timeout == ADV_MONITOR_UNSET_TIMEOUT &&
		sampling_period == ADV_MONITOR_UNSET_SAMPLING_PERIOD) {
		goto done;
	}

	if (l_rssi == ADV_MONITOR_UNSET_RSSI)
		l_rssi = ADV_MONITOR_MIN_RSSI;

	if (h_rssi == ADV_MONITOR_UNSET_RSSI)
		h_rssi = l_rssi;

	if (l_rssi_timeout == ADV_MONITOR_UNSET_TIMEOUT)
		l_rssi_timeout = ADV_MONITOR_DEFAULT_LOW_TIMEOUT;

	if (h_rssi_timeout == ADV_MONITOR_UNSET_TIMEOUT)
		h_rssi_timeout = ADV_MONITOR_DEFAULT_HIGH_TIMEOUT;

	if (sampling_period == ADV_MONITOR_UNSET_SAMPLING_PERIOD)
		sampling_period = ADV_MONITOR_DEFAULT_SAMPLING_PERIOD;

	if (h_rssi < ADV_MONITOR_MIN_RSSI || h_rssi > ADV_MONITOR_MAX_RSSI ||
		l_rssi < ADV_MONITOR_MIN_RSSI ||
		l_rssi > ADV_MONITOR_MAX_RSSI || h_rssi < l_rssi) {
		goto failed;
	}

	if (h_rssi_timeout < ADV_MONITOR_MIN_TIMEOUT ||
		h_rssi_timeout > ADV_MONITOR_MAX_TIMEOUT ||
		l_rssi_timeout < ADV_MONITOR_MIN_TIMEOUT ||
		l_rssi_timeout > ADV_MONITOR_MAX_TIMEOUT) {
		goto failed;
	}

	if (sampling_period > ADV_MONITOR_MAX_SAMPLING_PERIOD)
		goto failed;

	monitor->high_rssi = h_rssi;
	monitor->low_rssi = l_rssi;
	monitor->high_rssi_timeout = h_rssi_timeout;
	monitor->low_rssi_timeout = l_rssi_timeout;
	monitor->sampling_period = sampling_period;

done:
	DBG("Adv Monitor at %s initiated with high RSSI threshold %d, high "
		"RSSI threshold timeout %d, low RSSI threshold %d, low RSSI "
		"threshold timeout %d, sampling period %d", path,
		monitor->high_rssi, monitor->high_rssi_timeout,
		monitor->low_rssi, monitor->low_rssi_timeout,
		monitor->sampling_period);

	return true;

failed:
	btd_error(adapter_id,
			"Invalid argument of RSSI thresholds and timeouts "
			"of the Adv Monitor at path %s",
			path);

	return false;
}

/* Retrieves Patterns from the remote Adv Monitor object, verifies the values
 * and update the local Adv Monitor
 */
static bool parse_patterns(struct adv_monitor *monitor, const char *path)
{
	DBusMessageIter array, array_iter;
	uint16_t adapter_id = monitor->app->manager->adapter_id;

	if (!g_dbus_proxy_get_property(monitor->proxy, "Patterns", &array)) {
		btd_error(adapter_id,
				"Failed to retrieve property Patterns from the "
				"Adv Monitor at path %s", path);
		return false;
	}

	monitor->patterns = queue_new();

	if (dbus_message_iter_get_arg_type(&array) != DBUS_TYPE_ARRAY ||
		dbus_message_iter_get_element_type(&array) !=
		DBUS_TYPE_STRUCT) {
		goto failed;
	}

	dbus_message_iter_recurse(&array, &array_iter);

	while (dbus_message_iter_get_arg_type(&array_iter) ==
		DBUS_TYPE_STRUCT) {
		int value_len;
		uint8_t *value;
		uint8_t offset, ad_type;
		struct bt_ad_pattern *pattern;
		DBusMessageIter struct_iter, value_iter;

		dbus_message_iter_recurse(&array_iter, &struct_iter);

		// Extract start position
		if (dbus_message_iter_get_arg_type(&struct_iter) !=
			DBUS_TYPE_BYTE) {
			goto failed;
		}
		dbus_message_iter_get_basic(&struct_iter, &offset);
		if (!dbus_message_iter_next(&struct_iter))
			goto failed;

		// Extract AD data type
		if (dbus_message_iter_get_arg_type(&struct_iter) !=
			DBUS_TYPE_BYTE) {
			goto failed;
		}
		dbus_message_iter_get_basic(&struct_iter, &ad_type);
		if (!dbus_message_iter_next(&struct_iter))
			goto failed;

		// Extract value of a pattern
		if (dbus_message_iter_get_arg_type(&struct_iter) !=
			DBUS_TYPE_ARRAY) {
			goto failed;
		}
		dbus_message_iter_recurse(&struct_iter, &value_iter);
		dbus_message_iter_get_fixed_array(&value_iter, &value,
							&value_len);

		pattern = bt_ad_pattern_new(ad_type, offset, value_len, value);
		if (!pattern)
			goto failed;

		queue_push_tail(monitor->patterns, pattern);

		dbus_message_iter_next(&array_iter);
	}

	/* There must be at least one pattern. */
	if (queue_isempty(monitor->patterns))
		goto failed;

	return true;

failed:
	queue_destroy(monitor->patterns, pattern_free);
	monitor->patterns = NULL;

	btd_error(adapter_id, "Invalid argument of property Patterns of the "
			"Adv Monitor at path %s", path);

	return false;
}

/* Processes the content of the remote Adv Monitor */
static bool monitor_process(struct adv_monitor *monitor,
				struct adv_monitor_app *app)
{
	const char *path = g_dbus_proxy_get_path(monitor->proxy);

	monitor->state = MONITOR_STATE_FAILED;

	if (!parse_monitor_type(monitor, path))
		goto done;

	if (!parse_rssi_and_timeout(monitor, path))
		goto done;

	if (monitor->type == MONITOR_TYPE_OR_PATTERNS &&
		parse_patterns(monitor, path)) {
		monitor->state = MONITOR_STATE_INITED;
	}

done:
	return monitor->state != MONITOR_STATE_FAILED;
}

/* Handles the callback of Add Adv Patterns Monitor command */
static void add_adv_patterns_monitor_cb(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_add_adv_patterns_monitor *rp = param;
	struct adv_monitor *monitor = user_data;
	uint16_t adapter_id = monitor->app->manager->adapter_id;

	if (status != MGMT_STATUS_SUCCESS || !param) {
		btd_error(adapter_id,
				"Failed to Add Adv Patterns Monitor with status"
				" 0x%02x", status);
		monitor->state = MONITOR_STATE_FAILED;
		monitor_destroy(monitor);
		return;
	}

	if (length < sizeof(*rp)) {
		btd_error(adapter_id, "Wrong size of Add Adv Patterns Monitor "
				"response");
		monitor->state = MONITOR_STATE_FAILED;
		monitor_destroy(monitor);
		return;
	}

	monitor->monitor_handle = le16_to_cpu(rp->monitor_handle);
	monitor->state = MONITOR_STATE_ACTIVE;

	DBG("Calling Activate() on Adv Monitor of owner %s at path %s",
		monitor->app->owner, monitor->path);

	g_dbus_proxy_method_call(monitor->proxy, "Activate", NULL, NULL, NULL,
					NULL);

	DBG("Adv monitor with handle:0x%04x added", monitor->monitor_handle);
}

static bool monitor_rssi_is_unset(struct adv_monitor *monitor)
{
	return monitor->high_rssi == ADV_MONITOR_UNSET_RSSI &&
		monitor->low_rssi == ADV_MONITOR_UNSET_RSSI &&
		monitor->high_rssi_timeout == ADV_MONITOR_UNSET_TIMEOUT &&
		monitor->low_rssi_timeout == ADV_MONITOR_UNSET_TIMEOUT &&
		monitor->sampling_period == ADV_MONITOR_UNSET_SAMPLING_PERIOD;
}

/* sends MGMT_OP_ADD_ADV_PATTERNS_MONITOR */
static bool monitor_send_add_pattern(struct adv_monitor *monitor)
{
	struct mgmt_cp_add_adv_monitor *cp = NULL;
	uint8_t pattern_count, cp_len;
	const struct queue_entry *e;
	bool success = true;

	pattern_count = queue_length(monitor->patterns);
	cp_len = sizeof(*cp) + pattern_count * sizeof(struct mgmt_adv_pattern);

	cp = malloc0(cp_len);
	if (!cp)
		return false;

	for (e = queue_get_entries(monitor->patterns); e; e = e->next) {
		struct bt_ad_pattern *pattern = e->data;

		memcpy(&cp->patterns[cp->pattern_count++], pattern,
							sizeof(*pattern));
	}

	if (!mgmt_send(monitor->app->manager->mgmt,
			MGMT_OP_ADD_ADV_PATTERNS_MONITOR,
			monitor->app->manager->adapter_id, cp_len, cp,
			add_adv_patterns_monitor_cb, monitor, NULL)) {
		error("Unable to send Add Adv Patterns Monitor command");
		success = false;
	}

	free(cp);
	return success;
}

/* sends MGMT_OP_ADD_ADV_PATTERNS_MONITOR_RSSI */
static bool monitor_send_add_pattern_rssi(struct adv_monitor *monitor)
{
	struct mgmt_cp_add_adv_patterns_monitor_rssi *cp = NULL;
	uint8_t pattern_count, cp_len;
	const struct queue_entry *e;
	bool success = true;

	pattern_count = queue_length(monitor->patterns);
	cp_len = sizeof(*cp) + pattern_count * sizeof(struct mgmt_adv_pattern);

	cp = malloc0(cp_len);
	if (!cp)
		return false;

	cp->rssi.high_threshold = monitor->high_rssi;
	/* High threshold timeout is unsupported in kernel. Value must be 0. */
	cp->rssi.high_threshold_timeout = 0;
	cp->rssi.low_threshold = monitor->low_rssi;
	cp->rssi.low_threshold_timeout = htobs(monitor->low_rssi_timeout);
	cp->rssi.sampling_period = monitor->sampling_period;

	for (e = queue_get_entries(monitor->patterns); e; e = e->next) {
		struct bt_ad_pattern *pattern = e->data;

		memcpy(&cp->patterns[cp->pattern_count++], pattern,
							sizeof(*pattern));
	}

	if (!mgmt_send(monitor->app->manager->mgmt,
			MGMT_OP_ADD_ADV_PATTERNS_MONITOR_RSSI,
			monitor->app->manager->adapter_id, cp_len, cp,
			add_adv_patterns_monitor_cb, monitor, NULL)) {
		error("Unable to send Add Adv Patterns Monitor RSSI command");
		success = false;
	}

	free(cp);
	return success;
}

/* Handles an Adv Monitor D-Bus proxy added event */
static void monitor_proxy_added_cb(GDBusProxy *proxy, void *user_data)
{
	struct adv_monitor *monitor;
	struct adv_monitor_app *app = user_data;
	uint16_t adapter_id = app->manager->adapter_id;
	const char *path = g_dbus_proxy_get_path(proxy);
	const char *iface = g_dbus_proxy_get_interface(proxy);

	if (strcmp(iface, ADV_MONITOR_INTERFACE) != 0 ||
		!g_str_has_prefix(path, app->path)) {
		return;
	}

	if (queue_find(app->monitors, monitor_match, proxy)) {
		btd_error(adapter_id,
				"Adv Monitor proxy already exists with path %s",
				path);
		return;
	}

	monitor = monitor_new(app, proxy);
	if (!monitor) {
		btd_error(adapter_id,
				"Failed to allocate an Adv Monitor for the "
				"object at %s", path);
		return;
	}

	if (!monitor_process(monitor, app)) {
		monitor_destroy(monitor);
		DBG("Adv Monitor at path %s released due to invalid content",
			path);
		return;
	}

	queue_push_tail(app->monitors, monitor);

	if (monitor_rssi_is_unset(monitor))
		monitor_send_add_pattern(monitor);
	else
		monitor_send_add_pattern_rssi(monitor);

	DBG("Adv Monitor allocated for the object at path %s", path);
}

/* Handles the removal of an Adv Monitor D-Bus proxy */
static void monitor_proxy_removed_cb(GDBusProxy *proxy, void *user_data)
{
	struct adv_monitor *monitor;
	struct adv_monitor_app *app = user_data;

	monitor = queue_find(app->monitors, monitor_match, proxy);

	if (!monitor)
		return;

	DBG("Adv Monitor removed in state %02x with path %s", monitor->state,
		monitor->path);

	monitor_state_released(monitor, NULL);
	monitor_destroy(monitor);
}

/* Creates an app object, initiates it and sets D-Bus event handlers */
static struct adv_monitor_app *app_create(DBusConnection *conn,
					DBusMessage *msg, const char *sender,
					const char *path,
					struct btd_adv_monitor_manager *manager)
{
	struct adv_monitor_app *app;

	if (!path || !sender || !manager)
		return NULL;

	app = new0(struct adv_monitor_app, 1);
	if (!app)
		return NULL;

	app->owner = g_strdup(sender);
	app->path = g_strdup(path);
	app->manager = manager;
	app->reg = NULL;

	app->client = g_dbus_client_new_full(conn, sender, path, path);
	if (!app->client) {
		app_destroy(app);
		return NULL;
	}

	app->monitors = queue_new();

	app->reg = dbus_message_ref(msg);

	g_dbus_client_set_disconnect_watch(app->client, app_disconnect_cb, app);

	/* Note that any property changes on a monitor object would not affect
	 * the content of the corresponding monitor.
	 */
	g_dbus_client_set_proxy_handlers(app->client, monitor_proxy_added_cb,
						monitor_proxy_removed_cb, NULL,
						app);

	g_dbus_client_set_ready_watch(app->client, app_ready_cb, app);

	return app;
}

/* Matches an app based on its owner and path */
static bool app_match(const void *a, const void *b)
{
	const struct adv_monitor_app *app = a;
	const struct app_match_data *match = b;

	if (match->owner && strcmp(app->owner, match->owner))
		return false;

	if (match->path && strcmp(app->path, match->path))
		return false;

	return true;
}

/* Handles a RegisterMonitor D-Bus call */
static DBusMessage *register_monitor(DBusConnection *conn, DBusMessage *msg,
					void *user_data)
{
	DBusMessageIter args;
	struct app_match_data match;
	struct adv_monitor_app *app;
	struct btd_adv_monitor_manager *manager = user_data;

	if (!dbus_message_iter_init(msg, &args))
		return btd_error_invalid_args(msg);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_OBJECT_PATH)
		return btd_error_invalid_args(msg);

	dbus_message_iter_get_basic(&args, &match.path);

	if (!strlen(match.path) || !g_str_has_prefix(match.path, "/"))
		return btd_error_invalid_args(msg);

	match.owner = dbus_message_get_sender(msg);

	if (queue_find(manager->apps, app_match, &match))
		return btd_error_already_exists(msg);

	app = app_create(conn, msg, match.owner, match.path, manager);
	if (!app) {
		btd_error(manager->adapter_id,
				"Failed to reserve %s for Adv Monitor app %s",
				match.path, match.owner);
		return btd_error_failed(msg,
					"Failed to create Adv Monitor app");
	}

	queue_push_tail(manager->apps, app);

	return NULL;
}

/* Handles UnregisterMonitor D-Bus call */
static DBusMessage *unregister_monitor(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	DBusMessageIter args;
	struct app_match_data match;
	struct adv_monitor_app *app;
	struct btd_adv_monitor_manager *manager = user_data;

	if (!dbus_message_iter_init(msg, &args))
		return btd_error_invalid_args(msg);

	if (dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_OBJECT_PATH)
		return btd_error_invalid_args(msg);

	dbus_message_iter_get_basic(&args, &match.path);

	if (!strlen(match.path) || !g_str_has_prefix(match.path, "/"))
		return btd_error_invalid_args(msg);

	match.owner = dbus_message_get_sender(msg);

	app = queue_find(manager->apps, app_match, &match);
	if (!app)
		return btd_error_does_not_exist(msg);

	queue_remove(manager->apps, app);
	app_destroy(app);

	btd_info(manager->adapter_id,
			"Path %s removed along with Adv Monitor app %s",
			match.path, match.owner);

	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable adv_monitor_methods[] = {
	{ GDBUS_EXPERIMENTAL_ASYNC_METHOD("RegisterMonitor",
					GDBUS_ARGS({ "application", "o" }),
					NULL, register_monitor) },
	{ GDBUS_EXPERIMENTAL_ASYNC_METHOD("UnregisterMonitor",
					GDBUS_ARGS({ "application", "o" }),
					NULL, unregister_monitor) },
	{ }
};

/* Gets SupportedMonitorTypes property */
static gboolean get_supported_monitor_types(const GDBusPropertyTable *property,
						DBusMessageIter *iter,
						void *data)
{
	DBusMessageIter entry;
	const struct adv_monitor_type *t;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
						DBUS_TYPE_STRING_AS_STRING,
						&entry);

	for (t = supported_types; t->name; t++) {
		dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING,
						&t->name);
	}

	dbus_message_iter_close_container(iter, &entry);

	return TRUE;
}

const struct adv_monitor_feature {
	uint32_t mask;
	const char *name;
} supported_features[] = {
	{ MGMT_ADV_MONITOR_FEATURE_MASK_OR_PATTERNS, "controller-patterns" },
	{ }
};

/* Gets SupportedFeatures property */
static gboolean get_supported_features(const GDBusPropertyTable *property,
						DBusMessageIter *iter,
						void *data)
{
	DBusMessageIter entry;
	const struct adv_monitor_feature *f;
	struct btd_adv_monitor_manager *manager = data;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
						DBUS_TYPE_STRING_AS_STRING,
						&entry);

	for (f = supported_features; f->name; f++) {
		if (manager->supported_features & f->mask) {
			dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING,
							&f->name);
		}
	}

	dbus_message_iter_close_container(iter, &entry);

	return TRUE;
}

static const GDBusPropertyTable adv_monitor_properties[] = {
	{"SupportedMonitorTypes", "as", get_supported_monitor_types, NULL, NULL,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL},
	{"SupportedFeatures", "as", get_supported_features, NULL, NULL,
					G_DBUS_PROPERTY_FLAG_EXPERIMENTAL},
	{ }
};

/* Matches a monitor based on its handle */
static bool removed_monitor_match(const void *data, const void *user_data)
{
	const uint16_t *handle = user_data;
	const struct adv_monitor *monitor = data;

	if (!data || !handle)
		return false;

	return monitor->monitor_handle == *handle;
}

/* Updates monitor state to 'removed' */
static void monitor_state_removed(void *data, void *user_data)
{
	struct adv_monitor *monitor = data;

	if (!monitor && monitor->state != MONITOR_STATE_ACTIVE)
		return;

	monitor->state = MONITOR_STATE_REMOVED;

	DBG("Adv monitor with handle:0x%04x removed by kernel",
		monitor->monitor_handle);
}

/* Remove the matched monitor and reports the removal to the app */
static void app_remove_monitor(void *data, void *user_data)
{
	struct adv_monitor_app *app = data;
	struct adv_monitor *monitor;
	uint16_t *handle = user_data;

	if (handle && *handle == 0) {
		/* handle = 0 indicates kernel has removed all monitors */
		queue_foreach(app->monitors, monitor_state_removed, NULL);
		queue_destroy(app->monitors, monitor_destroy);

		return;
	}

	monitor = queue_find(app->monitors, removed_monitor_match, handle);
	if (monitor) {
		DBG("Adv Monitor at path %s removed", monitor->path);

		monitor_state_removed(monitor, NULL);
		monitor_destroy(monitor);
	}
}

/* Processes Adv Monitor removed event from kernel */
static void adv_monitor_removed_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adv_monitor_manager *manager = user_data;
	const struct mgmt_ev_adv_monitor_removed *ev = param;
	uint16_t handle = ev->monitor_handle;
	const uint16_t adapter_id = manager->adapter_id;

	if (length < sizeof(*ev)) {
		btd_error(adapter_id,
				"Wrong size of Adv Monitor Removed event");
		return;
	}

	/* Traverse the apps to find the monitor */
	queue_foreach(manager->apps, app_remove_monitor, &handle);

	DBG("Adv Monitor removed event with handle 0x%04x processed",
		ev->monitor_handle);
}

/* Allocates a manager object */
static struct btd_adv_monitor_manager *manager_new(
						struct btd_adapter *adapter,
						struct mgmt *mgmt)
{
	struct btd_adv_monitor_manager *manager;

	if (!adapter || !mgmt)
		return NULL;

	manager = new0(struct btd_adv_monitor_manager, 1);
	if (!manager)
		return NULL;

	manager->adapter = adapter;
	manager->mgmt = mgmt_ref(mgmt);
	manager->adapter_id = btd_adapter_get_index(adapter);
	manager->apps = queue_new();

	mgmt_register(manager->mgmt, MGMT_EV_ADV_MONITOR_REMOVED,
			manager->adapter_id, adv_monitor_removed_callback,
			manager, NULL);

	return manager;
}

/* Frees a manager object */
static void manager_free(struct btd_adv_monitor_manager *manager)
{
	mgmt_unref(manager->mgmt);

	queue_destroy(manager->apps, app_destroy);

	free(manager);
}

/* Destroys a manager object and unregisters its D-Bus interface */
static void manager_destroy(struct btd_adv_monitor_manager *manager)
{
	if (!manager)
		return;

	g_dbus_unregister_interface(btd_get_dbus_connection(),
					adapter_get_path(manager->adapter),
					ADV_MONITOR_MGR_INTERFACE);

	manager_free(manager);
}

/* Initiates manager's members based on the return of
 * MGMT_OP_READ_ADV_MONITOR_FEATURES
 */
static void read_adv_monitor_features_cb(uint8_t status, uint16_t length,
						const void *param,
						void *user_data)
{
	const struct mgmt_rp_read_adv_monitor_features *rp = param;
	struct btd_adv_monitor_manager *manager = user_data;

	if (status != MGMT_STATUS_SUCCESS || !param) {
		btd_error(manager->adapter_id,
				"Failed to Read Adv Monitor Features with "
				"status 0x%02x", status);
		return;
	}

	if (length < sizeof(*rp)) {
		btd_error(manager->adapter_id,
				"Wrong size of Read Adv Monitor Features "
				"response");
		return;
	}

	manager->supported_features = le32_to_cpu(rp->supported_features);
	manager->enabled_features = le32_to_cpu(rp->enabled_features);
	manager->max_num_monitors = le16_to_cpu(rp->max_num_handles);
	manager->max_num_patterns = rp->max_num_patterns;

	btd_info(manager->adapter_id, "Adv Monitor Manager created with "
			"supported features:0x%08x, enabled features:0x%08x, "
			"max number of supported monitors:%d, "
			"max number of supported patterns:%d",
			manager->supported_features, manager->enabled_features,
			manager->max_num_monitors, manager->max_num_patterns);
}

/* Creates a manager and registers its D-Bus interface */
struct btd_adv_monitor_manager *btd_adv_monitor_manager_create(
						struct btd_adapter *adapter,
						struct mgmt *mgmt)
{
	struct btd_adv_monitor_manager *manager;

	manager = manager_new(adapter, mgmt);
	if (!manager)
		return NULL;

	if (!g_dbus_register_interface(btd_get_dbus_connection(),
					adapter_get_path(manager->adapter),
					ADV_MONITOR_MGR_INTERFACE,
					adv_monitor_methods, NULL,
					adv_monitor_properties, manager,
					NULL)) {
		btd_error(manager->adapter_id,
				"Failed to register "
				ADV_MONITOR_MGR_INTERFACE);
		manager_free(manager);
		return NULL;
	}

	if (!mgmt_send(manager->mgmt, MGMT_OP_READ_ADV_MONITOR_FEATURES,
			manager->adapter_id, 0, NULL,
			read_adv_monitor_features_cb, manager, NULL)) {
		btd_error(manager->adapter_id,
				"Failed to send Read Adv Monitor Features");
		manager_destroy(manager);
		return NULL;
	}

	return manager;
}

/* Destroys a manager and unregisters its D-Bus interface */
void btd_adv_monitor_manager_destroy(struct btd_adv_monitor_manager *manager)
{
	if (!manager)
		return;

	btd_info(manager->adapter_id, "Destroy Adv Monitor Manager");

	manager_destroy(manager);
}

/* Processes the content matching based pattern(s) of a monitor */
static void adv_match_per_monitor(void *data, void *user_data)
{
	struct adv_monitor *monitor = data;
	struct adv_content_filter_info *info = user_data;

	if (!monitor) {
		error("Unexpected NULL adv_monitor object upon match");
		return;
	}

	if (monitor->state != MONITOR_STATE_ACTIVE)
		return;

	if (monitor->type == MONITOR_TYPE_OR_PATTERNS &&
		bt_ad_pattern_match(info->ad, monitor->patterns)) {
		goto matched;
	}

	return;

matched:
	if (!info->matched_monitors)
		info->matched_monitors = queue_new();

	queue_push_tail(info->matched_monitors, monitor);
}

/* Processes the content matching for the monitor(s) of an app */
static void adv_match_per_app(void *data, void *user_data)
{
	struct adv_monitor_app *app = data;

	if (!app) {
		error("Unexpected NULL adv_monitor_app object upon match");
		return;
	}

	queue_foreach(app->monitors, adv_match_per_monitor, user_data);
}

/* Processes the content matching for every app without RSSI filtering and
 * notifying monitors. The caller is responsible of releasing the memory of the
 * list but not the ad data.
 * Returns the list of monitors whose content match the ad data.
 */
struct queue *btd_adv_monitor_content_filter(
				struct btd_adv_monitor_manager *manager,
				struct bt_ad *ad)
{
	struct adv_content_filter_info info;

	if (!manager || !ad)
		return NULL;

	info.ad = ad;
	info.matched_monitors = NULL;

	queue_foreach(manager->apps, adv_match_per_app, &info);

	return info.matched_monitors;
}

/* Wraps adv_monitor_filter_rssi() to processes the content-matched monitor with
 * RSSI filtering and notifies it on device found/lost event
 */
static void monitor_filter_rssi(void *data, void *user_data)
{
	struct adv_monitor *monitor = data;
	struct adv_rssi_filter_info *info = user_data;

	if (!monitor || !info)
		return;

	adv_monitor_filter_rssi(monitor, info->device, info->rssi);
}

/* Processes every content-matched monitor with RSSI filtering and notifies on
 * device found/lost event. The caller is responsible of releasing the memory
 * of matched_monitors list but not its data.
 */
void btd_adv_monitor_notify_monitors(struct btd_adv_monitor_manager *manager,
					struct btd_device *device, int8_t rssi,
					struct queue *matched_monitors)
{
	struct adv_rssi_filter_info info;

	if (!manager || !device || !matched_monitors ||
		queue_isempty(matched_monitors)) {
		return;
	}

	info.device = device;
	info.rssi = rssi;

	queue_foreach(matched_monitors, monitor_filter_rssi, &info);
}

/* Matches a device based on btd_device object */
static bool monitor_device_match(const void *a, const void *b)
{
	const struct adv_monitor_device *dev = a;
	const struct btd_device *device = b;

	if (!dev) {
		error("Unexpected NULL adv_monitor_device object upon match");
		return false;
	}

	if (dev->device != device)
		return false;

	return true;
}

/* Frees a monitor device object */
static void monitor_device_free(void *data)
{
	struct adv_monitor_device *dev = data;

	if (!dev) {
		error("Unexpected NULL adv_monitor_device object upon free");
		return;
	}

	if (dev->lost_timer) {
		g_source_remove(dev->lost_timer);
		dev->lost_timer = 0;
	}

	dev->monitor = NULL;
	dev->device = NULL;

	free(dev);
}

/* Removes a device from monitor->devices list */
static void remove_device_from_monitor(void *data, void *user_data)
{
	struct adv_monitor *monitor = data;
	struct btd_device *device = user_data;
	struct adv_monitor_device *dev = NULL;

	if (!monitor) {
		error("Unexpected NULL adv_monitor object upon device remove");
		return;
	}

	dev = queue_remove_if(monitor->devices, monitor_device_match, device);
	if (dev) {
		DBG("Device removed from the Adv Monitor at path %s",
		    monitor->path);
		monitor_device_free(dev);
	}
}

/* Removes a device from every monitor in an app */
static void remove_device_from_app(void *data, void *user_data)
{
	struct adv_monitor_app *app = data;
	struct btd_device *device = user_data;

	if (!app) {
		error("Unexpected NULL adv_monitor_app object upon device "
			"remove");
		return;
	}

	queue_foreach(app->monitors, remove_device_from_monitor, device);
}

/* Removes a device from every monitor in all apps */
void btd_adv_monitor_device_remove(struct btd_adv_monitor_manager *manager,
				   struct btd_device *device)
{
	if (!manager || !device)
		return;

	queue_foreach(manager->apps, remove_device_from_app, device);
}

/* Creates a device object to track the per-device information */
static struct adv_monitor_device *monitor_device_create(
			struct adv_monitor *monitor,
			struct btd_device *device)
{
	struct adv_monitor_device *dev = NULL;

	dev = new0(struct adv_monitor_device, 1);
	if (!dev)
		return NULL;

	dev->monitor = monitor;
	dev->device = device;

	queue_push_tail(monitor->devices, dev);

	return dev;
}

/* Includes found/lost device's object path into the dbus message */
static void report_device_state_setup(DBusMessageIter *iter, void *user_data)
{
	const char *path = device_get_path(user_data);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
}

/* Handles a situation where the device goes offline/out-of-range */
static gboolean handle_device_lost_timeout(gpointer user_data)
{
	struct adv_monitor_device *dev = user_data;
	struct adv_monitor *monitor = dev->monitor;
	time_t curr_time = time(NULL);

	DBG("Device Lost timeout triggered for device %p "
	    "for the Adv Monitor at path %s", dev->device, monitor->path);

	dev->lost_timer = 0;

	if (dev->found && dev->last_seen) {
		/* We were tracking for the Low RSSI filter. Check if there is
		 * any Adv received after the timeout function is invoked.
		 * If not, report the Device Lost event.
		 */
		if (difftime(curr_time, dev->last_seen) >=
		    monitor->low_rssi_timeout) {
			dev->found = false;

			DBG("Calling DeviceLost() on Adv Monitor of owner %s "
			    "at path %s", monitor->app->owner, monitor->path);

			g_dbus_proxy_method_call(monitor->proxy, "DeviceLost",
						 report_device_state_setup,
						 NULL, dev->device, NULL);
		}
	}

	return FALSE;
}

/* Filters an Adv based on its RSSI value */
static void adv_monitor_filter_rssi(struct adv_monitor *monitor,
				    struct btd_device *device, int8_t rssi)
{
	struct adv_monitor_device *dev = NULL;
	time_t curr_time = time(NULL);
	uint16_t adapter_id = monitor->app->manager->adapter_id;

	/* If the RSSI thresholds and timeouts are not specified, report the
	 * DeviceFound() event without tracking for the RSSI as the Adv has
	 * already matched the pattern filter.
	 */
	if (monitor_rssi_is_unset(monitor)) {
		DBG("Calling DeviceFound() on Adv Monitor of owner %s "
		    "at path %s", monitor->app->owner, monitor->path);

		g_dbus_proxy_method_call(monitor->proxy, "DeviceFound",
					 report_device_state_setup, NULL,
					 device, NULL);

		return;
	}

	dev = queue_find(monitor->devices, monitor_device_match, device);
	if (!dev) {
		dev = monitor_device_create(monitor, device);
		if (!dev) {
			btd_error(adapter_id,
				"Failed to create Adv Monitor device object.");
			return;
		}
	}

	if (dev->lost_timer) {
		g_source_remove(dev->lost_timer);
		dev->lost_timer = 0;
	}

	/* Reset the timings of found/lost if a device has been offline for
	 * longer than the high/low timeouts.
	 */
	if (dev->last_seen) {
		if (difftime(curr_time, dev->last_seen) >
		    monitor->high_rssi_timeout) {
			dev->high_rssi_first_seen = 0;
		}

		if (difftime(curr_time, dev->last_seen) >
		    monitor->low_rssi_timeout) {
			dev->low_rssi_first_seen = 0;
		}
	}
	dev->last_seen = curr_time;

	/* Check for the found devices (if the device is not already found) */
	if (!dev->found && rssi > monitor->high_rssi) {
		if (dev->high_rssi_first_seen) {
			if (difftime(curr_time, dev->high_rssi_first_seen) >=
			    monitor->high_rssi_timeout) {
				dev->found = true;

				DBG("Calling DeviceFound() on Adv Monitor "
				    "of owner %s at path %s",
				    monitor->app->owner, monitor->path);

				g_dbus_proxy_method_call(
					monitor->proxy, "DeviceFound",
					report_device_state_setup, NULL,
					dev->device, NULL);
			}
		} else {
			dev->high_rssi_first_seen = curr_time;
		}
	} else {
		dev->high_rssi_first_seen = 0;
	}

	/* Check for the lost devices (only if the device is already found, as
	 * it doesn't make any sense to report the Device Lost event if the
	 * device is not found yet)
	 */
	if (dev->found && rssi < monitor->low_rssi) {
		if (dev->low_rssi_first_seen) {
			if (difftime(curr_time, dev->low_rssi_first_seen) >=
			    monitor->low_rssi_timeout) {
				dev->found = false;

				DBG("Calling DeviceLost() on Adv Monitor "
				    "of owner %s at path %s",
				    monitor->app->owner, monitor->path);

				g_dbus_proxy_method_call(
					monitor->proxy, "DeviceLost",
					report_device_state_setup, NULL,
					dev->device, NULL);
			}
		} else {
			dev->low_rssi_first_seen = curr_time;
		}
	} else {
		dev->low_rssi_first_seen = 0;
	}

	/* Setup a timer to track if the device goes offline/out-of-range, only
	 * if we are tracking for the Low RSSI Threshold. If we are tracking
	 * the High RSSI Threshold, nothing needs to be done.
	 */
	if (dev->found) {
		dev->lost_timer =
			g_timeout_add_seconds(monitor->low_rssi_timeout,
					      handle_device_lost_timeout, dev);
	}
}
