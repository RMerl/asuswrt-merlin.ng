/*
 * Copyright (C) 2013 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include <cutils/properties.h>

#include "hal.h"
#include "hal-utils.h"

/*
 * converts uuid to string
 * buf should be at least 39 bytes
 *
 * returns string representation of uuid
 */
const char *bt_uuid_t2str(const uint8_t *uuid, char *buf)
{
	int shift = 0;
	unsigned int i;
	int is_bt;

	if (!uuid)
		return strcpy(buf, "NULL");

	is_bt = !memcmp(&uuid[4], &BT_BASE_UUID[4], HAL_UUID_LEN - 4);

	for (i = 0; i < HAL_UUID_LEN; i++) {
		if (i == 4 && is_bt)
			break;

		if (i == 4 || i == 6 || i == 8 || i == 10) {
			buf[i * 2 + shift] = '-';
			shift++;
		}
		sprintf(buf + i * 2 + shift, "%02x", uuid[i]);
	}

	return buf;
}

const char *btuuid2str(const uint8_t *uuid)
{
	static char buf[MAX_UUID_STR_LEN];

	return bt_uuid_t2str(uuid, buf);
}

INTMAP(bt_status_t, -1, "(unknown)")
	DELEMENT(BT_STATUS_SUCCESS),
	DELEMENT(BT_STATUS_FAIL),
	DELEMENT(BT_STATUS_NOT_READY),
	DELEMENT(BT_STATUS_NOMEM),
	DELEMENT(BT_STATUS_BUSY),
	DELEMENT(BT_STATUS_DONE),
	DELEMENT(BT_STATUS_UNSUPPORTED),
	DELEMENT(BT_STATUS_PARM_INVALID),
	DELEMENT(BT_STATUS_UNHANDLED),
	DELEMENT(BT_STATUS_AUTH_FAILURE),
	DELEMENT(BT_STATUS_RMT_DEV_DOWN),
ENDMAP

INTMAP(bt_state_t, -1, "(unknown)")
	DELEMENT(BT_STATE_OFF),
	DELEMENT(BT_STATE_ON),
ENDMAP

INTMAP(bt_device_type_t, -1, "(unknown)")
	DELEMENT(BT_DEVICE_DEVTYPE_BREDR),
	DELEMENT(BT_DEVICE_DEVTYPE_BLE),
	DELEMENT(BT_DEVICE_DEVTYPE_DUAL),
ENDMAP

INTMAP(bt_scan_mode_t, -1, "(unknown)")
	DELEMENT(BT_SCAN_MODE_NONE),
	DELEMENT(BT_SCAN_MODE_CONNECTABLE),
	DELEMENT(BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE),
ENDMAP

INTMAP(bt_discovery_state_t, -1, "(unknown)")
	DELEMENT(BT_DISCOVERY_STOPPED),
	DELEMENT(BT_DISCOVERY_STARTED),
ENDMAP

INTMAP(bt_acl_state_t, -1, "(unknown)")
	DELEMENT(BT_ACL_STATE_CONNECTED),
	DELEMENT(BT_ACL_STATE_DISCONNECTED),
ENDMAP

INTMAP(bt_bond_state_t, -1, "(unknown)")
	DELEMENT(BT_BOND_STATE_NONE),
	DELEMENT(BT_BOND_STATE_BONDING),
	DELEMENT(BT_BOND_STATE_BONDED),
ENDMAP

INTMAP(bt_ssp_variant_t, -1, "(unknown)")
	DELEMENT(BT_SSP_VARIANT_PASSKEY_CONFIRMATION),
	DELEMENT(BT_SSP_VARIANT_PASSKEY_ENTRY),
	DELEMENT(BT_SSP_VARIANT_CONSENT),
	DELEMENT(BT_SSP_VARIANT_PASSKEY_NOTIFICATION),
ENDMAP

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
INTMAP(bt_property_type_t, -1, "(unknown)")
	DELEMENT(BT_PROPERTY_BDNAME),
	DELEMENT(BT_PROPERTY_BDADDR),
	DELEMENT(BT_PROPERTY_UUIDS),
	DELEMENT(BT_PROPERTY_CLASS_OF_DEVICE),
	DELEMENT(BT_PROPERTY_TYPE_OF_DEVICE),
	DELEMENT(BT_PROPERTY_SERVICE_RECORD),
	DELEMENT(BT_PROPERTY_ADAPTER_SCAN_MODE),
	DELEMENT(BT_PROPERTY_ADAPTER_BONDED_DEVICES),
	DELEMENT(BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT),
	DELEMENT(BT_PROPERTY_REMOTE_FRIENDLY_NAME),
	DELEMENT(BT_PROPERTY_REMOTE_RSSI),
	DELEMENT(BT_PROPERTY_REMOTE_VERSION_INFO),
	DELEMENT(BT_PROPERTY_LOCAL_LE_FEATURES),
	DELEMENT(BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP),
