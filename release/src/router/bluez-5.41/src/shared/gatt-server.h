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

#include <stdint.h>

struct bt_gatt_server;

struct bt_gatt_server *bt_gatt_server_new(struct gatt_db *db,
					struct bt_att *att, uint16_t mtu);

struct bt_gatt_server *bt_gatt_server_ref(struct bt_gatt_server *server);
void bt_gatt_server_unref(struct bt_gatt_server *server);

typedef void (*bt_gatt_server_destroy_func_t)(void *user_data);
typedef void (*bt_gatt_server_debug_func_t)(const char *str, void *user_data);
typedef void (*bt_gatt_server_conf_func_t)(void *user_data);

bool bt_gatt_server_set_debug(struct bt_gatt_server *server,
					bt_gatt_server_debug_func_t callback,
					void *user_data,
					bt_gatt_server_destroy_func_t destroy);

bool bt_gatt_server_send_notification(struct bt_gatt_server *server,
					uint16_t handle, const uint8_t *value,
					uint16_t length);

bool bt_gatt_server_send_indication(struct bt_gatt_server *server,
					uint16_t handle, const uint8_t *value,
					uint16_t length,
					bt_gatt_server_conf_func_t callback,
					void *user_data,
					bt_gatt_server_destroy_func_t destroy);
