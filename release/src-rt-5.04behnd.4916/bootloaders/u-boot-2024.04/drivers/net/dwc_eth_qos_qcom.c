// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022-2023 Sumit Garg <sumit.garg@linaro.org>
 *
 * Qcom DWMAC specific glue layer
 */

#include <common.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <phy.h>
#include <reset.h>
#include <syscon.h>
#include <linux/bitops.h>
#include <linux/delay.h>

#include "dwc_eth_qos.h"

/* RGMII_IO_MACRO_CONFIG fields */
#define RGMII_CONFIG_FUNC_CLK_EN		BIT(30)
#define RGMII_CONFIG_POS_NEG_DATA_SEL		BIT(23)
#define RGMII_CONFIG_GPIO_CFG_RX_INT		GENMASK(21, 20)
#define RGMII_CONFIG_GPIO_CFG_TX_INT		GENMASK(19, 17)
#define RGMII_CONFIG_MAX_SPD_PRG_9		GENMASK(16, 8)
#define RGMII_CONFIG_MAX_SPD_PRG_2		GENMASK(7, 6)
#define RGMII_CONFIG_INTF_SEL			GENMASK(5, 4)
#define RGMII_CONFIG_BYPASS_TX_ID_EN		BIT(3)
#define RGMII_CONFIG_LOOPBACK_EN		BIT(2)
#define RGMII_CONFIG_PROG_SWAP			BIT(1)
#define RGMII_CONFIG_DDR_MODE			BIT(0)

/* SDCC_HC_REG_DLL_CONFIG fields */
#define SDCC_DLL_CONFIG_DLL_RST			BIT(30)
#define SDCC_DLL_CONFIG_PDN			BIT(29)
#define SDCC_DLL_CONFIG_MCLK_FREQ		GENMASK(26, 24)
#define SDCC_DLL_CONFIG_CDR_SELEXT		GENMASK(23, 20)
#define SDCC_DLL_CONFIG_CDR_EXT_EN		BIT(19)
#define SDCC_DLL_CONFIG_CK_OUT_EN		BIT(18)
#define SDCC_DLL_CONFIG_CDR_EN			BIT(17)
#define SDCC_DLL_CONFIG_DLL_EN			BIT(16)
#define SDCC_DLL_MCLK_GATING_EN			BIT(5)
#define SDCC_DLL_CDR_FINE_PHASE			GENMASK(3, 2)

/* SDCC_HC_REG_DDR_CONFIG fields */
#define SDCC_DDR_CONFIG_PRG_DLY_EN		BIT(31)
#define SDCC_DDR_CONFIG_EXT_PRG_RCLK_DLY	GENMASK(26, 21)
#define SDCC_DDR_CONFIG_EXT_PRG_RCLK_DLY_CODE	GENMASK(29, 27)
#define SDCC_DDR_CONFIG_EXT_PRG_RCLK_DLY_EN	BIT(30)
#define SDCC_DDR_CONFIG_PRG_RCLK_DLY		GENMASK(8, 0)

/* SDCC_HC_REG_DLL_CONFIG2 fields */
#define SDCC_DLL_CONFIG2_DLL_CLOCK_DIS		BIT(21)
#define SDCC_DLL_CONFIG2_MCLK_FREQ_CALC		GENMASK(17, 10)
#define SDCC_DLL_CONFIG2_DDR_TRAFFIC_INIT_SEL	GENMASK(3, 2)
#define SDCC_DLL_CONFIG2_DDR_TRAFFIC_INIT_SW	BIT(1)
#define SDCC_DLL_CONFIG2_DDR_CAL_EN		BIT(0)

/* SDC4_STATUS bits */
#define SDC4_STATUS_DLL_LOCK			BIT(7)

/* RGMII_IO_MACRO_CONFIG2 fields */
#define RGMII_CONFIG2_RSVD_CONFIG15		GENMASK(31, 17)
#define RGMII_CONFIG2_RGMII_CLK_SEL_CFG		BIT(16)
#define RGMII_CONFIG2_TX_TO_RX_LOOPBACK_EN	BIT(13)
#define RGMII_CONFIG2_CLK_DIVIDE_SEL		BIT(12)
#define RGMII_CONFIG2_RX_PROG_SWAP		BIT(7)
#define RGMII_CONFIG2_DATA_DIVIDE_CLK_SEL	BIT(6)
#define RGMII_CONFIG2_TX_CLK_PHASE_SHIFT_EN	BIT(5)

