// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek PCIe host controller driver.
 *
 * Copyright (c) 2017-2019 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 *	   Honghui Zhang <honghui.zhang@mediatek.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <log.h>
#include <malloc.h>
#include <pci.h>
#include <reset.h>
#include <asm/io.h>
#include <dm/devres.h>
#include <linux/bitops.h>
#include <linux/iopoll.h>
#include <linux/list.h>
#include <linux/printk.h>
#include "pci_internal.h"

/* PCIe shared registers */
#define PCIE_SYS_CFG		0x00
#define PCIE_INT_ENABLE		0x0c
#define PCIE_CFG_ADDR		0x20
#define PCIE_CFG_DATA		0x24

/* PCIe per port registers */
#define PCIE_BAR0_SETUP		0x10
#define PCIE_CLASS		0x34
#define PCIE_LINK_STATUS	0x50

#define PCIE_PORT_INT_EN(x)	BIT(20 + (x))
#define PCIE_PORT_PERST(x)	BIT(1 + (x))
#define PCIE_PORT_LINKUP	BIT(0)
#define PCIE_BAR_MAP_MAX	GENMASK(31, 16)

#define PCIE_BAR_ENABLE		BIT(0)
#define PCIE_REVISION_ID	BIT(0)
#define PCIE_CLASS_CODE		(0x60400 << 8)

/* MediaTek specific configuration registers */
#define PCIE_FTS_NUM		0x70c
#define PCIE_FTS_NUM_MASK	GENMASK(15, 8)
#define PCIE_FTS_NUM_L0(x)	((x) & 0xff << 8)

#define PCIE_FC_CREDIT		0x73c
#define PCIE_FC_CREDIT_MASK	(GENMASK(31, 31) | GENMASK(28, 16))
#define PCIE_FC_CREDIT_VAL(x)	((x) << 16)

/* PCIe V2 share registers */
#define PCIE_SYS_CFG_V2		0x0
#define PCIE_CSR_LTSSM_EN(x)	BIT(0 + (x) * 8)
#define PCIE_CSR_ASPM_L1_EN(x)	BIT(1 + (x) * 8)

/* PCIe V2 per-port registers */
#define PCIE_CONF_VEND_ID	0x100
#define PCIE_CONF_DEVICE_ID	0x102
#define PCIE_CONF_CLASS_ID	0x106

#define PCIE_AHB_TRANS_BASE0_L	0x438
#define PCIE_AHB_TRANS_BASE0_H	0x43c
#define AHB2PCIE_SIZE(x)	((x) & GENMASK(4, 0))
#define PCIE_AXI_WINDOW0	0x448
#define WIN_ENABLE		BIT(7)

/*
 * Define PCIe to AHB window size as 2^33 to support max 8GB address space
 * translate, support least 4GB DRAM size access from EP DMA(physical DRAM
 * start from 0x40000000).
 */
#define PCIE2AHB_SIZE	0x21

/* PCIe V2 configuration transaction header */
#define PCIE_CFG_HEADER0	0x460
#define PCIE_CFG_HEADER1	0x464
#define PCIE_CFG_HEADER2	0x468
#define PCIE_CFG_WDATA		0x470
#define PCIE_APP_TLP_REQ	0x488
#define PCIE_CFG_RDATA		0x48c
#define APP_CFG_REQ		BIT(0)
#define APP_CPL_STATUS		GENMASK(7, 5)

#define CFG_WRRD_TYPE_0		4
#define CFG_WR_FMT		2
#define CFG_RD_FMT		0

#define CFG_DW0_LENGTH(length)	((length) & GENMASK(9, 0))
#define CFG_DW0_TYPE(type)	(((type) << 24) & GENMASK(28, 24))
#define CFG_DW0_FMT(fmt)	(((fmt) << 29) & GENMASK(31, 29))
#define CFG_DW2_REGN(regn)	((regn) & GENMASK(11, 2))
#define CFG_DW2_FUN(fun)	(((fun) << 16) & GENMASK(18, 16))
#define CFG_DW2_DEV(dev)	(((dev) << 19) & GENMASK(23, 19))
#define CFG_DW2_BUS(bus)	(((bus) << 24) & GENMASK(31, 24))
#define CFG_HEADER_DW0(type, fmt) \
	(CFG_DW0_LENGTH(1) | CFG_DW0_TYPE(type) | CFG_DW0_FMT(fmt))
