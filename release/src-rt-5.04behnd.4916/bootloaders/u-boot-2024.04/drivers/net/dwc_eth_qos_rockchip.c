// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright Contributors to the U-Boot project.
 *
 * rk_gmac_ops ported from linux drivers/net/ethernet/stmicro/stmmac/dwmac-rk.c
 *
 * Ported code is intentionally left as close as possible with linux counter
 * part in order to simplify future porting of fixes and support for other SoCs.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <net.h>
#include <phy.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <linux/delay.h>

#include "dwc_eth_qos.h"

struct rk_gmac_ops {
	const char *compatible;
	int (*set_to_rgmii)(struct udevice *dev,
			    int tx_delay, int rx_delay);
	int (*set_to_rmii)(struct udevice *dev);
	int (*set_gmac_speed)(struct udevice *dev);
	void (*set_clock_selection)(struct udevice *dev, bool enable);
	u32 regs[3];
};

struct rockchip_platform_data {
	struct reset_ctl_bulk resets;
	const struct rk_gmac_ops *ops;
	int id;
	bool clock_input;
	struct regmap *grf;
	struct regmap *php_grf;
};

#define HIWORD_UPDATE(val, mask, shift) \
		((val) << (shift) | (mask) << ((shift) + 16))

#define GRF_BIT(nr)	(BIT(nr) | BIT((nr) + 16))
#define GRF_CLR_BIT(nr)	(BIT((nr) + 16))

#define RK3568_GRF_GMAC0_CON0		0x0380
#define RK3568_GRF_GMAC0_CON1		0x0384
#define RK3568_GRF_GMAC1_CON0		0x0388
#define RK3568_GRF_GMAC1_CON1		0x038c

/* RK3568_GRF_GMAC0_CON1 && RK3568_GRF_GMAC1_CON1 */
#define RK3568_GMAC_PHY_INTF_SEL_RGMII	\
		(GRF_BIT(4) | GRF_CLR_BIT(5) | GRF_CLR_BIT(6))
#define RK3568_GMAC_PHY_INTF_SEL_RMII	\
		(GRF_CLR_BIT(4) | GRF_CLR_BIT(5) | GRF_BIT(6))
#define RK3568_GMAC_FLOW_CTRL			GRF_BIT(3)
#define RK3568_GMAC_FLOW_CTRL_CLR		GRF_CLR_BIT(3)
#define RK3568_GMAC_RXCLK_DLY_ENABLE		GRF_BIT(1)
#define RK3568_GMAC_RXCLK_DLY_DISABLE		GRF_CLR_BIT(1)
#define RK3568_GMAC_TXCLK_DLY_ENABLE		GRF_BIT(0)
#define RK3568_GMAC_TXCLK_DLY_DISABLE		GRF_CLR_BIT(0)

/* RK3568_GRF_GMAC0_CON0 && RK3568_GRF_GMAC1_CON0 */
#define RK3568_GMAC_CLK_RX_DL_CFG(val)	HIWORD_UPDATE(val, 0x7F, 8)
#define RK3568_GMAC_CLK_TX_DL_CFG(val)	HIWORD_UPDATE(val, 0x7F, 0)

static int rk3568_set_to_rgmii(struct udevice *dev,
			       int tx_delay, int rx_delay)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct rockchip_platform_data *data = pdata->priv_pdata;
	u32 con0, con1;

	con0 = (data->id == 1) ? RK3568_GRF_GMAC1_CON0 :
				 RK3568_GRF_GMAC0_CON0;
	con1 = (data->id == 1) ? RK3568_GRF_GMAC1_CON1 :
				 RK3568_GRF_GMAC0_CON1;

	regmap_write(data->grf, con0,
		     RK3568_GMAC_CLK_RX_DL_CFG(rx_delay) |
		     RK3568_GMAC_CLK_TX_DL_CFG(tx_delay));

	regmap_write(data->grf, con1,
		     RK3568_GMAC_PHY_INTF_SEL_RGMII |
		     RK3568_GMAC_RXCLK_DLY_ENABLE |
		     RK3568_GMAC_TXCLK_DLY_ENABLE);

	return 0;
}

static int rk3568_set_to_rmii(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct rockchip_platform_data *data = pdata->priv_pdata;
	u32 con1;

	con1 = (data->id == 1) ? RK3568_GRF_GMAC1_CON1 :
				 RK3568_GRF_GMAC0_CON1;
	regmap_write(data->grf, con1, RK3568_GMAC_PHY_INTF_SEL_RMII);

	return 0;
}

static int rk3568_set_gmac_speed(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	ulong rate;
	int ret;

	switch (eqos->phy->speed) {
	case SPEED_10:
		rate = 2500000;
		break;
	case SPEED_100:
		rate = 25000000;
		break;
	case SPEED_1000:
		rate = 125000000;
		break;
	default:
		return -EINVAL;
	}

	ret = clk_set_rate(&eqos->clk_tx, rate);
	if (ret < 0)
		return ret;

	return 0;
}

