// SPDX-License-Identifier: GPL-2.0+
/*
 * TI PIPE3 PHY
 *
 * (C) Copyright 2013
 * Texas Instruments, <www.ti.com>
 */

#include <common.h>
#include <sata.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <linux/errno.h>
#include "pipe3-phy.h"

/* PLLCTRL Registers */
#define PLL_STATUS              0x00000004
#define PLL_GO                  0x00000008
#define PLL_CONFIGURATION1      0x0000000C
#define PLL_CONFIGURATION2      0x00000010
#define PLL_CONFIGURATION3      0x00000014
#define PLL_CONFIGURATION4      0x00000020

#define PLL_REGM_MASK           0x001FFE00
#define PLL_REGM_SHIFT          9
#define PLL_REGM_F_MASK         0x0003FFFF
#define PLL_REGM_F_SHIFT        0
#define PLL_REGN_MASK           0x000001FE
#define PLL_REGN_SHIFT          1
#define PLL_SELFREQDCO_MASK     0x0000000E
#define PLL_SELFREQDCO_SHIFT    1
#define PLL_SD_MASK             0x0003FC00
#define PLL_SD_SHIFT            10
#define SET_PLL_GO              0x1
#define PLL_TICOPWDN            BIT(16)
#define PLL_LDOPWDN             BIT(15)
#define PLL_LOCK                0x2
#define PLL_IDLE                0x1

/* PHY POWER CONTROL Register */
#define OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_CMD_MASK         0x003FC000
#define OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_CMD_SHIFT        0xE

#define OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_FREQ_MASK        0xFFC00000
#define OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_FREQ_SHIFT       0x16

#define OMAP_CTRL_PIPE3_PHY_TX_RX_POWERON       0x3
#define OMAP_CTRL_PIPE3_PHY_TX_RX_POWEROFF      0x0


#define PLL_IDLE_TIME   100     /* in milliseconds */
#define PLL_LOCK_TIME   100     /* in milliseconds */

static inline u32 omap_pipe3_readl(void __iomem *addr, unsigned offset)
{
	return __raw_readl(addr + offset);
}

static inline void omap_pipe3_writel(void __iomem *addr, unsigned offset,
		u32 data)
{
	__raw_writel(data, addr + offset);
}

static struct pipe3_dpll_params *omap_pipe3_get_dpll_params(struct omap_pipe3
									*pipe3)
{
	u32 rate;
	struct pipe3_dpll_map *dpll_map = pipe3->dpll_map;

	rate = get_sys_clk_freq();

	for (; dpll_map->rate; dpll_map++) {
		if (rate == dpll_map->rate)
			return &dpll_map->params;
	}

	printf("%s: No DPLL configuration for %u Hz SYS CLK\n",
	       __func__, rate);
	return NULL;
}


static int omap_pipe3_wait_lock(struct omap_pipe3 *phy)
{
	u32 val;
	int timeout = PLL_LOCK_TIME;

	do {
		mdelay(1);
		val = omap_pipe3_readl(phy->pll_ctrl_base, PLL_STATUS);
		if (val & PLL_LOCK)
			break;
	} while (--timeout);

	if (!(val & PLL_LOCK)) {
		printf("%s: DPLL failed to lock\n", __func__);
		return -EBUSY;
	}

	return 0;
}

