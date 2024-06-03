// SPDX-License-Identifier: GPL-2.0
/*
 * Renesas RCar Gen3 PCIEC driver
 *
 * Copyright (C) 2018-2019 Marek Vasut <marek.vasut@gmail.com>
 *
 * Based on Linux PCIe driver for Renesas R-Car SoCs
 *  Copyright (C) 2014 Renesas Electronics Europe Ltd
 *
 * Based on:
 *  arch/sh/drivers/pci/pcie-sh7786.c
 *  arch/sh/drivers/pci/ops-sh7786.c
 *  Copyright (C) 2009 - 2011  Paul Mundt
 *
 * Author: Phil Edworthy <phil.edworthy@renesas.com>
 */

#include <common.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <pci.h>
#include <wait_bit.h>

#define PCIECAR			0x000010
#define PCIECCTLR		0x000018
#define  CONFIG_SEND_ENABLE	BIT(31)
#define  TYPE0			(0 << 8)
#define  TYPE1			BIT(8)
#define PCIECDR			0x000020
#define PCIEMSR			0x000028
#define PCIEINTXR		0x000400
#define PCIEPHYSR		0x0007f0
#define  PHYRDY			BIT(0)
#define PCIEMSITXR		0x000840

/* Transfer control */
#define PCIETCTLR		0x02000
#define  CFINIT			1
#define PCIETSTR		0x02004
#define  DATA_LINK_ACTIVE	1
#define PCIEERRFR		0x02020
#define  UNSUPPORTED_REQUEST	BIT(4)
#define PCIEMSIFR		0x02044
#define PCIEMSIALR		0x02048
#define  MSIFE			1
#define PCIEMSIAUR		0x0204c
#define PCIEMSIIER		0x02050

/* root port address */
#define PCIEPRAR(x)		(0x02080 + ((x) * 0x4))

/* local address reg & mask */
#define PCIELAR(x)		(0x02200 + ((x) * 0x20))
#define PCIELAMR(x)		(0x02208 + ((x) * 0x20))
#define  LAM_PREFETCH		BIT(3)
#define  LAM_64BIT		BIT(2)
#define  LAR_ENABLE		BIT(1)

/* PCIe address reg & mask */
#define PCIEPALR(x)		(0x03400 + ((x) * 0x20))
#define PCIEPAUR(x)		(0x03404 + ((x) * 0x20))
#define PCIEPAMR(x)		(0x03408 + ((x) * 0x20))
#define PCIEPTCTLR(x)		(0x0340c + ((x) * 0x20))
#define  PAR_ENABLE		BIT(31)
#define  IO_SPACE		BIT(8)

/* Configuration */
#define PCICONF(x)		(0x010000 + ((x) * 0x4))
#define PMCAP(x)		(0x010040 + ((x) * 0x4))
#define EXPCAP(x)		(0x010070 + ((x) * 0x4))
#define VCCAP(x)		(0x010100 + ((x) * 0x4))

/* link layer */
#define IDSETR1			0x011004
#define TLCTLR			0x011048
#define MACSR			0x011054
#define  SPCHGFIN		BIT(4)
#define  SPCHGFAIL		BIT(6)
#define  SPCHGSUC		BIT(7)
#define  LINK_SPEED		(0xf << 16)
#define  LINK_SPEED_2_5GTS	(1 << 16)
#define  LINK_SPEED_5_0GTS	(2 << 16)
#define MACCTLR			0x011058
#define  SPEED_CHANGE		BIT(24)
#define  SCRAMBLE_DISABLE	BIT(27)
#define MACS2R			0x011078
#define MACCGSPSETR		0x011084
#define  SPCNGRSN		BIT(31)

/* R-Car H1 PHY */
#define H1_PCIEPHYADRR		0x04000c
#define  WRITE_CMD		BIT(16)
#define  PHY_ACK		BIT(24)
#define  RATE_POS		12
#define  LANE_POS		8
#define  ADR_POS		0
#define H1_PCIEPHYDOUTR		0x040014

