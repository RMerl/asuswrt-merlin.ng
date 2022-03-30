// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010, CompuLab, Ltd.
 * Author: Mike Rapoport <mike@compulab.co.il>
 *
 * Based on NVIDIA PCIe driver
 * Copyright (c) 2008-2009, NVIDIA Corporation.
 *
 * Copyright (c) 2013-2014, NVIDIA Corporation.
 */

#define pr_fmt(fmt) "tegra-pcie: " fmt

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <pci.h>
#include <pci_tegra.h>
#include <power-domain.h>
#include <reset.h>

#include <asm/io.h>
#include <asm/gpio.h>

#include <linux/ioport.h>
#include <linux/list.h>

#ifndef CONFIG_TEGRA186
#include <asm/arch/clock.h>
#include <asm/arch/powergate.h>
#include <asm/arch-tegra/xusb-padctl.h>
#include <dt-bindings/pinctrl/pinctrl-tegra-xusb.h>
#endif

/*
 * FIXME: TODO: This driver contains a number of ifdef CONFIG_TEGRA186 that
 * should not be present. These are needed because newer Tegra SoCs support
 * only the standard clock/reset APIs, whereas older Tegra SoCs support only
 * a custom Tegra-specific API. ASAP the older Tegra SoCs' code should be
 * fixed to implement the standard APIs, and all drivers converted to solely
 * use the new standard APIs, with no ifdefs.
 */

#define AFI_AXI_BAR0_SZ	0x00
#define AFI_AXI_BAR1_SZ	0x04
#define AFI_AXI_BAR2_SZ	0x08
#define AFI_AXI_BAR3_SZ	0x0c
#define AFI_AXI_BAR4_SZ	0x10
#define AFI_AXI_BAR5_SZ	0x14

#define AFI_AXI_BAR0_START	0x18
#define AFI_AXI_BAR1_START	0x1c
#define AFI_AXI_BAR2_START	0x20
#define AFI_AXI_BAR3_START	0x24
#define AFI_AXI_BAR4_START	0x28
#define AFI_AXI_BAR5_START	0x2c

#define AFI_FPCI_BAR0	0x30
#define AFI_FPCI_BAR1	0x34
#define AFI_FPCI_BAR2	0x38
#define AFI_FPCI_BAR3	0x3c
#define AFI_FPCI_BAR4	0x40
#define AFI_FPCI_BAR5	0x44

#define AFI_CACHE_BAR0_SZ	0x48
#define AFI_CACHE_BAR0_ST	0x4c
#define AFI_CACHE_BAR1_SZ	0x50
#define AFI_CACHE_BAR1_ST	0x54

#define AFI_MSI_BAR_SZ		0x60
#define AFI_MSI_FPCI_BAR_ST	0x64
#define AFI_MSI_AXI_BAR_ST	0x68

#define AFI_CONFIGURATION		0xac
#define  AFI_CONFIGURATION_EN_FPCI	(1 << 0)

#define AFI_FPCI_ERROR_MASKS	0xb0

#define AFI_INTR_MASK		0xb4
#define  AFI_INTR_MASK_INT_MASK	(1 << 0)
#define  AFI_INTR_MASK_MSI_MASK	(1 << 8)

#define AFI_SM_INTR_ENABLE	0xc4
#define  AFI_SM_INTR_INTA_ASSERT	(1 << 0)
#define  AFI_SM_INTR_INTB_ASSERT	(1 << 1)
#define  AFI_SM_INTR_INTC_ASSERT	(1 << 2)
#define  AFI_SM_INTR_INTD_ASSERT	(1 << 3)
#define  AFI_SM_INTR_INTA_DEASSERT	(1 << 4)
#define  AFI_SM_INTR_INTB_DEASSERT	(1 << 5)
#define  AFI_SM_INTR_INTC_DEASSERT	(1 << 6)
#define  AFI_SM_INTR_INTD_DEASSERT	(1 << 7)

#define AFI_AFI_INTR_ENABLE		0xc8
#define  AFI_INTR_EN_INI_SLVERR		(1 << 0)
#define  AFI_INTR_EN_INI_DECERR		(1 << 1)
#define  AFI_INTR_EN_TGT_SLVERR		(1 << 2)
#define  AFI_INTR_EN_TGT_DECERR		(1 << 3)
#define  AFI_INTR_EN_TGT_WRERR		(1 << 4)
#define  AFI_INTR_EN_DFPCI_DECERR	(1 << 5)
#define  AFI_INTR_EN_AXI_DECERR		(1 << 6)
#define  AFI_INTR_EN_FPCI_TIMEOUT	(1 << 7)
#define  AFI_INTR_EN_PRSNT_SENSE	(1 << 8)

#define AFI_PCIE_CONFIG					0x0f8
#define  AFI_PCIE_CONFIG_PCIE_DISABLE(x)		(1 << ((x) + 1))
#define  AFI_PCIE_CONFIG_PCIE_DISABLE_ALL		0xe
#define  AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_MASK	(0xf << 20)
#define  AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_SINGLE	(0x0 << 20)
#define  AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_420	(0x0 << 20)
#define  AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_X2_X1	(0x0 << 20)
#define  AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_DUAL	(0x1 << 20)
#define  AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_222	(0x1 << 20)
#define  AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_X4_X1	(0x1 << 20)
#define  AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_411	(0x2 << 20)
#define  AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_T186_401	(0x0 << 20)
#define  AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_T186_211	(0x1 << 20)
#define  AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_T186_111	(0x2 << 20)