static int omap_pipe3_dpll_program(struct omap_pipe3 *phy)
{
	u32                     val;
	struct pipe3_dpll_params *dpll_params;

	dpll_params = omap_pipe3_get_dpll_params(phy);
	if (!dpll_params) {
		printf("%s: Invalid DPLL parameters\n", __func__);
		return -EINVAL;
	}

	val = omap_pipe3_readl(phy->pll_ctrl_base, PLL_CONFIGURATION1);
	val &= ~PLL_REGN_MASK;
	val |= dpll_params->n << PLL_REGN_SHIFT;
	omap_pipe3_writel(phy->pll_ctrl_base, PLL_CONFIGURATION1, val);

	val = omap_pipe3_readl(phy->pll_ctrl_base, PLL_CONFIGURATION2);
	val &= ~PLL_SELFREQDCO_MASK;
	val |= dpll_params->freq << PLL_SELFREQDCO_SHIFT;
	omap_pipe3_writel(phy->pll_ctrl_base, PLL_CONFIGURATION2, val);

	val = omap_pipe3_readl(phy->pll_ctrl_base, PLL_CONFIGURATION1);
	val &= ~PLL_REGM_MASK;
	val |= dpll_params->m << PLL_REGM_SHIFT;
	omap_pipe3_writel(phy->pll_ctrl_base, PLL_CONFIGURATION1, val);

	val = omap_pipe3_readl(phy->pll_ctrl_base, PLL_CONFIGURATION4);
	val &= ~PLL_REGM_F_MASK;
	val |= dpll_params->mf << PLL_REGM_F_SHIFT;
	omap_pipe3_writel(phy->pll_ctrl_base, PLL_CONFIGURATION4, val);

	val = omap_pipe3_readl(phy->pll_ctrl_base, PLL_CONFIGURATION3);
	val &= ~PLL_SD_MASK;
	val |= dpll_params->sd << PLL_SD_SHIFT;
	omap_pipe3_writel(phy->pll_ctrl_base, PLL_CONFIGURATION3, val);

	omap_pipe3_writel(phy->pll_ctrl_base, PLL_GO, SET_PLL_GO);

	return omap_pipe3_wait_lock(phy);
}

static void omap_control_phy_power(struct omap_pipe3 *phy, int on)
{
	u32 val, rate;

	val = readl(phy->power_reg);

	rate = get_sys_clk_freq();
	rate = rate/1000000;

	if (on) {
		val &= ~(OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_CMD_MASK |
				OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_FREQ_MASK);
		val |= OMAP_CTRL_PIPE3_PHY_TX_RX_POWERON <<
			OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_CMD_SHIFT;
		val |= rate <<
			OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_FREQ_SHIFT;
	} else {
		val &= ~OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_CMD_MASK;
		val |= OMAP_CTRL_PIPE3_PHY_TX_RX_POWEROFF <<
			OMAP_CTRL_PIPE3_PHY_PWRCTL_CLK_CMD_SHIFT;
	}

	writel(val, phy->power_reg);
}

int phy_pipe3_power_on(struct omap_pipe3 *phy)
{
	int ret;
	u32 val;

	/* Program the DPLL only if not locked */
	val = omap_pipe3_readl(phy->pll_ctrl_base, PLL_STATUS);
	if (!(val & PLL_LOCK)) {
		ret = omap_pipe3_dpll_program(phy);
		if (ret)
			return ret;
	} else {
		/* else just bring it out of IDLE mode */
		val = omap_pipe3_readl(phy->pll_ctrl_base, PLL_CONFIGURATION2);
		if (val & PLL_IDLE) {
			val &= ~PLL_IDLE;
			omap_pipe3_writel(phy->pll_ctrl_base,
					  PLL_CONFIGURATION2, val);
			ret = omap_pipe3_wait_lock(phy);
			if (ret)
				return ret;
		}
	}

	/* Power up the PHY */
	omap_control_phy_power(phy, 1);

	return 0;
}

int phy_pipe3_power_off(struct omap_pipe3 *phy)
{
	u32 val;
	int timeout = PLL_IDLE_TIME;

	/* Power down the PHY */
	omap_control_phy_power(phy, 0);

	/* Put DPLL in IDLE mode */
	val = omap_pipe3_readl(phy->pll_ctrl_base, PLL_CONFIGURATION2);
	val |= PLL_IDLE;
	omap_pipe3_writel(phy->pll_ctrl_base, PLL_CONFIGURATION2, val);

	/* wait for LDO and Oscillator to power down */
	do {
		mdelay(1);
		val = omap_pipe3_readl(phy->pll_ctrl_base, PLL_STATUS);
		if ((val & PLL_TICOPWDN) && (val & PLL_LDOPWDN))
			break;
	} while (--timeout);

	if (!(val & PLL_TICOPWDN) || !(val & PLL_LDOPWDN)) {
		printf("%s: Failed to power down DPLL: PLL_STATUS 0x%x\n",
		       __func__, val);
		return -EBUSY;
	}

	return 0;
}

