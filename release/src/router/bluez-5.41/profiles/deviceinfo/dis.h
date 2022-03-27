/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
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

struct bt_dis;

struct bt_dis *bt_dis_new(void *primary);

struct bt_dis *bt_dis_ref(struct bt_dis *dis);
void bt_dis_unref(struct bt_dis *dis);

bool bt_dis_attach(struct bt_dis *dis, void *gatt);
void bt_dis_detach(struct bt_dis *dis);

typedef void (*bt_dis_notify) (uint8_t source, uint16_t vendor,
					uint16_t product, uint16_t version,
					void *user_data);

bool bt_dis_set_notification(struct bt_dis *dis, bt_dis_notify func,
							void *user_data);
