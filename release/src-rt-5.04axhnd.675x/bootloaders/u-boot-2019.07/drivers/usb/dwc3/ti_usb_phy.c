// SPDX-License-Identifier: GPL-2.0+
/**
 * ti_usb_phy.c - USB3 and USB3 PHY programming for dwc3
 *
 * Copyright (C) 2015 Texas Instruments Incorporated - http://www.ti.com
 *
 * Author: Kishon Vijay Abraham I <kishon@ti.com>
 *
 * Taken from Linux Kernel v3.16 (drivers/phy/phy-ti-pipe3.c and
 * drivers/phy/phy-omap-usb2.c) and ported to uboot.
 *
 * "commit 56042e : phy: ti-pipe3: Fix suspend/resume and module reload" for
 * phy-ti-pipe3.c
 *
 * "commit eb82a3 : phy: omap-usb2: Balance pm_runtime_enable() on probe failure
 * and remove" for phy-omap-usb2.c
 */

#include <common.h>
#include <malloc.h>
#include <ti-usb-phy-uboot.h>
#include <linux/ioport.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <dm.h>

#include "linux-compat.h"

#define PLL_STATUS		0x00000004
#define PLL_GO			0x00000008
#define PLL_CONFIGURATION1	0x0000000C
#define PLL_CONFIGURATION2	0x00000010
#define PLL_CONFIGURATION3	0x00000014
#define PLL_CONFIGURATION4	0x00000020

#define PLL_REGM_MASK		0x001FFE00
#define PLL_REGM_SHIFT		0x9
#define PLL_REGM_F_MASK		0x0003FFFF
#define PLL_REGM_F_SHIFT	0x0
#define PLL_REGN_MASK		0x000001FE
#define PLL_REGN_SHIFT		0x1
#define PLL_SELFREQDCO_MASK	0x0000000E
#define PLL_SELFREQDCO_SHIFT	0x1
#define PLL_SD_MASK		0x0003FC00
#define PLL_SD_SHIFT		10
#define SET_PLL_GO		0x1
#define PLL_LDOPWDN		BIT(15)
#define PLL_TICOPWDN		BIT(16)
#define PLL_LOCK		0x2
#define PLL_IDLE		0x1

#define OMAP_CTRL_DEV_PHY_PD				BIT(0)
#define OMAP_CTRL_USB3_PHY_PWRCTL_CLK_CMD_MASK		0x003FC000
#define OMAP_CTRL_USB3_PHY_PWRCTL_CLK_CMD_SHIFT		0xE

#define OMAP_CTRL_USB3_PHY_PWRCTL_CLK_FREQ_MASK		0xFFC00000
#define OMAP_CTRL_USB3_PHY_PWRCTL_CLK_FREQ_SHIFT	0x16

#define OMAP_CTRL_USB3_PHY_TX_RX_POWERON	0x3
#define OMAP_CTRL_USB3_PHY_TX_RX_POWEROFF	0x0

#define OMAP_CTRL_USB2_PHY_PD			BIT(28)

#define AM437X_CTRL_USB2_PHY_PD			BIT(0)
#define AM437X_CTRL_USB2_OTG_PD			BIT(1)
#define AM437X_CTRL_USB2_OTGVDET_EN		BIT(19)
#define AM437X_CTRL_USB2_OTGSESSEND_EN		BIT(20)

static LIST_HEAD(ti_usb_phy_list);
typedef unsigned int u32;

struct usb3_dpll_params {
	u16	m;
	u8	n;
	u8	freq:3;
	u8	sd;
	u32	mf;
};

struct usb3_dpll_map {
	unsigned long rate;
	struct usb3_dpll_params params;
	struct usb3_dpll_map *dpll_map;
};

struct ti_usb_phy {
	void __iomem *pll_ctrl_base;
	void __iomem *usb2_phy_power;
	void __iomem *usb3_phy_power;
	struct usb3_dpll_map *dpll_map;
	struct list_head list;
	int index;
};

