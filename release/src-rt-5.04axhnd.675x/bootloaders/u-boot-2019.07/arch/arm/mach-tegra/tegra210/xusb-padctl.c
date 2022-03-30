// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2014-2015, NVIDIA CORPORATION.  All rights reserved.
 */

#define pr_fmt(fmt) "tegra-xusb-padctl: " fmt

#include <common.h>
#include <errno.h>
#include <dm/of_access.h>
#include <dm/ofnode.h>

#include "../xusb-padctl-common.h"

#include <asm/arch/clock.h>

#include <dt-bindings/pinctrl/pinctrl-tegra-xusb.h>

DECLARE_GLOBAL_DATA_PTR;

enum tegra210_function {
	TEGRA210_FUNC_SNPS,
	TEGRA210_FUNC_XUSB,
	TEGRA210_FUNC_UART,
	TEGRA210_FUNC_PCIE_X1,
	TEGRA210_FUNC_PCIE_X4,
	TEGRA210_FUNC_USB3,
	TEGRA210_FUNC_SATA,
	TEGRA210_FUNC_RSVD,
};

static const char *const tegra210_functions[] = {
	"snps",
	"xusb",
	"uart",
	"pcie-x1",
	"pcie-x4",
	"usb3",
	"sata",
	"rsvd",
};

static const unsigned int tegra210_otg_functions[] = {
	TEGRA210_FUNC_SNPS,
	TEGRA210_FUNC_XUSB,
	TEGRA210_FUNC_UART,
	TEGRA210_FUNC_RSVD,
};

static const unsigned int tegra210_usb_functions[] = {
	TEGRA210_FUNC_SNPS,
	TEGRA210_FUNC_XUSB,
};

static const unsigned int tegra210_pci_functions[] = {
	TEGRA210_FUNC_PCIE_X1,
	TEGRA210_FUNC_USB3,
	TEGRA210_FUNC_SATA,
	TEGRA210_FUNC_PCIE_X4,
};

#define TEGRA210_LANE(_name, _offset, _shift, _mask, _iddq, _funcs)	\
	{								\
		.name = _name,						\
		.offset = _offset,					\
		.shift = _shift,					\
		.mask = _mask,						\
		.iddq = _iddq,						\
		.num_funcs = ARRAY_SIZE(tegra210_##_funcs##_functions),	\
		.funcs = tegra210_##_funcs##_functions,			\
	}

static const struct tegra_xusb_padctl_lane tegra210_lanes[] = {
	TEGRA210_LANE("otg-0",     0x004,  0, 0x3, 0, otg),
	TEGRA210_LANE("otg-1",     0x004,  2, 0x3, 0, otg),
	TEGRA210_LANE("otg-2",     0x004,  4, 0x3, 0, otg),
	TEGRA210_LANE("otg-3",     0x004,  6, 0x3, 0, otg),
	TEGRA210_LANE("usb2-bias", 0x004, 18, 0x3, 0, otg),
	TEGRA210_LANE("hsic-0",    0x004, 14, 0x1, 0, usb),
	TEGRA210_LANE("hsic-1",    0x004, 15, 0x1, 0, usb),
	TEGRA210_LANE("pcie-0",    0x028, 12, 0x3, 1, pci),
	TEGRA210_LANE("pcie-1",    0x028, 14, 0x3, 2, pci),
	TEGRA210_LANE("pcie-2",    0x028, 16, 0x3, 3, pci),
	TEGRA210_LANE("pcie-3",    0x028, 18, 0x3, 4, pci),
	TEGRA210_LANE("pcie-4",    0x028, 20, 0x3, 5, pci),
	TEGRA210_LANE("pcie-5",    0x028, 22, 0x3, 6, pci),
	TEGRA210_LANE("pcie-6",    0x028, 24, 0x3, 7, pci),
	TEGRA210_LANE("sata-0",    0x028, 30, 0x3, 8, pci),
};

#define XUSB_PADCTL_ELPG_PROGRAM 0x024
#define  XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_VCORE_DOWN (1 << 31)
#define  XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN_EARLY (1 << 30)
#define  XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN (1 << 29)

static int tegra_xusb_padctl_enable(struct tegra_xusb_padctl *padctl)
{
	u32 value;

	if (padctl->enable++ > 0)
		return 0;

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value &= ~XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	udelay(100);

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value &= ~XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN_EARLY;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	udelay(100);

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value &= ~XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_VCORE_DOWN;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	return 0;
}

static int tegra_xusb_padctl_disable(struct tegra_xusb_padctl *padctl)
{
	u32 value;

	if (padctl->enable == 0) {
		pr_err("unbalanced enable/disable");
		return 0;
	}

	if (--padctl->enable > 0)
		return 0;

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value |= XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_VCORE_DOWN;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	udelay(100);

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value |= XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN_EARLY;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	udelay(100);

	value = padctl_readl(padctl, XUSB_PADCTL_ELPG_PROGRAM);
	value |= XUSB_PADCTL_ELPG_PROGRAM_AUX_MUX_LP0_CLAMP_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_ELPG_PROGRAM);

	return 0;
}