#define CFG_HEADER_DW1(where, size) \
	(GENMASK(((size) - 1), 0) << ((where) & 0x3))
#define CFG_HEADER_DW2(regn, fun, dev, bus) \
	(CFG_DW2_REGN(regn) | CFG_DW2_FUN(fun) | \
	CFG_DW2_DEV(dev) | CFG_DW2_BUS(bus))

#define PCIE_RST_CTRL		0x510
#define PCIE_PHY_RSTB		BIT(0)
#define PCIE_PIPE_SRSTB		BIT(1)
#define PCIE_MAC_SRSTB		BIT(2)
#define PCIE_CRSTB		BIT(3)
#define PCIE_PERSTB		BIT(8)
#define PCIE_LINKDOWN_RST_EN	GENMASK(15, 13)
#define PCIE_LINK_STATUS_V2	0x804
#define PCIE_PORT_LINKUP_V2	BIT(11)

#define PCI_VENDOR_ID_MEDIATEK	0x14c3

enum MTK_PCIE_GEN {PCIE_V1, PCIE_V2, PCIE_V3};

struct mtk_pcie_port {
	void __iomem *base;
	struct list_head list;
	struct mtk_pcie *pcie;
	struct reset_ctl reset;
	struct clk sys_ck;
	struct clk ahb_ck;
	struct clk axi_ck;
	struct clk aux_ck;
	struct clk obff_ck;
	struct clk pipe_ck;
	struct phy phy;
	u32 slot;
};

struct mtk_pcie {
	void __iomem *base;
	void *priv;
	struct clk free_ck;
	struct list_head ports;
};

static int mtk_pcie_config_address(const struct udevice *udev, pci_dev_t bdf,
				   uint offset, void **paddress)
{
	struct mtk_pcie *pcie = dev_get_priv(udev);
	u32 val;

	val = PCI_CONF1_EXT_ADDRESS(PCI_BUS(bdf), PCI_DEV(bdf),
				    PCI_FUNC(bdf), offset) & ~PCI_CONF1_ENABLE;
	writel(val, pcie->base + PCIE_CFG_ADDR);
	*paddress = pcie->base + PCIE_CFG_DATA + (offset & 3);

	return 0;
}

static int mtk_pcie_read_config(const struct udevice *bus, pci_dev_t bdf,
				uint offset, ulong *valuep,
				enum pci_size_t size)
{
	return pci_generic_mmap_read_config(bus, mtk_pcie_config_address,
					    bdf, offset, valuep, size);
}

static int mtk_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				 uint offset, ulong value,
				 enum pci_size_t size)
{
	return pci_generic_mmap_write_config(bus, mtk_pcie_config_address,
					     bdf, offset, value, size);
}

static const struct dm_pci_ops mtk_pcie_ops = {
	.read_config	= mtk_pcie_read_config,
	.write_config	= mtk_pcie_write_config,
};

static int mtk_pcie_check_cfg_cpld(struct mtk_pcie_port *port)
{
	u32 val;
	int err;

	err = readl_poll_timeout(port->base + PCIE_APP_TLP_REQ, val,
				 !(val & APP_CFG_REQ), 100 * 1000);
	if (err)
		return -1;

	if (readl(port->base + PCIE_APP_TLP_REQ) & APP_CPL_STATUS)
		return -1;

	return 0;
}

static int mtk_pcie_hw_rd_cfg(struct mtk_pcie_port *port, u32 bus, pci_dev_t devfn,
			      int where, int size, ulong *val)
{
	u32 tmp;

	writel(CFG_HEADER_DW0(CFG_WRRD_TYPE_0, CFG_RD_FMT),
	       port->base + PCIE_CFG_HEADER0);
	writel(CFG_HEADER_DW1(where, size), port->base + PCIE_CFG_HEADER1);
	writel(CFG_HEADER_DW2(where, PCI_FUNC(devfn), PCI_DEV(devfn), bus),
	       port->base + PCIE_CFG_HEADER2);

	/* Trigger h/w to transmit Cfgrd TLP */
	tmp = readl(port->base + PCIE_APP_TLP_REQ);
	tmp |= APP_CFG_REQ;
	writel(tmp, port->base + PCIE_APP_TLP_REQ);

	/* Check completion status */
	if (mtk_pcie_check_cfg_cpld(port))
		return -1;

	/* Read cpld payload of Cfgrd */
	*val = readl(port->base + PCIE_CFG_RDATA);

	if (size == 1)
		*val = (*val >> (8 * (where & 3))) & 0xff;
	else if (size == 2)
		*val = (*val >> (8 * (where & 3))) & 0xffff;

	return 0;
}