struct dwmac_rgmii_regs {
	u32 io_macro_config;		/* 0x00 */
	u32 sdcc_hc_dll_config;		/* 0x04 */
	u32 reserved_1;			/* 0x08 */
	u32 sdcc_hc_ddr_config;		/* 0x0c */
	u32 sdcc_hc_dll_config2;	/* 0x10 */
	u32 sdc4_status;		/* 0x14 */
	u32 sdcc_usr_ctl;		/* 0x18 */
	u32 io_macro_config2;		/* 0x1c */
	u32 io_macro_debug1;		/* 0x20 */
	u32 reserved_2;			/* 0x24 */
	u32 emac_sys_low_power_dbg;	/* 0x28 */
	u32 reserved_3[53];		/* upto 0x100 */
};

static struct dwmac_rgmii_regs emac_v2_3_0_por = {
	.io_macro_config = 0x00C01343,
	.sdcc_hc_dll_config = 0x2004642C,
	.sdcc_hc_ddr_config = 0x00000000,
	.sdcc_hc_dll_config2 = 0x00200000,
	.sdcc_usr_ctl = 0x00010800,
	.io_macro_config2 = 0x00002060
};

static void ethqos_set_func_clk_en(struct dwmac_rgmii_regs *regs)
{
	setbits_le32(&regs->io_macro_config, RGMII_CONFIG_FUNC_CLK_EN);
}

static int ethqos_dll_configure(struct udevice *dev,
				struct dwmac_rgmii_regs *regs)
{
	unsigned int val;
	int retry = 1000;

	/* Set CDR_EN */
	setbits_le32(&regs->sdcc_hc_dll_config, SDCC_DLL_CONFIG_CDR_EN);

	/* Set CDR_EXT_EN */
	setbits_le32(&regs->sdcc_hc_dll_config, SDCC_DLL_CONFIG_CDR_EXT_EN);

	/* Clear CK_OUT_EN */
	clrbits_le32(&regs->sdcc_hc_dll_config, SDCC_DLL_CONFIG_CK_OUT_EN);

	/* Set DLL_EN */
	setbits_le32(&regs->sdcc_hc_dll_config, SDCC_DLL_CONFIG_DLL_EN);

	clrbits_le32(&regs->sdcc_hc_dll_config, SDCC_DLL_MCLK_GATING_EN);

	clrbits_le32(&regs->sdcc_hc_dll_config, SDCC_DLL_CDR_FINE_PHASE);

	/* Wait for CK_OUT_EN clear */
	do {
		val = readl(&regs->sdcc_hc_dll_config);
		val &= SDCC_DLL_CONFIG_CK_OUT_EN;
		if (!val)
			break;
		mdelay(1);
		retry--;
	} while (retry > 0);
	if (!retry)
		dev_err(dev, "Clear CK_OUT_EN timedout\n");

	/* Set CK_OUT_EN */
	setbits_le32(&regs->sdcc_hc_dll_config, SDCC_DLL_CONFIG_CK_OUT_EN);

	/* Wait for CK_OUT_EN set */
	retry = 1000;
	do {
		val = readl(&regs->sdcc_hc_dll_config);
		val &= SDCC_DLL_CONFIG_CK_OUT_EN;
		if (val)
			break;
		mdelay(1);
		retry--;
	} while (retry > 0);
	if (!retry)
		dev_err(dev, "Set CK_OUT_EN timedout\n");

	/* Set DDR_CAL_EN */
	setbits_le32(&regs->sdcc_hc_dll_config2, SDCC_DLL_CONFIG2_DDR_CAL_EN);

	clrbits_le32(&regs->sdcc_hc_dll_config2,
		     SDCC_DLL_CONFIG2_DLL_CLOCK_DIS);

	clrsetbits_le32(&regs->sdcc_hc_dll_config2,
			SDCC_DLL_CONFIG2_MCLK_FREQ_CALC, 0x1A << 10);

	clrsetbits_le32(&regs->sdcc_hc_dll_config2,
			SDCC_DLL_CONFIG2_DDR_TRAFFIC_INIT_SEL, BIT(2));

