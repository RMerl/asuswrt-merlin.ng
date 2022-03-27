/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
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

#include <stdbool.h>
#include <stdint.h>

#define BT_GAP_ADDR_TYPE_BREDR		0x00
#define BT_GAP_ADDR_TYPE_LE_PUBLIC	0x01
#define BT_GAP_ADDR_TYPE_LE_RANDOM	0x02

struct bt_gap;

struct bt_gap *bt_gap_new_default(void);
struct bt_gap *bt_gap_new_index(uint16_t index);

struct bt_gap *bt_gap_ref(struct bt_gap *gap);
void bt_gap_unref(struct bt_gap *gap);

typedef void (*bt_gap_destroy_func_t)(void *user_data);
typedef void (*bt_gap_ready_func_t)(bool success, void *user_data);

bool bt_gap_set_ready_handler(struct bt_gap *gap,
				bt_gap_ready_func_t handler, void *user_data,
				bt_gap_destroy_func_t destroy);

bool bt_gap_set_static_addr(struct bt_gap *gap, uint8_t addr[6]);
bool bt_gap_set_local_irk(struct bt_gap *gap, uint8_t key[16]);
bool bt_gap_add_peer_irk(struct bt_gap *gap, uint8_t addr_type,
					uint8_t addr[6], uint8_t key[16]);
