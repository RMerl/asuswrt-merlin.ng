// SPDX-License-Identifier: GPL-2.0+
/*
 * ENETC ethernet controller driver
 * Copyright 2019 NXP
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <pci.h>
#include <miiphy.h>
#include <asm/io.h>
#include <asm/processor.h>
#include <miiphy.h>

#include "fsl_enetc.h"

static void enetc_mdio_wait_bsy(struct enetc_mdio_priv *priv)
{
	int to = 10000;

	while ((enetc_read(priv, ENETC_MDIO_CFG) & ENETC_EMDIO_CFG_BSY) &&
	       --to)
		cpu_relax();
	if (!to)
		printf("T");
}

int enetc_mdio_read_priv(struct enetc_mdio_priv *priv, int addr, int devad,
			 int reg)
{
	if (devad == MDIO_DEVAD_NONE)
		enetc_write(priv, ENETC_MDIO_CFG, ENETC_EMDIO_CFG_C22);
	else
		enetc_write(priv, ENETC_MDIO_CFG, ENETC_EMDIO_CFG_C45);
	enetc_mdio_wait_bsy(priv);

	if (devad == MDIO_DEVAD_NONE) {
		enetc_write(priv, ENETC_MDIO_CTL, ENETC_MDIO_CTL_READ |
			    (addr << 5) | reg);
	} else {
		enetc_write(priv, ENETC_MDIO_CTL, (addr << 5) + devad);
		enetc_mdio_wait_bsy(priv);

		enetc_write(priv, ENETC_MDIO_STAT, reg);
		enetc_mdio_wait_bsy(priv);

		enetc_write(priv, ENETC_MDIO_CTL, ENETC_MDIO_CTL_READ |
			    (addr << 5) | devad);
	}

	enetc_mdio_wait_bsy(priv);
	if (enetc_read(priv, ENETC_MDIO_CFG) & ENETC_EMDIO_CFG_RD_ER)
		return ENETC_MDIO_READ_ERR;

	return enetc_read(priv, ENETC_MDIO_DATA);
}

int enetc_mdio_write_priv(struct enetc_mdio_priv *priv, int addr, int devad,
			  int reg, u16 val)
{
	if (devad == MDIO_DEVAD_NONE)
		enetc_write(priv, ENETC_MDIO_CFG, ENETC_EMDIO_CFG_C22);
	else
		enetc_write(priv, ENETC_MDIO_CFG, ENETC_EMDIO_CFG_C45);
	enetc_mdio_wait_bsy(priv);

	if (devad != MDIO_DEVAD_NONE) {
		enetc_write(priv, ENETC_MDIO_CTL, (addr << 5) + devad);
		enetc_write(priv, ENETC_MDIO_STAT, reg);
	} else {
		enetc_write(priv, ENETC_MDIO_CTL, (addr << 5) + reg);
	}
	enetc_mdio_wait_bsy(priv);

	enetc_write(priv, ENETC_MDIO_DATA, val);
	enetc_mdio_wait_bsy(priv);

	return 0;
}

/* DM wrappers */
static int dm_enetc_mdio_read(struct udevice *dev, int addr, int devad, int reg)
{
	struct enetc_mdio_priv *priv = dev_get_priv(dev);

	return enetc_mdio_read_priv(priv, addr, devad, reg);
}

static int dm_enetc_mdio_write(struct udevice *dev, int addr, int devad,
			       int reg, u16 val)
{
	struct enetc_mdio_priv *priv = dev_get_priv(dev);

	return enetc_mdio_write_priv(priv, addr, devad, reg, val);
}

static const struct mdio_ops enetc_mdio_ops = {
	.read = dm_enetc_mdio_read,
	.write = dm_enetc_mdio_write,
};

static int enetc_mdio_bind(struct udevice *dev)
{
	char name[16];
	static int eth_num_devices;

	/*
	 * prefer using PCI function numbers to number interfaces, but these
	 * are only available if dts nodes are present.  For PCI they are
	 * optional, handle that case too.  Just in case some nodes are present
	 * and some are not, use different naming scheme - enetc-N based on
	 * PCI function # and enetc#N based on interface count
	 */
	if (ofnode_valid(dev_ofnode(dev)))
		sprintf(name, "emdio-%u", PCI_FUNC(pci_get_devfn(dev)));
	else
		sprintf(name, "emdio#%u", eth_num_devices++);
	device_set_name(dev, name);

	return 0;
}

static int enetc_mdio_probe(struct udevice *dev)
{
	struct enetc_mdio_priv *priv = dev_get_priv(dev);

	priv->regs_base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE, 0);
	if (!priv->regs_base) {
		enetc_dbg(dev, "failed to map BAR0\n");
		return -EINVAL;
	}

	priv->regs_base += ENETC_MDIO_BASE;

	dm_pci_clrset_config16(dev, PCI_COMMAND, 0, PCI_COMMAND_MEMORY);

	return 0;
}

U_BOOT_DRIVER(enetc_mdio) = {
	.name	= "enetc_mdio",
	.id	= UCLASS_MDIO,
	.bind	= enetc_mdio_bind,
	.probe	= enetc_mdio_probe,
	.ops	= &enetc_mdio_ops,
	.priv_auto	= sizeof(struct enetc_mdio_priv),
};

static struct pci_device_id enetc_mdio_ids[] = {
	{ PCI_DEVICE(PCI_VENDOR_ID_FREESCALE, PCI_DEVICE_ID_ENETC_MDIO) },
	{ }
};

U_BOOT_PCI_DEVICE(enetc_mdio, enetc_mdio_ids);
