// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2018 NXP
 * Peng Fan <peng.fan@nxp.com>
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/arch/sci/sci.h>
#include <asm/arch/clock.h>
#include <dt-bindings/clock/imx8qxp-clock.h>
#include <dt-bindings/soc/imx_rsrc.h>
#include <misc.h>

#include "clk-imx8.h"

#if CONFIG_IS_ENABLED(CMD_CLK)
struct imx8_clks imx8_clk_names[] = {
	{ IMX8QXP_A35_DIV, "A35_DIV" },
	{ IMX8QXP_I2C0_CLK, "I2C0" },
	{ IMX8QXP_I2C1_CLK, "I2C1" },
	{ IMX8QXP_I2C2_CLK, "I2C2" },
	{ IMX8QXP_I2C3_CLK, "I2C3" },
	{ IMX8QXP_UART0_CLK, "UART0" },
	{ IMX8QXP_UART1_CLK, "UART1" },
	{ IMX8QXP_UART2_CLK, "UART2" },
	{ IMX8QXP_UART3_CLK, "UART3" },
	{ IMX8QXP_SDHC0_CLK, "SDHC0" },
	{ IMX8QXP_SDHC1_CLK, "SDHC1" },
	{ IMX8QXP_ENET0_AHB_CLK, "ENET0_AHB" },
	{ IMX8QXP_ENET0_IPG_CLK, "ENET0_IPG" },
	{ IMX8QXP_ENET0_REF_DIV, "ENET0_REF" },
	{ IMX8QXP_ENET0_PTP_CLK, "ENET0_PTP" },
	{ IMX8QXP_ENET1_AHB_CLK, "ENET1_AHB" },
	{ IMX8QXP_ENET1_IPG_CLK, "ENET1_IPG" },
	{ IMX8QXP_ENET1_REF_DIV, "ENET1_REF" },
	{ IMX8QXP_ENET1_PTP_CLK, "ENET1_PTP" },
};

int num_clks = ARRAY_SIZE(imx8_clk_names);
#endif

ulong imx8_clk_get_rate(struct clk *clk)
{
	sc_pm_clk_t pm_clk;
	ulong rate;
	u16 resource;
	int ret;

	debug("%s(#%lu)\n", __func__, clk->id);

	switch (clk->id) {
	case IMX8QXP_A35_DIV:
		resource = SC_R_A35;
		pm_clk = SC_PM_CLK_CPU;
		break;
	case IMX8QXP_I2C0_CLK:
		resource = SC_R_I2C_0;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_I2C1_CLK:
		resource = SC_R_I2C_1;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_I2C2_CLK:
		resource = SC_R_I2C_2;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_I2C3_CLK:
		resource = SC_R_I2C_3;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_SDHC0_IPG_CLK:
	case IMX8QXP_SDHC0_CLK:
	case IMX8QXP_SDHC0_DIV:
		resource = SC_R_SDHC_0;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_SDHC1_IPG_CLK:
	case IMX8QXP_SDHC1_CLK:
	case IMX8QXP_SDHC1_DIV:
		resource = SC_R_SDHC_1;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_UART0_IPG_CLK:
	case IMX8QXP_UART0_CLK:
		resource = SC_R_UART_0;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_UART1_CLK:
		resource = SC_R_UART_1;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_UART2_CLK:
		resource = SC_R_UART_2;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_UART3_CLK:
		resource = SC_R_UART_3;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_ENET0_IPG_CLK:
	case IMX8QXP_ENET0_AHB_CLK:
	case IMX8QXP_ENET0_REF_DIV:
	case IMX8QXP_ENET0_PTP_CLK:
		resource = SC_R_ENET_0;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_ENET1_IPG_CLK:
	case IMX8QXP_ENET1_AHB_CLK:
	case IMX8QXP_ENET1_REF_DIV:
	case IMX8QXP_ENET1_PTP_CLK:
		resource = SC_R_ENET_1;
		pm_clk = SC_PM_CLK_PER;
		break;
	default:
		if (clk->id < IMX8QXP_UART0_IPG_CLK ||
		    clk->id >= IMX8QXP_CLK_END) {
			printf("%s(Invalid clk ID #%lu)\n",
			       __func__, clk->id);
			return -EINVAL;
		}
		return -ENOTSUPP;
	};

	ret = sc_pm_get_clock_rate(-1, resource, pm_clk,
				   (sc_pm_clock_rate_t *)&rate);
	if (ret) {
		printf("%s err %d\n", __func__, ret);
		return ret;
	}

	return rate;
}

