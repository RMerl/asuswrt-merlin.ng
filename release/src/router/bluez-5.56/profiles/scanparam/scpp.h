/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 */

struct bt_scpp;

struct bt_scpp *bt_scpp_new(void *primary);

struct bt_scpp *bt_scpp_ref(struct bt_scpp *scan);
void bt_scpp_unref(struct bt_scpp *scan);

bool bt_scpp_attach(struct bt_scpp *scan, void *gatt);
void bt_scpp_detach(struct bt_scpp *scan);

bool bt_scpp_set_interval(struct bt_scpp *scan, uint16_t value);
bool bt_scpp_set_window(struct bt_scpp *scan, uint16_t value);
