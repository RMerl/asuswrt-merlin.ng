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

#include <stdbool.h>
#include <stdint.h>

#include "profiles/input/uhid_copy.h"

struct bt_uhid;

struct bt_uhid *bt_uhid_new_default(void);
struct bt_uhid *bt_uhid_new(int fd);

struct bt_uhid *bt_uhid_ref(struct bt_uhid *uhid);
void bt_uhid_unref(struct bt_uhid *uhid);

bool bt_uhid_set_close_on_unref(struct bt_uhid *uhid, bool do_close);

typedef void (*bt_uhid_callback_t)(struct uhid_event *ev, void *user_data);
unsigned int bt_uhid_register(struct bt_uhid *uhid, uint32_t event,
				bt_uhid_callback_t func, void *user_data);
bool bt_uhid_unregister(struct bt_uhid *uhid, unsigned int id);

int bt_uhid_send(struct bt_uhid *uhid, const struct uhid_event *ev);
