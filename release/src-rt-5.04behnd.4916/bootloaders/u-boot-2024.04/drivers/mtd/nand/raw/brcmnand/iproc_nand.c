// SPDX-License-Identifier: GPL-2.0
/*
 * Code borrowed from the Linux driver
 * Copyright (C) 2015 Broadcom Corporation
 */

#include <common.h>
#include <asm/io.h>
#include <memalign.h>
#include <nand.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <dm.h>

#include "brcmnand.h"

struct iproc_nand_soc {
	struct brcmnand_soc soc;
	void __iomem *idm_base;
	void __iomem *ext_base;
};

#define IPROC_NAND_CTLR_READY_OFFSET	0x10
#define IPROC_NAND_CTLR_READY		BIT(0)

#define IPROC_NAND_IO_CTRL_OFFSET	0x00
#define IPROC_NAND_APB_LE_MODE		BIT(24)
#define IPROC_NAND_INT_CTRL_READ_ENABLE	BIT(6)

static bool iproc_nand_intc_ack(struct brcmnand_soc *soc)
{
	struct iproc_nand_soc *priv =
			container_of(soc, struct iproc_nand_soc, soc);
	void __iomem *mmio = priv->ext_base + IPROC_NAND_CTLR_READY_OFFSET;
	u32 val = brcmnand_readl(mmio);

	if (val & IPROC_NAND_CTLR_READY) {
		brcmnand_writel(IPROC_NAND_CTLR_READY, mmio);
		return true;
	}

	return false;
}

static void iproc_nand_intc_set(struct brcmnand_soc *soc, bool en)
{
	struct iproc_nand_soc *priv =
			container_of(soc, struct iproc_nand_soc, soc);
	void __iomem *mmio = priv->idm_base + IPROC_NAND_IO_CTRL_OFFSET;
	u32 val = brcmnand_readl(mmio);

	if (en)
		val |= IPROC_NAND_INT_CTRL_READ_ENABLE;
	else
		val &= ~IPROC_NAND_INT_CTRL_READ_ENABLE;

	brcmnand_writel(val, mmio);
}

static void iproc_nand_apb_access(struct brcmnand_soc *soc, bool prepare,
				  bool is_param)
{
	struct iproc_nand_soc *priv =
		container_of(soc, struct iproc_nand_soc, soc);
	void __iomem *mmio = priv->idm_base + IPROC_NAND_IO_CTRL_OFFSET;
	u32 val;

	val = brcmnand_readl(mmio);

	/*
	 * In the case of BE or when dealing with NAND data, always configure
	 * the APB bus to LE mode before accessing the FIFO and back to BE mode
	 * after the access is done
	 */
	if (IS_ENABLED(CONFIG_SYS_BIG_ENDIAN) || !is_param) {
		if (prepare)
			val |= IPROC_NAND_APB_LE_MODE;
		else
			val &= ~IPROC_NAND_APB_LE_MODE;
	} else { /* when in LE accessing the parameter page, keep APB in BE */
		val &= ~IPROC_NAND_APB_LE_MODE;
	}

	brcmnand_writel(val, mmio);
}

static int iproc_nand_probe(struct udevice *dev)
{
	struct udevice *pdev = dev;
	struct iproc_nand_soc *priv = dev_get_priv(dev);
	struct brcmnand_soc *soc;
	struct resource res;
	int ret;

	soc = &priv->soc;

	ret = dev_read_resource_byname(pdev, "iproc-idm", &res);
	if (ret)
		return ret;

	priv->idm_base = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(priv->idm_base))
		return PTR_ERR(priv->idm_base);

	ret = dev_read_resource_byname(pdev, "iproc-ext", &res);
	if (ret)
		return ret;

	priv->ext_base = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(priv->ext_base))
		return PTR_ERR(priv->ext_base);

	soc->ctlrdy_ack = iproc_nand_intc_ack;
	soc->ctlrdy_set_enabled = iproc_nand_intc_set;
	soc->prepare_data_bus = iproc_nand_apb_access;

	return brcmnand_probe(pdev, soc);
}

static const struct udevice_id iproc_nand_dt_ids[] = {
	{
		.compatible = "brcm,nand-iproc",
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(iproc_nand) = {
	.name = "iproc-nand",
	.id = UCLASS_MTD,
	.of_match = iproc_nand_dt_ids,
	.probe = iproc_nand_probe,
	.priv_auto = sizeof(struct iproc_nand_soc),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(iproc_nand), &dev);
	if (ret && ret != -ENODEV)
		pr_err("Failed to initialize %s. (error %d)\n", dev->name,
		       ret);
}
