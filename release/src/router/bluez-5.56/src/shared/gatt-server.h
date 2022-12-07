/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Google Inc.
 *
 *
 */

#include <stdint.h>

struct bt_gatt_server;

struct bt_gatt_server *bt_gatt_server_new(struct gatt_db *db,
					struct bt_att *att, uint16_t mtu,
					uint8_t min_enc_size);
uint16_t bt_gatt_server_get_mtu(struct bt_gatt_server *server);
struct bt_att *bt_gatt_server_get_att(struct bt_gatt_server *server);

struct bt_gatt_server *bt_gatt_server_ref(struct bt_gatt_server *server);
void bt_gatt_server_unref(struct bt_gatt_server *server);

typedef void (*bt_gatt_server_destroy_func_t)(void *user_data);
typedef void (*bt_gatt_server_debug_func_t)(const char *str, void *user_data);
typedef void (*bt_gatt_server_conf_func_t)(void *user_data);

bool bt_gatt_server_set_debug(struct bt_gatt_server *server,
					bt_gatt_server_debug_func_t callback,
					void *user_data,
					bt_gatt_server_destroy_func_t destroy);

typedef uint8_t (*bt_gatt_server_authorize_cb_t)(struct bt_att *att,
					uint8_t opcode, uint16_t handle,
					void *user_data);
bool bt_gatt_server_set_authorize(struct bt_gatt_server *server,
					bt_gatt_server_authorize_cb_t cb,
					void *user_data);

bool bt_gatt_server_send_notification(struct bt_gatt_server *server,
					uint16_t handle, const uint8_t *value,
					uint16_t length, bool multiple);

bool bt_gatt_server_send_indication(struct bt_gatt_server *server,
					uint16_t handle, const uint8_t *value,
					uint16_t length,
					bt_gatt_server_conf_func_t callback,
					void *user_data,
					bt_gatt_server_destroy_func_t destroy);
