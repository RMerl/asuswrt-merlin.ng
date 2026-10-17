// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2009-2010, 2013 Freescale Semiconductor, Inc.
 *	Jun-jie Zhang <b18070@freescale.com>
 *	Mingkai Hu <Mingkai.hu@freescale.com>
 */

#include <common.h>
#include <miiphy.h>
#include <phy.h>
#include <fsl_mdio.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <tsec.h>

#ifdef CONFIG_DM_MDIO
struct tsec_mdio_priv {
	struct tsec_mii_mng __iomem *regs;
};
#endif

void tsec_local_mdio_write(struct tsec_mii_mng __iomem *phyregs, int port_addr,
		int dev_addr, int regnum, int value)
{
	int timeout = 1000000;

	out_be32(&phyregs->miimadd, (port_addr << 8) | (regnum & 0x1f));
	out_be32(&phyregs->miimcon, value);
	/* Memory barrier */
	mb();

	while ((in_be32(&phyregs->miimind) & MIIMIND_BUSY) && timeout--)
		;
}

int tsec_local_mdio_read(struct tsec_mii_mng __iomem *phyregs, int port_addr,
		int dev_addr, int regnum)
{
	int value;
	int timeout = 1000000;

	/* Put the address of the phy, and the register number into MIIMADD */
	out_be32(&phyregs->miimadd, (port_addr << 8) | (regnum & 0x1f));

	/* Clear the command register, and wait */
	out_be32(&phyregs->miimcom, 0);
	/* Memory barrier */
	mb();

	/* Initiate a read command, and wait */
	out_be32(&phyregs->miimcom, MIIMCOM_READ_CYCLE);
	/* Memory barrier */
	mb();

	/* Wait for the the indication that the read is done */
	while ((in_be32(&phyregs->miimind) & (MIIMIND_NOTVALID | MIIMIND_BUSY))
			&& timeout--)
		;

	/* Grab the value read from the PHY */
	value = in_be32(&phyregs->miimstat);

	return value;
}

#if defined(CONFIG_PHYLIB)
static int fsl_pq_mdio_reset(struct mii_dev *bus)
{
	struct tsec_mii_mng __iomem *regs;
#ifndef CONFIG_DM_MDIO
	regs = (struct tsec_mii_mng __iomem *)bus->priv;
#else
	struct tsec_mdio_priv *priv;

	if (!bus->priv)
		return -EINVAL;

	priv = dev_get_priv(bus->priv);
	regs = priv->regs;
#endif

	/* Reset MII (due to new addresses) */
	out_be32(&regs->miimcfg, MIIMCFG_RESET_MGMT);

	out_be32(&regs->miimcfg, MIIMCFG_INIT_VALUE);

	while (in_be32(&regs->miimind) & MIIMIND_BUSY)
		;

	return 0;
}
#endif

int tsec_phy_read(struct mii_dev *bus, int addr, int dev_addr, int regnum)
{
	struct tsec_mii_mng __iomem *phyregs;
#ifndef CONFIG_DM_MDIO
	phyregs = (struct tsec_mii_mng __iomem *)bus->priv;
#else
	struct tsec_mdio_priv *priv;

	if (!bus->priv)
		return -EINVAL;

	priv = dev_get_priv(bus->priv);
	phyregs = priv->regs;
#endif

	return tsec_local_mdio_read(phyregs, addr, dev_addr, regnum);
}

int tsec_phy_write(struct mii_dev *bus, int addr, int dev_addr, int regnum,
			u16 value)
{
	struct tsec_mii_mng __iomem *phyregs;
#ifndef CONFIG_DM_MDIO
	phyregs = (struct tsec_mii_mng __iomem *)bus->priv;
#else
	struct tsec_mdio_priv *priv;

	if (!bus->priv)
		return -EINVAL;

	priv = dev_get_priv(bus->priv);
	phyregs = priv->regs;
#endif

	tsec_local_mdio_write(phyregs, addr, dev_addr, regnum, value);

	return 0;
}

