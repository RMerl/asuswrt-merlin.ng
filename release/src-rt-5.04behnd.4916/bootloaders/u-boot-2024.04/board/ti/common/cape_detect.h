/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2021
 * Köry Maincent, Bootlin, <kory.maincent@bootlin.com>
 */

#ifndef __CAPE_DETECT_H
#define __CAPE_DETECT_H

struct am335x_cape_eeprom_id {
	unsigned int header;
	char eeprom_rev[2];
	char board_name[32];
	char version[4];
	char manufacturer[16];
	char part_number[16];
};

#define CAPE_EEPROM_FIRST_ADDR	0x54
#define CAPE_EEPROM_LAST_ADDR	0x57

#define CAPE_EEPROM_ADDR_LEN 0x10

#define CAPE_MAGIC 0xEE3355AA

int extension_board_scan(struct list_head *extension_list);

#endif /* __CAPE_DETECT_H */
