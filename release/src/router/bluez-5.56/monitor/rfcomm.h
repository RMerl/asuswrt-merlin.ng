/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#define RFCOMM_SABM	0x2f
#define RFCOMM_DISC	0x43
#define RFCOMM_UA	0x63
#define RFCOMM_DM	0x0f
#define RFCOMM_UIH	0xef

#define RFCOMM_GET_TYPE(control)	((control) & 0xef)
#define RFCOMM_GET_DLCI(address)	((address & 0xfc) >> 2)
#define RFCOMM_GET_CHANNEL(address)	((address & 0xf8) >> 3)
#define RFCOMM_GET_DIR(address)		((address & 0x04) >> 2)
#define RFCOMM_TEST_EA(length)		((length & 0x01))

struct rfcomm_hdr {
	uint8_t address;
	uint8_t control;
	uint8_t length;
} __attribute__((packed));

struct rfcomm_cmd {
	uint8_t address;
	uint8_t control;
	uint8_t length;
	uint8_t fcs;
} __attribute__((packed));

#define RFCOMM_TEST    0x08
#define RFCOMM_FCON    0x28
#define RFCOMM_FCOFF   0x18
#define RFCOMM_MSC     0x38
#define RFCOMM_RPN     0x24
#define RFCOMM_RLS     0x14
#define RFCOMM_PN      0x20
#define RFCOMM_NSC     0x04

#define RFCOMM_TEST_CR(type)		((type & 0x02))
#define RFCOMM_GET_MCC_TYPE(type)	((type & 0xfc) >> 2)

struct rfcomm_mcc {
	uint8_t type;
	uint8_t length;
} __attribute__((packed));

struct rfcomm_msc {
	uint8_t dlci;
	uint8_t v24_sig;
} __attribute__((packed));

struct rfcomm_pn {
	uint8_t  dlci;
	uint8_t  flow_ctrl;
	uint8_t  priority;
	uint8_t  ack_timer;
	uint16_t mtu;
	uint8_t  max_retrans;
	uint8_t  credits;
} __attribute__((packed));
