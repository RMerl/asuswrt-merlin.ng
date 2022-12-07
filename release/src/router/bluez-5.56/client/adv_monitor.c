// SPDX-License-Identifier: GPL-2.0-or-later
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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "gdbus/gdbus.h"
#include "src/shared/ad.h"
#include "src/shared/util.h"
#include "src/shared/shell.h"
#include "adv_monitor.h"

#define ADV_MONITOR_APP_PATH	"/org/bluez/adv_monitor_app"
#define ADV_MONITOR_INTERFACE	"org.bluez.AdvertisementMonitor1"

#define RSSI_UNSET_THRESHOLD		127
#define RSSI_UNSET_TIMEOUT		0
#define RSSI_UNSET_SAMPLING_PERIOD	256

struct rssi_setting {
	int16_t high_threshold;
	uint16_t high_timeout;
	int16_t low_threshold;
	uint16_t low_timeout;
	uint16_t sampling_period;
};

struct pattern {
	uint8_t start_pos;
	uint8_t ad_data_type;
	uint8_t content_len;
	uint8_t content[BT_AD_MAX_DATA_LEN];
};

struct adv_monitor {
	uint8_t idx;
	char *path;
	char *type;
	struct rssi_setting *rssi;
	GSList *patterns;
};

static struct adv_monitor_manager {
	GSList *supported_types;
	GSList *supported_features;
	GDBusProxy *proxy;
	gboolean app_registered;
} manager = { NULL, NULL, NULL, FALSE };

static uint8_t adv_mon_idx;
static GSList *adv_mons;
static struct rssi_setting *current_rssi;

static void remove_adv_monitor(void *data, void *user_data);

static DBusMessage *release_adv_monitor(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct adv_monitor *adv_monitor = user_data;

	bt_shell_printf("Advertisement monitor %d released\n",
							adv_monitor->idx);
	remove_adv_monitor(adv_monitor, conn);

	return dbus_message_new_method_return(msg);
}

static DBusMessage *activate_adv_monitor(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct adv_monitor *adv_monitor = user_data;

	bt_shell_printf("Advertisement monitor %d activated\n",
							adv_monitor->idx);
	return dbus_message_new_method_return(msg);
}

static DBusMessage *device_found_adv_monitor(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct adv_monitor *adv_monitor = user_data;
	const char *device;

	dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &device,
							DBUS_TYPE_INVALID);
	bt_shell_printf("Advertisement monitor %d found device %s\n",
						adv_monitor->idx, device);
	return dbus_message_new_method_return(msg);
}

static DBusMessage *device_lost_adv_monitor(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct adv_monitor *adv_monitor = user_data;
	const char *device;

	dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &device,
							DBUS_TYPE_INVALID);
	bt_shell_printf("Advertisement monitor %d lost device %s\n",
						adv_monitor->idx, device);
	return dbus_message_new_method_return(msg);
}

static const GDBusMethodTable adv_monitor_methods[] = {
	{ GDBUS_ASYNC_METHOD("Release", NULL, NULL, release_adv_monitor) },
	{ GDBUS_ASYNC_METHOD("Activate", NULL, NULL, activate_adv_monitor) },
	{ GDBUS_ASYNC_METHOD("DeviceFound", GDBUS_ARGS({ "device", "o" }),
			NULL, device_found_adv_monitor) },
	{ GDBUS_ASYNC_METHOD("DeviceLost", GDBUS_ARGS({ "device", "o" }),
			NULL, device_lost_adv_monitor) },
	{ }
};


static gboolean get_type(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	struct adv_monitor *adv_monitor = user_data;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING,
							&adv_monitor->type);
	return TRUE;
}

static gboolean get_low_threshold(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	struct adv_monitor *adv_monitor = user_data;
	struct rssi_setting *rssi = adv_monitor->rssi;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_INT16,
							&rssi->low_threshold);
	return TRUE;
}

static gboolean get_high_threshold(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	struct adv_monitor *adv_monitor = user_data;
	struct rssi_setting *rssi = adv_monitor->rssi;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_INT16,
							&rssi->high_threshold);
	return TRUE;
}

static gboolean get_low_timeout(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	struct adv_monitor *adv_monitor = user_data;
	struct rssi_setting *rssi = adv_monitor->rssi;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16,
							&rssi->low_timeout);
	return TRUE;
}