static int mtk_pcie_hw_wr_cfg(struct mtk_pcie_port *port, u32 bus, pci_dev_t devfn,
			      int where, int size, u32 val)
{
	/* Write PCIe configuration transaction header for Cfgwr */
	writel(CFG_HEADER_DW0(CFG_WRRD_TYPE_0, CFG_WR_FMT),
	       port->base + PCIE_CFG_HEADER0);
	writel(CFG_HEADER_DW1(where, size), port->base + PCIE_CFG_HEADER1);
	writel(CFG_HEADER_DW2(where, PCI_FUNC(devfn), PCI_DEV(devfn), bus),
	       port->base + PCIE_CFG_HEADER2);

	/* Write Cfgwr data */
	val = val << 8 * (where & 3);
	writel(val, port->base + PCIE_CFG_WDATA);

	/* Trigger h/w to transmit Cfgwr TLP */
	val = readl(port->base + PCIE_APP_TLP_REQ);
	val |= APP_CFG_REQ;
	writel(val, port->base + PCIE_APP_TLP_REQ);

	/* Check completion status */
	return mtk_pcie_check_cfg_cpld(port);
}

static struct mtk_pcie_port *mtk_pcie_find_port(const struct udevice *bus,
						pci_dev_t bdf)
{
	struct mtk_pcie *pcie = dev_get_priv(bus);
	struct mtk_pcie_port *port;
	struct udevice *dev;
	struct pci_child_plat *pplat = NULL;
	int ret = 0;

	if (PCI_BUS(bdf) != 0) {
		ret = pci_get_bus(PCI_BUS(bdf), &dev);
		if (ret) {
			debug("No such device,ret = %d\n", ret);
			return NULL;
		}

		while (dev_seq(dev->parent) != 0)
			dev = dev->parent;

		pplat = dev_get_parent_plat(dev);
	}

	list_for_each_entry(port, &pcie->ports, list) {
		if ((PCI_BUS(bdf) == 0) && (PCI_DEV(bdf) == port->slot))
			return port;

		if (PCI_BUS(bdf) != 0 && PCI_DEV(bdf) == 0 &&
		    PCI_DEV(pplat->devfn) == port->slot)
			return port;
	}

	return NULL;
}

static int mtk_pcie_config_read(const struct udevice *bus, pci_dev_t bdf,
				uint offset, ulong *valuep,
				enum pci_size_t size)
{
	struct mtk_pcie_port *port;
	int ret;

	port = mtk_pcie_find_port(bus, bdf);
	if (!port) {
		*valuep = pci_get_ff(size);
		return 0;
	}

	ret = mtk_pcie_hw_rd_cfg(port, PCI_BUS(bdf), bdf, offset, (1 << size), valuep);
	if (ret)
		*valuep = pci_get_ff(size);

	return ret;
}

static int mtk_pcie_config_write(struct udevice *bus, pci_dev_t bdf,
				 uint offset, ulong value,
				 enum pci_size_t size)
{
	struct mtk_pcie_port *port;

	port = mtk_pcie_find_port(bus, bdf);
	if (!port)
		return 0;

	/* Do not modify RC bar 0/1. */
	if (PCI_BUS(bdf) == 0 && (offset == 0x10 || offset == 0x14))
		return 0;

	return mtk_pcie_hw_wr_cfg(port, PCI_BUS(bdf), bdf, offset, (1 << size), value);
}

static const struct dm_pci_ops mtk_pcie_ops_v2 = {
	.read_config  = mtk_pcie_config_read,
	.write_config = mtk_pcie_config_write,
};

static void mtk_pcie_port_free(struct mtk_pcie_port *port)
{
	list_del(&port->list);
	free(port);
}