	setbits_le32(&regs->sdcc_hc_dll_config2,
		     SDCC_DLL_CONFIG2_DDR_TRAFFIC_INIT_SW);

	return 0;
}

static int ethqos_rgmii_macro_init(struct udevice *dev,
				   struct dwmac_rgmii_regs *regs,
				   unsigned long speed)
{
	/* Disable loopback mode */
	clrbits_le32(&regs->io_macro_config2,
		     RGMII_CONFIG2_TX_TO_RX_LOOPBACK_EN);

	/* Select RGMII, write 0 to interface select */
	clrbits_le32(&regs->io_macro_config, RGMII_CONFIG_INTF_SEL);

	switch (speed) {
	case SPEED_1000:
		setbits_le32(&regs->io_macro_config, RGMII_CONFIG_DDR_MODE);
		clrbits_le32(&regs->io_macro_config,
			     RGMII_CONFIG_BYPASS_TX_ID_EN);
		setbits_le32(&regs->io_macro_config,
			     RGMII_CONFIG_POS_NEG_DATA_SEL);
		setbits_le32(&regs->io_macro_config, RGMII_CONFIG_PROG_SWAP);

		clrbits_le32(&regs->io_macro_config2,
			     RGMII_CONFIG2_DATA_DIVIDE_CLK_SEL);
		setbits_le32(&regs->io_macro_config2,
			     RGMII_CONFIG2_TX_CLK_PHASE_SHIFT_EN);
		clrbits_le32(&regs->io_macro_config2,
			     RGMII_CONFIG2_RSVD_CONFIG15);
		setbits_le32(&regs->io_macro_config2,
			     RGMII_CONFIG2_RX_PROG_SWAP);

		/* Set PRG_RCLK_DLY to 57 for 1.8 ns delay */
		clrsetbits_le32(&regs->sdcc_hc_ddr_config,
				SDCC_DDR_CONFIG_PRG_RCLK_DLY, 57);
		setbits_le32(&regs->sdcc_hc_ddr_config, SDCC_DDR_CONFIG_PRG_DLY_EN);

		setbits_le32(&regs->io_macro_config, RGMII_CONFIG_LOOPBACK_EN);
		break;

	case SPEED_100:
		setbits_le32(&regs->io_macro_config, RGMII_CONFIG_DDR_MODE);
		setbits_le32(&regs->io_macro_config,
			     RGMII_CONFIG_BYPASS_TX_ID_EN);
		clrbits_le32(&regs->io_macro_config,
			     RGMII_CONFIG_POS_NEG_DATA_SEL);
		clrbits_le32(&regs->io_macro_config, RGMII_CONFIG_PROG_SWAP);
		clrsetbits_le32(&regs->io_macro_config,
				RGMII_CONFIG_MAX_SPD_PRG_2, BIT(6));

		clrbits_le32(&regs->io_macro_config2,
			     RGMII_CONFIG2_DATA_DIVIDE_CLK_SEL);
		setbits_le32(&regs->io_macro_config2,
			     RGMII_CONFIG2_TX_CLK_PHASE_SHIFT_EN);
		clrbits_le32(&regs->io_macro_config2,
			     RGMII_CONFIG2_RSVD_CONFIG15);
		clrbits_le32(&regs->io_macro_config2,
			     RGMII_CONFIG2_RX_PROG_SWAP);

		/* Write 0x5 to PRG_RCLK_DLY_CODE */
		clrsetbits_le32(&regs->sdcc_hc_ddr_config,
				SDCC_DDR_CONFIG_EXT_PRG_RCLK_DLY_CODE,
				(BIT(29) | BIT(27)));
		setbits_le32(&regs->sdcc_hc_ddr_config,
			     SDCC_DDR_CONFIG_EXT_PRG_RCLK_DLY);
		setbits_le32(&regs->sdcc_hc_ddr_config,
			     SDCC_DDR_CONFIG_EXT_PRG_RCLK_DLY_EN);

		setbits_le32(&regs->io_macro_config, RGMII_CONFIG_LOOPBACK_EN);
		break;

	case SPEED_10:
		setbits_le32(&regs->io_macro_config, RGMII_CONFIG_DDR_MODE);
		setbits_le32(&regs->io_macro_config,
			     RGMII_CONFIG_BYPASS_TX_ID_EN);
		clrbits_le32(&regs->io_macro_config,
			     RGMII_CONFIG_POS_NEG_DATA_SEL);
		clrbits_le32(&regs->io_macro_config, RGMII_CONFIG_PROG_SWAP);
		clrsetbits_le32(&regs->io_macro_config,
				RGMII_CONFIG_MAX_SPD_PRG_9,
				BIT(12) | GENMASK(9, 8));

		clrbits_le32(&regs->io_macro_config2,
			     RGMII_CONFIG2_DATA_DIVIDE_CLK_SEL);
		clrbits_le32(&regs->io_macro_config2,
			     RGMII_CONFIG2_TX_CLK_PHASE_SHIFT_EN);
		clrbits_le32(&regs->io_macro_config2,
			     RGMII_CONFIG2_RSVD_CONFIG15);
		clrbits_le32(&regs->io_macro_config2,
			     RGMII_CONFIG2_RX_PROG_SWAP);

		/* Write 0x5 to PRG_RCLK_DLY_CODE */
		clrsetbits_le32(&regs->sdcc_hc_ddr_config,
				SDCC_DDR_CONFIG_EXT_PRG_RCLK_DLY_CODE,
				(BIT(29) | BIT(27)));
		setbits_le32(&regs->sdcc_hc_ddr_config,
			     SDCC_DDR_CONFIG_EXT_PRG_RCLK_DLY);
		setbits_le32(&regs->sdcc_hc_ddr_config,
			     SDCC_DDR_CONFIG_EXT_PRG_RCLK_DLY_EN);

		setbits_le32(&regs->io_macro_config, RGMII_CONFIG_LOOPBACK_EN);
		break;

	default:
		dev_err(dev, "Invalid speed %ld\n", speed);
		return -EINVAL;
	}

	return 0;
}

