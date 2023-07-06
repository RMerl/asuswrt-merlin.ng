// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <dirent.h>

#include <glib.h>
#include <dbus/dbus.h>

#include "bluetooth/bluetooth.h"
#include "bluetooth/hci.h"
#include "bluetooth/hci_lib.h"
#include "bluetooth/sdp.h"
#include "bluetooth/sdp_lib.h"
#include "lib/uuid.h"
#include "lib/mgmt.h"

#include "gdbus/gdbus.h"

#include "log.h"
#include "textfile.h"

#include "src/shared/mgmt.h"
#include "src/shared/util.h"
#include "src/shared/queue.h"
#include "src/shared/att.h"
#include "src/shared/gatt-db.h"

#include "btio/btio.h"
#include "btd.h"
#include "sdpd.h"
#include "adapter.h"
#include "device.h"
#include "profile.h"
#include "dbus-common.h"
#include "error.h"
#include "uuid-helper.h"
#include "agent.h"
#include "storage.h"
#include "attrib/gattrib.h"
#include "attrib/att.h"
#include "attrib/gatt.h"
#include "attrib-server.h"
#include "gatt-database.h"
#include "advertising.h"
#include "adv_monitor.h"
#include "eir.h"
#define TYPEDEF_BOOL
#include <bcmnvram.h>
#include <bcmparams.h>
#include <shared.h>
#include "battery.h"

#define MODE_OFF		0x00
#define MODE_CONNECTABLE	0x01
#define MODE_DISCOVERABLE	0x02
#define MODE_UNKNOWN		0xff

#define CONN_SCAN_TIMEOUT (3)
#define IDLE_DISCOV_TIMEOUT (5)
#define TEMP_DEV_TIMEOUT (3 * 60)
#define BONDING_TIMEOUT (2 * 60)

#define SCAN_TYPE_BREDR (1 << BDADDR_BREDR)
#define SCAN_TYPE_LE ((1 << BDADDR_LE_PUBLIC) | (1 << BDADDR_LE_RANDOM))
#define SCAN_TYPE_DUAL (SCAN_TYPE_BREDR | SCAN_TYPE_LE)

#define HCI_RSSI_INVALID	127
#define DISTANCE_VAL_INVALID	0x7FFF
#define PATHLOSS_MAX		137

/*
 * These are known security keys that have been compromised.
 * If this grows or there are needs to be platform specific, it is
 * conceivable that these could be read from a config file.
 */
static const struct mgmt_blocked_key_info blocked_keys[] = {
	/* Google Titan Security Keys */
	{ HCI_BLOCKED_KEY_TYPE_LTK,
		{0xbf, 0x01, 0xfb, 0x9d, 0x4e, 0xf3, 0xbc, 0x36,
		 0xd8, 0x74, 0xf5, 0x39, 0x41, 0x38, 0x68, 0x4c}},
	{ HCI_BLOCKED_KEY_TYPE_IRK,
		{0xa5, 0x99, 0xba, 0xe4, 0xe1, 0x7c, 0xa6, 0x18,
		 0x22, 0x8e, 0x07, 0x56, 0xb4, 0xe8, 0x5f, 0x01}},
};

static DBusConnection *dbus_conn = NULL;

static uint32_t kernel_features = 0;

static GList *adapter_list = NULL;
static unsigned int adapter_remaining = 0;
static bool powering_down = false;

static GSList *adapters = NULL;

static struct mgmt *mgmt_master = NULL;

static uint8_t mgmt_version = 0;
static uint8_t mgmt_revision = 0;

static GSList *adapter_drivers = NULL;

static GSList *disconnect_list = NULL;
static GSList *conn_fail_list = NULL;

struct link_key_info {
	bdaddr_t bdaddr;
	unsigned char key[16];
	uint8_t type;
	uint8_t pin_len;
	bool is_blocked;
};

struct smp_ltk_info {
	bdaddr_t bdaddr;
	uint8_t bdaddr_type;
	uint8_t authenticated;
	bool master;
	uint8_t enc_size;
	uint16_t ediv;
	uint64_t rand;
	uint8_t val[16];
	bool is_blocked;
};

struct irk_info {
	bdaddr_t bdaddr;
	uint8_t bdaddr_type;
	uint8_t val[16];
	bool is_blocked;
};

struct conn_param {
	bdaddr_t bdaddr;
	uint8_t  bdaddr_type;
	uint16_t min_interval;
	uint16_t max_interval;
	uint16_t latency;
	uint16_t timeout;
};

struct discovery_filter {
	uint8_t type;
	char *pattern;
	uint16_t pathloss;
	int16_t rssi;
	GSList *uuids;
	bool duplicate;
	bool discoverable;
};

struct discovery_client {
	struct btd_adapter *adapter;
	DBusMessage *msg;
	char *owner;
	guint watch;
	struct discovery_filter *discovery_filter;
};

struct service_auth {
	guint id;
	unsigned int svc_id;
	service_auth_cb cb;
	void *user_data;
	const char *uuid;
	struct btd_device *device;
	struct btd_adapter *adapter;
	struct agent *agent;		/* NULL for queued auths */
};

struct btd_adapter_pin_cb_iter {
	GSList *it;			/* current callback function */
	unsigned int attempt;		/* numer of times it() was called */
	/* When the iterator reaches the end, it is NULL and attempt is 0 */
};

struct btd_adapter {
	int ref_count;

	uint16_t dev_id;
	struct mgmt *mgmt;

	bdaddr_t bdaddr;		/* controller Bluetooth address */
	uint8_t bdaddr_type;		/* address type */
	uint32_t dev_class;		/* controller class of device */
	char *name;			/* controller device name */
	char *short_name;		/* controller short name */
	uint32_t supported_settings;	/* controller supported settings */
	uint32_t pending_settings;	/* pending controller settings */
	uint32_t current_settings;	/* current controller settings */

	char *path;			/* adapter object path */
	uint16_t manufacturer;		/* adapter manufacturer */
	uint8_t major_class;		/* configured major class */
	uint8_t minor_class;		/* configured minor class */
	char *system_name;		/* configured system name */
	char *modalias;			/* device id (modalias) */
	bool stored_discoverable;	/* stored discoverable mode */
	uint32_t discoverable_timeout;	/* discoverable time(sec) */
	uint32_t pairable_timeout;	/* pairable time(sec) */

	char *current_alias;		/* current adapter name alias */
	char *stored_alias;		/* stored adapter name alias */

	bool discovering;		/* discovering property state */
	bool filtered_discovery;	/* we are doing filtered discovery */
	bool no_scan_restart_delay;	/* when this flag is set, restart scan
					 * without delay */
	uint8_t discovery_type;		/* current active discovery type */
	uint8_t discovery_enable;	/* discovery enabled/disabled */
	bool discovery_suspended;	/* discovery has been suspended */
	bool discovery_discoverable;	/* discoverable while discovering */
	GSList *discovery_list;		/* list of discovery clients */
	GSList *set_filter_list;	/* list of clients that specified
					 * filter, but don't scan yet
					 */
	/* current discovery filter, if any */
	struct mgmt_cp_start_service_discovery *current_discovery_filter;
	struct discovery_client *client;	/* active discovery client */

	GSList *discovery_found;	/* list of found devices */
	guint discovery_idle_timeout;	/* timeout between discovery runs */
	guint passive_scan_timeout;	/* timeout between passive scans */

	guint pairable_timeout_id;	/* pairable timeout id */
	guint auth_idle_id;		/* Pending authorization dequeue */
	GQueue *auths;			/* Ongoing and pending auths */
	bool pincode_requested;		/* PIN requested during last bonding */
	GSList *connections;		/* Connected devices */
	GSList *devices;		/* Devices structure pointers */
	GSList *connect_list;		/* Devices to connect when found */
	struct btd_device *connect_le;	/* LE device waiting to be connected */
	sdp_list_t *services;		/* Services associated to adapter */

	struct btd_gatt_database *database;
	struct btd_adv_manager *adv_manager;

	struct btd_adv_monitor_manager *adv_monitor_manager;

	struct btd_battery_provider_manager *battery_provider_manager;

	gboolean initialized;

	GSList *pin_callbacks;
	GSList *msd_callbacks;

	GSList *drivers;
	GSList *profiles;

	struct oob_handler *oob_handler;

	unsigned int load_ltks_id;
	guint load_ltks_timeout;

	unsigned int confirm_name_id;
	guint confirm_name_timeout;

	unsigned int pair_device_id;
	guint pair_device_timeout;

	unsigned int db_id;		/* Service event handler for GATT db */

	bool is_default;		/* true if adapter is default one */

	bool le_simult_roles_supported;
};

typedef enum {
	ADAPTER_AUTHORIZE_DISCONNECTED = 0,
	ADAPTER_AUTHORIZE_CHECK_CONNECTED
} adapter_authorize_type;

static struct btd_adapter *btd_adapter_lookup(uint16_t index)
{
	GList *list;

	for (list = g_list_first(adapter_list); list;
						list = g_list_next(list)) {
		struct btd_adapter *adapter = list->data;

		if (adapter->dev_id == index)
			return adapter;
	}

	return NULL;
}

struct btd_adapter *btd_adapter_get_default(void)
{
	GList *list;

	for (list = g_list_first(adapter_list); list;
						list = g_list_next(list)) {
		struct btd_adapter *adapter = list->data;

		if (adapter->is_default)
			return adapter;
	}

	return NULL;
}

bool btd_adapter_is_default(struct btd_adapter *adapter)
{
	if (!adapter)
		return false;

	return adapter->is_default;
}

uint16_t btd_adapter_get_index(struct btd_adapter *adapter)
{
	if (!adapter)
		return MGMT_INDEX_NONE;

	return adapter->dev_id;
}

static gboolean process_auth_queue(gpointer user_data);

static void dev_class_changed_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	const struct mgmt_cod *rp = param;
	uint32_t dev_class;

	if (length < sizeof(*rp)) {
		btd_error(adapter->dev_id,
			"Wrong size of class of device changed parameters");
		return;
	}

	dev_class = rp->val[0] | (rp->val[1] << 8) | (rp->val[2] << 16);

	if (dev_class == adapter->dev_class)
		return;

	DBG("Class: 0x%06x", dev_class);

	adapter->dev_class = dev_class;

	g_dbus_emit_property_changed(dbus_conn, adapter->path,
						ADAPTER_INTERFACE, "Class");
}

static void set_dev_class_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
				"Failed to set device class: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	/*
	 * The parameters are identical and also the task that is
	 * required in both cases. So it is safe to just call the
	 * event handling functions here.
	 */
	dev_class_changed_callback(adapter->dev_id, length, param, adapter);
}

static void set_dev_class(struct btd_adapter *adapter)
{
	struct mgmt_cp_set_dev_class cp;

	/*
	 * If the controller does not support BR/EDR operation,
	 * there is no point in trying to set a major and minor
	 * class value.
	 *
	 * This is an optimization for Low Energy only controllers.
	 */
	if (!(adapter->supported_settings & MGMT_SETTING_BREDR))
		return;

	memset(&cp, 0, sizeof(cp));

	/*
	 * Silly workaround for a really stupid kernel bug :(
	 *
	 * All current kernel versions assign the major and minor numbers
	 * straight to dev_class[0] and dev_class[1] without considering
	 * the proper bit shifting.
	 *
	 * To make this work, shift the value in userspace for now until
	 * we get a fixed kernel version.
	 */
	cp.major = adapter->major_class & 0x1f;
	cp.minor = adapter->minor_class << 2;

	DBG("sending set device class command for index %u", adapter->dev_id);

	if (mgmt_send(adapter->mgmt, MGMT_OP_SET_DEV_CLASS,
				adapter->dev_id, sizeof(cp), &cp,
				set_dev_class_complete, adapter, NULL) > 0)
		return;

	btd_error(adapter->dev_id,
		"Failed to set class of device for index %u", adapter->dev_id);
}

void btd_adapter_set_class(struct btd_adapter *adapter, uint8_t major,
							uint8_t minor)
{
	if (adapter->major_class == major && adapter->minor_class == minor)
		return;

	DBG("class: major %u minor %u", major, minor);

	adapter->major_class = major;
	adapter->minor_class = minor;

	set_dev_class(adapter);
}

static uint8_t get_mode(const char *mode)
{
	if (strcasecmp("off", mode) == 0)
		return MODE_OFF;
	else if (strcasecmp("connectable", mode) == 0)
		return MODE_CONNECTABLE;
	else if (strcasecmp("discoverable", mode) == 0)
		return MODE_DISCOVERABLE;
	else
		return MODE_UNKNOWN;
}

const char *btd_adapter_get_storage_dir(struct btd_adapter *adapter)
{
	static char dir[25];

	if (adapter->bdaddr_type == BDADDR_LE_RANDOM) {
		strcpy(dir, "static-");
		ba2str(&adapter->bdaddr, dir + 7);
	} else {
		ba2str(&adapter->bdaddr, dir);
	}

	return dir;
}

uint8_t btd_adapter_get_address_type(struct btd_adapter *adapter)
{
	return adapter->bdaddr_type;
}

static void store_adapter_info(struct btd_adapter *adapter)
{
	GKeyFile *key_file;
	char filename[PATH_MAX];
	char *str;
	gsize length = 0;
	gboolean discoverable;

	key_file = g_key_file_new();

	if (adapter->pairable_timeout != btd_opts.pairto)
		g_key_file_set_integer(key_file, "General", "PairableTimeout",
					adapter->pairable_timeout);

	if ((adapter->current_settings & MGMT_SETTING_DISCOVERABLE) &&
						!adapter->discoverable_timeout)
		discoverable = TRUE;
	else
		discoverable = FALSE;

	g_key_file_set_boolean(key_file, "General", "Discoverable",
							discoverable);

	if (adapter->discoverable_timeout != btd_opts.discovto)
		g_key_file_set_integer(key_file, "General",
					"DiscoverableTimeout",
					adapter->discoverable_timeout);

	if (adapter->stored_alias)
		g_key_file_set_string(key_file, "General", "Alias",
							adapter->stored_alias);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/settings",
					btd_adapter_get_storage_dir(adapter));

	create_file(filename, S_IRUSR | S_IWUSR);

	str = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, str, length, NULL);
	g_free(str);

	g_key_file_free(key_file);
}

static void trigger_pairable_timeout(struct btd_adapter *adapter);
static void adapter_start(struct btd_adapter *adapter);
static void adapter_stop(struct btd_adapter *adapter);
static void trigger_passive_scanning(struct btd_adapter *adapter);
static bool set_mode(struct btd_adapter *adapter, uint16_t opcode,
							uint8_t mode);

static void settings_changed(struct btd_adapter *adapter, uint32_t settings)
{
	uint32_t changed_mask;

	changed_mask = adapter->current_settings ^ settings;

	adapter->current_settings = settings;
	adapter->pending_settings &= ~changed_mask;

	DBG("Changed settings: 0x%08x", changed_mask);
	DBG("Pending settings: 0x%08x", adapter->pending_settings);

	if (changed_mask & MGMT_SETTING_POWERED) {
	        g_dbus_emit_property_changed(dbus_conn, adapter->path,
					ADAPTER_INTERFACE, "Powered");

		if (adapter->current_settings & MGMT_SETTING_POWERED) {
			adapter_start(adapter);
		} else {
			adapter_stop(adapter);

			if (powering_down) {
				adapter_remaining--;

				if (!adapter_remaining)
					btd_exit();
			}
		}
	}

	if ((changed_mask & MGMT_SETTING_LE) &&
				btd_adapter_get_powered(adapter) &&
				(adapter->current_settings & MGMT_SETTING_LE))
		trigger_passive_scanning(adapter);

	if (changed_mask & MGMT_SETTING_DISCOVERABLE) {
		g_dbus_emit_property_changed(dbus_conn, adapter->path,
					ADAPTER_INTERFACE, "Discoverable");
		store_adapter_info(adapter);
		btd_adv_manager_refresh(adapter->adv_manager);
	}

	if (changed_mask & MGMT_SETTING_BONDABLE) {
		g_dbus_emit_property_changed(dbus_conn, adapter->path,
					ADAPTER_INTERFACE, "Pairable");

		trigger_pairable_timeout(adapter);
	}
}

static void new_settings_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	uint32_t settings;

	if (length < sizeof(settings)) {
		btd_error(adapter->dev_id,
				"Wrong size of new settings parameters");
		return;
	}

	settings = get_le32(param);

	if (settings == adapter->current_settings)
		return;

	DBG("Settings: 0x%08x", settings);

	settings_changed(adapter, settings);
}

static void set_mode_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id, "Failed to set mode: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	/*
	 * The parameters are identical and also the task that is
	 * required in both cases. So it is safe to just call the
	 * event handling functions here.
	 */
	new_settings_callback(adapter->dev_id, length, param, adapter);
}

static void remove_temporary_devices(struct btd_adapter *adapter)
{
	GSList *l, *next;

	for (l = adapter->devices; l; l = next) {
		struct btd_device *dev = l->data;

		next = g_slist_next(l);
		if (device_is_temporary(dev))
			btd_adapter_remove_device(adapter, dev);
	}
}

static bool set_mode(struct btd_adapter *adapter, uint16_t opcode,
							uint8_t mode)
{
	struct mgmt_mode cp;
	uint32_t setting = 0;

	memset(&cp, 0, sizeof(cp));
	cp.val = mode;

	switch (mode) {
	case MGMT_OP_SET_POWERED:
		setting = MGMT_SETTING_POWERED;
		break;
	case MGMT_OP_SET_CONNECTABLE:
		setting = MGMT_SETTING_CONNECTABLE;
		break;
	case MGMT_OP_SET_FAST_CONNECTABLE:
		setting = MGMT_SETTING_FAST_CONNECTABLE;
		break;
	case MGMT_OP_SET_DISCOVERABLE:
		setting = MGMT_SETTING_DISCOVERABLE;
		break;
	case MGMT_OP_SET_BONDABLE:
		setting = MGMT_SETTING_BONDABLE;
		break;
	}

	adapter->pending_settings |= setting;

	DBG("sending set mode[%u] command for index %u opcode 0x%04x", mode, adapter->dev_id, opcode);

	if (mgmt_send(adapter->mgmt, opcode,
				adapter->dev_id, sizeof(cp), &cp,
				set_mode_complete, adapter, NULL) > 0)
		return true;

	btd_error(adapter->dev_id, "Failed to set mode for index %u",
							adapter->dev_id);

	return false;
}

static bool set_discoverable(struct btd_adapter *adapter, uint8_t mode,
							uint16_t timeout)
{
	struct mgmt_cp_set_discoverable cp;

	memset(&cp, 0, sizeof(cp));
	cp.val = mode;
	cp.timeout = htobs(timeout);

	DBG("sending set mode command for index %u", adapter->dev_id);

	if (btd_has_kernel_features(KERNEL_CONN_CONTROL)) {
		if (mode)
			set_mode(adapter, MGMT_OP_SET_CONNECTABLE, mode);
		else
			/* This also disables discoverable so we're done */
			return set_mode(adapter, MGMT_OP_SET_CONNECTABLE,
									mode);
	}

	if (mgmt_send(adapter->mgmt, MGMT_OP_SET_DISCOVERABLE,
				adapter->dev_id, sizeof(cp), &cp,
				set_mode_complete, adapter, NULL) > 0)
		return true;

	btd_error(adapter->dev_id, "Failed to set mode for index %u",
							adapter->dev_id);

	return false;
}

static gboolean pairable_timeout_handler(gpointer user_data)
{
	struct btd_adapter *adapter = user_data;

	adapter->pairable_timeout_id = 0;

	set_mode(adapter, MGMT_OP_SET_BONDABLE, 0x00);

	return FALSE;
}

static void trigger_pairable_timeout(struct btd_adapter *adapter)
{
	if (adapter->pairable_timeout_id > 0) {
		g_source_remove(adapter->pairable_timeout_id);
		adapter->pairable_timeout_id = 0;
	}

	if (!(adapter->current_settings & MGMT_SETTING_BONDABLE))
		return;

	if (adapter->pairable_timeout > 0)
		adapter->pairable_timeout_id =
			g_timeout_add_seconds(adapter->pairable_timeout,
					pairable_timeout_handler, adapter);
}

static void local_name_changed_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	const struct mgmt_cp_set_local_name *rp = param;

	if (length < sizeof(*rp)) {
		btd_error(adapter->dev_id,
				"Wrong size of local name changed parameters");
		return;
	}

	if (!g_strcmp0(adapter->short_name, (const char *) rp->short_name) &&
			!g_strcmp0(adapter->name, (const char *) rp->name))
		return;

	DBG("Name: %s", rp->name);
	DBG("Short name: %s", rp->short_name);

	g_free(adapter->name);
	adapter->name = g_strdup((const char *) rp->name);

	g_free(adapter->short_name);
	adapter->short_name = g_strdup((const char *) rp->short_name);

	/*
	 * Changing the name (even manually via HCI) will update the
	 * current alias property.
	 *
	 * In case the name is empty, use the short name.
	 *
	 * There is a difference between the stored alias (which is
	 * configured by the user) and the current alias. The current
	 * alias is temporary for the lifetime of the daemon.
	 */
	if (adapter->name && adapter->name[0] != '\0') {
		g_free(adapter->current_alias);
		adapter->current_alias = g_strdup(adapter->name);
	} else {
		g_free(adapter->current_alias);
		adapter->current_alias = g_strdup(adapter->short_name);
	}

	DBG("Current alias: %s", adapter->current_alias);

	if (!adapter->current_alias)
		return;

	g_dbus_emit_property_changed(dbus_conn, adapter->path,
						ADAPTER_INTERFACE, "Alias");

	attrib_gap_set(adapter, GATT_CHARAC_DEVICE_NAME,
				(const uint8_t *) adapter->current_alias,
					strlen(adapter->current_alias));
}

static void set_local_name_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
				"Failed to set local name: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	/*
	 * The parameters are identical and also the task that is
	 * required in both cases. So it is safe to just call the
	 * event handling functions here.
	 */
	local_name_changed_callback(adapter->dev_id, length, param, adapter);
}

static int set_name(struct btd_adapter *adapter, const char *name)
{
	struct mgmt_cp_set_local_name cp;
	char maxname[MAX_NAME_LENGTH];

	memset(maxname, 0, sizeof(maxname));
	strncpy(maxname, name, MAX_NAME_LENGTH - 1);

	if (!g_utf8_validate(maxname, -1, NULL)) {
		btd_error(adapter->dev_id,
			"Name change failed: supplied name isn't valid UTF-8");
		return -EINVAL;
	}

	memset(&cp, 0, sizeof(cp));
	strncpy((char *) cp.name, maxname, sizeof(cp.name) - 1);

	DBG("sending set local name command for index %u", adapter->dev_id);

	if (mgmt_send(adapter->mgmt, MGMT_OP_SET_LOCAL_NAME,
				adapter->dev_id, sizeof(cp), &cp,
				set_local_name_complete, adapter, NULL) > 0)
		return 0;

	btd_error(adapter->dev_id, "Failed to set local name for index %u",
							adapter->dev_id);

	return -EIO;
}

int adapter_set_name(struct btd_adapter *adapter, const char *name)
{
	if (g_strcmp0(adapter->system_name, name) == 0)
		return 0;

	DBG("name: %s", name);

	g_free(adapter->system_name);
	adapter->system_name = g_strdup(name);

	g_dbus_emit_property_changed(dbus_conn, adapter->path,
						ADAPTER_INTERFACE, "Name");

	/* alias is preferred over system name */
	if (adapter->stored_alias)
		return 0;

	DBG("alias: %s", name);

	g_dbus_emit_property_changed(dbus_conn, adapter->path,
						ADAPTER_INTERFACE, "Alias");

	return set_name(adapter, name);
}

struct btd_device *btd_adapter_find_device(struct btd_adapter *adapter,
							const bdaddr_t *dst,
							uint8_t bdaddr_type)
{
	struct device_addr_type addr;
	struct btd_device *device;
	GSList *list;

	if (!adapter)
		return NULL;

	bacpy(&addr.bdaddr, dst);
	addr.bdaddr_type = bdaddr_type;

	list = g_slist_find_custom(adapter->devices, &addr,
							device_addr_type_cmp);
	if (!list)
		return NULL;

	device = list->data;

	DBG("Type : %s", bdaddr_type==BDADDR_BREDR?"BR/EDR":"LE");
	/*
	 * If we're looking up based on public address and the address
	 * was not previously used over this bearer we may need to
	 * update LE or BR/EDR support information.
	 */
	if (bdaddr_type == BDADDR_BREDR)
		device_set_bredr_support(device);
	else
		device_set_le_support(device, bdaddr_type);

	return device;
}

static int device_path_cmp(gconstpointer a, gconstpointer b)
{
	const struct btd_device *device = a;
	const char *path = b;
	const char *dev_path = device_get_path(device);

	return strcasecmp(dev_path, path);
}

struct btd_device *btd_adapter_find_device_by_path(struct btd_adapter *adapter,
						   const char *path)
{
	GSList *list;

	if (!adapter)
		return NULL;

	list = g_slist_find_custom(adapter->devices, path, device_path_cmp);
	if (!list)
		return NULL;

	return list->data;
}

static void uuid_to_uuid128(uuid_t *uuid128, const uuid_t *uuid)
{
	if (uuid->type == SDP_UUID16)
		sdp_uuid16_to_uuid128(uuid128, uuid);
	else if (uuid->type == SDP_UUID32)
		sdp_uuid32_to_uuid128(uuid128, uuid);
	else
		memcpy(uuid128, uuid, sizeof(*uuid));
}

static bool is_supported_uuid(const uuid_t *uuid)
{
	uuid_t tmp;

	DBG("");
	/* mgmt versions from 1.3 onwards support all types of UUIDs */
	if (MGMT_VERSION(mgmt_version, mgmt_revision) >= MGMT_VERSION(1, 3))
		return true;

	uuid_to_uuid128(&tmp, uuid);

	if (!sdp_uuid128_to_uuid(&tmp))
		return false;

	if (tmp.type != SDP_UUID16)
		return false;

	return true;
}

static void add_uuid_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	DBG("");
	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id, "Failed to add UUID: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	/*
	 * The parameters are identical and also the task that is
	 * required in both cases. So it is safe to just call the
	 * event handling functions here.
	 */
	dev_class_changed_callback(adapter->dev_id, length, param, adapter);

	if (adapter->initialized)
		g_dbus_emit_property_changed(dbus_conn, adapter->path,
						ADAPTER_INTERFACE, "UUIDs");
}

static int add_uuid(struct btd_adapter *adapter, uuid_t *uuid, uint8_t svc_hint)
{
	struct mgmt_cp_add_uuid cp;
	uuid_t uuid128;
	uint128_t uint128;

	DBG("");
	if (!is_supported_uuid(uuid)) {
		btd_warn(adapter->dev_id,
				"Ignoring unsupported UUID for addition");
		return 0;
	}

	uuid_to_uuid128(&uuid128, uuid);

	ntoh128((uint128_t *) uuid128.value.uuid128.data, &uint128);
	htob128(&uint128, (uint128_t *) cp.uuid);
	cp.svc_hint = svc_hint;

	DBG("sending add uuid command for index %u", adapter->dev_id);

	if (mgmt_send(adapter->mgmt, MGMT_OP_ADD_UUID,
				adapter->dev_id, sizeof(cp), &cp,
				add_uuid_complete, adapter, NULL) > 0)
		return 0;

	btd_error(adapter->dev_id, "Failed to add UUID for index %u",
							adapter->dev_id);

	return -EIO;
}

static void remove_uuid_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id, "Failed to remove UUID: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	/*
	 * The parameters are identical and also the task that is
	 * required in both cases. So it is safe to just call the
	 * event handling functions here.
	 */
	dev_class_changed_callback(adapter->dev_id, length, param, adapter);

	if (adapter->initialized)
		g_dbus_emit_property_changed(dbus_conn, adapter->path,
						ADAPTER_INTERFACE, "UUIDs");
}

static int remove_uuid(struct btd_adapter *adapter, uuid_t *uuid)
{
	struct mgmt_cp_remove_uuid cp;
	uuid_t uuid128;
	uint128_t uint128;

	if (!is_supported_uuid(uuid)) {
		btd_warn(adapter->dev_id,
				"Ignoring unsupported UUID for removal");
		return 0;
	}

	uuid_to_uuid128(&uuid128, uuid);

	ntoh128((uint128_t *) uuid128.value.uuid128.data, &uint128);
	htob128(&uint128, (uint128_t *) cp.uuid);

	DBG("sending remove uuid command for index %u", adapter->dev_id);

	if (mgmt_send(adapter->mgmt, MGMT_OP_REMOVE_UUID,
				adapter->dev_id, sizeof(cp), &cp,
				remove_uuid_complete, adapter, NULL) > 0)
		return 0;

	btd_error(adapter->dev_id, "Failed to remove UUID for index %u",
							adapter->dev_id);

	return -EIO;
}

static void clear_uuids_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id, "Failed to clear UUIDs: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	/*
	 * The parameters are identical and also the task that is
	 * required in both cases. So it is safe to just call the
	 * event handling functions here.
	 */
	dev_class_changed_callback(adapter->dev_id, length, param, adapter);
}

static int clear_uuids(struct btd_adapter *adapter)
{
	struct mgmt_cp_remove_uuid cp;

	memset(&cp, 0, sizeof(cp));

	DBG("sending clear uuids command for index %u", adapter->dev_id);

	if (mgmt_send(adapter->mgmt, MGMT_OP_REMOVE_UUID,
				adapter->dev_id, sizeof(cp), &cp,
				clear_uuids_complete, adapter, NULL) > 0)
		return 0;

	btd_error(adapter->dev_id, "Failed to clear UUIDs for index %u",
							adapter->dev_id);

	return -EIO;
}

static uint8_t get_uuid_mask(uuid_t *uuid)
{
	if (uuid->type != SDP_UUID16)
		return 0;

	switch (uuid->value.uuid16) {
	case DIALUP_NET_SVCLASS_ID:
	case CIP_SVCLASS_ID:
		return 0x42;	/* Telephony & Networking */
	case IRMC_SYNC_SVCLASS_ID:
	case OBEX_OBJPUSH_SVCLASS_ID:
	case OBEX_FILETRANS_SVCLASS_ID:
	case IRMC_SYNC_CMD_SVCLASS_ID:
	case PBAP_PSE_SVCLASS_ID:
		return 0x10;	/* Object Transfer */
	case HEADSET_SVCLASS_ID:
	case HANDSFREE_SVCLASS_ID:
		return 0x20;	/* Audio */
	case CORDLESS_TELEPHONY_SVCLASS_ID:
	case INTERCOM_SVCLASS_ID:
	case FAX_SVCLASS_ID:
	case SAP_SVCLASS_ID:
	/*
	 * Setting the telephony bit for the handsfree audio gateway
	 * role is not required by the HFP specification, but the
	 * Nokia 616 carkit is just plain broken! It will refuse
	 * pairing without this bit set.
	 */
	case HANDSFREE_AGW_SVCLASS_ID:
		return 0x40;	/* Telephony */
	case AUDIO_SOURCE_SVCLASS_ID:
	case VIDEO_SOURCE_SVCLASS_ID:
		return 0x08;	/* Capturing */
	case AUDIO_SINK_SVCLASS_ID:
	case VIDEO_SINK_SVCLASS_ID:
		return 0x04;	/* Rendering */
	case PANU_SVCLASS_ID:
	case NAP_SVCLASS_ID:
	case GN_SVCLASS_ID:
		return 0x02;	/* Networking */
	default:
		return 0;
	}
}

static int uuid_cmp(const void *a, const void *b)
{
	const sdp_record_t *rec = a;
	const uuid_t *uuid = b;

	return sdp_uuid_cmp(&rec->svclass, uuid);
}

static void adapter_service_insert(struct btd_adapter *adapter, sdp_record_t *rec)
{
	sdp_list_t *browse_list = NULL;
	uuid_t browse_uuid;
	gboolean new_uuid;

	DBG("%s", adapter->path);

	/* skip record without a browse group */
	if (sdp_get_browse_groups(rec, &browse_list) < 0) {
		DBG("skipping record without browse group");
		return;
	}

	sdp_uuid16_create(&browse_uuid, PUBLIC_BROWSE_GROUP);

	/* skip record without public browse group */
	if (!sdp_list_find(browse_list, &browse_uuid, sdp_uuid_cmp))
		goto done;

	if (sdp_list_find(adapter->services, &rec->svclass, uuid_cmp) == NULL)
		new_uuid = TRUE;
	else
		new_uuid = FALSE;

	adapter->services = sdp_list_insert_sorted(adapter->services, rec,
								record_sort);

	if (new_uuid) {
		uint8_t svc_hint = get_uuid_mask(&rec->svclass);
		add_uuid(adapter, &rec->svclass, svc_hint);
	}

done:
	sdp_list_free(browse_list, free);
}

int adapter_service_add(struct btd_adapter *adapter, sdp_record_t *rec)
{
	int ret;

	DBG("%s", adapter->path);

	ret = add_record_to_server(&adapter->bdaddr, rec);
	if (ret < 0)
		return ret;

	adapter_service_insert(adapter, rec);

	return 0;
}

void adapter_service_remove(struct btd_adapter *adapter, uint32_t handle)
{
	sdp_record_t *rec = sdp_record_find(handle);

	DBG("%s", adapter->path);

	if (!rec)
		return;

	adapter->services = sdp_list_remove(adapter->services, rec);

	if (sdp_list_find(adapter->services, &rec->svclass, uuid_cmp) == NULL)
		remove_uuid(adapter, &rec->svclass);

	remove_record_from_server(rec->handle);
}

static struct btd_device *adapter_create_device(struct btd_adapter *adapter,
						const bdaddr_t *bdaddr,
						uint8_t bdaddr_type)
{
	struct btd_device *device;

	DBG("create Client");
	device = device_create(adapter, bdaddr, bdaddr_type);
	if (!device)
		return NULL;

	adapter->devices = g_slist_append(adapter->devices, device);

	return device;
}

