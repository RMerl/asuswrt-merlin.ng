/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Google Inc.
 *
 *
 */

struct btd_gatt_client;

struct btd_gatt_client *btd_gatt_client_new(struct btd_device *device);
void btd_gatt_client_destroy(struct btd_gatt_client *client);

void btd_gatt_client_ready(struct btd_gatt_client *client);
void btd_gatt_client_connected(struct btd_gatt_client *client);
void btd_gatt_client_service_added(struct btd_gatt_client *client,
					struct gatt_db_attribute *attrib);
void btd_gatt_client_service_removed(struct btd_gatt_client *client,
					struct gatt_db_attribute *attrib);
void btd_gatt_client_disconnected(struct btd_gatt_client *client);

typedef void (*btd_gatt_client_service_path_t)(const char *service_path,
							void *user_data);
void btd_gatt_client_foreach_service(struct btd_gatt_client *client,
					btd_gatt_client_service_path_t func,
					void *user_data);
