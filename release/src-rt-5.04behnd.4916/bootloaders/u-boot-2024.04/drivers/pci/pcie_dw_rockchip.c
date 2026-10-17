// SPDX-License-Identifier: GPL-2.0+
/*
 * Rockchip DesignWare based PCIe host controller driver
 *
 * Copyright (c) 2021 Rockchip, Inc.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <generic-phy.h>
#include <pci.h>
#include <power-domain.h>
#include <reset.h>
#include <syscon.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm-generic/gpio.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/iopoll.h>
#include <linux/delay.h>
#include <power/regulator.h>

#include "pcie_dw_common.h"

DECLARE_GLOBAL_DATA_PTR;

/**
 * struct rk_pcie - RK DW PCIe controller state
 *
 * @vpcie3v3: The 3.3v power supply for slot
 * @apb_base: The base address of vendor regs
 * @rst_gpio: The #PERST signal for slot
 */
struct rk_pcie {
	/* Must be first member of the struct */
	struct pcie_dw  dw;
	struct udevice  *vpcie3v3;
	void		*apb_base;
	struct phy	phy;
	struct clk_bulk	clks;
	struct reset_ctl_bulk	rsts;
	struct gpio_desc	rst_gpio;
	u32		gen;
	u32		num_lanes;
};

/* Parameters for the waiting for iATU enabled routine */
#define PCIE_CLIENT_GENERAL_DEBUG	0x104
#define PCIE_CLIENT_HOT_RESET_CTRL	0x180
#define PCIE_LTSSM_ENABLE_ENHANCE	BIT(4)
#define PCIE_CLIENT_LTSSM_STATUS	0x300
#define SMLH_LINKUP			BIT(16)
#define RDLH_LINKUP			BIT(17)
#define PCIE_CLIENT_DBG_FIFO_MODE_CON	0x310
#define PCIE_CLIENT_DBG_FIFO_PTN_HIT_D0 0x320
#define PCIE_CLIENT_DBG_FIFO_PTN_HIT_D1 0x324
#define PCIE_CLIENT_DBG_FIFO_TRN_HIT_D0 0x328
#define PCIE_CLIENT_DBG_FIFO_TRN_HIT_D1 0x32c
#define PCIE_CLIENT_DBG_FIFO_STATUS	0x350
#define PCIE_CLIENT_DBG_TRANSITION_DATA	0xffff0000
#define PCIE_CLIENT_DBF_EN		0xffff0003

#define PCIE_TYPE0_HDR_DBI2_OFFSET	0x100000

static int rk_pcie_read(void __iomem *addr, int size, u32 *val)
{
	if ((uintptr_t)addr & (size - 1)) {
		*val = 0;
		return -EOPNOTSUPP;
	}

	if (size == 4) {
		*val = readl(addr);
	} else if (size == 2) {
		*val = readw(addr);
	} else if (size == 1) {
		*val = readb(addr);
	} else {
		*val = 0;
		return -ENODEV;
	}

	return 0;
}

static int rk_pcie_write(void __iomem *addr, int size, u32 val)
{
	if ((uintptr_t)addr & (size - 1))
		return -EOPNOTSUPP;

	if (size == 4)
		writel(val, addr);
	else if (size == 2)
		writew(val, addr);
	else if (size == 1)
		writeb(val, addr);
	else
		return -ENODEV;

	return 0;
}

static u32 __rk_pcie_read_apb(struct rk_pcie *rk_pcie, void __iomem *base,
			      u32 reg, size_t size)
{
	int ret;
	u32 val;

	ret = rk_pcie_read(base + reg, size, &val);
	if (ret)
		dev_err(rk_pcie->dw.dev, "Read APB address failed\n");

	return val;
}

static void __rk_pcie_write_apb(struct rk_pcie *rk_pcie, void __iomem *base,
				u32 reg, size_t size, u32 val)
{
	int ret;

	ret = rk_pcie_write(base + reg, size, val);
	if (ret)
		dev_err(rk_pcie->dw.dev, "Write APB address failed\n");
}

/**
 * rk_pcie_readl_apb() - Read vendor regs
 *
 * @rk_pcie: Pointer to the PCI controller state
 * @reg: Offset of regs
 */
static inline u32 rk_pcie_readl_apb(struct rk_pcie *rk_pcie, u32 reg)
{
	return __rk_pcie_read_apb(rk_pcie, rk_pcie->apb_base, reg, 0x4);
}

/**
 * rk_pcie_writel_apb() - Write vendor regs
 *
 * @rk_pcie: Pointer to the PCI controller state
 * @reg: Offset of regs
 * @val: Value to be writen
 */
static inline void rk_pcie_writel_apb(struct rk_pcie *rk_pcie, u32 reg,
				      u32 val)
{
	__rk_pcie_write_apb(rk_pcie, rk_pcie->apb_base, reg, 0x4, val);
}

