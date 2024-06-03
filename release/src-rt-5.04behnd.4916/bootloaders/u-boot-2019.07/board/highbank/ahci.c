// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2012 Calxeda, Inc.
 */

#include <common.h>
#include <ahci.h>
#include <asm/io.h>

#define CPHY_MAP(dev, addr) ((((dev) & 0x1f) << 7) | (((addr) >> 9) & 0x7f))
#define CPHY_ADDR(base, dev, addr) ((base) | (((addr) & 0x1ff) << 2))
#define CPHY_BASE			0xfff58000
#define CPHY_WIDTH			0x1000
#define CPHY_DTE_XS			5
#define CPHY_MII			31
#define SERDES_CR_CTL			0x80a0
#define SERDES_CR_ADDR			0x80a1
#define SERDES_CR_DATA			0x80a2
#define CR_BUSY				0x0001
#define CR_START			0x0001
#define CR_WR_RDN			0x0002
#define CPHY_TX_INPUT_STS		0x2001
#define CPHY_RX_INPUT_STS		0x2002
#define CPHY_SATA_TX_OVERRIDE_BIT	0x8000
#define CPHY_SATA_RX_OVERRIDE_BIT	0x4000
#define CPHY_TX_INPUT_OVERRIDE		0x2004
#define CPHY_RX_INPUT_OVERRIDE		0x2005
#define SPHY_LANE			0x100
#define SPHY_HALF_RATE			0x0001
#define CPHY_SATA_DPLL_MODE		0x0700
#define CPHY_SATA_DPLL_SHIFT		8
#define CPHY_SATA_TX_ATTEN		0x1c00
#define CPHY_SATA_TX_ATTEN_SHIFT	10

#define HB_SREG_SATA_ATTEN		0xfff3cf24

#define SATA_PORT_BASE			0xffe08000
#define SATA_VERSIONR			0xf8
#define SATA_HB_VERSION			0x3332302a

static u32 __combo_phy_reg_read(u8 phy, u8 dev, u32 addr)
{
	u32 data;
	writel(CPHY_MAP(dev, addr), CPHY_BASE + 0x800 + CPHY_WIDTH * phy);
	data = readl(CPHY_ADDR(CPHY_BASE + CPHY_WIDTH * phy, dev, addr));
	return data;
}

static void __combo_phy_reg_write(u8 phy, u8 dev, u32 addr, u32 data)
{
	writel(CPHY_MAP(dev, addr), CPHY_BASE + 0x800 + CPHY_WIDTH * phy);
	writel(data, CPHY_ADDR(CPHY_BASE + CPHY_WIDTH * phy, dev, addr));
}

static u32 combo_phy_read(u8 phy, u32 addr)
{
	u8 dev = CPHY_DTE_XS;
	if (phy == 5)
		dev = CPHY_MII;
	while (__combo_phy_reg_read(phy, dev, SERDES_CR_CTL) & CR_BUSY)
		udelay(5);
	__combo_phy_reg_write(phy, dev, SERDES_CR_ADDR, addr);
	__combo_phy_reg_write(phy, dev, SERDES_CR_CTL, CR_START);
	while (__combo_phy_reg_read(phy, dev, SERDES_CR_CTL) & CR_BUSY)
		udelay(5);
	return __combo_phy_reg_read(phy, dev, SERDES_CR_DATA);
}

static void combo_phy_write(u8 phy, u32 addr, u32 data)
{
	u8 dev = CPHY_DTE_XS;
	if (phy == 5)
		dev = CPHY_MII;
	while (__combo_phy_reg_read(phy, dev, SERDES_CR_CTL) & CR_BUSY)
		udelay(5);
	__combo_phy_reg_write(phy, dev, SERDES_CR_ADDR, addr);
	__combo_phy_reg_write(phy, dev, SERDES_CR_DATA, data);
	__combo_phy_reg_write(phy, dev, SERDES_CR_CTL, CR_WR_RDN | CR_START);
}

static void cphy_spread_spectrum_override(u8 phy, u8 lane, u32 val)
{
	u32 tmp;
	tmp = combo_phy_read(phy, CPHY_RX_INPUT_STS + lane * SPHY_LANE);
	tmp &= ~CPHY_SATA_RX_OVERRIDE_BIT;
	combo_phy_write(phy, CPHY_RX_INPUT_OVERRIDE + lane * SPHY_LANE, tmp);

	tmp |= CPHY_SATA_RX_OVERRIDE_BIT;
	combo_phy_write(phy, CPHY_RX_INPUT_OVERRIDE + lane * SPHY_LANE, tmp);

	tmp &= ~CPHY_SATA_DPLL_MODE;
	tmp |= (val << CPHY_SATA_DPLL_SHIFT) & CPHY_SATA_DPLL_MODE;
	combo_phy_write(phy, CPHY_RX_INPUT_OVERRIDE + lane * SPHY_LANE, tmp);
}