#define AFI_FUSE			0x104
#define  AFI_FUSE_PCIE_T0_GEN2_DIS	(1 << 2)

#define AFI_PEX0_CTRL			0x110
#define AFI_PEX1_CTRL			0x118
#define AFI_PEX2_CTRL			0x128
#define AFI_PEX2_CTRL_T186		0x19c
#define  AFI_PEX_CTRL_RST		(1 << 0)
#define  AFI_PEX_CTRL_CLKREQ_EN		(1 << 1)
#define  AFI_PEX_CTRL_REFCLK_EN		(1 << 3)
#define  AFI_PEX_CTRL_OVERRIDE_EN	(1 << 4)

#define AFI_PLLE_CONTROL		0x160
#define  AFI_PLLE_CONTROL_BYPASS_PADS2PLLE_CONTROL (1 << 9)
#define  AFI_PLLE_CONTROL_PADS2PLLE_CONTROL_EN (1 << 1)

#define AFI_PEXBIAS_CTRL_0		0x168

#define PADS_CTL_SEL		0x0000009C

#define PADS_CTL		0x000000A0
#define  PADS_CTL_IDDQ_1L	(1 <<  0)
#define  PADS_CTL_TX_DATA_EN_1L	(1 <<  6)
#define  PADS_CTL_RX_DATA_EN_1L	(1 << 10)

#define PADS_PLL_CTL_TEGRA20			0x000000B8
#define PADS_PLL_CTL_TEGRA30			0x000000B4
#define  PADS_PLL_CTL_RST_B4SM			(0x1 <<  1)
#define  PADS_PLL_CTL_LOCKDET			(0x1 <<  8)
#define  PADS_PLL_CTL_REFCLK_MASK		(0x3 << 16)
#define  PADS_PLL_CTL_REFCLK_INTERNAL_CML	(0x0 << 16)
#define  PADS_PLL_CTL_REFCLK_INTERNAL_CMOS	(0x1 << 16)
#define  PADS_PLL_CTL_REFCLK_EXTERNAL		(0x2 << 16)
#define  PADS_PLL_CTL_TXCLKREF_MASK		(0x1 << 20)
#define  PADS_PLL_CTL_TXCLKREF_DIV10		(0x0 << 20)
#define  PADS_PLL_CTL_TXCLKREF_DIV5		(0x1 << 20)
#define  PADS_PLL_CTL_TXCLKREF_BUF_EN		(0x1 << 22)

#define PADS_REFCLK_CFG0			0x000000C8
#define PADS_REFCLK_CFG1			0x000000CC

/*
 * Fields in PADS_REFCLK_CFG*. Those registers form an array of 16-bit
 * entries, one entry per PCIe port. These field definitions and desired
 * values aren't in the TRM, but do come from NVIDIA.
 */
#define PADS_REFCLK_CFG_TERM_SHIFT		2  /* 6:2 */
#define PADS_REFCLK_CFG_E_TERM_SHIFT		7
#define PADS_REFCLK_CFG_PREDI_SHIFT		8  /* 11:8 */
#define PADS_REFCLK_CFG_DRVI_SHIFT		12 /* 15:12 */

#define RP_VEND_XP	0x00000F00
#define  RP_VEND_XP_DL_UP	(1 << 30)

#define RP_VEND_CTL2				0x00000FA8
#define  RP_VEND_CTL2_PCA_ENABLE		(1 << 7)

#define RP_PRIV_MISC	0x00000FE0
#define  RP_PRIV_MISC_PRSNT_MAP_EP_PRSNT (0xE << 0)
#define  RP_PRIV_MISC_PRSNT_MAP_EP_ABSNT (0xF << 0)

#define RP_LINK_CONTROL_STATUS			0x00000090
#define  RP_LINK_CONTROL_STATUS_DL_LINK_ACTIVE	0x20000000
#define  RP_LINK_CONTROL_STATUS_LINKSTAT_MASK	0x3fff0000

enum tegra_pci_id {
	TEGRA20_PCIE,
	TEGRA30_PCIE,
	TEGRA124_PCIE,
	TEGRA210_PCIE,
	TEGRA186_PCIE,
};

struct tegra_pcie_port {
	struct tegra_pcie *pcie;

	struct fdt_resource regs;
	unsigned int num_lanes;
	unsigned int index;

	struct list_head list;
};

struct tegra_pcie_soc {
	unsigned int num_ports;
	unsigned long pads_pll_ctl;
	unsigned long tx_ref_sel;
	unsigned long afi_pex2_ctrl;
	u32 pads_refclk_cfg0;
	u32 pads_refclk_cfg1;
	bool has_pex_clkreq_en;
	bool has_pex_bias_ctrl;
	bool has_cml_clk;
	bool has_gen2;
	bool force_pca_enable;
};

struct tegra_pcie {
	struct resource pads;
	struct resource afi;
	struct resource cs;

	struct list_head ports;
	unsigned long xbar;

