// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2000-2001  Qualcomm Incorporated
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/signalfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <glib.h>

#include <dbus/dbus.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"

#include "gdbus/gdbus.h"
#include "btio/btio.h"

#include "log.h"
#include "backtrace.h"

#include "shared/att-types.h"
#include "shared/mainloop.h"
#include "lib/uuid.h"
#include "shared/util.h"
#include "btd.h"
#include "sdpd.h"
#include "adapter.h"
#include "device.h"
#include "dbus-common.h"
#include "agent.h"
#include "profile.h"
#define TYPEDEF_BOOL
#include <bcmnvram.h>
#include <bcmparams.h>
#include <shared.h>
#include <shutils.h>

#define BLUEZ_NAME "org.bluez"

#define DEFAULT_PAIRABLE_TIMEOUT       0 /* disabled */
#define DEFAULT_DISCOVERABLE_TIMEOUT 180 /* 3 minutes */
#define DEFAULT_TEMPORARY_TIMEOUT     30 /* 30 seconds */

#define SHUTDOWN_GRACE_SECONDS 10

struct btd_opts btd_opts;
static GKeyFile *main_conf;
static char *main_conf_file_path;

static const char *supported_options[] = {
	"Name",
	"Class",
	"DiscoverableTimeout",
	"AlwaysPairable",
	"PairableTimeout",
	"DeviceID",
	"ReverseServiceDiscovery",
	"NameResolving",
	"DebugKeys",
	"ControllerMode",
	"MultiProfile",
	"FastConnectable",
	"Privacy",
	"JustWorksRepairing",
	"TemporaryTimeout",
	NULL
};

static const char *br_options[] = {
	"PageScanType",
	"PageScanInterval",
	"PageScanWindow",
	"InquiryScanType",
	"InquiryScanInterval",
	"InquiryScanWindow",
	"LinkSupervisionTimeout",
	"PageTimeout",
	"MinSniffInterval",
	"MaxSniffInterval",
	NULL
};

static const char *le_options[] = {
	"MinAdvertisementInterval",
	"MaxAdvertisementInterval",
	"MultiAdvertisementRotationInterval",
	"ScanIntervalAutoConnect",
	"ScanWindowAutoConnect",
	"ScanIntervalSuspend",
	"ScanWindowSuspend",
	"ScanIntervalDiscovery",
	"ScanWindowDiscovery",
	"ScanIntervalAdvMonitoring",
	"ScanWindowAdvMonitoring",
	"ScanIntervalConnect",
	"ScanWindowConnect",
	"MinConnectionInterval",
	"MaxConnectionInterval",
	"ConnectionLatency",
	"ConnectionSupervisionTimeout",
	"Autoconnecttimeout",
	"AdvMonAllowlistScanDuration",
	"AdvMonNoFilterScanDuration",
	"EnableAdvMonInterleaveScan",
	NULL
};

static const char *policy_options[] = {
	"ReconnectUUIDs",
	"ReconnectAttempts",
	"ReconnectIntervals",
	"AutoEnable",
	"ResumeDelay",
	NULL
};

static const char *gatt_options[] = {
	"Cache",
	"KeySize",
	"ExchangeMTU",
	"Channels",
	NULL
};

static const char *avdtp_options[] = {
	"SessionMode",
	"StreamMode",
	NULL
};

static const struct group_table {
	const char *name;
	const char **options;
} valid_groups[] = {
	{ "General",	supported_options },
	{ "BR",		br_options },
	{ "LE",		le_options },
	{ "Policy",	policy_options },
	{ "GATT",	gatt_options },
	{ "AVDTP",	avdtp_options },
	{ }
};


GKeyFile *btd_get_main_conf(void)
{
	return main_conf;
}

static GKeyFile *load_config(const char *file)
{
	GError *err = NULL;
	GKeyFile *keyfile;

	keyfile = g_key_file_new();

	g_key_file_set_list_separator(keyfile, ',');

	if (!g_key_file_load_from_file(keyfile, file, 0, &err)) {
		if (!g_error_matches(err, G_FILE_ERROR, G_FILE_ERROR_NOENT))
			error("Parsing %s failed: %s", file, err->message);
		g_error_free(err);
		g_key_file_free(keyfile);
		return NULL;
	}

	return keyfile;
}

