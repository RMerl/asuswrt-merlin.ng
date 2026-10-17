// SPDX-License-Identifier: BSD-3-Clause
/*
 * Clock drivers for Qualcomm QCS404
 *
 * (C) Copyright 2022 Sumit Garg <sumit.garg@linaro.org>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <dt-bindings/clock/qcom,gcc-qcs404.h>

#include "clock-qcom.h"

/* Clocks: (from CLK_CTL_BASE)  */
#define GPLL0_STATUS			(0x21000)
#define GPLL1_STATUS			(0x20000)
#define APCS_GPLL_ENA_VOTE		(0x45000)
#define APCS_CLOCK_BRANCH_ENA_VOTE	(0x45004)

/* BLSP1 AHB clock (root clock for BLSP) */
#define BLSP1_AHB_CBCR			0x1008

/* Uart clock control registers */
#define BLSP1_UART2_BCR			(0x3028)
#define BLSP1_UART2_APPS_CBCR		(0x302C)
#define BLSP1_UART2_APPS_CMD_RCGR	(0x3034)
#define BLSP1_UART2_APPS_CFG_RCGR	(0x3038)
#define BLSP1_UART2_APPS_M		(0x303C)
#define BLSP1_UART2_APPS_N		(0x3040)
#define BLSP1_UART2_APPS_D		(0x3044)

/* I2C controller clock control registerss */
#define BLSP1_QUP0_I2C_APPS_CBCR	(0x6028)
#define BLSP1_QUP0_I2C_APPS_CMD_RCGR	(0x602C)
#define BLSP1_QUP0_I2C_APPS_CFG_RCGR	(0x6030)
#define BLSP1_QUP1_I2C_APPS_CBCR	(0x2008)
#define BLSP1_QUP1_I2C_APPS_CMD_RCGR	(0x200C)
#define BLSP1_QUP1_I2C_APPS_CFG_RCGR	(0x2010)
#define BLSP1_QUP2_I2C_APPS_CBCR	(0x3010)
#define BLSP1_QUP2_I2C_APPS_CMD_RCGR	(0x3000)
#define BLSP1_QUP2_I2C_APPS_CFG_RCGR	(0x3004)
#define BLSP1_QUP3_I2C_APPS_CBCR	(0x4020)
#define BLSP1_QUP3_I2C_APPS_CMD_RCGR	(0x4000)
#define BLSP1_QUP3_I2C_APPS_CFG_RCGR	(0x4004)
#define BLSP1_QUP4_I2C_APPS_CBCR	(0x5020)
#define BLSP1_QUP4_I2C_APPS_CMD_RCGR	(0x5000)
#define BLSP1_QUP4_I2C_APPS_CFG_RCGR	(0x5004)

/* SD controller clock control registers */
#define SDCC_BCR(n)			(((n) * 0x1000) + 0x41000)
#define SDCC_CMD_RCGR(n)		(((n) * 0x1000) + 0x41004)
#define SDCC_CFG_RCGR(n)		(((n) * 0x1000) + 0x41008)
#define SDCC_M(n)			(((n) * 0x1000) + 0x4100C)
#define SDCC_N(n)			(((n) * 0x1000) + 0x41010)
#define SDCC_D(n)			(((n) * 0x1000) + 0x41014)
#define SDCC_APPS_CBCR(n)		(((n) * 0x1000) + 0x41018)
#define SDCC_AHB_CBCR(n)		(((n) * 0x1000) + 0x4101C)

/* USB-3.0 controller clock control registers */
#define SYS_NOC_USB3_CBCR		(0x26014)
#define USB30_BCR			(0x39000)
#define USB3PHY_BCR			(0x39008)
#define USB30_MASTER_CBCR		(0x3900C)
#define USB30_SLEEP_CBCR		(0x39010)
#define USB30_MOCK_UTMI_CBCR		(0x39014)
#define USB30_MOCK_UTMI_CMD_RCGR	(0x3901C)
#define USB30_MOCK_UTMI_CFG_RCGR	(0x39020)
#define USB30_MASTER_CMD_RCGR		(0x39028)
#define USB30_MASTER_CFG_RCGR		(0x3902C)
#define USB30_MASTER_M			(0x39030)
#define USB30_MASTER_N			(0x39034)
#define USB30_MASTER_D			(0x39038)
#define USB2A_PHY_SLEEP_CBCR		(0x4102C)
#define USB_HS_PHY_CFG_AHB_CBCR		(0x41030)

