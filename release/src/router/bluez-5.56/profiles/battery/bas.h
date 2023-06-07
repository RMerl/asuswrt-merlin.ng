/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 */

struct bt_bas;

struct bt_bas *bt_bas_new(void *primary);

struct bt_bas *bt_bas_ref(struct bt_bas *bas);
void bt_bas_unref(struct bt_bas *bas);

bool bt_bas_attach(struct bt_bas *bas, void *gatt);
void bt_bas_detach(struct bt_bas *bas);