static void parse_did(const char *did)
{
	int result;
	uint16_t vendor, product, version , source;

	/* version and source are optional */
	version = 0x0000;
	source = 0x0002;

	result = sscanf(did, "bluetooth:%4hx:%4hx:%4hx",
					&vendor, &product, &version);
	if (result != EOF && result >= 2) {
		source = 0x0001;
		goto done;
	}

	result = sscanf(did, "usb:%4hx:%4hx:%4hx",
					&vendor, &product, &version);
	if (result != EOF && result >= 2)
		goto done;

	result = sscanf(did, "%4hx:%4hx:%4hx", &vendor, &product, &version);
	if (result == EOF || result < 2)
		return;

done:
	btd_opts.did_source = source;
	btd_opts.did_vendor = vendor;
	btd_opts.did_product = product;
	btd_opts.did_version = version;
}

static bt_gatt_cache_t parse_gatt_cache(const char *cache)
{
	if (!strcmp(cache, "always")) {
		return BT_GATT_CACHE_ALWAYS;
	} else if (!strcmp(cache, "yes")) {
		return BT_GATT_CACHE_YES;
	} else if (!strcmp(cache, "no")) {
		return BT_GATT_CACHE_NO;
	} else {
		DBG("Invalid value for KeepCache=%s", cache);
		return BT_GATT_CACHE_ALWAYS;
	}
}

static enum jw_repairing_t parse_jw_repairing(const char *jw_repairing)
{
	if (!strcmp(jw_repairing, "never")) {
		return JW_REPAIRING_NEVER;
	} else if (!strcmp(jw_repairing, "confirm")) {
		return JW_REPAIRING_CONFIRM;
	} else if (!strcmp(jw_repairing, "always")) {
		return JW_REPAIRING_ALWAYS;
	} else {
		return JW_REPAIRING_NEVER;
	}
}


static void check_options(GKeyFile *config, const char *group,
						const char **options)
{
	char **keys;
	int i;

	keys = g_key_file_get_keys(config, group, NULL, NULL);

	for (i = 0; keys != NULL && keys[i] != NULL; i++) {
		bool found;
		unsigned int j;

		found = false;
		for (j = 0; options != NULL && options[j] != NULL; j++) {
			if (g_str_equal(keys[i], options[j])) {
				found = true;
				break;
			}
		}

		if (!found)
			warn("Unknown key %s for group %s in %s",
					keys[i], group, main_conf_file_path);
	}

	g_strfreev(keys);
}

static void check_config(GKeyFile *config)
{
	char **keys;
	int i;
	const struct group_table *group;

	if (!config)
		return;

	keys = g_key_file_get_groups(config, NULL);

	for (i = 0; keys != NULL && keys[i] != NULL; i++) {
		bool match = false;

		for (group = valid_groups; group && group->name ; group++) {
			if (g_str_equal(keys[i], group->name)) {
				match = true;
				break;
			}
		}

		if (!match)
			warn("Unknown group %s in %s", keys[i],
						main_conf_file_path);
	}

	g_strfreev(keys);

	for (group = valid_groups; group && group->name; group++)
		check_options(config, group->name, group->options);
}

static int get_mode(const char *str)
{
	if (strcmp(str, "dual") == 0)
		return BT_MODE_DUAL;
	else if (strcmp(str, "bredr") == 0)
		return BT_MODE_BREDR;
	else if (strcmp(str, "le") == 0)
		return BT_MODE_LE;

	error("Unknown controller mode \"%s\"", str);

	return BT_MODE_DUAL;
}

struct config_param {
	const char * const val_name;
	void * const val;
	const size_t size;
	const uint16_t min;
	const uint16_t max;
};

