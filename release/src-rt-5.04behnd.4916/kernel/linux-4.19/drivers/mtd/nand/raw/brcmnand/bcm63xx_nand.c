#if defined(CONFIG_BCM_KF_MTD_BCMNAND)
/*
<:copyright-BRCM:2016:GPL/GPL:standard

   Copyright (c) 2016 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

#include <linux/device.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include "brcmnand.h"
#include "board.h"
#include "bcm_dgasp.h"
#include "flash_api.h"

struct bcm63xx_nand_soc_priv {
	void __iomem *base;
};

#define BCM63XX_NAND_INT_STATUS		0x00
#define BCM63XX_NAND_INT_STATUS_MASK	0xfff
#define BCM63XX_NAND_INT_STATUS_CTLRDY  (0x1<<4)
#define BCM63XX_NAND_INT_EN		0x04
#define BCM63XX_NAND_INT_EN_CTLRDY	(0x1<<4)

static bool bcm63xx_nand_intc_ack(struct brcmnand_soc *soc)
{
	struct bcm63xx_nand_soc_priv *priv = soc->priv;
	void __iomem *mmio = priv->base + BCM63XX_NAND_INT_STATUS;
	u32 val = brcmnand_readl(mmio);

	if (val & BCM63XX_NAND_INT_STATUS_CTLRDY) {
		brcmnand_writel(val & ~BCM63XX_NAND_INT_STATUS_CTLRDY, mmio);
		return true;
	}

	return false;
}

static void bcm63xx_nand_intc_set(struct brcmnand_soc *soc, bool en)
{
	struct bcm63xx_nand_soc_priv *priv = soc->priv;
	void __iomem *mmio = priv->base + BCM63XX_NAND_INT_EN;
	u32 val = brcmnand_readl(mmio);

	if (en)
		val |= BCM63XX_NAND_INT_EN_CTLRDY;
	else
		val &= ~BCM63XX_NAND_INT_EN_CTLRDY;

	brcmnand_writel(val, mmio);
}

static int bcm63xx_check_dying_gasp(struct brcmnand_soc *soc)
{
	return kerSysIsDyingGaspTriggered();
}


static int bcm63xx_nand_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct bcm63xx_nand_soc_priv *priv;
	struct brcmnand_soc *soc;
	struct resource *res;

	soc = devm_kzalloc(dev, sizeof(*soc), GFP_KERNEL);
	if (!soc)
		return -ENOMEM;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "nand-int-base");
	priv->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

	soc->pdev = pdev;
	soc->priv = priv;
	soc->ctlrdy_ack = bcm63xx_nand_intc_ack;
	soc->ctlrdy_set_enabled = bcm63xx_nand_intc_set;
	soc->check_dying_gasp = bcm63xx_check_dying_gasp;

	return brcmnand_probe(pdev, soc);
}

static const struct of_device_id bcm63xx_nand_of_match[] = {
	{ .compatible = "brcm,nand-bcm63xx" },
	{},
};
MODULE_DEVICE_TABLE(of, bcm63xx_nand_of_match);

static struct platform_driver bcm63xx_nand_driver = {
	.probe			= bcm63xx_nand_probe,
	.remove			= brcmnand_remove,
	.driver = {
		.name		= "bcm63xx_nand",
		.pm		= &brcmnand_pm_ops,
		.of_match_table	= bcm63xx_nand_of_match,
	}
};
module_platform_driver(bcm63xx_nand_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Brian Norris");
MODULE_DESCRIPTION("NAND driver for BCM63XX devices");
#endif
