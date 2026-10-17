// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <i2c_eeprom.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>

#include "lpddr4_timing.h"
#include "../common/dh_common.h"
#include "../common/dh_imx.h"

DECLARE_GLOBAL_DATA_PTR;

int mach_cpu_init(void)
{
	icache_enable();
	return 0;
}

int board_phys_sdram_size(phys_size_t *size)
{
	const u16 memsz[] = { 512, 1024, 1536, 2048, 3072, 4096, 6144, 8192 };
	const u8 ecc = readl(DDRC_ECCCFG0(0)) & DDRC_ECCCFG0_ECC_MODE_MASK;
	u8 memcfg = dh_get_memcfg();

	/* 896 kiB, i.e. 1 MiB without 12.5% reserved for in-band ECC */
	*size = (u64)memsz[memcfg] * (SZ_1M - (ecc ? (SZ_1M / 8) : 0));

	return 0;
}

static int dh_imx8_setup_ethaddr(void)
{
	unsigned char enetaddr[6];

	if (dh_mac_is_in_env("ethaddr"))
		return 0;

	if (!dh_imx_get_mac_from_fuse(enetaddr))
		goto out;

	if (!dh_get_mac_from_eeprom(enetaddr, "eeprom0"))
		goto out;

	return -ENXIO;

out:
	return eth_env_set_enetaddr("ethaddr", enetaddr);
}

static int dh_imx8_setup_eth1addr(void)
{
	unsigned char enetaddr[6];

	if (dh_mac_is_in_env("eth1addr"))
		return 0;

	if (!dh_imx_get_mac_from_fuse(enetaddr))
		goto increment_out;

	if (!dh_get_mac_from_eeprom(enetaddr, "eeprom1"))
		goto out;

	/*
	 * Populate second ethernet MAC from first ethernet EEPROM with MAC
	 * address LSByte incremented by 1. This is only used on SoMs without
	 * second ethernet EEPROM, i.e. early prototypes.
	 */
	if (!dh_get_mac_from_eeprom(enetaddr, "eeprom0"))
		goto increment_out;

	return -ENXIO;

increment_out:
	enetaddr[5]++;

out:
	return eth_env_set_enetaddr("eth1addr", enetaddr);
}

int dh_setup_mac_address(void)
{
	int ret;

	ret = dh_imx8_setup_ethaddr();
	if (ret)
		printf("%s: Unable to setup ethaddr! ret = %d\n", __func__, ret);

	ret = dh_imx8_setup_eth1addr();
	if (ret)
		printf("%s: Unable to setup eth1addr! ret = %d\n", __func__, ret);

	return ret;
}

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	dh_setup_mac_address();
	return 0;
}

enum env_location env_get_location(enum env_operation op, int prio)
{
	return prio ? ENVL_UNKNOWN : ENVL_SPI_FLASH;
}
