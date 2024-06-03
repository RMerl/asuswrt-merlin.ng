// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright © 2010-2019 Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <common.h>
#include <asm/io.h>
#include <memalign.h>
#include <nand.h>
#include <clk.h>
#include <linux/ioport.h>
#include <linux/completion.h>
#include <linux/errno.h>
#include <linux/log2.h>
#include <asm/processor.h>
#include <dm.h>

#include "brcmnand.h"
#include "brcmnand_compat.h"
#if defined(CONFIG_BCMBCA_STRAP)
#include "bcm_strap_drv.h"
#endif
struct bcmbca_nand_strap_mapping {
	int ecc_lvl;
	int spare_size;
	int sect_1k;
};

/* use boot_sel[2:0] as index to table */
#define MAX_NAND_ECC_STRAP			8
struct bcmbca_nand_strap_mapping strap_mapping[MAX_NAND_ECC_STRAP] = {
	{ 0,  16, 0}, /* ECC disabled */
	{15,  16, 0}, /* 1 bit Hamming ECC */
	{ 4,  16, 0}, /* BCH-n ECC */
	{ 8,  27, 0},
	{12,  27, 0},
	{24,  54, 1},
	{40,  90, 1},
	{60, 120, 1},
};

struct bcmbca_nand_soc {
	struct brcmnand_soc soc;
	void __iomem *base;
};

#define BCMBCA_NAND_INT		0x00
#define BCMBCA_NAND_STATUS_SHIFT	0
#define BCMBCA_NAND_STATUS_MASK	(0xfff << BCMBCA_NAND_STATUS_SHIFT)

#define BCMBCA_NAND_INT_EN		0x04
#define BCMBCA_NAND_ENABLE_SHIFT	0
#define BCMBCA_NAND_ENABLE_MASK	(0xffff << BCMBCA_NAND_ENABLE_SHIFT)

#define BCMBCA_NAND_BASE_ACC_OFFSET			0x50
#define BCMBCA_NAND_BASE_ACC_ECC_SHIFT		16
#define BCMBCA_NAND_BASE_ACC_ECC_MASK		(0x1f << BCMBCA_NAND_BASE_ACC_ECC_SHIFT)
#define BCMBCA_NAND_BASE_ACC_SPARE_SHIFT	0
#define BCMBCA_NAND_BASE_ACC_SPARE_MASK		(0x7f << BCMBCA_NAND_BASE_ACC_SPARE_SHIFT)
#define BCMBCA_NAND_BASE_ACC_1KSECT_SHIFT	7
#define BCMBCA_NAND_BASE_ACC_1KSECT_MASK	(0x1 << BCMBCA_NAND_BASE_ACC_1KSECT_SHIFT)

enum {
	BCMBCA_NP_READ		= BIT(0),
	BCMBCA_BLOCK_ERASE		= BIT(1),
	BCMBCA_COPY_BACK		= BIT(2),
	BCMBCA_PAGE_PGM		= BIT(3),
	BCMBCA_CTRL_READY		= BIT(4),
	BCMBCA_DEV_RBPIN		= BIT(5),
	BCMBCA_ECC_ERR_UNC		= BIT(6),
	BCMBCA_ECC_ERR_CORR	= BIT(7),
};

#ifndef CONFIG_SMC_BASED
static bool bcmbca_nand_intc_ack(struct brcmnand_soc *soc)
{
	struct bcmbca_nand_soc *priv =
			container_of(soc, struct bcmbca_nand_soc, soc);
	void __iomem *mmio = priv->base + BCMBCA_NAND_INT;
	u32 val = brcmnand_readl(mmio);

	if (val & (BCMBCA_CTRL_READY << BCMBCA_NAND_STATUS_SHIFT)) {
		/* Ack interrupt */
		val &= ~BCMBCA_NAND_STATUS_MASK;
		val |= BCMBCA_CTRL_READY << BCMBCA_NAND_STATUS_SHIFT;
		brcmnand_writel(val, mmio);
		return true;
	}

	return false;
}

static void bcmbca_nand_intc_set(struct brcmnand_soc *soc, bool en)
{
	struct bcmbca_nand_soc *priv =
			container_of(soc, struct bcmbca_nand_soc, soc);
	void __iomem *mmio = priv->base + BCMBCA_NAND_INT_EN;
	u32 val = brcmnand_readl(mmio);

	/* Don't ack any interrupts */
	val &= ~BCMBCA_NAND_STATUS_MASK;

	if (en)
		val |= BCMBCA_CTRL_READY << BCMBCA_NAND_ENABLE_SHIFT;
	else
		val &= ~(BCMBCA_CTRL_READY << BCMBCA_NAND_ENABLE_SHIFT);

	brcmnand_writel(val, mmio);
}
#endif

static int bcmbca_nand_probe(struct udevice *dev)
{
	struct udevice *pdev = dev;
	struct bcmbca_nand_soc *priv = dev_get_priv(dev);
	struct brcmnand_soc *soc = &priv->soc;

#ifndef CONFIG_SMC_BASED
	struct resource res;
	void __iomem *nand_base;
	u32 value;
	nand_ecc_t ecc;

	dev_read_resource_byname(pdev, "nand-int-base", &res);
	priv->base = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

	soc->ctlrdy_ack = bcmbca_nand_intc_ack;
	soc->ctlrdy_set_enabled = bcmbca_nand_intc_set;

	/* Disable and ack all interrupts  */
	brcmnand_writel(0, priv->base + BCMBCA_NAND_INT_EN);
	brcmnand_writel(0, priv->base + BCMBCA_NAND_INT);

	/* acc reg ecc level and spare area size fix up for non NAND booting */
#if defined(CONFIG_BCMBCA_STRAP)	
	if (bcm_get_boot_device() != BOOT_DEVICE_NAND) {
		dev_read_resource_byname(pdev, "nand", &res);
		nand_base = devm_ioremap(dev, res.start, resource_size(&res));
		if (!IS_ERR_OR_NULL(nand_base)) {
			value = readl_relaxed(nand_base + BCMBCA_NAND_BASE_ACC_OFFSET);
			value &= ~(BCMBCA_NAND_BASE_ACC_ECC_MASK |\
					   BCMBCA_NAND_BASE_ACC_SPARE_MASK |\
					   BCMBCA_NAND_BASE_ACC_1KSECT_MASK);
			ecc = bcm_get_nand_ecc();
			value |=
				(strap_mapping[ecc].ecc_lvl) << BCMBCA_NAND_BASE_ACC_ECC_SHIFT;
			value |=
				(strap_mapping[ecc].spare_size) << BCMBCA_NAND_BASE_ACC_SPARE_SHIFT;
			value |=
				(strap_mapping[ecc].sect_1k) << BCMBCA_NAND_BASE_ACC_1KSECT_SHIFT;
			debug("fix up nand acc register to 0x%x\n", value);	
			writel_relaxed(value, nand_base + BCMBCA_NAND_BASE_ACC_OFFSET);
			iounmap(nand_base);
		}
	}
#endif
#endif
	return brcmnand_probe(pdev, soc);
}

static const struct udevice_id bcmbca_nand_dt_ids[] = {
	{
		.compatible = "brcm,nand-bcmbca",
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(brcm_nand) = {
	.name = "brcm-nand",
	.id = UCLASS_MTD,
	.of_match = bcmbca_nand_dt_ids,
	.probe = bcmbca_nand_probe,
	.priv_auto_alloc_size = sizeof(struct bcmbca_nand_soc),
};


MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("David Regan");
MODULE_DESCRIPTION("NAND driver for Broadcom chips");
MODULE_ALIAS("platform:brcmnand");