/* ETH controller clock control registers */
#define ETH_PTP_CBCR			(0x4e004)
#define ETH_RGMII_CBCR			(0x4e008)
#define ETH_SLAVE_AHB_CBCR		(0x4e00c)
#define ETH_AXI_CBCR			(0x4e010)
#define EMAC_PTP_CMD_RCGR		(0x4e014)
#define EMAC_PTP_CFG_RCGR		(0x4e018)
#define EMAC_CMD_RCGR			(0x4e01c)
#define EMAC_CFG_RCGR			(0x4e020)
#define EMAC_M				(0x4e024)
#define EMAC_N				(0x4e028)
#define EMAC_D				(0x4e02c)


/* GPLL0 clock control registers */
#define GPLL0_STATUS_ACTIVE BIT(31)

#define CFG_CLK_SRC_GPLL1	BIT(8)
#define GPLL1_STATUS_ACTIVE	BIT(31)

static struct vote_clk gcc_blsp1_ahb_clk = {
	.cbcr_reg = BLSP1_AHB_CBCR,
	.ena_vote = APCS_CLOCK_BRANCH_ENA_VOTE,
	.vote_bit = BIT(10) | BIT(5) | BIT(4),
};

static const struct bcr_regs uart2_regs = {
	.cfg_rcgr = BLSP1_UART2_APPS_CFG_RCGR,
	.cmd_rcgr = BLSP1_UART2_APPS_CMD_RCGR,
	.M = BLSP1_UART2_APPS_M,
	.N = BLSP1_UART2_APPS_N,
	.D = BLSP1_UART2_APPS_D,
};

static const struct bcr_regs sdc_regs = {
	.cfg_rcgr = SDCC_CFG_RCGR(1),
	.cmd_rcgr = SDCC_CMD_RCGR(1),
	.M = SDCC_M(1),
	.N = SDCC_N(1),
	.D = SDCC_D(1),
};

static struct pll_vote_clk gpll0_vote_clk = {
	.status = GPLL0_STATUS,
	.status_bit = GPLL0_STATUS_ACTIVE,
	.ena_vote = APCS_GPLL_ENA_VOTE,
	.vote_bit = BIT(0),
};

static struct pll_vote_clk gpll1_vote_clk = {
	.status = GPLL1_STATUS,
	.status_bit = GPLL1_STATUS_ACTIVE,
	.ena_vote = APCS_GPLL_ENA_VOTE,
	.vote_bit = BIT(1),
};

static const struct bcr_regs usb30_master_regs = {
	.cfg_rcgr = USB30_MASTER_CFG_RCGR,
	.cmd_rcgr = USB30_MASTER_CMD_RCGR,
	.M = USB30_MASTER_M,
	.N = USB30_MASTER_N,
	.D = USB30_MASTER_D,
};

static const struct bcr_regs emac_regs = {
	.cfg_rcgr = EMAC_CFG_RCGR,
	.cmd_rcgr = EMAC_CMD_RCGR,
	.M = EMAC_M,
	.N = EMAC_N,
	.D = EMAC_D,
};

static const struct bcr_regs emac_ptp_regs = {
	.cfg_rcgr = EMAC_PTP_CFG_RCGR,
	.cmd_rcgr = EMAC_PTP_CMD_RCGR,
	.M = EMAC_M,
	.N = EMAC_N,
	.D = EMAC_D,
};

static const struct bcr_regs blsp1_qup0_i2c_apps_regs = {
	.cmd_rcgr = BLSP1_QUP0_I2C_APPS_CMD_RCGR,
	.cfg_rcgr = BLSP1_QUP0_I2C_APPS_CFG_RCGR,
	/* mnd_width = 0 */
};

static const struct bcr_regs blsp1_qup1_i2c_apps_regs = {
	.cmd_rcgr = BLSP1_QUP1_I2C_APPS_CMD_RCGR,
	.cfg_rcgr = BLSP1_QUP1_I2C_APPS_CFG_RCGR,
	/* mnd_width = 0 */
};

static const struct bcr_regs blsp1_qup2_i2c_apps_regs = {
	.cmd_rcgr = BLSP1_QUP2_I2C_APPS_CMD_RCGR,
	.cfg_rcgr = BLSP1_QUP2_I2C_APPS_CFG_RCGR,
	/* mnd_width = 0 */
};

static const struct bcr_regs blsp1_qup3_i2c_apps_regs = {
	.cmd_rcgr = BLSP1_QUP3_I2C_APPS_CMD_RCGR,
	.cfg_rcgr = BLSP1_QUP3_I2C_APPS_CFG_RCGR,
	/* mnd_width = 0 */
};

static const struct bcr_regs blsp1_qup4_i2c_apps_regs = {
	.cmd_rcgr = BLSP1_QUP4_I2C_APPS_CMD_RCGR,
	.cfg_rcgr = BLSP1_QUP4_I2C_APPS_CFG_RCGR,
	/* mnd_width = 0 */
};