/**
 * rk_pcie_configure() - Configure link capabilities and speed
 *
 * @rk_pcie: Pointer to the PCI controller state
 *
 * Configure the link capabilities and speed in the PCIe root complex.
 */
static void rk_pcie_configure(struct rk_pcie *pci)
{
	u32 val;

	dw_pcie_dbi_write_enable(&pci->dw, true);

	/* Disable BAR 0 and BAR 1 */
	writel(0, pci->dw.dbi_base + PCIE_TYPE0_HDR_DBI2_OFFSET +
	       PCI_BASE_ADDRESS_0);
	writel(0, pci->dw.dbi_base + PCIE_TYPE0_HDR_DBI2_OFFSET +
	       PCI_BASE_ADDRESS_1);

	clrsetbits_le32(pci->dw.dbi_base + PCIE_LINK_CAPABILITY,
			TARGET_LINK_SPEED_MASK, pci->gen);

	clrsetbits_le32(pci->dw.dbi_base + PCIE_LINK_CTL_2,
			TARGET_LINK_SPEED_MASK, pci->gen);

	/* Set the number of lanes */
	val = readl(pci->dw.dbi_base + PCIE_PORT_LINK_CONTROL);
	val &= ~PORT_LINK_FAST_LINK_MODE;
	val |= PORT_LINK_DLL_LINK_EN;
	val &= ~PORT_LINK_MODE_MASK;
	switch (pci->num_lanes) {
	case 1:
		val |= PORT_LINK_MODE_1_LANES;
		break;
	case 2:
		val |= PORT_LINK_MODE_2_LANES;
		break;
	case 4:
		val |= PORT_LINK_MODE_4_LANES;
		break;
	default:
		dev_err(pci->dw.dev, "num-lanes %u: invalid value\n", pci->num_lanes);
		goto out;
	}
	writel(val, pci->dw.dbi_base + PCIE_PORT_LINK_CONTROL);

	/* Set link width speed control register */
	val = readl(pci->dw.dbi_base + PCIE_LINK_WIDTH_SPEED_CONTROL);
	val &= ~PORT_LOGIC_LINK_WIDTH_MASK;
	switch (pci->num_lanes) {
	case 1:
		val |= PORT_LOGIC_LINK_WIDTH_1_LANES;
		break;
	case 2:
		val |= PORT_LOGIC_LINK_WIDTH_2_LANES;
		break;
	case 4:
		val |= PORT_LOGIC_LINK_WIDTH_4_LANES;
		break;
	}
	writel(val, pci->dw.dbi_base + PCIE_LINK_WIDTH_SPEED_CONTROL);

out:
	dw_pcie_dbi_write_enable(&pci->dw, false);
}

static void rk_pcie_enable_debug(struct rk_pcie *rk_pcie)
{
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_DBG_FIFO_PTN_HIT_D0,
			   PCIE_CLIENT_DBG_TRANSITION_DATA);
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_DBG_FIFO_PTN_HIT_D1,
			   PCIE_CLIENT_DBG_TRANSITION_DATA);
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_DBG_FIFO_TRN_HIT_D0,
			   PCIE_CLIENT_DBG_TRANSITION_DATA);
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_DBG_FIFO_TRN_HIT_D1,
			   PCIE_CLIENT_DBG_TRANSITION_DATA);
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_DBG_FIFO_MODE_CON,
			   PCIE_CLIENT_DBF_EN);
}

static void rk_pcie_debug_dump(struct rk_pcie *rk_pcie)
{
	u32 loop;

	debug("ltssm = 0x%x\n",
	      rk_pcie_readl_apb(rk_pcie, PCIE_CLIENT_LTSSM_STATUS));
	for (loop = 0; loop < 64; loop++)
		debug("fifo_status = 0x%x\n",
		      rk_pcie_readl_apb(rk_pcie, PCIE_CLIENT_DBG_FIFO_STATUS));
}

static inline void rk_pcie_link_status_clear(struct rk_pcie *rk_pcie)
{
	rk_pcie_writel_apb(rk_pcie, PCIE_CLIENT_GENERAL_DEBUG, 0x0);
}

static inline void rk_pcie_disable_ltssm(struct rk_pcie *rk_pcie)
{
	rk_pcie_writel_apb(rk_pcie, 0x0, 0xc0008);
}

static inline void rk_pcie_enable_ltssm(struct rk_pcie *rk_pcie)
{
	rk_pcie_writel_apb(rk_pcie, 0x0, 0xc000c);
}

static int is_link_up(struct rk_pcie *priv)
{
	u32 val;

	val = rk_pcie_readl_apb(priv, PCIE_CLIENT_LTSSM_STATUS);
	if ((val & (RDLH_LINKUP | SMLH_LINKUP)) == 0x30000 &&
	    (val & GENMASK(5, 0)) == 0x11)
		return 1;

	return 0;
}

