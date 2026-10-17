// SPDX-License-Identifier: GPL-2.0
/**
 * samsung_usb_phy.c - DesignWare USB3 (DWC3) PHY handling file
 *
 * Copyright (C) 2015 Samsung Electronics
 *
 * Author: Joonyoung Shim <jy0922.shim@samsung.com>
 */

#include <common.h>
#include <asm/arch/power.h>
#include <asm/arch/xhci-exynos.h>
#include <linux/delay.h>

void exynos5_usb3_phy_init(struct exynos_usb3_phy *phy)
{
	u32 reg;

	/* Reset USB 3.0 PHY */
	writel(0x0, &phy->phy_reg0);

	clrbits_le32(&phy->phy_param0,
			/* Select PHY CLK source */
			PHYPARAM0_REF_USE_PAD |
			/* Set Loss-of-Signal Detector sensitivity */
			PHYPARAM0_REF_LOSLEVEL_MASK);
	setbits_le32(&phy->phy_param0, PHYPARAM0_REF_LOSLEVEL);


	writel(0x0, &phy->phy_resume);

	/*
	 * Setting the Frame length Adj value[6:1] to default 0x20
	 * See xHCI 1.0 spec, 5.2.4
	 */
	setbits_le32(&phy->link_system,
			LINKSYSTEM_XHCI_VERSION_CONTROL |
			LINKSYSTEM_FLADJ(0x20));

	/* Set Tx De-Emphasis level */
	clrbits_le32(&phy->phy_param1, PHYPARAM1_PCS_TXDEEMPH_MASK);
	setbits_le32(&phy->phy_param1, PHYPARAM1_PCS_TXDEEMPH);

	setbits_le32(&phy->phy_batchg, PHYBATCHG_UTMI_CLKSEL);

	/* PHYTEST POWERDOWN Control */
	clrbits_le32(&phy->phy_test,
			PHYTEST_POWERDOWN_SSP |
			PHYTEST_POWERDOWN_HSP);

	/* UTMI Power Control */
	writel(PHYUTMI_OTGDISABLE, &phy->phy_utmi);

		/* Use core clock from main PLL */
	reg = PHYCLKRST_REFCLKSEL_EXT_REFCLK |
		/* Default 24Mhz crystal clock */
		PHYCLKRST_FSEL(FSEL_CLKSEL_24M) |
		PHYCLKRST_MPLL_MULTIPLIER_24MHZ_REF |
		PHYCLKRST_SSC_REFCLKSEL(0) |
		/* Force PortReset of PHY */
		PHYCLKRST_PORTRESET |
		/* Digital power supply in normal operating mode */
		PHYCLKRST_RETENABLEN |
		/* Enable ref clock for SS function */
		PHYCLKRST_REF_SSP_EN |
		/* Enable spread spectrum */
		PHYCLKRST_SSC_EN |
		/* Power down HS Bias and PLL blocks in suspend mode */
		PHYCLKRST_COMMONONN;

	writel(reg, &phy->phy_clk_rst);

	/* giving time to Phy clock to settle before resetting */
	udelay(10);

	reg &= ~PHYCLKRST_PORTRESET;
	writel(reg, &phy->phy_clk_rst);
}
