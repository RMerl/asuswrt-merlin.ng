/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation. All rights reserved.
 *
 *
 */

bool bt_handsfree_register(struct ipc *ipc, const bdaddr_t *addr, uint8_t mode,
							int max_clients);
void bt_handsfree_unregister(void);