static void service_auth_cancel(struct service_auth *auth)
{
	DBusError derr;

	if (auth->svc_id > 0)
		device_remove_svc_complete_callback(auth->device,
								auth->svc_id);

	dbus_error_init(&derr);
	dbus_set_error_const(&derr, ERROR_INTERFACE ".Canceled", NULL);

	auth->cb(&derr, auth->user_data);

	dbus_error_free(&derr);

	if (auth->agent != NULL) {
		agent_cancel(auth->agent);
		agent_unref(auth->agent);
	}

	g_free(auth);
}

void btd_adapter_remove_device(struct btd_adapter *adapter,
				struct btd_device *dev)
{
	GList *l;

	adapter->connect_list = g_slist_remove(adapter->connect_list, dev);

	adapter->devices = g_slist_remove(adapter->devices, dev);
	btd_adv_monitor_device_remove(adapter->adv_monitor_manager, dev);

	adapter->discovery_found = g_slist_remove(adapter->discovery_found,
									dev);

	DBG("");
	adapter->connections = g_slist_remove(adapter->connections, dev);

	if (adapter->connect_le == dev)
		adapter->connect_le = NULL;

	l = adapter->auths->head;
	while (l != NULL) {
		struct service_auth *auth = l->data;
		GList *next = g_list_next(l);

		if (auth->device != dev) {
			l = next;
			continue;
		}

		g_queue_delete_link(adapter->auths, l);
		l = next;

		service_auth_cancel(auth);
	}

	device_remove(dev, TRUE);
}

struct btd_device *btd_adapter_get_device(struct btd_adapter *adapter,
					const bdaddr_t *addr,
					uint8_t addr_type)
{
	struct btd_device *device;

	DBG("get Client");
	if (!adapter)
		return NULL;

	device = btd_adapter_find_device(adapter, addr, addr_type);
	if (device)
		return device;

	return adapter_create_device(adapter, addr, addr_type);
}

sdp_list_t *btd_adapter_get_services(struct btd_adapter *adapter)
{
	return adapter->services;
}

static void passive_scanning_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	const struct mgmt_cp_start_discovery *rp = param;

	DBG("status 0x%02x", status);

	if (length < sizeof(*rp)) {
		btd_error(adapter->dev_id,
			"Wrong size of start scanning return parameters");
		return;
	}

	if (status == MGMT_STATUS_SUCCESS) {
		adapter->discovery_type = rp->type;
		adapter->discovery_enable = 0x01;
	}
}

static gboolean passive_scanning_timeout(gpointer user_data)
{
	struct btd_adapter *adapter = user_data;
	struct mgmt_cp_start_discovery cp;

	adapter->passive_scan_timeout = 0;

	cp.type = SCAN_TYPE_LE;

	mgmt_send(adapter->mgmt, MGMT_OP_START_DISCOVERY,
				adapter->dev_id, sizeof(cp), &cp,
				passive_scanning_complete, adapter, NULL);

	return FALSE;
}

static void trigger_passive_scanning(struct btd_adapter *adapter)
{
	if (!(adapter->current_settings & MGMT_SETTING_LE))
		return;

	DBG("");

	if (adapter->passive_scan_timeout > 0) {
		g_source_remove(adapter->passive_scan_timeout);
		adapter->passive_scan_timeout = 0;
	}

	/*
	 * When the kernel background scanning is available, there is
	 * no need to start any discovery. The kernel will keep scanning
	 * as long as devices are in its auto-connection list.
	 */
	if (btd_has_kernel_features(KERNEL_CONN_CONTROL))
		return;

	/*
	 * If any client is running a discovery right now, then do not
	 * even try to start passive scanning.
	 *
	 * The discovery procedure is using interleaved scanning and
	 * thus will discover Low Energy devices as well.
	 */
	if (adapter->discovery_list)
		return;

	if (adapter->discovery_enable == 0x01)
		return;

	/*
	 * In case the discovery is suspended (for example for an ongoing
	 * pairing attempt), then also do not start passive scanning.
	 */
	if (adapter->discovery_suspended)
		return;

	/*
	 * If the list of connectable Low Energy devices is empty,
	 * then do not start passive scanning.
	 */
	if (!adapter->connect_list)
		return;

	adapter->passive_scan_timeout = g_timeout_add_seconds(CONN_SCAN_TIMEOUT,
					passive_scanning_timeout, adapter);
}

static void stop_passive_scanning_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	struct btd_device *dev;
	int err;

	DBG("status 0x%02x (%s)", status, mgmt_errstr(status));

	dev = adapter->connect_le;
	adapter->connect_le = NULL;

	/*
	 * When the kernel background scanning is available, there is
	 * no need to stop any discovery. The kernel will handle the
	 * auto-connection by itself.
	 */
	if (btd_has_kernel_features(KERNEL_CONN_CONTROL))
		return;

	/*
	 * MGMT_STATUS_REJECTED may be returned from kernel because the passive
	 * scan timer had expired in kernel and passive scan was disabled just
	 * around the time we called stop_passive_scanning().
	 */
	if (status != MGMT_STATUS_SUCCESS && status != MGMT_STATUS_REJECTED) {
		btd_error(adapter->dev_id, "Stopping passive scanning failed: %s",
							mgmt_errstr(status));
		return;
	}

	adapter->discovery_type = 0x00;
	adapter->discovery_enable = 0x00;

	if (!dev) {
		DBG("Device removed while stopping passive scanning");
		trigger_passive_scanning(adapter);
		return;
	}

	err = device_connect_le(dev);
	if (err < 0) {
		btd_error(adapter->dev_id, "LE auto connection failed: %s (%d)",
							strerror(-err), -err);
		trigger_passive_scanning(adapter);
	}
}

static void stop_passive_scanning(struct btd_adapter *adapter)
{
	struct mgmt_cp_stop_discovery cp;

	DBG("");

	/* If there are any normal discovery clients passive scanning
	 * wont be running */
	if (adapter->discovery_list)
		return;

	if (adapter->discovery_enable == 0x00)
		return;

	cp.type = adapter->discovery_type;

	mgmt_send(adapter->mgmt, MGMT_OP_STOP_DISCOVERY,
			adapter->dev_id, sizeof(cp), &cp,
			stop_passive_scanning_complete, adapter, NULL);
}

static void cancel_passive_scanning(struct btd_adapter *adapter)
{
	if (!(adapter->current_settings & MGMT_SETTING_LE))
		return;

	DBG("");

	if (adapter->passive_scan_timeout > 0) {
		g_source_remove(adapter->passive_scan_timeout);
		adapter->passive_scan_timeout = 0;
	}
}

static uint8_t get_scan_type(struct btd_adapter *adapter)
{
	uint8_t type;

	if (adapter->current_settings & MGMT_SETTING_BREDR)
		type = SCAN_TYPE_BREDR;
	else
		type = 0;

	if (adapter->current_settings & MGMT_SETTING_LE)
		type |= SCAN_TYPE_LE;

	return type;
}

static void free_discovery_filter(struct discovery_filter *discovery_filter)
{
	if (!discovery_filter)
		return;

	g_slist_free_full(discovery_filter->uuids, free);
	free(discovery_filter->pattern);
	g_free(discovery_filter);
}

static void invalidate_rssi_and_tx_power(gpointer a)
{
	struct btd_device *dev = a;

	device_set_rssi(dev, 0);
	device_set_tx_power(dev, 127);
}

static void discovery_cleanup(struct btd_adapter *adapter, int timeout)
{
	GSList *l, *next;

	adapter->discovery_type = 0x00;

	if (adapter->discovery_idle_timeout > 0) {
		g_source_remove(adapter->discovery_idle_timeout);
		adapter->discovery_idle_timeout = 0;
	}

	g_slist_free_full(adapter->discovery_found,
						invalidate_rssi_and_tx_power);
	adapter->discovery_found = NULL;

	if (!adapter->devices)
		return;

	for (l = adapter->devices; l != NULL; l = next) {
		struct btd_device *dev = l->data;

		next = g_slist_next(l);

		if (device_is_temporary(dev) && !device_is_connectable(dev))
			btd_adapter_remove_device(adapter, dev);
	}
}

static void discovery_free(void *user_data)
{
	struct discovery_client *client = user_data;
	struct btd_adapter *adapter = client->adapter;

	DBG("%p", client);

	if (client->watch)
		g_dbus_remove_watch(dbus_conn, client->watch);

	if (client->discovery_filter) {
		free_discovery_filter(client->discovery_filter);
		client->discovery_filter = NULL;
	}

	if (client->msg) {
		if (client == adapter->client) {
			g_dbus_send_message(dbus_conn,
						btd_error_busy(client->msg));
			adapter->client = NULL;
		}
		dbus_message_unref(client->msg);
	}

	g_free(client->owner);
	g_free(client);
}

static void discovery_remove(struct discovery_client *client)
{
	struct btd_adapter *adapter = client->adapter;

	DBG("owner %s", client->owner);

	adapter->set_filter_list = g_slist_remove(adapter->set_filter_list,
								client);

	adapter->discovery_list = g_slist_remove(adapter->discovery_list,
								client);

	if (adapter->client == client)
		adapter->client = NULL;

	if (client->watch && client->discovery_filter)
		adapter->set_filter_list = g_slist_prepend(
					adapter->set_filter_list, client);
	else
		discovery_free(client);

	/*
	 * If there are other client discoveries in progress, then leave
	 * it active. If not, then make sure to stop the restart timeout.
	 */
	if (adapter->discovery_list)
		return;

	discovery_cleanup(adapter, TEMP_DEV_TIMEOUT);
}

static void trigger_start_discovery(struct btd_adapter *adapter, guint delay);

static struct discovery_client *discovery_complete(struct btd_adapter *adapter,
						uint8_t status)
{
	struct discovery_client *client = adapter->client;
	DBusMessage *reply;

	if (!client)
		return NULL;

	adapter->client = NULL;

	if (!client->msg)
		return client;

	if (!status) {
		g_dbus_send_reply(dbus_conn, client->msg, DBUS_TYPE_INVALID);
	} else  {
		reply = btd_error_busy(client->msg);
		g_dbus_send_message(dbus_conn, reply);
	}

	dbus_message_unref(client->msg);
	client->msg = NULL;

	return client;
}

static void start_discovery_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	struct discovery_client *client;
	const struct mgmt_cp_start_discovery *rp = param;

	DBG("status 0x%02x", status);

	/* Is there are no clients the discovery must have been stopped while
	 * discovery command was pending.
	 */
	if (!adapter->discovery_list) {
		struct mgmt_cp_stop_discovery cp;

		if (status != MGMT_STATUS_SUCCESS)
			return;

		/* Stop discovering as there are no clients left */
		cp.type = rp->type;
		mgmt_send(adapter->mgmt, MGMT_OP_STOP_DISCOVERY,
					adapter->dev_id, sizeof(cp), &cp,
					NULL, NULL, NULL);
		return;
	}

	if (length < sizeof(*rp)) {
		btd_error(adapter->dev_id,
			"Wrong size of start discovery return parameters");
		discovery_complete(adapter, MGMT_STATUS_FAILED);
		return;
	}

	if (status == MGMT_STATUS_SUCCESS) {
		adapter->discovery_type = rp->type;
		adapter->discovery_enable = 0x01;

		if (adapter->current_discovery_filter)
			adapter->filtered_discovery = true;
		else
			adapter->filtered_discovery = false;

		discovery_complete(adapter, status);

		if (adapter->discovering)
			return;

		adapter->discovering = true;
		g_dbus_emit_property_changed(dbus_conn, adapter->path,
					ADAPTER_INTERFACE, "Discovering");
		return;
	}

	/* Reply with an error if the first discovery has failed */
	client = discovery_complete(adapter, status);
	if (client) {
		discovery_remove(client);
		return;
	}

	/*
	 * In case the restart of the discovery failed, then just trigger
	 * it for the next idle timeout again.
	 */
	trigger_start_discovery(adapter, IDLE_DISCOV_TIMEOUT * 2);
}

static gboolean start_discovery_timeout(gpointer user_data)
{
	struct btd_adapter *adapter = user_data;
	struct mgmt_cp_start_service_discovery *sd_cp;
	uint8_t new_type;

	DBG("");

	adapter->discovery_idle_timeout = 0;

	/* If we're doing filtered discovery, it must be quickly restarted */
	adapter->no_scan_restart_delay = !!adapter->current_discovery_filter;

	DBG("adapter->current_discovery_filter == %d",
	    !!adapter->current_discovery_filter);

	new_type = get_scan_type(adapter);

	if (adapter->discovery_enable == 0x01) {
		struct mgmt_cp_stop_discovery cp;

		/*
		 * If we're asked to start regular discovery, and there is an
		 * already running regular discovery and it has the same type,
		 * then just keep it.
		 */
		if (!adapter->current_discovery_filter &&
		    !adapter->filtered_discovery &&
		    adapter->discovery_type == new_type) {
			if (adapter->discovering)
				return FALSE;

			adapter->discovering = true;
			g_dbus_emit_property_changed(dbus_conn, adapter->path,
					ADAPTER_INTERFACE, "Discovering");
			return FALSE;
		}

		/*
		 * Otherwise the current discovery must be stopped. So
		 * queue up a stop discovery command.
		 *
		 * This can happen if a passive scanning for Low Energy
		 * devices is ongoing, or scan type is changed between
		 * regular and filtered, or filter was updated.
		 */
		cp.type = adapter->discovery_type;
		mgmt_send(adapter->mgmt, MGMT_OP_STOP_DISCOVERY,
					adapter->dev_id, sizeof(cp), &cp,
					NULL, NULL, NULL);

		/* Don't even bother to try to quickly start discovery
		 * just after stopping it, it would fail with status
		 * MGMT_BUSY. Instead discovering_callback will take
		 * care of that.
		 */
		return FALSE;

	}

	/* Regular discovery is required */
	if (!adapter->current_discovery_filter) {
		struct mgmt_cp_start_discovery cp;

		cp.type = new_type;
		mgmt_send(adapter->mgmt, MGMT_OP_START_DISCOVERY,
					adapter->dev_id, sizeof(cp), &cp,
					start_discovery_complete, adapter,
					NULL);

		return FALSE;
	}

	/* Filtered discovery is required */
	sd_cp = adapter->current_discovery_filter;

	DBG("sending MGMT_OP_START_SERVICE_DISCOVERY %d, %d, %d",
				sd_cp->rssi, sd_cp->type,
				btohs(sd_cp->uuid_count));

	mgmt_send(adapter->mgmt, MGMT_OP_START_SERVICE_DISCOVERY,
		  adapter->dev_id, sizeof(*sd_cp) +
		  btohs(sd_cp->uuid_count) * 16,
		  sd_cp, start_discovery_complete, adapter, NULL);

	return FALSE;
}

static void trigger_start_discovery(struct btd_adapter *adapter, guint delay)
{

	DBG("");

	cancel_passive_scanning(adapter);

	if (adapter->discovery_idle_timeout > 0) {
		g_source_remove(adapter->discovery_idle_timeout);
		adapter->discovery_idle_timeout = 0;
	}

	/*
	 * If the controller got powered down in between, then ensure
	 * that we do not keep trying to restart discovery.
	 *
	 * This is safe-guard and should actually never trigger.
	 */
	if (!btd_adapter_get_powered(adapter))
		return;

	adapter->discovery_idle_timeout = g_timeout_add_seconds(delay,
					start_discovery_timeout, adapter);
}

static void suspend_discovery_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	DBG("status 0x%02x", status);

	if (status == MGMT_STATUS_SUCCESS) {
		adapter->discovery_type = 0x00;
		adapter->discovery_enable = 0x00;
		return;
	}
}

static void suspend_discovery(struct btd_adapter *adapter)
{
	struct mgmt_cp_stop_discovery cp;

	DBG("");

	adapter->discovery_suspended = true;

	/*
	 * If there are no clients discovering right now, then there is
	 * also nothing to suspend.
	 */
	if (!adapter->discovery_list)
		return;

	/*
	 * In case of being inside the idle phase, make sure to remove
	 * the timeout to not trigger a restart.
	 *
	 * The restart will be triggered when the discovery is resumed.
	 */
	if (adapter->discovery_idle_timeout > 0) {
		g_source_remove(adapter->discovery_idle_timeout);
		adapter->discovery_idle_timeout = 0;
	}

	if (adapter->discovery_enable == 0x00)
		return;

	cp.type = adapter->discovery_type;

	mgmt_send(adapter->mgmt, MGMT_OP_STOP_DISCOVERY,
				adapter->dev_id, sizeof(cp), &cp,
				suspend_discovery_complete, adapter, NULL);
}

static void resume_discovery(struct btd_adapter *adapter)
{
	DBG("");

	adapter->discovery_suspended = false;

	/*
	 * If there are no clients discovering right now, then there is
	 * also nothing to resume.
	 */
	if (!adapter->discovery_list)
		return;

	/*
	 * Treat a suspended discovery session the same as extra long
	 * idle time for a normal discovery. So just trigger the default
	 * restart procedure.
	 */
	trigger_start_discovery(adapter, IDLE_DISCOV_TIMEOUT);
}

static void discovering_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_discovering *ev = param;
	struct btd_adapter *adapter = user_data;

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small discovering event");
		return;
	}

	DBG("hci%u type %u discovering %u method %d", adapter->dev_id, ev->type,
				ev->discovering, adapter->filtered_discovery);

	if (adapter->discovery_enable == ev->discovering)
		return;

	adapter->discovery_type = ev->type;
	adapter->discovery_enable = ev->discovering;

	/*
	 * Check for existing discoveries triggered by client applications
	 * and ignore all others.
	 *
	 * If there are no clients, then it is good idea to trigger a
	 * passive scanning attempt.
	 */
	if (!adapter->discovery_list) {
		if (!adapter->connect_le)
			trigger_passive_scanning(adapter);
		return;
	}

	if (adapter->discovery_suspended)
		return;

	switch (adapter->discovery_enable) {
	case 0x00:
		if (adapter->no_scan_restart_delay)
			trigger_start_discovery(adapter, 0);
		else
			trigger_start_discovery(adapter, IDLE_DISCOV_TIMEOUT);
		break;

	case 0x01:
		if (adapter->discovery_idle_timeout > 0) {
			g_source_remove(adapter->discovery_idle_timeout);
			adapter->discovery_idle_timeout = 0;
		}

		break;
	}
}

static bool set_discovery_discoverable(struct btd_adapter *adapter, bool enable)
{
	if (adapter->discovery_discoverable == enable)
		return true;

	/* Reset discoverable filter if already set */
	if (enable && (adapter->current_settings & MGMT_OP_SET_DISCOVERABLE))
		return true;

	adapter->discovery_discoverable = enable;

	return set_discoverable(adapter, enable, 0);
}

static void stop_discovery_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	struct discovery_client *client;

	DBG("status 0x%02x", status);

	client = discovery_complete(adapter, status);
	if (client)
		discovery_remove(client);

	if (status != MGMT_STATUS_SUCCESS)
		return;

	adapter->discovery_type = 0x00;
	adapter->discovery_enable = 0x00;
	adapter->filtered_discovery = false;
	adapter->no_scan_restart_delay = false;
	adapter->discovering = false;
	g_dbus_emit_property_changed(dbus_conn, adapter->path,
					ADAPTER_INTERFACE, "Discovering");

	trigger_passive_scanning(adapter);
}

static int compare_sender(gconstpointer a, gconstpointer b)
{
	const struct discovery_client *client = a;
	const char *sender = b;

	return g_strcmp0(client->owner, sender);
}

static gint g_strcmp(gconstpointer a, gconstpointer b)
{
	return strcmp(a, b);
}

static void extract_unique_uuids(gpointer data, gpointer user_data)
{
	char *uuid_str = data;
	GSList **uuids = user_data;

	if (!g_slist_find_custom(*uuids, uuid_str, g_strcmp))
		*uuids = g_slist_insert_sorted(*uuids, uuid_str, g_strcmp);
}

/*
 * This method merges all adapter filters into rssi, transport and uuids.
 * Returns 1 if there was no filtered scan, 0 otherwise.
 */
static int merge_discovery_filters(struct btd_adapter *adapter, int *rssi,
					uint8_t *transport, GSList **uuids)
{
	GSList *l;
	bool empty_uuid = false;
	bool has_regular_discovery = false;
	bool has_filtered_discovery = false;

	for (l = adapter->discovery_list; l != NULL; l = g_slist_next(l)) {
		struct discovery_client *client = l->data;
		struct discovery_filter *item = client->discovery_filter;

		if (!item) {
			has_regular_discovery = true;
			continue;
		}

		has_filtered_discovery = true;

		*transport |= item->type;

		/*
		 * Rule for merging rssi and pathloss into rssi field of kernel
		 * filter is as follow:
		 * - if there's any client without proximity filter, then do no
		 *   proximity filtering,
		 * - if all clients specified RSSI, then use lowest value,
		 * - if any client specified pathloss, then kernel filter should
		 *   do no proximity, as kernel can't compute pathloss. We'll do
		 *   filtering on our own.
		 */
		if (item->rssi == DISTANCE_VAL_INVALID)
			*rssi = HCI_RSSI_INVALID;
		else if (*rssi != HCI_RSSI_INVALID && *rssi >= item->rssi)
			*rssi = item->rssi;
		else if (item->pathloss != DISTANCE_VAL_INVALID)
			*rssi = HCI_RSSI_INVALID;

		if (!g_slist_length(item->uuids))
			empty_uuid = true;

		g_slist_foreach(item->uuids, extract_unique_uuids, uuids);
	}

	/* If no proximity filtering is set, disable it */
	if (*rssi == DISTANCE_VAL_INVALID)
		*rssi = HCI_RSSI_INVALID;

	/*
	 * Empty_uuid variable determines wether there was any filter with no
	 * uuids. In this case someone might be looking for all devices in
	 * certain proximity, and we need to have empty uuids in kernel filter.
	 */
	if (empty_uuid) {
		g_slist_free(*uuids);
		*uuids = NULL;
	}

	if (has_regular_discovery) {
		if (!has_filtered_discovery)
			return 1;

		/*
		 * It there is both regular and filtered scan running, then
		 * clear whole fitler to report all devices.
		 */
		*transport = get_scan_type(adapter);
		*rssi = HCI_RSSI_INVALID;
		g_slist_free(*uuids);
		*uuids = NULL;
	}

	return 0;
}

static void populate_mgmt_filter_uuids(uint8_t (*mgmt_uuids)[16], GSList *uuids)
{
	GSList *l;

	for (l = uuids; l != NULL; l = g_slist_next(l)) {
		bt_uuid_t uuid, u128;
		uint128_t uint128;

		bt_string_to_uuid(&uuid, l->data);
		bt_uuid_to_uuid128(&uuid, &u128);

		ntoh128((uint128_t *) u128.value.u128.data, &uint128);
		htob128(&uint128, (uint128_t *) mgmt_uuids);

		mgmt_uuids++;
	}
}

/*
 * This method merges all adapter filters into one that will be send to kernel.
 * cp_ptr is set to null when regular non-filtered discovery is needed,
 * otherwise it's pointing to filter. Returns 0 on succes, -1 on error
 */
static int discovery_filter_to_mgmt_cp(struct btd_adapter *adapter,
		       struct mgmt_cp_start_service_discovery **cp_ptr)
{
	GSList *uuids = NULL;
	struct mgmt_cp_start_service_discovery *cp;
	int rssi = DISTANCE_VAL_INVALID;
	int uuid_count;
	uint8_t discovery_type = 0;

	DBG("");

	if (merge_discovery_filters(adapter, &rssi, &discovery_type, &uuids)) {
		/* There are only regular scans, run just regular scan. */
		*cp_ptr = NULL;
		return 0;
	}

	uuid_count = g_slist_length(uuids);

	cp = g_try_malloc(sizeof(*cp) + 16*uuid_count);
	*cp_ptr = cp;
	if (!cp) {
		g_slist_free(uuids);
		return -1;
	}

	cp->type = discovery_type;
	cp->rssi = rssi;
	cp->uuid_count = htobs(uuid_count);
	populate_mgmt_filter_uuids(cp->uuids, uuids);

	g_slist_free(uuids);
	return 0;
}

static bool filters_equal(struct mgmt_cp_start_service_discovery *a,
		   struct mgmt_cp_start_service_discovery *b) {
	if (!a && !b)
		return true;

	if ((!a && b) || (a && !b))
		return false;

	if (a->type != b->type)
		return false;

	if (a->rssi != b->rssi)
		return false;

	/*
	 * When we create mgmt_cp_start_service_discovery structure inside
	 * discovery_filter_to_mgmt_cp, we always keep uuids sorted, and
	 * unique, so we're safe to compare uuid_count, and uuids like that.
	 */
	if (a->uuid_count != b->uuid_count)
		return false;

	if (memcmp(a->uuids, b->uuids, 16 * a->uuid_count) != 0)
		return false;

	return true;
}

static int update_discovery_filter(struct btd_adapter *adapter)
{
	struct mgmt_cp_start_service_discovery *sd_cp;
	GSList *l;


	DBG("");

	if (discovery_filter_to_mgmt_cp(adapter, &sd_cp)) {
		btd_error(adapter->dev_id,
				"discovery_filter_to_mgmt_cp returned error");
		return -ENOMEM;
	}

	for (l = adapter->discovery_list; l; l = g_slist_next(l)) {
		struct discovery_client *client = l->data;

		if (!client->discovery_filter)
			continue;

		if (client->discovery_filter->discoverable)
			break;
	}

	set_discovery_discoverable(adapter, l ? true : false);

	/*
	 * If filters are equal, then don't update scan, except for when
	 * starting discovery.
	 */
	if (filters_equal(adapter->current_discovery_filter, sd_cp) &&
	    adapter->discovering != 0) {
		DBG("filters were equal, deciding to not restart the scan.");
		g_free(sd_cp);
		return 0;
	}

	g_free(adapter->current_discovery_filter);
	adapter->current_discovery_filter = sd_cp;

	trigger_start_discovery(adapter, 0);

	return -EINPROGRESS;
}

static int discovery_stop(struct discovery_client *client)
{
	struct btd_adapter *adapter = client->adapter;
	struct mgmt_cp_stop_discovery cp;

	/* Check if there are more client discovering */
	if (g_slist_next(adapter->discovery_list)) {
		discovery_remove(client);
		update_discovery_filter(adapter);
		return 0;
	}

	if (adapter->discovery_discoverable)
		set_discovery_discoverable(adapter, false);

	/*
	 * In the idle phase of a discovery, there is no need to stop it
	 * and so it is enough to send out the signal and just return.
	 */
	if (adapter->discovery_enable == 0x00) {
		discovery_remove(client);
		adapter->discovering = false;
		g_dbus_emit_property_changed(dbus_conn, adapter->path,
					ADAPTER_INTERFACE, "Discovering");

		trigger_passive_scanning(adapter);

		return 0;
	}

	cp.type = adapter->discovery_type;
	adapter->client = client;

	mgmt_send(adapter->mgmt, MGMT_OP_STOP_DISCOVERY,
			adapter->dev_id, sizeof(cp), &cp,
			stop_discovery_complete, adapter, NULL);

	return -EINPROGRESS;
}

static void discovery_disconnect(DBusConnection *conn, void *user_data)
{
	struct discovery_client *client = user_data;

	DBG("owner %s", client->owner);

	client->watch = 0;

	discovery_stop(client);
}

/*
 * Returns true if client was already discovering, false otherwise. *client
 * will point to discovering client, or client that have pre-set his filter.
 */
static bool get_discovery_client(struct btd_adapter *adapter, const char *owner,
				struct discovery_client **client)
{
	GSList *list = g_slist_find_custom(adapter->discovery_list, owner,
								compare_sender);
	if (list) {
		*client = list->data;
		return true;
	}

	list = g_slist_find_custom(adapter->set_filter_list, owner,
								compare_sender);
	if (list) {
		*client = list->data;
		return false;
	}

	*client = NULL;
	return false;
}

static DBusMessage *start_discovery(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	const char *sender = dbus_message_get_sender(msg);
	struct discovery_client *client;
	bool is_discovering;
	int err;

	DBG("sender %s", sender);

	if (!btd_adapter_get_powered(adapter))
		return btd_error_not_ready(msg);

	is_discovering = get_discovery_client(adapter, sender, &client);

	/*
	 * Every client can only start one discovery, if the client
	 * already started a discovery then return an error.
	 */
	if (is_discovering)
		return btd_error_busy(msg);

	/*
	 * If there was pre-set filter, just reconnect it to discovery_list,
	 * and trigger scan.
	 */
	if (client) {
		if (client->msg)
			return btd_error_busy(msg);

		adapter->set_filter_list = g_slist_remove(
					     adapter->set_filter_list, client);
		adapter->discovery_list = g_slist_prepend(
					      adapter->discovery_list, client);

		goto done;
	}

	client = g_new0(struct discovery_client, 1);

	client->adapter = adapter;
	client->owner = g_strdup(sender);
	client->discovery_filter = NULL;
	client->watch = g_dbus_add_disconnect_watch(dbus_conn, sender,
						discovery_disconnect, client,
						NULL);
	adapter->discovery_list = g_slist_prepend(adapter->discovery_list,
								client);

done:
	/*
	 * Just trigger the discovery here. In case an already running
	 * discovery in idle phase exists, it will be restarted right
	 * away.
	 */
	err = update_discovery_filter(adapter);
	if (!err)
		return dbus_message_new_method_return(msg);

	/* If the discovery has to be started wait it complete to reply */
	if (err == -EINPROGRESS) {
		client->msg = dbus_message_ref(msg);
		adapter->client = client;
		return NULL;
	}

	return btd_error_failed(msg, strerror(-err));
}

static bool parse_uuids(DBusMessageIter *value, struct discovery_filter *filter)
{
	DBusMessageIter arriter;

	if (dbus_message_iter_get_arg_type(value) != DBUS_TYPE_ARRAY)
		return false;

	dbus_message_iter_recurse(value, &arriter);
	while (dbus_message_iter_get_arg_type(&arriter) != DBUS_TYPE_INVALID) {
		bt_uuid_t uuid, u128;
		char uuidstr[MAX_LEN_UUID_STR + 1];
		char *uuid_param;

		if (dbus_message_iter_get_arg_type(&arriter) !=
						DBUS_TYPE_STRING)
			return false;

		dbus_message_iter_get_basic(&arriter, &uuid_param);

		if (bt_string_to_uuid(&uuid, uuid_param))
			return false;

		bt_uuid_to_uuid128(&uuid, &u128);
		bt_uuid_to_string(&u128, uuidstr, sizeof(uuidstr));

		filter->uuids = g_slist_prepend(filter->uuids, strdup(uuidstr));

		dbus_message_iter_next(&arriter);
	}

	return true;
}

static bool parse_rssi(DBusMessageIter *value, struct discovery_filter *filter)
{
	if (dbus_message_iter_get_arg_type(value) != DBUS_TYPE_INT16)
		return false;

	dbus_message_iter_get_basic(value, &filter->rssi);
	/* -127 <= RSSI <= +20 (spec V4.2 [Vol 2, Part E] 7.7.65.2) */
	if (filter->rssi > 20 || filter->rssi < -127)
		return false;

	return true;
}

static bool parse_pathloss(DBusMessageIter *value,
				struct discovery_filter *filter)
{
	if (dbus_message_iter_get_arg_type(value) != DBUS_TYPE_UINT16)
		return false;

	dbus_message_iter_get_basic(value, &filter->pathloss);
	/* pathloss filter must be smaller that PATHLOSS_MAX */
	if (filter->pathloss > PATHLOSS_MAX)
		return false;

	return true;
}

static bool parse_transport(DBusMessageIter *value, 
					struct discovery_filter *filter)
{
	char *transport_str;

	if (dbus_message_iter_get_arg_type(value) != DBUS_TYPE_STRING)
		return false;

	dbus_message_iter_get_basic(value, &transport_str);

	if (!strcmp(transport_str, "bredr"))
		filter->type = SCAN_TYPE_BREDR;
	else if (!strcmp(transport_str, "le"))
		filter->type = SCAN_TYPE_LE;
	else if (strcmp(transport_str, "auto"))
		return false;

	return true;
}

static bool parse_duplicate_data(DBusMessageIter *value,
					struct discovery_filter *filter)
{
	if (dbus_message_iter_get_arg_type(value) != DBUS_TYPE_BOOLEAN)
		return false;

	dbus_message_iter_get_basic(value, &filter->duplicate);

	return true;
}

static bool parse_discoverable(DBusMessageIter *value,
					struct discovery_filter *filter)
{
	if (dbus_message_iter_get_arg_type(value) != DBUS_TYPE_BOOLEAN)
		return false;

	dbus_message_iter_get_basic(value, &filter->discoverable);

	return true;
}

static bool parse_pattern(DBusMessageIter *value,
					struct discovery_filter *filter)
{
	const char *pattern;

	if (dbus_message_iter_get_arg_type(value) != DBUS_TYPE_STRING)
		return false;

	dbus_message_iter_get_basic(value, &pattern);

	free(filter->pattern);
	filter->pattern = strdup(pattern);

	return true;
}

struct filter_parser {
	const char *name;
	bool (*func)(DBusMessageIter *iter, struct discovery_filter *filter);
} parsers[] = {
	{ "UUIDs", parse_uuids },
	{ "RSSI", parse_rssi },
	{ "Pathloss", parse_pathloss },
	{ "Transport", parse_transport },
	{ "DuplicateData", parse_duplicate_data },
	{ "Discoverable", parse_discoverable },
	{ "Pattern", parse_pattern },
	{ }
};

static bool parse_discovery_filter_entry(char *key, DBusMessageIter *value,
						struct discovery_filter *filter)
{
	struct filter_parser *parser;

	for (parser = parsers; parser && parser->name; parser++) {
		if (!strcmp(parser->name, key))
			return parser->func(value, filter);
	}