static ulong qcs404_clk_set_rate(struct clk *clk, ulong rate)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case GCC_BLSP1_UART2_APPS_CLK:
		/* UART: 115200 */
		clk_rcg_set_rate_mnd(priv->base, &uart2_regs, 0, 12, 125,
				     CFG_CLK_SRC_CXO, 16);
		clk_enable_cbc(priv->base + BLSP1_UART2_APPS_CBCR);
		break;
	case GCC_BLSP1_AHB_CLK:
		clk_enable_vote_clk(priv->base, &gcc_blsp1_ahb_clk);
		break;
	case GCC_SDCC1_APPS_CLK:
		/* SDCC1: 200MHz */
		clk_rcg_set_rate_mnd(priv->base, &sdc_regs, 7, 0, 0,
				     CFG_CLK_SRC_GPLL0, 8);
		clk_enable_gpll0(priv->base, &gpll0_vote_clk);
		clk_enable_cbc(priv->base + SDCC_APPS_CBCR(1));
		break;
	case GCC_SDCC1_AHB_CLK:
		clk_enable_cbc(priv->base + SDCC_AHB_CBCR(1));
		break;
	case GCC_ETH_RGMII_CLK:
		if (rate == 250000000)
			clk_rcg_set_rate_mnd(priv->base, &emac_regs, 3, 0, 0,
					     CFG_CLK_SRC_GPLL1, 8);
		else if (rate == 125000000)
			clk_rcg_set_rate_mnd(priv->base, &emac_regs, 7, 0, 0,
					     CFG_CLK_SRC_GPLL1, 8);
		else if (rate == 50000000)
			clk_rcg_set_rate_mnd(priv->base, &emac_regs, 19, 0, 0,
					     CFG_CLK_SRC_GPLL1, 8);
		else if (rate == 5000000)
			clk_rcg_set_rate_mnd(priv->base, &emac_regs, 3, 1, 50,
					     CFG_CLK_SRC_GPLL1, 8);
		break;
	default:
		return 0;
	}

	return 0;
}

static int qcs404_clk_enable(struct clk *clk)
{
	struct msm_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case GCC_USB30_MASTER_CLK:
		clk_enable_cbc(priv->base + USB30_MASTER_CBCR);
		clk_rcg_set_rate_mnd(priv->base, &usb30_master_regs, 7, 0, 0,
				     CFG_CLK_SRC_GPLL0, 8);
		break;
	case GCC_SYS_NOC_USB3_CLK:
		clk_enable_cbc(priv->base + SYS_NOC_USB3_CBCR);
		break;
	case GCC_USB30_SLEEP_CLK:
		clk_enable_cbc(priv->base + USB30_SLEEP_CBCR);
		break;
	case GCC_USB30_MOCK_UTMI_CLK:
		clk_enable_cbc(priv->base + USB30_MOCK_UTMI_CBCR);
		break;
	case GCC_USB_HS_PHY_CFG_AHB_CLK:
		clk_enable_cbc(priv->base + USB_HS_PHY_CFG_AHB_CBCR);
		break;
	case GCC_USB2A_PHY_SLEEP_CLK:
		clk_enable_cbc(priv->base + USB_HS_PHY_CFG_AHB_CBCR);
		break;
	case GCC_ETH_PTP_CLK:
		/* SPEED_1000: freq -> 250MHz */
		clk_enable_cbc(priv->base + ETH_PTP_CBCR);
		clk_enable_gpll0(priv->base, &gpll1_vote_clk);
		clk_rcg_set_rate_mnd(priv->base, &emac_ptp_regs, 3, 0, 0,
				     CFG_CLK_SRC_GPLL1, 8);
		break;
	case GCC_ETH_RGMII_CLK:
		/* SPEED_1000: freq -> 250MHz */
		clk_enable_cbc(priv->base + ETH_RGMII_CBCR);
		clk_enable_gpll0(priv->base, &gpll1_vote_clk);
		clk_rcg_set_rate_mnd(priv->base, &emac_regs, 3, 0, 0,
				     CFG_CLK_SRC_GPLL1, 8);
		break;
	case GCC_ETH_SLAVE_AHB_CLK:
		clk_enable_cbc(priv->base + ETH_SLAVE_AHB_CBCR);
		break;
	case GCC_ETH_AXI_CLK:
		clk_enable_cbc(priv->base + ETH_AXI_CBCR);
		break;
	case GCC_BLSP1_AHB_CLK:
		clk_enable_vote_clk(priv->base, &gcc_blsp1_ahb_clk);
		break;
	case GCC_BLSP1_QUP0_I2C_APPS_CLK:
		clk_enable_cbc(priv->base + BLSP1_QUP0_I2C_APPS_CBCR);
		clk_rcg_set_rate(priv->base, &blsp1_qup0_i2c_apps_regs, 0,
				 CFG_CLK_SRC_CXO);
		break;
	case GCC_BLSP1_QUP1_I2C_APPS_CLK:
		clk_enable_cbc(priv->base + BLSP1_QUP1_I2C_APPS_CBCR);
		clk_rcg_set_rate(priv->base, &blsp1_qup1_i2c_apps_regs, 0,
				 CFG_CLK_SRC_CXO);
		break;
	case GCC_BLSP1_QUP2_I2C_APPS_CLK:
		clk_enable_cbc(priv->base + BLSP1_QUP2_I2C_APPS_CBCR);
		clk_rcg_set_rate(priv->base, &blsp1_qup2_i2c_apps_regs, 0,
				 CFG_CLK_SRC_CXO);
		break;
	case GCC_BLSP1_QUP3_I2C_APPS_CLK:
		clk_enable_cbc(priv->base + BLSP1_QUP3_I2C_APPS_CBCR);
		clk_rcg_set_rate(priv->base, &blsp1_qup3_i2c_apps_regs, 0,
				 CFG_CLK_SRC_CXO);
		break;
	case GCC_BLSP1_QUP4_I2C_APPS_CLK:
		clk_enable_cbc(priv->base + BLSP1_QUP4_I2C_APPS_CBCR);
		clk_rcg_set_rate(priv->base, &blsp1_qup4_i2c_apps_regs, 0,
				 CFG_CLK_SRC_CXO);
		break;
	default:
		return 0;
	}

	return 0;
}