/* R-Car Gen2 PHY */
#define GEN2_PCIEPHYADDR	0x780
#define GEN2_PCIEPHYDATA	0x784
#define GEN2_PCIEPHYCTRL	0x78c

#define INT_PCI_MSI_NR		32

#define RCONF(x)		(PCICONF(0) + (x))
#define RPMCAP(x)		(PMCAP(0) + (x))
#define REXPCAP(x)		(EXPCAP(0) + (x))
#define RVCCAP(x)		(VCCAP(0) + (x))

#define PCIE_CONF_BUS(b)	(((b) & 0xff) << 24)
#define PCIE_CONF_DEV(d)	(((d) & 0x1f) << 19)
#define PCIE_CONF_FUNC(f)	(((f) & 0x7) << 16)

#define RCAR_PCI_MAX_RESOURCES	4
#define MAX_NR_INBOUND_MAPS	6

#define PCI_EXP_FLAGS		2		/* Capabilities register */
#define PCI_EXP_FLAGS_TYPE	0x00f0		/* Device/Port type */
#define PCI_EXP_TYPE_ROOT_PORT	0x4		/* Root Port */
#define PCI_EXP_LNKCAP		12		/* Link Capabilities */
#define PCI_EXP_LNKCAP_DLLLARC	0x00100000	/* Data Link Layer Link Active Reporting Capable */
#define PCI_EXP_SLTCAP		20		/* Slot Capabilities */
#define PCI_EXP_SLTCAP_PSN	0xfff80000	/* Physical Slot Number */

enum {
	RCAR_PCI_ACCESS_READ,
	RCAR_PCI_ACCESS_WRITE,
};

struct rcar_gen3_pcie_priv {
	fdt_addr_t		regs;
};

static void rcar_rmw32(struct udevice *dev, int where, u32 mask, u32 data)
{
	struct rcar_gen3_pcie_priv *priv = dev_get_platdata(dev);
	int shift = 8 * (where & 3);

	clrsetbits_le32(priv->regs + (where & ~3),
			mask << shift, data << shift);
}

static u32 rcar_read_conf(struct udevice *dev, int where)
{
	struct rcar_gen3_pcie_priv *priv = dev_get_platdata(dev);
	int shift = 8 * (where & 3);

	return readl(priv->regs + (where & ~3)) >> shift;
}

static int rcar_pcie_config_access(struct udevice *udev,
				   unsigned char access_type,
				   pci_dev_t bdf, int where, ulong *data)
{
	struct rcar_gen3_pcie_priv *priv = dev_get_platdata(udev);
	u32 reg = where & ~3;

	/* Clear errors */
	clrbits_le32(priv->regs + PCIEERRFR, 0);

	/* Set the PIO address */
	writel((bdf << 8) | reg, priv->regs + PCIECAR);

	/* Enable the configuration access */
	if (!PCI_BUS(bdf))
		writel(CONFIG_SEND_ENABLE | TYPE0, priv->regs + PCIECCTLR);
	else
		writel(CONFIG_SEND_ENABLE | TYPE1, priv->regs + PCIECCTLR);

	/* Check for errors */
	if (readl(priv->regs + PCIEERRFR) & UNSUPPORTED_REQUEST)
		return -ENODEV;

	/* Check for master and target aborts */
	if (rcar_read_conf(udev, RCONF(PCI_STATUS)) &
		(PCI_STATUS_REC_MASTER_ABORT | PCI_STATUS_REC_TARGET_ABORT))
		return -ENODEV;

	if (access_type == RCAR_PCI_ACCESS_READ)
		*data = readl(priv->regs + PCIECDR);
	else
		writel(*data, priv->regs + PCIECDR);

	/* Disable the configuration access */
	writel(0, priv->regs + PCIECCTLR);

	return 0;
}

