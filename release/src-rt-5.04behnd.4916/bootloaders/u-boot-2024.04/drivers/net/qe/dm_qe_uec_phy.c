// SPDX-License-Identifier: GPL-2.0+
/*
 * QE UEC ethernet phy controller driver
 *
 * based on phy parts of drivers/qe/uec.c and drivers/qe/uec_phy.c
 * from NXP
 *
 * Copyright (C) 2020 Heiko Schocher <hs@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <miiphy.h>
#include <phy.h>
#include <asm/io.h>
#include <linux/ioport.h>

#include "dm_qe_uec.h"

struct qe_uec_mdio_priv {
	struct ucc_mii_mng *base;
};

static int
qe_uec_mdio_read(struct udevice *dev, int addr, int devad, int reg)
{
	struct qe_uec_mdio_priv *priv = dev_get_priv(dev);
	struct ucc_mii_mng *regs = priv->base;
	u32 tmp_reg;
	u16 value;

	debug("%s: regs: %p addr: %x devad: %x reg: %x\n", __func__, regs,
	      addr, devad, reg);
	/* Setting up the MII management Address Register */
	tmp_reg = ((u32)addr << MIIMADD_PHY_ADDRESS_SHIFT) | reg;
	out_be32(&regs->miimadd, tmp_reg);

	/* clear MII management command cycle */
	out_be32(&regs->miimcom, 0);
	sync();

	/* Perform an MII management read cycle */
	out_be32(&regs->miimcom, MIIMCOM_READ_CYCLE);

	/* Wait till MII management write is complete */
	while ((in_be32(&regs->miimind)) &
	       (MIIMIND_NOT_VALID | MIIMIND_BUSY))
		;

	/* Read MII management status  */
	value = (u16)in_be32(&regs->miimstat);
	if (value == 0xffff)
		return -EINVAL;

	return value;
};

static int
qe_uec_mdio_write(struct udevice *dev, int addr, int devad, int reg,
		  u16 value)
{
	struct qe_uec_mdio_priv *priv = dev_get_priv(dev);
	struct ucc_mii_mng *regs = priv->base;
	u32 tmp_reg;

	debug("%s: regs: %p addr: %x devad: %x reg: %x val: %x\n", __func__,
	      regs, addr, devad, reg, value);

	/* Stop the MII management read cycle */
	out_be32(&regs->miimcom, 0);
	/* Setting up the MII management Address Register */
	tmp_reg = ((u32)addr << MIIMADD_PHY_ADDRESS_SHIFT) | reg;
	out_be32(&regs->miimadd, tmp_reg);

	/* Setting up the MII management Control Register with the value */
	out_be32(&regs->miimcon, (u32)value);
	sync();

	/* Wait till MII management write is complete */
	while ((in_be32(&regs->miimind)) & MIIMIND_BUSY)
		;

	return 0;
};

static const struct mdio_ops qe_uec_mdio_ops = {
	.read = qe_uec_mdio_read,
	.write = qe_uec_mdio_write,
};

static int qe_uec_mdio_probe(struct udevice *dev)
{
	struct qe_uec_mdio_priv *priv = dev_get_priv(dev);
	fdt_size_t base;
	ofnode node;
	u32 num = 0;
	int ret = -ENODEV;

	priv->base = dev_read_addr_ptr(dev);
	base = (fdt_size_t)priv->base;

	/*
	 * idea from linux:
	 * drivers/net/ethernet/freescale/fsl_pq_mdio.c
	 *
	 * Find the UCC node that controls the given MDIO node
	 *
	 * For some reason, the QE MDIO nodes are not children of the UCC
	 * devices that control them.  Therefore, we need to scan all UCC
	 * nodes looking for the one that encompases the given MDIO node.
	 * We do this by comparing physical addresses.  The 'start' and
	 * 'end' addresses of the MDIO node are passed, and the correct
	 * UCC node will cover the entire address range.
	 */
	node = ofnode_by_compatible(ofnode_null(), "ucc_geth");
	while (ofnode_valid(node)) {
		fdt_size_t size;
		fdt_addr_t addr;

		addr = ofnode_get_addr_index(node, 0);
		ret = ofnode_get_addr_size_index(node, 0, &size);

		if (addr == FDT_ADDR_T_NONE) {
			node = ofnode_by_compatible(node, "ucc_geth");
			continue;
		}

		/* check if priv->base in start end */
		if (base > addr && base < (addr + size)) {
			ret = ofnode_read_u32(node, "cell-index", &num);
			if (ret)
				ret = ofnode_read_u32(node, "device-id",
						      &num);
			break;
		}
		node = ofnode_by_compatible(node, "ucc_geth");
	}

	if (ret) {
		printf("%s: no cell-index nor device-id found!", __func__);
		return ret;
	}

	/* Setup MII master clock source */
	qe_set_mii_clk_src(num - 1);

	return 0;
}

static const struct udevice_id qe_uec_mdio_ids[] = {
	{ .compatible = "fsl,ucc-mdio" },
	{ }
};

U_BOOT_DRIVER(mvmdio) = {
	.name			= "qe_uec_mdio",
	.id			= UCLASS_MDIO,
	.of_match		= qe_uec_mdio_ids,
	.probe			= qe_uec_mdio_probe,
	.ops			= &qe_uec_mdio_ops,
	.priv_auto	= sizeof(struct qe_uec_mdio_priv),
};
