// SPDX-License-Identifier: GPL-2.0+
/*
 * Rockchip AXI PCIe host controller driver
 *
 * Copyright (c) 2016 Rockchip, Inc.
 * Copyright (c) 2020 Amarula Solutions(India)
 * Copyright (c) 2020 Jagan Teki <jagan@amarulasolutions.com>
 * Copyright (c) 2019 Patrick Wildt <patrick@blueri.se>
 * Copyright (c) 2018 Mark Kettenis <kettenis@openbsd.org>
 *
 * Bits taken from Linux Rockchip PCIe host controller.
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <generic-phy.h>
#include <pci.h>
#include <power/regulator.h>
#include <reset.h>
#include <asm-generic/gpio.h>
#include <linux/iopoll.h>

#define HIWORD_UPDATE(mask, val)        (((mask) << 16) | (val))
#define HIWORD_UPDATE_BIT(val)          HIWORD_UPDATE(val, val)

#define ENCODE_LANES(x)                 ((((x) >> 1) & 3) << 4)
#define PCIE_CLIENT_BASE                0x0
#define PCIE_CLIENT_CONFIG              (PCIE_CLIENT_BASE + 0x00)
#define PCIE_CLIENT_CONF_ENABLE         HIWORD_UPDATE_BIT(0x0001)
#define PCIE_CLIENT_LINK_TRAIN_ENABLE   HIWORD_UPDATE_BIT(0x0002)
#define PCIE_CLIENT_MODE_RC             HIWORD_UPDATE_BIT(0x0040)
#define PCIE_CLIENT_GEN_SEL_1           HIWORD_UPDATE(0x0080, 0)
#define PCIE_CLIENT_BASIC_STATUS1	0x0048
#define PCIE_CLIENT_LINK_STATUS_UP	GENMASK(21, 20)
#define PCIE_CLIENT_LINK_STATUS_MASK	GENMASK(21, 20)
#define PCIE_LINK_UP(x) \
	(((x) & PCIE_CLIENT_LINK_STATUS_MASK) == PCIE_CLIENT_LINK_STATUS_UP)
#define PCIE_RC_NORMAL_BASE		0x800000
#define PCIE_LM_BASE			0x900000
#define PCIE_LM_VENDOR_ID              (PCIE_LM_BASE + 0x44)
#define PCIE_LM_VENDOR_ROCKCHIP		0x1d87
#define PCIE_LM_RCBAR			(PCIE_LM_BASE + 0x300)
#define PCIE_LM_RCBARPIE		BIT(19)
#define PCIE_LM_RCBARPIS		BIT(20)
#define PCIE_RC_BASE			0xa00000
#define PCIE_RC_CONFIG_DCR		(PCIE_RC_BASE + 0x0c4)
#define PCIE_RC_CONFIG_DCR_CSPL_SHIFT	18
#define PCIE_RC_CONFIG_DCR_CPLS_SHIFT	26
#define PCIE_RC_PCIE_LCAP		(PCIE_RC_BASE + 0x0cc)
#define PCIE_RC_PCIE_LCAP_APMS_L0S	BIT(10)
#define PCIE_ATR_BASE			0xc00000
#define PCIE_ATR_OB_ADDR0(i)		(PCIE_ATR_BASE + 0x000 + (i) * 0x20)
#define PCIE_ATR_OB_ADDR1(i)		(PCIE_ATR_BASE + 0x004 + (i) * 0x20)
#define PCIE_ATR_OB_DESC0(i)		(PCIE_ATR_BASE + 0x008 + (i) * 0x20)
#define PCIE_ATR_OB_DESC1(i)		(PCIE_ATR_BASE + 0x00c + (i) * 0x20)
#define PCIE_ATR_IB_ADDR0(i)		(PCIE_ATR_BASE + 0x800 + (i) * 0x8)
#define PCIE_ATR_IB_ADDR1(i)		(PCIE_ATR_BASE + 0x804 + (i) * 0x8)
#define PCIE_ATR_HDR_MEM		0x2
#define PCIE_ATR_HDR_IO			0x6
#define PCIE_ATR_HDR_CFG_TYPE0		0xa
#define PCIE_ATR_HDR_CFG_TYPE1		0xb
#define PCIE_ATR_HDR_RID		BIT(23)

#define PCIE_ATR_OB_REGION0_SIZE	(32 * 1024 * 1024)
#define PCIE_ATR_OB_REGION_SIZE		(1 * 1024 * 1024)

struct rockchip_pcie {
	fdt_addr_t axi_base;
	fdt_addr_t apb_base;
	int first_busno;
	struct udevice *dev;

	/* resets */
	struct reset_ctl core_rst;
	struct reset_ctl mgmt_rst;
	struct reset_ctl mgmt_sticky_rst;
	struct reset_ctl pipe_rst;
	struct reset_ctl pm_rst;
	struct reset_ctl pclk_rst;
	struct reset_ctl aclk_rst;

	/* gpio */
	struct gpio_desc ep_gpio;

	/* vpcie regulators */
	struct udevice *vpcie12v;
	struct udevice *vpcie3v3;
	struct udevice *vpcie1v8;
	struct udevice *vpcie0v9;

	/* phy */
	struct phy pcie_phy;
};