/* sys_grf */
#define RK3588_GRF_GMAC_CON7			0x031c
#define RK3588_GRF_GMAC_CON8			0x0320
#define RK3588_GRF_GMAC_CON9			0x0324

#define RK3588_GMAC_RXCLK_DLY_ENABLE(id)	GRF_BIT(2 * (id) + 3)
#define RK3588_GMAC_RXCLK_DLY_DISABLE(id)	GRF_CLR_BIT(2 * (id) + 3)
#define RK3588_GMAC_TXCLK_DLY_ENABLE(id)	GRF_BIT(2 * (id) + 2)
#define RK3588_GMAC_TXCLK_DLY_DISABLE(id)	GRF_CLR_BIT(2 * (id) + 2)

#define RK3588_GMAC_CLK_RX_DL_CFG(val)		HIWORD_UPDATE(val, 0xFF, 8)
#define RK3588_GMAC_CLK_TX_DL_CFG(val)		HIWORD_UPDATE(val, 0xFF, 0)

/* php_grf */
#define RK3588_GRF_GMAC_CON0			0x0008
#define RK3588_GRF_CLK_CON1			0x0070

#define RK3588_GMAC_PHY_INTF_SEL_RGMII(id)	\
	(GRF_BIT(3 + (id) * 6) | GRF_CLR_BIT(4 + (id) * 6) | GRF_CLR_BIT(5 + (id) * 6))
#define RK3588_GMAC_PHY_INTF_SEL_RMII(id)	\
	(GRF_CLR_BIT(3 + (id) * 6) | GRF_CLR_BIT(4 + (id) * 6) | GRF_BIT(5 + (id) * 6))

#define RK3588_GMAC_CLK_RMII_MODE(id)		GRF_BIT(5 * (id))
#define RK3588_GMAC_CLK_RGMII_MODE(id)		GRF_CLR_BIT(5 * (id))

#define RK3588_GMAC_CLK_SELET_CRU(id)		GRF_BIT(5 * (id) + 4)
#define RK3588_GMAC_CLK_SELET_IO(id)		GRF_CLR_BIT(5 * (id) + 4)

#define RK3588_GMAC_CLK_RMII_DIV2(id)		GRF_BIT(5 * (id) + 2)
#define RK3588_GMAC_CLK_RMII_DIV20(id)		GRF_CLR_BIT(5 * (id) + 2)

#define RK3588_GMAC_CLK_RGMII_DIV1(id)		\
			(GRF_CLR_BIT(5 * (id) + 2) | GRF_CLR_BIT(5 * (id) + 3))
#define RK3588_GMAC_CLK_RGMII_DIV5(id)		\
			(GRF_BIT(5 * (id) + 2) | GRF_BIT(5 * (id) + 3))
#define RK3588_GMAC_CLK_RGMII_DIV50(id)		\
			(GRF_CLR_BIT(5 * (id) + 2) | GRF_BIT(5 * (id) + 3))

#define RK3588_GMAC_CLK_RMII_GATE(id)		GRF_BIT(5 * (id) + 1)
#define RK3588_GMAC_CLK_RMII_NOGATE(id)		GRF_CLR_BIT(5 * (id) + 1)

static int rk3588_set_to_rgmii(struct udevice *dev,
			       int tx_delay, int rx_delay)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct rockchip_platform_data *data = pdata->priv_pdata;
	u32 offset_con, id = data->id;

	offset_con = data->id == 1 ? RK3588_GRF_GMAC_CON9 :
				     RK3588_GRF_GMAC_CON8;

	regmap_write(data->php_grf, RK3588_GRF_GMAC_CON0,
		     RK3588_GMAC_PHY_INTF_SEL_RGMII(id));

	regmap_write(data->php_grf, RK3588_GRF_CLK_CON1,
		     RK3588_GMAC_CLK_RGMII_MODE(id));

	regmap_write(data->grf, RK3588_GRF_GMAC_CON7,
		     RK3588_GMAC_RXCLK_DLY_ENABLE(id) |
		     RK3588_GMAC_TXCLK_DLY_ENABLE(id));

	regmap_write(data->grf, offset_con,
		     RK3588_GMAC_CLK_RX_DL_CFG(rx_delay) |
		     RK3588_GMAC_CLK_TX_DL_CFG(tx_delay));

	return 0;
}