static void parse_mode_config(GKeyFile *config, const char *group,
				const struct config_param *params,
				size_t params_len)
{
	uint16_t i;

	if (!config)
		return;

	for (i = 0; i < params_len; ++i) {
		GError *err = NULL;
		int val = g_key_file_get_integer(config, group,
						params[i].val_name, &err);
		if (err) {
			DBG("%s", err->message);
			g_clear_error(&err);
		} else {
			info("%s=%d", params[i].val_name, val);

			val = MAX(val, params[i].min);
			val = MIN(val, params[i].max);

			val = htobl(val);
			memcpy(params[i].val, &val, params[i].size);
			++btd_opts.defaults.num_entries;
		}
	}
}

static void parse_br_config(GKeyFile *config)
{
	static const struct config_param params[] = {
		{ "PageScanType",
		  &btd_opts.defaults.br.page_scan_type,
		  sizeof(btd_opts.defaults.br.page_scan_type),
		  0,
		  1},
		{ "PageScanInterval",
		  &btd_opts.defaults.br.page_scan_interval,
		  sizeof(btd_opts.defaults.br.page_scan_interval),
		  0x0012,
		  0x1000},
		{ "PageScanWindow",
		  &btd_opts.defaults.br.page_scan_win,
		  sizeof(btd_opts.defaults.br.page_scan_win),
		  0x0011,
		  0x1000},
		{ "InquiryScanType",
		  &btd_opts.defaults.br.scan_type,
		  sizeof(btd_opts.defaults.br.scan_type),
		  0,
		  1},
		{ "InquiryScanInterval",
		  &btd_opts.defaults.br.scan_interval,
		  sizeof(btd_opts.defaults.br.scan_interval),
		  0x0012,
		  0x1000},
		{ "InquiryScanWindow",
		  &btd_opts.defaults.br.scan_win,
		  sizeof(btd_opts.defaults.br.scan_win),
		  0x0011,
		  0x1000},
		{ "LinkSupervisionTimeout",
		  &btd_opts.defaults.br.link_supervision_timeout,
		  sizeof(btd_opts.defaults.br.link_supervision_timeout),
		  0x0001,
		  0xFFFF},
		{ "PageTimeout",
		  &btd_opts.defaults.br.page_timeout,
		  sizeof(btd_opts.defaults.br.page_timeout),
		  0x0001,
		  0xFFFF},
		{ "MinSniffInterval",
		  &btd_opts.defaults.br.min_sniff_interval,
		  sizeof(btd_opts.defaults.br.min_sniff_interval),
		  0x0001,
		  0xFFFE},
		{ "MaxSniffInterval",
		  &btd_opts.defaults.br.max_sniff_interval,
		  sizeof(btd_opts.defaults.br.max_sniff_interval),
		  0x0001,
		  0xFFFE},
	};

	if (btd_opts.mode == BT_MODE_LE)
		return;

	parse_mode_config(config, "BR", params, ARRAY_SIZE(params));
}

