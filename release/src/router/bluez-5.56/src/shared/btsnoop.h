/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012-2014  Intel Corporation. All rights reserved.
 *
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <sys/time.h>

#define BTSNOOP_FORMAT_INVALID		0
#define BTSNOOP_FORMAT_HCI		1001
#define BTSNOOP_FORMAT_UART		1002
#define BTSNOOP_FORMAT_BCSP		1003
#define BTSNOOP_FORMAT_3WIRE		1004
#define BTSNOOP_FORMAT_MONITOR		2001
#define BTSNOOP_FORMAT_SIMULATOR	2002

#define BTSNOOP_FLAG_PKLG_SUPPORT	(1 << 0)

#define BTSNOOP_OPCODE_NEW_INDEX	0
#define BTSNOOP_OPCODE_DEL_INDEX	1
#define BTSNOOP_OPCODE_COMMAND_PKT	2
#define BTSNOOP_OPCODE_EVENT_PKT	3
#define BTSNOOP_OPCODE_ACL_TX_PKT	4
#define BTSNOOP_OPCODE_ACL_RX_PKT	5
#define BTSNOOP_OPCODE_SCO_TX_PKT	6
#define BTSNOOP_OPCODE_SCO_RX_PKT	7
#define BTSNOOP_OPCODE_OPEN_INDEX	8
#define BTSNOOP_OPCODE_CLOSE_INDEX	9
#define BTSNOOP_OPCODE_INDEX_INFO	10
#define BTSNOOP_OPCODE_VENDOR_DIAG	11
#define BTSNOOP_OPCODE_SYSTEM_NOTE	12
#define BTSNOOP_OPCODE_USER_LOGGING	13
#define BTSNOOP_OPCODE_CTRL_OPEN	14
#define BTSNOOP_OPCODE_CTRL_CLOSE	15
#define BTSNOOP_OPCODE_CTRL_COMMAND	16
#define BTSNOOP_OPCODE_CTRL_EVENT	17
#define BTSNOOP_OPCODE_ISO_TX_PKT	18
#define BTSNOOP_OPCODE_ISO_RX_PKT	19

#define BTSNOOP_MAX_PACKET_SIZE		(1486 + 4)

#define BTSNOOP_TYPE_PRIMARY	0
#define BTSNOOP_TYPE_AMP	1

#define BTSNOOP_BUS_VIRTUAL	0
#define BTSNOOP_BUS_USB		1
#define BTSNOOP_BUS_PCCARD	2
#define BTSNOOP_BUS_UART	3
#define BTSNOOP_BUS_RS232	4
#define BTSNOOP_BUS_PCI		5
#define BTSNOOP_BUS_SDIO	6
#define BTSNOOP_BUS_SPI		7
#define BTSNOOP_BUS_I2C		8
#define BTSNOOP_BUS_SMD		9

struct btsnoop_opcode_new_index {
	uint8_t  type;
	uint8_t  bus;
	uint8_t  bdaddr[6];
	char     name[8];
} __attribute__((packed));

struct btsnoop_opcode_index_info {
	uint8_t  bdaddr[6];
	uint16_t manufacturer;
} __attribute__((packed));

#define BTSNOOP_PRIORITY_EMERG		0
#define BTSNOOP_PRIORITY_ALERT		1
#define BTSNOOP_PRIORITY_CRIT		2
#define BTSNOOP_PRIORITY_ERR		3
#define BTSNOOP_PRIORITY_WARNING	4
#define BTSNOOP_PRIORITY_NOTICE		5
#define BTSNOOP_PRIORITY_INFO		6
#define BTSNOOP_PRIORITY_DEBUG		7

struct btsnoop_opcode_user_logging {
	uint8_t  priority;
	uint8_t  ident_len;
} __attribute__((packed));

struct btsnoop;

struct btsnoop *btsnoop_open(const char *path, unsigned long flags);
struct btsnoop *btsnoop_create(const char *path, size_t max_size,
				unsigned int max_count, uint32_t format);

struct btsnoop *btsnoop_ref(struct btsnoop *btsnoop);
void btsnoop_unref(struct btsnoop *btsnoop);

uint32_t btsnoop_get_format(struct btsnoop *btsnoop);

bool btsnoop_write(struct btsnoop *btsnoop, struct timeval *tv, uint32_t flags,
			uint32_t drops, const void *data, uint16_t size);
bool btsnoop_write_hci(struct btsnoop *btsnoop, struct timeval *tv,
			uint16_t index, uint16_t opcode, uint32_t drops,
			const void *data, uint16_t size);
bool btsnoop_write_phy(struct btsnoop *btsnoop, struct timeval *tv,
			uint16_t frequency, const void *data, uint16_t size);

bool btsnoop_read_hci(struct btsnoop *btsnoop, struct timeval *tv,
					uint16_t *index, uint16_t *opcode,
					void *data, uint16_t *size);
bool btsnoop_read_phy(struct btsnoop *btsnoop, struct timeval *tv,
			uint16_t *frequency, void *data, uint16_t *size);
