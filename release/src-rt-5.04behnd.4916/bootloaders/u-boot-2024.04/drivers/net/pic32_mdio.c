// SPDX-License-Identifier: GPL-2.0+
/*
 * pic32_mdio.c: PIC32 MDIO/MII driver, part of pic32_eth.c.
 *
 * Copyright 2015 Microchip Inc.
 *	Purna Chandra Mandal <purna.mandal@microchip.com>
 */
#include <common.h>
#include <phy.h>
#include <miiphy.h>
#include <errno.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <linux/delay.h>
#include "pic32_eth.h"

static int pic32_mdio_write(struct mii_dev *bus,
			    int addr, int dev_addr,
			    int reg, u16 value)
{
	u32 v;
	struct pic32_mii_regs *mii_regs = bus->priv;

	/* Wait for the previous operation to finish */
	wait_for_bit_le32(&mii_regs->mind.raw, MIIMIND_BUSY,
			  false, CONFIG_SYS_HZ, true);

	/* Put phyaddr and regaddr into MIIMADD */
	v = (addr << MIIMADD_PHYADDR_SHIFT) | (reg & MIIMADD_REGADDR);
	writel(v, &mii_regs->madr.raw);

	/* Initiate a write command */
	writel(value, &mii_regs->mwtd.raw);

	/* Wait 30 clock cycles for busy flag to be set */
	udelay(12);

	/* Wait for write to complete */
	wait_for_bit_le32(&mii_regs->mind.raw, MIIMIND_BUSY,
			  false, CONFIG_SYS_HZ, true);

	return 0;
}

static int pic32_mdio_read(struct mii_dev *bus, int addr, int devaddr, int reg)
{
	u32 v;
	struct pic32_mii_regs *mii_regs = bus->priv;

	/* Wait for the previous operation to finish */
	wait_for_bit_le32(&mii_regs->mind.raw, MIIMIND_BUSY,
			  false, CONFIG_SYS_HZ, true);

	/* Put phyaddr and regaddr into MIIMADD */
	v = (addr << MIIMADD_PHYADDR_SHIFT) | (reg & MIIMADD_REGADDR);
	writel(v, &mii_regs->madr.raw);

	/* Initiate a read command */
	writel(MIIMCMD_READ, &mii_regs->mcmd.raw);

	/* Wait 30 clock cycles for busy flag to be set */
	udelay(12);

	/* Wait for read to complete */
	wait_for_bit_le32(&mii_regs->mind.raw,
			  MIIMIND_NOTVALID | MIIMIND_BUSY,
			  false, CONFIG_SYS_HZ, false);

	/* Clear the command register */
	writel(0, &mii_regs->mcmd.raw);

	/* Grab the value read from the PHY */
	v = readl(&mii_regs->mrdd.raw);
	return v;
}

static int pic32_mdio_reset(struct mii_dev *bus)
{
	struct pic32_mii_regs *mii_regs = bus->priv;

	/* Reset MII (due to new addresses) */
	writel(MIIMCFG_RSTMGMT, &mii_regs->mcfg.raw);

	/* Wait for the operation to finish */
	wait_for_bit_le32(&mii_regs->mind.raw, MIIMIND_BUSY,
		     false, CONFIG_SYS_HZ, true);

	/* Clear reset bit */
	writel(0, &mii_regs->mcfg);

	/* Wait for the operation to finish */
	wait_for_bit_le32(&mii_regs->mind.raw, MIIMIND_BUSY,
			  false, CONFIG_SYS_HZ, true);

	/* Set the MII Management Clock (MDC) - no faster than 2.5 MHz */
	writel(MIIMCFG_CLKSEL_DIV40, &mii_regs->mcfg.raw);

	/* Wait for the operation to finish */
	wait_for_bit_le32(&mii_regs->mind.raw, MIIMIND_BUSY,
			  false, CONFIG_SYS_HZ, true);
	return 0;
}

int pic32_mdio_init(const char *name, ulong ioaddr)
{
	struct mii_dev *bus;

	bus = mdio_alloc();
	if (!bus) {
		printf("Failed to allocate PIC32-MDIO bus\n");
		return -ENOMEM;
	}

	bus->read = pic32_mdio_read;
	bus->write = pic32_mdio_write;
	bus->reset = pic32_mdio_reset;
	strncpy(bus->name, name, sizeof(bus->name));
	bus->priv = (void *)ioaddr;

	return mdio_register(bus);
}
