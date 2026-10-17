// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021
 * Köry Maincent, Bootlin, <kory.maincent@bootlin.com>
 */

#include <common.h>
#include <malloc.h>
#include <i2c.h>
#include <extension_board.h>

#include "cape_detect.h"

static void sanitize_field(char *text, size_t size)
{
	char *c = NULL;

	for (c = text; c < text + (int)size; c++) {
		if (*c == 0xFF)
			*c = 0;
	}
}

int extension_board_scan(struct list_head *extension_list)
{
	struct extension *cape;
	struct am335x_cape_eeprom_id eeprom_header;

	int num_capes = 0;
	int ret, i;
	struct udevice *dev;
	unsigned char addr;

	char process_cape_part_number[17] = {'0'};
	char process_cape_version[5] = {'0'};
	uint8_t cursor = 0;

	for (addr = CAPE_EEPROM_FIRST_ADDR; addr <= CAPE_EEPROM_LAST_ADDR; addr++) {
		ret = i2c_get_chip_for_busnum(CONFIG_CAPE_EEPROM_BUS_NUM, addr, 1, &dev);
		if (ret)
			continue;

		/* Move the read cursor to the beginning of the EEPROM */
		dm_i2c_write(dev, 0, &cursor, 1);
		ret = dm_i2c_read(dev, 0, (uint8_t *)&eeprom_header,
				  sizeof(struct am335x_cape_eeprom_id));
		if (ret) {
			printf("Cannot read i2c EEPROM\n");
			continue;
		}

		if (eeprom_header.header != CAPE_MAGIC)
			continue;

		sanitize_field(eeprom_header.board_name, sizeof(eeprom_header.board_name));
		sanitize_field(eeprom_header.version, sizeof(eeprom_header.version));
		sanitize_field(eeprom_header.manufacturer, sizeof(eeprom_header.manufacturer));
		sanitize_field(eeprom_header.part_number, sizeof(eeprom_header.part_number));

		/* Process cape part_number */
		memset(process_cape_part_number, 0, sizeof(process_cape_part_number));
		strncpy(process_cape_part_number, eeprom_header.part_number, 16);
		/* Some capes end with '.' */
		for (i = 15; i >= 0; i--) {
			if (process_cape_part_number[i] == '.')
				process_cape_part_number[i] = '\0';
			else
				break;
		}

		/* Process cape version */
		memset(process_cape_version, 0, sizeof(process_cape_version));
		strncpy(process_cape_version, eeprom_header.version, 4);
		for (i = 0; i < 4; i++) {
			if (process_cape_version[i] == 0)
				process_cape_version[i] = '0';
		}

		printf("BeagleBone Cape: %s (0x%x)\n", eeprom_header.board_name, addr);

		cape = calloc(1, sizeof(struct extension));
		if (!cape) {
			printf("Error in memory allocation\n");
			return num_capes;
		}

		snprintf(cape->overlay, sizeof(cape->overlay), "%s-%s.dtbo",
			 process_cape_part_number, process_cape_version);
		strncpy(cape->name, eeprom_header.board_name, 32);
		strncpy(cape->version, process_cape_version, 4);
		strncpy(cape->owner, eeprom_header.manufacturer, 16);
		list_add_tail(&cape->list, extension_list);
		num_capes++;
	}
	return num_capes;
}
