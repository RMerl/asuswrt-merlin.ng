/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation. All rights reserved.
 *
 *
 */

struct hal_sock_connect_signal {
	short   size;
	uint8_t bdaddr[6];
	int     channel;
	int     status;
} __attribute__((packed));

void bt_socket_register(struct ipc *ipc, const bdaddr_t *addr, uint8_t mode);
void bt_socket_unregister(void);
