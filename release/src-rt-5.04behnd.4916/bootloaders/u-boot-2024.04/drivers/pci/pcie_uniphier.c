// SPDX-License-Identifier: GPL-2.0
/*
 * pcie_uniphier.c - Socionext UniPhier PCIe driver
 * Copyright 2019-2021 Socionext, Inc.
 */

#include <clk.h>
#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <generic-phy.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <pci.h>
#include <reset.h>

DECLARE_GLOBAL_DATA_PTR;

/* DBI registers */
#define PCIE_LINK_STATUS_REG		0x0080
#define PCIE_LINK_STATUS_WIDTH_MASK	GENMASK(25, 20)
#define PCIE_LINK_STATUS_SPEED_MASK	GENMASK(19, 16)

#define PCIE_MISC_CONTROL_1_OFF		0x08BC
#define PCIE_DBI_RO_WR_EN		BIT(0)

/* DBI iATU registers */
#define PCIE_ATU_VIEWPORT		0x0900
#define PCIE_ATU_REGION_INBOUND		BIT(31)
#define PCIE_ATU_REGION_OUTBOUND	0
#define PCIE_ATU_REGION_INDEX_MASK	GENMASK(3, 0)

#define PCIE_ATU_CR1			0x0904
#define PCIE_ATU_TYPE_MEM		0
#define PCIE_ATU_TYPE_IO		2
#define PCIE_ATU_TYPE_CFG0		4
#define PCIE_ATU_TYPE_CFG1		5

#define PCIE_ATU_CR2			0x0908
#define PCIE_ATU_ENABLE			BIT(31)
#define PCIE_ATU_MATCH_MODE		BIT(30)
#define PCIE_ATU_BAR_NUM_MASK		GENMASK(10, 8)

#define PCIE_ATU_LOWER_BASE		0x090C
#define PCIE_ATU_UPPER_BASE		0x0910
#define PCIE_ATU_LIMIT			0x0914
#define PCIE_ATU_LOWER_TARGET		0x0918
#define PCIE_ATU_BUS(x)			FIELD_PREP(GENMASK(31, 24), x)
#define PCIE_ATU_DEV(x)			FIELD_PREP(GENMASK(23, 19), x)
#define PCIE_ATU_FUNC(x)		FIELD_PREP(GENMASK(18, 16), x)
#define PCIE_ATU_UPPER_TARGET		0x091C

/* Link Glue registers */
#define PCL_PINCTRL0			0x002c
#define PCL_PERST_PLDN_REGEN		BIT(12)
#define PCL_PERST_NOE_REGEN		BIT(11)
#define PCL_PERST_OUT_REGEN		BIT(8)
#define PCL_PERST_PLDN_REGVAL		BIT(4)
#define PCL_PERST_NOE_REGVAL		BIT(3)
#define PCL_PERST_OUT_REGVAL		BIT(0)

#define PCL_MODE			0x8000
#define PCL_MODE_REGEN			BIT(8)
#define PCL_MODE_REGVAL			BIT(0)

#define PCL_APP_READY_CTRL		0x8008
#define PCL_APP_LTSSM_ENABLE		BIT(0)

#define PCL_APP_PM0			0x8078
#define PCL_SYS_AUX_PWR_DET		BIT(8)

#define PCL_STATUS_LINK			0x8140
#define PCL_RDLH_LINK_UP		BIT(1)
#define PCL_XMLH_LINK_UP		BIT(0)

#define LINK_UP_TIMEOUT_MS		100

struct uniphier_pcie_priv {
	void *base;
	void *dbi_base;
	void *cfg_base;
	fdt_size_t cfg_size;
	struct fdt_resource link_res;
	struct fdt_resource dbi_res;
	struct fdt_resource cfg_res;

	struct clk clk;
	struct reset_ctl rst;
	struct phy phy;

	struct pci_region io;
	struct pci_region mem;
};