	const struct tegra_pcie_soc *soc;

#ifdef CONFIG_TEGRA186
	struct clk clk_afi;
	struct clk clk_pex;
	struct reset_ctl reset_afi;
	struct reset_ctl reset_pex;
	struct reset_ctl reset_pcie_x;
	struct power_domain pwrdom;
#else
	struct tegra_xusb_phy *phy;
#endif
};

static void afi_writel(struct tegra_pcie *pcie, unsigned long value,
		       unsigned long offset)
{
	writel(value, pcie->afi.start + offset);
}

static unsigned long afi_readl(struct tegra_pcie *pcie, unsigned long offset)
{
	return readl(pcie->afi.start + offset);
}

static void pads_writel(struct tegra_pcie *pcie, unsigned long value,
			unsigned long offset)
{
	writel(value, pcie->pads.start + offset);
}

#ifndef CONFIG_TEGRA186
static unsigned long pads_readl(struct tegra_pcie *pcie, unsigned long offset)
{
	return readl(pcie->pads.start + offset);
}
#endif

static unsigned long rp_readl(struct tegra_pcie_port *port,
			      unsigned long offset)
{
	return readl(port->regs.start + offset);
}

static void rp_writel(struct tegra_pcie_port *port, unsigned long value,
		      unsigned long offset)
{
	writel(value, port->regs.start + offset);
}

static unsigned long tegra_pcie_conf_offset(pci_dev_t bdf, int where)
{
	return ((where & 0xf00) << 16) | (PCI_BUS(bdf) << 16) |
	       (PCI_DEV(bdf) << 11) | (PCI_FUNC(bdf) << 8) |
	       (where & 0xfc);
}

static int tegra_pcie_conf_address(struct tegra_pcie *pcie, pci_dev_t bdf,
				   int where, unsigned long *address)
{
	unsigned int bus = PCI_BUS(bdf);

	if (bus == 0) {
		unsigned int dev = PCI_DEV(bdf);
		struct tegra_pcie_port *port;

		list_for_each_entry(port, &pcie->ports, list) {
			if (port->index + 1 == dev) {
				*address = port->regs.start + (where & ~3);
				return 0;
			}
		}
		return -EFAULT;
	} else {
#ifdef CONFIG_TEGRA20
		unsigned int dev = PCI_DEV(bdf);
		if (dev != 0)
			return -EFAULT;
#endif

		*address = pcie->cs.start + tegra_pcie_conf_offset(bdf, where);
		return 0;
	}
}

static int pci_tegra_read_config(struct udevice *bus, pci_dev_t bdf,
				 uint offset, ulong *valuep,
				 enum pci_size_t size)
{
	struct tegra_pcie *pcie = dev_get_priv(bus);
	unsigned long address, value;
	int err;

	err = tegra_pcie_conf_address(pcie, bdf, offset, &address);
	if (err < 0) {
		value = 0xffffffff;
		goto done;
	}

	value = readl(address);

#ifdef CONFIG_TEGRA20
	/* fixup root port class */
	if (PCI_BUS(bdf) == 0) {
		if ((offset & ~3) == PCI_CLASS_REVISION) {
			value &= ~0x00ff0000;
			value |= PCI_CLASS_BRIDGE_PCI << 16;
		}
	}
#endif

done:
	*valuep = pci_conv_32_to_size(value, offset, size);

	return 0;
}

static int pci_tegra_write_config(struct udevice *bus, pci_dev_t bdf,
				  uint offset, ulong value,
				  enum pci_size_t size)
{
	struct tegra_pcie *pcie = dev_get_priv(bus);
	unsigned long address;
	ulong old;
	int err;

	err = tegra_pcie_conf_address(pcie, bdf, offset, &address);
	if (err < 0)
		return 0;

	old = readl(address);
	value = pci_conv_size_to_32(old, value, offset, size);
	writel(value, address);

	return 0;
}

static int tegra_pcie_port_parse_dt(ofnode node, struct tegra_pcie_port *port)
{
	const u32 *addr;
	int len;

	addr = ofnode_get_property(node, "assigned-addresses", &len);
	if (!addr) {
		pr_err("property \"assigned-addresses\" not found");
		return -FDT_ERR_NOTFOUND;
	}

	port->regs.start = fdt32_to_cpu(addr[2]);
	port->regs.end = port->regs.start + fdt32_to_cpu(addr[4]);

	return 0;
}

static int tegra_pcie_get_xbar_config(ofnode node, u32 lanes,
				      enum tegra_pci_id id, unsigned long *xbar)
{
	switch (id) {
	case TEGRA20_PCIE:
		switch (lanes) {
		case 0x00000004:
			debug("single-mode configuration\n");
			*xbar = AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_SINGLE;
			return 0;

		case 0x00000202:
			debug("dual-mode configuration\n");
			*xbar = AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_DUAL;
			return 0;
		}
		break;
	case TEGRA30_PCIE:
		switch (lanes) {
		case 0x00000204:
			debug("4x1, 2x1 configuration\n");
			*xbar = AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_420;
			return 0;

		case 0x00020202:
			debug("2x3 configuration\n");
			*xbar = AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_222;
			return 0;

		case 0x00010104:
			debug("4x1, 1x2 configuration\n");
			*xbar = AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_411;
			return 0;
		}
		break;
	case TEGRA124_PCIE:
	case TEGRA210_PCIE:
		switch (lanes) {
		case 0x0000104:
			debug("4x1, 1x1 configuration\n");
			*xbar = AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_X4_X1;
			return 0;

		case 0x0000102:
			debug("2x1, 1x1 configuration\n");
			*xbar = AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_X2_X1;
			return 0;
		}
		break;
	case TEGRA186_PCIE:
		switch (lanes) {
		case 0x0010004:
			debug("x4 x1 configuration\n");
			*xbar = AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_T186_401;
			return 0;

		case 0x0010102:
			debug("x2 x1 x1 configuration\n");
			*xbar = AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_T186_211;
			return 0;

		case 0x0010101:
			debug("x1 x1 x1 configuration\n");
			*xbar = AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_T186_111;
			return 0;
		}
		break;
	default:
		break;
	}