static int rcar_gen3_pcie_addr_valid(pci_dev_t d, uint where)
{
	u32 slot;

	if (PCI_FUNC(d))
		return -EINVAL;

	slot = PCI_DEV(d);
	if (slot != 1)
		return -EINVAL;

	return 0;
}

static int rcar_gen3_pcie_read_config(struct udevice *dev, pci_dev_t bdf,
				      uint where, ulong *val,
				      enum pci_size_t size)
{
	ulong reg;
	int ret;

	ret = rcar_gen3_pcie_addr_valid(bdf, where);
	if (ret) {
		*val = pci_get_ff(size);
		return 0;
	}

	ret = rcar_pcie_config_access(dev, RCAR_PCI_ACCESS_READ,
				      bdf, where, &reg);
	if (ret != 0)
		reg = 0xffffffffUL;

	*val = pci_conv_32_to_size(reg, where, size);

	return ret;
}

static int rcar_gen3_pcie_write_config(struct udevice *dev, pci_dev_t bdf,
				       uint where, ulong val,
				       enum pci_size_t size)
{
	ulong data;
	int ret;

	ret = rcar_gen3_pcie_addr_valid(bdf, where);
	if (ret)
		return ret;

	data = pci_conv_32_to_size(val, where, size);

	ret = rcar_pcie_config_access(dev, RCAR_PCI_ACCESS_WRITE,
				      bdf, where, &data);

	return ret;
}

static int rcar_gen3_pcie_wait_for_phyrdy(struct udevice *dev)
{
	struct rcar_gen3_pcie_priv *priv = dev_get_platdata(dev);

	return wait_for_bit_le32((void *)priv->regs + PCIEPHYSR, PHYRDY,
				 true, 50, false);
}

static int rcar_gen3_pcie_wait_for_dl(struct udevice *dev)
{
	struct rcar_gen3_pcie_priv *priv = dev_get_platdata(dev);

	return wait_for_bit_le32((void *)priv->regs + PCIETSTR,
				 DATA_LINK_ACTIVE, true, 50, false);
}

static int rcar_gen3_pcie_hw_init(struct udevice *dev)
{
	struct rcar_gen3_pcie_priv *priv = dev_get_platdata(dev);
	int ret;

	/* Begin initialization */
	writel(0, priv->regs + PCIETCTLR);

	/* Set mode */
	writel(1, priv->regs + PCIEMSR);

	ret = rcar_gen3_pcie_wait_for_phyrdy(dev);
	if (ret)
		return ret;

	/*
	 * Initial header for port config space is type 1, set the device
	 * class to match. Hardware takes care of propagating the IDSETR
	 * settings, so there is no need to bother with a quirk.
	 */
	writel(PCI_CLASS_BRIDGE_PCI << 16, priv->regs + IDSETR1);

	/*
	 * Setup Secondary Bus Number & Subordinate Bus Number, even though
	 * they aren't used, to avoid bridge being detected as broken.
	 */
	rcar_rmw32(dev, RCONF(PCI_SECONDARY_BUS), 0xff, 1);
	rcar_rmw32(dev, RCONF(PCI_SUBORDINATE_BUS), 0xff, 1);

	/* Initialize default capabilities. */
	rcar_rmw32(dev, REXPCAP(0), 0xff, PCI_CAP_ID_EXP);
	rcar_rmw32(dev, REXPCAP(PCI_EXP_FLAGS),
		   PCI_EXP_FLAGS_TYPE, PCI_EXP_TYPE_ROOT_PORT << 4);
	rcar_rmw32(dev, RCONF(PCI_HEADER_TYPE), 0x7f,
		   PCI_HEADER_TYPE_BRIDGE);

	/* Enable data link layer active state reporting */
	rcar_rmw32(dev, REXPCAP(PCI_EXP_LNKCAP),
		   PCI_EXP_LNKCAP_DLLLARC, PCI_EXP_LNKCAP_DLLLARC);

	/* Write out the physical slot number = 0 */
	rcar_rmw32(dev, REXPCAP(PCI_EXP_SLTCAP),
		   PCI_EXP_SLTCAP_PSN, 0);

	/* Set the completion timer timeout to the maximum 50ms. */
	rcar_rmw32(dev, TLCTLR + 1, 0x3f, 50);

	/* Terminate list of capabilities (Next Capability Offset=0) */
	rcar_rmw32(dev, RVCCAP(0), 0xfff00000, 0);

	/* Finish initialization - establish a PCI Express link */
	writel(CFINIT, priv->regs + PCIETCTLR);

	return rcar_gen3_pcie_wait_for_dl(dev);
}

