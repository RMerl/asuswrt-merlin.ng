// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 PHYTEC Messtechnik GmbH
 * Author: Wadim Egorov <w.egorov@phytec.de>
 */

#include <eeprom.h>
#include <init.h>
#include <log.h>
#include <net.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <i2c.h>
#include <i2c_eeprom.h>
#include <netdev.h>
#include <linux/bitops.h>
#include "som.h"

static int valid_rk3288_som(struct rk3288_som *som)
{
	unsigned char *p = (unsigned char *)som;
	unsigned char *e = p + sizeof(struct rk3288_som) - 1;
	int hw = 0;

	while (p < e) {
		hw += hweight8(*p);
		p++;
	}

	return hw == som->bs;
}

int rk3288_board_late_init(void)
{
	int ret;
	struct udevice *dev;
	struct rk3288_som opt;
	int off;

	/* Get the identificatioin page of M24C32-D EEPROM */
	off = fdt_path_offset(gd->fdt_blob, "eeprom0");
	if (off < 0) {
		printf("%s: No eeprom0 path offset\n", __func__);
		return off;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_I2C_EEPROM, off, &dev);
	if (ret) {
		printf("%s: Could not find EEPROM\n", __func__);
		return ret;
	}

	ret = i2c_set_chip_offset_len(dev, 2);
	if (ret)
		return ret;

	ret = i2c_eeprom_read(dev, 0, (uint8_t *)&opt,
				sizeof(struct rk3288_som));
	if (ret) {
		printf("%s: Could not read EEPROM\n", __func__);
		return ret;
	}

	if (opt.api_version != 0 || !valid_rk3288_som(&opt)) {
		printf("Invalid data or wrong EEPROM layout version.\n");
		/* Proceed anyway, since there is no fallback option */
	}

	if (is_valid_ethaddr(opt.mac))
		eth_env_set_enetaddr("ethaddr", opt.mac);

	return 0;
}
