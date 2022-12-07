/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
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
bool bt_uhid_unregister_all(struct bt_uhid *uhid);

int bt_uhid_send(struct bt_uhid *uhid, const struct uhid_event *ev);