static int mtk_pcie_startup_port(struct mtk_pcie_port *port)
{
	struct mtk_pcie *pcie = port->pcie;
	u32 val;
	int err;

	/* assert port PERST_N */
	setbits_le32(pcie->base + PCIE_SYS_CFG, PCIE_PORT_PERST(port->slot));
	/* de-assert port PERST_N */
	clrbits_le32(pcie->base + PCIE_SYS_CFG, PCIE_PORT_PERST(port->slot));

	/* 100ms timeout value should be enough for Gen1/2 training */
	err = readl_poll_timeout(port->base + PCIE_LINK_STATUS, val,
				 !!(val & PCIE_PORT_LINKUP), 100000);
	if (err)
		return -ETIMEDOUT;

	/* disable interrupt */
	clrbits_le32(pcie->base + PCIE_INT_ENABLE,
		     PCIE_PORT_INT_EN(port->slot));

	/* map to all DDR region. We need to set it before cfg operation. */
	writel(PCIE_BAR_MAP_MAX | PCIE_BAR_ENABLE,
	       port->base + PCIE_BAR0_SETUP);

	/* configure class code and revision ID */
	writel(PCIE_CLASS_CODE | PCIE_REVISION_ID, port->base + PCIE_CLASS);

	/* configure FC credit */
	val = PCI_CONF1_EXT_ADDRESS(0, port->slot, 0, PCIE_FC_CREDIT) & ~PCI_CONF1_ENABLE;
	writel(val, pcie->base + PCIE_CFG_ADDR);
	clrsetbits_le32(pcie->base + PCIE_CFG_DATA, PCIE_FC_CREDIT_MASK,
			PCIE_FC_CREDIT_VAL(0x806c));

	/* configure RC FTS number to 250 when it leaves L0s */
	val = PCI_CONF1_EXT_ADDRESS(0, port->slot, 0, PCIE_FTS_NUM) & ~PCI_CONF1_ENABLE;
	writel(val, pcie->base + PCIE_CFG_ADDR);
	clrsetbits_le32(pcie->base + PCIE_CFG_DATA, PCIE_FTS_NUM_MASK,
			PCIE_FTS_NUM_L0(0x50));

	return 0;
}

static int mtk_pcie_startup_port_v2(struct mtk_pcie_port *port)
{
	struct mtk_pcie *pcie = port->pcie;
	struct udevice *dev = pcie->priv;
	struct pci_region *pci_mem;
	u32 val;
	int err;

	/* MT7622/MT7629 platforms need to enable LTSSM and ASPM from PCIe subsys */
	if (pcie->base) {
		val = readl(pcie->base + PCIE_SYS_CFG_V2);
		val |= PCIE_CSR_LTSSM_EN(port->slot) |
		       PCIE_CSR_ASPM_L1_EN(port->slot);
		writel(val, pcie->base + PCIE_SYS_CFG_V2);
	}

	/* Assert all reset signals */
	writel(0, port->base + PCIE_RST_CTRL);

	/*
	 * Enable PCIe link down reset, if link status changed from link up to
	 * link down, this will reset MAC control registers and configuration
	 * space.
	 */
	writel(PCIE_LINKDOWN_RST_EN, port->base + PCIE_RST_CTRL);
	udelay(500);

	/* De-assert PHY, PE, PIPE, MAC and configuration reset	*/
	val = readl(port->base + PCIE_RST_CTRL);
	val |= PCIE_PHY_RSTB | PCIE_PIPE_SRSTB | PCIE_MAC_SRSTB | PCIE_CRSTB;
	writel(val, port->base + PCIE_RST_CTRL);

	mdelay(100);
	val |= PCIE_PERSTB;
	writel(val, port->base + PCIE_RST_CTRL);

	/* Set up vendor ID and class code */
	val = PCI_VENDOR_ID_MEDIATEK;
	writew(val, port->base + PCIE_CONF_VEND_ID);

	val = PCI_CLASS_BRIDGE_PCI;
	writew(val, port->base + PCIE_CONF_CLASS_ID);

	/* 100ms timeout value should be enough for Gen1/2 training */
	err = readl_poll_timeout(port->base + PCIE_LINK_STATUS_V2, val,
				 !!(val & PCIE_PORT_LINKUP_V2),
				 100 * 1000);
	if (err)
		return -ETIMEDOUT;

	pci_get_regions(dev, NULL, &pci_mem, NULL);

	/* Set AHB to PCIe translation windows */
	val = lower_32_bits(pci_mem->bus_start) |
	      AHB2PCIE_SIZE(fls(pci_mem->size) - 1);
	writel(val, port->base + PCIE_AHB_TRANS_BASE0_L);

	val = upper_32_bits(pci_mem->bus_start);
	writel(val, port->base + PCIE_AHB_TRANS_BASE0_H);

	/* Set PCIe to AXI translation memory space.*/
	val = PCIE2AHB_SIZE | WIN_ENABLE;
	writel(val, port->base + PCIE_AXI_WINDOW0);

	return 0;
}