static int rockchip_pcie_rd_conf(const struct udevice *udev, pci_dev_t bdf,
				 uint offset, ulong *valuep,
				 enum pci_size_t size)
{
	struct rockchip_pcie *priv = dev_get_priv(udev);
	unsigned int bus = PCI_BUS(bdf);
	unsigned int dev = PCI_DEV(bdf);
	int where = PCIE_ECAM_OFFSET(PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), offset & ~0x3);
	ulong value;

	if (bus == priv->first_busno && dev == 0) {
		value = readl(priv->apb_base + PCIE_RC_NORMAL_BASE + where);
		*valuep = pci_conv_32_to_size(value, offset, size);
		return 0;
	}

	if ((bus == priv->first_busno + 1) && dev == 0) {
		value = readl(priv->axi_base + where);
		*valuep = pci_conv_32_to_size(value, offset, size);
		return 0;
	}

	*valuep = pci_get_ff(size);

	return 0;
}

static int rockchip_pcie_wr_conf(struct udevice *udev, pci_dev_t bdf,
				 uint offset, ulong value,
				 enum pci_size_t size)
{
	struct rockchip_pcie *priv = dev_get_priv(udev);
	unsigned int bus = PCI_BUS(bdf);
	unsigned int dev = PCI_DEV(bdf);
	int where = PCIE_ECAM_OFFSET(PCI_BUS(bdf), PCI_DEV(bdf), PCI_FUNC(bdf), offset & ~0x3);
	ulong old;

	if (bus == priv->first_busno && dev == 0) {
		old = readl(priv->apb_base + PCIE_RC_NORMAL_BASE + where);
		value = pci_conv_size_to_32(old, value, offset, size);
		writel(value, priv->apb_base + PCIE_RC_NORMAL_BASE + where);
		return 0;
	}

	if ((bus == priv->first_busno + 1) && dev == 0) {
		old = readl(priv->axi_base + where);
		value = pci_conv_size_to_32(old, value, offset, size);
		writel(value, priv->axi_base + where);
		return 0;
	}

	return 0;
}

