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

static int fsl_pq_mdio_reset(struct mii_dev *bus)
{
	struct tsec_mii_mng __iomem *regs =
		(struct tsec_mii_mng __iomem *)bus->priv;

	/* Reset MII (due to new addresses) */
	out_be32(&regs->miimcfg, MIIMCFG_RESET_MGMT);

	out_be32(&regs->miimcfg, MIIMCFG_INIT_VALUE);

	while (in_be32(&regs->miimind) & MIIMIND_BUSY)
		;

	return 0;
}

int tsec_phy_read(struct mii_dev *bus, int addr, int dev_addr, int regnum)
{
	struct tsec_mii_mng __iomem *phyregs =
		(struct tsec_mii_mng __iomem *)bus->priv;

	return tsec_local_mdio_read(phyregs, addr, dev_addr, regnum);
}

int tsec_phy_write(struct mii_dev *bus, int addr, int dev_addr, int regnum,
			u16 value)
{
	struct tsec_mii_mng __iomem *phyregs =
		(struct tsec_mii_mng __iomem *)bus->priv;

	tsec_local_mdio_write(phyregs, addr, dev_addr, regnum, value);

	return 0;
}

int fsl_pq_mdio_init(bd_t *bis, struct fsl_pq_mdio_info *info)
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