static void cphy_tx_attenuation_override(u8 phy, u8 lane)
{
	u32 val;
	u32 tmp;
	u8  shift;

	shift = ((phy == 5) ? 4 : lane) * 4;

	val = (readl(HB_SREG_SATA_ATTEN) >> shift) & 0xf;

	if (val & 0x8)
		return;

	tmp = combo_phy_read(phy, CPHY_TX_INPUT_STS + lane * SPHY_LANE);
	tmp &= ~CPHY_SATA_TX_OVERRIDE_BIT;
	combo_phy_write(phy, CPHY_TX_INPUT_OVERRIDE + lane * SPHY_LANE, tmp);

	tmp |= CPHY_SATA_TX_OVERRIDE_BIT;
	combo_phy_write(phy, CPHY_TX_INPUT_OVERRIDE + lane * SPHY_LANE, tmp);

	tmp |= (val << CPHY_SATA_TX_ATTEN_SHIFT) & CPHY_SATA_TX_ATTEN;
	combo_phy_write(phy, CPHY_TX_INPUT_OVERRIDE + lane * SPHY_LANE, tmp);
}

static void cphy_disable_port_overrides(u8 port)
{
	u32 tmp;
	u8 lane = 0, phy = 0;

	if (port == 0)
		phy = 5;
	else if (port < 5)
		lane = port - 1;
	else
		return;
	tmp = combo_phy_read(phy, CPHY_RX_INPUT_STS + lane * SPHY_LANE);
	tmp &= ~CPHY_SATA_RX_OVERRIDE_BIT;
	combo_phy_write(phy, CPHY_RX_INPUT_OVERRIDE + lane * SPHY_LANE, tmp);

	tmp = combo_phy_read(phy, CPHY_TX_INPUT_OVERRIDE + lane * SPHY_LANE);
	tmp &= ~CPHY_SATA_TX_OVERRIDE_BIT;
	combo_phy_write(phy, CPHY_TX_INPUT_OVERRIDE + lane * SPHY_LANE, tmp);
}

void cphy_disable_overrides(void)
{
	int i;
	u32 port_map;

	port_map = readl(0xffe08000 + HOST_PORTS_IMPL);
	for (i = 0; i < 5; i++) {
		if (port_map & (1 << i))
			cphy_disable_port_overrides(i);
	}
}

static void cphy_override_lane(u8 port)
{
	u32 tmp, k = 0;
	u8 lane = 0, phy = 0;

	if (port == 0)
		phy = 5;
	else if (port < 5)
		lane = port - 1;
	else
		return;

	do {
		tmp = combo_phy_read(0, CPHY_RX_INPUT_STS +
					lane * SPHY_LANE);
	} while ((tmp & SPHY_HALF_RATE) && (k++ < 1000));
	cphy_spread_spectrum_override(phy, lane, 3);
	cphy_tx_attenuation_override(phy, lane);
}

#define WAIT_MS_LINKUP	4

int ahci_link_up(struct ahci_uc_priv *probe_ent, int port)
{
	u32 tmp;
	int j = 0;
	u8 *port_mmio = (u8 *)probe_ent->port[port].port_mmio;
	u32 is_highbank = readl(SATA_PORT_BASE + SATA_VERSIONR) ==
				SATA_HB_VERSION ? 1 : 0;

	/* Bring up SATA link.
	 * SATA link bringup time is usually less than 1 ms; only very
	 * rarely has it taken between 1-2 ms. Never seen it above 2 ms.
	 */
	while (j < WAIT_MS_LINKUP) {
		if (is_highbank && (j == 0)) {
			cphy_disable_port_overrides(port);
			writel(0x301, port_mmio + PORT_SCR_CTL);
			udelay(1000);
			writel(0x300, port_mmio + PORT_SCR_CTL);
			udelay(1000);
			cphy_override_lane(port);
		}

		tmp = readl(port_mmio + PORT_SCR_STAT);
		if ((tmp & 0xf) == 0x3)
			return 0;
		udelay(1000);
		j++;

		if ((j == WAIT_MS_LINKUP) && (tmp & 0xf))
			j = 0;	/* retry phy reset */
	}
	return 1;
}
