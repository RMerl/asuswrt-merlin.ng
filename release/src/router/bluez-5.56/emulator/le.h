/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2012  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#include <stdbool.h>

struct bt_le;

struct bt_le *bt_le_new(void);

struct bt_le *bt_le_ref(struct bt_le *le);
void bt_le_unref(struct bt_le *le);
