/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2016  Intel Corporation
 *
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
