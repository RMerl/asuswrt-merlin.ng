/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
