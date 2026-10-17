// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Vyacheslav Bocharov
 * Author: Vyacheslav Bocharov <adeep@lexina.in>
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <adc.h>
#include <env.h>
#include <init.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/gx.h>
#include <asm/arch/sm.h>
#include <asm/arch/eth.h>
#include <asm/arch/mem.h>

#define EFUSE_SN_OFFSET		50
#define EFUSE_SN_SIZE		32
#define EFUSE_MAC_OFFSET	0
#define EFUSE_MAC_SIZE		6
#define EFUSE_USID_OFFSET	18
#define EFUSE_USID_SIZE		32

int misc_init_r(void)
{
	u8 mac_addr[EFUSE_MAC_SIZE + 1];
	char serial[EFUSE_SN_SIZE + 1];
	char usid[EFUSE_USID_SIZE + 1];
	ssize_t len;
	unsigned int adcval;
	int ret;

	if (!eth_env_get_enetaddr("ethaddr", mac_addr)) {
		len = meson_sm_read_efuse(EFUSE_MAC_OFFSET,
					  mac_addr, EFUSE_MAC_SIZE);
		mac_addr[len] = '\0';
		if (len == EFUSE_MAC_SIZE && is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("ethaddr", mac_addr);
		else
			meson_generate_serial_ethaddr();
	}

	if (!env_get("serial")) {
		len = meson_sm_read_efuse(EFUSE_SN_OFFSET, serial,
					  EFUSE_SN_SIZE);
		serial[len] = '\0';
		if (len == EFUSE_SN_SIZE)
			env_set("serial", serial);
	}

	if (!env_get("usid")) {
		len = meson_sm_read_efuse(EFUSE_USID_OFFSET, usid,
					  EFUSE_USID_SIZE);
		usid[len] = '\0';
		if (len == EFUSE_USID_SIZE)
			env_set("usid", usid);
	}

	ret = adc_channel_single_shot("adc@8680", 0, &adcval);
	if (adcval < 3000)
		env_set("userbutton", "true");
	else
		env_set("userbutton", "false");

	return 0;
}
