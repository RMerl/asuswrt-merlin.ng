// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <miiphy.h>
#include <asm/io.h>
#include <fsl_memac.h>

#ifdef CONFIG_SYS_MEMAC_LITTLE_ENDIAN
#define memac_out_32(a, v)	out_le32(a, v)
#define memac_clrbits_32(a, v)	clrbits_le32(a, v)
#define memac_setbits_32(a, v)	setbits_le32(a, v)
#else
#define memac_out_32(a, v)	out_be32(a, v)
#define memac_clrbits_32(a, v)	clrbits_be32(a, v)
#define memac_setbits_32(a, v)	setbits_be32(a, v)
#endif

static u32 memac_in_32(u32 *reg)
{
#ifdef CONFIG_SYS_MEMAC_LITTLE_ENDIAN
	return in_le32(reg);
#else
	return in_be32(reg);
#endif
}

struct fsl_ls_mdio_priv {
	void *regs_base;
};

static u32 fsl_ls_mdio_setup_operation(struct udevice *dev, int addr, int devad,
				       int reg)
{
	struct fsl_ls_mdio_priv *priv = dev_get_priv(dev);
	struct memac_mdio_controller *regs;
	u32 mdio_ctl;
	u32 c45 = 1;

	regs = (struct memac_mdio_controller *)(priv->regs_base);
	if (devad == MDIO_DEVAD_NONE) {
		c45 = 0; /* clause 22 */
		devad = reg & 0x1f;
		memac_clrbits_32(&regs->mdio_stat, MDIO_STAT_ENC);
	} else {
		memac_setbits_32(&regs->mdio_stat, MDIO_STAT_ENC);
	}

	/* Wait till the bus is free */
	while ((memac_in_32(&regs->mdio_stat)) & MDIO_STAT_BSY)
		;

	/* Set the Port and Device Addrs */
	mdio_ctl = MDIO_CTL_PORT_ADDR(addr) | MDIO_CTL_DEV_ADDR(devad);
	memac_out_32(&regs->mdio_ctl, mdio_ctl);

	/* Set the register address */
	if (c45)
		memac_out_32(&regs->mdio_addr, reg & 0xffff);

	/* Wait till the bus is free */
	while ((memac_in_32(&regs->mdio_stat)) & MDIO_STAT_BSY)
		;

	return mdio_ctl;
}

static int dm_fsl_ls_mdio_read(struct udevice *dev, int addr,
			       int devad, int reg)
{
	struct fsl_ls_mdio_priv *priv = dev_get_priv(dev);
	struct memac_mdio_controller *regs;
	u32 mdio_ctl;

	regs = (struct memac_mdio_controller *)(priv->regs_base);
	mdio_ctl = fsl_ls_mdio_setup_operation(dev, addr, devad, reg);

	/* Initiate the read */
	mdio_ctl |= MDIO_CTL_READ;
	memac_out_32(&regs->mdio_ctl, mdio_ctl);

	/* Wait till the MDIO write is complete */
	while ((memac_in_32(&regs->mdio_stat)) & MDIO_STAT_BSY)
		;

	/* Return all Fs if nothing was there */
	if (memac_in_32(&regs->mdio_stat) & MDIO_STAT_RD_ER)
		return 0xffff;

	return memac_in_32(&regs->mdio_data) & 0xffff;
}

static int dm_fsl_ls_mdio_write(struct udevice *dev, int addr, int devad,
				int reg, u16 val)
{
	struct fsl_ls_mdio_priv *priv = dev_get_priv(dev);
	struct memac_mdio_controller *regs;

	regs = (struct memac_mdio_controller *)(priv->regs_base);
	fsl_ls_mdio_setup_operation(dev, addr, devad, reg);

	/* Write the value to the register */
	memac_out_32(&regs->mdio_data, MDIO_DATA(val));

	/* Wait till the MDIO write is complete */
	while ((memac_in_32(&regs->mdio_stat)) & MDIO_STAT_BSY)
		;

	return 0;
}

static const struct mdio_ops fsl_ls_mdio_ops = {
	.read = dm_fsl_ls_mdio_read,
	.write = dm_fsl_ls_mdio_write,
};

static int fsl_ls_mdio_probe(struct udevice *dev)
{
	struct fsl_ls_mdio_priv *priv = dev_get_priv(dev);
	struct memac_mdio_controller *regs;

	priv->regs_base = dev_read_addr_ptr(dev);
	if (!priv->regs_base)
		return -ENODEV;

	regs = (struct memac_mdio_controller *)(priv->regs_base);

	memac_setbits_32(&regs->mdio_stat,
			 MDIO_STAT_CLKDIV(258) | MDIO_STAT_NEG);

	return 0;
}

static const struct udevice_id fsl_ls_mdio_of_ids[] = {
	{ .compatible = "fsl,ls-mdio" },
};

U_BOOT_DRIVER(fsl_ls_mdio) = {
	.name = "fsl_ls_mdio",
	.id = UCLASS_MDIO,
	.of_match = fsl_ls_mdio_of_ids,
	.probe = fsl_ls_mdio_probe,
	.ops = &fsl_ls_mdio_ops,
	.priv_auto	= sizeof(struct fsl_ls_mdio_priv),
};