	return -FDT_ERR_NOTFOUND;
}

static int tegra_pcie_parse_port_info(ofnode node, uint *index, uint *lanes)
{
	struct fdt_pci_addr addr;
	int err;

	err = ofnode_read_u32_default(node, "nvidia,num-lanes", -1);
	if (err < 0) {
		pr_err("failed to parse \"nvidia,num-lanes\" property");
		return err;
	}

	*lanes = err;

	err = ofnode_read_pci_addr(node, 0, "reg", &addr);
	if (err < 0) {
		pr_err("failed to parse \"reg\" property");
		return err;
	}

	*index = PCI_DEV(addr.phys_hi) - 1;

	return 0;
}

int __weak tegra_pcie_board_init(void)
{
	return 0;
}

static int tegra_pcie_parse_dt(struct udevice *dev, enum tegra_pci_id id,
			       struct tegra_pcie *pcie)
{
	ofnode subnode;
	u32 lanes = 0;
	int err;

	err = dev_read_resource(dev, 0, &pcie->pads);
	if (err < 0) {
		pr_err("resource \"pads\" not found");
		return err;
	}

	err = dev_read_resource(dev, 1, &pcie->afi);
	if (err < 0) {
		pr_err("resource \"afi\" not found");
		return err;
	}

	err = dev_read_resource(dev, 2, &pcie->cs);
	if (err < 0) {
		pr_err("resource \"cs\" not found");
		return err;
	}

	err = tegra_pcie_board_init();
	if (err < 0) {
		pr_err("tegra_pcie_board_init() failed: err=%d", err);
		return err;
	}

#ifndef CONFIG_TEGRA186
	pcie->phy = tegra_xusb_phy_get(TEGRA_XUSB_PADCTL_PCIE);
	if (pcie->phy) {
		err = tegra_xusb_phy_prepare(pcie->phy);
		if (err < 0) {
			pr_err("failed to prepare PHY: %d", err);
			return err;
		}
	}
#endif

	dev_for_each_subnode(subnode, dev) {
		unsigned int index = 0, num_lanes = 0;
		struct tegra_pcie_port *port;

		err = tegra_pcie_parse_port_info(subnode, &index, &num_lanes);
		if (err < 0) {
			pr_err("failed to obtain root port info");
			continue;
		}

		lanes |= num_lanes << (index << 3);

		if (!ofnode_is_available(subnode))
			continue;

		port = malloc(sizeof(*port));
		if (!port)
			continue;

		memset(port, 0, sizeof(*port));
		port->num_lanes = num_lanes;
		port->index = index;

		err = tegra_pcie_port_parse_dt(subnode, port);
		if (err < 0) {
			free(port);
			continue;
		}

		list_add_tail(&port->list, &pcie->ports);
		port->pcie = pcie;
	}

	err = tegra_pcie_get_xbar_config(dev_ofnode(dev), lanes, id,
					 &pcie->xbar);
	if (err < 0) {
		pr_err("invalid lane configuration");
		return err;
	}

	return 0;
}

#ifdef CONFIG_TEGRA186
static int tegra_pcie_power_on(struct tegra_pcie *pcie)
{
	int ret;

	ret = power_domain_on(&pcie->pwrdom);
	if (ret) {
		pr_err("power_domain_on() failed: %d\n", ret);
		return ret;
	}

	ret = clk_enable(&pcie->clk_afi);
	if (ret) {
		pr_err("clk_enable(afi) failed: %d\n", ret);
		return ret;
	}

	ret = clk_enable(&pcie->clk_pex);
	if (ret) {
		pr_err("clk_enable(pex) failed: %d\n", ret);
		return ret;
	}

	ret = reset_deassert(&pcie->reset_afi);
	if (ret) {
		pr_err("reset_deassert(afi) failed: %d\n", ret);
		return ret;
	}

	ret = reset_deassert(&pcie->reset_pex);
	if (ret) {
		pr_err("reset_deassert(pex) failed: %d\n", ret);
		return ret;
	}

	return 0;
}
#else
static int tegra_pcie_power_on(struct tegra_pcie *pcie)
{
	const struct tegra_pcie_soc *soc = pcie->soc;
	unsigned long value;
	int err;

	/* reset PCIEXCLK logic, AFI controller and PCIe controller */
	reset_set_enable(PERIPH_ID_PCIEXCLK, 1);
	reset_set_enable(PERIPH_ID_AFI, 1);
	reset_set_enable(PERIPH_ID_PCIE, 1);

	err = tegra_powergate_power_off(TEGRA_POWERGATE_PCIE);
	if (err < 0) {
		pr_err("failed to power off PCIe partition: %d", err);
		return err;
	}

	err = tegra_powergate_sequence_power_up(TEGRA_POWERGATE_PCIE,
						PERIPH_ID_PCIE);
	if (err < 0) {
		pr_err("failed to power up PCIe partition: %d", err);
		return err;
	}

	/* take AFI controller out of reset */
	reset_set_enable(PERIPH_ID_AFI, 0);

	/* enable AFI clock */
	clock_enable(PERIPH_ID_AFI);

	if (soc->has_cml_clk) {
		/* enable CML clock */
		value = readl(NV_PA_CLK_RST_BASE + 0x48c);
		value |= (1 << 0);
		value &= ~(1 << 1);
		writel(value, NV_PA_CLK_RST_BASE + 0x48c);
	}

	err = tegra_plle_enable();
	if (err < 0) {
		pr_err("failed to enable PLLE: %d\n", err);
		return err;
	}

	return 0;
}