	DBG("Unknown key parameter: %s!\n", key);
	return false;
}

/*
 * This method is responsible for parsing parameters to SetDiscoveryFilter. If
 * filter in msg was empty, sets *filter to NULL. If whole parsing was
 * successful, sets *filter to proper value.
 * Returns false on any error, and true on success.
 */
static bool parse_discovery_filter_dict(struct btd_adapter *adapter,
					struct discovery_filter **filter,
					DBusMessage *msg)
{
	DBusMessageIter iter, subiter, dictiter, variantiter;
	bool is_empty = true;

	*filter = g_try_malloc(sizeof(**filter));
	if (!*filter)
		return false;

	(*filter)->uuids = NULL;
	(*filter)->pathloss = DISTANCE_VAL_INVALID;
	(*filter)->rssi = DISTANCE_VAL_INVALID;
	(*filter)->type = get_scan_type(adapter);
	(*filter)->duplicate = false;
	(*filter)->discoverable = false;
	(*filter)->pattern = NULL;

	dbus_message_iter_init(msg, &iter);
	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
	    dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_DICT_ENTRY)
		goto invalid_args;

	dbus_message_iter_recurse(&iter, &subiter);
	do {
		int type = dbus_message_iter_get_arg_type(&subiter);
		char *key;

		if (type == DBUS_TYPE_INVALID)
			break;

		is_empty = false;
		dbus_message_iter_recurse(&subiter, &dictiter);

		dbus_message_iter_get_basic(&dictiter, &key);
		if (!dbus_message_iter_next(&dictiter))
			goto invalid_args;

		if (dbus_message_iter_get_arg_type(&dictiter) !=
							     DBUS_TYPE_VARIANT)
			goto invalid_args;

		dbus_message_iter_recurse(&dictiter, &variantiter);

		if (!parse_discovery_filter_entry(key, &variantiter, *filter))
			goto invalid_args;

		dbus_message_iter_next(&subiter);
	} while (true);

	if (is_empty) {
		g_free(*filter);
		*filter = NULL;
		return true;
	}

	/* only pathlos or rssi can be set, never both */
	if ((*filter)->pathloss != DISTANCE_VAL_INVALID &&
	    (*filter)->rssi != DISTANCE_VAL_INVALID)
		goto invalid_args;

	DBG("filtered discovery params: transport: %d rssi: %d pathloss: %d "
		" duplicate data: %s discoverable %s pattern %s",
		(*filter)->type, (*filter)->rssi, (*filter)->pathloss,
		(*filter)->duplicate ? "true" : "false",
		(*filter)->discoverable ? "true" : "false",
		(*filter)->pattern);

	return true;

invalid_args:
	g_slist_free_full((*filter)->uuids, g_free);
	g_free(*filter);
	*filter = NULL;
	return false;
}

static DBusMessage *set_discovery_filter(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	struct discovery_client *client;
	struct discovery_filter *discovery_filter;
	const char *sender = dbus_message_get_sender(msg);
	bool is_discovering;

	DBG("sender %s", sender);

	if (!btd_adapter_get_powered(adapter))
		return btd_error_not_ready(msg);

	if (MGMT_VERSION(mgmt_version, mgmt_revision) < MGMT_VERSION(1, 8))
		return btd_error_not_supported(msg);

	/* parse parameters */
	if (!parse_discovery_filter_dict(adapter, &discovery_filter, msg))
		return btd_error_invalid_args(msg);

	is_discovering = get_discovery_client(adapter, sender, &client);

	if (client) {
		free_discovery_filter(client->discovery_filter);
		client->discovery_filter = discovery_filter;

		if (is_discovering)
			update_discovery_filter(adapter);

		if (discovery_filter || is_discovering)
			return dbus_message_new_method_return(msg);

		/* Removing pre-set filter */
		adapter->set_filter_list = g_slist_remove(
					      adapter->set_filter_list,
					      client);
		discovery_free(client);
		DBG("successfully cleared pre-set filter");
	} else if (discovery_filter) {
		/* Client pre-setting his filter for first time */
		client = g_new0(struct discovery_client, 1);
		client->adapter = adapter;
		client->owner = g_strdup(sender);
		client->discovery_filter = discovery_filter;
		client->watch = g_dbus_add_disconnect_watch(dbus_conn, sender,
						discovery_disconnect, client,
						NULL);
		adapter->set_filter_list = g_slist_prepend(
					     adapter->set_filter_list, client);

		DBG("successfully pre-set filter");
	}

	return dbus_message_new_method_return(msg);
}

static DBusMessage *stop_discovery(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	const char *sender = dbus_message_get_sender(msg);
	struct discovery_client *client;
	GSList *list;
	int err;

	DBG("sender %s", sender);

	if (!btd_adapter_get_powered(adapter))
		return btd_error_not_ready(msg);

	list = g_slist_find_custom(adapter->discovery_list, sender,
						compare_sender);
	if (!list)
		return btd_error_failed(msg, "No discovery started");

	client = list->data;

	if (client->msg)
		return btd_error_busy(msg);

	err = discovery_stop(client);
	switch (err) {
	case 0:
		return dbus_message_new_method_return(msg);
	case -EINPROGRESS:
		client->msg = dbus_message_ref(msg);
		adapter->client = client;
		return NULL;
	default:
		return btd_error_failed(msg, strerror(-err));
	}
}

static gboolean property_get_address(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	char addr[18];
	const char *str = addr;

	ba2str(&adapter->bdaddr, addr);

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &str);

	return TRUE;
}

static gboolean property_get_address_type(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	const char *str;

	if ((adapter->current_settings & MGMT_SETTING_LE) &&
				(adapter->bdaddr_type == BDADDR_LE_RANDOM))
		str = "random";
	else
		str = "public";

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &str);

	return TRUE;
}

static gboolean property_get_name(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	const char *str = adapter->system_name ? : "";

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &str);

	return TRUE;
}

static gboolean property_get_alias(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	const char *str;

	if (adapter->current_alias)
		str = adapter->current_alias;
	else if (adapter->stored_alias)
		str = adapter->stored_alias;
	else
		str = adapter->system_name ? : "";

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &str);

	return TRUE;
}

static void property_set_alias(const GDBusPropertyTable *property,
				DBusMessageIter *iter,
				GDBusPendingPropertySet id, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	const char *name;
	int ret;

	dbus_message_iter_get_basic(iter, &name);

	if (g_str_equal(name, "")  == TRUE) {
		if (adapter->stored_alias == NULL) {
			/* no alias set, nothing to restore */
			g_dbus_pending_property_success(id);
			return;
		}

		/* restore to system name */
		ret = set_name(adapter, adapter->system_name);
	} else {
		if (g_strcmp0(adapter->stored_alias, name) == 0) {
			/* alias already set, nothing to do */
			g_dbus_pending_property_success(id);
			return;
		}

		/* set to alias */
		ret = set_name(adapter, name);
	}

	if (ret >= 0) {
		g_free(adapter->stored_alias);

		if (g_str_equal(name, "")  == TRUE)
			adapter->stored_alias = NULL;
		else
			adapter->stored_alias = g_strdup(name);

		store_adapter_info(adapter);

		g_dbus_pending_property_success(id);
		return;
	}

	if (ret == -EINVAL)
		g_dbus_pending_property_error(id,
					ERROR_INTERFACE ".InvalidArguments",
					"Invalid arguments in method call");
	else
		g_dbus_pending_property_error(id, ERROR_INTERFACE ".Failed",
							strerror(-ret));
}

static gboolean property_get_class(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	dbus_uint32_t val = adapter->dev_class;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32, &val);

	return TRUE;
}

static gboolean property_get_mode(struct btd_adapter *adapter,
				uint32_t setting, DBusMessageIter *iter)
{
	dbus_bool_t enable;

	enable = (adapter->current_settings & setting) ? TRUE : FALSE;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &enable);

	return TRUE;
}

struct property_set_data {
	struct btd_adapter *adapter;
	GDBusPendingPropertySet id;
};

static void property_set_mode_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct property_set_data *data = user_data;
	struct btd_adapter *adapter = data->adapter;

	DBG("%s (0x%02x)", mgmt_errstr(status), status);

	if (status != MGMT_STATUS_SUCCESS) {
		const char *dbus_err;

		btd_error(adapter->dev_id, "Failed to set mode: %s (0x%02x)",
						mgmt_errstr(status), status);

		if (status == MGMT_STATUS_RFKILLED)
			dbus_err = ERROR_INTERFACE ".Blocked";
		else
			dbus_err = ERROR_INTERFACE ".Failed";

		g_dbus_pending_property_error(data->id, dbus_err,
							mgmt_errstr(status));
		return;
	}

	g_dbus_pending_property_success(data->id);

	/*
	 * The parameters are identical and also the task that is
	 * required in both cases. So it is safe to just call the
	 * event handling functions here.
	 */
	new_settings_callback(adapter->dev_id, length, param, adapter);
}

static void clear_discoverable(struct btd_adapter *adapter)
{
	if (!btd_has_kernel_features(KERNEL_CONN_CONTROL))
		return;

	if (!(adapter->current_settings & MGMT_SETTING_DISCOVERABLE))
		return;

	/* If no timeout is set do nothing as both connectable and discoverable
	 * flags are persistent on power toggle.
	 */
	if (!adapter->discoverable_timeout)
		return;

	/* If timeout was set kernel clears discoverable on its own when
	 * powering off controller. This would leave connectable flag set
	 * after power on.
	 *
	 * With kernel control clearing connectable clear also discoverable
	 * flag so we need to clear connectable.
	 */
	set_mode(adapter, MGMT_OP_SET_CONNECTABLE, 0x00);
}

static void property_set_mode(struct btd_adapter *adapter, uint32_t setting,
						DBusMessageIter *value,
						GDBusPendingPropertySet id)
{
	struct property_set_data *data;
	struct mgmt_cp_set_discoverable cp;
	void *param;
	dbus_bool_t enable, current_enable;
	uint16_t opcode, len;
	uint8_t mode;

	dbus_message_iter_get_basic(value, &enable);

	if (adapter->current_settings & setting)
		current_enable = TRUE;
	else
		current_enable = FALSE;

	if (enable == current_enable || adapter->pending_settings & setting) {
		g_dbus_pending_property_success(id);
		return;
	}

	mode = (enable == TRUE) ? 0x01 : 0x00;

	adapter->pending_settings |= setting;

	switch (setting) {
	case MGMT_SETTING_POWERED:
		opcode = MGMT_OP_SET_POWERED;
		param = &mode;
		len = sizeof(mode);

		if (!mode) {
			clear_discoverable(adapter);
			remove_temporary_devices(adapter);
		}

		break;
	case MGMT_SETTING_DISCOVERABLE:
		if (btd_has_kernel_features(KERNEL_CONN_CONTROL)) {
			if (mode) {
				set_mode(adapter, MGMT_OP_SET_CONNECTABLE,
									mode);
			} else {
				opcode = MGMT_OP_SET_CONNECTABLE;
				param = &mode;
				len = sizeof(mode);
				break;
			}
		}

		memset(&cp, 0, sizeof(cp));
		cp.val = mode;
		if (cp.val)
			cp.timeout = htobs(adapter->discoverable_timeout);

		opcode = MGMT_OP_SET_DISCOVERABLE;
		param = &cp;
		len = sizeof(cp);
		break;
	case MGMT_SETTING_BONDABLE:
		opcode = MGMT_OP_SET_BONDABLE;
		param = &mode;
		len = sizeof(mode);
		break;
	default:
		goto failed;
	}

	DBG("sending %s command for index %u", mgmt_opstr(opcode),
							adapter->dev_id);

	data = g_try_new0(struct property_set_data, 1);
	if (!data)
		goto failed;

	data->adapter = adapter;
	data->id = id;

	if (mgmt_send(adapter->mgmt, opcode, adapter->dev_id, len, param,
			property_set_mode_complete, data, g_free) > 0)
		return;

	g_free(data);

failed:
	btd_error(adapter->dev_id, "Failed to set mode for index %u",
							adapter->dev_id);

	g_dbus_pending_property_error(id, ERROR_INTERFACE ".Failed", NULL);
}

static gboolean property_get_powered(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	return property_get_mode(adapter, MGMT_SETTING_POWERED, iter);
}

static void property_set_powered(const GDBusPropertyTable *property,
				DBusMessageIter *iter,
				GDBusPendingPropertySet id, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	if (powering_down) {
		g_dbus_pending_property_error(id, ERROR_INTERFACE ".Failed",
							"Powering down");
		return;
	}

	property_set_mode(adapter, MGMT_SETTING_POWERED, iter, id);
}

static gboolean property_get_discoverable(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	return property_get_mode(adapter, MGMT_SETTING_DISCOVERABLE, iter);
}

static void property_set_discoverable(const GDBusPropertyTable *property,
				DBusMessageIter *iter,
				GDBusPendingPropertySet id, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	if (adapter->discoverable_timeout > 0 &&
			!btd_adapter_get_powered(adapter)) {
		g_dbus_pending_property_error(id, ERROR_INTERFACE ".Failed",
								"Not Powered");
		return;
	}

	/* Reset discovery_discoverable as Discoverable takes precedence */
	adapter->discovery_discoverable = false;

	property_set_mode(adapter, MGMT_SETTING_DISCOVERABLE, iter, id);
}

static gboolean property_get_discoverable_timeout(
					const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	dbus_uint32_t value = adapter->discoverable_timeout;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32, &value);

	return TRUE;
}

static void property_set_discoverable_timeout(
				const GDBusPropertyTable *property,
				DBusMessageIter *iter,
				GDBusPendingPropertySet id, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	bool enabled;
	dbus_uint32_t value;

	dbus_message_iter_get_basic(iter, &value);

	adapter->discoverable_timeout = value;

	g_dbus_pending_property_success(id);

	store_adapter_info(adapter);

	g_dbus_emit_property_changed(dbus_conn, adapter->path,
				ADAPTER_INTERFACE, "DiscoverableTimeout");

	if (adapter->pending_settings & MGMT_SETTING_DISCOVERABLE) {
		if (adapter->current_settings & MGMT_SETTING_DISCOVERABLE)
			enabled = false;
		else
			enabled = true;
	} else {
		if (adapter->current_settings & MGMT_SETTING_DISCOVERABLE)
			enabled = true;
		else
			enabled = false;
	}

	if (enabled)
		set_discoverable(adapter, 0x01, adapter->discoverable_timeout);
}

static gboolean property_get_pairable(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	return property_get_mode(adapter, MGMT_SETTING_BONDABLE, iter);
}

static void property_set_pairable(const GDBusPropertyTable *property,
				DBusMessageIter *iter,
				GDBusPendingPropertySet id, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	property_set_mode(adapter, MGMT_SETTING_BONDABLE, iter, id);
}

static gboolean property_get_pairable_timeout(
					const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	dbus_uint32_t value = adapter->pairable_timeout;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_UINT32, &value);

	return TRUE;
}

static void property_set_pairable_timeout(const GDBusPropertyTable *property,
				DBusMessageIter *iter,
				GDBusPendingPropertySet id, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	dbus_uint32_t value;

	dbus_message_iter_get_basic(iter, &value);

	adapter->pairable_timeout = value;

	g_dbus_pending_property_success(id);

	store_adapter_info(adapter);

	g_dbus_emit_property_changed(dbus_conn, adapter->path,
					ADAPTER_INTERFACE, "PairableTimeout");

	trigger_pairable_timeout(adapter);
}

static gboolean property_get_discovering(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	dbus_bool_t discovering = adapter->discovering;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_BOOLEAN, &discovering);

	return TRUE;
}

static void add_gatt_uuid(struct gatt_db_attribute *attrib, void *user_data)
{
	GHashTable *uuids = user_data;
	bt_uuid_t uuid, u128;
	char uuidstr[MAX_LEN_UUID_STR + 1];

	if (!gatt_db_service_get_active(attrib))
		return;

	if (!gatt_db_attribute_get_service_uuid(attrib, &uuid))
		return;

	bt_uuid_to_uuid128(&uuid, &u128);
	bt_uuid_to_string(&u128, uuidstr, sizeof(uuidstr));

	g_hash_table_add(uuids, strdup(uuidstr));
}

static void iter_append_uuid(gpointer key, gpointer value, gpointer user_data)
{
	DBusMessageIter *iter = user_data;
	const char *uuid = key;

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &uuid);
}

static gboolean property_get_uuids(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	DBusMessageIter entry;
	sdp_list_t *l;
	struct gatt_db *db;
	GHashTable *uuids;

	uuids = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);
	if (!uuids)
		return FALSE;

	/* SDP records */
	for (l = adapter->services; l != NULL; l = l->next) {
		sdp_record_t *rec = l->data;
		char *uuid;

		uuid = bt_uuid2string(&rec->svclass);
		if (uuid == NULL)
			continue;

		g_hash_table_add(uuids, uuid);
	}

	/* GATT services */
	db = btd_gatt_database_get_db(adapter->database);
	if (db)
		gatt_db_foreach_service(db, NULL, add_gatt_uuid, uuids);

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_STRING_AS_STRING, &entry);
	g_hash_table_foreach(uuids, iter_append_uuid, &entry);
	dbus_message_iter_close_container(iter, &entry);

	g_hash_table_destroy(uuids);

	return TRUE;
}

static gboolean property_exists_modalias(const GDBusPropertyTable *property,
							void *user_data)
{
	struct btd_adapter *adapter = user_data;

	return adapter->modalias ? TRUE : FALSE;
}

static gboolean property_get_modalias(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	const char *str = adapter->modalias ? : "";

	dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &str);

	return TRUE;
}

static gboolean property_get_roles(const GDBusPropertyTable *property,
					DBusMessageIter *iter, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	DBusMessageIter entry;

	dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_STRING_AS_STRING, &entry);

	if (adapter->supported_settings & MGMT_SETTING_LE) {
		const char *str = "central";
		dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &str);
	}

	if (adapter->supported_settings & MGMT_SETTING_ADVERTISING) {
		const char *str = "peripheral";
		dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &str);
	}

	if (adapter->le_simult_roles_supported) {
		const char *str = "central-peripheral";
		dbus_message_iter_append_basic(&entry, DBUS_TYPE_STRING, &str);
	}

	dbus_message_iter_close_container(iter, &entry);

	return TRUE;
}

static DBusMessage *remove_device(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	struct btd_device *device;
	const char *path;
	GSList *list;

	if (dbus_message_get_args(msg, NULL, DBUS_TYPE_OBJECT_PATH, &path,
						DBUS_TYPE_INVALID) == FALSE)
		return btd_error_invalid_args(msg);

	list = g_slist_find_custom(adapter->devices, path, device_path_cmp);
	if (!list)
		return btd_error_does_not_exist(msg);

	if (!btd_adapter_get_powered(adapter))
		return btd_error_not_ready(msg);

	device = list->data;

	btd_device_set_temporary(device, true);

	if (!btd_device_is_connected(device)) {
		btd_adapter_remove_device(adapter, device);
		return dbus_message_new_method_return(msg);
	}

	device_request_disconnect(device, msg);

	return NULL;
}

static DBusMessage *get_discovery_filters(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	DBusMessage *reply;
	DBusMessageIter iter, array;
	struct filter_parser *parser;

	reply = dbus_message_new_method_return(msg);

	dbus_message_iter_init_append(reply, &iter);

	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
					DBUS_TYPE_STRING_AS_STRING, &array);

	for (parser = parsers; parser && parser->name; parser++) {
		dbus_message_iter_append_basic(&array, DBUS_TYPE_STRING,
							&parser->name);
	}

	dbus_message_iter_close_container(&iter, &array);

	return reply;
}

struct device_connect_data {
	struct btd_adapter *adapter;
	bdaddr_t dst;
	uint8_t dst_type;
	DBusMessage *msg;
};

static void device_browse_cb(struct btd_device *dev, int err, void *user_data)
{
	DBG("err %d (%s)", err, strerror(-err));

	if (!err)
		btd_device_connect_services(dev, NULL);
}

static void device_connect_cb(GIOChannel *io, GError *gerr, gpointer user_data)
{
	struct device_connect_data *data = user_data;
	struct btd_adapter *adapter = data->adapter;
	struct btd_device *device;
	const char *path;

	DBG("%s", gerr ? gerr->message : "");

	if (gerr)
		goto failed;

	/* object might already exist due to mgmt socket event */
	device = btd_adapter_get_device(adapter, &data->dst, data->dst_type);
	if (!device)
		goto failed;

	path = device_get_path(device);

	g_dbus_send_reply(dbus_conn, data->msg, DBUS_TYPE_OBJECT_PATH, &path,
							DBUS_TYPE_INVALID);

	/* continue with service discovery and connection */
	btd_device_set_temporary(device, false);
	device_update_last_seen(device, data->dst_type);

	if (data->dst_type != BDADDR_BREDR){
		g_io_channel_set_close_on_unref(io, FALSE);
		device_attach_att(device, io);
	}

	device_discover_services(device);
	device_wait_for_svc_complete(device, device_browse_cb, NULL);

	g_io_channel_unref(io);
	dbus_message_unref(data->msg);
	free(data);
	return;

failed:
	g_dbus_send_error(dbus_conn, data->msg, "org.bluez.Failed", NULL);
	g_io_channel_unref(io);
	dbus_message_unref(data->msg);
	free(data);
}

static void device_connect(struct btd_adapter *adapter, const bdaddr_t *dst,
					uint8_t dst_type, DBusMessage *msg)
{
	struct device_connect_data *data;
	GIOChannel *io;

	data = new0(struct device_connect_data, 1);
	data->adapter = adapter;
	bacpy(&data->dst, dst);
	data->dst_type = dst_type;
	data->msg = dbus_message_ref(msg);

	if (dst_type == BDADDR_BREDR)
		io = bt_io_connect(device_connect_cb, data, NULL, NULL,
				BT_IO_OPT_SOURCE_BDADDR, &adapter->bdaddr,
				BT_IO_OPT_SOURCE_TYPE, BDADDR_BREDR,
				BT_IO_OPT_DEST_BDADDR, dst,
				BT_IO_OPT_DEST_TYPE, BDADDR_BREDR,
				BT_IO_OPT_PSM, SDP_PSM,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
				BT_IO_OPT_INVALID);
	else
		io = bt_io_connect(device_connect_cb, data, NULL, NULL,
				BT_IO_OPT_SOURCE_BDADDR, &adapter->bdaddr,
				BT_IO_OPT_SOURCE_TYPE, adapter->bdaddr_type,
				BT_IO_OPT_DEST_BDADDR, dst,
				BT_IO_OPT_DEST_TYPE, dst_type,
				BT_IO_OPT_CID, ATT_CID,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_LOW,
				BT_IO_OPT_INVALID);

	if (!io) {
		g_dbus_send_message(dbus_conn,
				btd_error_failed(msg, "Connect failed"));
		dbus_message_unref(data->msg);
		free(data);
	}
}

static DBusMessage *connect_device(DBusConnection *conn,
					DBusMessage *msg, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	DBusMessageIter iter, subiter, dictiter, value;
	uint8_t addr_type = BDADDR_BREDR;
	bdaddr_t addr = *BDADDR_ANY;

	DBG("sender %s", dbus_message_get_sender(msg));

	if (!btd_adapter_get_powered(adapter))
		return btd_error_not_ready(msg);

	dbus_message_iter_init(msg, &iter);
	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY ||
	    dbus_message_iter_get_element_type(&iter) != DBUS_TYPE_DICT_ENTRY)
		return btd_error_invalid_args(msg);

	dbus_message_iter_recurse(&iter, &subiter);
	while (true) {
		int type = dbus_message_iter_get_arg_type(&subiter);
		char *key;
		char *str;

		if (type == DBUS_TYPE_INVALID)
			break;

		dbus_message_iter_recurse(&subiter, &dictiter);

		dbus_message_iter_get_basic(&dictiter, &key);
		if (!dbus_message_iter_next(&dictiter))
			return btd_error_invalid_args(msg);

		if (dbus_message_iter_get_arg_type(&dictiter) !=
							DBUS_TYPE_VARIANT)
			return btd_error_invalid_args(msg);

		dbus_message_iter_recurse(&dictiter, &value);

		if (!strcmp(key, "Address")) {
			if (dbus_message_iter_get_arg_type(&value) !=
							DBUS_TYPE_STRING)
				return btd_error_invalid_args(msg);

			dbus_message_iter_get_basic(&value, &str);

			if (str2ba(str, &addr) < 0 )
				return btd_error_invalid_args(msg);
		} else if (!strcmp(key, "AddressType")) {
			if (dbus_message_iter_get_arg_type(&value) !=
							DBUS_TYPE_STRING)
				return btd_error_invalid_args(msg);

			dbus_message_iter_get_basic(&value, &str);


			if (!strcmp(str, "public"))
				addr_type = BDADDR_LE_PUBLIC;
			else if (!strcmp(str, "random"))
				addr_type = BDADDR_LE_RANDOM;
			else
				return btd_error_invalid_args(msg);
		} else {
			return btd_error_invalid_args(msg);
		}

		dbus_message_iter_next(&subiter);
	}

	if (!bacmp(&addr, BDADDR_ANY))
		return btd_error_invalid_args(msg);

	if (btd_adapter_find_device(adapter, &addr, addr_type))
		return btd_error_already_exists(msg);

	device_connect(adapter, &addr, addr_type, msg);
	return NULL;
}

static const GDBusMethodTable adapter_methods[] = {
	{ GDBUS_ASYNC_METHOD("StartDiscovery", NULL, NULL, start_discovery) },
	{ GDBUS_METHOD("SetDiscoveryFilter",
				GDBUS_ARGS({ "properties", "a{sv}" }), NULL,
				set_discovery_filter) },
	{ GDBUS_ASYNC_METHOD("StopDiscovery", NULL, NULL, stop_discovery) },
	{ GDBUS_ASYNC_METHOD("RemoveDevice",
			GDBUS_ARGS({ "device", "o" }), NULL, remove_device) },
	{ GDBUS_METHOD("GetDiscoveryFilters", NULL,
			GDBUS_ARGS({ "filters", "as" }),
			get_discovery_filters) },
	{ GDBUS_EXPERIMENTAL_ASYNC_METHOD("ConnectDevice",
				GDBUS_ARGS({ "properties", "a{sv}" }), NULL,
				connect_device) },
	{ }
};

static const GDBusPropertyTable adapter_properties[] = {
	{ "Address", "s", property_get_address },
	{ "AddressType", "s", property_get_address_type },
	{ "Name", "s", property_get_name },
	{ "Alias", "s", property_get_alias, property_set_alias },
	{ "Class", "u", property_get_class },
	{ "Powered", "b", property_get_powered, property_set_powered },
	{ "Discoverable", "b", property_get_discoverable,
					property_set_discoverable },
	{ "DiscoverableTimeout", "u", property_get_discoverable_timeout,
					property_set_discoverable_timeout },
	{ "Pairable", "b", property_get_pairable, property_set_pairable },
	{ "PairableTimeout", "u", property_get_pairable_timeout,
					property_set_pairable_timeout },
	{ "Discovering", "b", property_get_discovering },
	{ "UUIDs", "as", property_get_uuids },
	{ "Modalias", "s", property_get_modalias, NULL,
					property_exists_modalias },
	{ "Roles", "as", property_get_roles },
	{ }
};

static int str2buf(const char *str, uint8_t *buf, size_t blen)
{
	int i, dlen;

	if (str == NULL)
		return -EINVAL;

	memset(buf, 0, blen);

	dlen = MIN((strlen(str) / 2), blen);

	for (i = 0; i < dlen; i++)
		sscanf(str + (i * 2), "%02hhX", &buf[i]);

	return 0;
}

static bool is_blocked_key(uint8_t key_type, uint8_t *key_value)
{
	uint32_t i = 0;

	for (i = 0; i < ARRAY_SIZE(blocked_keys); ++i) {
		if (key_type == blocked_keys[i].type &&
				!memcmp(blocked_keys[i].val, key_value,
						sizeof(blocked_keys[i].val)))
			return true;
	}

	return false;
}

static struct link_key_info *get_key_info(GKeyFile *key_file, const char *peer)
{
	struct link_key_info *info = NULL;
	char *str;

	str = g_key_file_get_string(key_file, "LinkKey", "Key", NULL);
	if (!str || strlen(str) < 32)
		goto failed;

	info = g_new0(struct link_key_info, 1);

	str2ba(peer, &info->bdaddr);

	if (!strncmp(str, "0x", 2))
		str2buf(&str[2], info->key, sizeof(info->key));
	else
		str2buf(&str[0], info->key, sizeof(info->key));

	info->type = g_key_file_get_integer(key_file, "LinkKey", "Type", NULL);
	info->pin_len = g_key_file_get_integer(key_file, "LinkKey", "PINLength",
						NULL);

	info->is_blocked = is_blocked_key(HCI_BLOCKED_KEY_TYPE_LINKKEY,
								info->key);

failed:
	g_free(str);

	return info;
}

static struct smp_ltk_info *get_ltk(GKeyFile *key_file, const char *peer,
					uint8_t peer_type, const char *group)
{
	struct smp_ltk_info *ltk = NULL;
	GError *gerr = NULL;
	bool master;
	char *key;
	char *rand = NULL;

	key = g_key_file_get_string(key_file, group, "Key", NULL);
	if (!key || strlen(key) < 32)
		goto failed;

	rand = g_key_file_get_string(key_file, group, "Rand", NULL);
	if (!rand)
		goto failed;

	ltk = g_new0(struct smp_ltk_info, 1);

	/* Default to assuming a master key */
	ltk->master = true;

	str2ba(peer, &ltk->bdaddr);
	ltk->bdaddr_type = peer_type;

	/*
	 * Long term keys should respond to an identity address which can
	 * either be a public address or a random static address. Keys
	 * stored for resolvable random and unresolvable random addresses
	 * are ignored.
	 *
	 * This is an extra sanity check for older kernel versions or older
	 * daemons that might have been instructed to store long term keys
	 * for these temporary addresses.
	 */
	if (ltk->bdaddr_type == BDADDR_LE_RANDOM &&
					(ltk->bdaddr.b[5] & 0xc0) != 0xc0) {
		g_free(ltk);
		ltk = NULL;
		goto failed;
	}

	if (!strncmp(key, "0x", 2))
		str2buf(&key[2], ltk->val, sizeof(ltk->val));
	else
		str2buf(&key[0], ltk->val, sizeof(ltk->val));

	if (!strncmp(rand, "0x", 2)) {
		uint64_t rand_le;
		str2buf(&rand[2], (uint8_t *) &rand_le, sizeof(rand_le));
		ltk->rand = le64_to_cpu(rand_le);
	} else {
		sscanf(rand, "%" PRIu64, &ltk->rand);
	}

	ltk->authenticated = g_key_file_get_integer(key_file, group,
							"Authenticated", NULL);
	ltk->enc_size = g_key_file_get_integer(key_file, group, "EncSize",
									NULL);
	ltk->ediv = g_key_file_get_integer(key_file, group, "EDiv", NULL);

	master = g_key_file_get_boolean(key_file, group, "Master", &gerr);
	if (gerr)
		g_error_free(gerr);
	else
		ltk->master = master;

	ltk->is_blocked = is_blocked_key(HCI_BLOCKED_KEY_TYPE_LTK,
								ltk->val);

failed:
	g_free(key);
	g_free(rand);

	return ltk;
}

static struct smp_ltk_info *get_ltk_info(GKeyFile *key_file, const char *peer,
							uint8_t bdaddr_type)
{
	DBG("%s", peer);

	return get_ltk(key_file, peer, bdaddr_type, "LongTermKey");
}

static struct smp_ltk_info *get_slave_ltk_info(GKeyFile *key_file,
							const char *peer,
							uint8_t bdaddr_type)
{
	struct smp_ltk_info *ltk;

	DBG("%s", peer);

	ltk = get_ltk(key_file, peer, bdaddr_type, "SlaveLongTermKey");
	if (ltk)
		ltk->master = false;

	return ltk;
}

static struct irk_info *get_irk_info(GKeyFile *key_file, const char *peer,
							uint8_t bdaddr_type)
{
	struct irk_info *irk = NULL;
	char *str;

	str = g_key_file_get_string(key_file, "IdentityResolvingKey", "Key", NULL);
	if (!str || strlen(str) < 32)
		goto failed;

	irk = g_new0(struct irk_info, 1);

	str2ba(peer, &irk->bdaddr);
	irk->bdaddr_type = bdaddr_type;

	if (!strncmp(str, "0x", 2))
		str2buf(&str[2], irk->val, sizeof(irk->val));
	else
		str2buf(&str[0], irk->val, sizeof(irk->val));

	irk->is_blocked = is_blocked_key(HCI_BLOCKED_KEY_TYPE_LINKKEY,
								irk->val);

failed:
	g_free(str);

	return irk;
}

static struct conn_param *get_conn_param(GKeyFile *key_file, const char *peer,
							uint8_t bdaddr_type)
{
	struct conn_param *param;

	if (!g_key_file_has_group(key_file, "ConnectionParameters"))
		return NULL;

	param = g_new0(struct conn_param, 1);

	param->min_interval = g_key_file_get_integer(key_file,
							"ConnectionParameters",
							"MinInterval", NULL);
	param->max_interval = g_key_file_get_integer(key_file,
							"ConnectionParameters",
							"MaxInterval", NULL);
	param->latency = g_key_file_get_integer(key_file,
							"ConnectionParameters",
							"Latency", NULL);
	param->timeout = g_key_file_get_integer(key_file,
							"ConnectionParameters",
							"Timeout", NULL);
	str2ba(peer, &param->bdaddr);
	param->bdaddr_type = bdaddr_type;

	return param;
}