static int pcie_dw_get_link_speed(struct uniphier_pcie_priv *priv)
{
	u32 val = readl(priv->dbi_base + PCIE_LINK_STATUS_REG);

	return FIELD_GET(PCIE_LINK_STATUS_SPEED_MASK, val);
}

static int pcie_dw_get_link_width(struct uniphier_pcie_priv *priv)
{
	u32 val = readl(priv->dbi_base + PCIE_LINK_STATUS_REG);

	return FIELD_GET(PCIE_LINK_STATUS_WIDTH_MASK, val);
}

static void pcie_dw_prog_outbound_atu(struct uniphier_pcie_priv *priv,
				      int index, int type, u64 cpu_addr,
				      u64 pci_addr, u32 size)
{
	writel(PCIE_ATU_REGION_OUTBOUND
	       | FIELD_PREP(PCIE_ATU_REGION_INDEX_MASK, index),
	       priv->dbi_base + PCIE_ATU_VIEWPORT);
	writel(lower_32_bits(cpu_addr),
	       priv->dbi_base + PCIE_ATU_LOWER_BASE);
	writel(upper_32_bits(cpu_addr),
	       priv->dbi_base + PCIE_ATU_UPPER_BASE);
	writel(lower_32_bits(cpu_addr + size - 1),
	       priv->dbi_base + PCIE_ATU_LIMIT);
	writel(lower_32_bits(pci_addr),
	       priv->dbi_base + PCIE_ATU_LOWER_TARGET);
	writel(upper_32_bits(pci_addr),
	       priv->dbi_base + PCIE_ATU_UPPER_TARGET);

	writel(type, priv->dbi_base + PCIE_ATU_CR1);
	writel(PCIE_ATU_ENABLE, priv->dbi_base + PCIE_ATU_CR2);
}

static int uniphier_pcie_addr_valid(pci_dev_t bdf, int first_busno)
{
	/* accept only device {0,1} on first bus */
	if ((PCI_BUS(bdf) != first_busno) || (PCI_DEV(bdf) > 1))
		return -EINVAL;

	return 0;
}

static int uniphier_pcie_conf_address(const struct udevice *dev, pci_dev_t bdf,
				      uint offset, void **paddr)
{
	struct uniphier_pcie_priv *priv = dev_get_priv(dev);
	u32 busdev;
	int seq = dev_seq(dev);
	int ret;

	ret = uniphier_pcie_addr_valid(bdf, seq);
	if (ret)
		return ret;

	if ((PCI_BUS(bdf) == seq) && !PCI_DEV(bdf)) {
		*paddr = (void *)(priv->dbi_base + offset);
		return 0;
	}

	busdev = PCIE_ATU_BUS(PCI_BUS(bdf) - seq)
		| PCIE_ATU_DEV(PCI_DEV(bdf))
		| PCIE_ATU_FUNC(PCI_FUNC(bdf));

	pcie_dw_prog_outbound_atu(priv, 0,
				  PCIE_ATU_TYPE_CFG0, (u64)priv->cfg_base,
				  busdev, priv->cfg_size);
	*paddr = (void *)(priv->cfg_base + offset);

	return 0;
}

static int uniphier_pcie_read_config(const struct udevice *dev, pci_dev_t bdf,
				     uint offset, ulong *valp,
				     enum pci_size_t size)
{
	return pci_generic_mmap_read_config(dev, uniphier_pcie_conf_address,
					    bdf, offset, valp, size);
}

static int uniphier_pcie_write_config(struct udevice *dev, pci_dev_t bdf,
				      uint offset, ulong val,
				      enum pci_size_t size)
{
	return pci_generic_mmap_write_config(dev, uniphier_pcie_conf_address,
					     bdf, offset, val, size);
}