static int ethqos_configure(struct udevice *dev,
			    struct dwmac_rgmii_regs *regs,
			    unsigned long speed)
{
	unsigned int retry = 1000;

	/* Reset to POR values and enable clk */
	writel(emac_v2_3_0_por.io_macro_config, &regs->io_macro_config);
	writel(emac_v2_3_0_por.sdcc_hc_dll_config, &regs->sdcc_hc_dll_config);
	writel(emac_v2_3_0_por.sdcc_hc_ddr_config, &regs->sdcc_hc_ddr_config);
	writel(emac_v2_3_0_por.sdcc_hc_dll_config2, &regs->sdcc_hc_dll_config2);
	writel(emac_v2_3_0_por.sdcc_usr_ctl, &regs->sdcc_usr_ctl);
	writel(emac_v2_3_0_por.io_macro_config2, &regs->io_macro_config2);

	ethqos_set_func_clk_en(regs);

	/* Initialize the DLL first */

	/* Set DLL_RST */
	setbits_le32(&regs->sdcc_hc_dll_config, SDCC_DLL_CONFIG_DLL_RST);

	/* Set PDN */
	setbits_le32(&regs->sdcc_hc_dll_config, SDCC_DLL_CONFIG_PDN);

	/* Clear DLL_RST */
	clrbits_le32(&regs->sdcc_hc_dll_config, SDCC_DLL_CONFIG_DLL_RST);

	/* Clear PDN */
	clrbits_le32(&regs->sdcc_hc_dll_config, SDCC_DLL_CONFIG_PDN);

	if (speed == SPEED_1000) {
		/* Set DLL_EN */
		setbits_le32(&regs->sdcc_hc_dll_config, SDCC_DLL_CONFIG_DLL_EN);

		/* Set CK_OUT_EN */
		setbits_le32(&regs->sdcc_hc_dll_config,
			     SDCC_DLL_CONFIG_CK_OUT_EN);

		/* Set USR_CTL bit 26 with mask of 3 bits */
		clrsetbits_le32(&regs->sdcc_usr_ctl, GENMASK(26, 24), BIT(26));

		/* wait for DLL LOCK */
		do {
			mdelay(1);
			if (readl(&regs->sdc4_status) & SDC4_STATUS_DLL_LOCK)
				break;
			retry--;
		} while (retry > 0);
		if (!retry)
			dev_err(dev, "Timeout while waiting for DLL lock\n");

		ethqos_dll_configure(dev, regs);
	}

	ethqos_rgmii_macro_init(dev, regs, speed);

	return 0;
}