static int generate_and_write_irk(uint8_t *irk, GKeyFile *key_file,
							const char *filename)
{
	struct bt_crypto *crypto;
	char str_irk_out[33];
	gsize length = 0;
	char *str;
	int i;

	crypto = bt_crypto_new();
	if (!crypto) {
		error("Failed to open crypto");
		return -1;
	}

	if (!bt_crypto_random_bytes(crypto, irk, 16)) {
		error("Failed to generate IRK");
		bt_crypto_unref(crypto);
		return -1;
	}

	bt_crypto_unref(crypto);

	for (i = 0; i < 16; i++)
		sprintf(str_irk_out + (i * 2), "%02x", irk[i]);

	str_irk_out[32] = '\0';
	info("Generated IRK successfully");

	g_key_file_set_string(key_file, "General", "IdentityResolvingKey",
								str_irk_out);
	str = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, str, length, NULL);
	g_free(str);
	DBG("Generated IRK written to file");
	return 0;
}

static int load_irk(struct btd_adapter *adapter, uint8_t *irk)
{
	char filename[PATH_MAX];
	GKeyFile *key_file;
	char *str_irk;
	int ret;

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/identity",
					btd_adapter_get_storage_dir(adapter));

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	str_irk = g_key_file_get_string(key_file, "General",
						"IdentityResolvingKey", NULL);
	if (!str_irk) {
		info("No IRK stored");
		ret = generate_and_write_irk(irk, key_file, filename);
		g_key_file_free(key_file);
		return ret;
	}

	g_key_file_free(key_file);

	if (strlen(str_irk) != 32 || str2buf(str_irk, irk, 16)) {
		/* TODO re-create new IRK here? */
		error("Invalid IRK format, disabling privacy");
		g_free(str_irk);
		return -1;
	}

	g_free(str_irk);
	DBG("Successfully read IRK from file");
	return 0;
}

static void set_privacy_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id, "Failed to set privacy: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	DBG("Successfuly set privacy for index %u", adapter->dev_id);
}

static int set_privacy(struct btd_adapter *adapter, uint8_t privacy)
{
	struct mgmt_cp_set_privacy cp;

	memset(&cp, 0, sizeof(cp));

	if (privacy) {
		uint8_t irk[16];

		if (load_irk(adapter, irk) == 0) {
			cp.privacy = privacy;
			memcpy(cp.irk, irk, 16);
		}
	}

	DBG("sending set privacy command for index %u", adapter->dev_id);
	DBG("setting privacy mode 0x%02x for index %u", cp.privacy,
							adapter->dev_id);

	if (mgmt_send(adapter->mgmt, MGMT_OP_SET_PRIVACY,
				adapter->dev_id, sizeof(cp), &cp,
				set_privacy_complete, adapter, NULL) > 0)
		return 0;

	btd_error(adapter->dev_id, "Failed to set privacy for index %u",
							adapter->dev_id);

	return -1;
}

static void load_link_keys_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
			"Failed to load link keys for hci%u: %s (0x%02x)",
				adapter->dev_id, mgmt_errstr(status), status);
		return;
	}

	DBG("link keys loaded for hci%u", adapter->dev_id);
}

static void load_link_keys(struct btd_adapter *adapter, GSList *keys,
							bool debug_keys)
{
	struct mgmt_cp_load_link_keys *cp;
	struct mgmt_link_key_info *key;
	size_t key_count, cp_size;
	unsigned int id;
	GSList *l;

	/*
	 * If the controller does not support BR/EDR operation,
	 * there is no point in trying to load the link keys into
	 * the kernel.
	 *
	 * This is an optimization for Low Energy only controllers.
	 */
	if (!(adapter->supported_settings & MGMT_SETTING_BREDR))
		return;

	key_count = g_slist_length(keys);

	DBG("hci%u keys %zu debug_keys %d", adapter->dev_id, key_count,
								debug_keys);

	cp_size = sizeof(*cp) + (key_count * sizeof(*key));

	cp = g_try_malloc0(cp_size);
	if (cp == NULL) {
		btd_error(adapter->dev_id, "No memory for link keys for hci%u",
							adapter->dev_id);
		return;
	}

	/*
	 * Even if the list of stored keys is empty, it is important to
	 * load an empty list into the kernel. That way it is ensured
	 * that no old keys from a previous daemon are present.
	 *
	 * In addition it is also the only way to toggle the different
	 * behavior for debug keys.
	 */
	cp->debug_keys = debug_keys;
	cp->key_count = htobs(key_count);

	for (l = keys, key = cp->keys; l != NULL; l = g_slist_next(l), key++) {
		struct link_key_info *info = l->data;

		bacpy(&key->addr.bdaddr, &info->bdaddr);
		key->addr.type = BDADDR_BREDR;
		key->type = info->type;
		memcpy(key->val, info->key, 16);
		key->pin_len = info->pin_len;
	}

	id = mgmt_send(adapter->mgmt, MGMT_OP_LOAD_LINK_KEYS,
				adapter->dev_id, cp_size, cp,
				load_link_keys_complete, adapter, NULL);

	g_free(cp);

	if (id == 0)
		btd_error(adapter->dev_id, "Failed to load link keys for hci%u",
							adapter->dev_id);
}

static gboolean load_ltks_timeout(gpointer user_data)
{
	struct btd_adapter *adapter = user_data;

	btd_error(adapter->dev_id, "Loading LTKs timed out for hci%u",
							adapter->dev_id);

	adapter->load_ltks_timeout = 0;

	mgmt_cancel(adapter->mgmt, adapter->load_ltks_id);
	adapter->load_ltks_id = 0;

	return FALSE;
}

static void load_ltks_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
				"Failed to load LTKs for hci%u: %s (0x%02x)",
				adapter->dev_id, mgmt_errstr(status), status);
	}

	adapter->load_ltks_id = 0;

	g_source_remove(adapter->load_ltks_timeout);
	adapter->load_ltks_timeout = 0;

	DBG("LTKs loaded for hci%u", adapter->dev_id);
}

static void load_ltks(struct btd_adapter *adapter, GSList *keys)
{
	struct mgmt_cp_load_long_term_keys *cp;
	struct mgmt_ltk_info *key;
	size_t key_count, cp_size;
	GSList *l;

	/*
	 * If the controller does not support Low Energy operation,
	 * there is no point in trying to load the long term keys
	 * into the kernel.
	 *
	 * While there is no harm in loading keys into the kernel,
	 * this is an optimization to avoid a confusing warning
	 * message when the loading of the keys timed out due to
	 * a kernel bug (see comment below).
	 */
	if (!(adapter->supported_settings & MGMT_SETTING_LE))
		return;

	key_count = g_slist_length(keys);

	DBG("hci%u keys %zu", adapter->dev_id, key_count);

	cp_size = sizeof(*cp) + (key_count * sizeof(*key));

	cp = g_try_malloc0(cp_size);
	if (cp == NULL) {
		btd_error(adapter->dev_id, "No memory for LTKs for hci%u",
							adapter->dev_id);
		return;
	}

	/*
	 * Even if the list of stored keys is empty, it is important to
	 * load an empty list into the kernel. That way it is ensured
	 * that no old keys from a previous daemon are present.
	 */
	cp->key_count = htobs(key_count);

	for (l = keys, key = cp->keys; l != NULL; l = g_slist_next(l), key++) {
		struct smp_ltk_info *info = l->data;

		bacpy(&key->addr.bdaddr, &info->bdaddr);
		key->addr.type = info->bdaddr_type;
		memcpy(key->val, info->val, sizeof(info->val));
		key->rand = cpu_to_le64(info->rand);
		key->ediv = cpu_to_le16(info->ediv);
		key->type = info->authenticated;
		key->master = info->master;
		key->enc_size = info->enc_size;
	}

	adapter->load_ltks_id = mgmt_send(adapter->mgmt,
					MGMT_OP_LOAD_LONG_TERM_KEYS,
					adapter->dev_id, cp_size, cp,
					load_ltks_complete, adapter, NULL);

	g_free(cp);

	if (adapter->load_ltks_id == 0) {
		btd_error(adapter->dev_id, "Failed to load LTKs for hci%u",
							adapter->dev_id);
		return;
	}

	/*
	 * This timeout handling is needed since the kernel is stupid
	 * and forgets to send a command complete response. However in
	 * case of failures it does send a command status.
	 */
	adapter->load_ltks_timeout = g_timeout_add_seconds(2,
						load_ltks_timeout, adapter);
}

static void load_irks_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	if (status == MGMT_STATUS_UNKNOWN_COMMAND) {
		btd_info(adapter->dev_id,
			"Load IRKs failed: Kernel doesn't support LE Privacy");
		return;
	}

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
				"Failed to load IRKs for hci%u: %s (0x%02x)",
				adapter->dev_id, mgmt_errstr(status), status);
		return;
	}

	DBG("IRKs loaded for hci%u", adapter->dev_id);
}

static void load_irks(struct btd_adapter *adapter, GSList *irks)
{
	struct mgmt_cp_load_irks *cp;
	struct mgmt_irk_info *irk;
	size_t irk_count, cp_size;
	unsigned int id;
	GSList *l;

	/*
	 * If the controller does not support LE Privacy operation,
	 * there is no support for loading identity resolving keys
	 * into the kernel.
	 */
	if (!(adapter->supported_settings & MGMT_SETTING_PRIVACY))
		return;

	irk_count = g_slist_length(irks);

	DBG("hci%u irks %zu", adapter->dev_id, irk_count);

	cp_size = sizeof(*cp) + (irk_count * sizeof(*irk));

	cp = g_try_malloc0(cp_size);
	if (cp == NULL) {
		btd_error(adapter->dev_id, "No memory for IRKs for hci%u",
							adapter->dev_id);
		return;
	}

	/*
	 * Even if the list of stored keys is empty, it is important to
	 * load an empty list into the kernel. That way we tell the
	 * kernel that we are able to handle New IRK events.
	 */
	cp->irk_count = htobs(irk_count);

	for (l = irks, irk = cp->irks; l != NULL; l = g_slist_next(l), irk++) {
		struct irk_info *info = l->data;

		bacpy(&irk->addr.bdaddr, &info->bdaddr);
		irk->addr.type = info->bdaddr_type;
		memcpy(irk->val, info->val, sizeof(irk->val));
	}

	id = mgmt_send(adapter->mgmt, MGMT_OP_LOAD_IRKS, adapter->dev_id,
			cp_size, cp, load_irks_complete, adapter, NULL);

	g_free(cp);

	if (id == 0)
		btd_error(adapter->dev_id, "Failed to IRKs for hci%u",
							adapter->dev_id);
}

static void load_conn_params_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
			"hci%u Load Connection Parameters failed: %s (0x%02x)",
				adapter->dev_id, mgmt_errstr(status), status);
		return;
	}

	DBG("Connection Parameters loaded for hci%u", adapter->dev_id);
}

static void load_conn_params(struct btd_adapter *adapter, GSList *params)
{
	struct mgmt_cp_load_conn_param *cp;
	struct mgmt_conn_param *param;
	size_t param_count, cp_size;
	unsigned int id;
	GSList *l;

	/*
	 * If the controller does not support Low Energy operation,
	 * there is no point in trying to load the connection
	 * parameters into the kernel.
	 */
	if (!(adapter->supported_settings & MGMT_SETTING_LE))
		return;

	param_count = g_slist_length(params);

	DBG("hci%u conn params %zu", adapter->dev_id, param_count);

	cp_size = sizeof(*cp) + (param_count * sizeof(*param));

	cp = g_try_malloc0(cp_size);
	if (cp == NULL) {
		btd_error(adapter->dev_id,
			"Failed to allocate memory for connection parameters");
		return;
	}

	cp->param_count = htobs(param_count);

	for (l = params, param = cp->params; l; l = g_slist_next(l), param++) {
		struct conn_param *info = l->data;

		bacpy(&param->addr.bdaddr, &info->bdaddr);
		param->addr.type = info->bdaddr_type;
		param->min_interval = htobs(info->min_interval);
		param->max_interval = htobs(info->max_interval);
		param->latency = htobs(info->latency);
		param->timeout = htobs(info->timeout);
	}

	id = mgmt_send(adapter->mgmt, MGMT_OP_LOAD_CONN_PARAM, adapter->dev_id,
			cp_size, cp, load_conn_params_complete, adapter, NULL);

	g_free(cp);

	if (id == 0)
		btd_error(adapter->dev_id, "Load connection parameters failed");
}

static uint8_t get_le_addr_type(GKeyFile *keyfile)
{
	uint8_t addr_type;
	char *type;

	type = g_key_file_get_string(keyfile, "General", "AddressType", NULL);
	if (!type)
		return BDADDR_LE_PUBLIC;

	if (g_str_equal(type, "public"))
		addr_type = BDADDR_LE_PUBLIC;
	else if (g_str_equal(type, "static"))
		addr_type = BDADDR_LE_RANDOM;
	else
		addr_type = BDADDR_LE_PUBLIC;

	g_free(type);

	return addr_type;
}

static void probe_devices(void *user_data)
{
	struct btd_device *device = user_data;

	device_probe_profiles(device, btd_device_get_uuids(device));
}

static bool load_bredr_defaults(struct btd_adapter *adapter,
				struct mgmt_tlv_list *list,
				struct btd_br_defaults *defaults)
{
	if (btd_opts.mode == BT_MODE_LE)
		return true;

	if (defaults->page_scan_type != 0xFFFF) {
		if (!mgmt_tlv_add_fixed(list, 0x0000,
					&defaults->page_scan_type))
			return false;
	}

	if (defaults->page_scan_interval) {
		if (!mgmt_tlv_add_fixed(list, 0x0001,
					&defaults->page_scan_interval))
			return false;
	}

	if (defaults->page_scan_win) {
		if (!mgmt_tlv_add_fixed(list, 0x0002,
					&defaults->page_scan_win))
			return false;
	}

	if (defaults->scan_type != 0xFFFF) {
		if (!mgmt_tlv_add_fixed(list, 0x0003,
					&defaults->scan_type))
			return false;
	}

	if (defaults->scan_interval) {
		if (!mgmt_tlv_add_fixed(list, 0x0004,
					&defaults->scan_interval))
			return false;
	}

	if (defaults->scan_win) {
		if (!mgmt_tlv_add_fixed(list, 0x0005,
					&defaults->scan_win))
			return false;
	}

	if (defaults->link_supervision_timeout) {
		if (!mgmt_tlv_add_fixed(list, 0x0006,
					&defaults->link_supervision_timeout))
			return false;
	}

	if (defaults->page_timeout) {
		if (!mgmt_tlv_add_fixed(list, 0x0007,
					&defaults->page_timeout))
			return false;
	}

	if (defaults->min_sniff_interval) {
		if (!mgmt_tlv_add_fixed(list, 0x0008,
					&defaults->min_sniff_interval))
			return false;
	}

	if (defaults->max_sniff_interval) {
		if (!mgmt_tlv_add_fixed(list, 0x0009,
					&defaults->max_sniff_interval))
			return false;
	}

	return true;
}

static bool load_le_defaults(struct btd_adapter *adapter,
				struct mgmt_tlv_list *list,
				struct btd_le_defaults *defaults)
{
	if (btd_opts.mode == BT_MODE_BREDR)
		return true;

	if (defaults->min_adv_interval) {
		if (!mgmt_tlv_add_fixed(list, 0x000a,
					&defaults->min_adv_interval))
			return false;
	}

	if (defaults->max_adv_interval) {
		if (!mgmt_tlv_add_fixed(list, 0x000b,
					&defaults->max_adv_interval))
			return false;
	}

	if (defaults->adv_rotation_interval) {
		if (!mgmt_tlv_add_fixed(list, 0x000c,
					&defaults->adv_rotation_interval))
			return false;
	}

	if (defaults->scan_interval_autoconnect) {
		if (!mgmt_tlv_add_fixed(list, 0x000d,
					&defaults->scan_interval_autoconnect))
			return false;
	}

	if (defaults->scan_win_autoconnect) {
		if (!mgmt_tlv_add_fixed(list, 0x000e,
					&defaults->scan_win_autoconnect))
			return false;
	}

	if (defaults->scan_interval_suspend) {
		if (!mgmt_tlv_add_fixed(list, 0x000f,
					&defaults->scan_interval_suspend))
			return false;
	}

	if (defaults->scan_win_suspend) {
		if (!mgmt_tlv_add_fixed(list, 0x0010,
					&defaults->scan_win_suspend))
			return false;
	}

	if (defaults->scan_interval_discovery) {
		if (!mgmt_tlv_add_fixed(list, 0x0011,
					&defaults->scan_interval_discovery))
			return false;
	}

	if (defaults->scan_win_discovery) {
		if (!mgmt_tlv_add_fixed(list, 0x0012,
					&defaults->scan_win_discovery))
			return false;
	}

	if (defaults->scan_interval_adv_monitor) {
		if (!mgmt_tlv_add_fixed(list, 0x0013,
					&defaults->scan_interval_adv_monitor))
			return false;
	}

	if (defaults->scan_win_adv_monitor) {
		if (!mgmt_tlv_add_fixed(list, 0x0014,
					&defaults->scan_win_adv_monitor))
			return false;
	}

	if (defaults->scan_interval_connect) {
		if (!mgmt_tlv_add_fixed(list, 0x0015,
					&defaults->scan_interval_connect))
			return false;
	}

	if (defaults->scan_win_connect) {
		if (!mgmt_tlv_add_fixed(list, 0x0016,
					&defaults->scan_win_connect))
			return false;
	}

	if (defaults->min_conn_interval) {
		if (!mgmt_tlv_add_fixed(list, 0x0017,
					&defaults->min_conn_interval))
			return false;
	}

	if (defaults->max_conn_interval) {
		if (!mgmt_tlv_add_fixed(list, 0x0018,
					&defaults->max_conn_interval))
			return false;
	}

	if (defaults->conn_latency) {
		if (!mgmt_tlv_add_fixed(list, 0x0019,
					&defaults->conn_latency))
			return false;
	}

	if (defaults->conn_lsto) {
		if (!mgmt_tlv_add_fixed(list, 0x001a,
					&defaults->conn_lsto))
			return false;
	}

	if (defaults->autoconnect_timeout) {
		if (!mgmt_tlv_add_fixed(list, 0x001b,
					&defaults->autoconnect_timeout))
			return false;
	}

	if (defaults->advmon_allowlist_scan_duration) {
		if (!mgmt_tlv_add_fixed(list, 0x001d,
				&defaults->advmon_allowlist_scan_duration))
			return false;
	}

	if (defaults->advmon_no_filter_scan_duration) {
		if (!mgmt_tlv_add_fixed(list, 0x001e,
				&defaults->advmon_no_filter_scan_duration))
			return false;
	}

	if (defaults->enable_advmon_interleave_scan != 0xFF) {
		if (!mgmt_tlv_add_fixed(list, 0x001f,
				&defaults->enable_advmon_interleave_scan))
			return false;
	}

	return true;
}

static void load_defaults(struct btd_adapter *adapter)
{
	struct mgmt_tlv_list *list;
	unsigned int err = 0;

	if (!btd_opts.defaults.num_entries ||
	    !btd_has_kernel_features(KERNEL_SET_SYSTEM_CONFIG))
		return;

	list = mgmt_tlv_list_new();

	if (!load_bredr_defaults(adapter, list, &btd_opts.defaults.br))
		goto done;

	if (!load_le_defaults(adapter, list, &btd_opts.defaults.le))
		goto done;

	err = mgmt_send_tlv(adapter->mgmt, MGMT_OP_SET_DEF_SYSTEM_CONFIG,
			adapter->dev_id, list, NULL, NULL, NULL);

done:
	if (!err)
		btd_error(adapter->dev_id,
				"Failed to set default system config for hci%u",
				adapter->dev_id);

	mgmt_tlv_list_free(list);
}

static void load_devices(struct btd_adapter *adapter)
{
	char dirname[PATH_MAX];
	GSList *keys = NULL;
	GSList *ltks = NULL;
	GSList *irks = NULL;
	GSList *params = NULL;
	GSList *added_devices = NULL;
	DIR *dir;
	struct dirent *entry;

	snprintf(dirname, PATH_MAX, STORAGEDIR "/%s",
					btd_adapter_get_storage_dir(adapter));

	dir = opendir(dirname);
	if (!dir) {
		btd_error(adapter->dev_id,
				"Unable to open adapter storage directory: %s",
								dirname);
		return;
	}

	while ((entry = readdir(dir)) != NULL) {
		struct btd_device *device;
		char filename[PATH_MAX];
		GKeyFile *key_file;
		struct link_key_info *key_info;
		struct smp_ltk_info *ltk_info;
		struct smp_ltk_info *slave_ltk_info;
		GSList *list;
		struct irk_info *irk_info;
		struct conn_param *param;
		uint8_t bdaddr_type;

		if (entry->d_type == DT_UNKNOWN)
			entry->d_type = util_get_dt(dirname, entry->d_name);

		if (entry->d_type != DT_DIR || bachk(entry->d_name) < 0)
			continue;

		snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/info",
					btd_adapter_get_storage_dir(adapter),
					entry->d_name);

		key_file = g_key_file_new();
		g_key_file_load_from_file(key_file, filename, 0, NULL);

		key_info = get_key_info(key_file, entry->d_name);

		bdaddr_type = get_le_addr_type(key_file);

		ltk_info = get_ltk_info(key_file, entry->d_name, bdaddr_type);

		slave_ltk_info = get_slave_ltk_info(key_file, entry->d_name,
								bdaddr_type);

		irk_info = get_irk_info(key_file, entry->d_name, bdaddr_type);

		// If any key for the device is blocked, we discard all.
		if ((key_info && key_info->is_blocked) ||
				(ltk_info && ltk_info->is_blocked) ||
				(slave_ltk_info &&
					slave_ltk_info->is_blocked) ||
				(irk_info && irk_info->is_blocked)) {

			if (key_info) {
				g_free(key_info);
				key_info = NULL;
			}

			if (ltk_info) {
				g_free(ltk_info);
				ltk_info = NULL;
			}

			if (slave_ltk_info) {
				g_free(slave_ltk_info);
				slave_ltk_info = NULL;
			}

			if (irk_info) {
				g_free(irk_info);
				irk_info = NULL;
			}

			goto free;
		}

		if (key_info)
			keys = g_slist_append(keys, key_info);

		if (ltk_info)
			ltks = g_slist_append(ltks, ltk_info);

		if (slave_ltk_info)
			ltks = g_slist_append(ltks, slave_ltk_info);

		if (irk_info)
			irks = g_slist_append(irks, irk_info);

		param = get_conn_param(key_file, entry->d_name, bdaddr_type);
		if (param)
			params = g_slist_append(params, param);

		list = g_slist_find_custom(adapter->devices, entry->d_name,
							device_address_cmp);
		if (list) {
			device = list->data;
			goto device_exist;
		}

		device = device_create_from_storage(adapter, entry->d_name,
							key_file);
		if (!device)
			goto free;

		btd_device_set_temporary(device, false);
		adapter->devices = g_slist_append(adapter->devices, device);

		/* TODO: register services from pre-loaded list of primaries */

		added_devices = g_slist_append(added_devices, device);

device_exist:
		if (key_info) {
			device_set_paired(device, BDADDR_BREDR);
			device_set_bonded(device, BDADDR_BREDR);
		}

		if (ltk_info || slave_ltk_info) {
			device_set_paired(device, bdaddr_type);
			device_set_bonded(device, bdaddr_type);

			if (ltk_info)
				device_set_ltk_enc_size(device,
							ltk_info->enc_size);
			else if (slave_ltk_info)
				device_set_ltk_enc_size(device,
						slave_ltk_info->enc_size);
		}

free:
		g_key_file_free(key_file);
	}

	closedir(dir);

	load_link_keys(adapter, keys, btd_opts.debug_keys);
	g_slist_free_full(keys, g_free);

	load_ltks(adapter, ltks);
	g_slist_free_full(ltks, g_free);
	load_irks(adapter, irks);
	g_slist_free_full(irks, g_free);
	load_conn_params(adapter, params);
	g_slist_free_full(params, g_free);

	g_slist_free_full(added_devices, probe_devices);
}

int btd_adapter_block_address(struct btd_adapter *adapter,
				const bdaddr_t *bdaddr, uint8_t bdaddr_type)
{
	struct mgmt_cp_block_device cp;
	char addr[18];

	ba2str(bdaddr, addr);
	DBG("hci%u %s", adapter->dev_id, addr);

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, bdaddr);
	cp.addr.type = bdaddr_type;

	if (mgmt_send(adapter->mgmt, MGMT_OP_BLOCK_DEVICE,
				adapter->dev_id, sizeof(cp), &cp,
				NULL, NULL, NULL) > 0)
		return 0;

	return -EIO;
}

int btd_adapter_unblock_address(struct btd_adapter *adapter,
				const bdaddr_t *bdaddr, uint8_t bdaddr_type)
{
	struct mgmt_cp_unblock_device cp;
	char addr[18];

	ba2str(bdaddr, addr);
	DBG("hci%u %s", adapter->dev_id, addr);

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, bdaddr);
	cp.addr.type = bdaddr_type;

	if (mgmt_send(adapter->mgmt, MGMT_OP_UNBLOCK_DEVICE,
				adapter->dev_id, sizeof(cp), &cp,
				NULL, NULL, NULL) > 0)
		return 0;

	return -EIO;
}

static int clear_blocked(struct btd_adapter *adapter)
{
	return btd_adapter_unblock_address(adapter, BDADDR_ANY, 0);
}

static void probe_driver(struct btd_adapter *adapter, gpointer user_data)
{
	struct btd_adapter_driver *driver = user_data;
	int err;

	if (driver->probe == NULL)
		return;

	err = driver->probe(adapter);
	if (err < 0) {
		btd_error(adapter->dev_id, "%s: %s (%d)", driver->name,
							strerror(-err), -err);
		return;
	}

	adapter->drivers = g_slist_prepend(adapter->drivers, driver);
}

static void load_drivers(struct btd_adapter *adapter)
{
	GSList *l;

	for (l = adapter_drivers; l; l = l->next)
		probe_driver(adapter, l->data);
}

static void probe_profile(struct btd_profile *profile, void *data)
{
	struct btd_adapter *adapter = data;
	int err;

	if (profile->adapter_probe == NULL)
		return;

	err = profile->adapter_probe(profile, adapter);
	if (err < 0) {
		btd_error(adapter->dev_id, "%s: %s (%d)", profile->name,
							strerror(-err), -err);
		return;
	}

	adapter->profiles = g_slist_prepend(adapter->profiles, profile);
}

void adapter_add_profile(struct btd_adapter *adapter, gpointer p)
{
	struct btd_profile *profile = p;

	if (!adapter->initialized)
		return;

	probe_profile(profile, adapter);

	g_slist_foreach(adapter->devices, device_probe_profile, profile);
}

void adapter_remove_profile(struct btd_adapter *adapter, gpointer p)
{
	struct btd_profile *profile = p;

	if (!adapter->initialized)
		return;

	if (profile->device_remove)
		g_slist_foreach(adapter->devices, device_remove_profile, p);

	adapter->profiles = g_slist_remove(adapter->profiles, profile);

	if (profile->adapter_remove)
		profile->adapter_remove(profile, adapter);
}

static void adapter_add_connection(struct btd_adapter *adapter,
						struct btd_device *device,
						uint8_t bdaddr_type)
{
	device_add_connection(device, bdaddr_type);

	if (g_slist_find(adapter->connections, device)) {
		btd_error(adapter->dev_id,
				"Device is already marked as connected");
		return;
	}

	adapter->connections = g_slist_append(adapter->connections, device);
}

static void get_connections_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	const struct mgmt_rp_get_connections *rp = param;
	uint16_t i, conn_count;

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
				"Failed to get connections: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	if (length < sizeof(*rp)) {
		btd_error(adapter->dev_id,
				"Wrong size of get connections response");
		return;
	}

	conn_count = btohs(rp->conn_count);

	DBG("Connection count: %d", conn_count);

	if (conn_count * sizeof(struct mgmt_addr_info) +
						sizeof(*rp) != length) {
		btd_error(adapter->dev_id,
			"Incorrect packet size for get connections response");
		return;
	}

	for (i = 0; i < conn_count; i++) {
		const struct mgmt_addr_info *addr = &rp->addr[i];
		struct btd_device *device;
		char address[18];

		ba2str(&addr->bdaddr, address);
		DBG("Adding existing connection to %s", address);

		device = btd_adapter_get_device(adapter, &addr->bdaddr,
								addr->type);
		if (device)
			adapter_add_connection(adapter, device, addr->type);
	}
}

static void load_connections(struct btd_adapter *adapter)
{
	DBG("sending get connections command for index %u", adapter->dev_id);

	if (mgmt_send(adapter->mgmt, MGMT_OP_GET_CONNECTIONS,
				adapter->dev_id, 0, NULL,
				get_connections_complete, adapter, NULL) > 0)
		return;

	btd_error(adapter->dev_id, "Failed to get connections for index %u",
							adapter->dev_id);
}

bool btd_adapter_get_pairable(struct btd_adapter *adapter)
{
	if (adapter->current_settings & MGMT_SETTING_BONDABLE)
		return true;

	return false;
}

bool btd_adapter_get_powered(struct btd_adapter *adapter)
{
	if ((adapter->current_settings & MGMT_SETTING_POWERED) &&
			!(adapter->pending_settings & MGMT_SETTING_POWERED))
		return true;

	return false;
}

bool btd_adapter_get_connectable(struct btd_adapter *adapter)
{
	if (adapter->current_settings & MGMT_SETTING_CONNECTABLE)
		return true;

	return false;
}

bool btd_adapter_get_discoverable(struct btd_adapter *adapter)
{
	if (adapter->current_settings & MGMT_SETTING_DISCOVERABLE)
		return true;

	return false;
}

bool btd_adapter_get_bredr(struct btd_adapter *adapter)
{
	if (adapter->current_settings & MGMT_SETTING_BREDR)
		return true;

	return false;
}

struct btd_gatt_database *btd_adapter_get_database(struct btd_adapter *adapter)
{
	if (!adapter)
		return NULL;

	return adapter->database;
}

uint32_t btd_adapter_get_class(struct btd_adapter *adapter)
{
	return adapter->dev_class;
}

const char *btd_adapter_get_name(struct btd_adapter *adapter)
{
	if (adapter->stored_alias)
		return adapter->stored_alias;

	if (adapter->system_name)
		return adapter->system_name;

	return NULL;
}

int adapter_connect_list_add(struct btd_adapter *adapter,
					struct btd_device *device)
{
	/*
	 * If the adapter->connect_le device is getting added back to
	 * the connect list it probably means that the connect attempt
	 * failed and hence we should clear this pointer
	 */
	if (device == adapter->connect_le)
		adapter->connect_le = NULL;

	/*
	 * If kernel background scanning is supported then the
	 * adapter_auto_connect_add() function is used to maintain what to
	 * connect.
	 */
	if (btd_has_kernel_features(KERNEL_CONN_CONTROL))
		return 0;

	if (g_slist_find(adapter->connect_list, device)) {
		DBG("ignoring already added device %s",
						device_get_path(device));
		goto done;
	}

	if (!(adapter->supported_settings & MGMT_SETTING_LE)) {
		btd_error(adapter->dev_id,
			"Can't add %s to non-LE capable adapter connect list",
						device_get_path(device));
		return -ENOTSUP;
	}

	adapter->connect_list = g_slist_append(adapter->connect_list, device);
	DBG("%s added to %s's connect_list", device_get_path(device),
							adapter->system_name);

done:
	if (!btd_adapter_get_powered(adapter))
		return 0;

	trigger_passive_scanning(adapter);

	return 0;
}

void adapter_connect_list_remove(struct btd_adapter *adapter,
					struct btd_device *device)
{
	/*
	 * If the adapter->connect_le device is being removed from the
	 * connect list it means the connection was successful and hence
	 * the pointer should be cleared
	 */
	if (device == adapter->connect_le)
		adapter->connect_le = NULL;

	if (btd_has_kernel_features(KERNEL_CONN_CONTROL))
		return;

	if (!g_slist_find(adapter->connect_list, device)) {
		DBG("device %s is not on the list, ignoring",
						device_get_path(device));
		return;
	}

	adapter->connect_list = g_slist_remove(adapter->connect_list, device);
	DBG("%s removed from %s's connect_list", device_get_path(device),
							adapter->system_name);

	if (!adapter->connect_list) {
		stop_passive_scanning(adapter);
		return;
	}

	if (!btd_adapter_get_powered(adapter))
		return;

	trigger_passive_scanning(adapter);
}

static void add_whitelist_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_add_device *rp = param;
	struct btd_adapter *adapter = user_data;
	struct btd_device *dev;
	char addr[18];

	if (length < sizeof(*rp)) {
		btd_error(adapter->dev_id,
				"Too small Add Device complete event");
		return;
	}

	ba2str(&rp->addr.bdaddr, addr);

	dev = btd_adapter_find_device(adapter, &rp->addr.bdaddr,
							rp->addr.type);
	if (!dev) {
		btd_error(adapter->dev_id,
			"Add Device complete for unknown device %s", addr);
		return;
	}

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
					"Failed to add device %s: %s (0x%02x)",
					addr, mgmt_errstr(status), status);
		return;
	}

	DBG("%s added to kernel whitelist", addr);
}

void adapter_whitelist_add(struct btd_adapter *adapter, struct btd_device *dev)
{
	struct mgmt_cp_add_device cp;

	if (!btd_has_kernel_features(KERNEL_CONN_CONTROL))
		return;

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, device_get_address(dev));
	cp.addr.type = BDADDR_BREDR;
	cp.action = 0x01;

	mgmt_send(adapter->mgmt, MGMT_OP_ADD_DEVICE,
				adapter->dev_id, sizeof(cp), &cp,
				add_whitelist_complete, adapter, NULL);
}

static void remove_whitelist_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_remove_device *rp = param;
	char addr[18];

	if (length < sizeof(*rp)) {
		error("Too small Remove Device complete event");
		return;
	}

	ba2str(&rp->addr.bdaddr, addr);

	if (status != MGMT_STATUS_SUCCESS) {
		error("Failed to remove device %s: %s (0x%02x)",
					addr, mgmt_errstr(status), status);
		return;
	}

	DBG("%s removed from kernel whitelist", addr);
}