static gboolean get_high_timeout(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	struct adv_monitor *adv_monitor = user_data;
	struct rssi_setting *rssi = adv_monitor->rssi;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16,
							&rssi->high_timeout);
	return TRUE;
}

static gboolean get_sampling_period(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	struct adv_monitor *adv_monitor = user_data;
	struct rssi_setting *rssi = adv_monitor->rssi;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT16,
							&rssi->sampling_period);
	return TRUE;
}

static gboolean low_threshold_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct adv_monitor *adv_monitor = data;

	return adv_monitor->rssi != NULL &&
		adv_monitor->rssi->low_threshold != RSSI_UNSET_THRESHOLD;
}

static gboolean high_threshold_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct adv_monitor *adv_monitor = data;

	return adv_monitor->rssi != NULL &&
		adv_monitor->rssi->high_threshold != RSSI_UNSET_THRESHOLD;
}

static gboolean low_timeout_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct adv_monitor *adv_monitor = data;

	return adv_monitor->rssi != NULL &&
		adv_monitor->rssi->low_timeout != RSSI_UNSET_TIMEOUT;
}

static gboolean high_timeout_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct adv_monitor *adv_monitor = data;

	return adv_monitor->rssi != NULL &&
		adv_monitor->rssi->high_timeout != RSSI_UNSET_TIMEOUT;
}

static gboolean sampling_period_exists(const GDBusPropertyTable *property,
								void *data)
{
	struct adv_monitor *adv_monitor = data;

	return adv_monitor->rssi != NULL &&
		adv_monitor->rssi->sampling_period !=
						RSSI_UNSET_SAMPLING_PERIOD;
}

static void append_pattern_content_to_dbus(DBusMessageIter *iter,
							struct pattern *pattern)
{
	DBusMessageIter data_iter;
	int idx;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_BYTE_AS_STRING, &data_iter);
	for (idx = 0; idx < pattern->content_len; idx++)
		dbus_message_iter_append_basic(&data_iter, DBUS_TYPE_BYTE,
							&pattern->content[idx]);
	dbus_message_iter_close_container(iter, &data_iter);
}

static void append_pattern_to_dbus(void *data, void *user_data)
{
	struct pattern *pattern = data;
	DBusMessageIter *array_iter = user_data;
	DBusMessageIter data_iter;

	dbus_message_iter_open_container(array_iter, DBUS_TYPE_STRUCT,
							NULL, &data_iter);
	dbus_message_iter_append_basic(&data_iter, DBUS_TYPE_BYTE,
							&pattern->start_pos);
	dbus_message_iter_append_basic(&data_iter, DBUS_TYPE_BYTE,
							&pattern->ad_data_type);
	append_pattern_content_to_dbus(&data_iter, pattern);
	dbus_message_iter_close_container(array_iter, &data_iter);
}

static gboolean get_patterns(const GDBusPropertyTable *property,
				DBusMessageIter *iter, void *user_data)
{
	struct adv_monitor *adv_monitor = user_data;
	DBusMessageIter array_iter;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "(yyay)",
								&array_iter);
	g_slist_foreach(adv_monitor->patterns, append_pattern_to_dbus,
								&array_iter);
	dbus_message_iter_close_container(iter, &array_iter);
	return TRUE;
}

static gboolean pattern_exists(const GDBusPropertyTable *property, void *data)
{
	struct adv_monitor *adv_monitor = data;

	return adv_monitor->patterns != NULL;
}

static const GDBusPropertyTable adv_monitor_props[] = {
	{ "Type", "s", get_type },
	{ "RSSILowThreshold", "n", get_low_threshold, NULL,
						low_threshold_exists },
	{ "RSSIHighThreshold", "n", get_high_threshold, NULL,
						high_threshold_exists },
	{ "RSSILowTimeout", "q", get_low_timeout, NULL, low_timeout_exists },
	{ "RSSIHighTimeout", "q", get_high_timeout, NULL, high_timeout_exists },
	{ "RSSISamplingPeriod", "q", get_sampling_period, NULL,
						sampling_period_exists },
	{ "Patterns", "a(yyay)", get_patterns, NULL, pattern_exists },
	{ }
};

static void set_supported_list(GSList **list, DBusMessageIter *iter)
{
	char *str;
	DBusMessageIter subiter;

	dbus_message_iter_recurse(iter, &subiter);
	while (dbus_message_iter_get_arg_type(&subiter) ==
						DBUS_TYPE_STRING) {
		dbus_message_iter_get_basic(&subiter, &str);
		*list = g_slist_append(*list, str);
		dbus_message_iter_next(&subiter);
	}
}

