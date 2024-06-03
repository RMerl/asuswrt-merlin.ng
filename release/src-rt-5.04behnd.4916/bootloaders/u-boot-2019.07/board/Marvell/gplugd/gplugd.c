// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <contact@8051projects.net>
 *
 * Based on Aspenite:
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
 */

#include <common.h>
#include <mvmfp.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mfp.h>
#include <asm/arch/armada100.h>
#include <asm/gpio.h>
#include <miiphy.h>
#include <asm/mach-types.h>

#ifdef CONFIG_ARMADA100_FEC
#include <net.h>
#include <netdev.h>
#endif /* CONFIG_ARMADA100_FEC */

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	u32 mfp_cfg[] = {
		/* I2C */
		MFP105_CI2C_SDA,
		MFP106_CI2C_SCL,

		/* Enable Console on UART3 */
		MFPO8_UART3_TXD,
		MFPO9_UART3_RXD,

		/* Ethernet PHY Interface */
		MFP086_ETH_TXCLK,
		MFP087_ETH_TXEN,
		MFP088_ETH_TXDQ3,
		MFP089_ETH_TXDQ2,
		MFP090_ETH_TXDQ1,
		MFP091_ETH_TXDQ0,
		MFP092_ETH_CRS,
		MFP093_ETH_COL,
		MFP094_ETH_RXCLK,
		MFP095_ETH_RXER,
		MFP096_ETH_RXDQ3,
		MFP097_ETH_RXDQ2,
		MFP098_ETH_RXDQ1,
		MFP099_ETH_RXDQ0,
		MFP100_ETH_MDC,
		MFP101_ETH_MDIO,
		MFP103_ETH_RXDV,

		/* SSP2 */
		MFP107_SSP2_RXD,
		MFP108_SSP2_TXD,
		MFP110_SSP2_CS,
		MFP111_SSP2_CLK,

		MFP_EOC		/*End of configuration*/
	};
	/* configure MFP's */
	mfp_config(mfp_cfg);
	return 0;
}

int board_init(void)
{
	struct armd1apb2_registers *apb2_regs =
		(struct armd1apb2_registers *)ARMD1_APBC2_BASE;

	/* arch number of Board */
	gd->bd->bi_arch_number = MACH_TYPE_GPLUGD;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = armd1_sdram_base(0) + 0x100;
	/* Assert PHY_RST# */
	gpio_direction_output(CONFIG_SYS_GPIO_PHY_RST, GPIO_LOW);
	udelay(10);
	/* Deassert PHY_RST# */
	gpio_set_value(CONFIG_SYS_GPIO_PHY_RST, GPIO_HIGH);

	/* Enable SSP2 clock */
	writel(SSP2_APBCLK | SSP2_FNCLK, &apb2_regs->ssp2_clkrst);
	return 0;
}

#ifdef CONFIG_ARMADA100_FEC
int board_eth_init(bd_t *bis)
{
	struct armd1apmu_registers *apmu_regs =
		(struct armd1apmu_registers *)ARMD1_APMU_BASE;

	/* Enable clock of ethernet controller */
	writel(FE_CLK_RST | FE_CLK_ENA, &apmu_regs->fecrc);

	return armada100_fec_register(ARMD1_FEC_BASE);
}

#ifdef CONFIG_RESET_PHY_R
/* Configure and initialize PHY chip 88E3015 */
void reset_phy(void)
{
	u16 phy_adr;
	const char *name = "armd-fec0";

	if (miiphy_set_current_dev(name))
		return;

	/* command to read PHY dev address */
	if (miiphy_read(name, 0xff, 0xff, &phy_adr)) {
		printf("Err..%s could not read PHY dev address\n", __func__);
		return;
	}

	/* Set Ethernet LED in TX blink mode */
	miiphy_write(name, phy_adr, PHY_LED_MAN_REG, 0x00);
	miiphy_write(name, phy_adr, PHY_LED_PAR_SEL_REG, PHY_LED_VAL);

	/* reset the phy */
	miiphy_reset(name, phy_adr);
	debug("88E3015 Initialized on %s\n", name);
}
#endif /* CONFIG_RESET_PHY_R */
#endif /* CONFIG_ARMADA100_FEC */