void adapter_whitelist_remove(struct btd_adapter *adapter, struct btd_device *dev)
{
	struct mgmt_cp_remove_device cp;

	if (!btd_has_kernel_features(KERNEL_CONN_CONTROL))
		return;

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, device_get_address(dev));
	cp.addr.type = BDADDR_BREDR;

	mgmt_send(adapter->mgmt, MGMT_OP_REMOVE_DEVICE,
				adapter->dev_id, sizeof(cp), &cp,
				remove_whitelist_complete, adapter, NULL);
}

static void add_device_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_add_device *rp = param;
	struct btd_adapter *adapter = user_data;
	struct btd_device *dev;
	char addr[18];

	if (length < sizeof(*rp)) {
		btd_error(adapter->dev_id,
				"Too small Add Device complete event");
		return;
	}

	ba2str(&rp->addr.bdaddr, addr);

	dev = btd_adapter_find_device(adapter, &rp->addr.bdaddr,
							rp->addr.type);
	if (!dev) {
		btd_error(adapter->dev_id,
			"Add Device complete for unknown device %s", addr);
		return;
	}

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
			"Failed to add device %s (%u): %s (0x%02x)",
			addr, rp->addr.type, mgmt_errstr(status), status);
		adapter->connect_list = g_slist_remove(adapter->connect_list,
									dev);
		return;
	}

	DBG("%s (%u) added to kernel connect list", addr, rp->addr.type);
}

void adapter_auto_connect_add(struct btd_adapter *adapter,
					struct btd_device *device)
{
	struct mgmt_cp_add_device cp;
	const bdaddr_t *bdaddr;
	uint8_t bdaddr_type;
	unsigned int id;

	if (!btd_has_kernel_features(KERNEL_CONN_CONTROL))
		return;

	if (g_slist_find(adapter->connect_list, device)) {
		DBG("ignoring already added device %s",
						device_get_path(device));
		return;
	}

	bdaddr = device_get_address(device);
	bdaddr_type = btd_device_get_bdaddr_type(device);

	if (bdaddr_type == BDADDR_BREDR) {
		DBG("auto-connection feature is not avaiable for BR/EDR");
		return;
	}

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, bdaddr);
	cp.addr.type = bdaddr_type;
	cp.action = 0x02;

	id = mgmt_send(adapter->mgmt, MGMT_OP_ADD_DEVICE,
			adapter->dev_id, sizeof(cp), &cp, add_device_complete,
			adapter, NULL);
	if (id == 0)
		return;

	adapter->connect_list = g_slist_append(adapter->connect_list, device);
}

static void set_device_wakeable_complete(uint8_t status, uint16_t length,
					 const void *param, void *user_data)
{
	const struct mgmt_rp_set_device_flags *rp = param;
	struct btd_adapter *adapter = user_data;
	struct btd_device *dev;
	char addr[18];

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id, "Set device flags return status: %s",
			  mgmt_errstr(status));
		return;
	}

	if (length < sizeof(*rp)) {
		btd_error(adapter->dev_id,
			  "Too small Set Device Flags complete event: %d",
			  length);
		return;
	}

	ba2str(&rp->addr.bdaddr, addr);

	dev = btd_adapter_find_device(adapter, &rp->addr.bdaddr, rp->addr.type);
	if (!dev) {
		btd_error(adapter->dev_id,
			  "Set Device Flags complete for unknown device %s",
			  addr);
		return;
	}

	device_set_wake_allowed_complete(dev);
}

void adapter_set_device_wakeable(struct btd_adapter *adapter,
				 struct btd_device *device, bool wakeable)
{
	struct mgmt_cp_set_device_flags cp;
	const bdaddr_t *bdaddr;
	uint8_t bdaddr_type;

	if (!btd_has_kernel_features(KERNEL_CONN_CONTROL))
		return;

	bdaddr = device_get_address(device);
	bdaddr_type = btd_device_get_bdaddr_type(device);

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, bdaddr);
	cp.addr.type = bdaddr_type;
	cp.current_flags = btd_device_get_current_flags(device);
	if (wakeable)
		cp.current_flags |= DEVICE_FLAG_REMOTE_WAKEUP;
	else
		cp.current_flags &= ~DEVICE_FLAG_REMOTE_WAKEUP;

	mgmt_send(adapter->mgmt, MGMT_OP_SET_DEVICE_FLAGS, adapter->dev_id,
		  sizeof(cp), &cp, set_device_wakeable_complete, adapter, NULL);
}

static void device_flags_changed_callback(uint16_t index, uint16_t length,
					  const void *param, void *user_data)
{
	const struct mgmt_ev_device_flags_changed *ev = param;
	struct btd_adapter *adapter = user_data;
	struct btd_device *dev;
	char addr[18];

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id,
			  "Too small Device Flags Changed event: %d",
			  length);
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);

	dev = btd_adapter_find_device(adapter, &ev->addr.bdaddr, ev->addr.type);
	if (!dev) {
		btd_error(adapter->dev_id,
			"Device Flags Changed for unknown device %s", addr);
		return;
	}

	btd_device_flags_changed(dev, ev->supported_flags, ev->current_flags);
}


static void remove_device_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_remove_device *rp = param;
	char addr[18];

	if (length < sizeof(*rp)) {
		error("Too small Remove Device complete event");
		return;
	}

	ba2str(&rp->addr.bdaddr, addr);

	if (status != MGMT_STATUS_SUCCESS) {
		error("Failed to remove device %s (%u): %s (0x%02x)",
			addr, rp->addr.type, mgmt_errstr(status), status);
		return;
	}

	DBG("%s (%u) removed from kernel connect list", addr, rp->addr.type);
}

void adapter_auto_connect_remove(struct btd_adapter *adapter,
					struct btd_device *device)
{
	struct mgmt_cp_remove_device cp;
	const bdaddr_t *bdaddr;
	uint8_t bdaddr_type;
	unsigned int id;

	if (!btd_has_kernel_features(KERNEL_CONN_CONTROL))
		return;

	if (!g_slist_find(adapter->connect_list, device)) {
		DBG("ignoring not added device %s", device_get_path(device));
		return;
	}

	bdaddr = device_get_address(device);
	bdaddr_type = btd_device_get_bdaddr_type(device);

	if (bdaddr_type == BDADDR_BREDR) {
		DBG("auto-connection feature is not avaiable for BR/EDR");
		return;
	}

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, bdaddr);
	cp.addr.type = bdaddr_type;

	id = mgmt_send(adapter->mgmt, MGMT_OP_REMOVE_DEVICE,
			adapter->dev_id, sizeof(cp), &cp,
			remove_device_complete, adapter, NULL);
	if (id == 0)
		return;

	adapter->connect_list = g_slist_remove(adapter->connect_list, device);
}

static void adapter_start(struct btd_adapter *adapter)
{
	g_dbus_emit_property_changed(dbus_conn, adapter->path,
						ADAPTER_INTERFACE, "Powered");

	DBG("adapter %s has been enabled", adapter->path);

	trigger_passive_scanning(adapter);
}

static void reply_pending_requests(struct btd_adapter *adapter)
{
	GSList *l;

	if (!adapter)
		return;

	/* pending bonding */
	for (l = adapter->devices; l; l = l->next) {
		struct btd_device *device = l->data;

		if (device_is_bonding(device, NULL))
			device_bonding_failed(device,
						HCI_OE_USER_ENDED_CONNECTION);
	}
}

static void remove_driver(gpointer data, gpointer user_data)
{
	struct btd_adapter_driver *driver = data;
	struct btd_adapter *adapter = user_data;

	if (driver->remove)
		driver->remove(adapter);
}

static void remove_profile(gpointer data, gpointer user_data)
{
	struct btd_profile *profile = data;
	struct btd_adapter *adapter = user_data;

	if (profile->adapter_remove)
		profile->adapter_remove(profile, adapter);
}

static void unload_drivers(struct btd_adapter *adapter)
{
	g_slist_foreach(adapter->drivers, remove_driver, adapter);
	g_slist_free(adapter->drivers);
	adapter->drivers = NULL;

	g_slist_foreach(adapter->profiles, remove_profile, adapter);
	g_slist_free(adapter->profiles);
	adapter->profiles = NULL;
}

static void free_service_auth(gpointer data, gpointer user_data)
{
	struct service_auth *auth = data;

	g_free(auth);
}

static void remove_discovery_list(struct btd_adapter *adapter)
{
	g_slist_free_full(adapter->set_filter_list, discovery_free);
	adapter->set_filter_list = NULL;

	g_slist_free_full(adapter->discovery_list, discovery_free);
	adapter->discovery_list = NULL;
}

static void adapter_free(gpointer user_data)
{
	struct btd_adapter *adapter = user_data;

	DBG("%p", adapter);

	/* Make sure the adapter's discovery list is cleaned up before freeing
	 * the adapter.
	 */
	remove_discovery_list(adapter);

	if (adapter->pairable_timeout_id > 0) {
		g_source_remove(adapter->pairable_timeout_id);
		adapter->pairable_timeout_id = 0;
	}

	if (adapter->passive_scan_timeout > 0) {
		g_source_remove(adapter->passive_scan_timeout);
		adapter->passive_scan_timeout = 0;
	}

	if (adapter->load_ltks_timeout > 0)
		g_source_remove(adapter->load_ltks_timeout);

	if (adapter->confirm_name_timeout > 0)
		g_source_remove(adapter->confirm_name_timeout);

	if (adapter->pair_device_timeout > 0)
		g_source_remove(adapter->pair_device_timeout);

	if (adapter->auth_idle_id)
		g_source_remove(adapter->auth_idle_id);

	g_queue_foreach(adapter->auths, free_service_auth, NULL);
	g_queue_free(adapter->auths);

	/*
	 * Unregister all handlers for this specific index since
	 * the adapter bound to them is no longer valid.
	 *
	 * This also avoids having multiple instances of the same
	 * handler in case indexes got removed and re-added.
	 */
	mgmt_unregister_index(adapter->mgmt, adapter->dev_id);

	/*
	 * Cancel all pending commands for this specific index
	 * since the adapter bound to them is no longer valid.
	 */
	mgmt_cancel_index(adapter->mgmt, adapter->dev_id);

	mgmt_unref(adapter->mgmt);

	sdp_list_free(adapter->services, NULL);

	g_slist_free(adapter->connections);

	g_free(adapter->path);
	g_free(adapter->name);
	g_free(adapter->short_name);
	g_free(adapter->system_name);
	g_free(adapter->stored_alias);
	g_free(adapter->current_alias);
	free(adapter->modalias);
	g_free(adapter);
}

struct btd_adapter *btd_adapter_ref(struct btd_adapter *adapter)
{
	__sync_fetch_and_add(&adapter->ref_count, 1);

	return adapter;
}

void btd_adapter_unref(struct btd_adapter *adapter)
{
	if (__sync_sub_and_fetch(&adapter->ref_count, 1))
		return;

	if (!adapter->path) {
		DBG("Freeing adapter %u", adapter->dev_id);

		adapter_free(adapter);
		return;
	}

	DBG("Freeing adapter %s", adapter->path);

	g_dbus_unregister_interface(dbus_conn, adapter->path,
						ADAPTER_INTERFACE);
}

static void convert_names_entry(char *key, char *value, void *user_data)
{
	char *address = user_data;
	char *str = key;
	char filename[PATH_MAX];
	GKeyFile *key_file;
	char *data;
	gsize length = 0;

	if (strchr(key, '#'))
		str[17] = '\0';

	if (bachk(str) != 0)
		return;

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/cache/%s", address, str);
	create_file(filename, S_IRUSR | S_IWUSR);

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);
	g_key_file_set_string(key_file, "General", "Name", value);

	data = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, data, length, NULL);
	g_free(data);

	g_key_file_free(key_file);
}

struct device_converter {
	char *address;
	void (*cb)(GKeyFile *key_file, void *value);
	gboolean force;
};

static void set_device_type(GKeyFile *key_file, char type)
{
	char *techno;
	char *addr_type = NULL;
	char *str;

	switch (type) {
	case BDADDR_BREDR:
		techno = "BR/EDR";
		break;
	case BDADDR_LE_PUBLIC:
		techno = "LE";
		addr_type = "public";
		break;
	case BDADDR_LE_RANDOM:
		techno = "LE";
		addr_type = "static";
		break;
	default:
		return;
	}

	str = g_key_file_get_string(key_file, "General",
					"SupportedTechnologies", NULL);
	if (!str)
		g_key_file_set_string(key_file, "General",
					"SupportedTechnologies", techno);
	else if (!strstr(str, techno))
		g_key_file_set_string(key_file, "General",
					"SupportedTechnologies", "BR/EDR;LE");

	g_free(str);

	if (addr_type)
		g_key_file_set_string(key_file, "General", "AddressType",
					addr_type);
}

static void convert_aliases_entry(GKeyFile *key_file, void *value)
{
	g_key_file_set_string(key_file, "General", "Alias", value);
}

static void convert_trusts_entry(GKeyFile *key_file, void *value)
{
	g_key_file_set_boolean(key_file, "General", "Trusted", TRUE);
}

static void convert_classes_entry(GKeyFile *key_file, void *value)
{
	g_key_file_set_string(key_file, "General", "Class", value);
}

static void convert_blocked_entry(GKeyFile *key_file, void *value)
{
	g_key_file_set_boolean(key_file, "General", "Blocked", TRUE);
}

static void convert_did_entry(GKeyFile *key_file, void *value)
{
	char *vendor_str, *product_str, *version_str;
	uint16_t val;

	vendor_str = strchr(value, ' ');
	if (!vendor_str)
		return;

	*(vendor_str++) = 0;

	if (g_str_equal(value, "FFFF"))
		return;

	product_str = strchr(vendor_str, ' ');
	if (!product_str)
		return;

	*(product_str++) = 0;

	version_str = strchr(product_str, ' ');
	if (!version_str)
		return;

	*(version_str++) = 0;

	val = (uint16_t) strtol(value, NULL, 16);
	g_key_file_set_integer(key_file, "DeviceID", "Source", val);

	val = (uint16_t) strtol(vendor_str, NULL, 16);
	g_key_file_set_integer(key_file, "DeviceID", "Vendor", val);

	val = (uint16_t) strtol(product_str, NULL, 16);
	g_key_file_set_integer(key_file, "DeviceID", "Product", val);

	val = (uint16_t) strtol(version_str, NULL, 16);
	g_key_file_set_integer(key_file, "DeviceID", "Version", val);
}

static void convert_linkkey_entry(GKeyFile *key_file, void *value)
{
	char *type_str, *length_str, *str;
	int val;

	type_str = strchr(value, ' ');
	if (!type_str)
		return;

	*(type_str++) = 0;

	length_str = strchr(type_str, ' ');
	if (!length_str)
		return;

	*(length_str++) = 0;

	str = g_strconcat("0x", value, NULL);
	g_key_file_set_string(key_file, "LinkKey", "Key", str);
	g_free(str);

	val = strtol(type_str, NULL, 16);
	g_key_file_set_integer(key_file, "LinkKey", "Type", val);

	val = strtol(length_str, NULL, 16);
	g_key_file_set_integer(key_file, "LinkKey", "PINLength", val);
}

static void convert_ltk_entry(GKeyFile *key_file, void *value)
{
	char *auth_str, *rand_str, *str;
	int i, ret;
	unsigned char auth, master, enc_size;
	unsigned short ediv;

	auth_str = strchr(value, ' ');
	if (!auth_str)
		return;

	*(auth_str++) = 0;

	for (i = 0, rand_str = auth_str; i < 4; i++) {
		rand_str = strchr(rand_str, ' ');
		if (!rand_str || rand_str[1] == '\0')
			return;

		rand_str++;
	}

	ret = sscanf(auth_str, " %hhd %hhd %hhd %hd", &auth, &master,
							&enc_size, &ediv);
	if (ret < 4)
		return;

	str = g_strconcat("0x", value, NULL);
	g_key_file_set_string(key_file, "LongTermKey", "Key", str);
	g_free(str);

	g_key_file_set_integer(key_file, "LongTermKey", "Authenticated", auth);
	g_key_file_set_integer(key_file, "LongTermKey", "Master", master);
	g_key_file_set_integer(key_file, "LongTermKey", "EncSize", enc_size);
	g_key_file_set_integer(key_file, "LongTermKey", "EDiv", ediv);

	str = g_strconcat("0x", rand_str, NULL);
	g_key_file_set_string(key_file, "LongTermKey", "Rand", str);
	g_free(str);
}

static void convert_profiles_entry(GKeyFile *key_file, void *value)
{
	g_strdelimit(value, " ", ';');
	g_key_file_set_string(key_file, "General", "Services", value);
}

static void convert_appearances_entry(GKeyFile *key_file, void *value)
{
	g_key_file_set_string(key_file, "General", "Appearance", value);
}

static void convert_entry(char *key, char *value, void *user_data)
{
	struct device_converter *converter = user_data;
	char type = BDADDR_BREDR;
	char filename[PATH_MAX];
	GKeyFile *key_file;
	char *data;
	gsize length = 0;

	if (strchr(key, '#')) {
		key[17] = '\0';
		type = key[18] - '0';
	}

	if (bachk(key) != 0)
		return;

	if (converter->force == FALSE) {
		struct stat st;
		int err;

		snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s",
				converter->address, key);

		err = stat(filename, &st);
		if (err || !S_ISDIR(st.st_mode))
			return;
	}

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/info",
			converter->address, key);

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	set_device_type(key_file, type);

	converter->cb(key_file, value);

	data = g_key_file_to_data(key_file, &length, NULL);
	if (length > 0) {
		create_file(filename, S_IRUSR | S_IWUSR);
		g_file_set_contents(filename, data, length, NULL);
	}

	g_free(data);

	g_key_file_free(key_file);
}

static void convert_file(char *file, char *address,
				void (*cb)(GKeyFile *key_file, void *value),
				gboolean force)
{
	char filename[PATH_MAX];
	struct device_converter converter;

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s", address, file);

	converter.address = address;
	converter.cb = cb;
	converter.force = force;

	textfile_foreach(filename, convert_entry, &converter);
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

static void store_attribute_uuid(GKeyFile *key_file, uint16_t start,
					uint16_t end, char *att_uuid,
					uuid_t uuid)
{
	char handle[6], uuid_str[33];
	int i;

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

	sprintf(handle, "%hu", start);
	g_key_file_set_string(key_file, handle, "UUID", att_uuid);
	g_key_file_set_string(key_file, handle, "Value", uuid_str);
	g_key_file_set_integer(key_file, handle, "EndGroupHandle", end);
}

static void store_sdp_record(char *local, char *peer, int handle, char *value)
{
	char filename[PATH_MAX];
	GKeyFile *key_file;
	char handle_str[11];
	char *data;
	gsize length = 0;

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/cache/%s", local, peer);

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	sprintf(handle_str, "0x%8.8X", handle);
	g_key_file_set_string(key_file, "ServiceRecords", handle_str, value);

	data = g_key_file_to_data(key_file, &length, NULL);
	if (length > 0) {
		create_file(filename, S_IRUSR | S_IWUSR);
		g_file_set_contents(filename, data, length, NULL);
	}

	g_free(data);

	g_key_file_free(key_file);
}

static void convert_sdp_entry(char *key, char *value, void *user_data)
{
	char *src_addr = user_data;
	char dst_addr[18];
	char type = BDADDR_BREDR;
	int handle, ret;
	char filename[PATH_MAX];
	GKeyFile *key_file;
	struct stat st;
	sdp_record_t *rec;
	uuid_t uuid;
	char *att_uuid, *prim_uuid;
	uint16_t start = 0, end = 0, psm = 0;
	int err;
	char *data;
	gsize length = 0;

	ret = sscanf(key, "%17s#%hhu#%08X", dst_addr, &type, &handle);
	if (ret < 3) {
		ret = sscanf(key, "%17s#%08X", dst_addr, &handle);
		if (ret < 2)
			return;
	}

	if (bachk(dst_addr) != 0)
		return;

	/* Check if the device directory has been created as records should
	 * only be converted for known devices */
	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s", src_addr, dst_addr);

	err = stat(filename, &st);
	if (err || !S_ISDIR(st.st_mode))
		return;

	/* store device records in cache */
	store_sdp_record(src_addr, dst_addr, handle, value);

	/* Retrieve device record and check if there is an
	 * attribute entry in it */
	sdp_uuid16_create(&uuid, ATT_UUID);
	att_uuid = bt_uuid2string(&uuid);

	sdp_uuid16_create(&uuid, GATT_PRIM_SVC_UUID);
	prim_uuid = bt_uuid2string(&uuid);

	rec = record_from_string(value);

	if (record_has_uuid(rec, att_uuid))
		goto failed;

	/* TODO: Do this through btd_gatt_database */
	if (!gatt_parse_record(rec, &uuid, &psm, &start, &end))
		goto failed;

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/attributes", src_addr,
								dst_addr);

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	store_attribute_uuid(key_file, start, end, prim_uuid, uuid);

	data = g_key_file_to_data(key_file, &length, NULL);
	if (length > 0) {
		create_file(filename, S_IRUSR | S_IWUSR);
		g_file_set_contents(filename, data, length, NULL);
	}

	g_free(data);
	g_key_file_free(key_file);

failed:
	sdp_record_free(rec);
	free(prim_uuid);
	free(att_uuid);
}

static void convert_primaries_entry(char *key, char *value, void *user_data)
{
	char *address = user_data;
	int device_type = -1;
	uuid_t uuid;
	char **services, **service, *prim_uuid;
	char filename[PATH_MAX];
	GKeyFile *key_file;
	int ret;
	uint16_t start, end;
	char uuid_str[MAX_LEN_UUID_STR + 1];
	char *data;
	gsize length = 0;

	if (strchr(key, '#')) {
		key[17] = '\0';
		device_type = key[18] - '0';
	}

	if (bachk(key) != 0)
		return;

	services = g_strsplit(value, " ", 0);
	if (services == NULL)
		return;

	sdp_uuid16_create(&uuid, GATT_PRIM_SVC_UUID);
	prim_uuid = bt_uuid2string(&uuid);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/attributes", address,
									key);
	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	for (service = services; *service; service++) {
		ret = sscanf(*service, "%04hX#%04hX#%s", &start, &end,
								uuid_str);
		if (ret < 3)
			continue;

		bt_string2uuid(&uuid, uuid_str);
		sdp_uuid128_to_uuid(&uuid);

		store_attribute_uuid(key_file, start, end, prim_uuid, uuid);
	}

	g_strfreev(services);

	data = g_key_file_to_data(key_file, &length, NULL);
	if (length == 0)
		goto end;

	create_file(filename, S_IRUSR | S_IWUSR);
	g_file_set_contents(filename, data, length, NULL);

	if (device_type < 0)
		goto end;

	g_free(data);
	g_key_file_free(key_file);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/info", address, key);

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);
	set_device_type(key_file, device_type);

	data = g_key_file_to_data(key_file, &length, NULL);
	if (length > 0) {
		create_file(filename, S_IRUSR | S_IWUSR);
		g_file_set_contents(filename, data, length, NULL);
	}

end:
	g_free(data);
	free(prim_uuid);
	g_key_file_free(key_file);
}

static void convert_ccc_entry(char *key, char *value, void *user_data)
{
	char *src_addr = user_data;
	char dst_addr[18];
	char type = BDADDR_BREDR;
	uint16_t handle;
	int ret, err;
	char filename[PATH_MAX];
	GKeyFile *key_file;
	struct stat st;
	char group[6];
	char *data;
	gsize length = 0;

	ret = sscanf(key, "%17s#%hhu#%04hX", dst_addr, &type, &handle);
	if (ret < 3)
		return;

	if (bachk(dst_addr) != 0)
		return;

	/* Check if the device directory has been created as records should
	 * only be converted for known devices */
	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s", src_addr, dst_addr);

	err = stat(filename, &st);
	if (err || !S_ISDIR(st.st_mode))
		return;

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/ccc", src_addr,
								dst_addr);
	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	sprintf(group, "%hu", handle);
	g_key_file_set_string(key_file, group, "Value", value);

	data = g_key_file_to_data(key_file, &length, NULL);
	if (length > 0) {
		create_file(filename, S_IRUSR | S_IWUSR);
		g_file_set_contents(filename, data, length, NULL);
	}

	g_free(data);
	g_key_file_free(key_file);
}

static void convert_gatt_entry(char *key, char *value, void *user_data)
{
	char *src_addr = user_data;
	char dst_addr[18];
	char type = BDADDR_BREDR;
	uint16_t handle;
	int ret, err;
	char filename[PATH_MAX];
	GKeyFile *key_file;
	struct stat st;
	char group[6];
	char *data;
	gsize length = 0;

	ret = sscanf(key, "%17s#%hhu#%04hX", dst_addr, &type, &handle);
	if (ret < 3)
		return;

	if (bachk(dst_addr) != 0)
		return;

	/* Check if the device directory has been created as records should
	 * only be converted for known devices */
	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s", src_addr, dst_addr);

	err = stat(filename, &st);
	if (err || !S_ISDIR(st.st_mode))
		return;

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/gatt", src_addr,
								dst_addr);
	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	sprintf(group, "%hu", handle);
	g_key_file_set_string(key_file, group, "Value", value);

	data = g_key_file_to_data(key_file, &length, NULL);
	if (length > 0) {
		create_file(filename, S_IRUSR | S_IWUSR);
		g_file_set_contents(filename, data, length, NULL);
	}

	g_free(data);
	g_key_file_free(key_file);
}

static void convert_proximity_entry(char *key, char *value, void *user_data)
{
	char *src_addr = user_data;
	char *alert;
	char filename[PATH_MAX];
	GKeyFile *key_file;
	struct stat st;
	int err;
	char *data;
	gsize length = 0;

	if (!strchr(key, '#'))
		return;

	key[17] = '\0';
	alert = &key[18];

	if (bachk(key) != 0)
		return;

	/* Check if the device directory has been created as records should
	 * only be converted for known devices */
	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s", src_addr, key);

	err = stat(filename, &st);
	if (err || !S_ISDIR(st.st_mode))
		return;

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/proximity", src_addr,
									key);
	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	g_key_file_set_string(key_file, alert, "Level", value);

	data = g_key_file_to_data(key_file, &length, NULL);
	if (length > 0) {
		create_file(filename, S_IRUSR | S_IWUSR);
		g_file_set_contents(filename, data, length, NULL);
	}

	g_free(data);
	g_key_file_free(key_file);
}

static void convert_device_storage(struct btd_adapter *adapter)
{
	char filename[PATH_MAX];
	char address[18];

	ba2str(&adapter->bdaddr, address);

	/* Convert device's name cache */
	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/names", address);
	textfile_foreach(filename, convert_names_entry, address);

	/* Convert aliases */
	convert_file("aliases", address, convert_aliases_entry, TRUE);

	/* Convert trusts */
	convert_file("trusts", address, convert_trusts_entry, TRUE);

	/* Convert blocked */
	convert_file("blocked", address, convert_blocked_entry, TRUE);

	/* Convert profiles */
	convert_file("profiles", address, convert_profiles_entry, TRUE);

	/* Convert primaries */
	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/primaries", address);
	textfile_foreach(filename, convert_primaries_entry, address);

	/* Convert linkkeys */
	convert_file("linkkeys", address, convert_linkkey_entry, TRUE);

	/* Convert longtermkeys */
	convert_file("longtermkeys", address, convert_ltk_entry, TRUE);

	/* Convert classes */
	convert_file("classes", address, convert_classes_entry, FALSE);

	/* Convert device ids */
	convert_file("did", address, convert_did_entry, FALSE);

	/* Convert sdp */
	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/sdp", address);
	textfile_foreach(filename, convert_sdp_entry, address);

	/* Convert ccc */
	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/ccc", address);
	textfile_foreach(filename, convert_ccc_entry, address);

	/* Convert appearances */
	convert_file("appearances", address, convert_appearances_entry, FALSE);

	/* Convert gatt */
	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/gatt", address);
	textfile_foreach(filename, convert_gatt_entry, address);

	/* Convert proximity */
	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/proximity", address);
	textfile_foreach(filename, convert_proximity_entry, address);
}

static void convert_config(struct btd_adapter *adapter, const char *filename,
							GKeyFile *key_file)
{
	char address[18];
	char str[MAX_NAME_LENGTH + 1];
	char config_path[PATH_MAX];
	int timeout;
	uint8_t mode;
	char *data;
	gsize length = 0;

	ba2str(&adapter->bdaddr, address);
	snprintf(config_path, PATH_MAX, STORAGEDIR "/%s/config", address);

	if (read_pairable_timeout(address, &timeout) == 0)
		g_key_file_set_integer(key_file, "General",
						"PairableTimeout", timeout);

	if (read_discoverable_timeout(address, &timeout) == 0)
		g_key_file_set_integer(key_file, "General",
						"DiscoverableTimeout", timeout);

	if (read_on_mode(address, str, sizeof(str)) == 0) {
		mode = get_mode(str);
		g_key_file_set_boolean(key_file, "General", "Discoverable",
					mode == MODE_DISCOVERABLE);
	}

	if (read_local_name(&adapter->bdaddr, str) == 0)
		g_key_file_set_string(key_file, "General", "Alias", str);

	create_file(filename, S_IRUSR | S_IWUSR);

	data = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, data, length, NULL);
	g_free(data);
}

static void fix_storage(struct btd_adapter *adapter)
{
	char filename[PATH_MAX];
	char address[18];
	char *converted;

	ba2str(&adapter->bdaddr, address);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/config", address);
	converted = textfile_get(filename, "converted");
	if (!converted)
		return;

	free(converted);

	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/names", address);
	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/aliases", address);
	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/trusts", address);
	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/blocked", address);
	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/profiles", address);
	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/primaries", address);
	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/linkkeys", address);
	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/longtermkeys", address);
	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/classes", address);
	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/did", address);
	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/sdp", address);
	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/ccc", address);
	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/appearances", address);
	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/gatt", address);
	textfile_del(filename, "converted");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/proximity", address);
	textfile_del(filename, "converted");
}

static void load_config(struct btd_adapter *adapter)
{
	GKeyFile *key_file;
	char filename[PATH_MAX];
	struct stat st;
	GError *gerr = NULL;

	key_file = g_key_file_new();

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/settings",
					btd_adapter_get_storage_dir(adapter));

	if (stat(filename, &st) < 0) {
		convert_config(adapter, filename, key_file);
		convert_device_storage(adapter);
	}

	g_key_file_load_from_file(key_file, filename, 0, NULL);

	/* Get alias */
	adapter->stored_alias = g_key_file_get_string(key_file, "General",
								"Alias", NULL);
	if (!adapter->stored_alias) {
		/* fallback */
		adapter->stored_alias = g_key_file_get_string(key_file,
						"General", "Name", NULL);
	}

	/* Get pairable timeout */
	adapter->pairable_timeout = g_key_file_get_integer(key_file, "General",
						"PairableTimeout", &gerr);
	if (gerr) {
		adapter->pairable_timeout = btd_opts.pairto;
		g_error_free(gerr);
		gerr = NULL;
	}

	/* Get discoverable mode */
	adapter->stored_discoverable = g_key_file_get_boolean(key_file,
					"General", "Discoverable", &gerr);
	if (gerr) {
		adapter->stored_discoverable = false;
		g_error_free(gerr);
		gerr = NULL;
	}

	/* Get discoverable timeout */
	adapter->discoverable_timeout = g_key_file_get_integer(key_file,
				"General", "DiscoverableTimeout", &gerr);
	if (gerr) {
		adapter->discoverable_timeout = btd_opts.discovto;
		g_error_free(gerr);
		gerr = NULL;
	}

	g_key_file_free(key_file);
}

static struct btd_adapter *btd_adapter_new(uint16_t index)
{
	struct btd_adapter *adapter;

	adapter = g_try_new0(struct btd_adapter, 1);
	if (!adapter)
		return NULL;

	adapter->dev_id = index;
	adapter->mgmt = mgmt_ref(mgmt_master);
	adapter->pincode_requested = false;

	/*
	 * Setup default configuration values. These are either adapter
	 * defaults or from a system wide configuration file.
	 *
	 * Some value might be overwritten later on by adapter specific
	 * configuration. This is to make sure that sane defaults are
	 * always present.
	 */
	adapter->system_name = g_strdup(btd_opts.name);
	adapter->major_class = (btd_opts.class & 0x001f00) >> 8;
	adapter->minor_class = (btd_opts.class & 0x0000fc) >> 2;
	adapter->modalias = bt_modalias(btd_opts.did_source,
						btd_opts.did_vendor,
						btd_opts.did_product,
						btd_opts.did_version);
	adapter->discoverable_timeout = btd_opts.discovto;
	adapter->pairable_timeout = btd_opts.pairto;

	DBG("System name: %s", adapter->system_name);
	DBG("Major class: %u", adapter->major_class);
	DBG("Minor class: %u", adapter->minor_class);
	DBG("Modalias: %s", adapter->modalias);
	DBG("Discoverable timeout: %u seconds", adapter->discoverable_timeout);
	DBG("Pairable timeout: %u seconds", adapter->pairable_timeout);

	adapter->auths = g_queue_new();

	return btd_adapter_ref(adapter);
}

static void adapter_remove(struct btd_adapter *adapter)
{
	GSList *l;
	struct gatt_db *db;

	DBG("Removing adapter %s", adapter->path);

	g_slist_free(adapter->connect_list);
	adapter->connect_list = NULL;

	for (l = adapter->devices; l; l = l->next)
		device_remove(l->data, FALSE);

	g_slist_free(adapter->devices);
	adapter->devices = NULL;

	discovery_cleanup(adapter, 0);

	unload_drivers(adapter);

	db = btd_gatt_database_get_db(adapter->database);
	gatt_db_unregister(db, adapter->db_id);
	adapter->db_id = 0;

	btd_gatt_database_destroy(adapter->database);
	adapter->database = NULL;

	btd_adv_manager_destroy(adapter->adv_manager);
	adapter->adv_manager = NULL;

	btd_adv_monitor_manager_destroy(adapter->adv_monitor_manager);
	adapter->adv_monitor_manager = NULL;

	btd_battery_provider_manager_destroy(adapter->battery_provider_manager);
	adapter->battery_provider_manager = NULL;

	g_slist_free(adapter->pin_callbacks);
	adapter->pin_callbacks = NULL;

	g_slist_free(adapter->msd_callbacks);
	adapter->msd_callbacks = NULL;
}