ulong imx8_clk_set_rate(struct clk *clk, unsigned long rate)
{
	sc_pm_clk_t pm_clk;
	u32 new_rate = rate;
	u16 resource;
	int ret;

	debug("%s(#%lu), rate: %lu\n", __func__, clk->id, rate);

	switch (clk->id) {
	case IMX8QXP_I2C0_CLK:
		resource = SC_R_I2C_0;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_I2C1_CLK:
		resource = SC_R_I2C_1;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_I2C2_CLK:
		resource = SC_R_I2C_2;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_I2C3_CLK:
		resource = SC_R_I2C_3;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_UART0_CLK:
		resource = SC_R_UART_0;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_UART1_CLK:
		resource = SC_R_UART_1;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_UART2_CLK:
		resource = SC_R_UART_2;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_UART3_CLK:
		resource = SC_R_UART_3;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_SDHC0_IPG_CLK:
	case IMX8QXP_SDHC0_CLK:
	case IMX8QXP_SDHC0_DIV:
		resource = SC_R_SDHC_0;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_SDHC1_SEL:
	case IMX8QXP_SDHC0_SEL:
		return 0;
	case IMX8QXP_SDHC1_IPG_CLK:
	case IMX8QXP_SDHC1_CLK:
	case IMX8QXP_SDHC1_DIV:
		resource = SC_R_SDHC_1;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_ENET0_IPG_CLK:
	case IMX8QXP_ENET0_AHB_CLK:
	case IMX8QXP_ENET0_REF_DIV:
	case IMX8QXP_ENET0_PTP_CLK:
		resource = SC_R_ENET_0;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_ENET1_IPG_CLK:
	case IMX8QXP_ENET1_AHB_CLK:
	case IMX8QXP_ENET1_REF_DIV:
	case IMX8QXP_ENET1_PTP_CLK:
		resource = SC_R_ENET_1;
		pm_clk = SC_PM_CLK_PER;
		break;
	default:
		if (clk->id < IMX8QXP_UART0_IPG_CLK ||
		    clk->id >= IMX8QXP_CLK_END) {
			printf("%s(Invalid clk ID #%lu)\n",
			       __func__, clk->id);
			return -EINVAL;
		}
		return -ENOTSUPP;
	};

	ret = sc_pm_set_clock_rate(-1, resource, pm_clk, &new_rate);
	if (ret) {
		printf("%s err %d\n", __func__, ret);
		return ret;
	}

	return new_rate;
}

int __imx8_clk_enable(struct clk *clk, bool enable)
{
	sc_pm_clk_t pm_clk;
	u16 resource;
	int ret;

	debug("%s(#%lu)\n", __func__, clk->id);

	switch (clk->id) {
	case IMX8QXP_I2C0_CLK:
		resource = SC_R_I2C_0;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_I2C1_CLK:
		resource = SC_R_I2C_1;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_I2C2_CLK:
		resource = SC_R_I2C_2;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_I2C3_CLK:
		resource = SC_R_I2C_3;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_UART0_CLK:
		resource = SC_R_UART_0;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_UART1_CLK:
		resource = SC_R_UART_1;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_UART2_CLK:
		resource = SC_R_UART_2;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_UART3_CLK:
		resource = SC_R_UART_3;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_SDHC0_IPG_CLK:
	case IMX8QXP_SDHC0_CLK:
	case IMX8QXP_SDHC0_DIV:
		resource = SC_R_SDHC_0;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_SDHC1_IPG_CLK:
	case IMX8QXP_SDHC1_CLK:
	case IMX8QXP_SDHC1_DIV:
		resource = SC_R_SDHC_1;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_ENET0_IPG_CLK:
	case IMX8QXP_ENET0_AHB_CLK:
	case IMX8QXP_ENET0_REF_DIV:
	case IMX8QXP_ENET0_PTP_CLK:
		resource = SC_R_ENET_0;
		pm_clk = SC_PM_CLK_PER;
		break;
	case IMX8QXP_ENET1_IPG_CLK:
	case IMX8QXP_ENET1_AHB_CLK:
	case IMX8QXP_ENET1_REF_DIV:
	case IMX8QXP_ENET1_PTP_CLK:
		resource = SC_R_ENET_1;
		pm_clk = SC_PM_CLK_PER;
		break;
	default:
		if (clk->id < IMX8QXP_UART0_IPG_CLK ||
		    clk->id >= IMX8QXP_CLK_END) {
			printf("%s(Invalid clk ID #%lu)\n",
			       __func__, clk->id);
			return -EINVAL;
		}
		return -ENOTSUPP;
	}

	ret = sc_pm_clock_enable(-1, resource, pm_clk, enable, 0);
	if (ret) {
		printf("%s err %d\n", __func__, ret);
		return ret;
	}

	return 0;
}
