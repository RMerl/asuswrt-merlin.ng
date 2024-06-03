// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#include <common.h>

#include <miiphy.h>
#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
#include <gdsys_fpga.h>
#else
#include <fdtdec.h>
#include <dm.h>
#include <regmap.h>
#endif

#include "ihs_mdio.h"

#ifndef CONFIG_GDSYS_LEGACY_DRIVERS
enum {
	REG_MDIO_CONTROL = 0x0,
	REG_MDIO_ADDR_DATA = 0x2,
	REG_MDIO_RX_DATA = 0x4,
};

static inline u16 read_reg(struct udevice *fpga, uint base, uint addr)
{
	struct regmap *map;
	u8 *ptr;

	regmap_init_mem(dev_ofnode(fpga), &map);
	ptr = regmap_get_range(map, 0);

	return in_le16((u16 *)(ptr + base + addr));
}

static inline void write_reg(struct udevice *fpga, uint base, uint addr,
			     u16 val)
{
	struct regmap *map;
	u8 *ptr;

	regmap_init_mem(dev_ofnode(fpga), &map);
	ptr = regmap_get_range(map, 0);

	out_le16((u16 *)(ptr + base + addr), val);
}
#endif

static inline u16 read_control(struct ihs_mdio_info *info)
{
	u16 val;
#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
	FPGA_GET_REG(info->fpga, mdio.control, &val);
#else
	val = read_reg(info->fpga, info->base, REG_MDIO_CONTROL);
#endif
	return val;
}

static inline void write_control(struct ihs_mdio_info *info, u16 val)
{
#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
	FPGA_SET_REG(info->fpga, mdio.control, val);
#else
	write_reg(info->fpga, info->base, REG_MDIO_CONTROL, val);
#endif
}

static inline void write_addr_data(struct ihs_mdio_info *info, u16 val)
{
#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
	FPGA_SET_REG(info->fpga, mdio.address_data, val);
#else
	write_reg(info->fpga, info->base, REG_MDIO_ADDR_DATA, val);
#endif
}

static inline u16 read_rx_data(struct ihs_mdio_info *info)
{
	u16 val;
#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
	FPGA_GET_REG(info->fpga, mdio.rx_data, &val);
#else
	val = read_reg(info->fpga, info->base, REG_MDIO_RX_DATA);
#endif
	return val;
}

static int ihs_mdio_idle(struct mii_dev *bus)
{
	struct ihs_mdio_info *info = bus->priv;
	u16 val;
	unsigned int ctr = 0;

	do {
		val = read_control(info);
		udelay(100);
		if (ctr++ > 10)
			return -1;
	} while (!(val & (1 << 12)));

	return 0;
}

static int ihs_mdio_reset(struct mii_dev *bus)
{
	ihs_mdio_idle(bus);

	return 0;
}

static int ihs_mdio_read(struct mii_dev *bus, int addr, int dev_addr,
			 int regnum)
{
	struct ihs_mdio_info *info = bus->priv;
	u16 val;

	ihs_mdio_idle(bus);

	write_control(info,
		      ((addr & 0x1f) << 5) | (regnum & 0x1f) | (2 << 10));

	/* wait for rx data available */
	udelay(100);

	val = read_rx_data(info);

	return val;
}

static int ihs_mdio_write(struct mii_dev *bus, int addr, int dev_addr,
			  int regnum, u16 value)
{
	struct ihs_mdio_info *info = bus->priv;

	ihs_mdio_idle(bus);

	write_addr_data(info, value);
	write_control(info, ((addr & 0x1f) << 5) | (regnum & 0x1f) | (1 << 10));

	return 0;
}

int ihs_mdio_init(struct ihs_mdio_info *info)
{
	struct mii_dev *bus = mdio_alloc();

	if (!bus) {
		printf("Failed to allocate FSL MDIO bus\n");
		return -1;
	}

	bus->read = ihs_mdio_read;
	bus->write = ihs_mdio_write;
	bus->reset = ihs_mdio_reset;
	strcpy(bus->name, info->name);

	bus->priv = info;

	return mdio_register(bus);
}