const char *adapter_get_path(struct btd_adapter *adapter)
{
	if (!adapter)
		return NULL;

	return adapter->path;
}

const bdaddr_t *btd_adapter_get_address(struct btd_adapter *adapter)
{
	return &adapter->bdaddr;
}

static gboolean confirm_name_timeout(gpointer user_data)
{
	struct btd_adapter *adapter = user_data;

	btd_error(adapter->dev_id, "Confirm name timed out for hci%u",
							adapter->dev_id);

	adapter->confirm_name_timeout = 0;

	mgmt_cancel(adapter->mgmt, adapter->confirm_name_id);
	adapter->confirm_name_id = 0;

	return FALSE;
}

static void confirm_name_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
				"Failed to confirm name for hci%u: %s (0x%02x)",
				adapter->dev_id, mgmt_errstr(status), status);
	}

	adapter->confirm_name_id = 0;

	g_source_remove(adapter->confirm_name_timeout);
	adapter->confirm_name_timeout = 0;

	DBG("Confirm name complete for hci%u", adapter->dev_id);
}

static void confirm_name(struct btd_adapter *adapter, const bdaddr_t *bdaddr,
					uint8_t bdaddr_type, bool name_known)
{
	struct mgmt_cp_confirm_name cp;
	char addr[18];

	ba2str(bdaddr, addr);
	DBG("hci%d bdaddr %s name_known %u", adapter->dev_id, addr,
								name_known);

	/*
	 * If the kernel does not answer the confirm name command with
	 * a command complete or command status in time, this might
	 * race against another device found event that also requires
	 * to confirm the name. If there is a pending command, just
	 * cancel it to be safe here.
	 */
	if (adapter->confirm_name_id > 0) {
		btd_warn(adapter->dev_id,
				"Found pending confirm name for hci%u",
							adapter->dev_id);
		mgmt_cancel(adapter->mgmt, adapter->confirm_name_id);
	}

	if (adapter->confirm_name_timeout > 0) {
		g_source_remove(adapter->confirm_name_timeout);
		adapter->confirm_name_timeout = 0;
	}

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, bdaddr);
	cp.addr.type = bdaddr_type;
	cp.name_known = name_known;

	adapter->confirm_name_id = mgmt_reply(adapter->mgmt,
					MGMT_OP_CONFIRM_NAME,
					adapter->dev_id, sizeof(cp), &cp,
					confirm_name_complete, adapter, NULL);

	if (adapter->confirm_name_id == 0) {
		btd_error(adapter->dev_id, "Failed to confirm name for hci%u",
							adapter->dev_id);
		return;
	}

	/*
	 * This timeout handling is needed since the kernel is stupid
	 * and forgets to send a command complete response. However in
	 * case of failures it does send a command status.
	 */
	adapter->confirm_name_timeout = g_timeout_add_seconds(2,
						confirm_name_timeout, adapter);
}

static void adapter_msd_notify(struct btd_adapter *adapter,
							struct btd_device *dev,
							GSList *msd_list)
{
	GSList *cb_l, *cb_next;
	GSList *msd_l, *msd_next;

	for (cb_l = adapter->msd_callbacks; cb_l != NULL; cb_l = cb_next) {
		btd_msd_cb_t cb = cb_l->data;

		cb_next = g_slist_next(cb_l);

		for (msd_l = msd_list; msd_l != NULL; msd_l = msd_next) {
			const struct eir_msd *msd = msd_l->data;

			msd_next = g_slist_next(msd_l);

			cb(adapter, dev, msd->company, msd->data,
								msd->data_len);
		}
	}
}

static bool is_filter_match(GSList *discovery_filter, struct eir_data *eir_data,
								int8_t rssi)
{
	GSList *l, *m;
	bool got_match = false;

	for (l = discovery_filter; l != NULL && got_match != true;
							l = g_slist_next(l)) {
		struct discovery_client *client = l->data;
		struct discovery_filter *item = client->discovery_filter;

		/*
		 * If one of currently running scans is regular scan, then
		 * return all devices as matches
		 */
		if (!item) {
			got_match = true;
			continue;
		}

		/* if someone started discovery with empty uuids, he wants all
		 * devices in given proximity.
		 */
		if (!item->uuids)
			got_match = true;
		else {
			for (m = item->uuids; m != NULL && got_match != true;
							m = g_slist_next(m)) {
				/* m->data contains string representation of
				 * uuid.
				 */
				if (g_slist_find_custom(eir_data->services,
							m->data,
							g_strcmp) != NULL)
					got_match = true;
			}
		}

		if (got_match) {
			/* we have service match, check proximity */
			if (item->rssi == DISTANCE_VAL_INVALID ||
			    item->rssi <= rssi ||
			    item->pathloss == DISTANCE_VAL_INVALID ||
			    (eir_data->tx_power != 127 &&
			     eir_data->tx_power - rssi <= item->pathloss))
				return true;

			got_match = false;
		}
	}

	return got_match;
}

static void filter_duplicate_data(void *data, void *user_data)
{
	struct discovery_client *client = data;
	bool *duplicate = user_data;

	if (*duplicate || !client->discovery_filter)
		return;

	*duplicate = client->discovery_filter->duplicate;
}

static bool device_is_discoverable(struct btd_adapter *adapter,
					struct eir_data *eir, const char *addr,
					uint8_t bdaddr_type)
{
	GSList *l;
	bool discoverable;

	if (bdaddr_type == BDADDR_BREDR || adapter->filtered_discovery)
		discoverable = true;
	else
		discoverable = eir->flags & (EIR_LIM_DISC | EIR_GEN_DISC);

	/*
	 * Mark as not discoverable if no client has requested discovery and
	 * report has not set any discoverable flags.
	 */
	if (!adapter->discovery_list && !discoverable)
		return false;

	/* Do a prefix match for both address and name if pattern is set */
	for (l = adapter->discovery_list; l; l = g_slist_next(l)) {
		struct discovery_client *client = l->data;
		struct discovery_filter *filter = client->discovery_filter;
		size_t pattern_len;

		if (!filter || !filter->pattern)
			continue;

		/* Reset discoverable if a client has a pattern filter */
		discoverable = false;

		pattern_len = strlen(filter->pattern);
		if (!pattern_len)
			return true;

		if (!strncmp(filter->pattern, addr, pattern_len))
			return true;

		if (eir->name && !strncmp(filter->pattern, eir->name,
							pattern_len))
			return true;
	}

	return discoverable;
}

static void update_found_devices(struct btd_adapter *adapter,
					const bdaddr_t *bdaddr,
					uint8_t bdaddr_type, int8_t rssi,
					bool confirm, bool legacy,
					bool not_connectable,
					const uint8_t *data, uint8_t data_len)
{
	struct btd_device *dev;
	struct bt_ad *ad = NULL;
	struct eir_data eir_data;
	bool name_known, discoverable;
	char addr[18];
	bool duplicate = false;
	struct queue *matched_monitors = NULL;

	if (bdaddr_type != BDADDR_BREDR)
		ad = bt_ad_new_with_data(data_len, data);

	/* During the background scanning, update the device only when the data
	 * match at least one Adv monitor
	 */
	if (ad) {
		matched_monitors = btd_adv_monitor_content_filter(
					adapter->adv_monitor_manager, ad);
		bt_ad_unref(ad);
		ad = NULL;
	}

	if (!adapter->discovering && !matched_monitors)
		return;

	memset(&eir_data, 0, sizeof(eir_data));
	eir_parse(&eir_data, data, data_len);

	ba2str(bdaddr, addr);

	discoverable = device_is_discoverable(adapter, &eir_data, addr,
							bdaddr_type);

	dev = btd_adapter_find_device(adapter, bdaddr, bdaddr_type);
	if (!dev) {
		if (!discoverable) {
			eir_data_free(&eir_data);
			return;
		}

		dev = adapter_create_device(adapter, bdaddr, bdaddr_type);
	}

	if (!dev) {
		btd_error(adapter->dev_id,
			"Unable to create object for found device %s", addr);
		eir_data_free(&eir_data);
		return;
	}

	device_update_last_seen(dev, bdaddr_type);

	/*
	 * FIXME: We need to check for non-zero flags first because
	 * older kernels send separate adv_ind and scan_rsp. Newer
	 * kernels send them merged, so once we know which mgmt version
	 * supports this we can make the non-zero check conditional.
	 */
	if (bdaddr_type != BDADDR_BREDR && eir_data.flags &&
					!(eir_data.flags & EIR_BREDR_UNSUP)) {
		device_set_bredr_support(dev);
		/* Update last seen for BR/EDR in case its flag is set */
		device_update_last_seen(dev, BDADDR_BREDR);
	}

	if (eir_data.name != NULL && eir_data.name_complete)
		device_store_cached_name(dev, eir_data.name);

	/*
	 * Only skip devices that are not connected, are temporary, and there
	 * is no active discovery session ongoing and no matched Adv monitors
	 */
	if (!btd_device_is_connected(dev) &&
		(device_is_temporary(dev) && !adapter->discovery_list) &&
		!matched_monitors) {
		eir_data_free(&eir_data);
		return;
	}

	/* If there is no matched Adv monitors, don't continue if not
	 * discoverable or if active discovery filter don't match.
	 */
	if (!matched_monitors && (!discoverable ||
		(adapter->filtered_discovery && !is_filter_match(
				adapter->discovery_list, &eir_data, rssi)))) {
		eir_data_free(&eir_data);
		return;
	}

	device_set_legacy(dev, legacy);

	if (adapter->filtered_discovery)
		device_set_rssi_with_delta(dev, rssi, 0);
	else
		device_set_rssi(dev, rssi);

	if (eir_data.tx_power != 127)
		device_set_tx_power(dev, eir_data.tx_power);

	if (eir_data.appearance != 0)
		device_set_appearance(dev, eir_data.appearance);

	/* Report an unknown name to the kernel even if there is a short name
	 * known, but still update the name with the known short name. */
	name_known = device_name_known(dev);

	if (eir_data.name && (eir_data.name_complete || !name_known))
		btd_device_device_set_name(dev, eir_data.name);

	if (eir_data.class != 0)
		device_set_class(dev, eir_data.class);

	if (eir_data.did_source || eir_data.did_vendor ||
			eir_data.did_product || eir_data.did_version)
		btd_device_set_pnpid(dev, eir_data.did_source,
							eir_data.did_vendor,
							eir_data.did_product,
							eir_data.did_version);

	device_add_eir_uuids(dev, eir_data.services);

	if (adapter->discovery_list)
		g_slist_foreach(adapter->discovery_list, filter_duplicate_data,
								&duplicate);

	if (eir_data.msd_list) {
		device_set_manufacturer_data(dev, eir_data.msd_list, duplicate);
		adapter_msd_notify(adapter, dev, eir_data.msd_list);
	}

	if (eir_data.sd_list)
		device_set_service_data(dev, eir_data.sd_list, duplicate);

	if (eir_data.data_list)
		device_set_data(dev, eir_data.data_list, duplicate);

	if (bdaddr_type != BDADDR_BREDR)
		device_set_flags(dev, eir_data.flags);

	eir_data_free(&eir_data);

	/* After the device is updated, notify the matched Adv monitors */
	if (matched_monitors) {
		btd_adv_monitor_notify_monitors(adapter->adv_monitor_manager,
						dev, rssi, matched_monitors);
		queue_destroy(matched_monitors, NULL);
		matched_monitors = NULL;
	}

	/*
	 * Only if at least one client has requested discovery, maintain
	 * list of found devices and name confirming for legacy devices.
	 * Otherwise, this is an event from passive discovery and we
	 * should check if the device needs connecting to.
	 */
	if (!adapter->discovery_list)
		goto connect_le;

	if (g_slist_find(adapter->discovery_found, dev))
		return;

	if (confirm)
		confirm_name(adapter, bdaddr, bdaddr_type, name_known);

	adapter->discovery_found = g_slist_prepend(adapter->discovery_found,
									dev);

	return;

connect_le:
	/* Ignore non-connectable events */
	if (not_connectable)
		return;

	/*
	 * If we're in the process of stopping passive scanning and
	 * connecting another (or maybe even the same) LE device just
	 * ignore this one.
	 */
	if (adapter->connect_le)
		return;

	/*
	 * If kernel background scan is used then the kernel is
	 * responsible for connecting.
	 */
	if (btd_has_kernel_features(KERNEL_CONN_CONTROL))
		return;

	/*
	 * If this is an LE device that's not connected and part of the
	 * connect_list stop passive scanning so that a connection
	 * attempt to it can be made
	 */
	if (bdaddr_type != BDADDR_BREDR && !btd_device_is_connected(dev) &&
				g_slist_find(adapter->connect_list, dev)) {
		adapter->connect_le = dev;
		stop_passive_scanning(adapter);
	}
}

static void device_found_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_device_found *ev = param;
	struct btd_adapter *adapter = user_data;
	const uint8_t *eir;
	uint16_t eir_len;
	uint32_t flags;
	bool confirm_name;
	bool legacy;
	char addr[18];

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id,
			"Too short device found event (%u bytes)", length);
		return;
	}

	eir_len = btohs(ev->eir_len);
	if (length != sizeof(*ev) + eir_len) {
		btd_error(adapter->dev_id,
				"Device found event size mismatch (%u != %zu)",
					length, sizeof(*ev) + eir_len);
		return;
	}

	if (eir_len == 0)
		eir = NULL;
	else
		eir = ev->eir;

	flags = btohl(ev->flags);

	ba2str(&ev->addr.bdaddr, addr);
	DBG("hci%u addr %s, rssi %d flags 0x%04x eir_len %u",
			index, addr, ev->rssi, flags, eir_len);

	confirm_name = (flags & MGMT_DEV_FOUND_CONFIRM_NAME);
	legacy = (flags & MGMT_DEV_FOUND_LEGACY_PAIRING);

	update_found_devices(adapter, &ev->addr.bdaddr, ev->addr.type,
					ev->rssi, confirm_name, legacy,
					flags & MGMT_DEV_FOUND_NOT_CONNECTABLE,
					eir, eir_len);
}

struct agent *adapter_get_agent(struct btd_adapter *adapter)
{
	return agent_get(NULL);
}

static void adapter_remove_connection(struct btd_adapter *adapter,
						struct btd_device *device,
						uint8_t bdaddr_type)
{
#if defined(MAPAC1750)
	char buf[256];

	memset(buf, '\0', sizeof(buf));
	strncpy(buf, nvram_safe_get("bt_turn_off_service"), sizeof(buf));
#endif
	DBG("");

	if (!g_slist_find(adapter->connections, device)) {
		btd_error(adapter->dev_id, "No matching connection for device");
		return;
	}

	device_remove_connection(device, bdaddr_type);

	if (device_is_authenticating(device))
		device_cancel_authentication(device, TRUE);

	/* If another bearer is still connected */
	if (btd_device_is_connected(device))
		return;

	adapter->connections = g_slist_remove(adapter->connections, device);

	if ( 1
#if defined(RTCONFIG_LP5523)
		&& !g_slist_length(adapter->connections)
#elif defined(MAPAC1750)
		&& (strstr(buf, "ble_qis_done") == NULL)
#endif
	) {
#if defined(RTCONFIG_LP5523)
		lp55xx_leds_proc(LP55XX_BTCOR_LEDS, LP55XX_ACT_NONE);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
		set_rgbled(RGBLED_DEFAULT_STANDBY);
#endif
#if defined(RTCONFIG_PRELINK) 
		nvram_set_int("ble_dut_con", 0);
		notify_rc_and_wait("restart_ble_rename_ssid");
#endif
	}

#ifdef RTCONFIG_LANTIQ
	notify_rc("restart_bluetooth_service");
#endif
#if defined(RTAX95Q) || defined(XT8PRO) || defined(BM68) || defined(XT8_V2) || defined(RTAXE95Q) || defined(ET8PRO) || defined(ET8_V2) || defined(RTAX56_XD4) || defined(XD4PRO) || defined(ET12) || defined(XT12) || defined(BC109) || defined(BC105) || defined(EBG19) || defined(EBG15) || defined(XC5)
	/// enable advertising
	if (bdaddr_type != BDADDR_BREDR) {
		int dd = hci_open_dev(adapter->dev_id);
		if (dd < 0) {
			btd_warn(adapter->dev_id, "Open hci device failed");
		}
		else {
			if (hci_le_set_advertise_enable(dd, 0x01, 1000) < 0) {
				btd_warn(adapter->dev_id, "Enable advertise failed");
			}
			hci_close_dev(dd);
		}
	}
#endif
}

static void adapter_stop(struct btd_adapter *adapter)
{
	/* check pending requests */
	reply_pending_requests(adapter);

	cancel_passive_scanning(adapter);

	remove_discovery_list(adapter);

	discovery_cleanup(adapter, 0);

	adapter->filtered_discovery = false;
	adapter->no_scan_restart_delay = false;
	g_free(adapter->current_discovery_filter);
	adapter->current_discovery_filter = NULL;

	adapter->discovering = false;

	while (adapter->connections) {
		struct btd_device *device = adapter->connections->data;
		uint8_t addr_type = btd_device_get_bdaddr_type(device);

		adapter_remove_connection(adapter, device, BDADDR_BREDR);
		if (addr_type != BDADDR_BREDR)
			adapter_remove_connection(adapter, device, addr_type);
	}

	g_dbus_emit_property_changed(dbus_conn, adapter->path,
					ADAPTER_INTERFACE, "Discovering");

	if (adapter->dev_class) {
		/* the kernel should reset the class of device when powering
		 * down, but it does not. So force it here ... */
		adapter->dev_class = 0;
		g_dbus_emit_property_changed(dbus_conn, adapter->path,
						ADAPTER_INTERFACE, "Class");
	}

	g_dbus_emit_property_changed(dbus_conn, adapter->path,
						ADAPTER_INTERFACE, "Powered");

	DBG("adapter %s has been disabled", adapter->path);
}

int btd_register_adapter_driver(struct btd_adapter_driver *driver)
{
	adapter_drivers = g_slist_append(adapter_drivers, driver);

	if (driver->probe == NULL)
		return 0;

	adapter_foreach(probe_driver, driver);

	return 0;
}

static void unload_driver(struct btd_adapter *adapter, gpointer data)
{
	struct btd_adapter_driver *driver = data;

	if (driver->remove)
		driver->remove(adapter);

	adapter->drivers = g_slist_remove(adapter->drivers, data);
}

void btd_unregister_adapter_driver(struct btd_adapter_driver *driver)
{
	adapter_drivers = g_slist_remove(adapter_drivers, driver);

	adapter_foreach(unload_driver, driver);
}

static void agent_auth_cb(struct agent *agent, DBusError *derr,
							void *user_data)
{
	struct btd_adapter *adapter = user_data;
	struct service_auth *auth = g_queue_pop_head(adapter->auths);

	if (!auth) {
		DBG("No pending authorization");
		return;
	}

	auth->cb(derr, auth->user_data);

	if (auth->agent)
		agent_unref(auth->agent);

	g_free(auth);

	/* Stop processing if queue is empty */
	if (g_queue_is_empty(adapter->auths)) {
		if (adapter->auth_idle_id > 0)
			g_source_remove(adapter->auth_idle_id);
		return;
	}

	if (adapter->auth_idle_id > 0)
		return;

	adapter->auth_idle_id = g_idle_add(process_auth_queue, adapter);
}

static gboolean process_auth_queue(gpointer user_data)
{
	struct btd_adapter *adapter = user_data;
	DBusError err;

	adapter->auth_idle_id = 0;

	dbus_error_init(&err);
	dbus_set_error_const(&err, ERROR_INTERFACE ".Rejected", NULL);

	while (!g_queue_is_empty(adapter->auths)) {
		struct service_auth *auth = adapter->auths->head->data;
		struct btd_device *device = auth->device;

		/* Wait services to be resolved before asking authorization */
		if (auth->svc_id > 0)
			return FALSE;

		if (device_is_trusted(device) == TRUE) {
			auth->cb(NULL, auth->user_data);
			goto next;
		}

		/* If agent is set authorization is already ongoing */
		if (auth->agent)
			return FALSE;

		auth->agent = agent_get(NULL);
		if (auth->agent == NULL) {
			btd_warn(adapter->dev_id,
					"Authentication attempt without agent");
			auth->cb(&err, auth->user_data);
			goto next;
		}

		if (agent_authorize_service(auth->agent, device, auth->uuid,
					agent_auth_cb, adapter, NULL) < 0) {
			auth->cb(&err, auth->user_data);
			goto next;
		}

		break;

next:
		if (auth->agent)
			agent_unref(auth->agent);

		g_free(auth);

		g_queue_pop_head(adapter->auths);
	}

	dbus_error_free(&err);

	return FALSE;
}

static void svc_complete(struct btd_device *dev, int err, void *user_data)
{
	struct service_auth *auth = user_data;
	struct btd_adapter *adapter = auth->adapter;

	auth->svc_id = 0;

	if (adapter->auth_idle_id != 0)
		return;

	adapter->auth_idle_id = g_idle_add(process_auth_queue, adapter);
}

static int adapter_authorize(struct btd_adapter *adapter, const bdaddr_t *dst,
					const char *uuid,
					adapter_authorize_type check_for_connection,
					service_auth_cb cb, void *user_data)
{
	struct service_auth *auth;
	struct btd_device *device;
	static guint id = 0;

	device = btd_adapter_find_device(adapter, dst, BDADDR_BREDR);
	if (!device)
		return 0;

	if (device_is_disconnecting(device)) {
		DBG("Authorization request while disconnecting");
		return 0;
	}

	/* Device connected? */
	if (check_for_connection && !g_slist_find(adapter->connections, device))
		btd_error(adapter->dev_id,
			"Authorization request for non-connected device!?");

	auth = g_try_new0(struct service_auth, 1);
	if (!auth)
		return 0;

	auth->cb = cb;
	auth->user_data = user_data;
	auth->uuid = uuid;
	auth->device = device;
	auth->adapter = adapter;
	auth->id = ++id;
	if (check_for_connection)
		auth->svc_id = device_wait_for_svc_complete(device, svc_complete, auth);
	else {
		if (adapter->auth_idle_id == 0)
			adapter->auth_idle_id = g_idle_add(process_auth_queue, adapter);
	}

	g_queue_push_tail(adapter->auths, auth);

	return auth->id;
}

guint btd_request_authorization(const bdaddr_t *src, const bdaddr_t *dst,
					const char *uuid, service_auth_cb cb,
					void *user_data)
{
	struct btd_adapter *adapter;
	GSList *l;

	if (bacmp(src, BDADDR_ANY) != 0) {
		adapter = adapter_find(src);
		if (!adapter)
			return 0;

		return adapter_authorize(adapter, dst, uuid,
				ADAPTER_AUTHORIZE_CHECK_CONNECTED, cb, user_data);
	}

	for (l = adapters; l != NULL; l = g_slist_next(l)) {
		guint id;

		adapter = l->data;

		id = adapter_authorize(adapter, dst, uuid,
				ADAPTER_AUTHORIZE_CHECK_CONNECTED, cb, user_data);
		if (id != 0)
			return id;
	}

	return 0;
}

guint btd_request_authorization_cable_configured(const bdaddr_t *src, const bdaddr_t *dst,
						const char *uuid, service_auth_cb cb,
						void *user_data)
{
	struct btd_adapter *adapter;

	if (bacmp(src, BDADDR_ANY) == 0)
		return 0;

	adapter = adapter_find(src);
	if (!adapter)
		return 0;

	return adapter_authorize(adapter, dst, uuid,
			ADAPTER_AUTHORIZE_DISCONNECTED, cb, user_data);
}

static struct service_auth *find_authorization(guint id)
{
	GSList *l;
	GList *l2;

	for (l = adapters; l != NULL; l = g_slist_next(l)) {
		struct btd_adapter *adapter = l->data;

		for (l2 = adapter->auths->head; l2 != NULL; l2 = l2->next) {
			struct service_auth *auth = l2->data;

			if (auth->id == id)
				return auth;
		}
	}

	return NULL;
}

int btd_cancel_authorization(guint id)
{
	struct service_auth *auth;

	auth = find_authorization(id);
	if (auth == NULL)
		return -EPERM;

	if (auth->svc_id > 0)
		device_remove_svc_complete_callback(auth->device,
								auth->svc_id);

	g_queue_remove(auth->adapter->auths, auth);

	if (auth->agent) {
		agent_cancel(auth->agent);
		agent_unref(auth->agent);
	}

	g_free(auth);

	return 0;
}

int btd_adapter_restore_powered(struct btd_adapter *adapter)
{
	if (btd_adapter_get_powered(adapter))
		return 0;

	set_mode(adapter, MGMT_OP_SET_POWERED, 0x01);

	return 0;
}

void btd_adapter_register_pin_cb(struct btd_adapter *adapter,
							btd_adapter_pin_cb_t cb)
{
	adapter->pin_callbacks = g_slist_prepend(adapter->pin_callbacks, cb);
}

void btd_adapter_unregister_pin_cb(struct btd_adapter *adapter,
							btd_adapter_pin_cb_t cb)
{
	adapter->pin_callbacks = g_slist_remove(adapter->pin_callbacks, cb);
}

void btd_adapter_unregister_msd_cb(struct btd_adapter *adapter,
							btd_msd_cb_t cb)
{
	adapter->msd_callbacks = g_slist_remove(adapter->msd_callbacks, cb);
}

void btd_adapter_register_msd_cb(struct btd_adapter *adapter,
							btd_msd_cb_t cb)
{
	adapter->msd_callbacks = g_slist_prepend(adapter->msd_callbacks, cb);
}

int btd_adapter_set_fast_connectable(struct btd_adapter *adapter,
							gboolean enable)
{
	if (!btd_adapter_get_powered(adapter))
		return -EINVAL;

	set_mode(adapter, MGMT_OP_SET_FAST_CONNECTABLE, enable ? 0x01 : 0x00);

	return 0;
}

int btd_adapter_read_clock(struct btd_adapter *adapter, const bdaddr_t *bdaddr,
				int which, int timeout, uint32_t *clock,
				uint16_t *accuracy)
{
	if (!btd_adapter_get_powered(adapter))
		return -EINVAL;

	return -ENOSYS;
}

int btd_adapter_remove_bonding(struct btd_adapter *adapter,
				const bdaddr_t *bdaddr, uint8_t bdaddr_type)
{
	struct mgmt_cp_unpair_device cp;

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, bdaddr);
	cp.addr.type = bdaddr_type;
	cp.disconnect = 1;

	if (mgmt_send(adapter->mgmt, MGMT_OP_UNPAIR_DEVICE,
				adapter->dev_id, sizeof(cp), &cp,
				NULL, NULL, NULL) > 0)
		return 0;

	return -EIO;
}

static void pincode_reply_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_device *device = user_data;

	/* If the MGMT_OP_PIN_CODE_REPLY command is acknowledged, move the
	 * starting time to that point. This give a better sense of time
	 * evaluating the pincode. */
	device_bonding_restart_timer(device);
}

int btd_adapter_pincode_reply(struct btd_adapter *adapter,
					const bdaddr_t *bdaddr,
					const char *pin, size_t pin_len)
{
	struct btd_device *device;
	unsigned int id;
	char addr[18];

	ba2str(bdaddr, addr);
	DBG("hci%u addr %s pinlen %zu", adapter->dev_id, addr, pin_len);

	if (pin == NULL) {
		struct mgmt_cp_pin_code_neg_reply cp;

		memset(&cp, 0, sizeof(cp));
		bacpy(&cp.addr.bdaddr, bdaddr);
		cp.addr.type = BDADDR_BREDR;

		id = mgmt_reply(adapter->mgmt, MGMT_OP_PIN_CODE_NEG_REPLY,
					adapter->dev_id, sizeof(cp), &cp,
					NULL, NULL, NULL);
	} else {
		struct mgmt_cp_pin_code_reply cp;

		if (pin_len > 16)
			return -EINVAL;

		memset(&cp, 0, sizeof(cp));
		bacpy(&cp.addr.bdaddr, bdaddr);
		cp.addr.type = BDADDR_BREDR;
		cp.pin_len = pin_len;
		memcpy(cp.pin_code, pin, pin_len);

		/* Since a pincode was requested, update the starting time to
		 * the point where the pincode is provided. */
		device = btd_adapter_find_device(adapter, bdaddr, BDADDR_BREDR);
		device_bonding_restart_timer(device);

		id = mgmt_reply(adapter->mgmt, MGMT_OP_PIN_CODE_REPLY,
					adapter->dev_id, sizeof(cp), &cp,
					pincode_reply_complete, device, NULL);
	}

	if (id == 0)
		return -EIO;

	return 0;
}

int btd_adapter_confirm_reply(struct btd_adapter *adapter,
				const bdaddr_t *bdaddr, uint8_t bdaddr_type,
				gboolean success)
{
	struct mgmt_cp_user_confirm_reply cp;
	uint16_t opcode;
	char addr[18];

	ba2str(bdaddr, addr);
	DBG("hci%u addr %s success %d", adapter->dev_id, addr, success);

	if (success)
		opcode = MGMT_OP_USER_CONFIRM_REPLY;
	else
		opcode = MGMT_OP_USER_CONFIRM_NEG_REPLY;

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, bdaddr);
	cp.addr.type = bdaddr_type;

	if (mgmt_reply(adapter->mgmt, opcode, adapter->dev_id, sizeof(cp), &cp,
							NULL, NULL, NULL) > 0)
		return 0;

	return -EIO;
}

static void user_confirm_request_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_user_confirm_request *ev = param;
	struct btd_adapter *adapter = user_data;
	struct btd_device *device;
	char addr[18];
	int err;

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id,
				"Too small user confirm request event");
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);
	DBG("hci%u %s confirm_hint %u", adapter->dev_id, addr,
							ev->confirm_hint);
	device = btd_adapter_get_device(adapter, &ev->addr.bdaddr,
								ev->addr.type);
	if (!device) {
		btd_error(adapter->dev_id,
				"Unable to get device object for %s", addr);
		return;
	}

	err = device_confirm_passkey(device, ev->addr.type, btohl(ev->value),
							ev->confirm_hint);
	if (err < 0) {
		btd_error(adapter->dev_id,
				"device_confirm_passkey: %s", strerror(-err));
		btd_adapter_confirm_reply(adapter, &ev->addr.bdaddr,
							ev->addr.type, FALSE);
	}
}

int btd_adapter_passkey_reply(struct btd_adapter *adapter,
				const bdaddr_t *bdaddr, uint8_t bdaddr_type,
				uint32_t passkey)
{
	unsigned int id;
	char addr[18];

	ba2str(bdaddr, addr);
	DBG("hci%u addr %s passkey %06u", adapter->dev_id, addr, passkey);

	if (passkey == INVALID_PASSKEY) {
		struct mgmt_cp_user_passkey_neg_reply cp;

		memset(&cp, 0, sizeof(cp));
		bacpy(&cp.addr.bdaddr, bdaddr);
		cp.addr.type = bdaddr_type;

		id = mgmt_reply(adapter->mgmt, MGMT_OP_USER_PASSKEY_NEG_REPLY,
					adapter->dev_id, sizeof(cp), &cp,
					NULL, NULL, NULL);
	} else {
		struct mgmt_cp_user_passkey_reply cp;

		memset(&cp, 0, sizeof(cp));
		bacpy(&cp.addr.bdaddr, bdaddr);
		cp.addr.type = bdaddr_type;
		cp.passkey = htobl(passkey);

		id = mgmt_reply(adapter->mgmt, MGMT_OP_USER_PASSKEY_REPLY,
					adapter->dev_id, sizeof(cp), &cp,
					NULL, NULL, NULL);
	}

	if (id == 0)
		return -EIO;

	return 0;
}

static void user_passkey_request_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_user_passkey_request *ev = param;
	struct btd_adapter *adapter = user_data;
	struct btd_device *device;
	char addr[18];
	int err;

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small passkey request event");
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);
	DBG("hci%u %s", index, addr);

	device = btd_adapter_get_device(adapter, &ev->addr.bdaddr,
								ev->addr.type);
	if (!device) {
		btd_error(adapter->dev_id,
				"Unable to get device object for %s", addr);
		return;
	}

	err = device_request_passkey(device, ev->addr.type);
	if (err < 0) {
		btd_error(adapter->dev_id,
				"device_request_passkey: %s", strerror(-err));
		btd_adapter_passkey_reply(adapter, &ev->addr.bdaddr,
					ev->addr.type, INVALID_PASSKEY);
	}
}

static void user_passkey_notify_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_passkey_notify *ev = param;
	struct btd_adapter *adapter = user_data;
	struct btd_device *device;
	uint32_t passkey;
	char addr[18];
	int err;

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small passkey notify event");
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);
	DBG("hci%u %s", index, addr);

	device = btd_adapter_get_device(adapter, &ev->addr.bdaddr,
								ev->addr.type);
	if (!device) {
		btd_error(adapter->dev_id,
				"Unable to get device object for %s", addr);
		return;
	}

	passkey = get_le32(&ev->passkey);

	DBG("passkey %06u entered %u", passkey, ev->entered);

	err = device_notify_passkey(device, ev->addr.type, passkey,
								ev->entered);
	if (err < 0)
		btd_error(adapter->dev_id,
				"device_notify_passkey: %s", strerror(-err));
}