static int rk3588_set_to_rmii(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct rockchip_platform_data *data = pdata->priv_pdata;

	regmap_write(data->php_grf, RK3588_GRF_GMAC_CON0,
		     RK3588_GMAC_PHY_INTF_SEL_RMII(data->id));

	regmap_write(data->php_grf, RK3588_GRF_CLK_CON1,
		     RK3588_GMAC_CLK_RMII_MODE(data->id));

	return 0;
}

static int rk3588_set_gmac_speed(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct rockchip_platform_data *data = pdata->priv_pdata;
	u32 val = 0, id = data->id;

	switch (eqos->phy->speed) {
	case SPEED_10:
		if (pdata->phy_interface == PHY_INTERFACE_MODE_RMII)
			val = RK3588_GMAC_CLK_RMII_DIV20(id);
		else
			val = RK3588_GMAC_CLK_RGMII_DIV50(id);
		break;
	case SPEED_100:
		if (pdata->phy_interface == PHY_INTERFACE_MODE_RMII)
			val = RK3588_GMAC_CLK_RMII_DIV2(id);
		else
			val = RK3588_GMAC_CLK_RGMII_DIV5(id);
		break;
	case SPEED_1000:
		if (pdata->phy_interface != PHY_INTERFACE_MODE_RMII)
			val = RK3588_GMAC_CLK_RGMII_DIV1(id);
		else
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}

	regmap_write(data->php_grf, RK3588_GRF_CLK_CON1, val);

	return 0;
}

static void rk3588_set_clock_selection(struct udevice *dev, bool enable)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct rockchip_platform_data *data = pdata->priv_pdata;

	u32 val = data->clock_input ? RK3588_GMAC_CLK_SELET_IO(data->id) :
				      RK3588_GMAC_CLK_SELET_CRU(data->id);

	val |= enable ? RK3588_GMAC_CLK_RMII_NOGATE(data->id) :
			RK3588_GMAC_CLK_RMII_GATE(data->id);

	regmap_write(data->php_grf, RK3588_GRF_CLK_CON1, val);
}

static const struct rk_gmac_ops rk_gmac_ops[] = {
	{
		.compatible = "rockchip,rk3568-gmac",
		.set_to_rgmii = rk3568_set_to_rgmii,
		.set_to_rmii = rk3568_set_to_rmii,
		.set_gmac_speed = rk3568_set_gmac_speed,
		.regs = {
			0xfe2a0000, /* gmac0 */
			0xfe010000, /* gmac1 */
			0x0, /* sentinel */
		},
	},
	{
		.compatible = "rockchip,rk3588-gmac",
		.set_to_rgmii = rk3588_set_to_rgmii,
		.set_to_rmii = rk3588_set_to_rmii,
		.set_gmac_speed = rk3588_set_gmac_speed,
		.set_clock_selection = rk3588_set_clock_selection,
		.regs = {
			0xfe1b0000, /* gmac0 */
			0xfe1c0000, /* gmac1 */
			0x0, /* sentinel */
		},
	},
	{ }
};

static const struct rk_gmac_ops *get_rk_gmac_ops(struct udevice *dev)
{
	const struct rk_gmac_ops *ops = rk_gmac_ops;

	while (ops->compatible) {
		if (device_is_compatible(dev, ops->compatible))
			return ops;
		ops++;
	}

	return NULL;
}

static int eqos_probe_resources_rk(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct rockchip_platform_data *data;
	const char *clock_in_out;
	int reset_flags = GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE;
	int ret;

	data = calloc(1, sizeof(struct rockchip_platform_data));
	if (!data)
		return -ENOMEM;

	data->ops = get_rk_gmac_ops(dev);
	if (!data->ops) {
		ret = -EINVAL;
		goto err_free;
	}

	for (int i = 0; data->ops->regs[i]; i++) {
		if (data->ops->regs[i] == (u32)eqos->regs) {
			data->id = i;
			break;
		}
	}

	pdata->priv_pdata = data;
	pdata->phy_interface = eqos->config->interface(dev);
	pdata->max_speed = eqos->max_speed;

	if (pdata->phy_interface == PHY_INTERFACE_MODE_NA) {
		pr_err("Invalid PHY interface\n");
		ret = -EINVAL;
		goto err_free;
	}

	data->grf = syscon_regmap_lookup_by_phandle(dev, "rockchip,grf");
	if (IS_ERR(data->grf)) {
		dev_err(dev, "Missing rockchip,grf property\n");
		ret = -EINVAL;
		goto err_free;
	}

	if (device_is_compatible(dev, "rockchip,rk3588-gmac")) {
		data->php_grf =
			syscon_regmap_lookup_by_phandle(dev, "rockchip,php-grf");
		if (IS_ERR(data->php_grf)) {
			dev_err(dev, "Missing rockchip,php-grf property\n");
			ret = -EINVAL;
			goto err_free;
		}
	}

	ret = reset_get_bulk(dev, &data->resets);
	if (ret < 0)
		goto err_free;

	reset_assert_bulk(&data->resets);

	ret = clk_get_by_name(dev, "stmmaceth", &eqos->clk_master_bus);
	if (ret) {
		dev_dbg(dev, "clk_get_by_name(stmmaceth) failed: %d", ret);
		goto err_release_resets;
	}

	if (device_is_compatible(dev, "rockchip,rk3568-gmac")) {
		ret = clk_get_by_name(dev, "clk_mac_speed", &eqos->clk_tx);
		if (ret) {
			dev_dbg(dev, "clk_get_by_name(clk_mac_speed) failed: %d", ret);
			goto err_release_resets;
		}
	}

	clock_in_out = dev_read_string(dev, "clock_in_out");
	if (clock_in_out && !strcmp(clock_in_out, "input"))
		data->clock_input = true;
	else
		data->clock_input = false;

	/* snps,reset props are deprecated, do bare minimum to support them */
	if (dev_read_bool(dev, "snps,reset-active-low"))
		reset_flags |= GPIOD_ACTIVE_LOW;

	dev_read_u32_array(dev, "snps,reset-delays-us", eqos->reset_delays, 3);

	gpio_request_by_name(dev, "snps,reset-gpio", 0,
			     &eqos->phy_reset_gpio, reset_flags);

	return 0;

err_release_resets:
	reset_release_bulk(&data->resets);
err_free:
	free(data);

	return ret;
}