/**
 * rk_pcie_link_up() - Wait for the link to come up
 *
 * @rk_pcie: Pointer to the PCI controller state
 *
 * Return: 1 (true) for active line and negetive (false) for no link (timeout)
 */
static int rk_pcie_link_up(struct rk_pcie *priv)
{
	int retries;

	if (is_link_up(priv)) {
		printf("PCI Link already up before configuration!\n");
		return 1;
	}

	/* DW pre link configurations */
	rk_pcie_configure(priv);

	rk_pcie_disable_ltssm(priv);
	rk_pcie_link_status_clear(priv);
	rk_pcie_enable_debug(priv);

	/* Reset the device */
	if (dm_gpio_is_valid(&priv->rst_gpio))
		dm_gpio_set_value(&priv->rst_gpio, 0);

	/* Enable LTSSM */
	rk_pcie_enable_ltssm(priv);

	/*
	 * PCIe requires the refclk to be stable for 100ms prior to releasing
	 * PERST. See table 2-4 in section 2.6.2 AC Specifications of the PCI
	 * Express Card Electromechanical Specification, 1.1. However, we don't
	 * know if the refclk is coming from RC's PHY or external OSC. If it's
	 * from RC, so enabling LTSSM is the just right place to release #PERST.
	 */
	mdelay(100);
	if (dm_gpio_is_valid(&priv->rst_gpio))
		dm_gpio_set_value(&priv->rst_gpio, 1);

	/* Check if the link is up or not */
	for (retries = 0; retries < 10; retries++) {
		if (is_link_up(priv))
			break;

		mdelay(100);
	}

	if (retries >= 10) {
		dev_err(priv->dw.dev, "PCIe-%d Link Fail\n",
			dev_seq(priv->dw.dev));
		return -EIO;
	}

	dev_info(priv->dw.dev, "PCIe Link up, LTSSM is 0x%x\n",
		 rk_pcie_readl_apb(priv, PCIE_CLIENT_LTSSM_STATUS));
	rk_pcie_debug_dump(priv);
	return 0;
}

static int rockchip_pcie_init_port(struct udevice *dev)
{
	int ret;
	u32 val;
	struct rk_pcie *priv = dev_get_priv(dev);

	ret = reset_assert_bulk(&priv->rsts);
	if (ret) {
		dev_err(dev, "failed to assert resets (ret=%d)\n", ret);
		return ret;
	}

	/* Set power and maybe external ref clk input */
	ret = regulator_set_enable_if_allowed(priv->vpcie3v3, true);
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "failed to enable vpcie3v3 (ret=%d)\n", ret);
		return ret;
	}

	ret = generic_phy_init(&priv->phy);
	if (ret) {
		dev_err(dev, "failed to init phy (ret=%d)\n", ret);
		goto err_disable_regulator;
	}

	ret = generic_phy_power_on(&priv->phy);
	if (ret) {
		dev_err(dev, "failed to power on phy (ret=%d)\n", ret);
		goto err_exit_phy;
	}

	ret = reset_deassert_bulk(&priv->rsts);
	if (ret) {
		dev_err(dev, "failed to deassert resets (ret=%d)\n", ret);
		goto err_power_off_phy;
	}

	ret = clk_enable_bulk(&priv->clks);
	if (ret) {
		dev_err(dev, "failed to enable clks (ret=%d)\n", ret);
		goto err_deassert_bulk;
	}

	/* LTSSM EN ctrl mode */
	val = rk_pcie_readl_apb(priv, PCIE_CLIENT_HOT_RESET_CTRL);
	val |= PCIE_LTSSM_ENABLE_ENHANCE | (PCIE_LTSSM_ENABLE_ENHANCE << 16);
	rk_pcie_writel_apb(priv, PCIE_CLIENT_HOT_RESET_CTRL, val);

	/* Set RC mode */
	rk_pcie_writel_apb(priv, 0x0, 0xf00040);
	pcie_dw_setup_host(&priv->dw);

	ret = rk_pcie_link_up(priv);
	if (ret < 0)
		goto err_link_up;

	return 0;
err_link_up:
	clk_disable_bulk(&priv->clks);
err_deassert_bulk:
	reset_assert_bulk(&priv->rsts);
err_power_off_phy:
	generic_phy_power_off(&priv->phy);
err_exit_phy:
	generic_phy_exit(&priv->phy);
err_disable_regulator:
	regulator_set_enable_if_allowed(priv->vpcie3v3, false);

	return ret;
}

