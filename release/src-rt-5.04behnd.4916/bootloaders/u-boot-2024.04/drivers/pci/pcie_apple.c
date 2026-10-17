// SPDX-License-Identifier: GPL-2.0
/*
 * PCIe host bridge driver for Apple system-on-chips.
 *
 * The HW is ECAM compliant.
 *
 * Initialization requires enabling power and clocks, along with a
 * number of register pokes.
 *
 * Copyright (C) 2021 Alyssa Rosenzweig <alyssa@rosenzweig.io>
 * Copyright (C) 2021 Google LLC
 * Copyright (C) 2021 Corellium LLC
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 *
 * Author: Alyssa Rosenzweig <alyssa@rosenzweig.io>
 * Author: Marc Zyngier <maz@kernel.org>
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <mapmem.h>
#include <pci.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#define CORE_RC_PHYIF_CTL		0x00024
#define   CORE_RC_PHYIF_CTL_RUN		BIT(0)
#define CORE_RC_PHYIF_STAT		0x00028
#define   CORE_RC_PHYIF_STAT_REFCLK	BIT(4)
#define CORE_RC_CTL			0x00050
#define   CORE_RC_CTL_RUN		BIT(0)
#define CORE_RC_STAT			0x00058
#define   CORE_RC_STAT_READY		BIT(0)
#define CORE_FABRIC_STAT		0x04000
#define   CORE_FABRIC_STAT_MASK		0x001F001F

#define CORE_PHY_DEFAULT_BASE(port)	(0x84000 + 0x4000 * (port))

#define PHY_LANE_CFG			0x00000
#define   PHY_LANE_CFG_REFCLK0REQ	BIT(0)
#define   PHY_LANE_CFG_REFCLK1REQ	BIT(1)
#define   PHY_LANE_CFG_REFCLK0ACK	BIT(2)
#define   PHY_LANE_CFG_REFCLK1ACK	BIT(3)
#define   PHY_LANE_CFG_REFCLKEN		(BIT(9) | BIT(10))
#define   PHY_LANE_CFG_REFCLKCGEN	(BIT(30) | BIT(31))
#define PHY_LANE_CTL			0x00004
#define   PHY_LANE_CTL_CFGACC		BIT(15)

#define PORT_LTSSMCTL			0x00080
#define   PORT_LTSSMCTL_START		BIT(0)
#define PORT_INTSTAT			0x00100
#define   PORT_INT_TUNNEL_ERR		31
#define   PORT_INT_CPL_TIMEOUT		23
#define   PORT_INT_RID2SID_MAPERR	22
#define   PORT_INT_CPL_ABORT		21
#define   PORT_INT_MSI_BAD_DATA		19
#define   PORT_INT_MSI_ERR		18
#define   PORT_INT_REQADDR_GT32		17
#define   PORT_INT_AF_TIMEOUT		15
#define   PORT_INT_LINK_DOWN		14
#define   PORT_INT_LINK_UP		12
#define   PORT_INT_LINK_BWMGMT		11
#define   PORT_INT_AER_MASK		(15 << 4)
#define   PORT_INT_PORT_ERR		4
#define   PORT_INT_INTx(i)		i
#define   PORT_INT_INTx_MASK		15
#define PORT_INTMSK			0x00104
#define PORT_INTMSKSET			0x00108
#define PORT_INTMSKCLR			0x0010c
#define PORT_MSICFG			0x00124
#define   PORT_MSICFG_EN		BIT(0)
#define   PORT_MSICFG_L2MSINUM_SHIFT	4
#define PORT_MSIBASE			0x00128
#define   PORT_MSIBASE_1_SHIFT		16
#define PORT_MSIADDR			0x00168
#define PORT_LINKSTS			0x00208
#define   PORT_LINKSTS_UP		BIT(0)
#define   PORT_LINKSTS_BUSY		BIT(2)
#define PORT_LINKCMDSTS			0x00210
#define PORT_OUTS_NPREQS		0x00284
#define   PORT_OUTS_NPREQS_REQ		BIT(24)
#define   PORT_OUTS_NPREQS_CPL		BIT(16)
#define PORT_RXWR_FIFO			0x00288
#define   PORT_RXWR_FIFO_HDR		GENMASK(15, 10)
#define   PORT_RXWR_FIFO_DATA		GENMASK(9, 0)
#define PORT_RXRD_FIFO			0x0028C
#define   PORT_RXRD_FIFO_REQ		GENMASK(6, 0)
#define PORT_OUTS_CPLS			0x00290
#define   PORT_OUTS_CPLS_SHRD		GENMASK(14, 8)
#define   PORT_OUTS_CPLS_WAIT		GENMASK(6, 0)
#define PORT_APPCLK			0x00800
#define   PORT_APPCLK_EN		BIT(0)
#define   PORT_APPCLK_CGDIS		BIT(8)
#define PORT_STATUS			0x00804
#define   PORT_STATUS_READY		BIT(0)
#define PORT_REFCLK			0x00810
#define   PORT_REFCLK_EN		BIT(0)
#define   PORT_REFCLK_CGDIS		BIT(8)
#define PORT_PERST			0x00814
#define   PORT_PERST_OFF		BIT(0)
#define PORT_RID2SID(i16)		(0x00828 + 4 * (i16))
#define   PORT_RID2SID_VALID		BIT(31)
#define   PORT_RID2SID_SID_SHIFT	16
#define   PORT_RID2SID_BUS_SHIFT	8
#define   PORT_RID2SID_DEV_SHIFT	3
#define   PORT_RID2SID_FUNC_SHIFT	0
#define PORT_OUTS_PREQS_HDR		0x00980
#define   PORT_OUTS_PREQS_HDR_MASK	GENMASK(9, 0)
#define PORT_OUTS_PREQS_DATA		0x00984
#define   PORT_OUTS_PREQS_DATA_MASK	GENMASK(15, 0)
#define PORT_TUNCTRL			0x00988
#define   PORT_TUNCTRL_PERST_ON		BIT(0)
#define   PORT_TUNCTRL_PERST_ACK_REQ	BIT(1)
#define PORT_TUNSTAT			0x0098c
#define   PORT_TUNSTAT_PERST_ON		BIT(0)
#define   PORT_TUNSTAT_PERST_ACK_PEND	BIT(1)
#define PORT_PREFMEM_ENABLE		0x00994

struct reg_info {
	u32 phy_lane_ctl;
	u32 port_refclk;
	u32 port_perst;
};

const struct reg_info t8103_hw = {
	.phy_lane_ctl = PHY_LANE_CTL,
	.port_refclk = PORT_REFCLK,
	.port_perst = PORT_PERST,
};

#define PORT_T602X_PERST		0x082c

const struct reg_info t602x_hw = {
	.phy_lane_ctl = 0,
	.port_refclk = 0,
	.port_perst = PORT_T602X_PERST,
};

struct apple_pcie_priv {
	struct udevice		*dev;
	void __iomem            *base;
	void __iomem            *cfg_base;
	struct list_head	ports;
	const struct reg_info	*hw;
};

struct apple_pcie_port {
	struct apple_pcie_priv	*pcie;
	struct gpio_desc	reset;
	ofnode			np;
	void __iomem		*base;
	void __iomem		*phy;
	struct list_head	entry;
	int			idx;
};

static void rmw_set(u32 set, void __iomem *addr)
{
	writel_relaxed(readl_relaxed(addr) | set, addr);
}

static void rmw_clear(u32 clr, void __iomem *addr)
{
	writel_relaxed(readl_relaxed(addr) & ~clr, addr);
}

static int apple_pcie_config_address(const struct udevice *bus,
				     pci_dev_t bdf, uint offset,
				     void **paddress)
{
	struct apple_pcie_priv *pcie = dev_get_priv(bus);
	void *addr;

	addr = pcie->cfg_base;
	addr += PCIE_ECAM_OFFSET(PCI_BUS(bdf), PCI_DEV(bdf),
				 PCI_FUNC(bdf), offset);
	*paddress = addr;

	return 0;
}

static int apple_pcie_read_config(const struct udevice *bus, pci_dev_t bdf,
				  uint offset, ulong *valuep,
				  enum pci_size_t size)
{
	int ret;

	ret = pci_generic_mmap_read_config(bus, apple_pcie_config_address,
					   bdf, offset, valuep, size);
	return ret;
}

static int apple_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				   uint offset, ulong value,
				   enum pci_size_t size)
{
	return pci_generic_mmap_write_config(bus, apple_pcie_config_address,
					     bdf, offset, value, size);
}

static const struct dm_pci_ops apple_pcie_ops = {
	.read_config = apple_pcie_read_config,
	.write_config = apple_pcie_write_config,
};

static int apple_pcie_setup_refclk(struct apple_pcie_priv *pcie,
				   struct apple_pcie_port *port)
{
	u32 stat;
	int res;

	if (pcie->hw->phy_lane_ctl)
		rmw_set(PHY_LANE_CTL_CFGACC, port->phy + pcie->hw->phy_lane_ctl);

	rmw_set(PHY_LANE_CFG_REFCLK0REQ, port->phy + PHY_LANE_CFG);

	res = readl_poll_sleep_timeout(port->phy + PHY_LANE_CFG,
				       stat, stat & PHY_LANE_CFG_REFCLK0ACK,
				       100, 50000);
	if (res < 0)
		return res;

	rmw_set(PHY_LANE_CFG_REFCLK1REQ, port->phy + PHY_LANE_CFG);
	res = readl_poll_sleep_timeout(port->phy + PHY_LANE_CFG,
				       stat, stat & PHY_LANE_CFG_REFCLK1ACK,
				       100, 50000);

	if (res < 0)
		return res;

	if (pcie->hw->phy_lane_ctl)
		rmw_clear(PHY_LANE_CTL_CFGACC, port->phy + pcie->hw->phy_lane_ctl);

	rmw_set(PHY_LANE_CFG_REFCLKEN, port->phy + PHY_LANE_CFG);

	if (pcie->hw->port_refclk)
		rmw_set(PORT_REFCLK_EN, port->base + pcie->hw->port_refclk);

	return 0;
}

static int apple_pcie_setup_port(struct apple_pcie_priv *pcie, ofnode np)
{
	struct apple_pcie_port *port;
	struct gpio_desc reset;
	fdt_addr_t addr;
	u32 stat, idx;
	int ret;
	char name[16];

	ret = gpio_request_by_name_nodev(np, "reset-gpios", 0, &reset, 0);
	if (ret)
		return ret;

	port = devm_kzalloc(pcie->dev, sizeof(*port), GFP_KERNEL);
	if (!port)
		return -ENOMEM;

	ret = ofnode_read_u32_index(np, "reg", 0, &idx);
	if (ret)
		return ret;

	/* Use the first reg entry to work out the port index */
	port->idx = idx >> 11;
	port->pcie = pcie;
	port->reset = reset;
	port->np = np;

	snprintf(name, sizeof(name), "port%d", port->idx);
	addr = dev_read_addr_name(pcie->dev, name);
	if (addr == FDT_ADDR_T_NONE)
		addr = dev_read_addr_index(pcie->dev, port->idx + 2);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;
	port->base = map_sysmem(addr, 0);

	snprintf(name, sizeof(name), "phy%d", port->idx);
	addr = dev_read_addr_name(pcie->dev, name);
	if (addr == FDT_ADDR_T_NONE)
		port->phy = pcie->base + CORE_PHY_DEFAULT_BASE(port->idx);
	else
		port->phy = map_sysmem(addr, 0);

	rmw_set(PORT_APPCLK_EN, port->base + PORT_APPCLK);

	/* Assert PERST# before setting up the clock */
	dm_gpio_set_value(&reset, 1);

	ret = apple_pcie_setup_refclk(pcie, port);
	if (ret < 0)
		return ret;

	/* The minimal Tperst-clk value is 100us (PCIe CEM r5.0, 2.9.2) */
	udelay(100);

	/* Deassert PERST# */
	rmw_set(PORT_PERST_OFF, port->base + pcie->hw->port_perst);
	dm_gpio_set_value(&reset, 0);

	/* Wait for 100ms after PERST# deassertion (PCIe r5.0, 6.6.1) */
	udelay(100 * 1000);

	ret = readl_poll_sleep_timeout(port->base + PORT_STATUS, stat,
				       stat & PORT_STATUS_READY, 100, 250000);
	if (ret < 0) {
		dev_err(pcie->dev, "port %d ready wait timeout\n", port->idx);
		return ret;
	}

	list_add_tail(&port->entry, &pcie->ports);

	writel_relaxed(PORT_LTSSMCTL_START, port->base + PORT_LTSSMCTL);

	/*
	 * Deliberately ignore the link not coming up as connected
	 * devices (e.g. the WiFi controller) may not be powerd up.
	 */
	readl_poll_sleep_timeout(port->base + PORT_LINKSTS, stat,
				 (stat & PORT_LINKSTS_UP), 100, 100000);

	if (pcie->hw->port_refclk)
		rmw_clear(PORT_REFCLK_CGDIS, port->base + PORT_REFCLK);
	else
		rmw_set(PHY_LANE_CFG_REFCLKCGEN, port->phy + PHY_LANE_CFG);
	rmw_clear(PORT_APPCLK_CGDIS, port->base + PORT_APPCLK);

	return 0;
}

