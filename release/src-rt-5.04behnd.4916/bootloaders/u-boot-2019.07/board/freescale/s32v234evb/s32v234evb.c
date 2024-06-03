// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013-2015, Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/siul.h>
#include <asm/arch/lpddr2.h>
#include <asm/arch/clock.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

void setup_iomux_ddr(void)
{
	lpddr2_config_iomux(DDR0);
	lpddr2_config_iomux(DDR1);

}

void ddr_phy_init(void)
{
}

void ddr_ctrl_init(void)
{
	config_mmdc(0);
	config_mmdc(1);
}

int dram_init(void)
{
	setup_iomux_ddr();

	ddr_ctrl_init();

	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

static void setup_iomux_uart(void)
{
	/* Muxing for linflex */
	/* Replace the magic values after bringup */

	/* set TXD - MSCR[12] PA12 */
	writel(SIUL2_UART_TXD, SIUL2_MSCRn(SIUL2_UART0_TXD_PAD));

	/* set RXD - MSCR[11] - PA11 */
	writel(SIUL2_UART_MSCR_RXD, SIUL2_MSCRn(SIUL2_UART0_MSCR_RXD_PAD));

	/* set RXD - IMCR[200] - 200 */
	writel(SIUL2_UART_IMCR_RXD, SIUL2_IMCRn(SIUL2_UART0_IMCR_RXD_PAD));
}

static void setup_iomux_enet(void)
{
}

static void setup_iomux_i2c(void)
{
}

#ifdef CONFIG_SYS_USE_NAND
void setup_iomux_nfc(void)
{
}
#endif

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[1] = {
	{USDHC_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	/* eSDHC1 is always present */
	return 1;
}

int board_mmc_init(bd_t * bis)
{
	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_USDHC_CLK);

	/* Set iomux PADS for USDHC */

	/* PK6 pad: uSDHC clk */
	writel(SIUL2_USDHC_PAD_CTRL_CLK, SIUL2_MSCRn(150));
	writel(0x3, SIUL2_MSCRn(902));

	/* PK7 pad: uSDHC CMD */
	writel(SIUL2_USDHC_PAD_CTRL_CMD, SIUL2_MSCRn(151));
	writel(0x3, SIUL2_MSCRn(901));

	/* PK8 pad: uSDHC DAT0 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT0_3, SIUL2_MSCRn(152));
	writel(0x3, SIUL2_MSCRn(903));

	/* PK9 pad: uSDHC DAT1 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT0_3, SIUL2_MSCRn(153));
	writel(0x3, SIUL2_MSCRn(904));

	/* PK10 pad: uSDHC DAT2 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT0_3, SIUL2_MSCRn(154));
	writel(0x3, SIUL2_MSCRn(905));

	/* PK11 pad: uSDHC DAT3 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT0_3, SIUL2_MSCRn(155));
	writel(0x3, SIUL2_MSCRn(906));

	/* PK15 pad: uSDHC DAT4 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT4_7, SIUL2_MSCRn(159));
	writel(0x3, SIUL2_MSCRn(907));

	/* PL0 pad: uSDHC DAT5 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT4_7, SIUL2_MSCRn(160));
	writel(0x3, SIUL2_MSCRn(908));

	/* PL1 pad: uSDHC DAT6 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT4_7, SIUL2_MSCRn(161));
	writel(0x3, SIUL2_MSCRn(909));

	/* PL2 pad: uSDHC DAT7 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT4_7, SIUL2_MSCRn(162));
	writel(0x3, SIUL2_MSCRn(910));

	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
}
#endif

static void mscm_init(void)
{
	struct mscm_ir *mscmir = (struct mscm_ir *)MSCM_BASE_ADDR;
	int i;

	for (i = 0; i < MSCM_IRSPRC_NUM; i++)
		writew(MSCM_IRSPRC_CPn_EN, &mscmir->irsprc[i]);
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_early_init_f(void)
{
	clock_init();
	mscm_init();

	setup_iomux_uart();
	setup_iomux_enet();
	setup_iomux_i2c();
#ifdef CONFIG_SYS_USE_NAND
	setup_iomux_nfc();
#endif
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int checkboard(void)
{
	puts("Board: s32v234evb\n");

	return 0;
}