static void mtk_pcie_enable_port(struct mtk_pcie_port *port)
{
	int err;

	err = clk_enable(&port->sys_ck);
	if (err)
		goto err_sys_clk;

	err = reset_assert(&port->reset);
	if (err)
		goto err_reset;

	err = reset_deassert(&port->reset);
	if (err)
		goto err_reset;

	err = generic_phy_init(&port->phy);
	if (err)
		goto err_phy_init;

	err = generic_phy_power_on(&port->phy);
	if (err)
		goto err_phy_on;

	if (!mtk_pcie_startup_port(port))
		return;

	pr_err("Port%d link down\n", port->slot);

	generic_phy_power_off(&port->phy);
err_phy_on:
	generic_phy_exit(&port->phy);
err_phy_init:
err_reset:
	clk_disable(&port->sys_ck);
err_sys_clk:
	mtk_pcie_port_free(port);
}

static void mtk_pcie_enable_port_v2(struct mtk_pcie_port *port)
{
	int err = 0;

	err = clk_enable(&port->sys_ck);
	if (err) {
		debug("clk_enable(sys_ck) failed: %d\n", err);
		goto exit;
	}

	err = clk_enable(&port->ahb_ck);
	if (err) {
		debug("clk_enable(ahb_ck) failed: %d\n", err);
		goto exit;
	}

	err = clk_enable(&port->aux_ck);
	if (err) {
		debug("clk_enable(aux_ck) failed: %d\n", err);
		goto exit;
	}

	err = clk_enable(&port->axi_ck);
	if (err) {
		debug("clk_enable(axi_ck) failed: %d\n", err);
		goto exit;
	}

	err = clk_enable(&port->obff_ck);
	if (err) {
		debug("clk_enable(obff_ck) failed: %d\n", err);
		goto exit;
	}

	err = clk_enable(&port->pipe_ck);
	if (err) {
		debug("clk_enable(pipe_ck) failed: %d\n", err);
		goto exit;
	}

	err = mtk_pcie_startup_port_v2(port);
	if (!err)
		return;

	pr_err("Port%d link down\n", port->slot);
exit:
	mtk_pcie_port_free(port);
}

static int mtk_pcie_parse_port(struct udevice *dev, u32 slot)
{
	struct mtk_pcie *pcie = dev_get_priv(dev);
	struct mtk_pcie_port *port;
	char name[10];
	int err;

	port = devm_kzalloc(dev, sizeof(*port), GFP_KERNEL);
	if (!port)
		return -ENOMEM;

	snprintf(name, sizeof(name), "port%d", slot);
	port->base = dev_remap_addr_name(dev, name);
	if (!port->base)
		return -ENOENT;

	snprintf(name, sizeof(name), "sys_ck%d", slot);
	err = clk_get_by_name(dev, name, &port->sys_ck);
	if (err)
		return err;

	err = reset_get_by_index(dev, slot, &port->reset);
	if (err)
		return err;

	err = generic_phy_get_by_index(dev, slot, &port->phy);
	if (err)
		return err;

	port->slot = slot;
	port->pcie = pcie;

	INIT_LIST_HEAD(&port->list);
	list_add_tail(&port->list, &pcie->ports);

	return 0;
}

static int mtk_pcie_parse_port_v2(struct udevice *dev, u32 slot)
{
	struct mtk_pcie *pcie = dev_get_priv(dev);
	struct mtk_pcie_port *port;
	char name[10];
	int err;

	port = devm_kzalloc(dev, sizeof(*port), GFP_KERNEL);
	if (!port)
		return -ENOMEM;

	snprintf(name, sizeof(name), "port%d", slot);
	port->base = dev_remap_addr_name(dev, name);
	if (!port->base) {
		debug("failed to map port%d base\n", slot);
		return -ENOENT;
	}

	snprintf(name, sizeof(name), "sys_ck%d", slot);
	err = clk_get_by_name(dev, name, &port->sys_ck);
	if (err) {
		debug("clk_get_by_name(sys_ck) failed: %d\n", err);
		return err;
	}

	snprintf(name, sizeof(name), "ahb_ck%d", slot);
	err = clk_get_by_name(dev, name, &port->ahb_ck);
	if (err) {
		debug("clk_get_by_name(ahb_ck) failed: %d\n", err);
		return err;
	}

	snprintf(name, sizeof(name), "aux_ck%d", slot);
	err = clk_get_by_name(dev, name, &port->aux_ck);
	if (err) {
		debug("clk_get_by_name(aux_ck) failed: %d\n", err);
		return err;
	}

	snprintf(name, sizeof(name), "axi_ck%d", slot);
	err = clk_get_by_name(dev, name, &port->axi_ck);
	if (err) {
		debug("clk_get_by_name(axi_ck) failed: %d\n", err);
		return err;
	}

	snprintf(name, sizeof(name), "obff_ck%d", slot);
	err = clk_get_by_name(dev, name, &port->obff_ck);
	if (err) {
		debug("clk_get_by_name(obff_ck) failed: %d\n", err);
		return err;
	}

	snprintf(name, sizeof(name), "pipe_ck%d", slot);
	err = clk_get_by_name(dev, name, &port->pipe_ck);
	if (err) {
		debug("clk_get_by_name(pipe_ck) failed: %d\n", err);
		return err;
	}

	port->slot = slot;
	port->pcie = pcie;

	INIT_LIST_HEAD(&port->list);
	list_add_tail(&port->list, &pcie->ports);

	return 0;
}

