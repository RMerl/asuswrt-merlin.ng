// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Stefan Roese <sr@denx.de>
 */

#include <common.h>
#include <init.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Those values and defines are taken from the Marvell U-Boot version
 * "u-boot-2013.01-2014_T2.0" for the board Armada 375 DB-88F6720
 */
#define DB_88F6720_MPP0_7		0x00020020 /* SPI */
#define DB_88F6720_MPP8_15		0x22000022 /* SPI , I2C */
#define DB_88F6720_MPP16_23		0x22222222 /* UART, TDM*/
#define DB_88F6720_MPP24_31		0x33333333 /* SDIO, SPI1*/
#define DB_88F6720_MPP32_39		0x04403330 /* SPI1, External SMI */
#define DB_88F6720_MPP40_47		0x22002044 /* UART1, GE0, SATA0 LED */
#define DB_88F6720_MPP48_55		0x22222222 /* GE0 */
#define DB_88F6720_MPP56_63		0x04444422 /* GE0 , LED_MATRIX, GPIO */
#define DB_88F6720_MPP64_67		0x014	/* LED_MATRIX, SATA1 LED*/

#define DB_88F6720_GPP_OUT_ENA_LOW	0xFFFFFFFF
#define DB_88F6720_GPP_OUT_ENA_MID	0x7FFFFFFF
#define DB_88F6720_GPP_OUT_ENA_HIGH	0xFFFFFFFF
#define DB_88F6720_GPP_OUT_VAL_LOW	0x0
#define DB_88F6720_GPP_OUT_VAL_MID	BIT(31)	/* SATA Power output enable */
#define DB_88F6720_GPP_OUT_VAL_HIGH	0x0
#define DB_88F6720_GPP_POL_LOW		0x0
#define DB_88F6720_GPP_POL_MID		0x0
#define DB_88F6720_GPP_POL_HIGH		0x0

int board_early_init_f(void)
{
	/* Configure MPP */
	writel(DB_88F6720_MPP0_7, MVEBU_MPP_BASE + 0x00);
	writel(DB_88F6720_MPP8_15, MVEBU_MPP_BASE + 0x04);
	writel(DB_88F6720_MPP16_23, MVEBU_MPP_BASE + 0x08);
	writel(DB_88F6720_MPP24_31, MVEBU_MPP_BASE + 0x0c);
	writel(DB_88F6720_MPP32_39, MVEBU_MPP_BASE + 0x10);
	writel(DB_88F6720_MPP40_47, MVEBU_MPP_BASE + 0x14);
	writel(DB_88F6720_MPP48_55, MVEBU_MPP_BASE + 0x18);
	writel(DB_88F6720_MPP56_63, MVEBU_MPP_BASE + 0x1c);
	writel(DB_88F6720_MPP64_67, MVEBU_MPP_BASE + 0x20);

	/* Configure GPIO */
	/* Set GPP Out value */
	writel(DB_88F6720_GPP_OUT_VAL_LOW, MVEBU_GPIO0_BASE + 0x00);
	writel(DB_88F6720_GPP_OUT_VAL_MID, MVEBU_GPIO1_BASE + 0x00);
	writel(DB_88F6720_GPP_OUT_VAL_HIGH, MVEBU_GPIO2_BASE + 0x00);

	/* Set GPP Polarity */
	writel(DB_88F6720_GPP_POL_LOW, MVEBU_GPIO0_BASE + 0x0c);
	writel(DB_88F6720_GPP_POL_MID, MVEBU_GPIO1_BASE + 0x0c);
	writel(DB_88F6720_GPP_POL_HIGH, MVEBU_GPIO2_BASE + 0x0c);

	/* Set GPP Out Enable */
	writel(DB_88F6720_GPP_OUT_ENA_LOW, MVEBU_GPIO0_BASE + 0x04);
	writel(DB_88F6720_GPP_OUT_ENA_MID, MVEBU_GPIO1_BASE + 0x04);
	writel(DB_88F6720_GPP_OUT_ENA_HIGH, MVEBU_GPIO2_BASE + 0x04);

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = mvebu_sdram_bar(0) + 0x100;

	return 0;
}

int checkboard(void)
{
	puts("Board: Marvell DB-88F6720\n");

	return 0;
}

int board_eth_init(struct bd_info *bis)
{
	cpu_eth_init(bis); /* Built in controller(s) come first */
	return pci_eth_init(bis);
}