static int rockchip_pcie_atr_init(struct rockchip_pcie *priv)
{
	struct udevice *ctlr = pci_get_controller(priv->dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	u64 addr, size, offset;
	u32 type;
	int i, region;

	/* Use region 0 to map PCI configuration space. */
	writel(25 - 1, priv->apb_base + PCIE_ATR_OB_ADDR0(0));
	writel(0, priv->apb_base + PCIE_ATR_OB_ADDR1(0));
	writel(PCIE_ATR_HDR_CFG_TYPE0 | PCIE_ATR_HDR_RID,
	       priv->apb_base + PCIE_ATR_OB_DESC0(0));
	writel(0, priv->apb_base + PCIE_ATR_OB_DESC1(0));

	for (i = 0; i < hose->region_count; i++) {
		if (hose->regions[i].flags == PCI_REGION_SYS_MEMORY)
			continue;

		if (hose->regions[i].flags == PCI_REGION_IO)
			type = PCIE_ATR_HDR_IO;
		else
			type = PCIE_ATR_HDR_MEM;

		/* Only support identity mappings. */
		if (hose->regions[i].bus_start !=
		    hose->regions[i].phys_start)
			return -EINVAL;

		/* Only support mappings aligned on a region boundary. */
		addr = hose->regions[i].bus_start;
		if (addr & (PCIE_ATR_OB_REGION_SIZE - 1))
			return -EINVAL;

		/* Mappings should lie between AXI and APB regions. */
		size = hose->regions[i].size;
		if (addr < (u64)priv->axi_base + PCIE_ATR_OB_REGION0_SIZE)
			return -EINVAL;
		if (addr + size > (u64)priv->apb_base)
			return -EINVAL;

		offset = addr - (u64)priv->axi_base - PCIE_ATR_OB_REGION0_SIZE;
		region = 1 + (offset / PCIE_ATR_OB_REGION_SIZE);
		while (size > 0) {
			writel(32 - 1,
			       priv->apb_base + PCIE_ATR_OB_ADDR0(region));
			writel(0, priv->apb_base + PCIE_ATR_OB_ADDR1(region));
			writel(type | PCIE_ATR_HDR_RID,
			       priv->apb_base + PCIE_ATR_OB_DESC0(region));
			writel(0, priv->apb_base + PCIE_ATR_OB_DESC1(region));

			addr += PCIE_ATR_OB_REGION_SIZE;
			size -= PCIE_ATR_OB_REGION_SIZE;
			region++;
		}
	}

	/* Passthrough inbound translations unmodified. */
	writel(32 - 1, priv->apb_base + PCIE_ATR_IB_ADDR0(2));
	writel(0, priv->apb_base + PCIE_ATR_IB_ADDR1(2));

	return 0;
}

static int rockchip_pcie_init_port(struct udevice *dev)
{
	struct rockchip_pcie *priv = dev_get_priv(dev);
	u32 cr, val, status;
	int ret;

	if (dm_gpio_is_valid(&priv->ep_gpio))
		dm_gpio_set_value(&priv->ep_gpio, 0);

	ret = reset_assert(&priv->aclk_rst);
	if (ret) {
		dev_err(dev, "failed to assert aclk reset (ret=%d)\n", ret);
		return ret;
	}

	ret = reset_assert(&priv->pclk_rst);
	if (ret) {
		dev_err(dev, "failed to assert pclk reset (ret=%d)\n", ret);
		return ret;
	}

	ret = reset_assert(&priv->pm_rst);
	if (ret) {
		dev_err(dev, "failed to assert pm reset (ret=%d)\n", ret);
		return ret;
	}

	ret = generic_phy_init(&priv->pcie_phy);
	if (ret) {
		dev_err(dev, "failed to init phy (ret=%d)\n", ret);
		goto err_exit_phy;
	}

	ret = reset_assert(&priv->core_rst);
	if (ret) {
		dev_err(dev, "failed to assert core reset (ret=%d)\n", ret);
		goto err_exit_phy;
	}

	ret = reset_assert(&priv->mgmt_rst);
	if (ret) {
		dev_err(dev, "failed to assert mgmt reset (ret=%d)\n", ret);
		goto err_exit_phy;
	}

	ret = reset_assert(&priv->mgmt_sticky_rst);
	if (ret) {
		dev_err(dev, "failed to assert mgmt-sticky reset (ret=%d)\n",
			ret);
		goto err_exit_phy;
	}

	ret = reset_assert(&priv->pipe_rst);
	if (ret) {
		dev_err(dev, "failed to assert pipe reset (ret=%d)\n", ret);
		goto err_exit_phy;
	}

	udelay(10);

	ret = reset_deassert(&priv->pm_rst);
	if (ret) {
		dev_err(dev, "failed to deassert pm reset (ret=%d)\n", ret);
		goto err_exit_phy;
	}

	ret = reset_deassert(&priv->aclk_rst);
	if (ret) {
		dev_err(dev, "failed to deassert aclk reset (ret=%d)\n", ret);
		goto err_exit_phy;
	}

	ret = reset_deassert(&priv->pclk_rst);
	if (ret) {
		dev_err(dev, "failed to deassert pclk reset (ret=%d)\n", ret);
		goto err_exit_phy;
	}

	/* Select GEN1 for now */
	cr = PCIE_CLIENT_GEN_SEL_1;
	/* Set Root complex mode */
	cr |= PCIE_CLIENT_CONF_ENABLE | PCIE_CLIENT_MODE_RC;
	writel(cr, priv->apb_base + PCIE_CLIENT_CONFIG);

	ret = generic_phy_power_on(&priv->pcie_phy);
	if (ret) {
		dev_err(dev, "failed to power on phy (ret=%d)\n", ret);
		goto err_power_off_phy;
	}

	ret = reset_deassert(&priv->mgmt_sticky_rst);
	if (ret) {
		dev_err(dev, "failed to deassert mgmt-sticky reset (ret=%d)\n",
			ret);
		goto err_power_off_phy;
	}

	ret = reset_deassert(&priv->core_rst);
	if (ret) {
		dev_err(dev, "failed to deassert core reset (ret=%d)\n", ret);
		goto err_power_off_phy;
	}

	ret = reset_deassert(&priv->mgmt_rst);
	if (ret) {
		dev_err(dev, "failed to deassert mgmt reset (ret=%d)\n", ret);
		goto err_power_off_phy;
	}

	ret = reset_deassert(&priv->pipe_rst);
	if (ret) {
		dev_err(dev, "failed to deassert pipe reset (ret=%d)\n", ret);
		goto err_power_off_phy;
	}

	/* Enable Gen1 training */
	writel(PCIE_CLIENT_LINK_TRAIN_ENABLE,
	       priv->apb_base + PCIE_CLIENT_CONFIG);

	if (dm_gpio_is_valid(&priv->ep_gpio))
		dm_gpio_set_value(&priv->ep_gpio, 1);

	ret = readl_poll_sleep_timeout
			(priv->apb_base + PCIE_CLIENT_BASIC_STATUS1,
			status, PCIE_LINK_UP(status), 20, 500 * 1000);
	if (ret) {
		dev_err(dev, "PCIe link training gen1 timeout!\n");
		goto err_power_off_phy;
	}

	/* Initialize Root Complex registers. */
	writel(PCIE_LM_VENDOR_ROCKCHIP, priv->apb_base + PCIE_LM_VENDOR_ID);
	writel(PCI_CLASS_BRIDGE_PCI_NORMAL << 8,
	       priv->apb_base + PCIE_RC_BASE + PCI_CLASS_REVISION);
	writel(PCIE_LM_RCBARPIE | PCIE_LM_RCBARPIS,
	       priv->apb_base + PCIE_LM_RCBAR);

	if (dev_read_bool(dev, "aspm-no-l0s")) {
		val = readl(priv->apb_base + PCIE_RC_PCIE_LCAP);
		val &= ~PCIE_RC_PCIE_LCAP_APMS_L0S;
		writel(val, priv->apb_base + PCIE_RC_PCIE_LCAP);
	}

	/* Configure Address Translation. */
	ret = rockchip_pcie_atr_init(priv);
	if (ret) {
		dev_err(dev, "PCIE-%d: ATR init failed\n", dev_seq(dev));
		goto err_power_off_phy;
	}

	return 0;

err_power_off_phy:
	generic_phy_power_off(&priv->pcie_phy);
err_exit_phy:
	generic_phy_exit(&priv->pcie_phy);
	return ret;
}

static int rockchip_pcie_set_vpcie(struct udevice *dev)
{
	struct rockchip_pcie *priv = dev_get_priv(dev);
	int ret;

	ret = regulator_set_enable_if_allowed(priv->vpcie12v, true);
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "failed to enable vpcie12v (ret=%d)\n", ret);
		return ret;
	}

	ret = regulator_set_enable_if_allowed(priv->vpcie3v3, true);
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "failed to enable vpcie3v3 (ret=%d)\n", ret);
		goto err_disable_12v;
	}

	ret = regulator_set_enable_if_allowed(priv->vpcie1v8, true);
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "failed to enable vpcie1v8 (ret=%d)\n", ret);
		goto err_disable_3v3;
	}

	ret = regulator_set_enable_if_allowed(priv->vpcie0v9, true);
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "failed to enable vpcie0v9 (ret=%d)\n", ret);
		goto err_disable_1v8;
	}

	return 0;

