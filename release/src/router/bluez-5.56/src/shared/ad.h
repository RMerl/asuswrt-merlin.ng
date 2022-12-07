/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Google Inc.
 *
 *
 */

#include <inttypes.h>
#include <stdbool.h>

#include "lib/bluetooth.h"
#include "lib/uuid.h"

#define BT_AD_MAX_DATA_LEN		31

#define BT_AD_FLAGS			0x01
#define BT_AD_UUID16_SOME		0x02
#define BT_AD_UUID16_ALL		0x03
#define BT_AD_UUID32_SOME		0x04
#define BT_AD_UUID32_ALL		0x05
#define BT_AD_UUID128_SOME		0x06
#define BT_AD_UUID128_ALL		0x07
#define BT_AD_NAME_SHORT		0x08
#define BT_AD_NAME_COMPLETE		0x09
#define BT_AD_TX_POWER			0x0a
#define BT_AD_CLASS_OF_DEV		0x0d
#define BT_AD_SSP_HASH			0x0e
#define BT_AD_SSP_RANDOMIZER		0x0f
#define BT_AD_DEVICE_ID			0x10
#define BT_AD_SMP_TK			0x10
#define BT_AD_SMP_OOB_FLAGS		0x11
#define BT_AD_SLAVE_CONN_INTERVAL	0x12
#define BT_AD_SOLICIT16			0x14
#define BT_AD_SOLICIT128		0x15
#define BT_AD_SERVICE_DATA16		0x16
#define BT_AD_PUBLIC_ADDRESS		0x17
#define BT_AD_RANDOM_ADDRESS		0x18
#define BT_AD_GAP_APPEARANCE		0x19
#define BT_AD_ADVERTISING_INTERVAL	0x1a
#define BT_AD_LE_DEVICE_ADDRESS		0x1b
#define BT_AD_LE_ROLE			0x1c
#define BT_AD_SSP_HASH_P256		0x1d
#define BT_AD_SSP_RANDOMIZER_P256	0x1e
#define BT_AD_SOLICIT32			0x1f
#define BT_AD_SERVICE_DATA32		0x20
#define BT_AD_SERVICE_DATA128		0x21
#define BT_AD_LE_SC_CONFIRM_VALUE	0x22
#define BT_AD_LE_SC_RANDOM_VALUE	0x23
#define BT_AD_URI			0x24
#define BT_AD_INDOOR_POSITIONING	0x25
#define BT_AD_TRANSPORT_DISCOVERY	0x26
#define BT_AD_LE_SUPPORTED_FEATURES	0x27
#define BT_AD_CHANNEL_MAP_UPDATE_IND	0x28
#define BT_AD_MESH_PROV			0x29
#define BT_AD_MESH_DATA			0x2a
#define BT_AD_MESH_BEACON		0x2b
#define BT_AD_3D_INFO_DATA		0x3d
#define BT_AD_MANUFACTURER_DATA		0xff

/* Low Energy Advertising Flags */
#define BT_AD_FLAG_LIMITED		0x01 /* Limited Discoverable */
#define BT_AD_FLAG_GENERAL		0x02 /* General Discoverable */
#define BT_AD_FLAG_NO_BREDR		0x04 /* BR/EDR not supported */

typedef void (*bt_ad_func_t)(void *data, void *user_data);

struct bt_ad;
struct queue;

struct bt_ad_manufacturer_data {
	uint16_t manufacturer_id;
	uint8_t *data;
	size_t len;
};

struct bt_ad_service_data {
	bt_uuid_t uuid;
	size_t len;
	void *data;
};

struct bt_ad_data {
	uint8_t type;
	uint8_t *data;
	size_t len;
};

struct bt_ad_pattern {
	uint8_t type;
	uint8_t offset;
	uint8_t len;
	uint8_t data[BT_AD_MAX_DATA_LEN];
};

struct bt_ad *bt_ad_new(void);

struct bt_ad *bt_ad_new_with_data(size_t len, const uint8_t *data);

struct bt_ad *bt_ad_ref(struct bt_ad *ad);

void bt_ad_unref(struct bt_ad *ad);

uint8_t *bt_ad_generate(struct bt_ad *ad, size_t *length);

bool bt_ad_add_service_uuid(struct bt_ad *ad, const bt_uuid_t *uuid);

bool bt_ad_remove_service_uuid(struct bt_ad *ad, bt_uuid_t *uuid);

void bt_ad_clear_service_uuid(struct bt_ad *ad);

bool bt_ad_add_manufacturer_data(struct bt_ad *ad, uint16_t manufacturer_data,
						void *data, size_t len);

bool bt_ad_has_manufacturer_data(struct bt_ad *ad,
				const struct bt_ad_manufacturer_data *data);

void bt_ad_foreach_manufacturer_data(struct bt_ad *ad, bt_ad_func_t func,
							void *user_data);

bool bt_ad_remove_manufacturer_data(struct bt_ad *ad, uint16_t manufacturer_id);

void bt_ad_clear_manufacturer_data(struct bt_ad *ad);

bool bt_ad_add_solicit_uuid(struct bt_ad *ad, const bt_uuid_t *uuid);

bool bt_ad_remove_solicit_uuid(struct bt_ad *ad, bt_uuid_t *uuid);

void bt_ad_clear_solicit_uuid(struct bt_ad *ad);

bool bt_ad_add_service_data(struct bt_ad *ad, const bt_uuid_t *uuid, void *data,
								size_t len);

bool bt_ad_has_service_data(struct bt_ad *ad,
					const struct bt_ad_service_data *data);

void bt_ad_foreach_service_data(struct bt_ad *ad, bt_ad_func_t func,
							void *user_data);

bool bt_ad_remove_service_data(struct bt_ad *ad, bt_uuid_t *uuid);

void bt_ad_clear_service_data(struct bt_ad *ad);

bool bt_ad_add_name(struct bt_ad *ad, const char *name);

void bt_ad_clear_name(struct bt_ad *ad);

bool bt_ad_add_appearance(struct bt_ad *ad, uint16_t appearance);

void bt_ad_clear_appearance(struct bt_ad *ad);

bool bt_ad_add_flags(struct bt_ad *ad, uint8_t *flags, size_t len);

bool bt_ad_has_flags(struct bt_ad *ad);

void bt_ad_clear_flags(struct bt_ad *ad);

bool bt_ad_add_data(struct bt_ad *ad, uint8_t type, void *data, size_t len);

bool bt_ad_has_data(struct bt_ad *ad, const struct bt_ad_data *data);

void bt_ad_foreach_data(struct bt_ad *ad, bt_ad_func_t func, void *user_data);

bool bt_ad_remove_data(struct bt_ad *ad, uint8_t type);

void bt_ad_clear_data(struct bt_ad *ad);

struct bt_ad_pattern *bt_ad_pattern_new(uint8_t type, size_t offset,
					size_t len, const uint8_t *data);

struct bt_ad_pattern *bt_ad_pattern_match(struct bt_ad *ad,
							struct queue *patterns);