static void parse_le_config(GKeyFile *config)
{
	static const struct config_param params[] = {
		{ "MinAdvertisementInterval",
		  &btd_opts.defaults.le.min_adv_interval,
		  sizeof(btd_opts.defaults.le.min_adv_interval),
		  0x0020,
		  0x4000},
		{ "MaxAdvertisementInterval",
		  &btd_opts.defaults.le.max_adv_interval,
		  sizeof(btd_opts.defaults.le.max_adv_interval),
		  0x0020,
		  0x4000},
		{ "MultiAdvertisementRotationInterval",
		  &btd_opts.defaults.le.adv_rotation_interval,
		  sizeof(btd_opts.defaults.le.adv_rotation_interval),
		  0x0001,
		  0xFFFF},
		{ "ScanIntervalAutoConnect",
		  &btd_opts.defaults.le.scan_interval_autoconnect,
		  sizeof(btd_opts.defaults.le.scan_interval_autoconnect),
		  0x0004,
		  0x4000},
		{ "ScanWindowAutoConnect",
		  &btd_opts.defaults.le.scan_win_autoconnect,
		  sizeof(btd_opts.defaults.le.scan_win_autoconnect),
		  0x0004,
		  0x4000},
		{ "ScanIntervalSuspend",
		  &btd_opts.defaults.le.scan_interval_suspend,
		  sizeof(btd_opts.defaults.le.scan_interval_suspend),
		  0x0004,
		  0x4000},
		{ "ScanWindowSuspend",
		  &btd_opts.defaults.le.scan_win_suspend,
		  sizeof(btd_opts.defaults.le.scan_win_suspend),
		  0x0004,
		  0x4000},
		{ "ScanIntervalDiscovery",
		  &btd_opts.defaults.le.scan_interval_discovery,
		  sizeof(btd_opts.defaults.le.scan_interval_discovery),
		  0x0004,
		  0x4000},
		{ "ScanWindowDiscovery",
		  &btd_opts.defaults.le.scan_win_discovery,
		  sizeof(btd_opts.defaults.le.scan_win_discovery),
		  0x0004,
		  0x4000},
		{ "ScanIntervalAdvMonitor",
		  &btd_opts.defaults.le.scan_interval_adv_monitor,
		  sizeof(btd_opts.defaults.le.scan_interval_adv_monitor),
		  0x0004,
		  0x4000},
		{ "ScanWindowAdvMonitor",
		  &btd_opts.defaults.le.scan_win_adv_monitor,
		  sizeof(btd_opts.defaults.le.scan_win_adv_monitor),
		  0x0004,
		  0x4000},
		{ "ScanIntervalConnect",
		  &btd_opts.defaults.le.scan_interval_connect,
		  sizeof(btd_opts.defaults.le.scan_interval_connect),
		  0x0004,
		  0x4000},
		{ "ScanWindowConnect",
		  &btd_opts.defaults.le.scan_win_connect,
		  sizeof(btd_opts.defaults.le.scan_win_connect),
		  0x0004,
		  0x4000},
		{ "MinConnectionInterval",
		  &btd_opts.defaults.le.min_conn_interval,
		  sizeof(btd_opts.defaults.le.min_conn_interval),
		  0x0006,
		  0x0C80},
		{ "MaxConnectionInterval",
		  &btd_opts.defaults.le.max_conn_interval,
		  sizeof(btd_opts.defaults.le.max_conn_interval),
		  0x0006,
		  0x0C80},
		{ "ConnectionLatency",
		  &btd_opts.defaults.le.conn_latency,
		  sizeof(btd_opts.defaults.le.conn_latency),
		  0x0000,
		  0x01F3},
		{ "ConnectionSupervisionTimeout",
		  &btd_opts.defaults.le.conn_lsto,
		  sizeof(btd_opts.defaults.le.conn_lsto),
		  0x000A,
		  0x0C80},
		{ "Autoconnecttimeout",
		  &btd_opts.defaults.le.autoconnect_timeout,
		  sizeof(btd_opts.defaults.le.autoconnect_timeout),
		  0x0001,
		  0x4000},
		{ "AdvMonAllowlistScanDuration",
		  &btd_opts.defaults.le.advmon_allowlist_scan_duration,
		  sizeof(btd_opts.defaults.le.advmon_allowlist_scan_duration),
		  1,
		  10000},
		{ "AdvMonNoFilterScanDuration",
		  &btd_opts.defaults.le.advmon_no_filter_scan_duration,
		  sizeof(btd_opts.defaults.le.advmon_no_filter_scan_duration),
		  1,
		  10000},
		{ "EnableAdvMonInterleaveScan",
		  &btd_opts.defaults.le.enable_advmon_interleave_scan,
		  sizeof(btd_opts.defaults.le.enable_advmon_interleave_scan),
		  0,
		  1},
	};

	if (btd_opts.mode == BT_MODE_BREDR)
		return;

	parse_mode_config(config, "LE", params, ARRAY_SIZE(params));
}