static void ethqos_rgmii_dump(struct udevice *dev,
			      struct dwmac_rgmii_regs *regs)
{
	dev_dbg(dev, "Rgmii register dump\n");
	dev_dbg(dev, "RGMII_IO_MACRO_CONFIG: %08x\n",
		readl(&regs->io_macro_config));
	dev_dbg(dev, "SDCC_HC_REG_DLL_CONFIG: %08x\n",
		readl(&regs->sdcc_hc_dll_config));
	dev_dbg(dev, "SDCC_HC_REG_DDR_CONFIG: %08x\n",
		readl(&regs->sdcc_hc_ddr_config));
	dev_dbg(dev, "SDCC_HC_REG_DLL_CONFIG2: %08x\n",
		readl(&regs->sdcc_hc_dll_config2));
	dev_dbg(dev, "SDC4_STATUS: %08x\n",
		readl(&regs->sdc4_status));
	dev_dbg(dev, "SDCC_USR_CTL: %08x\n",
		readl(&regs->sdcc_usr_ctl));
	dev_dbg(dev, "RGMII_IO_MACRO_CONFIG2: %08x\n",
		readl(&regs->io_macro_config2));
	dev_dbg(dev, "RGMII_IO_MACRO_DEBUG1: %08x\n",
		readl(&regs->io_macro_debug1));
	dev_dbg(dev, "EMAC_SYSTEM_LOW_POWER_DEBUG: %08x\n",
		readl(&regs->emac_sys_low_power_dbg));
}

static int qcom_eqos_rgmii_set_speed(struct udevice *dev,
				     void *rgmii_regs,
				     unsigned long speed)
{
	int ret;

	ethqos_rgmii_dump(dev, rgmii_regs);

	ret = ethqos_configure(dev, rgmii_regs, speed);
	if (ret)
		return ret;

	ethqos_rgmii_dump(dev, rgmii_regs);

	return 0;
}

static int qcom_eqos_rgmii_reset(struct udevice *dev, void *rgmii_regs)
{
	ethqos_set_func_clk_en(rgmii_regs);

	return 0;
}