#ifndef CONFIG_DM_MDIO
int fsl_pq_mdio_init(struct bd_info *bis, struct fsl_pq_mdio_info *info)
{
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate FSL MDIO bus\n");
		return -1;
	}

	bus->read = tsec_phy_read;
	bus->write = tsec_phy_write;
	bus->reset = fsl_pq_mdio_reset;
	strcpy(bus->name, info->name);

	bus->priv = (void *)info->regs;

	return mdio_register(bus);
}
#else /* CONFIG_DM_MDIO */
#if defined(CONFIG_PHYLIB)
static int tsec_mdio_read(struct udevice *dev, int addr, int devad, int reg)
{
	struct mdio_perdev_priv *pdata = (dev) ? dev_get_uclass_priv(dev) :
						 NULL;

	if (pdata && pdata->mii_bus)
		return tsec_phy_read(pdata->mii_bus, addr, devad, reg);

	return -1;
}

static int tsec_mdio_write(struct udevice *dev, int addr, int devad, int reg,
			   u16 val)
{
	struct mdio_perdev_priv *pdata = (dev) ? dev_get_uclass_priv(dev) :
						 NULL;

	if (pdata && pdata->mii_bus)
		return tsec_phy_write(pdata->mii_bus, addr, devad, reg, val);

	return -1;
}

static int tsec_mdio_reset(struct udevice *dev)
{
	struct mdio_perdev_priv *pdata = (dev) ? dev_get_uclass_priv(dev) :
						 NULL;

	if (pdata && pdata->mii_bus)
		return fsl_pq_mdio_reset(pdata->mii_bus);

	return -1;
}

static const struct mdio_ops tsec_mdio_ops = {
	.read = tsec_mdio_read,
	.write = tsec_mdio_write,
	.reset = tsec_mdio_reset,
};

static struct fsl_pq_mdio_data etsec2_data = {
	.mdio_regs_off = TSEC_MDIO_REGS_OFFSET,
};

static struct fsl_pq_mdio_data gianfar_data = {
	.mdio_regs_off = 0x0,
};

static struct fsl_pq_mdio_data fman_data = {
	.mdio_regs_off = 0x0,
};

static const struct udevice_id tsec_mdio_ids[] = {
	{ .compatible = "fsl,gianfar-tbi", .data = (ulong)&gianfar_data },
	{ .compatible = "fsl,gianfar-mdio", .data = (ulong)&gianfar_data },
	{ .compatible = "fsl,etsec2-tbi", .data = (ulong)&etsec2_data },
	{ .compatible = "fsl,etsec2-mdio", .data = (ulong)&etsec2_data },
	{ .compatible = "fsl,fman-mdio", .data = (ulong)&fman_data },
	{}
};

static int tsec_mdio_probe(struct udevice *dev)
{
	struct fsl_pq_mdio_data *data;
	struct tsec_mdio_priv *priv = (dev) ? dev_get_priv(dev) : NULL;
	struct mdio_perdev_priv *pdata = (dev) ? dev_get_uclass_priv(dev) :
						 NULL;

	if (!dev) {
		printf("%s dev = NULL\n", __func__);
		return -1;
	}
	if (!priv) {
		printf("dev_get_priv(dev %p) = NULL\n", dev);
		return -1;
	}

	data = (struct fsl_pq_mdio_data *)dev_get_driver_data(dev);
	priv->regs = dev_remap_addr(dev) + data->mdio_regs_off;
	debug("%s priv %p @ regs %p, pdata %p\n", __func__,
	      priv, priv->regs, pdata);

	return 0;
}

static int tsec_mdio_remove(struct udevice *dev)
{
	return 0;
}

U_BOOT_DRIVER(tsec_mdio) = {
	.name = "tsec_mdio",
	.id = UCLASS_MDIO,
	.of_match = tsec_mdio_ids,
	.probe = tsec_mdio_probe,
	.remove = tsec_mdio_remove,
	.ops = &tsec_mdio_ops,
	.priv_auto	= sizeof(struct tsec_mdio_priv),
	.plat_auto	= sizeof(struct mdio_perdev_priv),
};
#endif /* CONFIG_PHYLIB */
#endif /* CONFIG_DM_MDIO */