static int phy_prepare(struct tegra_xusb_phy *phy)
{
	int err;

	err = tegra_xusb_padctl_enable(phy->padctl);
	if (err < 0)
		return err;

	reset_set_enable(PERIPH_ID_PEX_USB_UPHY, 0);

	return 0;
}

static int phy_unprepare(struct tegra_xusb_phy *phy)
{
	reset_set_enable(PERIPH_ID_PEX_USB_UPHY, 1);

	return tegra_xusb_padctl_disable(phy->padctl);
}

#define XUSB_PADCTL_UPHY_PLL_P0_CTL1 0x360
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_FREQ_NDIV_MASK (0xff << 20)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_FREQ_NDIV(x) (((x) & 0xff) << 20)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_FREQ_MDIV_MASK (0x3 << 16)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_LOCKDET_STATUS (1 << 15)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_PWR_OVRD (1 << 4)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_ENABLE (1 << 3)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_SLEEP_MASK (0x3 << 1)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_SLEEP(x) (((x) & 0x3) << 1)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL1_IDDQ (1 << 0)

#define XUSB_PADCTL_UPHY_PLL_P0_CTL2 0x364
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_CTRL_MASK (0xffffff << 4)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_CTRL(x) (((x) & 0xffffff) << 4)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_OVRD (1 << 2)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_DONE (1 << 1)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_EN (1 << 0)

#define XUSB_PADCTL_UPHY_PLL_P0_CTL4 0x36c
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL4_TXCLKREF_EN (1 << 15)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL4_TXCLKREF_SEL_MASK (0x3 << 12)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL4_TXCLKREF_SEL(x) (((x) & 0x3) << 12)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL4_REFCLKBUF_EN (1 << 8)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL4_REFCLK_SEL_MASK (0xf << 4)

#define XUSB_PADCTL_UPHY_PLL_P0_CTL5 0x370
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL5_DCO_CTRL_MASK (0xff << 16)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL5_DCO_CTRL(x) (((x) & 0xff) << 16)

#define XUSB_PADCTL_UPHY_PLL_P0_CTL8 0x37c
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_DONE (1 << 31)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_OVRD (1 << 15)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_CLK_EN (1 << 13)
#define  XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_EN (1 << 12)

#define CLK_RST_XUSBIO_PLL_CFG0 0x51c
#define  CLK_RST_XUSBIO_PLL_CFG0_SEQ_ENABLE (1 << 24)
#define  CLK_RST_XUSBIO_PLL_CFG0_PADPLL_SLEEP_IDDQ (1 << 13)
#define  CLK_RST_XUSBIO_PLL_CFG0_PADPLL_USE_LOCKDET (1 << 6)
#define  CLK_RST_XUSBIO_PLL_CFG0_CLK_ENABLE_SWCTL (1 << 2)
#define  CLK_RST_XUSBIO_PLL_CFG0_PADPLL_RESET_SWCTL (1 << 0)