static void parse_config(GKeyFile *config)
{
	GError *err = NULL;
	char *str;
	int val;
	gboolean boolean;

	if (!config)
		return;

	check_config(config);

	DBG("parsing %s", main_conf_file_path);

	val = g_key_file_get_integer(config, "General",
						"DiscoverableTimeout", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("discovto=%d", val);
		btd_opts.discovto = val;
	}

	boolean = g_key_file_get_boolean(config, "General",
						"AlwaysPairable", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("pairable=%s", boolean ? "true" : "false");
		btd_opts.pairable = boolean;
	}

	val = g_key_file_get_integer(config, "General",
						"PairableTimeout", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("pairto=%d", val);
		btd_opts.pairto = val;
	}

	str = g_key_file_get_string(config, "General", "Privacy", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
		btd_opts.privacy = 0x00;
	} else {
		DBG("privacy=%s", str);

		if (!strcmp(str, "device"))
			btd_opts.privacy = 0x01;
		else if (!strcmp(str, "off"))
			btd_opts.privacy = 0x00;
		else {
			DBG("Invalid privacy option: %s", str);
			btd_opts.privacy = 0x00;
		}

		g_free(str);
	}

	str = g_key_file_get_string(config, "General",
						"JustWorksRepairing", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
		btd_opts.jw_repairing = JW_REPAIRING_NEVER;
	} else {
		DBG("just_works_repairing=%s", str);
		btd_opts.jw_repairing = parse_jw_repairing(str);
		g_free(str);
	}

	val = g_key_file_get_integer(config, "General",
						"TemporaryTimeout", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("tmpto=%d", val);
		btd_opts.tmpto = val;
	}

	str = g_key_file_get_string(config, "General", "Name", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("name=%s", str);
		g_free(btd_opts.name);
		btd_opts.name = str;
	}

	str = g_key_file_get_string(config, "General", "Class", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("class=%s", str);
		btd_opts.class = strtol(str, NULL, 16);
		g_free(str);
	}

	str = g_key_file_get_string(config, "General", "DeviceID", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("deviceid=%s", str);
		parse_did(str);
		g_free(str);
	}

	boolean = g_key_file_get_boolean(config, "General",
						"ReverseServiceDiscovery", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else
		btd_opts.reverse_discovery = boolean;

	boolean = g_key_file_get_boolean(config, "General",
						"NameResolving", &err);
	if (err)
		g_clear_error(&err);
	else
		btd_opts.name_resolv = boolean;

	boolean = g_key_file_get_boolean(config, "General",
						"DebugKeys", &err);
	if (err)
		g_clear_error(&err);
	else
		btd_opts.debug_keys = boolean;

	str = g_key_file_get_string(config, "General", "ControllerMode", &err);
	if (err) {
		g_clear_error(&err);
	} else {
		DBG("ControllerMode=%s", str);
		btd_opts.mode = get_mode(str);
		g_free(str);
	}

	str = g_key_file_get_string(config, "General", "MultiProfile", &err);
	if (err) {
		g_clear_error(&err);
	} else {
		DBG("MultiProfile=%s", str);

		if (!strcmp(str, "single"))
			btd_opts.mps = MPS_SINGLE;
		else if (!strcmp(str, "multiple"))
			btd_opts.mps = MPS_MULTIPLE;
		else
			btd_opts.mps = MPS_OFF;

		g_free(str);
	}

	boolean = g_key_file_get_boolean(config, "General",
						"FastConnectable", &err);
	if (err)
		g_clear_error(&err);
	else
		btd_opts.fast_conn = boolean;

	boolean = g_key_file_get_boolean(config, "General",
						"RefreshDiscovery", &err);
	if (err)
		g_clear_error(&err);
	else
		btd_opts.refresh_discovery = boolean;

	str = g_key_file_get_string(config, "GATT", "Cache", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		btd_opts.gatt_cache = parse_gatt_cache(str);
		g_free(str);
	}

	val = g_key_file_get_integer(config, "GATT", "KeySize", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("KeySize=%d", val);

		if (val >=7 && val <= 16)
			btd_opts.key_size = val;
	}

	val = g_key_file_get_integer(config, "GATT", "ExchangeMTU", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		/* Ensure the mtu is within a valid range. */
		val = MIN(val, BT_ATT_MAX_LE_MTU);
		val = MAX(val, BT_ATT_DEFAULT_LE_MTU);
		DBG("ExchangeMTU=%d", val);
		btd_opts.gatt_mtu = val;
	}

	val = g_key_file_get_integer(config, "GATT", "Channels", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("Channels=%d", val);
		/* Ensure the channels is within a valid range. */
		val = MIN(val, 5);
		val = MAX(val, 1);
		btd_opts.gatt_channels = val;
	}

	str = g_key_file_get_string(config, "AVDTP", "SessionMode", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("SessionMode=%s", str);

		if (!strcmp(str, "basic"))
			btd_opts.avdtp.session_mode = BT_IO_MODE_BASIC;
		else if (!strcmp(str, "ertm"))
			btd_opts.avdtp.session_mode = BT_IO_MODE_ERTM;
		else {
			DBG("Invalid mode option: %s", str);
			btd_opts.avdtp.session_mode = BT_IO_MODE_BASIC;
		}
	}

	val = g_key_file_get_integer(config, "AVDTP", "StreamMode", &err);
	if (err) {
		DBG("%s", err->message);
		g_clear_error(&err);
	} else {
		DBG("StreamMode=%s", str);

		if (!strcmp(str, "basic"))
			btd_opts.avdtp.stream_mode = BT_IO_MODE_BASIC;
		else if (!strcmp(str, "streaming"))
			btd_opts.avdtp.stream_mode = BT_IO_MODE_STREAMING;
		else {
			DBG("Invalid mode option: %s", str);
			btd_opts.avdtp.stream_mode = BT_IO_MODE_BASIC;
		}
	}

	parse_br_config(config);
	parse_le_config(config);
}

static void init_defaults(void)
{
	uint8_t major, minor;
	char *macp = NULL;
	unsigned char mac_binary[6];
	macp = get_2g_hwaddr();
	ether_atoe(macp, mac_binary);

	/* Default HCId settings */
	memset(&btd_opts, 0, sizeof(btd_opts));
#if defined(RTCONFIG_REALTEK)
	btd_opts.name = g_strdup_printf("ASUS_%02X_%s", mac_binary[5], nvram_safe_get("productid"));
#elif defined(RTCONFIG_LANTIQ) || defined(RTCONFIG_ALPINE)
	btd_opts.name = g_strdup_printf("ASUS_%02X", mac_binary[5]);
#elif defined(VZWAC1300)
	if (nvram_match("odmpid", "ASUSMESH-AC1300"))
		btd_opts.name = g_strdup_printf("ASUS_%02X_MESH", mac_binary[5]);
	else
		btd_opts.name = g_strdup_printf("VZW_%02X", mac_binary[5]);
#elif defined(RTCONFIG_SSID_AMAPS)
	btd_opts.name = g_strdup_printf("ASUS_%02X_AMAPS", mac_binary[5]);
#else
	btd_opts.name = g_strdup_printf("ASUS_%02X", mac_binary[5]);
#endif
	btd_opts.class = 0x000000;
	btd_opts.pairto = DEFAULT_PAIRABLE_TIMEOUT;
	btd_opts.discovto = DEFAULT_DISCOVERABLE_TIMEOUT;
	btd_opts.tmpto = DEFAULT_TEMPORARY_TIMEOUT;
	btd_opts.reverse_discovery = TRUE;
	btd_opts.name_resolv = TRUE;
	btd_opts.debug_keys = FALSE;
	btd_opts.refresh_discovery = TRUE;
#if !defined(RTAX95Q) && !defined(XT8PRO) && !defined(BM68) && !defined(XT8_V2) && !defined(RTAXE95Q) && !defined(ET8PRO) && !defined(ET8_V2) && !defined(RTAX56_XD4) && !defined(XD4PRO) && !defined(ET12) && !defined(XT12) && !defined(XC5)
	btd_opts.mode = BT_MODE_LE;
#endif

	btd_opts.defaults.num_entries = 0;
	btd_opts.defaults.br.page_scan_type = 0xFFFF;
	btd_opts.defaults.br.scan_type = 0xFFFF;
	btd_opts.defaults.le.enable_advmon_interleave_scan = 0xFF;

	if (sscanf(VERSION, "%hhu.%hhu", &major, &minor) != 2)
		return;

	btd_opts.did_source = 0x0002;		/* USB */
	btd_opts.did_vendor = 0x1d6b;		/* Linux Foundation */
	btd_opts.did_product = 0x0246;		/* BlueZ */
	btd_opts.did_version = (major << 8 | minor);

	btd_opts.gatt_cache = BT_GATT_CACHE_ALWAYS;
	btd_opts.gatt_mtu = BT_ATT_MAX_LE_MTU;
	btd_opts.gatt_channels = 3;

	btd_opts.avdtp.session_mode = BT_IO_MODE_BASIC;
	btd_opts.avdtp.stream_mode = BT_IO_MODE_BASIC;
}

static void log_handler(const gchar *log_domain, GLogLevelFlags log_level,
				const gchar *message, gpointer user_data)
{
	int priority;

	if (log_level & (G_LOG_LEVEL_ERROR |
				G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING))
		priority = 0x03;
	else
		priority = 0x06;

	btd_log(0xffff, priority, "GLib: %s", message);
	btd_backtrace(0xffff);
}

void btd_exit(void)
{
	mainloop_quit();
}

static gboolean quit_eventloop(gpointer user_data)
{
	btd_exit();
	return FALSE;
}

static void signal_callback(int signum, void *user_data)
{
	static bool terminated = false;

	switch (signum) {
	case SIGINT:
	case SIGTERM:
		if (!terminated) {
			info("Terminating");
			g_timeout_add_seconds(SHUTDOWN_GRACE_SECONDS,
							quit_eventloop, NULL);

			mainloop_sd_notify("STATUS=Powering down");
			adapter_shutdown();
		}

		terminated = true;
		break;
	case SIGUSR2:
		__btd_toggle_debug();
		break;
	}
}

static char *option_debug = NULL;
static char *option_plugin = NULL;
static char *option_noplugin = NULL;
static char *option_configfile = NULL;
static gboolean option_compat = FALSE;
static gboolean option_detach = TRUE;
static gboolean option_version = FALSE;
static gboolean option_experimental = FALSE;

static void free_options(void)
{
	g_free(option_debug);
	option_debug = NULL;

	g_free(option_plugin);
	option_plugin = NULL;

	g_free(option_noplugin);
	option_noplugin = NULL;

	g_free(option_configfile);
	option_configfile = NULL;
}

static void disconnect_dbus(void)
{
	DBusConnection *conn = btd_get_dbus_connection();

	if (!conn || !dbus_connection_get_is_connected(conn))
		return;

	g_dbus_detach_object_manager(conn);
	set_dbus_connection(NULL);

	dbus_connection_unref(conn);
}

static void disconnected_dbus(DBusConnection *conn, void *data)
{
	info("Disconnected from D-Bus. Exiting.");
	mainloop_quit();
}

static int connect_dbus(void)
{
	DBusConnection *conn;
	DBusError err;

	dbus_error_init(&err);

	conn = g_dbus_setup_bus(DBUS_BUS_SYSTEM, BLUEZ_NAME, &err);
	if (!conn) {
		if (dbus_error_is_set(&err)) {
			g_printerr("D-Bus setup failed: %s\n", err.message);
			dbus_error_free(&err);
			return -EIO;
		}
		return -EALREADY;
	}

	set_dbus_connection(conn);

	g_dbus_set_disconnect_function(conn, disconnected_dbus, NULL, NULL);
	g_dbus_attach_object_manager(conn);

	return 0;
}

static gboolean parse_debug(const char *key, const char *value,
				gpointer user_data, GError **error)
{
	if (value)
		option_debug = g_strdup(value);
	else
		option_debug = g_strdup("*");

	return TRUE;
}

static GOptionEntry options[] = {
	{ "debug", 'd', G_OPTION_FLAG_OPTIONAL_ARG,
				G_OPTION_ARG_CALLBACK, parse_debug,
				"Specify debug options to enable", "DEBUG" },
	{ "plugin", 'p', 0, G_OPTION_ARG_STRING, &option_plugin,
				"Specify plugins to load", "NAME,..," },
	{ "noplugin", 'P', 0, G_OPTION_ARG_STRING, &option_noplugin,
				"Specify plugins not to load", "NAME,..." },
	{ "configfile", 'f', 0, G_OPTION_ARG_STRING, &option_configfile,
			"Specify an explicit path to the config file", "FILE"},
	{ "compat", 'C', 0, G_OPTION_ARG_NONE, &option_compat,
				"Provide deprecated command line interfaces" },
	{ "experimental", 'E', 0, G_OPTION_ARG_NONE, &option_experimental,
				"Enable experimental interfaces" },
	{ "nodetach", 'n', G_OPTION_FLAG_REVERSE,
				G_OPTION_ARG_NONE, &option_detach,
				"Run with logging in foreground" },
	{ "version", 'v', 0, G_OPTION_ARG_NONE, &option_version,
				"Show version information and exit" },
	{ NULL },
};

int main(int argc, char *argv[])
{
	GOptionContext *context;
	GError *err = NULL;
	uint16_t sdp_mtu = 0;
	uint32_t sdp_flags = 0;
	int gdbus_flags = 0;

	init_defaults();

	context = g_option_context_new(NULL);
	g_option_context_add_main_entries(context, options, NULL);

	if (g_option_context_parse(context, &argc, &argv, &err) == FALSE) {
		if (err != NULL) {
			g_printerr("%s\n", err->message);
			g_error_free(err);
		} else
			g_printerr("An unknown error occurred\n");
		exit(1);
	}

	g_option_context_free(context);

	if (option_version == TRUE) {
		printf("%s\n", VERSION);
		exit(0);
	}

	umask(0077);

	btd_backtrace_init();

	mainloop_init();

	__btd_log_init(option_debug, option_detach);

	g_log_set_handler("GLib", G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL |
							G_LOG_FLAG_RECURSION,
							log_handler, NULL);

	mainloop_sd_notify("STATUS=Starting up");

	if (option_configfile)
		main_conf_file_path = option_configfile;
	else
		main_conf_file_path = CONFIGDIR "/main.conf";

	main_conf = load_config(main_conf_file_path);

	parse_config(main_conf);

	if (connect_dbus() < 0) {
		error("Unable to get on D-Bus");
		exit(1);
	}

	if (option_experimental)
		gdbus_flags = G_DBUS_FLAG_ENABLE_EXPERIMENTAL;

	g_dbus_set_flags(gdbus_flags);

	if (adapter_init() < 0) {
		error("Adapter handling initialization failed");
		exit(1);
	}

	btd_device_init();
	btd_agent_init();
	btd_profile_init();

	if (btd_opts.mode != BT_MODE_LE) {
		if (option_compat == TRUE)
			sdp_flags |= SDP_SERVER_COMPAT;

		start_sdp_server(sdp_mtu, sdp_flags);

		if (btd_opts.did_source > 0)
			register_device_id(btd_opts.did_source,
						btd_opts.did_vendor,
						btd_opts.did_product,
						btd_opts.did_version);
	}

	if (btd_opts.mps != MPS_OFF)
		register_mps(btd_opts.mps == MPS_MULTIPLE);

	/* Loading plugins has to be done after D-Bus has been setup since
	 * the plugins might wanna expose some paths on the bus. However the
	 * best order of how to init various subsystems of the Bluetooth
	 * daemon needs to be re-worked. */
	plugin_init(option_plugin, option_noplugin);

	/* no need to keep parsed option in memory */
	free_options();

	rfkill_init();

	DBG("Entering main loop");

	mainloop_sd_notify("STATUS=Running");
	mainloop_sd_notify("READY=1");

	mainloop_run_with_signal(signal_callback, NULL);

	mainloop_sd_notify("STATUS=Quitting");

	plugin_cleanup();

	btd_profile_cleanup();
	btd_agent_cleanup();
	btd_device_cleanup();

	adapter_cleanup();

	rfkill_exit();

	if (btd_opts.mode != BT_MODE_LE)
		stop_sdp_server();

	if (main_conf)
		g_key_file_free(main_conf);

	disconnect_dbus();

	info("Exit");

	__btd_log_cleanup();

	return 0;
}