static int rockchip_pcie_parse_dt(struct udevice *dev)
{
	struct rk_pcie *priv = dev_get_priv(dev);
	int ret;

	priv->dw.dbi_base = dev_read_addr_index_ptr(dev, 0);
	if (!priv->dw.dbi_base)
		return -EINVAL;

	dev_dbg(dev, "DBI address is 0x%p\n", priv->dw.dbi_base);

	priv->apb_base = dev_read_addr_index_ptr(dev, 1);
	if (!priv->apb_base)
		return -EINVAL;

	dev_dbg(dev, "APB address is 0x%p\n", priv->apb_base);

	priv->dw.cfg_base = dev_read_addr_size_index_ptr(dev, 2,
							 &priv->dw.cfg_size);
	if (!priv->dw.cfg_base)
		return -EINVAL;

	dev_dbg(dev, "CFG address is 0x%p\n", priv->dw.cfg_base);

	ret = gpio_request_by_name(dev, "reset-gpios", 0,
				   &priv->rst_gpio, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "failed to find reset-gpios property\n");
		return ret;
	}

	ret = reset_get_bulk(dev, &priv->rsts);
	if (ret) {
		dev_err(dev, "Can't get reset: %d\n", ret);
		goto rockchip_pcie_parse_dt_err_reset_get_bulk;
	}

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret) {
		dev_err(dev, "Can't get clock: %d\n", ret);
		goto rockchip_pcie_parse_dt_err_clk_get_bulk;
	}

	ret = device_get_supply_regulator(dev, "vpcie3v3-supply",
					  &priv->vpcie3v3);
	if (ret && ret != -ENOENT) {
		dev_err(dev, "failed to get vpcie3v3 supply (ret=%d)\n", ret);
		goto rockchip_pcie_parse_dt_err_supply_regulator;
	}

	ret = generic_phy_get_by_index(dev, 0, &priv->phy);
	if (ret) {
		dev_err(dev, "failed to get pcie phy (ret=%d)\n", ret);
		goto rockchip_pcie_parse_dt_err_phy_get_by_index;
	}

	priv->gen = dev_read_u32_default(dev, "max-link-speed",
					 LINK_SPEED_GEN_3);

	priv->num_lanes = dev_read_u32_default(dev, "num-lanes", 1);

	return 0;

rockchip_pcie_parse_dt_err_phy_get_by_index:
	/* regulators don't need release */
rockchip_pcie_parse_dt_err_supply_regulator:
	clk_release_bulk(&priv->clks);
rockchip_pcie_parse_dt_err_clk_get_bulk:
	reset_release_bulk(&priv->rsts);
rockchip_pcie_parse_dt_err_reset_get_bulk:
	dm_gpio_free(dev, &priv->rst_gpio);
	return ret;
}

/**
 * rockchip_pcie_probe() - Probe the PCIe bus for active link
 *
 * @dev: A pointer to the device being operated on
 *
 * Probe for an active link on the PCIe bus and configure the controller
 * to enable this port.
 *
 * Return: 0 on success, else -ENODEV
 */
static int rockchip_pcie_probe(struct udevice *dev)
{
	struct rk_pcie *priv = dev_get_priv(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	int ret = 0;

	priv->dw.first_busno = dev_seq(dev);
	priv->dw.dev = dev;

	ret = rockchip_pcie_parse_dt(dev);
	if (ret)
		return ret;

	ret = rockchip_pcie_init_port(dev);
	if (ret)
		goto rockchip_pcie_probe_err_init_port;

	dev_info(dev, "PCIE-%d: Link up (Gen%d-x%d, Bus%d)\n",
		 dev_seq(dev), pcie_dw_get_link_speed(&priv->dw),
		 pcie_dw_get_link_width(&priv->dw),
		 hose->first_busno);


	ret = pcie_dw_prog_outbound_atu_unroll(&priv->dw,
					       PCIE_ATU_REGION_INDEX0,
					       PCIE_ATU_TYPE_MEM,
					       priv->dw.mem.phys_start,
					       priv->dw.mem.bus_start,
					       priv->dw.mem.size);
	if (!ret)
		return ret;

rockchip_pcie_probe_err_init_port:
	clk_release_bulk(&priv->clks);
	reset_release_bulk(&priv->rsts);
	dm_gpio_free(dev, &priv->rst_gpio);

	return ret;
}

static const struct dm_pci_ops rockchip_pcie_ops = {
	.read_config	= pcie_dw_read_config,
	.write_config	= pcie_dw_write_config,
};

static const struct udevice_id rockchip_pcie_ids[] = {
	{ .compatible = "rockchip,rk3568-pcie" },
	{ .compatible = "rockchip,rk3588-pcie" },
	{ }
};

U_BOOT_DRIVER(rockchip_dw_pcie) = {
	.name			= "pcie_dw_rockchip",
	.id			= UCLASS_PCI,
	.of_match		= rockchip_pcie_ids,
	.ops			= &rockchip_pcie_ops,
	.probe			= rockchip_pcie_probe,
	.priv_auto		= sizeof(struct rk_pcie),
};
