/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 */

struct bt_dis;

struct bt_dis *bt_dis_new(struct gatt_db *db);
struct bt_dis *bt_dis_new_primary(void *primary);

struct bt_dis *bt_dis_ref(struct bt_dis *dis);
void bt_dis_unref(struct bt_dis *dis);

bool bt_dis_attach(struct bt_dis *dis, void *gatt);
void bt_dis_detach(struct bt_dis *dis);

typedef void (*bt_dis_notify) (uint8_t source, uint16_t vendor,
					uint16_t product, uint16_t version,
					void *user_data);

bool bt_dis_set_notification(struct bt_dis *dis, bt_dis_notify func,
							void *user_data);
