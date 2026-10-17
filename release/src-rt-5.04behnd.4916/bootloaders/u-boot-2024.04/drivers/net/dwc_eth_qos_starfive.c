// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 StarFive Technology Co., Ltd.
 * Author: Yanhong Wang<yanhong.wang@starfivetech.com>
 */

#include <common.h>
#include <asm/cache.h>
#include <asm/gpio.h>
#include <clk.h>
#include <dm.h>
#include <eth_phy.h>
#include <net.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>

#include "dwc_eth_qos.h"

#define STARFIVE_DWMAC_PHY_INFT_RGMII	0x1
#define STARFIVE_DWMAC_PHY_INFT_RMII	0x4
#define STARFIVE_DWMAC_PHY_INFT_FIELD	0x7U

struct starfive_platform_data {
	struct regmap *regmap;
	struct reset_ctl_bulk resets;
	struct clk_bulk clks;
	phy_interface_t interface;
	u32 offset;
	u32 shift;
	bool tx_use_rgmii_clk;
};

static int eqos_interface_init_jh7110(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct starfive_platform_data *data = pdata->priv_pdata;
	struct ofnode_phandle_args args;
	unsigned int mode;
	int ret;

	switch (data->interface) {
	case PHY_INTERFACE_MODE_RMII:
		mode = STARFIVE_DWMAC_PHY_INFT_RMII;
		break;

	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
		mode = STARFIVE_DWMAC_PHY_INFT_RGMII;
		break;

	default:
		return -EINVAL;
	}

	ret = dev_read_phandle_with_args(dev, "starfive,syscon", NULL,
					 2, 0, &args);
	if (ret)
		return ret;

	if (args.args_count != 2)
		return -EINVAL;

	data->offset = args.args[0];
	data->shift = args.args[1];
	data->regmap = syscon_regmap_lookup_by_phandle(dev, "starfive,syscon");
	if (IS_ERR(data->regmap)) {
		ret = PTR_ERR(data->regmap);
		pr_err("Failed to get regmap: %d\n", ret);
		return ret;
	}

	return regmap_update_bits(data->regmap, data->offset,
				  STARFIVE_DWMAC_PHY_INFT_FIELD << data->shift,
				  mode << data->shift);
}

static int eqos_set_tx_clk_speed_jh7110(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct starfive_platform_data *data = pdata->priv_pdata;
	struct clk *pclk, *c;
	ulong rate;
	int ret;

	/* Generally, the rgmii_tx clock is provided by the internal clock,
	 * which needs to match the corresponding clock frequency according
	 * to different speeds. If the rgmii_tx clock is provided by the
	 * external rgmii_rxin, there is no need to configure the clock
	 * internally, because rgmii_rxin will be adaptively adjusted.
	 */
	if (data->tx_use_rgmii_clk)
		return 0;

	switch (eqos->phy->speed) {
	case SPEED_1000:
		rate = 125 * 1000 * 1000;
		break;
	case SPEED_100:
		rate = 25 * 1000 * 1000;
		break;
	case SPEED_10:
		rate = 2.5 * 1000 * 1000;
		break;
	default:
		pr_err("invalid speed %d", eqos->phy->speed);
		return -EINVAL;
	}

	/* eqos->clk_tx clock has no set rate operation, so just set the parent
	 * clock rate directly
	 */
	ret = clk_get_by_id(eqos->clk_tx.id, &c);
	if (ret)
		return ret;

	pclk = clk_get_parent(c);
	if (pclk) {
		ret = clk_set_rate(pclk, rate);
		if (ret < 0) {
			pr_err("jh7110 (clk_tx, %lu) failed: %d", rate, ret);
			return ret;
		}
	}

	return 0;
}

static ulong eqos_get_tick_clk_rate_jh7110(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	return clk_get_rate(&eqos->clk_tx);
}

static int eqos_start_clks_jh7110(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct starfive_platform_data *data = pdata->priv_pdata;

	return clk_enable_bulk(&data->clks);
}

static int eqos_stop_clks_jh7110(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct starfive_platform_data *data = pdata->priv_pdata;

	return clk_disable_bulk(&data->clks);
}

static int eqos_start_resets_jh7110(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct starfive_platform_data *data = pdata->priv_pdata;

	return reset_deassert_bulk(&data->resets);
}

static int eqos_stop_resets_jh7110(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct starfive_platform_data *data = pdata->priv_pdata;

	return reset_assert_bulk(&data->resets);
}

static int eqos_remove_resources_jh7110(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct starfive_platform_data *data = pdata->priv_pdata;

	reset_assert_bulk(&data->resets);
	clk_disable_bulk(&data->clks);

	return 0;
}

static int eqos_probe_resources_jh7110(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct starfive_platform_data *data;
	int ret;

	data = calloc(1, sizeof(struct starfive_platform_data));
	if (!data)
		return -ENOMEM;

	pdata->priv_pdata = data;
	data->interface = eqos->config->interface(dev);
	if (data->interface == PHY_INTERFACE_MODE_NA) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	ret = reset_get_bulk(dev, &data->resets);
	if (ret < 0)
		return ret;

	ret = clk_get_bulk(dev, &data->clks);
	if (ret < 0)
		return ret;

	ret = clk_get_by_name(dev, "gtx", &eqos->clk_tx);
	if (ret)
		return ret;

	data->tx_use_rgmii_clk = dev_read_bool(dev, "starfive,tx-use-rgmii-clk");

	return eqos_interface_init_jh7110(dev);
}

static struct eqos_ops eqos_jh7110_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_jh7110,
	.eqos_remove_resources = eqos_remove_resources_jh7110,
	.eqos_stop_resets = eqos_stop_resets_jh7110,
	.eqos_start_resets = eqos_start_resets_jh7110,
	.eqos_stop_clks = eqos_stop_clks_jh7110,
	.eqos_start_clks = eqos_start_clks_jh7110,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_set_tx_clk_speed_jh7110,
	.eqos_get_enetaddr = eqos_null_ops,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_jh7110
};

/* mdio_wait: There is no need to wait after setting the MAC_MDIO_Address register
 * swr_wait: Software reset bit must be read at least 4 CSR clock cycles
 *          after it is written to 1.
 * config_mac: Enable rx queue to DCB mode.
 * config_mac_mdio: CSR clock range is 250-300 Mhz.
 * axi_bus_width: The width of the data bus is 64 bit.
 */
struct eqos_config __maybe_unused eqos_jh7110_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 0,
	.swr_wait = 4,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_250_300,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = dev_read_phy_mode,
	.ops = &eqos_jh7110_ops
};
