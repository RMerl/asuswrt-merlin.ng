/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 */

#define IPC_MTU 1024

#define IPC_STATUS_SUCCESS	0x00

struct ipc_hdr {
	uint8_t  service_id;
	uint8_t  opcode;
	uint16_t len;
	uint8_t  payload[0];
} __attribute__((packed));

#define IPC_OP_STATUS		0x00
struct ipc_status {
	uint8_t code;
} __attribute__((packed));