err_disable_1v8:
	regulator_set_enable_if_allowed(priv->vpcie1v8, false);
err_disable_3v3:
	regulator_set_enable_if_allowed(priv->vpcie3v3, false);
err_disable_12v:
	regulator_set_enable_if_allowed(priv->vpcie12v, false);
	return ret;
}

static int rockchip_pcie_parse_dt(struct udevice *dev)
{
	struct rockchip_pcie *priv = dev_get_priv(dev);
	int ret;

	priv->axi_base = dev_read_addr_name(dev, "axi-base");
	if (priv->axi_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->apb_base = dev_read_addr_name(dev, "apb-base");
	if (priv->apb_base == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = reset_get_by_name(dev, "core", &priv->core_rst);
	if (ret) {
		dev_err(dev, "failed to get core reset (ret=%d)\n", ret);
		return ret;
	}

	ret = reset_get_by_name(dev, "mgmt", &priv->mgmt_rst);
	if (ret) {
		dev_err(dev, "failed to get mgmt reset (ret=%d)\n", ret);
		return ret;
	}

	ret = reset_get_by_name(dev, "mgmt-sticky", &priv->mgmt_sticky_rst);
	if (ret) {
		dev_err(dev, "failed to get mgmt-sticky reset (ret=%d)\n", ret);
		return ret;
	}

	ret = reset_get_by_name(dev, "pipe", &priv->pipe_rst);
	if (ret) {
		dev_err(dev, "failed to get pipe reset (ret=%d)\n", ret);
		return ret;
	}

	ret = reset_get_by_name(dev, "pm", &priv->pm_rst);
	if (ret) {
		dev_err(dev, "failed to get pm reset (ret=%d)\n", ret);
		return ret;
	}

	ret = reset_get_by_name(dev, "pclk", &priv->pclk_rst);
	if (ret) {
		dev_err(dev, "failed to get pclk reset (ret=%d)\n", ret);
		return ret;
	}

	ret = reset_get_by_name(dev, "aclk", &priv->aclk_rst);
	if (ret) {
		dev_err(dev, "failed to get aclk reset (ret=%d)\n", ret);
		return ret;
	}

	ret = device_get_supply_regulator(dev, "vpcie12v-supply",
					  &priv->vpcie12v);
	if (ret && ret != -ENOENT) {
		dev_err(dev, "failed to get vpcie12v supply (ret=%d)\n", ret);
		return ret;
	}

	ret = device_get_supply_regulator(dev, "vpcie3v3-supply",
					  &priv->vpcie3v3);
	if (ret && ret != -ENOENT) {
		dev_err(dev, "failed to get vpcie3v3 supply (ret=%d)\n", ret);
		return ret;
	}

	ret = device_get_supply_regulator(dev, "vpcie1v8-supply",
					  &priv->vpcie1v8);
	if (ret && ret != -ENOENT) {
		dev_err(dev, "failed to get vpcie1v8 supply (ret=%d)\n", ret);
		return ret;
	}

	ret = device_get_supply_regulator(dev, "vpcie0v9-supply",
					  &priv->vpcie0v9);
	if (ret && ret != -ENOENT) {
		dev_err(dev, "failed to get vpcie0v9 supply (ret=%d)\n", ret);
		return ret;
	}

	ret = generic_phy_get_by_index(dev, 0, &priv->pcie_phy);
	if (ret) {
		dev_err(dev, "failed to get pcie-phy (ret=%d)\n", ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "ep-gpios", 0,
				   &priv->ep_gpio, GPIOD_IS_OUT);
	if (ret) {
		dev_err(dev, "failed to find ep-gpios property\n");
		return ret;
	}

	return 0;
}

static int rockchip_pcie_probe(struct udevice *dev)
{
	struct rockchip_pcie *priv = dev_get_priv(dev);
	struct udevice *ctlr = pci_get_controller(dev);
	struct pci_controller *hose = dev_get_uclass_priv(ctlr);
	int ret;

	priv->first_busno = dev_seq(dev);
	priv->dev = dev;

	ret = rockchip_pcie_parse_dt(dev);
	if (ret)
		return ret;

	ret = rockchip_pcie_set_vpcie(dev);
	if (ret)
		goto err_gpio_free;

	ret = rockchip_pcie_init_port(dev);
	if (ret)
		goto err_disable_vpcie;

	dev_info(dev, "PCIE-%d: Link up (Bus%d)\n",
		 dev_seq(dev), hose->first_busno);

	return 0;

err_disable_vpcie:
	regulator_set_enable_if_allowed(priv->vpcie0v9, false);
	regulator_set_enable_if_allowed(priv->vpcie1v8, false);
	regulator_set_enable_if_allowed(priv->vpcie3v3, false);
	regulator_set_enable_if_allowed(priv->vpcie12v, false);
err_gpio_free:
	if (dm_gpio_is_valid(&priv->ep_gpio))
		dm_gpio_free(dev, &priv->ep_gpio);
	return ret;
}

static const struct dm_pci_ops rockchip_pcie_ops = {
	.read_config	= rockchip_pcie_rd_conf,
	.write_config	= rockchip_pcie_wr_conf,
};

static const struct udevice_id rockchip_pcie_ids[] = {
	{ .compatible = "rockchip,rk3399-pcie" },
	{ }
};

U_BOOT_DRIVER(rockchip_pcie) = {
	.name		= "rockchip_pcie",
	.id		= UCLASS_PCI,
	.of_match	= rockchip_pcie_ids,
	.ops		= &rockchip_pcie_ops,
	.probe		= rockchip_pcie_probe,
	.priv_auto	= sizeof(struct rockchip_pcie),
};