static int apple_pcie_probe(struct udevice *dev)
{
	struct apple_pcie_priv *pcie = dev_get_priv(dev);
	fdt_addr_t addr;
	ofnode of_port;
	int i, ret;

	pcie->hw = (struct reg_info *)dev_get_driver_data(dev);

	pcie->dev = dev;
	addr = dev_read_addr_index(dev, 0);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;
	pcie->cfg_base = map_sysmem(addr, 0);

	addr = dev_read_addr_index(dev, 1);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;
	pcie->base = map_sysmem(addr, 0);

	INIT_LIST_HEAD(&pcie->ports);

	for (of_port = ofnode_first_subnode(dev_ofnode(dev));
	     ofnode_valid(of_port);
	     of_port = ofnode_next_subnode(of_port)) {
		if (!ofnode_is_enabled(of_port))
			continue;
		ret = apple_pcie_setup_port(pcie, of_port);
		if (ret) {
			dev_err(pcie->dev, "Port %d setup fail: %d\n", i, ret);
			return ret;
		}
	}

	return 0;
}

static int apple_pcie_remove(struct udevice *dev)
{
	struct apple_pcie_priv *pcie = dev_get_priv(dev);
	struct apple_pcie_port *port, *tmp;

	list_for_each_entry_safe(port, tmp, &pcie->ports, entry) {
		gpio_free_list_nodev(&port->reset, 1);
		free(port);
	}

	return 0;
}

static const struct udevice_id apple_pcie_of_match[] = {
	{ .compatible = "apple,t6020-pcie", .data = (ulong)&t602x_hw },
	{ .compatible = "apple,pcie", .data = (ulong)&t8103_hw },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(apple_pcie) = {
	.name = "apple_pcie",
	.id = UCLASS_PCI,
	.of_match = apple_pcie_of_match,
	.probe = apple_pcie_probe,
	.remove = apple_pcie_remove,
	.priv_auto = sizeof(struct apple_pcie_priv),
	.ops = &apple_pcie_ops,
};