static int pcie_phy_enable(struct tegra_xusb_phy *phy)
{
	struct tegra_xusb_padctl *padctl = phy->padctl;
	unsigned long start;
	u32 value;

	debug("> %s(phy=%p)\n", __func__, phy);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL2);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_CTRL_MASK;
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_CTRL(0x136);
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL2);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL5);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL5_DCO_CTRL_MASK;
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL5_DCO_CTRL(0x2a);
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL5);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL1);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL1_PWR_OVRD;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL2);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_OVRD;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL2);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL8);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_OVRD;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL8);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL4);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL4_TXCLKREF_SEL_MASK;
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL4_REFCLK_SEL_MASK;
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL4_TXCLKREF_SEL(2);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL4_TXCLKREF_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL4);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL1);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL1_FREQ_MDIV_MASK;
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL1_FREQ_NDIV_MASK;
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL1_FREQ_NDIV(25);
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL1);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL1_IDDQ;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL1);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL1_SLEEP_MASK;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL1);

	udelay(1);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL4);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL4_REFCLKBUF_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL4);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL2);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL2);

	debug("  waiting for calibration\n");

	start = get_timer(0);

	while (get_timer(start) < 250) {
		value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL2);
		if (value & XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_DONE)
			break;
	}
	if (!(value & XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_DONE)) {
		debug("  timeout\n");
		return -ETIMEDOUT;
	}
	debug("  done\n");

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL2);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL2);

	debug("  waiting for calibration to stop\n");

	start = get_timer(0);

	while (get_timer(start) < 250) {
		value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL2);
		if ((value & XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_DONE) == 0)
			break;
	}
	if (value & XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_DONE) {
		debug("  timeout\n");
		return -ETIMEDOUT;
	}
	debug("  done\n");

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL1);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL1_ENABLE;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL1);

	debug("  waiting for PLL to lock...\n");
	start = get_timer(0);

	while (get_timer(start) < 250) {
		value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL1);
		if (value & XUSB_PADCTL_UPHY_PLL_P0_CTL1_LOCKDET_STATUS)
			break;
	}
	if (!(value & XUSB_PADCTL_UPHY_PLL_P0_CTL1_LOCKDET_STATUS)) {
		debug("  timeout\n");
		return -ETIMEDOUT;
	}
	debug("  done\n");

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL8);
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_CLK_EN;
	value |= XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL8);

	debug("  waiting for register calibration...\n");
	start = get_timer(0);

	while (get_timer(start) < 250) {
		value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL8);
		if (value & XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_DONE)
			break;
	}
	if (!(value & XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_DONE)) {
		debug("  timeout\n");
		return -ETIMEDOUT;
	}
	debug("  done\n");

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL8);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL8);

	debug("  waiting for register calibration to stop...\n");
	start = get_timer(0);

	while (get_timer(start) < 250) {
		value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL8);
		if ((value & XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_DONE) == 0)
			break;
	}
	if (value & XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_DONE) {
		debug("  timeout\n");
		return -ETIMEDOUT;
	}
	debug("  done\n");

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL8);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_CLK_EN;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL8);

	value = readl(NV_PA_CLK_RST_BASE + CLK_RST_XUSBIO_PLL_CFG0);
	value &= ~CLK_RST_XUSBIO_PLL_CFG0_PADPLL_RESET_SWCTL;
	value &= ~CLK_RST_XUSBIO_PLL_CFG0_CLK_ENABLE_SWCTL;
	value |= CLK_RST_XUSBIO_PLL_CFG0_PADPLL_USE_LOCKDET;
	value |= CLK_RST_XUSBIO_PLL_CFG0_PADPLL_SLEEP_IDDQ;
	writel(value, NV_PA_CLK_RST_BASE + CLK_RST_XUSBIO_PLL_CFG0);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL1);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL1_PWR_OVRD;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL1);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL2);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL2_CAL_OVRD;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL2);

	value = padctl_readl(padctl, XUSB_PADCTL_UPHY_PLL_P0_CTL8);
	value &= ~XUSB_PADCTL_UPHY_PLL_P0_CTL8_RCAL_OVRD;
	padctl_writel(padctl, value, XUSB_PADCTL_UPHY_PLL_P0_CTL8);

	udelay(1);

	value = readl(NV_PA_CLK_RST_BASE + CLK_RST_XUSBIO_PLL_CFG0);
	value |= CLK_RST_XUSBIO_PLL_CFG0_SEQ_ENABLE;
	writel(value, NV_PA_CLK_RST_BASE + CLK_RST_XUSBIO_PLL_CFG0);

	debug("< %s()\n", __func__);
	return 0;
}

static int pcie_phy_disable(struct tegra_xusb_phy *phy)
{
	return 0;
}

static const struct tegra_xusb_phy_ops pcie_phy_ops = {
	.prepare = phy_prepare,
	.enable = pcie_phy_enable,
	.disable = pcie_phy_disable,
	.unprepare = phy_unprepare,
};

static struct tegra_xusb_phy tegra210_phys[] = {
	{
		.type = TEGRA_XUSB_PADCTL_PCIE,
		.ops = &pcie_phy_ops,
		.padctl = &padctl,
	},
};

static const struct tegra_xusb_padctl_soc tegra210_socdata = {
	.lanes = tegra210_lanes,
	.num_lanes = ARRAY_SIZE(tegra210_lanes),
	.functions = tegra210_functions,
	.num_functions = ARRAY_SIZE(tegra210_functions),
	.phys = tegra210_phys,
	.num_phys = ARRAY_SIZE(tegra210_phys),
};

void tegra_xusb_padctl_init(void)
{
	ofnode nodes[1];
	int count = 0;
	int ret;

	debug("%s: start\n", __func__);
	if (of_live_active()) {
		struct device_node *np = of_find_compatible_node(NULL, NULL,
						"nvidia,tegra210-xusb-padctl");

		debug("np=%p\n", np);
		if (np) {
			nodes[0] = np_to_ofnode(np);
			count = 1;
		}
	} else {
		int node_offsets[1];
		int i;

		count = fdtdec_find_aliases_for_id(gd->fdt_blob, "padctl",
				COMPAT_NVIDIA_TEGRA210_XUSB_PADCTL,
				node_offsets, ARRAY_SIZE(node_offsets));
		for (i = 0; i < count; i++)
			nodes[i] = offset_to_ofnode(node_offsets[i]);
	}

	ret = tegra_xusb_process_nodes(nodes, count, &tegra210_socdata);
	debug("%s: done, ret=%d\n", __func__, ret);
}
