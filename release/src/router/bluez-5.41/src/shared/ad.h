/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2015  Google Inc.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <inttypes.h>
#include <stdbool.h>

#include "lib/bluetooth.h"
#include "lib/uuid.h"

typedef void (*bt_ad_func_t)(void *data, void *user_data);

struct bt_ad;

struct bt_ad_manufacturer_data {
	uint16_t manufacturer_id;
	uint8_t *data;
	size_t len;
};

struct bt_ad_service_data {
	bt_uuid_t uuid;
	uint8_t *data;
	size_t len;
};

struct bt_ad *bt_ad_new(void);

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
