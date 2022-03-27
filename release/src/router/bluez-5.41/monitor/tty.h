/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2016  Intel Corporation
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

#include <stdint.h>

struct tty_hdr {
	uint16_t data_len;
	uint16_t opcode;
	uint8_t  flags;
	uint8_t  hdr_len;
	uint8_t  ext_hdr[0];
} __attribute__ ((packed));

#define TTY_EXTHDR_COMMAND_DROPS  1
#define TTY_EXTHDR_EVENT_DROPS    2
#define TTY_EXTHDR_ACL_TX_DROPS   3
#define TTY_EXTHDR_ACL_RX_DROPS   4
#define TTY_EXTHDR_SCO_TX_DROPS   5
#define TTY_EXTHDR_SCO_RX_DROPS   6
#define TTY_EXTHDR_OTHER_DROPS    7
#define TTY_EXTHDR_TS32           8