struct btd_adapter_pin_cb_iter *btd_adapter_pin_cb_iter_new(
						struct btd_adapter *adapter)
{
	struct btd_adapter_pin_cb_iter *iter =
				g_new0(struct btd_adapter_pin_cb_iter, 1);

	iter->it = adapter->pin_callbacks;
	iter->attempt = 1;

	return iter;
}

void btd_adapter_pin_cb_iter_free(struct btd_adapter_pin_cb_iter *iter)
{
	g_free(iter);
}

bool btd_adapter_pin_cb_iter_end(struct btd_adapter_pin_cb_iter *iter)
{
	return iter->it == NULL && iter->attempt == 0;
}

static ssize_t btd_adapter_pin_cb_iter_next(
					struct btd_adapter_pin_cb_iter *iter,
					struct btd_adapter *adapter,
					struct btd_device *device,
					char *pin_buf, bool *display)
{
	btd_adapter_pin_cb_t cb;
	ssize_t ret;

	while (iter->it != NULL) {
		cb = iter->it->data;
		ret = cb(adapter, device, pin_buf, display, iter->attempt);
		iter->attempt++;
		if (ret > 0)
			return ret;
		iter->attempt = 1;
		iter->it = g_slist_next(iter->it);
	}
	iter->attempt = 0;

	return 0;
}

static void pin_code_request_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_pin_code_request *ev = param;
	struct btd_adapter *adapter = user_data;
	struct btd_device *device;
	bool display = false;
	char pin[17];
	ssize_t pinlen;
	char addr[18];
	int err;
	struct btd_adapter_pin_cb_iter *iter;

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small PIN code request event");
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);

	DBG("hci%u %s", adapter->dev_id, addr);

	device = btd_adapter_get_device(adapter, &ev->addr.bdaddr,
								ev->addr.type);
	if (!device) {
		btd_error(adapter->dev_id,
				"Unable to get device object for %s", addr);
		return;
	}

	/* Flag the request of a pincode to allow a bonding retry. */
	adapter->pincode_requested = true;

	memset(pin, 0, sizeof(pin));

	iter = device_bonding_iter(device);
	if (iter == NULL)
		pinlen = 0;
	else
		pinlen = btd_adapter_pin_cb_iter_next(iter, adapter, device,
								pin, &display);

	if (pinlen > 0 && (!ev->secure || pinlen == 16)) {
		if (display && device_is_bonding(device, NULL)) {
			err = device_notify_pincode(device, ev->secure, pin);
			if (err < 0) {
				btd_error(adapter->dev_id,
						"device_notify_pin: %s",
							strerror(-err));
				btd_adapter_pincode_reply(adapter,
							&ev->addr.bdaddr,
							NULL, 0);
			}
		} else {
			btd_adapter_pincode_reply(adapter, &ev->addr.bdaddr,
								pin, pinlen);
		}
		return;
	}

	err = device_request_pincode(device, ev->secure);
	if (err < 0) {
		btd_error(adapter->dev_id, "device_request_pin: %s",
							strerror(-err));
		btd_adapter_pincode_reply(adapter, &ev->addr.bdaddr, NULL, 0);
	}
}

int adapter_cancel_bonding(struct btd_adapter *adapter, const bdaddr_t *bdaddr,
							uint8_t addr_type)
{
	struct mgmt_addr_info cp;
	char addr[18];

	ba2str(bdaddr, addr);
	DBG("hci%u bdaddr %s type %u", adapter->dev_id, addr, addr_type);

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.bdaddr, bdaddr);
	cp.type = addr_type;

	if (mgmt_reply(adapter->mgmt, MGMT_OP_CANCEL_PAIR_DEVICE,
				adapter->dev_id, sizeof(cp), &cp,
				NULL, NULL, NULL) > 0)
		return 0;

	return -EIO;
}

static void check_oob_bonding_complete(struct btd_adapter *adapter,
					const bdaddr_t *bdaddr, uint8_t status)
{
	if (!adapter->oob_handler || !adapter->oob_handler->bonding_cb)
		return;

	if (bacmp(bdaddr, &adapter->oob_handler->remote_addr) != 0)
		return;

	adapter->oob_handler->bonding_cb(adapter, bdaddr, status,
					adapter->oob_handler->user_data);

	g_free(adapter->oob_handler);
	adapter->oob_handler = NULL;
}

static void bonding_complete(struct btd_adapter *adapter,
					const bdaddr_t *bdaddr,
					uint8_t addr_type, uint8_t status)
{
	struct btd_device *device;

	if (status == 0)
		device = btd_adapter_get_device(adapter, bdaddr, addr_type);
	else
		device = btd_adapter_find_device(adapter, bdaddr, addr_type);

	if (device != NULL)
		device_bonding_complete(device, addr_type, status);

	resume_discovery(adapter);

	check_oob_bonding_complete(adapter, bdaddr, status);
}

/* bonding_attempt_complete() handles the end of a "bonding attempt" checking if
 * it should begin a new attempt or complete the bonding.
 */
static void bonding_attempt_complete(struct btd_adapter *adapter,
					const bdaddr_t *bdaddr,
					uint8_t addr_type, uint8_t status)
{
	struct btd_device *device;
	char addr[18];

	ba2str(bdaddr, addr);
	DBG("hci%u bdaddr %s type %u status 0x%x", adapter->dev_id, addr,
							addr_type, status);

	if (status == 0)
		device = btd_adapter_get_device(adapter, bdaddr, addr_type);
	else
		device = btd_adapter_find_device(adapter, bdaddr, addr_type);

	if (status == MGMT_STATUS_AUTH_FAILED && adapter->pincode_requested) {
		/* On faliure, issue a bonding_retry if possible. */
		if (device != NULL) {
			if (device_bonding_attempt_retry(device) == 0)
				return;
		}
	}

	/* Ignore disconnects during retry. */
	if (status == MGMT_STATUS_DISCONNECTED &&
					device && device_is_retrying(device))
		return;

	/* In any other case, finish the bonding. */
	bonding_complete(adapter, bdaddr, addr_type, status);
}

struct pair_device_data {
	struct btd_adapter *adapter;
	bdaddr_t bdaddr;
	uint8_t addr_type;
};

static void free_pair_device_data(void *user_data)
{
	struct pair_device_data *data = user_data;

	g_free(data);
}

static gboolean pair_device_timeout(gpointer user_data)
{
	struct pair_device_data *data = user_data;
	struct btd_adapter *adapter = data->adapter;

	btd_error(adapter->dev_id, "Pair device timed out for hci%u",
							adapter->dev_id);

	adapter->pair_device_timeout = 0;

	adapter_cancel_bonding(adapter, &data->bdaddr, data->addr_type);

	return FALSE;
}

static void pair_device_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_pair_device *rp = param;
	struct pair_device_data *data = user_data;
	struct btd_adapter *adapter = data->adapter;

	DBG("%s (0x%02x)", mgmt_errstr(status), status);

	adapter->pair_device_id = 0;

	if (adapter->pair_device_timeout > 0) {
		g_source_remove(adapter->pair_device_timeout);
		adapter->pair_device_timeout = 0;
	}

	/* Workaround for a kernel bug
	 *
	 * Broken kernels may reply to device pairing command with command
	 * status instead of command complete event e.g. if adapter was not
	 * powered.
	 */
	if (status != MGMT_STATUS_SUCCESS && length < sizeof(*rp)) {
		btd_error(adapter->dev_id, "Pair device failed: %s (0x%02x)",
						mgmt_errstr(status), status);

		bonding_attempt_complete(adapter, &data->bdaddr,
						data->addr_type, status);
		return;
	}

	if (length < sizeof(*rp)) {
		btd_error(adapter->dev_id, "Too small pair device response");
		return;
	}

	bonding_attempt_complete(adapter, &rp->addr.bdaddr, rp->addr.type,
									status);
}

int adapter_create_bonding(struct btd_adapter *adapter, const bdaddr_t *bdaddr,
					uint8_t addr_type, uint8_t io_cap)
{
	if (adapter->pair_device_id > 0) {
		btd_error(adapter->dev_id,
			"Unable pair since another pairing is in progress");
		return -EBUSY;
	}

	suspend_discovery(adapter);

	return adapter_bonding_attempt(adapter, bdaddr, addr_type, io_cap);
}

/* Starts a new bonding attempt in a fresh new bonding_req or a retried one. */
int adapter_bonding_attempt(struct btd_adapter *adapter, const bdaddr_t *bdaddr,
					uint8_t addr_type, uint8_t io_cap)
{
	struct mgmt_cp_pair_device cp;
	char addr[18];
	struct pair_device_data *data;
	unsigned int id;

	ba2str(bdaddr, addr);
	DBG("hci%u bdaddr %s type %d io_cap 0x%02x",
				adapter->dev_id, addr, addr_type, io_cap);

	/* Reset the pincode_requested flag for a new bonding attempt. */
	adapter->pincode_requested = false;

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, bdaddr);
	cp.addr.type = addr_type;
	cp.io_cap = io_cap;

	data = g_new0(struct pair_device_data, 1);
	data->adapter = adapter;
	bacpy(&data->bdaddr, bdaddr);
	data->addr_type = addr_type;

	id = mgmt_send(adapter->mgmt, MGMT_OP_PAIR_DEVICE,
				adapter->dev_id, sizeof(cp), &cp,
				pair_device_complete, data,
				free_pair_device_data);

	if (id == 0) {
		btd_error(adapter->dev_id, "Failed to pair %s for hci%u",
							addr, adapter->dev_id);
		free_pair_device_data(data);
		return -EIO;
	}

	adapter->pair_device_id = id;

	/* Due to a bug in the kernel it is possible that a LE pairing
	 * request never times out. Therefore, add a timer to clean up
	 * if no response arrives
	 */
	adapter->pair_device_timeout = g_timeout_add_seconds(BONDING_TIMEOUT,
						pair_device_timeout, data);

	return 0;
}

static void disconnect_notify(struct btd_device *dev, uint8_t reason)
{
	GSList *l;

	for (l = disconnect_list; l; l = g_slist_next(l)) {
		btd_disconnect_cb disconnect_cb = l->data;
		disconnect_cb(dev, reason);
	}
}

static void dev_disconnected(struct btd_adapter *adapter,
					const struct mgmt_addr_info *addr,
					uint8_t reason)
{
	struct btd_device *device;
	char dst[18];

	ba2str(&addr->bdaddr, dst);

	DBG("Device %s disconnected, reason %u", dst, reason);

	device = btd_adapter_find_device(adapter, &addr->bdaddr, addr->type);
	if (device) {
		adapter_remove_connection(adapter, device, addr->type);
		disconnect_notify(device, reason);
	}

	bonding_attempt_complete(adapter, &addr->bdaddr, addr->type,
						MGMT_STATUS_DISCONNECTED);
}

void btd_add_disconnect_cb(btd_disconnect_cb func)
{
	disconnect_list = g_slist_append(disconnect_list, func);
}

void btd_remove_disconnect_cb(btd_disconnect_cb func)
{
	disconnect_list = g_slist_remove(disconnect_list, func);
}

static void disconnect_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_disconnect *rp = param;
	struct btd_adapter *adapter = user_data;

	if (status == MGMT_STATUS_NOT_CONNECTED) {
		btd_warn(adapter->dev_id,
				"Disconnecting failed: already disconnected");
	} else if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
				"Failed to disconnect device: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	if (length < sizeof(*rp)) {
		btd_error(adapter->dev_id,
				"Too small device disconnect response");
		return;
	}

	dev_disconnected(adapter, &rp->addr, MGMT_DEV_DISCONN_LOCAL_HOST);
}

int btd_adapter_disconnect_device(struct btd_adapter *adapter,
						const bdaddr_t *bdaddr,
						uint8_t bdaddr_type)

{
	struct mgmt_cp_disconnect cp;

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, bdaddr);
	cp.addr.type = bdaddr_type;

	if (mgmt_send(adapter->mgmt, MGMT_OP_DISCONNECT,
				adapter->dev_id, sizeof(cp), &cp,
				disconnect_complete, adapter, NULL) > 0)
		return 0;

	return -EIO;
}

static void auth_failed_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_auth_failed *ev = param;
	struct btd_adapter *adapter = user_data;

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small auth failed mgmt event");
		return;
	}

	bonding_attempt_complete(adapter, &ev->addr.bdaddr, ev->addr.type,
								ev->status);
}

static void store_link_key(struct btd_adapter *adapter,
				struct btd_device *device, const uint8_t *key,
				uint8_t type, uint8_t pin_length)
{
	char device_addr[18];
	char filename[PATH_MAX];
	GKeyFile *key_file;
	gsize length = 0;
	char key_str[33];
	char *str;
	int i;

	ba2str(device_get_address(device), device_addr);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/info",
			btd_adapter_get_storage_dir(adapter), device_addr);
	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	for (i = 0; i < 16; i++)
		sprintf(key_str + (i * 2), "%2.2X", key[i]);

	g_key_file_set_string(key_file, "LinkKey", "Key", key_str);

	g_key_file_set_integer(key_file, "LinkKey", "Type", type);
	g_key_file_set_integer(key_file, "LinkKey", "PINLength", pin_length);

	create_file(filename, S_IRUSR | S_IWUSR);

	str = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, str, length, NULL);
	g_free(str);

	g_key_file_free(key_file);
}

static void new_link_key_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_new_link_key *ev = param;
	const struct mgmt_addr_info *addr = &ev->key.addr;
	struct btd_adapter *adapter = user_data;
	struct btd_device *device;
	char dst[18];

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small new link key event");
		return;
	}

	ba2str(&addr->bdaddr, dst);

	DBG("hci%u new key for %s type %u pin_len %u store_hint %u",
		adapter->dev_id, dst, ev->key.type, ev->key.pin_len,
		ev->store_hint);

	if (ev->key.pin_len > 16) {
		btd_error(adapter->dev_id,
				"Invalid PIN length (%u) in new_key event",
							ev->key.pin_len);
		return;
	}

	device = btd_adapter_get_device(adapter, &addr->bdaddr, addr->type);
	if (!device) {
		btd_error(adapter->dev_id,
				"Unable to get device object for %s", dst);
		return;
	}

	if (ev->store_hint) {
		const struct mgmt_link_key_info *key = &ev->key;

		store_link_key(adapter, device, key->val, key->type,
								key->pin_len);

		device_set_bonded(device, BDADDR_BREDR);
	}

	bonding_complete(adapter, &addr->bdaddr, addr->type, 0);
}

static void store_longtermkey(struct btd_adapter *adapter, const bdaddr_t *peer,
				uint8_t bdaddr_type, const unsigned char *key,
				uint8_t master, uint8_t authenticated,
				uint8_t enc_size, uint16_t ediv,
				uint64_t rand)
{
	const char *group = master ? "LongTermKey" : "SlaveLongTermKey";
	char device_addr[18];
	char filename[PATH_MAX];
	GKeyFile *key_file;
	char key_str[33];
	gsize length = 0;
	char *str;
	int i;

	if (master != 0x00 && master != 0x01) {
		error("Unsupported LTK type %u", master);
		return;
	}

	ba2str(peer, device_addr);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/info",
			btd_adapter_get_storage_dir(adapter), device_addr);
	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	/* Old files may contain this so remove it in case it exists */
	g_key_file_remove_key(key_file, "LongTermKey", "Master", NULL);

	for (i = 0; i < 16; i++)
		sprintf(key_str + (i * 2), "%2.2X", key[i]);

	g_key_file_set_string(key_file, group, "Key", key_str);

	g_key_file_set_integer(key_file, group, "Authenticated",
							authenticated);
	g_key_file_set_integer(key_file, group, "EncSize", enc_size);

	g_key_file_set_integer(key_file, group, "EDiv", ediv);
	g_key_file_set_uint64(key_file, group, "Rand", rand);

	create_file(filename, S_IRUSR | S_IWUSR);

	str = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, str, length, NULL);
	g_free(str);

	g_key_file_free(key_file);
}

static void new_long_term_key_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_new_long_term_key *ev = param;
	const struct mgmt_addr_info *addr = &ev->key.addr;
	struct btd_adapter *adapter = user_data;
	struct btd_device *device;
	bool persistent;
	char dst[18];

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small long term key event");
		return;
	}

	ba2str(&addr->bdaddr, dst);

	DBG("hci%u new LTK for %s type %u enc_size %u",
		adapter->dev_id, dst, ev->key.type, ev->key.enc_size);

	device = btd_adapter_get_device(adapter, &addr->bdaddr, addr->type);
	if (!device) {
		btd_error(adapter->dev_id,
				"Unable to get device object for %s", dst);
		return;
	}

	/*
	 * Some older kernel versions set store_hint for long term keys
	 * from resolvable and unresolvable random addresses, but there
	 * is no point in storing these. Next time around the device
	 * address will be invalid.
	 *
	 * So only for identity addresses (public and static random) use
	 * the store_hint as an indication if the long term key should
	 * be persistently stored.
	 *
	 */
	if (addr->type == BDADDR_LE_RANDOM &&
				(addr->bdaddr.b[5] & 0xc0) != 0xc0)
		persistent = false;
	else
		persistent = !!ev->store_hint;

	if (persistent) {
		const struct mgmt_ltk_info *key = &ev->key;
		uint16_t ediv;
		uint64_t rand;

		ediv = le16_to_cpu(key->ediv);
		rand = le64_to_cpu(key->rand);

		store_longtermkey(adapter, &key->addr.bdaddr,
					key->addr.type, key->val, key->master,
					key->type, key->enc_size, ediv, rand);

		device_set_bonded(device, addr->type);
	}

	device_set_ltk_enc_size(device, ev->key.enc_size);

	bonding_complete(adapter, &addr->bdaddr, addr->type, 0);
}

static void store_csrk(struct btd_adapter *adapter, const bdaddr_t *peer,
				uint8_t bdaddr_type, const unsigned char *key,
				uint32_t counter, uint8_t type)
{
	const char *group;
	char device_addr[18];
	char filename[PATH_MAX];
	GKeyFile *key_file;
	char key_str[33];
	gsize length = 0;
	gboolean auth;
	char *str;
	int i;

	switch (type) {
	case 0x00:
		group = "LocalSignatureKey";
		auth = FALSE;
		break;
	case 0x01:
		group = "RemoteSignatureKey";
		auth = FALSE;
		break;
	case 0x02:
		group = "LocalSignatureKey";
		auth = TRUE;
		break;
	case 0x03:
		group = "RemoteSignatureKey";
		auth = TRUE;
		break;
	default:
		warn("Unsupported CSRK type %u", type);
		return;
	}

	ba2str(peer, device_addr);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/info",
			btd_adapter_get_storage_dir(adapter), device_addr);

	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	for (i = 0; i < 16; i++)
		sprintf(key_str + (i * 2), "%2.2X", key[i]);

	g_key_file_set_string(key_file, group, "Key", key_str);
	g_key_file_set_integer(key_file, group, "Counter", counter);
	g_key_file_set_boolean(key_file, group, "Authenticated", auth);

	create_file(filename, S_IRUSR | S_IWUSR);

	str = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, str, length, NULL);
	g_free(str);

	g_key_file_free(key_file);
}

static void new_csrk_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_new_csrk *ev = param;
	const struct mgmt_addr_info *addr = &ev->key.addr;
	const struct mgmt_csrk_info *key = &ev->key;
	struct btd_adapter *adapter = user_data;
	struct btd_device *device;
	char dst[18];

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small CSRK event");
		return;
	}

	ba2str(&addr->bdaddr, dst);

	DBG("hci%u new CSRK for %s type %u", adapter->dev_id, dst,
								ev->key.type);

	device = btd_adapter_get_device(adapter, &addr->bdaddr, addr->type);
	if (!device) {
		btd_error(adapter->dev_id,
				"Unable to get device object for %s", dst);
		return;
	}

	if (!ev->store_hint)
		return;

	store_csrk(adapter, &key->addr.bdaddr, key->addr.type, key->val, 0,
								key->type);

	btd_device_set_temporary(device, false);
}

static void store_irk(struct btd_adapter *adapter, const bdaddr_t *peer,
				uint8_t bdaddr_type, const unsigned char *key)
{
	char device_addr[18];
	char filename[PATH_MAX];
	GKeyFile *key_file;
	char *store_data;
	char str[33];
	size_t length = 0;
	int i;

	ba2str(peer, device_addr);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/info",
			btd_adapter_get_storage_dir(adapter), device_addr);
	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	for (i = 0; i < 16; i++)
		sprintf(str + (i * 2), "%2.2X", key[i]);

	g_key_file_set_string(key_file, "IdentityResolvingKey", "Key", str);

	create_file(filename, S_IRUSR | S_IWUSR);

	store_data = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, store_data, length, NULL);
	g_free(store_data);

	g_key_file_free(key_file);
}

static void new_irk_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_new_irk *ev = param;
	const struct mgmt_addr_info *addr = &ev->key.addr;
	const struct mgmt_irk_info *irk = &ev->key;
	struct btd_adapter *adapter = user_data;
	struct btd_device *device, *duplicate;
	bool persistent;
	char dst[18], rpa[18];

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small New IRK event");
		return;
	}

	ba2str(&addr->bdaddr, dst);
	ba2str(&ev->rpa, rpa);

	DBG("hci%u new IRK for %s RPA %s", adapter->dev_id, dst, rpa);

	if (bacmp(&ev->rpa, BDADDR_ANY)) {
		device = btd_adapter_get_device(adapter, &ev->rpa,
							BDADDR_LE_RANDOM);
		duplicate = btd_adapter_find_device(adapter, &addr->bdaddr,
								addr->type);
		if (duplicate == device)
			duplicate = NULL;
	} else {
		device = btd_adapter_get_device(adapter, &addr->bdaddr,
								addr->type);
		duplicate = NULL;
	}

	if (!device) {
		btd_error(adapter->dev_id,
				"Unable to get device object for %s", dst);
		return;
	}

	device_update_addr(device, &addr->bdaddr, addr->type);

	if (duplicate)
		device_merge_duplicate(device, duplicate);

	persistent = !!ev->store_hint;
	if (!persistent)
		return;

	store_irk(adapter, &addr->bdaddr, addr->type, irk->val);

	btd_device_set_temporary(device, false);
}

static void store_conn_param(struct btd_adapter *adapter, const bdaddr_t *peer,
				uint8_t bdaddr_type, uint16_t min_interval,
				uint16_t max_interval, uint16_t latency,
				uint16_t timeout)
{
	char device_addr[18];
	char filename[PATH_MAX];
	GKeyFile *key_file;
	char *store_data;
	size_t length = 0;

	ba2str(peer, device_addr);

	DBG("");

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/info",
			btd_adapter_get_storage_dir(adapter), device_addr);
	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	g_key_file_set_integer(key_file, "ConnectionParameters",
						"MinInterval", min_interval);
	g_key_file_set_integer(key_file, "ConnectionParameters",
						"MaxInterval", max_interval);
	g_key_file_set_integer(key_file, "ConnectionParameters",
						"Latency", latency);
	g_key_file_set_integer(key_file, "ConnectionParameters",
						"Timeout", timeout);

	create_file(filename, S_IRUSR | S_IWUSR);

	store_data = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, store_data, length, NULL);
	g_free(store_data);

	g_key_file_free(key_file);
}

static void new_conn_param(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_new_conn_param *ev = param;
	struct btd_adapter *adapter = user_data;
	uint16_t min, max, latency, timeout;
	struct btd_device *dev;
	char dst[18];


	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id,
				"Too small New Connection Parameter event");
		return;
	}

	ba2str(&ev->addr.bdaddr, dst);

	min = btohs(ev->min_interval);
	max = btohs(ev->max_interval);
	latency = btohs(ev->latency);
	timeout = btohs(ev->timeout);

	DBG("hci%u %s (%u) min 0x%04x max 0x%04x latency 0x%04x timeout 0x%04x",
		adapter->dev_id, dst, ev->addr.type, min, max, latency, timeout);

	dev = btd_adapter_get_device(adapter, &ev->addr.bdaddr, ev->addr.type);
	if (!dev) {
		btd_error(adapter->dev_id,
				"Unable to get device object for %s", dst);
		return;
	}

	if (!ev->store_hint)
		return;

	store_conn_param(adapter, &ev->addr.bdaddr, ev->addr.type,
					ev->min_interval, ev->max_interval,
					ev->latency, ev->timeout);
}

int adapter_set_io_capability(struct btd_adapter *adapter, uint8_t io_cap)
{
	struct mgmt_cp_set_io_capability cp;

	if (!btd_opts.pairable) {
		if (io_cap == IO_CAPABILITY_INVALID) {
			if (adapter->current_settings & MGMT_SETTING_BONDABLE)
				set_mode(adapter, MGMT_OP_SET_BONDABLE, 0x00);

			return 0;
		}

		if (!(adapter->current_settings & MGMT_SETTING_BONDABLE))
			set_mode(adapter, MGMT_OP_SET_BONDABLE, 0x01);
	} else if (io_cap == IO_CAPABILITY_INVALID)
		io_cap = IO_CAPABILITY_NOINPUTNOOUTPUT;

	memset(&cp, 0, sizeof(cp));
	cp.io_capability = io_cap;

	if (mgmt_send(adapter->mgmt, MGMT_OP_SET_IO_CAPABILITY,
				adapter->dev_id, sizeof(cp), &cp,
				NULL, NULL, NULL) > 0)
		return 0;

	return -EIO;
}

int btd_adapter_add_remote_oob_data(struct btd_adapter *adapter,
					const bdaddr_t *bdaddr,
					uint8_t *hash, uint8_t *randomizer)
{
	struct mgmt_cp_add_remote_oob_data cp;
	char addr[18];

	ba2str(bdaddr, addr);
	DBG("hci%d bdaddr %s", adapter->dev_id, addr);

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, bdaddr);
	memcpy(cp.hash192, hash, 16);

	if (randomizer)
		memcpy(cp.rand192, randomizer, 16);

	if (mgmt_send(adapter->mgmt, MGMT_OP_ADD_REMOTE_OOB_DATA,
				adapter->dev_id, sizeof(cp), &cp,
				NULL, NULL, NULL) > 0)
		return 0;

	return -EIO;
}

int btd_adapter_remove_remote_oob_data(struct btd_adapter *adapter,
							const bdaddr_t *bdaddr)
{
	struct mgmt_cp_remove_remote_oob_data cp;
	char addr[18];

	ba2str(bdaddr, addr);
	DBG("hci%d bdaddr %s", adapter->dev_id, addr);

	memset(&cp, 0, sizeof(cp));
	bacpy(&cp.addr.bdaddr, bdaddr);

	if (mgmt_send(adapter->mgmt, MGMT_OP_REMOVE_REMOTE_OOB_DATA,
				adapter->dev_id, sizeof(cp), &cp,
				NULL, NULL, NULL) > 0)
		return 0;

	return -EIO;
}

bool btd_adapter_ssp_enabled(struct btd_adapter *adapter)
{
	if (adapter->current_settings & MGMT_SETTING_SSP)
		return true;

	return false;
}

void btd_adapter_set_oob_handler(struct btd_adapter *adapter,
						struct oob_handler *handler)
{
	adapter->oob_handler = handler;
}

gboolean btd_adapter_check_oob_handler(struct btd_adapter *adapter)
{
	return adapter->oob_handler != NULL;
}

static void read_local_oob_data_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_local_oob_data *rp = param;
	struct btd_adapter *adapter = user_data;
	const uint8_t *hash, *randomizer;

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
				"Read local OOB data failed: %s (0x%02x)",
						mgmt_errstr(status), status);
		hash = NULL;
		randomizer = NULL;
	} else if (length < sizeof(*rp)) {
		btd_error(adapter->dev_id,
				"Too small read local OOB data response");
		return;
	} else {
		hash = rp->hash192;
		randomizer = rp->rand192;
	}

	if (!adapter->oob_handler || !adapter->oob_handler->read_local_cb)
		return;

	adapter->oob_handler->read_local_cb(adapter, hash, randomizer,
					adapter->oob_handler->user_data);

	g_free(adapter->oob_handler);
	adapter->oob_handler = NULL;
}

int btd_adapter_read_local_oob_data(struct btd_adapter *adapter)
{
	DBG("hci%u", adapter->dev_id);

	if (mgmt_send(adapter->mgmt, MGMT_OP_READ_LOCAL_OOB_DATA,
			adapter->dev_id, 0, NULL, read_local_oob_data_complete,
			adapter, NULL) > 0)
		return 0;

	return -EIO;
}

void btd_adapter_for_each_device(struct btd_adapter *adapter,
			void (*cb)(struct btd_device *device, void *data),
			void *data)
{
	g_slist_foreach(adapter->devices, (GFunc) cb, data);
}

static int adapter_cmp(gconstpointer a, gconstpointer b)
{
	struct btd_adapter *adapter = (struct btd_adapter *) a;
	const bdaddr_t *bdaddr = b;

	return bacmp(&adapter->bdaddr, bdaddr);
}

static int adapter_id_cmp(gconstpointer a, gconstpointer b)
{
	struct btd_adapter *adapter = (struct btd_adapter *) a;
	uint16_t id = GPOINTER_TO_UINT(b);

	return adapter->dev_id == id ? 0 : -1;
}

struct btd_adapter *adapter_find(const bdaddr_t *sba)
{
	GSList *match;

	match = g_slist_find_custom(adapters, sba, adapter_cmp);
	if (!match)
		return NULL;

	return match->data;
}

struct btd_adapter *adapter_find_by_id(int id)
{
	GSList *match;

	match = g_slist_find_custom(adapters, GINT_TO_POINTER(id),
							adapter_id_cmp);
	if (!match)
		return NULL;

	return match->data;
}

void adapter_foreach(adapter_cb func, gpointer user_data)
{
	g_slist_foreach(adapters, (GFunc) func, user_data);
}

static int set_did(struct btd_adapter *adapter, uint16_t vendor,
			uint16_t product, uint16_t version, uint16_t source)
{
	struct mgmt_cp_set_device_id cp;

	DBG("hci%u source %x vendor %x product %x version %x",
			adapter->dev_id, source, vendor, product, version);

	memset(&cp, 0, sizeof(cp));

	cp.source = htobs(source);
	cp.vendor = htobs(vendor);
	cp.product = htobs(product);
	cp.version = htobs(version);

	if (mgmt_send(adapter->mgmt, MGMT_OP_SET_DEVICE_ID,
				adapter->dev_id, sizeof(cp), &cp,
				NULL, NULL, NULL) > 0)
		return 0;

	return -EIO;
}

static void services_modified(struct gatt_db_attribute *attrib, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	g_dbus_emit_property_changed(dbus_conn, adapter->path,
						ADAPTER_INTERFACE, "UUIDs");
}

static int adapter_register(struct btd_adapter *adapter)
{
	struct agent *agent;
	struct gatt_db *db;

	if (powering_down)
		return -EBUSY;

	adapter->path = g_strdup_printf("/org/bluez/hci%d", adapter->dev_id);

	if (!g_dbus_register_interface(dbus_conn,
					adapter->path, ADAPTER_INTERFACE,
					adapter_methods, NULL,
					adapter_properties, adapter,
					adapter_free)) {
		btd_error(adapter->dev_id,
				"Adapter interface init failed on path %s",
							adapter->path);
		g_free(adapter->path);
		adapter->path = NULL;
		return -EINVAL;
	}

	if (adapters == NULL)
		adapter->is_default = true;

	adapters = g_slist_append(adapters, adapter);

	agent = agent_get(NULL);
	if (agent) {
		uint8_t io_cap = agent_get_io_capability(agent);
		adapter_set_io_capability(adapter, io_cap);
		agent_unref(agent);
	}

	/* Don't start GATT database and advertising managers on
	 * non-LE controllers.
	 */
	if (!(adapter->supported_settings & MGMT_SETTING_LE) ||
					btd_opts.mode == BT_MODE_BREDR)
		goto load;

	adapter->database = btd_gatt_database_new(adapter);
	if (!adapter->database) {
		btd_error(adapter->dev_id,
				"Failed to create GATT database for adapter");
		adapters = g_slist_remove(adapters, adapter);
		return -EINVAL;
	}

	adapter->adv_manager = btd_adv_manager_new(adapter, adapter->mgmt);

	if (g_dbus_get_flags() & G_DBUS_FLAG_ENABLE_EXPERIMENTAL) {
		if (adapter->supported_settings & MGMT_SETTING_LE) {
			adapter->adv_monitor_manager =
				btd_adv_monitor_manager_create(adapter,
								adapter->mgmt);
			if (!adapter->adv_monitor_manager) {
				btd_error(adapter->dev_id,
						"Failed to create Adv Monitor "
						"Manager for adapter");
				return -EINVAL;
			}
		} else {
			btd_info(adapter->dev_id, "Adv Monitor Manager "
					"skipped, LE unavailable");
		}
	}

	if (g_dbus_get_flags() & G_DBUS_FLAG_ENABLE_EXPERIMENTAL) {
		adapter->battery_provider_manager =
			btd_battery_provider_manager_create(adapter);
	}

	db = btd_gatt_database_get_db(adapter->database);
	adapter->db_id = gatt_db_register(db, services_modified,
							services_modified,
							adapter, NULL);

load:
	mgmt_register(adapter->mgmt, MGMT_EV_DEVICE_FLAGS_CHANGED,
						adapter->dev_id,
						device_flags_changed_callback,
						adapter, NULL);

	load_config(adapter);
	fix_storage(adapter);
	load_drivers(adapter);
	btd_profile_foreach(probe_profile, adapter);
	clear_blocked(adapter);
	load_defaults(adapter);
	load_devices(adapter);

	/* restore Service Changed CCC value for bonded devices */
	btd_gatt_database_restore_svc_chng_ccc(adapter->database);

	/* retrieve the active connections: address the scenario where
	 * the are active connections before the daemon've started */
	if (btd_adapter_get_powered(adapter))
		load_connections(adapter);

	adapter->initialized = TRUE;

	if (btd_opts.did_source) {
		/* DeviceID record is added by sdpd-server before any other
		 * record is registered. */
		adapter_service_insert(adapter, sdp_record_find(0x10000));
		set_did(adapter, btd_opts.did_vendor, btd_opts.did_product,
				btd_opts.did_version, btd_opts.did_source);
	}

	DBG("Adapter %s registered", adapter->path);

	return 0;
}

