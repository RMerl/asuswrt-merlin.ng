/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Based on code developed by:
 *
 * Copyright (C) 2012 TQ-Systems GmbH
 * Daniel Gericke <daniel.gericke@tqs.de>
 */

#ifndef __KP_ID_REV_H_
#define __KP_ID_REV_H_

struct id_eeprom {
	u8 hrcw_primary[0x20];
	u8 mac[6];              /* 0x20 ... 0x25 */
	u8 rsv1[10];
	u8 serial[8];           /* 0x30 ... 0x37 */
	u8 rsv2[8];
	u8 id[0x40];            /* 0x40 ... 0x7f */
} __packed;

void show_eeprom(void);
int read_eeprom(void);
int read_board_id(void);
#endif /* __KP_ID_REV_H_ */