void adv_monitor_add_manager(DBusConnection *conn, GDBusProxy *proxy)
{
	DBusMessageIter iter;

	if (manager.proxy != NULL || manager.supported_types != NULL ||
					manager.supported_features != NULL) {
		bt_shell_printf("advertisement monitor manager already "
				"added\n");
		return;
	}

	manager.proxy = proxy;

	if (g_dbus_proxy_get_property(proxy, "SupportedMonitorTypes", &iter))
		set_supported_list(&(manager.supported_types), &iter);

	if (g_dbus_proxy_get_property(proxy, "SupportedFeatures", &iter))
		set_supported_list(&(manager.supported_features), &iter);

}

void adv_monitor_remove_manager(DBusConnection *conn)
{
	if (manager.supported_types != NULL) {
		g_slist_free(manager.supported_types);
		manager.supported_types = NULL;
	}

	if (manager.supported_features != NULL) {
		g_slist_free(manager.supported_features);
		manager.supported_features = NULL;
	}

	manager.proxy = NULL;
	manager.app_registered = FALSE;

	g_free(current_rssi);
	current_rssi = NULL;
}

static void register_setup(DBusMessageIter *iter, void *user_data)
{
	const char *path = "/";

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
}

static void register_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (!dbus_set_error_from_message(&error, message)) {
		bt_shell_printf("AdvertisementMonitor path registered\n");
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	bt_shell_printf("Failed to register path: %s\n", error.name);
	dbus_error_free(&error);
	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

static void unregister_setup(DBusMessageIter *iter, void *user_data)
{
	const char *path = "/";

	dbus_message_iter_append_basic(iter, DBUS_TYPE_OBJECT_PATH, &path);
}

static void unregister_reply(DBusMessage *message, void *user_data)
{
	DBusError error;

	dbus_error_init(&error);

	if (!dbus_set_error_from_message(&error, message)) {
		bt_shell_printf("AdvertisementMonitor path unregistered\n");
		return bt_shell_noninteractive_quit(EXIT_SUCCESS);
	}

	bt_shell_printf("Failed to unregister Advertisement Monitor:"
			" %s\n", error.name);
	dbus_error_free(&error);
	return bt_shell_noninteractive_quit(EXIT_FAILURE);
}

void adv_monitor_register_app(DBusConnection *conn)
{
	if (manager.app_registered) {
		bt_shell_printf("Advertisement Monitor already registered\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	} else if (manager.supported_types == NULL ||
		!g_dbus_proxy_method_call(manager.proxy, "RegisterMonitor",
					register_setup, register_reply,
					NULL, NULL)) {
		bt_shell_printf("Failed to register Advertisement Monitor\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
	manager.app_registered = TRUE;
}

void adv_monitor_unregister_app(DBusConnection *conn)
{
	if (!manager.app_registered) {
		bt_shell_printf("Advertisement Monitor not registered\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	} else if (!g_dbus_proxy_method_call(manager.proxy, "UnregisterMonitor",
					unregister_setup, unregister_reply,
					NULL, NULL)) {
		bt_shell_printf("Failed to unregister Advertisement Monitor\n");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}
	manager.app_registered = FALSE;
}

static void free_pattern(void *user_data)
{
	struct pattern *p = user_data;

	g_free(p);
}

static void free_adv_monitor(void *user_data)
{
	struct adv_monitor *adv_monitor = user_data;

	g_free(adv_monitor->path);
	g_free(adv_monitor->type);
	g_free(adv_monitor->rssi);
	g_slist_free_full(adv_monitor->patterns, free_pattern);
	g_free(adv_monitor);
}

static uint8_t str2bytearray(char *str, uint8_t *arr)
{
	int idx, len = strlen(str), arr_len = 0;

	if (len%2 != 0)
		return 0;

	for (idx = 0; idx < len; idx += 2) {
		if (sscanf(str+idx, "%2hhx", &arr[arr_len++]) < 1)
			return 0;
	}
	return arr_len;
}

static struct pattern *parse_pattern(char *parameter_list[])
{
	struct pattern *pat;

	pat = g_malloc0(sizeof(struct pattern));

	if (!pat) {
		bt_shell_printf("Failed to allocate pattern\n");
		bt_shell_noninteractive_quit(EXIT_FAILURE);
		return NULL;
	}

	pat->start_pos = atoi(parameter_list[0]);
	pat->ad_data_type = atoi(parameter_list[1]);
	pat->content_len = str2bytearray(parameter_list[2], pat->content);
	if (pat->content_len == 0) {
		free_pattern(pat);
		return NULL;
	}

	return pat;
}

static GSList *parse_patterns(char *pattern_list[], int num)
{
	GSList *patterns = NULL;
	int cnt;

	if (num == 0) {
		bt_shell_printf("No pattern provided\n");
		return NULL;
	}

	if (num%3) {
		bt_shell_printf("Expected %d more arguments\n", 3 - num%3);
		return NULL;
	}

	for (cnt = 0; cnt < num; cnt += 3) {
		struct pattern *pattern;

		pattern = parse_pattern(pattern_list+cnt);
		if (pattern == NULL) {
			g_slist_free_full(patterns, free_pattern);
			return NULL;
		}
		patterns = g_slist_append(patterns, pattern);
	}

	return patterns;
}

static void remove_adv_monitor(void *data, void *user_data)
{
	struct adv_monitor *adv_monitor = data;
	DBusConnection *conn = user_data;

	adv_mons = g_slist_remove(adv_mons, adv_monitor);
	g_dbus_unregister_interface(conn, adv_monitor->path,
							ADV_MONITOR_INTERFACE);
}

static gint cmp_adv_monitor_with_idx(gconstpointer a, gconstpointer b)
{
	const struct adv_monitor *adv_monitor = a;
	uint8_t idx = *(uint8_t *)b;

	return adv_monitor->idx != idx;
}

static struct adv_monitor *find_adv_monitor_with_idx(uint8_t monitor_idx)
{
	GSList *list;

	list = g_slist_find_custom(adv_mons, &monitor_idx,
						cmp_adv_monitor_with_idx);

	if (list)
		return (struct adv_monitor *)list->data;
	return NULL;
}

static void print_bytearray(char *prefix, uint8_t *arr, uint8_t len)
{
	int idx;

	bt_shell_printf("%s", prefix);
	for (idx = 0; idx < len; idx++)
		bt_shell_printf("%02hhx", arr[idx]);
	bt_shell_printf("\n");
}

static void print_adv_monitor(struct adv_monitor *adv_monitor)
{
	GSList *l;

	bt_shell_printf("Advertisement Monitor %d\n", adv_monitor->idx);
	bt_shell_printf("\tpath: %s\n", adv_monitor->path);
	bt_shell_printf("\ttype: %s\n", adv_monitor->type);
	if (adv_monitor->rssi) {
		bt_shell_printf("\trssi:\n");
		bt_shell_printf("\t\thigh threshold: %hd\n",
					adv_monitor->rssi->high_threshold);
		bt_shell_printf("\t\thigh threshold timeout: %hu\n",
					adv_monitor->rssi->high_timeout);
		bt_shell_printf("\t\tlow threshold: %hd\n",
					adv_monitor->rssi->low_threshold);
		bt_shell_printf("\t\tlow threshold timeout: %hu\n",
					adv_monitor->rssi->low_timeout);
		bt_shell_printf("\t\tsampling period: %hu\n",
					adv_monitor->rssi->sampling_period);
	}

	if (adv_monitor->patterns) {
		int idx = 1;

		for (l = adv_monitor->patterns; l; l = g_slist_next(l), idx++) {
			struct pattern *pattern = l->data;

			bt_shell_printf("\tpattern %d:\n", idx);
			bt_shell_printf("\t\tstart position: %hhu\n",
							pattern->start_pos);
			bt_shell_printf("\t\tAD data type: %hhu\n",
							pattern->ad_data_type);
			print_bytearray("\t\tcontent: ", pattern->content,
							pattern->content_len);
		}
	}
}

static struct rssi_setting *get_current_rssi(void)
{
	if (current_rssi)
		return current_rssi;

	current_rssi = g_malloc0(sizeof(struct rssi_setting));

	if (!current_rssi)
		bt_shell_printf("Failed to allocate rssi setting");

	current_rssi->low_threshold = RSSI_UNSET_THRESHOLD;
	current_rssi->high_threshold = RSSI_UNSET_THRESHOLD;
	current_rssi->low_timeout = RSSI_UNSET_TIMEOUT;
	current_rssi->high_timeout = RSSI_UNSET_TIMEOUT;
	current_rssi->sampling_period = RSSI_UNSET_SAMPLING_PERIOD;

	return current_rssi;
}

void adv_monitor_set_rssi_threshold(int16_t low_threshold,
							int16_t high_threshold)
{
	struct rssi_setting *rssi = get_current_rssi();

	if (!rssi)
		return;

	rssi->low_threshold = low_threshold;
	rssi->high_threshold = high_threshold;
}

void adv_monitor_set_rssi_timeout(uint16_t low_timeout, uint16_t high_timeout)
{
	struct rssi_setting *rssi = get_current_rssi();

	if (!rssi)
		return;

	rssi->low_timeout = low_timeout;
	rssi->high_timeout = high_timeout;
}

void adv_monitor_set_rssi_sampling_period(uint16_t sampling)
{
	struct rssi_setting *rssi = get_current_rssi();

	if (!rssi)
		return;

	rssi->sampling_period = sampling;
}

void adv_monitor_add_monitor(DBusConnection *conn, char *type,
							int argc, char *argv[])
{
	struct adv_monitor *adv_monitor;
	GSList *patterns = NULL;

	if (g_slist_length(adv_mons) >= UINT8_MAX) {
		bt_shell_printf("Number of advertisement monitor exceeds "
				"the limit");
		return;
	}

	while (find_adv_monitor_with_idx(adv_mon_idx))
		adv_mon_idx += 1;

	patterns = parse_patterns(argv+1, argc-1);
	if (patterns == NULL) {
		bt_shell_printf("pattern-list malformed\n");
		return;
	}

	adv_monitor = g_malloc0(sizeof(struct adv_monitor));

	if (!adv_monitor) {
		bt_shell_printf("Failed to allocate adv_monitor");
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	adv_monitor->idx = adv_mon_idx;
	adv_monitor->type = g_strdup(type);
	adv_monitor->rssi = current_rssi;
	adv_monitor->patterns = patterns;
	adv_monitor->path = g_strdup_printf("%s/%hhu", ADV_MONITOR_APP_PATH,
								adv_mon_idx);
	current_rssi = NULL;

	if (g_dbus_register_interface(conn, adv_monitor->path,
					ADV_MONITOR_INTERFACE,
					adv_monitor_methods, NULL,
					adv_monitor_props, adv_monitor,
					free_adv_monitor) == FALSE) {
		bt_shell_printf("Failed to register advertisement monitor\n");
		free_adv_monitor(adv_monitor);
		return bt_shell_noninteractive_quit(EXIT_FAILURE);
	}

	adv_mons = g_slist_append(adv_mons, adv_monitor);
	bt_shell_printf("Advertisement Monitor %d added\n", adv_monitor->idx);
}

void adv_monitor_print_monitor(DBusConnection *conn, int monitor_idx)
{
	struct adv_monitor *adv_monitor;
	GSList *l;

	if (monitor_idx < 0) {
		for (l = adv_mons; l; l = g_slist_next(l)) {
			adv_monitor = l->data;
			print_adv_monitor(adv_monitor);
		}
		return;
	}

	adv_monitor = find_adv_monitor_with_idx(monitor_idx);

	if (adv_monitor == NULL) {
		bt_shell_printf("Can't find monitor with index %d\n",
								monitor_idx);
		return;
	}

	print_adv_monitor(adv_monitor);
}

void adv_monitor_remove_monitor(DBusConnection *conn, int monitor_idx)
{
	struct adv_monitor *adv_monitor;

	if (monitor_idx < 0) {
		g_slist_foreach(adv_mons, remove_adv_monitor, conn);
		return;
	}

	adv_monitor = find_adv_monitor_with_idx(monitor_idx);
	if (adv_monitor == NULL) {
		bt_shell_printf("Can't find monitor with index %d\n",
								monitor_idx);
		return;
	}

	remove_adv_monitor(adv_monitor, conn);
	bt_shell_printf("Monitor %d deleted\n", monitor_idx);
}

static void print_supported_list(GSList *list)
{
	GSList *iter;

	for (iter = list; iter; iter = g_slist_next(iter)) {
		char *data = iter->data;

		printf(" %s", data);
	}
}

void adv_monitor_get_supported_info(void)
{
	bt_shell_printf("Supported Features:");
	print_supported_list(manager.supported_features);
	bt_shell_printf("\n");

	bt_shell_printf("Supported Moniter Types:");
	print_supported_list(manager.supported_types);
	bt_shell_printf("\n");
}
