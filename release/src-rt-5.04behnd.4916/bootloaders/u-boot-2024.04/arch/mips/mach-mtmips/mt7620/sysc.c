// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author:  Weijie Gao <weijie.gao@mediatek.com>
 *
 * Misc driver for manipulating System control registers
 */

#include <dm.h>
#include <misc.h>
#include <asm/io.h>
#include <asm/addrspace.h>
#include <dm/device_compat.h>
#include <mach/mt7620-sysc.h>
#include "mt7620.h"

struct mt7620_sysc_priv {
	void __iomem *base;
};

static int mt7620_sysc_read(struct udevice *dev, int offset, void *buf,
			    int size)
{
	struct mt7620_sysc_priv *priv = dev_get_priv(dev);
	u32 val;

	if (offset % sizeof(u32) || size != sizeof(u32) ||
	    offset >= SYSCTL_SIZE)
		return -EINVAL;

	val = readl(priv->base + offset);

	if (buf)
		*(u32 *)buf = val;

	return 0;
}

static int mt7620_sysc_write(struct udevice *dev, int offset, const void *buf,
			     int size)
{
	struct mt7620_sysc_priv *priv = dev_get_priv(dev);
	u32 val;

	if (offset % sizeof(u32) || size != sizeof(u32) ||
	    offset >= SYSCTL_SIZE || !buf)
		return -EINVAL;

	val = *(u32 *)buf;
	writel(val, priv->base + offset);

	return 0;
}

static int mt7620_sysc_ioctl(struct udevice *dev, unsigned long request,
			     void *buf)
{
	struct mt7620_sysc_priv *priv = dev_get_priv(dev);
	struct mt7620_sysc_chip_rev *chip_rev;
	struct mt7620_sysc_clks *clks;
	u32 val, shift;

	if (!buf)
		return -EINVAL;

	switch (request) {
	case MT7620_SYSC_IOCTL_GET_CLK:
		clks = buf;
		mt7620_get_clks(&clks->cpu_clk, &clks->sys_clk,
				&clks->xtal_clk);

		val = readl(priv->base + SYSCTL_CLKCFG0_REG);
		if (val & PERI_CLK_SEL)
			clks->peri_clk = clks->xtal_clk;
		else
			clks->peri_clk = 40000000;

		return 0;

	case MT7620_SYSC_IOCTL_GET_CHIP_REV:
		chip_rev = buf;

		val = readl(priv->base + SYSCTL_CHIP_REV_ID_REG);

		chip_rev->bga = !!(val & PKG_ID);
		chip_rev->ver = (val & VER_M) >> VER_S;
		chip_rev->eco = (val & ECO_M) >> ECO_S;

		return 0;

	case MT7620_SYSC_IOCTL_SET_GE1_MODE:
	case MT7620_SYSC_IOCTL_SET_GE2_MODE:
		val = *(u32 *)buf;

		if (val > MT7620_SYSC_GE_ESW_PHY)
			return -EINVAL;

		if (request == MT7620_SYSC_IOCTL_SET_GE1_MODE)
			shift = GE1_MODE_S;
		else
			shift = GE2_MODE_S;

		clrsetbits_32(priv->base + SYSCTL_SYSCFG1_REG,
			      GE_MODE_M << shift, val << shift);

		return 0;

	case MT7620_SYSC_IOCTL_SET_USB_MODE:
		val = *(u32 *)buf;

		if (val == MT7620_SYSC_USB_DEVICE_MODE)
			val = 0;
		else if (val == MT7620_SYSC_USB_HOST_MODE)
			val = USB0_HOST_MODE;

		clrsetbits_32(priv->base + SYSCTL_SYSCFG1_REG,
			      USB0_HOST_MODE, val);

		return 0;

	case MT7620_SYSC_IOCTL_SET_PCIE_MODE:
		val = *(u32 *)buf;

		if (val == MT7620_SYSC_PCIE_EP_MODE)
			val = 0;
		else if (val == MT7620_SYSC_PCIE_RC_MODE)
			val = PCIE_RC_MODE;

		clrsetbits_32(priv->base + SYSCTL_SYSCFG1_REG,
			      PCIE_RC_MODE, val);

		return 0;

	default:
		return -EINVAL;
	}
}

static int mt7620_sysc_probe(struct udevice *dev)
{
	struct mt7620_sysc_priv *priv = dev_get_priv(dev);

	priv->base = dev_remap_addr_index(dev, 0);
	if (!priv->base) {
		dev_err(dev, "failed to map sysc registers\n");
		return -EINVAL;
	}

	return 0;
}

static struct misc_ops mt7620_sysc_ops = {
	.read = mt7620_sysc_read,
	.write = mt7620_sysc_write,
	.ioctl = mt7620_sysc_ioctl,
};

static const struct udevice_id mt7620_sysc_ids[] = {
	{ .compatible = "mediatek,mt7620-sysc" },
	{ }
};

U_BOOT_DRIVER(mt7620_sysc) = {
	.name		= "mt7620_sysc",
	.id		= UCLASS_MISC,
	.of_match	= mt7620_sysc_ids,
	.probe		= mt7620_sysc_probe,
	.ops		= &mt7620_sysc_ops,
	.priv_auto	= sizeof(struct mt7620_sysc_priv),
	.flags = DM_FLAG_PRE_RELOC,
};
