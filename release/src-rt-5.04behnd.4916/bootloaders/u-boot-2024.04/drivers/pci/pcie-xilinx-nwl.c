// SPDX-License-Identifier: GPL-2.0
/*
 * PCIe host bridge driver for Xilinx / AMD ZynqMP NWL PCIe Bridge
 *
 * Based on the Linux driver which is:
 * (C) Copyright 2014 - 2015, Xilinx, Inc.
 *
 * Author: Stefan Roese <sr@denx.de>
 */

#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <mapmem.h>
#include <pci.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/ioport.h>

/* Bridge core config registers */
#define BRCFG_PCIE_RX0			0x00000000
#define BRCFG_PCIE_RX1			0x00000004
#define BRCFG_INTERRUPT			0x00000010
#define BRCFG_PCIE_RX_MSG_FILTER	0x00000020

/* Egress - Bridge translation registers */
#define E_BREG_CAPABILITIES		0x00000200
#define E_BREG_CONTROL			0x00000208
#define E_BREG_BASE_LO			0x00000210
#define E_BREG_BASE_HI			0x00000214
#define E_ECAM_CAPABILITIES		0x00000220
#define E_ECAM_CONTROL			0x00000228
#define E_ECAM_BASE_LO			0x00000230
#define E_ECAM_BASE_HI			0x00000234

#define I_ISUB_CONTROL			0x000003E8
#define SET_ISUB_CONTROL		BIT(0)
/* Rxed msg fifo  - Interrupt status registers */
#define MSGF_MISC_STATUS		0x00000400
#define MSGF_MISC_MASK			0x00000404
#define MSGF_LEG_STATUS			0x00000420
#define MSGF_LEG_MASK			0x00000424
#define MSGF_MSI_STATUS_LO		0x00000440
#define MSGF_MSI_STATUS_HI		0x00000444
#define MSGF_MSI_MASK_LO		0x00000448
#define MSGF_MSI_MASK_HI		0x0000044C

/* Msg filter mask bits */
#define CFG_ENABLE_PM_MSG_FWD		BIT(1)
#define CFG_ENABLE_INT_MSG_FWD		BIT(2)
#define CFG_ENABLE_ERR_MSG_FWD		BIT(3)
#define CFG_ENABLE_MSG_FILTER_MASK	(CFG_ENABLE_PM_MSG_FWD |	\
					 CFG_ENABLE_INT_MSG_FWD |	\
					 CFG_ENABLE_ERR_MSG_FWD)

/* Misc interrupt status mask bits */
#define MSGF_MISC_SR_RXMSG_AVAIL	BIT(0)
#define MSGF_MISC_SR_RXMSG_OVER		BIT(1)
#define MSGF_MISC_SR_SLAVE_ERR		BIT(4)
#define MSGF_MISC_SR_MASTER_ERR		BIT(5)
#define MSGF_MISC_SR_I_ADDR_ERR		BIT(6)
#define MSGF_MISC_SR_E_ADDR_ERR		BIT(7)
#define MSGF_MISC_SR_FATAL_AER		BIT(16)
#define MSGF_MISC_SR_NON_FATAL_AER	BIT(17)
#define MSGF_MISC_SR_CORR_AER		BIT(18)
#define MSGF_MISC_SR_UR_DETECT		BIT(20)
#define MSGF_MISC_SR_NON_FATAL_DEV	BIT(22)
#define MSGF_MISC_SR_FATAL_DEV		BIT(23)
#define MSGF_MISC_SR_LINK_DOWN		BIT(24)
#define MSGF_MSIC_SR_LINK_AUTO_BWIDTH	BIT(25)
#define MSGF_MSIC_SR_LINK_BWIDTH	BIT(26)

#define MSGF_MISC_SR_MASKALL		(MSGF_MISC_SR_RXMSG_AVAIL |	\
					 MSGF_MISC_SR_RXMSG_OVER |	\
					 MSGF_MISC_SR_SLAVE_ERR |	\
					 MSGF_MISC_SR_MASTER_ERR |	\
					 MSGF_MISC_SR_I_ADDR_ERR |	\
					 MSGF_MISC_SR_E_ADDR_ERR |	\
					 MSGF_MISC_SR_FATAL_AER |	\
					 MSGF_MISC_SR_NON_FATAL_AER |	\
					 MSGF_MISC_SR_CORR_AER |	\
					 MSGF_MISC_SR_UR_DETECT |	\
					 MSGF_MISC_SR_NON_FATAL_DEV |	\
					 MSGF_MISC_SR_FATAL_DEV |	\
					 MSGF_MISC_SR_LINK_DOWN |	\
					 MSGF_MSIC_SR_LINK_AUTO_BWIDTH | \
					 MSGF_MSIC_SR_LINK_BWIDTH)

