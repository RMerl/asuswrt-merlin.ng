/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

int server_init(gboolean secure);
int server_register(struct btd_adapter *adapter, uint16_t id);
int server_unregister(struct btd_adapter *adapter, uint16_t id);
