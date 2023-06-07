/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation. All rights reserved.
 *
 *
 */

bool bt_avrcp_register(struct ipc *ipc, const bdaddr_t *addr, uint8_t mode);
void bt_avrcp_unregister(void);

void bt_avrcp_connect(const bdaddr_t *dst);
void bt_avrcp_disconnect(const bdaddr_t *dst);