/* Legacy interrupt status mask bits */
#define MSGF_LEG_SR_INTA		BIT(0)
#define MSGF_LEG_SR_INTB		BIT(1)
#define MSGF_LEG_SR_INTC		BIT(2)
#define MSGF_LEG_SR_INTD		BIT(3)
#define MSGF_LEG_SR_MASKALL		(MSGF_LEG_SR_INTA | MSGF_LEG_SR_INTB | \
					 MSGF_LEG_SR_INTC | MSGF_LEG_SR_INTD)

/* MSI interrupt status mask bits */
#define MSGF_MSI_SR_LO_MASK		GENMASK(31, 0)
#define MSGF_MSI_SR_HI_MASK		GENMASK(31, 0)

/* Bridge config interrupt mask */
#define BRCFG_INTERRUPT_MASK		BIT(0)
#define BREG_PRESENT			BIT(0)
#define BREG_ENABLE			BIT(0)
#define BREG_ENABLE_FORCE		BIT(1)

/* E_ECAM status mask bits */
#define E_ECAM_PRESENT			BIT(0)
#define E_ECAM_CR_ENABLE		BIT(0)
#define E_ECAM_SIZE_LOC			GENMASK(20, 16)
#define E_ECAM_SIZE_SHIFT		16
#define NWL_ECAM_VALUE_DEFAULT		12

#define CFG_DMA_REG_BAR			GENMASK(2, 0)
#define CFG_PCIE_CACHE			GENMASK(7, 0)

/* Readin the PS_LINKUP */
#define PS_LINKUP_OFFSET		0x00000238
#define PCIE_PHY_LINKUP_BIT		BIT(0)
#define PHY_RDY_LINKUP_BIT		BIT(1)

/* Parameters for the waiting for link up routine */
#define LINK_WAIT_MAX_RETRIES          10
#define LINK_WAIT_USLEEP_MIN           90000
#define LINK_WAIT_USLEEP_MAX           100000

struct nwl_pcie {
	struct udevice *dev;
	void __iomem *breg_base;
	void __iomem *pcireg_base;
	void __iomem *ecam_base;
	phys_addr_t phys_breg_base;	/* Physical Bridge Register Base */
	phys_addr_t phys_ecam_base;	/* Physical Configuration Base */
	u32 ecam_value;
};

static int nwl_pcie_config_address(const struct udevice *bus,
				   pci_dev_t bdf, uint offset,
				   void **paddress)
{
	struct nwl_pcie *pcie = dev_get_priv(bus);
	void *addr;

	addr = pcie->ecam_base;
	addr += PCIE_ECAM_OFFSET(PCI_BUS(bdf) - dev_seq(bus),
				 PCI_DEV(bdf), PCI_FUNC(bdf), offset);
	*paddress = addr;

	return 0;
}

static int nwl_pcie_read_config(const struct udevice *bus, pci_dev_t bdf,
				uint offset, ulong *valuep,
				enum pci_size_t size)
{
	return pci_generic_mmap_read_config(bus, nwl_pcie_config_address,
					    bdf, offset, valuep, size);
}

static int nwl_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				 uint offset, ulong value,
				 enum pci_size_t size)
{
	return pci_generic_mmap_write_config(bus, nwl_pcie_config_address,
					     bdf, offset, value, size);
}

static const struct dm_pci_ops nwl_pcie_ops = {
	.read_config = nwl_pcie_read_config,
	.write_config = nwl_pcie_write_config,
};

static inline u32 nwl_bridge_readl(struct nwl_pcie *pcie, u32 off)
{
	return readl(pcie->breg_base + off);
}

static inline void nwl_bridge_writel(struct nwl_pcie *pcie, u32 val, u32 off)
{
	writel(val, pcie->breg_base + off);
}

static bool nwl_pcie_link_up(struct nwl_pcie *pcie)
{
	if (readl(pcie->pcireg_base + PS_LINKUP_OFFSET) & PCIE_PHY_LINKUP_BIT)
		return true;
	return false;
}

static bool nwl_phy_link_up(struct nwl_pcie *pcie)
{
	if (readl(pcie->pcireg_base + PS_LINKUP_OFFSET) & PHY_RDY_LINKUP_BIT)
		return true;
	return false;
}

static int nwl_wait_for_link(struct nwl_pcie *pcie)
{
	struct udevice *dev = pcie->dev;
	int retries;

	/* check if the link is up or not */
	for (retries = 0; retries < LINK_WAIT_MAX_RETRIES; retries++) {
		if (nwl_phy_link_up(pcie))
			return 0;
		udelay(LINK_WAIT_USLEEP_MIN);
	}

	dev_warn(dev, "PHY link never came up\n");
	return -ETIMEDOUT;
}