static int tegra_pcie_pll_wait(struct tegra_pcie *pcie, unsigned long timeout)
{
	const struct tegra_pcie_soc *soc = pcie->soc;
	unsigned long start = get_timer(0);
	u32 value;

	while (get_timer(start) < timeout) {
		value = pads_readl(pcie, soc->pads_pll_ctl);
		if (value & PADS_PLL_CTL_LOCKDET)
			return 0;
	}

	return -ETIMEDOUT;
}

static int tegra_pcie_phy_enable(struct tegra_pcie *pcie)
{
	const struct tegra_pcie_soc *soc = pcie->soc;
	u32 value;
	int err;

	/* initialize internal PHY, enable up to 16 PCIe lanes */
	pads_writel(pcie, 0, PADS_CTL_SEL);

	/* override IDDQ to 1 on all 4 lanes */
	value = pads_readl(pcie, PADS_CTL);
	value |= PADS_CTL_IDDQ_1L;
	pads_writel(pcie, value, PADS_CTL);

	/*
	 * Set up PHY PLL inputs select PLLE output as refclock, set TX
	 * ref sel to div10 (not div5).
	 */
	value = pads_readl(pcie, soc->pads_pll_ctl);
	value &= ~(PADS_PLL_CTL_REFCLK_MASK | PADS_PLL_CTL_TXCLKREF_MASK);
	value |= PADS_PLL_CTL_REFCLK_INTERNAL_CML | soc->tx_ref_sel;
	pads_writel(pcie, value, soc->pads_pll_ctl);

	/* reset PLL */
	value = pads_readl(pcie, soc->pads_pll_ctl);
	value &= ~PADS_PLL_CTL_RST_B4SM;
	pads_writel(pcie, value, soc->pads_pll_ctl);

	udelay(20);

	/* take PLL out of reset */
	value = pads_readl(pcie, soc->pads_pll_ctl);
	value |= PADS_PLL_CTL_RST_B4SM;
	pads_writel(pcie, value, soc->pads_pll_ctl);

	/* wait for the PLL to lock */
	err = tegra_pcie_pll_wait(pcie, 500);
	if (err < 0) {
		pr_err("PLL failed to lock: %d", err);
		return err;
	}

	/* turn off IDDQ override */
	value = pads_readl(pcie, PADS_CTL);
	value &= ~PADS_CTL_IDDQ_1L;
	pads_writel(pcie, value, PADS_CTL);

	/* enable TX/RX data */
	value = pads_readl(pcie, PADS_CTL);
	value |= PADS_CTL_TX_DATA_EN_1L | PADS_CTL_RX_DATA_EN_1L;
	pads_writel(pcie, value, PADS_CTL);

	return 0;
}
#endif