static int adapter_unregister(struct btd_adapter *adapter)
{
	DBG("Unregister path: %s", adapter->path);

	adapters = g_slist_remove(adapters, adapter);

	if (adapter->is_default && adapters != NULL) {
		struct btd_adapter *new_default;

		new_default = adapter_find_by_id(hci_get_route(NULL));
		if (new_default == NULL)
			new_default = adapters->data;

		new_default->is_default = true;
	}

	adapter_list = g_list_remove(adapter_list, adapter);

	adapter_remove(adapter);
	btd_adapter_unref(adapter);

	return 0;
}

static void disconnected_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_device_disconnected *ev = param;
	struct btd_adapter *adapter = user_data;
	uint8_t reason;

	if (length < sizeof(struct mgmt_addr_info)) {
		btd_error(adapter->dev_id,
				"Too small device disconnected event");
		return;
	}

	if (length < sizeof(*ev))
		reason = MGMT_DEV_DISCONN_UNKNOWN;
	else
		reason = ev->reason;

	dev_disconnected(adapter, &ev->addr, reason);
}

static void connected_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_device_connected *ev = param;
	struct btd_adapter *adapter = user_data;
	struct btd_device *device;
	struct eir_data eir_data;
	uint16_t eir_len;
	char addr[18];
	bool name_known;
//	int conn_len;

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small device connected event");
		return;
	}

	eir_len = btohs(ev->eir_len);
	if (length < sizeof(*ev) + eir_len) {
		btd_error(adapter->dev_id, "Too small device connected event");
		return;
	}

/*
	conn_len = g_slist_length(adapter->connections);
	if (conn_len)
	{
		info( "Maximum user connection");
		btd_adapter_disconnect_device(adapter, &ev->addr.bdaddr, ev->addr.type);
		return;
	}
	else
	{
#if defined(RTCONFIG_LP5523)
		lp55xx_leds_proc(LP55XX_BTCOR_LEDS, LP55XX_ACT_SBLINK);
#endif
	}
*/
#if defined(RTCONFIG_PRELINK) 
	nvram_set_int("ble_dut_con", 1);
#endif
#if defined(RTCONFIG_LP5523)
	lp55xx_leds_proc(LP55XX_BTCOR_LEDS, LP55XX_ACT_SBLINK);
#elif defined(RTCONFIG_FIXED_BRIGHTNESS_RGBLED)
	set_rgbled(RGBLED_BT_CONNECT);
#endif

	ba2str(&ev->addr.bdaddr, addr);
	DBG("hci%u device %s connected eir_len %u!", index, addr, eir_len);

	device = btd_adapter_get_device(adapter, &ev->addr.bdaddr,
								ev->addr.type);
	if (!device) {
		btd_error(adapter->dev_id,
				"Unable to get device object for %s", addr);
		return;
	}

	memset(&eir_data, 0, sizeof(eir_data));
	if (eir_len > 0)
		eir_parse(&eir_data, ev->eir, eir_len);

	if (eir_data.class != 0)
		device_set_class(device, eir_data.class);

	adapter_add_connection(adapter, device, ev->addr.type);

	name_known = device_name_known(device);

	if (eir_data.name && (eir_data.name_complete || !name_known)) {
		device_store_cached_name(device, eir_data.name);
		btd_device_device_set_name(device, eir_data.name);
	}

	if (eir_data.msd_list)
		adapter_msd_notify(adapter, device, eir_data.msd_list);


	eir_data_free(&eir_data);
}

static void controller_resume_notify(struct btd_adapter *adapter)
{
	GSList *l;

	for (l = adapter->drivers; l; l = g_slist_next(l)) {
		struct btd_adapter_driver *driver = l->data;
		if (driver->resume)
			driver->resume(adapter);
	}
}

static void controller_resume_callback(uint16_t index, uint16_t length,
				       const void *param, void *user_data)
{
	const struct mgmt_ev_controller_resume *ev = param;
	struct btd_adapter *adapter = user_data;

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small device resume event");
		return;
	}

	info("Controller resume with wake event 0x%x", ev->wake_reason);

	controller_resume_notify(adapter);
}

static void device_blocked_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_device_blocked *ev = param;
	struct btd_adapter *adapter = user_data;
	struct btd_device *device;
	char addr[18];

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small device blocked event");
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);
	DBG("hci%u %s blocked", index, addr);

	device = btd_adapter_find_device(adapter, &ev->addr.bdaddr,
								ev->addr.type);
	if (device)
		device_block(device, TRUE);
}

static void device_unblocked_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_device_unblocked *ev = param;
	struct btd_adapter *adapter = user_data;
	struct btd_device *device;
	char addr[18];

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small device unblocked event");
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);
	DBG("hci%u %s unblocked", index, addr);

	device = btd_adapter_find_device(adapter, &ev->addr.bdaddr,
								ev->addr.type);
	if (device)
		device_unblock(device, FALSE, TRUE);
}

static void conn_fail_notify(struct btd_device *dev, uint8_t status)
{
	GSList *l;

	for (l = conn_fail_list; l; l = g_slist_next(l)) {
		btd_conn_fail_cb conn_fail_cb = l->data;
		conn_fail_cb(dev, status);
	}
}

void btd_add_conn_fail_cb(btd_conn_fail_cb func)
{
	conn_fail_list = g_slist_append(conn_fail_list, func);
}

void btd_remove_conn_fail_cb(btd_conn_fail_cb func)
{
	conn_fail_list = g_slist_remove(conn_fail_list, func);
}

static void connect_failed_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_connect_failed *ev = param;
	struct btd_adapter *adapter = user_data;
	struct btd_device *device;
	char addr[18];

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small connect failed event");
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);

	DBG("hci%u %s status %u", index, addr, ev->status);

	device = btd_adapter_find_device(adapter, &ev->addr.bdaddr,
								ev->addr.type);
	if (device) {
		conn_fail_notify(device, ev->status);

		/* If the device is in a bonding process cancel any auth request
		 * sent to the agent before proceeding, but keep the bonding
		 * request structure. */
		if (device_is_bonding(device, NULL))
			device_cancel_authentication(device, FALSE);
	}

	/* In the case of security mode 3 devices */
	bonding_attempt_complete(adapter, &ev->addr.bdaddr, ev->addr.type,
								ev->status);

	/* If the device is scheduled to retry the bonding wait until the retry
	 * happens. In other case, proceed with cancel the bondig.
	 */
	if (device && device_is_bonding(device, NULL)
					&& !device_is_retrying(device)) {
		device_cancel_authentication(device, TRUE);
		device_bonding_failed(device, ev->status);
	}

	/* In the case the bonding was canceled or did exists, remove the device
	 * when it is temporary. */
	if (device && !device_is_bonding(device, NULL)
						&& device_is_temporary(device))
		btd_adapter_remove_device(adapter, device);
}

static void remove_keys(struct btd_adapter *adapter,
					struct btd_device *device, uint8_t type)
{
	char device_addr[18];
	char filename[PATH_MAX];
	GKeyFile *key_file;
	gsize length = 0;
	char *str;

	ba2str(device_get_address(device), device_addr);

	snprintf(filename, PATH_MAX, STORAGEDIR "/%s/%s/info",
			btd_adapter_get_storage_dir(adapter), device_addr);
	key_file = g_key_file_new();
	g_key_file_load_from_file(key_file, filename, 0, NULL);

	if (type == BDADDR_BREDR) {
		g_key_file_remove_group(key_file, "LinkKey", NULL);
	} else {
		g_key_file_remove_group(key_file, "LongTermKey", NULL);
		g_key_file_remove_group(key_file, "LocalSignatureKey", NULL);
		g_key_file_remove_group(key_file, "RemoteSignatureKey", NULL);
		g_key_file_remove_group(key_file, "IdentityResolvingKey", NULL);
	}

	str = g_key_file_to_data(key_file, &length, NULL);
	g_file_set_contents(filename, str, length, NULL);
	g_free(str);

	g_key_file_free(key_file);
}

static void unpaired_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_ev_device_unpaired *ev = param;
	struct btd_adapter *adapter = user_data;
	struct btd_device *device;
	char addr[18];

	if (length < sizeof(*ev)) {
		btd_error(adapter->dev_id, "Too small device unpaired event");
		return;
	}

	ba2str(&ev->addr.bdaddr, addr);

	DBG("hci%u addr %s", index, addr);

	device = btd_adapter_find_device(adapter, &ev->addr.bdaddr,
								ev->addr.type);
	if (!device) {
		btd_warn(adapter->dev_id,
			"No device object for unpaired device %s", addr);
		return;
	}

	remove_keys(adapter, device, ev->addr.type);
	device_set_unpaired(device, ev->addr.type);
}

static void clear_devices_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	if (status != MGMT_STATUS_SUCCESS) {
		error("Failed to clear devices: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}
}

static int clear_devices(struct btd_adapter *adapter)
{
	struct mgmt_cp_remove_device cp;

	if (!btd_has_kernel_features(KERNEL_CONN_CONTROL))
		return 0;

	memset(&cp, 0, sizeof(cp));

	DBG("sending clear devices command for index %u", adapter->dev_id);

	if (mgmt_send(adapter->mgmt, MGMT_OP_REMOVE_DEVICE,
				adapter->dev_id, sizeof(cp), &cp,
				clear_devices_complete, adapter, NULL) > 0)
		return 0;

	btd_error(adapter->dev_id, "Failed to clear devices for index %u",
							adapter->dev_id);

	return -EIO;
}

static bool get_static_addr(struct btd_adapter *adapter)
{
	struct bt_crypto *crypto;
	GKeyFile *file;
	char **addrs;
	char mfg[7];
	char *str;
	bool ret;
	gsize len, i;

	snprintf(mfg, sizeof(mfg), "0x%04x", adapter->manufacturer);

	file = g_key_file_new();
	g_key_file_load_from_file(file, STORAGEDIR "/addresses", 0, NULL);
	addrs = g_key_file_get_string_list(file, "Static", mfg, &len, NULL);
	if (addrs) {
		for (i = 0; i < len; i++) {
			bdaddr_t addr;

			str2ba(addrs[i], &addr);
			if (adapter_find(&addr))
				continue;

			/* Usable address found in list */
			bacpy(&adapter->bdaddr, &addr);
			adapter->bdaddr_type = BDADDR_LE_RANDOM;
			ret = true;
			goto done;
		}

		len++;
		addrs = g_renew(char *, addrs, len + 1);
	} else {
		len = 1;
		addrs = g_new(char *, len + 1);
	}

	/* Initialize slot for new address */
	addrs[len - 1] = g_malloc(18);
	addrs[len] = NULL;

	crypto = bt_crypto_new();
	if (!crypto) {
		error("Failed to open crypto");
		ret = false;
		goto done;
	}

	ret = bt_crypto_random_bytes(crypto, &adapter->bdaddr,
						sizeof(adapter->bdaddr));
	if (!ret) {
		error("Failed to generate static address");
		bt_crypto_unref(crypto);
		goto done;
	}

	bt_crypto_unref(crypto);

	adapter->bdaddr.b[5] |= 0xc0;
	adapter->bdaddr_type = BDADDR_LE_RANDOM;

	ba2str(&adapter->bdaddr, addrs[len - 1]);

	g_key_file_set_string_list(file, "Static", mfg,
						(const char **)addrs, len);

	str = g_key_file_to_data(file, &len, NULL);
	g_file_set_contents(STORAGEDIR "/addresses", str, len, NULL);
	g_free(str);

	ret = true;

done:
	g_key_file_free(file);
	g_strfreev(addrs);

	return ret;
}

static bool set_static_addr(struct btd_adapter *adapter)
{
	struct mgmt_cp_set_static_address cp;

	/* dual-mode adapters must have a public address */
	if (adapter->supported_settings & MGMT_SETTING_BREDR)
		return false;

	if (!(adapter->supported_settings & MGMT_SETTING_LE))
		return false;

	DBG("Setting static address");

	if (!get_static_addr(adapter))
		return false;

	bacpy(&cp.bdaddr, &adapter->bdaddr);
	if (mgmt_send(adapter->mgmt, MGMT_OP_SET_STATIC_ADDRESS,
				adapter->dev_id, sizeof(cp), &cp,
				NULL, NULL, NULL) > 0) {
		return true;
	}

	return false;
}

static void set_blocked_keys_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
				"Failed to set blocked keys: %s (0x%02x)",
				mgmt_errstr(status), status);
		return;
	}

	DBG("Successfully set blocked keys for index %u", adapter->dev_id);
}

static bool set_blocked_keys(struct btd_adapter *adapter)
{
	uint8_t buffer[sizeof(struct mgmt_cp_set_blocked_keys) +
					sizeof(blocked_keys)] = { 0 };
	struct mgmt_cp_set_blocked_keys *cp =
				(struct mgmt_cp_set_blocked_keys *)buffer;
	int i;

	cp->key_count = ARRAY_SIZE(blocked_keys);
	for (i = 0; i < cp->key_count; ++i) {
		cp->keys[i].type = blocked_keys[i].type;
		memcpy(cp->keys[i].val, blocked_keys[i].val,
						sizeof(cp->keys[i].val));
	}

	return mgmt_send(mgmt_master, MGMT_OP_SET_BLOCKED_KEYS, adapter->dev_id,
						sizeof(buffer),	buffer,
						set_blocked_keys_complete,
						adapter, NULL);
}

static void read_exp_features_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	const struct mgmt_rp_read_exp_features_info *rp = param;
	size_t feature_count = 0;
	size_t i = 0;

	DBG("index %u status 0x%02x", adapter->dev_id, status);

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
				"Failed to read exp features info: %s (0x%02x)",
				mgmt_errstr(status), status);
		return;
	}

	if (length < sizeof(*rp)) {
		btd_error(adapter->dev_id, "Response too small");
		return;
	}

	feature_count = le16_to_cpu(rp->feature_count);
	for (i = 0; i < feature_count; ++i) {

		/* 671b10b5-42c0-4696-9227-eb28d1b049d6 */
		static const uint8_t le_simult_central_peripheral[16] = {
			0xd6, 0x49, 0xb0, 0xd1, 0x28, 0xeb, 0x27, 0x92,
			0x96, 0x46, 0xc0, 0x42, 0xb5, 0x10, 0x1b, 0x67,
		};

		if (memcmp(rp->features[i].uuid, le_simult_central_peripheral,
				sizeof(le_simult_central_peripheral)) == 0) {
			uint32_t flags = le32_to_cpu(rp->features[i].flags);

			adapter->le_simult_roles_supported = flags & 0x01;
		}
	}
}

static void read_exp_features(struct btd_adapter *adapter)
{
	if (mgmt_send(adapter->mgmt, MGMT_OP_READ_EXP_FEATURES_INFO,
			adapter->dev_id, 0, NULL, read_exp_features_complete,
			adapter, NULL) > 0)
		return;

	btd_error(adapter->dev_id, "Failed to read exp features info");
}

static void read_info_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct btd_adapter *adapter = user_data;
	const struct mgmt_rp_read_info *rp = param;
	uint32_t missing_settings;
	int err;

	DBG("index %u status 0x%02x", adapter->dev_id, status);

	if (status != MGMT_STATUS_SUCCESS) {
		btd_error(adapter->dev_id,
				"Failed to read info for index %u: %s (0x%02x)",
				adapter->dev_id, mgmt_errstr(status), status);
		goto failed;
	}

	if (length < sizeof(*rp)) {
		btd_error(adapter->dev_id,
				"Too small read info complete response");
		goto failed;
	}

	/*
	 * Store controller information for class of device, device
	 * name, short name and settings.
	 *
	 * During the lifetime of the controller these will be updated by
	 * events and the information is required to keep the current
	 * state of the controller.
	 */
	adapter->dev_class = rp->dev_class[0] | (rp->dev_class[1] << 8) |
						(rp->dev_class[2] << 16);
	adapter->name = g_strdup((const char *) rp->name);
	adapter->short_name = g_strdup((const char *) rp->short_name);

	adapter->manufacturer = btohs(rp->manufacturer);

	adapter->supported_settings = btohl(rp->supported_settings);
	adapter->current_settings = btohl(rp->current_settings);

	clear_uuids(adapter);
	clear_devices(adapter);

	if (bacmp(&rp->bdaddr, BDADDR_ANY) == 0) {
		if (!set_static_addr(adapter)) {
			btd_error(adapter->dev_id,
					"No Bluetooth address for index %u",
					adapter->dev_id);
			goto failed;
		}
	} else {
		bacpy(&adapter->bdaddr, &rp->bdaddr);
		if (!(adapter->supported_settings & MGMT_SETTING_LE))
			adapter->bdaddr_type = BDADDR_BREDR;
		else
			adapter->bdaddr_type = BDADDR_LE_PUBLIC;
	}

	missing_settings = adapter->current_settings ^
						adapter->supported_settings;

	switch (btd_opts.mode) {
	case BT_MODE_DUAL:
		if (missing_settings & MGMT_SETTING_SSP)
			set_mode(adapter, MGMT_OP_SET_SSP, 0x01);
		if (missing_settings & MGMT_SETTING_LE)
			set_mode(adapter, MGMT_OP_SET_LE, 0x01);
		if (missing_settings & MGMT_SETTING_BREDR)
			set_mode(adapter, MGMT_OP_SET_BREDR, 0x01);
		break;
	case BT_MODE_BREDR:
		if (!(adapter->supported_settings & MGMT_SETTING_BREDR)) {
			btd_error(adapter->dev_id,
				"Ignoring adapter withouth BR/EDR support");
			goto failed;
		}

		if (missing_settings & MGMT_SETTING_SSP)
			set_mode(adapter, MGMT_OP_SET_SSP, 0x01);
		if (missing_settings & MGMT_SETTING_BREDR)
			set_mode(adapter, MGMT_OP_SET_BREDR, 0x01);
		if (adapter->current_settings & MGMT_SETTING_LE)
			set_mode(adapter, MGMT_OP_SET_LE, 0x00);
		break;
	case BT_MODE_LE:
		if (!(adapter->supported_settings & MGMT_SETTING_LE)) {
			btd_error(adapter->dev_id,
				"Ignoring adapter withouth LE support");
			goto failed;
		}

		DBG("%s mode, \n missing_settings:0x%08x MGMT_SETTING_LE:0x%08x, \ncurrent_settings:0x%08x MGMT_SETTING_BREDR:0x%08x",
				btd_opts.mode==0?"DUEL":btd_opts.mode==1?"BREDR":"LE",
				missing_settings, MGMT_SETTING_LE,
				adapter->current_settings, MGMT_SETTING_BREDR);
		if (missing_settings & MGMT_SETTING_LE)
			set_mode(adapter, MGMT_OP_SET_LE, 0x01);
		if (adapter->current_settings & MGMT_SETTING_BREDR)
			set_mode(adapter, MGMT_OP_SET_BREDR, 0x00);
		break;
	}

#if !defined(RTCONFIG_QCA) /* Disable SECURE_CONN setting */
	if (missing_settings & MGMT_SETTING_SECURE_CONN)
		set_mode(adapter, MGMT_OP_SET_SECURE_CONN, 0x01);
#endif

	if (adapter->supported_settings & MGMT_SETTING_PRIVACY)
		set_privacy(adapter, btd_opts.privacy);

	if (btd_opts.fast_conn &&
			(missing_settings & MGMT_SETTING_FAST_CONNECTABLE))
		set_mode(adapter, MGMT_OP_SET_FAST_CONNECTABLE, 0x01);

	if (btd_has_kernel_features(KERNEL_EXP_FEATURES))
		read_exp_features(adapter);

	err = adapter_register(adapter);
	if (err < 0) {
		btd_error(adapter->dev_id, "Unable to register new adapter");
		goto failed;
	}

	/*
	 * Register all event notification handlers for controller.
	 *
	 * The handlers are registered after a succcesful read of the
	 * controller info. From now on they can track updates and
	 * notifications.
	 */
	mgmt_register(adapter->mgmt, MGMT_EV_NEW_SETTINGS, adapter->dev_id,
					new_settings_callback, adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_CLASS_OF_DEV_CHANGED,
						adapter->dev_id,
						dev_class_changed_callback,
						adapter, NULL);
	mgmt_register(adapter->mgmt, MGMT_EV_LOCAL_NAME_CHANGED,
						adapter->dev_id,
						local_name_changed_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_DISCOVERING,
						adapter->dev_id,
						discovering_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_DEVICE_FOUND,
						adapter->dev_id,
						device_found_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_DEVICE_DISCONNECTED,
						adapter->dev_id,
						disconnected_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_DEVICE_CONNECTED,
						adapter->dev_id,
						connected_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_CONNECT_FAILED,
						adapter->dev_id,
						connect_failed_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_DEVICE_UNPAIRED,
						adapter->dev_id,
						unpaired_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_AUTH_FAILED,
						adapter->dev_id,
						auth_failed_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_NEW_LINK_KEY,
						adapter->dev_id,
						new_link_key_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_NEW_LONG_TERM_KEY,
						adapter->dev_id,
						new_long_term_key_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_NEW_CSRK,
						adapter->dev_id,
						new_csrk_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_NEW_IRK,
						adapter->dev_id,
						new_irk_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_NEW_CONN_PARAM,
						adapter->dev_id,
						new_conn_param,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_DEVICE_BLOCKED,
						adapter->dev_id,
						device_blocked_callback,
						adapter, NULL);
	mgmt_register(adapter->mgmt, MGMT_EV_DEVICE_UNBLOCKED,
						adapter->dev_id,
						device_unblocked_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_PIN_CODE_REQUEST,
						adapter->dev_id,
						pin_code_request_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_USER_CONFIRM_REQUEST,
						adapter->dev_id,
						user_confirm_request_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_USER_PASSKEY_REQUEST,
						adapter->dev_id,
						user_passkey_request_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_PASSKEY_NOTIFY,
						adapter->dev_id,
						user_passkey_notify_callback,
						adapter, NULL);

	mgmt_register(adapter->mgmt, MGMT_EV_CONTROLLER_RESUME,
						adapter->dev_id,
						controller_resume_callback,
						adapter, NULL);

	set_dev_class(adapter);

	set_name(adapter, btd_adapter_get_name(adapter));

	if (btd_has_kernel_features(KERNEL_BLOCKED_KEYS_SUPPORTED) &&
	    !set_blocked_keys(adapter)) {
		btd_error(adapter->dev_id,
				"Failed to set blocked keys for index %u",
				adapter->dev_id);
		goto failed;
	}

	if (btd_opts.pairable &&
			!(adapter->current_settings & MGMT_SETTING_BONDABLE))
		set_mode(adapter, MGMT_OP_SET_BONDABLE, 0x01);

	if (!btd_has_kernel_features(KERNEL_CONN_CONTROL))
		set_mode(adapter, MGMT_OP_SET_CONNECTABLE, 0x01);
	else if (adapter->current_settings & MGMT_SETTING_CONNECTABLE)
		set_mode(adapter, MGMT_OP_SET_CONNECTABLE, 0x00);

#if 0	/*Enable discoverable*/
	if (adapter->stored_discoverable && !adapter->discoverable_timeout)
#endif
		set_discoverable(adapter, 0x01, 0);

	if (btd_adapter_get_powered(adapter))
		adapter_start(adapter);

	return;

failed:
	/*
	 * Remove adapter from list in case of a failure.
	 *
	 * Leaving an adapter structure around for a controller that can
	 * not be initilized makes no sense at the moment.
	 *
	 * This is a simplification to avoid constant checks if the
	 * adapter is ready to do anything.
	 */
	adapter_list = g_list_remove(adapter_list, adapter);

	btd_adapter_unref(adapter);
}

static void reset_adv_monitors_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_remove_adv_monitor *rp = param;

	if (status != MGMT_STATUS_SUCCESS) {
		error("Failed to reset Adv Monitors: %s (0x%02x)",
			mgmt_errstr(status), status);
		return;
	}

	if (length < sizeof(*rp)) {
		error("Wrong size of remove Adv Monitor response for reset "
			"all Adv Monitors");
		return;
	}

	DBG("Removed all Adv Monitors");
}

static void reset_adv_monitors(uint16_t index)
{
	struct mgmt_cp_remove_adv_monitor cp;

	DBG("sending remove Adv Monitor command with handle 0");

	/* Handle 0 indicates to remove all */
	cp.monitor_handle = 0;
	if (mgmt_send(mgmt_master, MGMT_OP_REMOVE_ADV_MONITOR, index,
			sizeof(cp), &cp, reset_adv_monitors_complete, NULL,
			NULL) > 0) {
		return;
	}

	error("Failed to reset Adv Monitors");
}

static void index_added(uint16_t index, uint16_t length, const void *param,
							void *user_data)
{
	struct btd_adapter *adapter;

	DBG("index %u", index);

	adapter = btd_adapter_lookup(index);
	if (adapter) {
		btd_warn(adapter->dev_id,
			"Ignoring index added for an already existing adapter");
		return;
	}

#if 0 // ASUS
	reset_adv_monitors(index);
#endif

	adapter = btd_adapter_new(index);
	if (!adapter) {
		btd_error(index,
			"Unable to create new adapter for index %u", index);
		return;
	}

	/*
	 * Protect against potential two executions of read controller info.
	 *
	 * In case the start of the daemon and the action of adding a new
	 * controller coincide this function might be called twice.
	 *
	 * To avoid the double execution of reading the controller info,
	 * add the adapter already to the list. If an adapter is already
	 * present, the second notification will cause a warning. If the
	 * command fails the adapter is removed from the list again.
	 */
	adapter_list = g_list_append(adapter_list, adapter);

	DBG("sending read info command for index %u", index);

	if (mgmt_send(mgmt_master, MGMT_OP_READ_INFO, index, 0, NULL,
					read_info_complete, adapter, NULL) > 0)
		return;

	btd_error(adapter->dev_id,
			"Failed to read controller info for index %u", index);

	adapter_list = g_list_remove(adapter_list, adapter);

	btd_adapter_unref(adapter);
}

static void index_removed(uint16_t index, uint16_t length, const void *param,
							void *user_data)
{
	struct btd_adapter *adapter;

	DBG("index %u", index);

	adapter = btd_adapter_lookup(index);
	if (!adapter) {
		warn("Ignoring index removal for a non-existent adapter");
		return;
	}

	adapter_unregister(adapter);
}

static void read_index_list_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_index_list *rp = param;
	uint16_t num;
	int i;

	if (status != MGMT_STATUS_SUCCESS) {
		error("Failed to read index list: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	if (length < sizeof(*rp)) {
		error("Wrong size of read index list response");
		return;
	}

	num = btohs(rp->num_controllers);

	DBG("Number of controllers: %d", num);

	if (num * sizeof(uint16_t) + sizeof(*rp) != length) {
		error("Incorrect packet size for index list response");
		return;
	}

	for (i = 0; i < num; i++) {
		uint16_t index;

		index = btohs(rp->index[i]);

		DBG("Found index %u", index);

		/*
		 * Pretend to be index added event notification.
		 *
		 * It is safe to just trigger the procedure for index
		 * added notification. It does check against itself.
		 */
		index_added(index, 0, NULL, NULL);
	}
}

static void read_commands_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_commands *rp = param;
	uint16_t num_commands, num_events;
	size_t expected_len;
	int i;

	if (status != MGMT_STATUS_SUCCESS) {
		error("Failed to read supported commands: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	if (length < sizeof(*rp)) {
		error("Wrong size of read commands response");
		return;
	}

	num_commands = btohs(rp->num_commands);
	num_events = btohs(rp->num_events);

	DBG("Number of commands: %d", num_commands);
	DBG("Number of events: %d", num_events);

	expected_len = sizeof(*rp) + num_commands * sizeof(uint16_t) +
						num_events * sizeof(uint16_t);

	if (length < expected_len) {
		error("Too small reply for supported commands: (%u != %zu)",
							length, expected_len);
		return;
	}

	for (i = 0; i < num_commands; i++) {
		uint16_t op = get_le16(rp->opcodes + i);

		switch (op) {
		case MGMT_OP_ADD_DEVICE:
			DBG("enabling kernel-side connection control");
			kernel_features |= KERNEL_CONN_CONTROL;
			break;
		case MGMT_OP_SET_BLOCKED_KEYS:
			DBG("kernel supports the set_blocked_keys op");
			kernel_features |= KERNEL_BLOCKED_KEYS_SUPPORTED;
			break;
		case MGMT_OP_SET_DEF_SYSTEM_CONFIG:
			DBG("kernel supports set system confic");
			kernel_features |= KERNEL_SET_SYSTEM_CONFIG;
			break;
		case MGMT_OP_READ_EXP_FEATURES_INFO:
			DBG("kernel supports exp features");
			kernel_features |= KERNEL_EXP_FEATURES;
			break;
		case MGMT_OP_ADD_EXT_ADV_PARAMS:
			DBG("kernel supports ext adv commands");
			kernel_features |= KERNEL_HAS_EXT_ADV_ADD_CMDS;
			break;
		case MGMT_OP_READ_CONTROLLER_CAP:
			DBG("kernel supports controller cap command");
			kernel_features |= KERNEL_HAS_CONTROLLER_CAP_CMD;
			break;
		default:
			break;
		}
	}

	for (i = 0; i < num_events; i++) {
		uint16_t ev = get_le16(rp->opcodes + num_commands + i);

		switch(ev) {
		case MGMT_EV_CONTROLLER_RESUME:
			DBG("kernel supports suspend/resume events");
			kernel_features |= KERNEL_HAS_RESUME_EVT;
			break;
		}
	}
}

static void read_version_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	const struct mgmt_rp_read_version *rp = param;

	if (status != MGMT_STATUS_SUCCESS) {
		error("Failed to read version information: %s (0x%02x)",
						mgmt_errstr(status), status);
		return;
	}

	if (length < sizeof(*rp)) {
		error("Wrong size of read version response");
		return;
	}

	mgmt_version = rp->version;
	mgmt_revision = btohs(rp->revision);

	info("Bluetooth management interface %u.%u initialized",
						mgmt_version, mgmt_revision);

	if (mgmt_version < 1) {
		error("Version 1.0 or later of management interface required");
		abort();
	}

	DBG("sending read supported commands command");

	/*
	 * It is irrelevant if this command succeeds or fails. In case of
	 * failure safe settings are assumed.
	 */
	mgmt_send(mgmt_master, MGMT_OP_READ_COMMANDS,
				MGMT_INDEX_NONE, 0, NULL,
				read_commands_complete, NULL, NULL);

	mgmt_register(mgmt_master, MGMT_EV_INDEX_ADDED, MGMT_INDEX_NONE,
						index_added, NULL, NULL);
	mgmt_register(mgmt_master, MGMT_EV_INDEX_REMOVED, MGMT_INDEX_NONE,
						index_removed, NULL, NULL);

	DBG("sending read index list command");

	if (mgmt_send(mgmt_master, MGMT_OP_READ_INDEX_LIST,
				MGMT_INDEX_NONE, 0, NULL,
				read_index_list_complete, NULL, NULL) > 0)
		return;

	error("Failed to read controller index list");
}

static void mgmt_debug(const char *str, void *user_data)
{
	const char *prefix = user_data;

	info("%s%s", prefix, str);
}

int adapter_init(void)
{
	dbus_conn = btd_get_dbus_connection();

	mgmt_master = mgmt_new_default();
	if (!mgmt_master) {
		error("Failed to access management interface");
		return -EIO;
	}

	if (getenv("MGMT_DEBUG"))
		mgmt_set_debug(mgmt_master, mgmt_debug, "mgmt: ", NULL);

	DBG("sending read version command");

	if (mgmt_send(mgmt_master, MGMT_OP_READ_VERSION,
				MGMT_INDEX_NONE, 0, NULL,
				read_version_complete, NULL, NULL) > 0)
		return 0;

	error("Failed to read management version information");

	return -EIO;
}

void adapter_cleanup(void)
{
	g_list_free(adapter_list);

	while (adapters) {
		struct btd_adapter *adapter = adapters->data;

		adapter_remove(adapter);
		adapters = g_slist_remove(adapters, adapter);
		btd_adapter_unref(adapter);
	}

	/*
	 * In case there is another reference active, clear out
	 * registered handlers for index added and index removed.
	 *
	 * This is just an extra precaution to be safe, and in
	 * reality should not make a difference.
	 */
	mgmt_unregister_index(mgmt_master, MGMT_INDEX_NONE);

	/*
	 * In case there is another reference active, cancel
	 * all pending global commands.
	 *
	 * This is just an extra precaution to avoid callbacks
	 * that potentially then could leak memory or access
	 * an invalid structure.
	 */
	mgmt_cancel_index(mgmt_master, MGMT_INDEX_NONE);

	mgmt_unref(mgmt_master);
	mgmt_master = NULL;

	dbus_conn = NULL;
}

void adapter_shutdown(void)
{
	GList *list;

	DBG("");

	powering_down = true;

	for (list = g_list_first(adapter_list); list;
						list = g_list_next(list)) {
		struct btd_adapter *adapter = list->data;

		if (!(adapter->current_settings & MGMT_SETTING_POWERED))
			continue;

		clear_discoverable(adapter);
		remove_temporary_devices(adapter);
		set_mode(adapter, MGMT_OP_SET_POWERED, 0x00);

		adapter_remaining++;
	}

	if (!adapter_remaining)
		btd_exit();
}

/*
 * Check if workaround for broken ATT server socket behavior is needed
 * where we need to connect an ATT client socket before pairing to get
 * early access to the ATT channel.
 */
bool btd_le_connect_before_pairing(void)
{
	if (MGMT_VERSION(mgmt_version, mgmt_revision) < MGMT_VERSION(1, 4))
		return true;

	return false;
}

bool btd_has_kernel_features(uint32_t features)
{
	return (kernel_features & features) ? true : false;
}