static int mtk_pcie_probe(struct udevice *dev)
{
	struct mtk_pcie *pcie = dev_get_priv(dev);
	struct mtk_pcie_port *port, *tmp;
	ofnode subnode;
	int err;

	INIT_LIST_HEAD(&pcie->ports);

	pcie->base = dev_remap_addr_name(dev, "subsys");
	if (!pcie->base)
		return -ENOENT;

	err = clk_get_by_name(dev, "free_ck", &pcie->free_ck);
	if (err)
		return err;

	/* enable top level clock */
	err = clk_enable(&pcie->free_ck);
	if (err)
		return err;

	dev_for_each_subnode(subnode, dev) {
		struct fdt_pci_addr addr;
		u32 slot = 0;

		if (!ofnode_is_enabled(subnode))
			continue;

		err = ofnode_read_pci_addr(subnode, 0, "reg", &addr, NULL);
		if (err)
			return err;

		slot = PCI_DEV(addr.phys_hi);

		err = mtk_pcie_parse_port(dev, slot);
		if (err)
			return err;
	}

	/* enable each port, and then check link status */
	list_for_each_entry_safe(port, tmp, &pcie->ports, list)
		mtk_pcie_enable_port(port);

	return 0;
}

static int mtk_pcie_probe_v2(struct udevice *dev)
{
	struct mtk_pcie *pcie = dev_get_priv(dev);
	struct mtk_pcie_port *port, *tmp;
	struct fdt_pci_addr addr;
	ofnode subnode;
	unsigned int slot;
	int err;

	INIT_LIST_HEAD(&pcie->ports);

	pcie->base = dev_remap_addr_name(dev, "subsys");
	if (!pcie->base)
		return -ENOENT;

	pcie->priv = dev;

	dev_for_each_subnode(subnode, dev) {
		if (!ofnode_is_enabled(subnode))
			continue;

		err = ofnode_read_pci_addr(subnode, 0, "reg", &addr, NULL);
		if (err)
			return err;

		slot = PCI_DEV(addr.phys_hi);
		err = mtk_pcie_parse_port_v2(dev, slot);
		if (err)
			return err;
	}

	/* enable each port, and then check link status */
	list_for_each_entry_safe(port, tmp, &pcie->ports, list)
		mtk_pcie_enable_port_v2(port);

	return 0;
}

static const struct udevice_id mtk_pcie_ids[] = {
	{ .compatible = "mediatek,mt7623-pcie", PCIE_V1},
	{ }
};

U_BOOT_DRIVER(pcie_mediatek_v1) = {
	.name	= "pcie_mediatek_v1",
	.id	= UCLASS_PCI,
	.of_match = mtk_pcie_ids,
	.ops	= &mtk_pcie_ops,
	.probe	= mtk_pcie_probe,
	.priv_auto	= sizeof(struct mtk_pcie),
};

static const struct udevice_id mtk_pcie_ids_v2[] = {
	{ .compatible = "mediatek,mt7622-pcie", PCIE_V2},
	{ }
};

U_BOOT_DRIVER(pcie_mediatek_v2) = {
	.name	= "pcie_mediatek_v2",
	.id	= UCLASS_PCI,
	.of_match = mtk_pcie_ids_v2,
	.ops	= &mtk_pcie_ops_v2,
	.probe	= mtk_pcie_probe_v2,
	.priv_auto	= sizeof(struct mtk_pcie),
};
