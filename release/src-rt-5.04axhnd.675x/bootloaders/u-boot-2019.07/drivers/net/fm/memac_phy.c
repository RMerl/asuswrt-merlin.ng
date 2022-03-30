// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 *	Andy Fleming <afleming@gmail.com>
 *	Roy Zang <tie-fei.zang@freescale.com>
 * Some part is taken from tsec.c
 */
#include <common.h>
#include <miiphy.h>
#include <phy.h>
#include <asm/io.h>
#include <fsl_memac.h>
#include <fm_eth.h>

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

/*
 * Write value to the PHY for this device to the register at regnum, waiting
 * until the write is done before it returns.  All PHY configuration has to be
 * done through the TSEC1 MIIM regs
 */
int memac_mdio_write(struct mii_dev *bus, int port_addr, int dev_addr,
			int regnum, u16 value)
{
	u32 mdio_ctl;
	struct memac_mdio_controller *regs = bus->priv;
	u32 c45 = 1; /* Default to 10G interface */

	if (dev_addr == MDIO_DEVAD_NONE) {
		c45 = 0; /* clause 22 */
		dev_addr = regnum & 0x1f;
		memac_clrbits_32(&regs->mdio_stat, MDIO_STAT_ENC);
	} else
		memac_setbits_32(&regs->mdio_stat, MDIO_STAT_ENC);

	/* Wait till the bus is free */
	while ((memac_in_32(&regs->mdio_stat)) & MDIO_STAT_BSY)
		;

	/* Set the port and dev addr */
	mdio_ctl = MDIO_CTL_PORT_ADDR(port_addr) | MDIO_CTL_DEV_ADDR(dev_addr);
	memac_out_32(&regs->mdio_ctl, mdio_ctl);

	/* Set the register address */
	if (c45)
		memac_out_32(&regs->mdio_addr, regnum & 0xffff);

	/* Wait till the bus is free */
	while ((memac_in_32(&regs->mdio_stat)) & MDIO_STAT_BSY)
		;

	/* Write the value to the register */
	memac_out_32(&regs->mdio_data, MDIO_DATA(value));

	/* Wait till the MDIO write is complete */
	while ((memac_in_32(&regs->mdio_data)) & MDIO_DATA_BSY)
		;

	return 0;
}

/*
 * Reads from register regnum in the PHY for device dev, returning the value.
 * Clears miimcom first.  All PHY configuration has to be done through the
 * TSEC1 MIIM regs
 */
int memac_mdio_read(struct mii_dev *bus, int port_addr, int dev_addr,
			int regnum)
{
	u32 mdio_ctl;
	struct memac_mdio_controller *regs = bus->priv;
	u32 c45 = 1;

	if (dev_addr == MDIO_DEVAD_NONE) {
		if (!strcmp(bus->name, DEFAULT_FM_TGEC_MDIO_NAME))
			return 0xffff;
		c45 = 0; /* clause 22 */
		dev_addr = regnum & 0x1f;
		memac_clrbits_32(&regs->mdio_stat, MDIO_STAT_ENC);
	} else
		memac_setbits_32(&regs->mdio_stat, MDIO_STAT_ENC);

	/* Wait till the bus is free */
	while ((memac_in_32(&regs->mdio_stat)) & MDIO_STAT_BSY)
		;

	/* Set the Port and Device Addrs */
	mdio_ctl = MDIO_CTL_PORT_ADDR(port_addr) | MDIO_CTL_DEV_ADDR(dev_addr);
	memac_out_32(&regs->mdio_ctl, mdio_ctl);

	/* Set the register address */
	if (c45)
		memac_out_32(&regs->mdio_addr, regnum & 0xffff);

	/* Wait till the bus is free */
	while ((memac_in_32(&regs->mdio_stat)) & MDIO_STAT_BSY)
		;

	/* Initiate the read */
	mdio_ctl |= MDIO_CTL_READ;
	memac_out_32(&regs->mdio_ctl, mdio_ctl);

	/* Wait till the MDIO write is complete */
	while ((memac_in_32(&regs->mdio_data)) & MDIO_DATA_BSY)
		;

	/* Return all Fs if nothing was there */
	if (memac_in_32(&regs->mdio_stat) & MDIO_STAT_RD_ER)
		return 0xffff;

	return memac_in_32(&regs->mdio_data) & 0xffff;
}

int memac_mdio_reset(struct mii_dev *bus)
{
	return 0;
}

int fm_memac_mdio_init(bd_t *bis, struct memac_mdio_info *info)
{
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate FM TGEC MDIO bus\n");
		return -1;
	}

	bus->read = memac_mdio_read;
	bus->write = memac_mdio_write;
	bus->reset = memac_mdio_reset;
	strcpy(bus->name, info->name);

	bus->priv = info->regs;

	/*
	 * On some platforms like B4860, default value of MDIO_CLK_DIV bits
	 * in mdio_stat(mdio_cfg) register generates MDIO clock too high
	 * (much higher than 2.5MHz), violating the IEEE specs.
	 * On other platforms like T1040, default value of MDIO_CLK_DIV bits
	 * is zero, so MDIO clock is disabled.
	 * So, for proper functioning of MDIO, MDIO_CLK_DIV bits needs to
	 * be properly initialized.
	 * NEG bit default should be '1' as per FMAN-v3 RM, but on platform
	 * like T2080QDS, this bit default is '0', which leads to MDIO failure
	 * on XAUI PHY, so set this bit definitely.
	 */
	memac_setbits_32(
		&((struct memac_mdio_controller *)info->regs)->mdio_stat,
		MDIO_STAT_CLKDIV(258) | MDIO_STAT_NEG);

	return mdio_register(bus);
}
