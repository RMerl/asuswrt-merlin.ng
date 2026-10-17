// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <asm/io.h>
#include <memalign.h>
#include <nand.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <dm.h>
#include <linux/printk.h>

#include "brcmnand.h"

struct bcm6368_nand_soc {
	struct brcmnand_soc soc;
	void __iomem *base;
};

#define soc_to_priv(_soc) container_of(_soc, struct bcm6368_nand_soc, soc)

#define BCM6368_NAND_INT		0x00
#define  BCM6368_NAND_STATUS_SHIFT	0
#define  BCM6368_NAND_STATUS_MASK	(0xfff << BCM6368_NAND_STATUS_SHIFT)
#define  BCM6368_NAND_ENABLE_SHIFT	16
#define  BCM6368_NAND_ENABLE_MASK	(0xffff << BCM6368_NAND_ENABLE_SHIFT)

enum {
	BCM6368_NP_READ		= BIT(0),
	BCM6368_BLOCK_ERASE	= BIT(1),
	BCM6368_COPY_BACK	= BIT(2),
	BCM6368_PAGE_PGM	= BIT(3),
	BCM6368_CTRL_READY	= BIT(4),
	BCM6368_DEV_RBPIN	= BIT(5),
	BCM6368_ECC_ERR_UNC	= BIT(6),
	BCM6368_ECC_ERR_CORR	= BIT(7),
};

static bool bcm6368_nand_intc_ack(struct brcmnand_soc *soc)
{
	struct bcm6368_nand_soc *priv = soc_to_priv(soc);
	void __iomem *mmio = priv->base + BCM6368_NAND_INT;
	u32 val = brcmnand_readl(mmio);

	if (val & (BCM6368_CTRL_READY << BCM6368_NAND_STATUS_SHIFT)) {
		/* Ack interrupt */
		val &= ~BCM6368_NAND_STATUS_MASK;
		val |= BCM6368_CTRL_READY << BCM6368_NAND_STATUS_SHIFT;
		brcmnand_writel(val, mmio);
		return true;
	}

	return false;
}

static void bcm6368_nand_intc_set(struct brcmnand_soc *soc, bool en)
{
	struct bcm6368_nand_soc *priv = soc_to_priv(soc);
	void __iomem *mmio = priv->base + BCM6368_NAND_INT;
	u32 val = brcmnand_readl(mmio);

	/* Don't ack any interrupts */
	val &= ~BCM6368_NAND_STATUS_MASK;

	if (en)
		val |= BCM6368_CTRL_READY << BCM6368_NAND_ENABLE_SHIFT;
	else
		val &= ~(BCM6368_CTRL_READY << BCM6368_NAND_ENABLE_SHIFT);

	brcmnand_writel(val, mmio);
}

static int bcm6368_nand_probe(struct udevice *dev)
{
	struct bcm6368_nand_soc *priv = dev_get_priv(dev);
	struct brcmnand_soc *soc = &priv->soc;

	priv->base = dev_remap_addr_name(dev, "nand-int-base");
	if (!priv->base)
		return -EINVAL;

	soc->ctlrdy_ack = bcm6368_nand_intc_ack;
	soc->ctlrdy_set_enabled = bcm6368_nand_intc_set;

	/* Disable and ack all interrupts  */
	brcmnand_writel(0, priv->base + BCM6368_NAND_INT);
	brcmnand_writel(BCM6368_NAND_STATUS_MASK,
			priv->base + BCM6368_NAND_INT);

	return brcmnand_probe(dev, soc);
}

static const struct udevice_id bcm6368_nand_dt_ids[] = {
	{
		.compatible = "brcm,nand-bcm6368",
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(bcm6368_nand) = {
	.name = "bcm6368-nand",
	.id = UCLASS_MTD,
	.of_match = bcm6368_nand_dt_ids,
	.probe = bcm6368_nand_probe,
	.priv_auto	= sizeof(struct bcm6368_nand_soc),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(bcm6368_nand), &dev);
	if (ret && ret != -ENODEV)
		pr_err("Failed to initialize %s. (error %d)\n", dev->name,
		       ret);
}
