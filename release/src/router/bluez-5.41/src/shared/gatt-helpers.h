/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Google Inc.
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

/* This file defines helpers for performing client-side procedures defined by
 * the Generic Attribute Profile.
 */

#include <stdbool.h>
#include <stdint.h>

struct bt_gatt_result;

struct bt_gatt_iter {
	struct bt_gatt_result *result;
	uint16_t pos;
};

unsigned int bt_gatt_result_service_count(struct bt_gatt_result *result);
unsigned int bt_gatt_result_characteristic_count(struct bt_gatt_result *result);
unsigned int bt_gatt_result_descriptor_count(struct bt_gatt_result *result);
unsigned int bt_gatt_result_included_count(struct bt_gatt_result *result);

bool bt_gatt_iter_init(struct bt_gatt_iter *iter, struct bt_gatt_result *result);
bool bt_gatt_iter_next_service(struct bt_gatt_iter *iter,
				uint16_t *start_handle, uint16_t *end_handle,
				uint8_t uuid[16]);
bool bt_gatt_iter_next_characteristic(struct bt_gatt_iter *iter,
				uint16_t *start_handle, uint16_t *end_handle,
				uint16_t *value_handle, uint8_t *properties,
				uint8_t uuid[16]);
bool bt_gatt_iter_next_descriptor(struct bt_gatt_iter *iter, uint16_t *handle,
							uint8_t uuid[16]);
bool bt_gatt_iter_next_included_service(struct bt_gatt_iter *iter,
				uint16_t *handle, uint16_t *start_handle,
				uint16_t *end_handle, uint8_t uuid[16]);
bool bt_gatt_iter_next_read_by_type(struct bt_gatt_iter *iter,
				uint16_t *handle, uint16_t *length,
				const uint8_t **value);

typedef void (*bt_gatt_destroy_func_t)(void *user_data);

typedef void (*bt_gatt_result_callback_t)(bool success, uint8_t att_ecode,
							void *user_data);
typedef void (*bt_gatt_request_callback_t)(bool success, uint8_t att_ecode,
						struct bt_gatt_result *result,
						void *user_data);

struct bt_gatt_request;

struct bt_gatt_request *bt_gatt_request_ref(struct bt_gatt_request *req);
void bt_gatt_request_unref(struct bt_gatt_request *req);
void bt_gatt_request_cancel(struct bt_gatt_request *req);

unsigned int bt_gatt_exchange_mtu(struct bt_att *att, uint16_t client_rx_mtu,
					bt_gatt_result_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy);

struct bt_gatt_request *bt_gatt_discover_all_primary_services(
					struct bt_att *att, bt_uuid_t *uuid,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy);
struct bt_gatt_request *bt_gatt_discover_primary_services(
					struct bt_att *att, bt_uuid_t *uuid,
					uint16_t start, uint16_t end,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy);
struct bt_gatt_request *bt_gatt_discover_secondary_services(
					struct bt_att *att, bt_uuid_t *uuid,
					uint16_t start, uint16_t end,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy);
struct bt_gatt_request *bt_gatt_discover_included_services(struct bt_att *att,
					uint16_t start, uint16_t end,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy);
struct bt_gatt_request *bt_gatt_discover_characteristics(struct bt_att *att,
					uint16_t start, uint16_t end,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy);
struct bt_gatt_request *bt_gatt_discover_descriptors(struct bt_att *att,
					uint16_t start, uint16_t end,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy);

bool bt_gatt_read_by_type(struct bt_att *att, uint16_t start, uint16_t end,
					const bt_uuid_t *uuid,
					bt_gatt_request_callback_t callback,
					void *user_data,
					bt_gatt_destroy_func_t destroy);