static int tegra_pcie_enable_controller(struct tegra_pcie *pcie)
{
	const struct tegra_pcie_soc *soc = pcie->soc;
	struct tegra_pcie_port *port;
	u32 value;
	int err;

#ifdef CONFIG_TEGRA186
	{
#else
	if (pcie->phy) {
#endif
		value = afi_readl(pcie, AFI_PLLE_CONTROL);
		value &= ~AFI_PLLE_CONTROL_BYPASS_PADS2PLLE_CONTROL;
		value |= AFI_PLLE_CONTROL_PADS2PLLE_CONTROL_EN;
		afi_writel(pcie, value, AFI_PLLE_CONTROL);
	}

	if (soc->has_pex_bias_ctrl)
		afi_writel(pcie, 0, AFI_PEXBIAS_CTRL_0);

	value = afi_readl(pcie, AFI_PCIE_CONFIG);
	value &= ~AFI_PCIE_CONFIG_SM2TMS0_XBAR_CONFIG_MASK;
	value |= AFI_PCIE_CONFIG_PCIE_DISABLE_ALL | pcie->xbar;

	list_for_each_entry(port, &pcie->ports, list)
		value &= ~AFI_PCIE_CONFIG_PCIE_DISABLE(port->index);

	afi_writel(pcie, value, AFI_PCIE_CONFIG);

	value = afi_readl(pcie, AFI_FUSE);

	if (soc->has_gen2)
		value &= ~AFI_FUSE_PCIE_T0_GEN2_DIS;
	else
		value |= AFI_FUSE_PCIE_T0_GEN2_DIS;

	afi_writel(pcie, value, AFI_FUSE);

#ifndef CONFIG_TEGRA186
	if (pcie->phy)
		err = tegra_xusb_phy_enable(pcie->phy);
	else
		err = tegra_pcie_phy_enable(pcie);

	if (err < 0) {
		pr_err("failed to power on PHY: %d\n", err);
		return err;
	}
#endif

	/* take the PCIEXCLK logic out of reset */
#ifdef CONFIG_TEGRA186
	err = reset_deassert(&pcie->reset_pcie_x);
	if (err) {
		pr_err("reset_deassert(pcie_x) failed: %d\n", err);
		return err;
	}
#else
	reset_set_enable(PERIPH_ID_PCIEXCLK, 0);
#endif

	/* finally enable PCIe */
	value = afi_readl(pcie, AFI_CONFIGURATION);
	value |= AFI_CONFIGURATION_EN_FPCI;
	afi_writel(pcie, value, AFI_CONFIGURATION);

	/* disable all interrupts */
	afi_writel(pcie, 0, AFI_AFI_INTR_ENABLE);
	afi_writel(pcie, 0, AFI_SM_INTR_ENABLE);
	afi_writel(pcie, 0, AFI_INTR_MASK);
	afi_writel(pcie, 0, AFI_FPCI_ERROR_MASKS);

	return 0;
}

static int tegra_pcie_setup_translations(struct udevice *bus)
{
	struct tegra_pcie *pcie = dev_get_priv(bus);
	unsigned long fpci, axi, size;
	struct pci_region *io, *mem, *pref;
	int count;

	/* BAR 0: type 1 extended configuration space */
	fpci = 0xfe100000;
	size = resource_size(&pcie->cs);
	axi = pcie->cs.start;

	afi_writel(pcie, axi, AFI_AXI_BAR0_START);
	afi_writel(pcie, size >> 12, AFI_AXI_BAR0_SZ);
	afi_writel(pcie, fpci, AFI_FPCI_BAR0);

	count = pci_get_regions(bus, &io, &mem, &pref);
	if (count != 3)
		return -EINVAL;

	/* BAR 1: downstream I/O */
	fpci = 0xfdfc0000;
	size = io->size;
	axi = io->phys_start;

	afi_writel(pcie, axi, AFI_AXI_BAR1_START);
	afi_writel(pcie, size >> 12, AFI_AXI_BAR1_SZ);
	afi_writel(pcie, fpci, AFI_FPCI_BAR1);

	/* BAR 2: prefetchable memory */
	fpci = (((pref->phys_start >> 12) & 0x0fffffff) << 4) | 0x1;
	size = pref->size;
	axi = pref->phys_start;

	afi_writel(pcie, axi, AFI_AXI_BAR2_START);
	afi_writel(pcie, size >> 12, AFI_AXI_BAR2_SZ);
	afi_writel(pcie, fpci, AFI_FPCI_BAR2);

	/* BAR 3: non-prefetchable memory */
	fpci = (((mem->phys_start >> 12) & 0x0fffffff) << 4) | 0x1;
	size = mem->size;
	axi = mem->phys_start;

	afi_writel(pcie, axi, AFI_AXI_BAR3_START);
	afi_writel(pcie, size >> 12, AFI_AXI_BAR3_SZ);
	afi_writel(pcie, fpci, AFI_FPCI_BAR3);

	/* NULL out the remaining BARs as they are not used */
	afi_writel(pcie, 0, AFI_AXI_BAR4_START);
	afi_writel(pcie, 0, AFI_AXI_BAR4_SZ);
	afi_writel(pcie, 0, AFI_FPCI_BAR4);

	afi_writel(pcie, 0, AFI_AXI_BAR5_START);
	afi_writel(pcie, 0, AFI_AXI_BAR5_SZ);
	afi_writel(pcie, 0, AFI_FPCI_BAR5);

	/* map all upstream transactions as uncached */
	afi_writel(pcie, NV_PA_SDRAM_BASE, AFI_CACHE_BAR0_ST);
	afi_writel(pcie, 0, AFI_CACHE_BAR0_SZ);
	afi_writel(pcie, 0, AFI_CACHE_BAR1_ST);
	afi_writel(pcie, 0, AFI_CACHE_BAR1_SZ);

	/* MSI translations are setup only when needed */
	afi_writel(pcie, 0, AFI_MSI_FPCI_BAR_ST);
	afi_writel(pcie, 0, AFI_MSI_BAR_SZ);
	afi_writel(pcie, 0, AFI_MSI_AXI_BAR_ST);
	afi_writel(pcie, 0, AFI_MSI_BAR_SZ);

	return 0;
}

static unsigned long tegra_pcie_port_get_pex_ctrl(struct tegra_pcie_port *port)
{
	unsigned long ret = 0;

	switch (port->index) {
	case 0:
		ret = AFI_PEX0_CTRL;
		break;

	case 1:
		ret = AFI_PEX1_CTRL;
		break;

	case 2:
		ret = port->pcie->soc->afi_pex2_ctrl;
		break;
	}

	return ret;
}

void tegra_pcie_port_reset(struct tegra_pcie_port *port)
{
	unsigned long ctrl = tegra_pcie_port_get_pex_ctrl(port);
	unsigned long value;

	/* pulse reset signel */
	value = afi_readl(port->pcie, ctrl);
	value &= ~AFI_PEX_CTRL_RST;
	afi_writel(port->pcie, value, ctrl);

	udelay(2000);

	value = afi_readl(port->pcie, ctrl);
	value |= AFI_PEX_CTRL_RST;
	afi_writel(port->pcie, value, ctrl);
}

int tegra_pcie_port_index_of_port(struct tegra_pcie_port *port)
{
	return port->index;
}

void __weak tegra_pcie_board_port_reset(struct tegra_pcie_port *port)
{
	tegra_pcie_port_reset(port);
}

static void tegra_pcie_port_enable(struct tegra_pcie_port *port)
{
	struct tegra_pcie *pcie = port->pcie;
	const struct tegra_pcie_soc *soc = pcie->soc;
	unsigned long ctrl = tegra_pcie_port_get_pex_ctrl(port);
	unsigned long value;

	/* enable reference clock */
	value = afi_readl(pcie, ctrl);
	value |= AFI_PEX_CTRL_REFCLK_EN;

	if (pcie->soc->has_pex_clkreq_en)
		value |= AFI_PEX_CTRL_CLKREQ_EN;

	value |= AFI_PEX_CTRL_OVERRIDE_EN;

	afi_writel(pcie, value, ctrl);

	tegra_pcie_board_port_reset(port);

	if (soc->force_pca_enable) {
		value = rp_readl(port, RP_VEND_CTL2);
		value |= RP_VEND_CTL2_PCA_ENABLE;
		rp_writel(port, value, RP_VEND_CTL2);
	}

	/* configure the reference clock driver */
	pads_writel(pcie, soc->pads_refclk_cfg0, PADS_REFCLK_CFG0);
	if (soc->num_ports > 2)
		pads_writel(pcie, soc->pads_refclk_cfg1, PADS_REFCLK_CFG1);
}

static bool tegra_pcie_port_check_link(struct tegra_pcie_port *port)
{
	unsigned int retries = 3;
	unsigned long value;

	value = rp_readl(port, RP_PRIV_MISC);
	value &= ~RP_PRIV_MISC_PRSNT_MAP_EP_ABSNT;
	value |= RP_PRIV_MISC_PRSNT_MAP_EP_PRSNT;
	rp_writel(port, value, RP_PRIV_MISC);

	do {
		unsigned int timeout = 200;

		do {
			value = rp_readl(port, RP_VEND_XP);
			if (value & RP_VEND_XP_DL_UP)
				break;

			udelay(2000);
		} while (--timeout);

		if (!timeout) {
			debug("link %u down, retrying\n", port->index);
			goto retry;
		}

		timeout = 200;

		do {
			value = rp_readl(port, RP_LINK_CONTROL_STATUS);
			if (value & RP_LINK_CONTROL_STATUS_DL_LINK_ACTIVE)
				return true;

			udelay(2000);
		} while (--timeout);

retry:
		tegra_pcie_board_port_reset(port);
	} while (--retries);

	return false;
}

static void tegra_pcie_port_disable(struct tegra_pcie_port *port)
{
	unsigned long ctrl = tegra_pcie_port_get_pex_ctrl(port);
	unsigned long value;

	/* assert port reset */
	value = afi_readl(port->pcie, ctrl);
	value &= ~AFI_PEX_CTRL_RST;
	afi_writel(port->pcie, value, ctrl);

	/* disable reference clock */
	value = afi_readl(port->pcie, ctrl);
	value &= ~AFI_PEX_CTRL_REFCLK_EN;
	afi_writel(port->pcie, value, ctrl);
}

static void tegra_pcie_port_free(struct tegra_pcie_port *port)
{
	list_del(&port->list);
	free(port);
}

static int tegra_pcie_enable(struct tegra_pcie *pcie)
{
	struct tegra_pcie_port *port, *tmp;

	list_for_each_entry_safe(port, tmp, &pcie->ports, list) {
		debug("probing port %u, using %u lanes\n", port->index,
		      port->num_lanes);

		tegra_pcie_port_enable(port);

		if (tegra_pcie_port_check_link(port))
			continue;

		debug("link %u down, ignoring\n", port->index);

		tegra_pcie_port_disable(port);
		tegra_pcie_port_free(port);
	}

	return 0;
}

static const struct tegra_pcie_soc pci_tegra_soc[] = {
	[TEGRA20_PCIE] = {
		.num_ports = 2,
		.pads_pll_ctl = PADS_PLL_CTL_TEGRA20,
		.tx_ref_sel = PADS_PLL_CTL_TXCLKREF_DIV10,
		.pads_refclk_cfg0 = 0xfa5cfa5c,
		.has_pex_clkreq_en = false,
		.has_pex_bias_ctrl = false,
		.has_cml_clk = false,
		.has_gen2 = false,
	},
	[TEGRA30_PCIE] = {
		.num_ports = 3,
		.pads_pll_ctl = PADS_PLL_CTL_TEGRA30,
		.tx_ref_sel = PADS_PLL_CTL_TXCLKREF_BUF_EN,
		.afi_pex2_ctrl = AFI_PEX2_CTRL,
		.pads_refclk_cfg0 = 0xfa5cfa5c,
		.pads_refclk_cfg1 = 0xfa5cfa5c,
		.has_pex_clkreq_en = true,
		.has_pex_bias_ctrl = true,
		.has_cml_clk = true,
		.has_gen2 = false,
	},
	[TEGRA124_PCIE] = {
		.num_ports = 2,
		.pads_pll_ctl = PADS_PLL_CTL_TEGRA30,
		.tx_ref_sel = PADS_PLL_CTL_TXCLKREF_BUF_EN,
		.pads_refclk_cfg0 = 0x44ac44ac,
		.has_pex_clkreq_en = true,
		.has_pex_bias_ctrl = true,
		.has_cml_clk = true,
		.has_gen2 = true,
	},
	[TEGRA210_PCIE] = {
		.num_ports = 2,
		.pads_pll_ctl = PADS_PLL_CTL_TEGRA30,
		.tx_ref_sel = PADS_PLL_CTL_TXCLKREF_BUF_EN,
		.pads_refclk_cfg0 = 0x90b890b8,
		.has_pex_clkreq_en = true,
		.has_pex_bias_ctrl = true,
		.has_cml_clk = true,
		.has_gen2 = true,
		.force_pca_enable = true,
	},
	[TEGRA186_PCIE] = {
		.num_ports = 3,
		.afi_pex2_ctrl = AFI_PEX2_CTRL_T186,
		.pads_refclk_cfg0 = 0x80b880b8,
		.pads_refclk_cfg1 = 0x000480b8,
		.has_pex_clkreq_en = true,
		.has_pex_bias_ctrl = true,
		.has_gen2 = true,
	},
};

static int pci_tegra_ofdata_to_platdata(struct udevice *dev)
{
	struct tegra_pcie *pcie = dev_get_priv(dev);
	enum tegra_pci_id id;

	id = dev_get_driver_data(dev);
	pcie->soc = &pci_tegra_soc[id];

	INIT_LIST_HEAD(&pcie->ports);

	if (tegra_pcie_parse_dt(dev, id, pcie))
		return -EINVAL;

	return 0;
}

static int pci_tegra_probe(struct udevice *dev)
{
	struct tegra_pcie *pcie = dev_get_priv(dev);
	int err;

#ifdef CONFIG_TEGRA186
	err = clk_get_by_name(dev, "afi", &pcie->clk_afi);
	if (err) {
		debug("clk_get_by_name(afi) failed: %d\n", err);
		return err;
	}

	err = clk_get_by_name(dev, "pex", &pcie->clk_pex);
	if (err) {
		debug("clk_get_by_name(pex) failed: %d\n", err);
		return err;
	}

	err = reset_get_by_name(dev, "afi", &pcie->reset_afi);
	if (err) {
		debug("reset_get_by_name(afi) failed: %d\n", err);
		return err;
	}

	err = reset_get_by_name(dev, "pex", &pcie->reset_pex);
	if (err) {
		debug("reset_get_by_name(pex) failed: %d\n", err);
		return err;
	}

	err = reset_get_by_name(dev, "pcie_x", &pcie->reset_pcie_x);
	if (err) {
		debug("reset_get_by_name(pcie_x) failed: %d\n", err);
		return err;
	}

	err = power_domain_get(dev, &pcie->pwrdom);
	if (err) {
		debug("power_domain_get() failed: %d\n", err);
		return err;
	}
#endif

	err = tegra_pcie_power_on(pcie);
	if (err < 0) {
		pr_err("failed to power on");
		return err;
	}

	err = tegra_pcie_enable_controller(pcie);
	if (err < 0) {
		pr_err("failed to enable controller");
		return err;
	}

	err = tegra_pcie_setup_translations(dev);
	if (err < 0) {
		pr_err("failed to decode ranges");
		return err;
	}

	err = tegra_pcie_enable(pcie);
	if (err < 0) {
		pr_err("failed to enable PCIe");
		return err;
	}

	return 0;
}

static const struct dm_pci_ops pci_tegra_ops = {
	.read_config	= pci_tegra_read_config,
	.write_config	= pci_tegra_write_config,
};

static const struct udevice_id pci_tegra_ids[] = {
	{ .compatible = "nvidia,tegra20-pcie", .data = TEGRA20_PCIE },
	{ .compatible = "nvidia,tegra30-pcie", .data = TEGRA30_PCIE },
	{ .compatible = "nvidia,tegra124-pcie", .data = TEGRA124_PCIE },
	{ .compatible = "nvidia,tegra210-pcie", .data = TEGRA210_PCIE },
	{ .compatible = "nvidia,tegra186-pcie", .data = TEGRA186_PCIE },
	{ }
};

U_BOOT_DRIVER(pci_tegra) = {
	.name	= "pci_tegra",
	.id	= UCLASS_PCI,
	.of_match = pci_tegra_ids,
	.ops	= &pci_tegra_ops,
	.ofdata_to_platdata = pci_tegra_ofdata_to_platdata,
	.probe	= pci_tegra_probe,
	.priv_auto_alloc_size = sizeof(struct tegra_pcie),
};