static void uniphier_pcie_ltssm_enable(struct uniphier_pcie_priv *priv,
				       bool enable)
{
	u32 val;

	val = readl(priv->base + PCL_APP_READY_CTRL);
	if (enable)
		val |= PCL_APP_LTSSM_ENABLE;
	else
		val &= ~PCL_APP_LTSSM_ENABLE;
	writel(val, priv->base + PCL_APP_READY_CTRL);
}

static int uniphier_pcie_link_up(struct uniphier_pcie_priv *priv)
{
	u32 val, mask;

	val = readl(priv->base + PCL_STATUS_LINK);
	mask = PCL_RDLH_LINK_UP | PCL_XMLH_LINK_UP;

	return (val & mask) == mask;
}

static int uniphier_pcie_wait_link(struct uniphier_pcie_priv *priv)
{
	unsigned long timeout;

	timeout = get_timer(0) + LINK_UP_TIMEOUT_MS;

	while (get_timer(0) < timeout) {
		if (uniphier_pcie_link_up(priv))
			return 0;
	}

	return -ETIMEDOUT;
}

static int uniphier_pcie_establish_link(struct uniphier_pcie_priv *priv)
{
	if (uniphier_pcie_link_up(priv))
		return 0;

	uniphier_pcie_ltssm_enable(priv, true);

	return uniphier_pcie_wait_link(priv);
}

static void uniphier_pcie_init_rc(struct uniphier_pcie_priv *priv)
{
	u32 val;

	/* set RC mode */
	val = readl(priv->base + PCL_MODE);
	val |= PCL_MODE_REGEN;
	val &= ~PCL_MODE_REGVAL;
	writel(val, priv->base + PCL_MODE);

	/* use auxiliary power detection */
	val = readl(priv->base + PCL_APP_PM0);
	val |= PCL_SYS_AUX_PWR_DET;
	writel(val, priv->base + PCL_APP_PM0);

	/* assert PERST# */
	val = readl(priv->base + PCL_PINCTRL0);
	val &= ~(PCL_PERST_NOE_REGVAL | PCL_PERST_OUT_REGVAL
		 | PCL_PERST_PLDN_REGVAL);
	val |= PCL_PERST_NOE_REGEN | PCL_PERST_OUT_REGEN
		| PCL_PERST_PLDN_REGEN;
	writel(val, priv->base + PCL_PINCTRL0);

	uniphier_pcie_ltssm_enable(priv, false);

	mdelay(100);

	/* deassert PERST# */
	val = readl(priv->base + PCL_PINCTRL0);
	val |= PCL_PERST_OUT_REGVAL | PCL_PERST_OUT_REGEN;
	writel(val, priv->base + PCL_PINCTRL0);
}

static void uniphier_pcie_setup_rc(struct uniphier_pcie_priv *priv,
				   struct pci_controller *hose)
{
	/* Store the IO and MEM windows settings for future use by the ATU */
	priv->io.phys_start  = hose->regions[0].phys_start; /* IO base */
	priv->io.bus_start   = hose->regions[0].bus_start;  /* IO_bus_addr */
	priv->io.size	     = hose->regions[0].size;	    /* IO size */
	priv->mem.phys_start = hose->regions[1].phys_start; /* MEM base */
	priv->mem.bus_start  = hose->regions[1].bus_start;  /* MEM_bus_addr */
	priv->mem.size	     = hose->regions[1].size;	    /* MEM size */

	/* outbound: IO */
	pcie_dw_prog_outbound_atu(priv, 0,
				  PCIE_ATU_TYPE_IO, priv->io.phys_start,
				  priv->io.bus_start, priv->io.size);

	/* outbound: MEM */
	pcie_dw_prog_outbound_atu(priv, 1,
				  PCIE_ATU_TYPE_MEM, priv->mem.phys_start,
				  priv->mem.bus_start, priv->mem.size);
}