static int nwl_pcie_bridge_init(struct nwl_pcie *pcie)
{
	struct udevice *dev = pcie->dev;
	u32 breg_val, ecam_val;
	int err;

	breg_val = nwl_bridge_readl(pcie, E_BREG_CAPABILITIES) & BREG_PRESENT;
	if (!breg_val) {
		dev_err(dev, "BREG is not present\n");
		return breg_val;
	}

	/* Write bridge_off to breg base */
	nwl_bridge_writel(pcie, lower_32_bits(pcie->phys_breg_base),
			  E_BREG_BASE_LO);
	nwl_bridge_writel(pcie, upper_32_bits(pcie->phys_breg_base),
			  E_BREG_BASE_HI);

	/* Enable BREG */
	nwl_bridge_writel(pcie, ~BREG_ENABLE_FORCE & BREG_ENABLE,
			  E_BREG_CONTROL);

	/* Disable DMA channel registers */
	nwl_bridge_writel(pcie, nwl_bridge_readl(pcie, BRCFG_PCIE_RX0) |
			  CFG_DMA_REG_BAR, BRCFG_PCIE_RX0);

	/* Enable Ingress subtractive decode translation */
	nwl_bridge_writel(pcie, SET_ISUB_CONTROL, I_ISUB_CONTROL);

	/* Enable msg filtering details */
	nwl_bridge_writel(pcie, CFG_ENABLE_MSG_FILTER_MASK,
			  BRCFG_PCIE_RX_MSG_FILTER);

	err = nwl_wait_for_link(pcie);
	if (err)
		return err;

	ecam_val = nwl_bridge_readl(pcie, E_ECAM_CAPABILITIES) & E_ECAM_PRESENT;
	if (!ecam_val) {
		dev_err(dev, "ECAM is not present\n");
		return ecam_val;
	}

	/* Enable ECAM */
	nwl_bridge_writel(pcie, nwl_bridge_readl(pcie, E_ECAM_CONTROL) |
			  E_ECAM_CR_ENABLE, E_ECAM_CONTROL);

	nwl_bridge_writel(pcie, nwl_bridge_readl(pcie, E_ECAM_CONTROL) |
			  (pcie->ecam_value << E_ECAM_SIZE_SHIFT),
			  E_ECAM_CONTROL);

	nwl_bridge_writel(pcie, lower_32_bits(pcie->phys_ecam_base),
			  E_ECAM_BASE_LO);
	nwl_bridge_writel(pcie, upper_32_bits(pcie->phys_ecam_base),
			  E_ECAM_BASE_HI);

	if (nwl_pcie_link_up(pcie))
		dev_info(dev, "Link is UP\n");
	else
		dev_info(dev, "Link is DOWN\n");

	/* Disable all misc interrupts */
	nwl_bridge_writel(pcie, (u32)~MSGF_MISC_SR_MASKALL, MSGF_MISC_MASK);

	/* Clear pending misc interrupts */
	nwl_bridge_writel(pcie, nwl_bridge_readl(pcie, MSGF_MISC_STATUS) &
			  MSGF_MISC_SR_MASKALL, MSGF_MISC_STATUS);

	/* Disable all legacy interrupts */
	nwl_bridge_writel(pcie, (u32)~MSGF_LEG_SR_MASKALL, MSGF_LEG_MASK);

	/* Clear pending legacy interrupts */
	nwl_bridge_writel(pcie, nwl_bridge_readl(pcie, MSGF_LEG_STATUS) &
			  MSGF_LEG_SR_MASKALL, MSGF_LEG_STATUS);

	return 0;
}

static int nwl_pcie_parse_dt(struct nwl_pcie *pcie)
{
	struct udevice *dev = pcie->dev;
	struct resource res;
	int ret;

	ret = dev_read_resource_byname(dev, "breg", &res);
	if (ret)
		return ret;
	pcie->breg_base = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(pcie->breg_base))
		return PTR_ERR(pcie->breg_base);
	pcie->phys_breg_base = res.start;

	ret = dev_read_resource_byname(dev, "cfg", &res);
	if (ret)
		return ret;
	pcie->ecam_base = devm_ioremap(dev, res.start, resource_size(&res));
	if (IS_ERR(pcie->ecam_base))
		return PTR_ERR(pcie->ecam_base);
	pcie->phys_ecam_base = res.start;

	return 0;
}

static int nwl_pcie_probe(struct udevice *dev)
{
	struct nwl_pcie *pcie = dev_get_priv(dev);
	int err;

	pcie->dev = dev;
	pcie->ecam_value = NWL_ECAM_VALUE_DEFAULT;

	err = nwl_pcie_parse_dt(pcie);
	if (err) {
		dev_err(dev, "Parsing DT failed\n");
		return err;
	}

	err = nwl_pcie_bridge_init(pcie);
	if (err) {
		dev_err(dev, "HW Initialization failed\n");
		return err;
	}

	return 0;
}

static const struct udevice_id nwl_pcie_of_match[] = {
	{ .compatible = "xlnx,nwl-pcie-2.11", },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(nwl_pcie) = {
	.name = "nwl-pcie",
	.id = UCLASS_PCI,
	.of_match = nwl_pcie_of_match,
	.probe = nwl_pcie_probe,
	.priv_auto = sizeof(struct nwl_pcie),
	.ops = &nwl_pcie_ops,
};