static struct usb3_dpll_map dpll_map_usb[] = {
	{12000000, {1250, 5, 4, 20, 0} },	/* 12 MHz */
	{16800000, {3125, 20, 4, 20, 0} },	/* 16.8 MHz */
	{19200000, {1172, 8, 4, 20, 65537} },	/* 19.2 MHz */
	{20000000, {1000, 7, 4, 10, 0} },	/* 20 MHz */
	{26000000, {1250, 12, 4, 20, 0} },	/* 26 MHz */
	{38400000, {3125, 47, 4, 20, 92843} },	/* 38.4 MHz */
	{ },					/* Terminator */
};

static inline unsigned int ti_usb3_readl(void __iomem *base, u32 offset)
{
	return readl(base + offset);
}

static inline void ti_usb3_writel(void __iomem *base, u32 offset, u32 value)
{
	writel(value, base + offset);
}

#ifndef CONFIG_AM43XX
static struct usb3_dpll_params *ti_usb3_get_dpll_params(struct ti_usb_phy *phy)
{
	unsigned long rate;
	struct usb3_dpll_map *dpll_map = phy->dpll_map;

	rate = get_sys_clk_freq();

	for (; dpll_map->rate; dpll_map++) {
		if (rate == dpll_map->rate)
			return &dpll_map->params;
	}

	dev_err(phy->dev, "No DPLL configuration for %lu Hz SYS CLK\n", rate);

	return NULL;
}

static int ti_usb3_dpll_wait_lock(struct ti_usb_phy *phy)
{
	u32 val;
	do {
		val = ti_usb3_readl(phy->pll_ctrl_base, PLL_STATUS);
			if (val & PLL_LOCK)
				break;
	} while (1);

	return 0;
}

static int ti_usb3_dpll_program(struct ti_usb_phy *phy)
{
	u32			val;
	struct usb3_dpll_params	*dpll_params;

	if (!phy->pll_ctrl_base)
		return -EINVAL;

	dpll_params = ti_usb3_get_dpll_params(phy);
	if (!dpll_params)
		return -EINVAL;

	val = ti_usb3_readl(phy->pll_ctrl_base, PLL_CONFIGURATION1);
	val &= ~PLL_REGN_MASK;
	val |= dpll_params->n << PLL_REGN_SHIFT;
	ti_usb3_writel(phy->pll_ctrl_base, PLL_CONFIGURATION1, val);

	val = ti_usb3_readl(phy->pll_ctrl_base, PLL_CONFIGURATION2);
	val &= ~PLL_SELFREQDCO_MASK;
	val |= dpll_params->freq << PLL_SELFREQDCO_SHIFT;
	ti_usb3_writel(phy->pll_ctrl_base, PLL_CONFIGURATION2, val);

	val = ti_usb3_readl(phy->pll_ctrl_base, PLL_CONFIGURATION1);
	val &= ~PLL_REGM_MASK;
	val |= dpll_params->m << PLL_REGM_SHIFT;
	ti_usb3_writel(phy->pll_ctrl_base, PLL_CONFIGURATION1, val);

	val = ti_usb3_readl(phy->pll_ctrl_base, PLL_CONFIGURATION4);
	val &= ~PLL_REGM_F_MASK;
	val |= dpll_params->mf << PLL_REGM_F_SHIFT;
	ti_usb3_writel(phy->pll_ctrl_base, PLL_CONFIGURATION4, val);

	val = ti_usb3_readl(phy->pll_ctrl_base, PLL_CONFIGURATION3);
	val &= ~PLL_SD_MASK;
	val |= dpll_params->sd << PLL_SD_SHIFT;
	ti_usb3_writel(phy->pll_ctrl_base, PLL_CONFIGURATION3, val);

	ti_usb3_writel(phy->pll_ctrl_base, PLL_GO, SET_PLL_GO);

	return ti_usb3_dpll_wait_lock(phy);
}
#endif