ENDMAP
#else
INTMAP(bt_property_type_t, -1, "(unknown)")
	DELEMENT(BT_PROPERTY_BDNAME),
	DELEMENT(BT_PROPERTY_BDADDR),
	DELEMENT(BT_PROPERTY_UUIDS),
	DELEMENT(BT_PROPERTY_CLASS_OF_DEVICE),
	DELEMENT(BT_PROPERTY_TYPE_OF_DEVICE),
	DELEMENT(BT_PROPERTY_SERVICE_RECORD),
	DELEMENT(BT_PROPERTY_ADAPTER_SCAN_MODE),
	DELEMENT(BT_PROPERTY_ADAPTER_BONDED_DEVICES),
	DELEMENT(BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT),
	DELEMENT(BT_PROPERTY_REMOTE_FRIENDLY_NAME),
	DELEMENT(BT_PROPERTY_REMOTE_RSSI),
	DELEMENT(BT_PROPERTY_REMOTE_VERSION_INFO),
	DELEMENT(BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP),
ENDMAP
#endif

INTMAP(bt_cb_thread_evt, -1, "(unknown)")
	DELEMENT(ASSOCIATE_JVM),
	DELEMENT(DISASSOCIATE_JVM),
ENDMAP

/* Find first index of given value in table m */
int int2str_findint(int v, const struct int2str m[])
{
	int i;

	for (i = 0; m[i].str; ++i) {
		if (m[i].val == v)
			return i;
	}
	return -1;
}

/* Find first index of given string in table m */
int int2str_findstr(const char *str, const struct int2str m[])
{
	int i;

	for (i = 0; m[i].str; ++i) {
		if (strcmp(m[i].str, str) == 0)
			return i;
	}
	return -1;
}

/*
 * convert bd_addr to string
 * buf must be at least 18 char long
 *
 * returns buf
 */
const char *bt_bdaddr_t2str(const bt_bdaddr_t *bd_addr, char *buf)
{
	const uint8_t *p;

	if (!bd_addr)
		return strcpy(buf, "NULL");

	p = bd_addr->address;

	snprintf(buf, MAX_ADDR_STR_LEN, "%02x:%02x:%02x:%02x:%02x:%02x",
					p[0], p[1], p[2], p[3], p[4], p[5]);

	return buf;
}

/* converts string to bt_bdaddr_t */
void str2bt_bdaddr_t(const char *str, bt_bdaddr_t *bd_addr)
{
	uint8_t *p = bd_addr->address;

	sscanf(str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
				&p[0], &p[1], &p[2], &p[3], &p[4], &p[5]);
}

/* converts string to uuid */
void str2bt_uuid_t(const char *str, bt_uuid_t *uuid)
{
	int i = 0;

	memcpy(uuid, BT_BASE_UUID, sizeof(bt_uuid_t));

	while (*str && i < (int) sizeof(bt_uuid_t)) {
		while (*str == '-')
			str++;

		if (sscanf(str, "%02hhx", &uuid->uu[i]) != 1)
			break;

		i++;
		str += 2;
	}
}

const char *enum_defines(void *v, int i)
{
	const struct int2str *m = v;

	return m[i].str != NULL ? m[i].str : NULL;
}

const char *enum_strings(void *v, int i)
{
	const char **m = v;

	return m[i] != NULL ? m[i] : NULL;
}

const char *enum_one_string(void *v, int i)
{
	const char *m = v;

	return (i == 0) && (m[0] != 0) ? m : NULL;
}

const char *bdaddr2str(const bt_bdaddr_t *bd_addr)
{
	static char buf[MAX_ADDR_STR_LEN];

	return bt_bdaddr_t2str(bd_addr, buf);
}

static void bonded_devices2string(char *str, void *prop, int prop_len)
{
	int count = prop_len / sizeof(bt_bdaddr_t);
	bt_bdaddr_t *addr = prop;

	strcat(str, "{");

	while (count--) {
		strcat(str, bdaddr2str(addr));
		if (count)
			strcat(str, ", ");
		addr++;
	}

	strcat(str, "}");
}

