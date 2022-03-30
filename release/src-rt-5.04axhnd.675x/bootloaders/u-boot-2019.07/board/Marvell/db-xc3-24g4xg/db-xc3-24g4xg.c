// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <i2c.h>
#include <asm/gpio.h>
#include <linux/mbus.h>
#include <linux/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * These values and defines are taken from the Marvell U-Boot version
 * "u-boot-2013.01-2016_T1.0.eng_drop_v6"
 */
#define DB_DX_AC3_GPP_OUT_ENA_LOW	(~(BIT(0) | BIT(2) | BIT(3) | BIT(4) | BIT(6) | BIT(12) \
					| BIT(13) | BIT(16) | BIT(17) | BIT(20) | BIT(29)  | BIT(30)))
#define DB_DX_AC3_GPP_OUT_ENA_MID	(~(0))
#define DB_DX_AC3_GPP_OUT_VAL_LOW	(BIT(0) | BIT(2) | BIT(3) | BIT(4) | BIT(6) | BIT(12) \
					| BIT(13) | BIT(16) | BIT(17) | BIT(20) | BIT(29)  | BIT(30))
#define DB_DX_AC3_GPP_OUT_VAL_MID	0x0
#define DB_DX_AC3_GPP_POL_LOW		0x0
#define DB_DX_AC3_GPP_POL_MID		0x0

int board_early_init_f(void)
{
	/* Configure MPP */
	writel(0x00142222, MVEBU_MPP_BASE + 0x00);
	writel(0x11122000, MVEBU_MPP_BASE + 0x04);
	writel(0x44444004, MVEBU_MPP_BASE + 0x08);
	writel(0x14444444, MVEBU_MPP_BASE + 0x0c);
	writel(0x00000001, MVEBU_MPP_BASE + 0x10);

	/* Set GPP Out value */
	writel(DB_DX_AC3_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(DB_DX_AC3_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);

	/* Set GPP Polarity */
	writel(DB_DX_AC3_GPP_POL_LOW, MVEBU_GPIO0_BASE + 0x0c);
	writel(DB_DX_AC3_GPP_POL_MID, MVEBU_GPIO1_BASE + 0x0c);

	/* Set GPP Out Enable */
	writel(DB_DX_AC3_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(DB_DX_AC3_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	puts("Board: " CONFIG_SYS_BOARD "\n");

	return 0;
}
#endif