static int uniphier_pcie_probe(struct udevice *dev)
{
	struct uniphier_pcie_priv *priv = dev_get_priv(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	int ret;

	priv->base = map_physmem(priv->link_res.start,
				 fdt_resource_size(&priv->link_res),
				 MAP_NOCACHE);
	priv->dbi_base = map_physmem(priv->dbi_res.start,
				     fdt_resource_size(&priv->dbi_res),
				     MAP_NOCACHE);
	priv->cfg_size = fdt_resource_size(&priv->cfg_res);
	priv->cfg_base = map_physmem(priv->cfg_res.start,
				     priv->cfg_size, MAP_NOCACHE);

	ret = clk_enable(&priv->clk);
	if (ret) {
		dev_err(dev, "Failed to enable clk: %d\n", ret);
		return ret;
	}
	ret = reset_deassert(&priv->rst);
	if (ret) {
		dev_err(dev, "Failed to deassert reset: %d\n", ret);
		goto out_clk_release;
	}

	ret = generic_phy_init(&priv->phy);
	if (ret) {
		dev_err(dev, "Failed to initialize phy: %d\n", ret);
		goto out_reset_release;
	}

	ret = generic_phy_power_on(&priv->phy);
	if (ret) {
		dev_err(dev, "Failed to power on phy: %d\n", ret);
		goto out_phy_exit;
	}

	uniphier_pcie_init_rc(priv);

	/* set DBI to read only */
	writel(0, priv->dbi_base + PCIE_MISC_CONTROL_1_OFF);

	uniphier_pcie_setup_rc(priv, hose);

	if (uniphier_pcie_establish_link(priv)) {
		printf("PCIE-%d: Link down\n", dev_seq(dev));
	} else {
		printf("PCIE-%d: Link up (Gen%d-x%d, Bus%d)\n",
		       dev_seq(dev), pcie_dw_get_link_speed(priv),
		       pcie_dw_get_link_width(priv), hose->first_busno);
	}

	return 0;

out_phy_exit:
	generic_phy_exit(&priv->phy);
out_reset_release:
	reset_release_all(&priv->rst, 1);
out_clk_release:
	clk_release_all(&priv->clk, 1);

	return ret;
}

static int uniphier_pcie_of_to_plat(struct udevice *dev)
{
	struct uniphier_pcie_priv *priv = dev_get_priv(dev);
	const void *fdt = gd->fdt_blob;
	int node = dev_of_offset(dev);
	int ret;

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "link", &priv->link_res);
	if (ret) {
		dev_err(dev, "Failed to get link regs: %d\n", ret);
		return ret;
	}

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "dbi", &priv->dbi_res);
	if (ret) {
		dev_err(dev, "Failed to get dbi regs: %d\n", ret);
		return ret;
	}

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "config", &priv->cfg_res);
	if (ret) {
		dev_err(dev, "Failed to get config regs: %d\n", ret);
		return ret;
	}

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret) {
		dev_err(dev, "Failed to get clocks property: %d\n", ret);
		return ret;
	}

	ret = reset_get_by_index(dev, 0, &priv->rst);
	if (ret) {
		dev_err(dev, "Failed to get resets property: %d\n", ret);
		return ret;
	}

	ret = generic_phy_get_by_index(dev, 0, &priv->phy);
	if (ret) {
		dev_err(dev, "Failed to get phy property: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct dm_pci_ops uniphier_pcie_ops = {
	.read_config	= uniphier_pcie_read_config,
	.write_config	= uniphier_pcie_write_config,
};

static const struct udevice_id uniphier_pcie_ids[] = {
	{ .compatible = "socionext,uniphier-pcie", },
	{ /* Sentinel */ }
};

U_BOOT_DRIVER(pcie_uniphier) = {
	.name     = "uniphier-pcie",
	.id       = UCLASS_PCI,
	.of_match = uniphier_pcie_ids,
	.probe    = uniphier_pcie_probe,
	.ops      = &uniphier_pcie_ops,
	.of_to_plat = uniphier_pcie_of_to_plat,
	.priv_auto = sizeof(struct uniphier_pcie_priv),
};