static int rcar_gen3_pcie_probe(struct udevice *dev)
{
	struct rcar_gen3_pcie_priv *priv = dev_get_platdata(dev);
	struct pci_controller *hose = dev_get_uclass_priv(dev);
	struct clk pci_clk;
	u32 mask;
	int i, cnt, ret;

	ret = clk_get_by_index(dev, 0, &pci_clk);
	if (ret)
		return ret;

	ret = clk_enable(&pci_clk);
	if (ret)
		return ret;

	for (i = 0; i < hose->region_count; i++) {
		if (hose->regions[i].flags != PCI_REGION_SYS_MEMORY)
			continue;

		if (hose->regions[i].phys_start == 0)
			continue;

		mask = (hose->regions[i].size - 1) & ~0xf;
		mask |= LAR_ENABLE;
		writel(hose->regions[i].phys_start, priv->regs + PCIEPRAR(0));
		writel(hose->regions[i].phys_start, priv->regs + PCIELAR(0));
		writel(mask, priv->regs + PCIELAMR(0));
		break;
	}

	writel(0, priv->regs + PCIEPRAR(4));
	writel(0, priv->regs + PCIELAR(4));
	writel(0, priv->regs + PCIELAMR(4));

	ret = rcar_gen3_pcie_hw_init(dev);
	if (ret)
		return ret;

	for (i = 0, cnt = 0; i < hose->region_count; i++) {
		if (hose->regions[i].flags == PCI_REGION_SYS_MEMORY)
			continue;

		writel(0, priv->regs + PCIEPTCTLR(cnt));
		writel((hose->regions[i].size - 1) & ~0x7f,
		       priv->regs + PCIEPAMR(cnt));
		writel(upper_32_bits(hose->regions[i].phys_start),
		       priv->regs + PCIEPAUR(cnt));
		writel(lower_32_bits(hose->regions[i].phys_start),
		       priv->regs + PCIEPALR(cnt));
		mask = PAR_ENABLE;
		if (hose->regions[i].flags == PCI_REGION_IO)
			mask |= IO_SPACE;
		writel(mask, priv->regs + PCIEPTCTLR(cnt));

		cnt++;
	}

	return 0;
}

static int rcar_gen3_pcie_ofdata_to_platdata(struct udevice *dev)
{
	struct rcar_gen3_pcie_priv *priv = dev_get_platdata(dev);

	priv->regs = devfdt_get_addr_index(dev, 0);
	if (!priv->regs)
		return -EINVAL;

	return 0;
}

static const struct dm_pci_ops rcar_gen3_pcie_ops = {
	.read_config	= rcar_gen3_pcie_read_config,
	.write_config	= rcar_gen3_pcie_write_config,
};

static const struct udevice_id rcar_gen3_pcie_ids[] = {
	{ .compatible = "renesas,pcie-rcar-gen3" },
	{ }
};

U_BOOT_DRIVER(rcar_gen3_pcie) = {
	.name			= "rcar_gen3_pcie",
	.id			= UCLASS_PCI,
	.of_match		= rcar_gen3_pcie_ids,
	.ops			= &rcar_gen3_pcie_ops,
	.probe			= rcar_gen3_pcie_probe,
	.ofdata_to_platdata	= rcar_gen3_pcie_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct rcar_gen3_pcie_priv),
};