static int eqos_remove_resources_rk(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct rockchip_platform_data *data = pdata->priv_pdata;

	if (dm_gpio_is_valid(&eqos->phy_reset_gpio))
		dm_gpio_free(dev, &eqos->phy_reset_gpio);

	reset_release_bulk(&data->resets);
	free(data);

	return 0;
}

static int eqos_stop_resets_rk(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct rockchip_platform_data *data = pdata->priv_pdata;

	return reset_assert_bulk(&data->resets);
}

static int eqos_start_resets_rk(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct rockchip_platform_data *data = pdata->priv_pdata;

	return reset_deassert_bulk(&data->resets);
}

static int eqos_stop_clks_rk(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct rockchip_platform_data *data = pdata->priv_pdata;

	if (data->ops->set_clock_selection)
		data->ops->set_clock_selection(dev, false);

	return 0;
}

static int eqos_start_clks_rk(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct rockchip_platform_data *data = pdata->priv_pdata;
	int tx_delay, rx_delay, ret;

	if (dm_gpio_is_valid(&eqos->phy_reset_gpio)) {
		udelay(eqos->reset_delays[1]);

		ret = dm_gpio_set_value(&eqos->phy_reset_gpio, 0);
		if (ret < 0)
			return ret;

		udelay(eqos->reset_delays[2]);
	}

	if (data->ops->set_clock_selection)
		data->ops->set_clock_selection(dev, true);

	tx_delay = dev_read_u32_default(dev, "tx_delay", 0x30);
	rx_delay = dev_read_u32_default(dev, "rx_delay", 0x10);

	switch (pdata->phy_interface) {
	case PHY_INTERFACE_MODE_RGMII:
		return data->ops->set_to_rgmii(dev, tx_delay, rx_delay);
	case PHY_INTERFACE_MODE_RGMII_ID:
		return data->ops->set_to_rgmii(dev, 0, 0);
	case PHY_INTERFACE_MODE_RGMII_RXID:
		return data->ops->set_to_rgmii(dev, tx_delay, 0);
	case PHY_INTERFACE_MODE_RGMII_TXID:
		return data->ops->set_to_rgmii(dev, 0, rx_delay);
	case PHY_INTERFACE_MODE_RMII:
		return data->ops->set_to_rmii(dev);
	}

	return -EINVAL;
}

static int eqos_set_tx_clk_speed_rk(struct udevice *dev)
{
	struct eth_pdata *pdata = dev_get_plat(dev);
	struct rockchip_platform_data *data = pdata->priv_pdata;

	return data->ops->set_gmac_speed(dev);
}

static ulong eqos_get_tick_clk_rate_rk(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	return clk_get_rate(&eqos->clk_master_bus);
}

static struct eqos_ops eqos_rockchip_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_rk,
	.eqos_remove_resources = eqos_remove_resources_rk,
	.eqos_stop_resets = eqos_stop_resets_rk,
	.eqos_start_resets = eqos_start_resets_rk,
	.eqos_stop_clks = eqos_stop_clks_rk,
	.eqos_start_clks = eqos_start_clks_rk,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_set_tx_clk_speed_rk,
	.eqos_get_enetaddr = eqos_null_ops,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_rk,
};

struct eqos_config eqos_rockchip_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_100_150,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = dev_read_phy_mode,
	.ops = &eqos_rockchip_ops,
};
