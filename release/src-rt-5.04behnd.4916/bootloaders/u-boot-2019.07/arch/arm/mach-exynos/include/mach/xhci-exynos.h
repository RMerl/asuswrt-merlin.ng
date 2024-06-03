/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright (c) 2012 Samsung Electronics Co. Ltd
 *
 * Exynos Phy register definitions
 */

#ifndef _ASM_ARCH_XHCI_EXYNOS_H_
#define _ASM_ARCH_XHCI_EXYNOS_H_

/* Phy register MACRO definitions */

#define LINKSYSTEM_FLADJ_MASK			(0x3f << 1)
#define LINKSYSTEM_FLADJ(_x)			((_x) << 1)
#define LINKSYSTEM_XHCI_VERSION_CONTROL		(0x1 << 27)

#define PHYUTMI_OTGDISABLE			(1 << 6)
#define PHYUTMI_FORCESUSPEND			(1 << 1)
#define PHYUTMI_FORCESLEEP			(1 << 0)

#define PHYCLKRST_SSC_REFCLKSEL_MASK		(0xff << 23)
#define PHYCLKRST_SSC_REFCLKSEL(_x)		((_x) << 23)

#define PHYCLKRST_SSC_RANGE_MASK		(0x03 << 21)
#define PHYCLKRST_SSC_RANGE(_x)			((_x) << 21)

#define PHYCLKRST_SSC_EN			(0x1 << 20)
#define PHYCLKRST_REF_SSP_EN			(0x1 << 19)
#define PHYCLKRST_REF_CLKDIV2			(0x1 << 18)

#define PHYCLKRST_MPLL_MULTIPLIER_MASK		(0x7f << 11)
#define PHYCLKRST_MPLL_MULTIPLIER_100MHZ_REF	(0x19 << 11)
#define PHYCLKRST_MPLL_MULTIPLIER_50M_REF	(0x02 << 11)
#define PHYCLKRST_MPLL_MULTIPLIER_24MHZ_REF	(0x68 << 11)
#define PHYCLKRST_MPLL_MULTIPLIER_20MHZ_REF	(0x7d << 11)
#define PHYCLKRST_MPLL_MULTIPLIER_19200KHZ_REF	(0x02 << 11)

#define PHYCLKRST_FSEL_MASK			(0x3f << 5)
#define PHYCLKRST_FSEL(_x)			((_x) << 5)
#define PHYCLKRST_FSEL_PAD_100MHZ		(0x27 << 5)
#define PHYCLKRST_FSEL_PAD_24MHZ		(0x2a << 5)
#define PHYCLKRST_FSEL_PAD_20MHZ		(0x31 << 5)
#define PHYCLKRST_FSEL_PAD_19_2MHZ		(0x38 << 5)

#define PHYCLKRST_RETENABLEN			(0x1 << 4)

#define PHYCLKRST_REFCLKSEL_MASK		(0x03 << 2)
#define PHYCLKRST_REFCLKSEL_PAD_REFCLK		(0x2 << 2)
#define PHYCLKRST_REFCLKSEL_EXT_REFCLK		(0x3 << 2)

#define PHYCLKRST_PORTRESET			(0x1 << 1)
#define PHYCLKRST_COMMONONN			(0x1 << 0)

#define PHYPARAM0_REF_USE_PAD			(0x1 << 31)
#define PHYPARAM0_REF_LOSLEVEL_MASK		(0x1f << 26)
#define PHYPARAM0_REF_LOSLEVEL			(0x9 << 26)

#define PHYPARAM1_PCS_TXDEEMPH_MASK		(0x1f << 0)
#define PHYPARAM1_PCS_TXDEEMPH			(0x1c)

#define PHYTEST_POWERDOWN_SSP			(0x1 << 3)
#define PHYTEST_POWERDOWN_HSP			(0x1 << 2)

#define PHYBATCHG_UTMI_CLKSEL			(0x1 << 2)

#define FSEL_CLKSEL_24M				(0x5)

/* XHCI PHY register structure */
struct exynos_usb3_phy {
	unsigned int reserve1;
	unsigned int link_system;
	unsigned int phy_utmi;
	unsigned int phy_pipe;
	unsigned int phy_clk_rst;
	unsigned int phy_reg0;
	unsigned int phy_reg1;
	unsigned int phy_param0;
	unsigned int phy_param1;
	unsigned int phy_term;
	unsigned int phy_test;
	unsigned int phy_adp;
	unsigned int phy_batchg;
	unsigned int phy_resume;
	unsigned int reserve2[3];
	unsigned int link_port;
};

#endif /* _ASM_ARCH_XHCI_EXYNOS_H_ */
