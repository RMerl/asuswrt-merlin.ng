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

struct bt_amp;

struct bt_amp *bt_amp_new(void);

struct bt_amp *bt_amp_ref(struct bt_amp *amp);
void bt_amp_unref(struct bt_amp *amp);
