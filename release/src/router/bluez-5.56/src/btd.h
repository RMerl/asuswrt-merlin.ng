/* SPDX-License-Identifier: GPL-2.0-or-later */
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

typedef enum {
	BT_MODE_DUAL,
	BT_MODE_BREDR,
	BT_MODE_LE,
} bt_mode_t;

typedef enum {
	BT_GATT_CACHE_ALWAYS,
	BT_GATT_CACHE_YES,
	BT_GATT_CACHE_NO,
} bt_gatt_cache_t;

enum jw_repairing_t {
	JW_REPAIRING_NEVER,
	JW_REPAIRING_CONFIRM,
	JW_REPAIRING_ALWAYS,
};

enum mps_mode_t {
	MPS_OFF,
	MPS_SINGLE,
	MPS_MULTIPLE,
};

struct btd_br_defaults {
	uint16_t	page_scan_type;
	uint16_t	page_scan_interval;
	uint16_t	page_scan_win;

	uint16_t	scan_type;
	uint16_t	scan_interval;
	uint16_t	scan_win;

	uint16_t	link_supervision_timeout;
	uint16_t	page_timeout;

	uint16_t	min_sniff_interval;
	uint16_t	max_sniff_interval;
};

struct btd_le_defaults {
	uint16_t	min_adv_interval;
	uint16_t	max_adv_interval;
	uint16_t	adv_rotation_interval;

	uint16_t	scan_interval_autoconnect;
	uint16_t	scan_win_autoconnect;
	uint16_t	scan_interval_suspend;
	uint16_t	scan_win_suspend;
	uint16_t	scan_interval_discovery;
	uint16_t	scan_win_discovery;
	uint16_t	scan_interval_adv_monitor;
	uint16_t	scan_win_adv_monitor;
	uint16_t	scan_interval_connect;
	uint16_t	scan_win_connect;

	uint16_t	min_conn_interval;
	uint16_t	max_conn_interval;
	uint16_t	conn_latency;
	uint16_t	conn_lsto;
	uint16_t	autoconnect_timeout;

	uint16_t	advmon_allowlist_scan_duration;
	uint16_t	advmon_no_filter_scan_duration;
	uint8_t		enable_advmon_interleave_scan;
};

struct btd_defaults {
	uint16_t	num_entries;

	struct btd_br_defaults br;
	struct btd_le_defaults le;
};

struct btd_avdtp_opts {
	uint8_t  session_mode;
	uint8_t  stream_mode;
};

struct btd_opts {
	char		*name;
	uint32_t	class;
	gboolean	pairable;
	uint32_t	pairto;
	uint32_t	discovto;
	uint32_t	tmpto;
	uint8_t		privacy;

	struct btd_defaults defaults;

	gboolean	reverse_discovery;
	gboolean	name_resolv;
	gboolean	debug_keys;
	gboolean	fast_conn;
	gboolean	refresh_discovery;

	uint16_t	did_source;
	uint16_t	did_vendor;
	uint16_t	did_product;
	uint16_t	did_version;

	bt_mode_t	mode;
	bt_gatt_cache_t gatt_cache;
	uint16_t	gatt_mtu;
	uint8_t		gatt_channels;
	enum mps_mode_t	mps;

	struct btd_avdtp_opts avdtp;

	uint8_t		key_size;

	enum jw_repairing_t jw_repairing;
};

extern struct btd_opts btd_opts;

gboolean plugin_init(const char *enable, const char *disable);
void plugin_cleanup(void);

void rfkill_init(void);
void rfkill_exit(void);

GKeyFile *btd_get_main_conf(void);

void btd_exit(void);