static int eqos_start_clks_qcom(struct udevice *dev)
{
	if (IS_ENABLED(CONFIG_CLK)) {
		struct clk_bulk clocks;
		int ret;

		ret = clk_get_bulk(dev, &clocks);
		if (ret)
			return ret;

		ret = clk_enable_bulk(&clocks);
		if (ret)
			return ret;
	}

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_stop_clks_qcom(struct udevice *dev)
{
	if (IS_ENABLED(CONFIG_CLK)) {
		struct clk_bulk clocks;
		int ret;

		ret = clk_get_bulk(dev, &clocks);
		if (ret)
			return ret;

		ret = clk_disable_bulk(&clocks);
		if (ret)
			return ret;
	}

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_start_resets_qcom(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	if (!eqos->phy) {
		ret = dm_gpio_set_value(&eqos->phy_reset_gpio, 0);
		if (ret < 0) {
			pr_err("dm_gpio_set_value(phy_reset, assert) failed: %d", ret);
			return ret;
		}

		udelay(eqos->reset_delays[0]);

		ret = dm_gpio_set_value(&eqos->phy_reset_gpio, 1);
		if (ret < 0) {
			pr_err("dm_gpio_set_value(phy_reset, deassert) failed: %d", ret);
			return ret;
		}

		udelay(eqos->reset_delays[1]);

		ret = dm_gpio_set_value(&eqos->phy_reset_gpio, 0);
		if (ret < 0) {
			pr_err("dm_gpio_set_value(phy_reset, deassert) failed: %d", ret);
			return ret;
		}

		udelay(eqos->reset_delays[2]);
	}

	ret = reset_deassert(&eqos->reset_ctl);
	if (ret < 0) {
		pr_err("reset_deassert() failed: %d", ret);
		return ret;
	}

	ret = qcom_eqos_rgmii_reset(dev, eqos->eqos_qcom_rgmii_regs);
	if (ret < 0) {
		pr_err("qcom rgmii_reset failed: %d", ret);
		return ret;
	}

	debug("%s: OK\n", __func__);
	return 0;
}

/* Clock rates */
#define RGMII_1000_NOM_CLK_FREQ			(250 * 1000 * 1000UL)
#define RGMII_ID_MODE_100_LOW_SVS_CLK_FREQ	 (50 * 1000 * 1000UL)
#define RGMII_ID_MODE_10_LOW_SVS_CLK_FREQ	  (5 * 1000 * 1000UL)

static int eqos_set_tx_clk_speed_qcom(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	ulong rate;
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	switch (eqos->phy->speed) {
	case SPEED_1000:
		rate = RGMII_1000_NOM_CLK_FREQ;
		break;
	case SPEED_100:
		rate = RGMII_ID_MODE_100_LOW_SVS_CLK_FREQ;
		break;
	case SPEED_10:
		rate = RGMII_ID_MODE_10_LOW_SVS_CLK_FREQ;
		break;
	default:
		pr_err("invalid speed %d", eqos->phy->speed);
		return -EINVAL;
	}

	ret = clk_set_rate(&eqos->clk_tx, rate);
	if (ret < 0) {
		pr_err("clk_set_rate(tx_clk, %lu) failed: %d", rate, ret);
		return ret;
	}

	ret = qcom_eqos_rgmii_set_speed(dev, eqos->eqos_qcom_rgmii_regs,
					eqos->phy->speed);
	if (ret < 0) {
		pr_err("qcom set_speed: %d, failed: %d", eqos->phy->speed, ret);
		return ret;
	}

	return 0;
}

static int eqos_probe_resources_qcom(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	phy_interface_t interface;
	int reset_flags = GPIOD_IS_OUT;
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	interface = eqos->config->interface(dev);

	if (interface == PHY_INTERFACE_MODE_NA) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	eqos->max_speed = dev_read_u32_default(dev, "max-speed", 0);

	eqos->tx_fifo_sz = dev_read_u32_default(dev, "tx-fifo-depth", 0);
	eqos->rx_fifo_sz = dev_read_u32_default(dev, "rx-fifo-depth", 0);

	ret = reset_get_by_name(dev, "emac", &eqos->reset_ctl);
	if (ret) {
		pr_err("reset_get_by_name(rst) failed: %d", ret);
		return ret;
	}

	if (dev_read_bool(dev, "snps,reset-active-low"))
		reset_flags |= GPIOD_ACTIVE_LOW;

	ret = gpio_request_by_name(dev, "snps,reset-gpio", 0,
				   &eqos->phy_reset_gpio, reset_flags);
	if (ret == 0) {
		ret = dev_read_u32_array(dev, "snps,reset-delays-us",
					 eqos->reset_delays, 3);
	} else if (ret == -ENOENT) {
		ret = 0;
	}

	eqos->eqos_qcom_rgmii_regs = (void *)dev_read_addr_name(dev, "rgmii");
	if ((fdt_addr_t)eqos->eqos_qcom_rgmii_regs == FDT_ADDR_T_NONE) {
		pr_err("Invalid RGMII address\n");
		return -EINVAL;
	}

	ret = clk_get_by_name(dev, "rgmii", &eqos->clk_tx);
	if (ret) {
		pr_err("clk_get_by_name(tx) failed: %d", ret);
		return -EINVAL;
	}

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_remove_resources_qcom(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	dm_gpio_free(dev, &eqos->phy_reset_gpio);
	reset_free(&eqos->reset_ctl);

	debug("%s: OK\n", __func__);
	return 0;
}

static struct eqos_ops eqos_qcom_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_qcom,
	.eqos_remove_resources = eqos_remove_resources_qcom,
	.eqos_stop_resets = eqos_null_ops,
	.eqos_start_resets = eqos_start_resets_qcom,
	.eqos_stop_clks = eqos_stop_clks_qcom,
	.eqos_start_clks = eqos_start_clks_qcom,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_set_tx_clk_speed_qcom,
	.eqos_get_enetaddr = eqos_null_ops,
};

struct eqos_config __maybe_unused eqos_qcom_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_250_300,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = dev_read_phy_mode,
	.ops = &eqos_qcom_ops
};
