// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018
 * Lukasz Majewski, DENX Software Engineering, lukma@denx.de
 *
 * Based on code developed by:
 *
 * Copyright (C) 2012 TQ-Systems GmbH
 * Daniel Gericke <daniel.gericke@tqs.de>
 */

#include <common.h>
#include <env.h>
#include <i2c.h>
#include "kp_id_rev.h"
#include <net.h>

static int eeprom_has_been_read;
static struct id_eeprom eeprom;

void show_eeprom(void)
{
	char safe_string[33];
	int i;
	u8 *p;

	puts("Module EEPROM:\n");
	/* ID */
	for (i = 0; i <= sizeof(eeprom.id) && 0xff != eeprom.id[i]; ++i)
		safe_string[i] = eeprom.id[i];
	safe_string[i] = '\0';

	if (!strncmp(safe_string, "TQM", 3)) {
		printf("  ID: %s\n", safe_string);
		env_set("boardtype", safe_string);
	} else {
		puts("  unknown hardware variant\n");
	}

	/* Serial number */
	for (i = 0; (sizeof(eeprom.serial) >= i) &&
		    (eeprom.serial[i] >= 0x30) &&
		    (eeprom.serial[i] <= 0x39); ++i)
		safe_string[i] = eeprom.serial[i];
	safe_string[i] = '\0';

	if (strlen(safe_string) == 8) {
		printf("  SN: %s\n", safe_string);
		env_set("serial#", safe_string);
	} else {
		puts("  unknown serial number\n");
	}

	/* MAC address  */
	p = eeprom.mac;
	if (!is_valid_ethaddr(p)) {
		printf("  Not valid ETH EEPROM addr!\n");
		return;
	}

	printf("  MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
	       p[0], p[1], p[2], p[3], p[4], p[5]);

	eth_env_set_enetaddr("ethaddr", p);
}

#define I2C_EEPROM_BUS_NUM	1
#define I2C_EEPROM_ADDR		0x50
#define I2C_EEPROM_ADDR_LEN	2

int read_eeprom(void)
{
	struct udevice *dev;
	int ret;

	if (eeprom_has_been_read)
		return 0;

	ret = i2c_get_chip_for_busnum(I2C_EEPROM_BUS_NUM, I2C_EEPROM_ADDR,
				      I2C_EEPROM_ADDR_LEN, &dev);
	if (ret) {
		printf("Cannot find EEPROM !\n");
		return ret;
	}

	ret = dm_i2c_read(dev, 0x0, (uchar *)&eeprom, sizeof(eeprom));

	eeprom_has_been_read = (ret == 0) ? 1 : 0;
	return ret;
}

int read_board_id(void)
{
	unsigned char rev_id = 0x42;
	char rev_str[32], buf[8];
	struct udevice *dev;
	int ret;

	ret = i2c_get_chip_for_busnum(2, 0x22, 1, &dev);
	if (ret) {
		printf("Cannot find pcf8574 IO expander !\n");
		return ret;
	}

	dm_i2c_read(dev, 0x0, &rev_id, sizeof(rev_id));

	sprintf(rev_str, "%02X", rev_id);
	if (rev_id & 0x80) {
		printf("BBoard:4x00 Rev:%s\n", rev_str);
		env_set("boardtype", "ddc");
		env_set("fit_config", "imx53_kb_conf");
	} else {
		printf("BBoard:40x0 Rev:%s\n", rev_str);
		env_set("boardtype", "hsc");
		env_set("fit_config", "imx53_kb_40x0_conf");
	}

	sprintf(buf, "kp-%s", env_get("boardtype"));
	env_set("boardname", buf);
	env_set("boardsoc", "imx53");
	env_set("kb53_rev", rev_str);

	return 0;
}