static void uuids2string(char *str, void *prop, int prop_len)
{
	int count = prop_len / sizeof(bt_uuid_t);
	bt_uuid_t *uuid = prop;

	strcat(str, "{");

	while (count--) {
		strcat(str, btuuid2str(uuid->uu));
		if (count)
			strcat(str, ", ");
		uuid++;
	}

	strcat(str, "}");
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void local_le_feat2string(char *str, const bt_local_le_features_t *f)
{
	uint16_t scan_num;

	str += sprintf(str, "{\n");

	str += sprintf(str, "Privacy supported: %s,\n",
				f->local_privacy_enabled ? "TRUE" : "FALSE");

	str += sprintf(str, "Num of advertising instances: %u,\n",
							f->max_adv_instance);

	str += sprintf(str, "PRA offloading support: %s,\n",
				f->rpa_offload_supported ? "TRUE" : "FALSE");

	str += sprintf(str, "Num of offloaded IRKs: %u,\n",
							f->max_irk_list_size);

	str += sprintf(str, "Num of offloaded scan filters: %u,\n",
						f->max_adv_filter_supported);

	scan_num = (f->scan_result_storage_size_hibyte << 8) +
					f->scan_result_storage_size_lobyte;

	str += sprintf(str, "Num of offloaded scan results: %u,\n", scan_num);

	str += sprintf(str, "Activity & energy report support: %s\n",
			f->activity_energy_info_supported ? "TRUE" : "FALSE");

	sprintf(str, "}");
}
#endif

const char *btproperty2str(const bt_property_t *property)
{
	bt_service_record_t *rec;
	static char buf[4096];
	char *p;

	p = buf + sprintf(buf, "type=%s len=%d val=",
					bt_property_type_t2str(property->type),
					property->len);

	switch (property->type) {
	case BT_PROPERTY_BDNAME:
	case BT_PROPERTY_REMOTE_FRIENDLY_NAME:
		snprintf(p, property->len + 1, "%s",
					((bt_bdname_t *) property->val)->name);
		break;
	case BT_PROPERTY_BDADDR:
		sprintf(p, "%s", bdaddr2str((bt_bdaddr_t *) property->val));
		break;
	case BT_PROPERTY_CLASS_OF_DEVICE:
		sprintf(p, "%06x", *((int *) property->val));
		break;
	case BT_PROPERTY_TYPE_OF_DEVICE:
		sprintf(p, "%s", bt_device_type_t2str(
					*((bt_device_type_t *) property->val)));
		break;
	case BT_PROPERTY_REMOTE_RSSI:
		sprintf(p, "%d", *((char *) property->val));
		break;
	case BT_PROPERTY_ADAPTER_SCAN_MODE:
		sprintf(p, "%s",
			bt_scan_mode_t2str(*((bt_scan_mode_t *) property->val)));
		break;
	case BT_PROPERTY_ADAPTER_DISCOVERY_TIMEOUT:
		sprintf(p, "%d", *((int *) property->val));
		break;
	case BT_PROPERTY_ADAPTER_BONDED_DEVICES:
		bonded_devices2string(p, property->val, property->len);
		break;
	case BT_PROPERTY_UUIDS:
		uuids2string(p, property->val, property->len);
		break;
	case BT_PROPERTY_SERVICE_RECORD:
		rec = property->val;
		sprintf(p, "{%s, %d, %s}", btuuid2str(rec->uuid.uu),
						rec->channel, rec->name);
		break;
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	case BT_PROPERTY_LOCAL_LE_FEATURES:
		local_le_feat2string(p, property->val);
		break;
#endif
	case BT_PROPERTY_REMOTE_VERSION_INFO:
	case BT_PROPERTY_REMOTE_DEVICE_TIMESTAMP:
	default:
		sprintf(p, "%p", property->val);
		break;
	}

	return buf;
}

#define PROP_PREFIX "persist.sys.bluetooth."
#define PROP_PREFIX_RO "ro.bluetooth."

int get_config(const char *config_key, char *value, const char *fallback)
{
	char key[PROPERTY_KEY_MAX];
	int ret;

	if (strlen(config_key) + sizeof(PROP_PREFIX) > sizeof(key))
		return 0;

	snprintf(key, sizeof(key), PROP_PREFIX"%s", config_key);

	ret = property_get(key, value, "");
	if (ret > 0)
		return ret;

	snprintf(key, sizeof(key), PROP_PREFIX_RO"%s", config_key);

	ret = property_get(key, value, "");
	if (ret > 0)
		return ret;

	if (!fallback)
		return 0;

	return property_get(fallback, value, "");
}
