/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 */

static const char BLUEZ_SCO_SK_PATH[] = "\0bluez_sco_socket";

#define SCO_SERVICE_ID			0

#define SCO_STATUS_SUCCESS		IPC_STATUS_SUCCESS
#define SCO_STATUS_FAILED		0x01

#define SCO_OP_STATUS			IPC_OP_STATUS

#define SCO_OP_GET_FD			0x01
struct sco_cmd_get_fd {
	uint8_t bdaddr[6];
} __attribute__((packed));

struct sco_rsp_get_fd {
	uint16_t mtu;
} __attribute__((packed));