void ti_usb2_phy_power(struct ti_usb_phy *phy, int on)
{
	u32 val;

	val = readl(phy->usb2_phy_power);

	if (on) {
#if defined(CONFIG_DRA7XX)
		if (phy->index == 1)
			val &= ~OMAP_CTRL_USB2_PHY_PD;
		else
			val &= ~OMAP_CTRL_DEV_PHY_PD;
#elif defined(CONFIG_AM43XX)
		val &= ~(AM437X_CTRL_USB2_PHY_PD |
			 AM437X_CTRL_USB2_OTG_PD);
		val |= (AM437X_CTRL_USB2_OTGVDET_EN |
			AM437X_CTRL_USB2_OTGSESSEND_EN);
#endif
	} else {
#if defined(CONFIG_DRA7XX)
		if (phy->index == 1)
			val |= OMAP_CTRL_USB2_PHY_PD;
		else
			val |= OMAP_CTRL_DEV_PHY_PD;

#elif defined(CONFIG_AM43XX)
		val &= ~(AM437X_CTRL_USB2_OTGVDET_EN |
			 AM437X_CTRL_USB2_OTGSESSEND_EN);
		val |= (AM437X_CTRL_USB2_PHY_PD |
			AM437X_CTRL_USB2_OTG_PD);
#endif
	}
	writel(val, phy->usb2_phy_power);
}

#ifndef CONFIG_AM43XX
void ti_usb3_phy_power(struct ti_usb_phy *phy, int on)
{
	u32 val;
	u32 rate;
	rate = get_sys_clk_freq();
	rate = rate/1000000;

	if (!phy->usb3_phy_power)
		return;

	val = readl(phy->usb3_phy_power);
	if (on) {
		val &= ~(OMAP_CTRL_USB3_PHY_PWRCTL_CLK_CMD_MASK |
			OMAP_CTRL_USB3_PHY_PWRCTL_CLK_FREQ_MASK);
		val |= (OMAP_CTRL_USB3_PHY_TX_RX_POWERON) <<
			OMAP_CTRL_USB3_PHY_PWRCTL_CLK_CMD_SHIFT;
		val |= rate <<
			OMAP_CTRL_USB3_PHY_PWRCTL_CLK_FREQ_SHIFT;
	} else {
		val &= ~OMAP_CTRL_USB3_PHY_PWRCTL_CLK_CMD_MASK;
		val |= OMAP_CTRL_USB3_PHY_TX_RX_POWEROFF <<
			OMAP_CTRL_USB3_PHY_PWRCTL_CLK_CMD_SHIFT;
	}
	writel(val, phy->usb3_phy_power);
}
#endif

/**
 * ti_usb_phy_uboot_init - usb phy uboot initialization code
 * @dev: struct ti_usb_phy_device containing initialization data
 *
 * Entry point for ti usb phy driver. This driver handles initialization
 * of both usb2 phy and usb3 phy. Pointer to ti_usb_phy_device should be
 * passed containing base address and other initialization data.
 * Returns '0' on success and a negative value on failure.
 *
 * Generally called from board_usb_init() implemented in board file.
 */
int ti_usb_phy_uboot_init(struct ti_usb_phy_device *dev)
{
	struct ti_usb_phy *phy;

	phy = devm_kzalloc(NULL, sizeof(*phy), GFP_KERNEL);
	if (!phy) {
		dev_err(NULL, "unable to alloc mem for TI USB3 PHY\n");
		return -ENOMEM;
	}

	phy->dpll_map = dpll_map_usb;
	phy->index = dev->index;
	phy->pll_ctrl_base = dev->pll_ctrl_base;
	phy->usb2_phy_power = dev->usb2_phy_power;
	phy->usb3_phy_power = dev->usb3_phy_power;

#ifndef CONFIG_AM43XX
	ti_usb3_dpll_program(phy);
	ti_usb3_phy_power(phy, 1);
#endif
	ti_usb2_phy_power(phy, 1);
	mdelay(150);
	list_add_tail(&phy->list, &ti_usb_phy_list);

	return 0;
}

/**
 * ti_usb_phy_uboot_exit - usb phy uboot cleanup code
 * @index: index of this controller
 *
 * Performs cleanup of memory allocated in ti_usb_phy_uboot_init.
 * index of _this_ controller should be passed and should match with
 * the index passed in ti_usb_phy_device during init.
 *
 * Generally called from board file.
 */
void ti_usb_phy_uboot_exit(int index)
{
	struct ti_usb_phy *phy = NULL;

	list_for_each_entry(phy, &ti_usb_phy_list, list) {
		if (phy->index != index)
			continue;

		ti_usb2_phy_power(phy, 0);
#ifndef CONFIG_AM43XX
		ti_usb3_phy_power(phy, 0);
#endif
		list_del(&phy->list);
		kfree(phy);
		break;
	}
}