static const struct qcom_reset_map qcs404_gcc_resets[] = {
	[GCC_GENI_IR_BCR] = { 0x0F000 },
	[GCC_CDSP_RESTART] = { 0x18000 },
	[GCC_USB_HS_BCR] = { 0x41000 },
	[GCC_USB2_HS_PHY_ONLY_BCR] = { 0x41034 },
	[GCC_QUSB2_PHY_BCR] = { 0x4103c },
	[GCC_USB_HS_PHY_CFG_AHB_BCR] = { 0x0000c, 1 },
	[GCC_USB2A_PHY_BCR] = { 0x0000c, 0 },
	[GCC_USB3_PHY_BCR] = { 0x39004 },
	[GCC_USB_30_BCR] = { 0x39000 },
	[GCC_USB3PHY_PHY_BCR] = { 0x39008 },
	[GCC_PCIE_0_BCR] = { 0x3e000 },
	[GCC_PCIE_0_PHY_BCR] = { 0x3e004 },
	[GCC_PCIE_0_LINK_DOWN_BCR] = { 0x3e038 },
	[GCC_PCIEPHY_0_PHY_BCR] = { 0x3e03c },
	[GCC_PCIE_0_AXI_MASTER_STICKY_ARES] = { 0x3e040, 6},
	[GCC_PCIE_0_AHB_ARES] = { 0x3e040, 5 },
	[GCC_PCIE_0_AXI_SLAVE_ARES] = { 0x3e040, 4 },
	[GCC_PCIE_0_AXI_MASTER_ARES] = { 0x3e040, 3 },
	[GCC_PCIE_0_CORE_STICKY_ARES] = { 0x3e040, 2 },
	[GCC_PCIE_0_SLEEP_ARES] = { 0x3e040, 1 },
	[GCC_PCIE_0_PIPE_ARES] = { 0x3e040, 0 },
	[GCC_EMAC_BCR] = { 0x4e000 },
	[GCC_WDSP_RESTART] = {0x19000},
};

static const struct msm_clk_data qcs404_clk_gcc_data = {
	.resets = qcs404_gcc_resets,
	.num_resets = ARRAY_SIZE(qcs404_gcc_resets),
	.enable = qcs404_clk_enable,
	.set_rate = qcs404_clk_set_rate,
};

static const struct udevice_id gcc_qcs404_of_match[] = {
	{
		.compatible = "qcom,gcc-qcs404",
		.data = (ulong)&qcs404_clk_gcc_data
	},
	{ }
};

U_BOOT_DRIVER(gcc_qcs404) = {
	.name		= "gcc_qcs404",
	.id		= UCLASS_NOP,
	.of_match	= gcc_qcs404_of_match,
	.bind		= qcom_cc_bind,
	.flags		= DM_FLAG_PRE_RELOC,
};
