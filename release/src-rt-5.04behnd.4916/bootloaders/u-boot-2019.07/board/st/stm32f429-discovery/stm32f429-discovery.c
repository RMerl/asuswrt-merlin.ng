// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011, 2012, 2013
 * Yuri Tikhonov, Emcraft Systems, yur@emcraft.com
 * Alexander Potashev, Emcraft Systems, aspotashev@emcraft.com
 * Vladimir Khusainov, Emcraft Systems, vlad@emcraft.com
 * Pavel Boldin, Emcraft Systems, paboldin@emcraft.com
 *
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 */

#include <common.h>
#include <dm.h>

#include <asm/io.h>
#include <asm/arch/stm32.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	int rv;
	struct udevice *dev;

	rv = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (rv) {
		debug("DRAM init failed: %d\n", rv);
		return rv;
	}

	if (fdtdec_setup_mem_size_base() != 0)
		rv = -EINVAL;

	return rv;
}

int dram_init_banksize(void)
{
	fdtdec_setup_memory_banksize();

	return 0;
}

u32 get_board_rev(void)
{
	return 0;
}

int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = gd->bd->bi_dram[0].start + 0x100;

	return 0;
}

#ifdef CONFIG_MISC_INIT_R
int misc_init_r(void)
{
	char serialno[25];
	uint32_t u_id_low, u_id_mid, u_id_high;

	if (!env_get("serial#")) {
		u_id_low  = readl(&STM32_U_ID->u_id_low);
		u_id_mid  = readl(&STM32_U_ID->u_id_mid);
		u_id_high = readl(&STM32_U_ID->u_id_high);
		sprintf(serialno, "%08x%08x%08x",
			u_id_high, u_id_mid, u_id_low);
		env_set("serial#", serialno);
	}

	return 0;
}
#endif
