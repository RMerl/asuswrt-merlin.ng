// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Broadcom Corporation.
 */

#include <common.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <asm/arch/sysmap.h>
#include <asm/kona-common/clk.h>
#include "clk-core.h"

#define WR_ACCESS_ADDR			ESUB_CLK_BASE_ADDR
#define WR_ACCESS_PASSWORD				0xA5A500

#define PLLE_POST_RESETB_ADDR		(ESUB_CLK_BASE_ADDR + 0x00000C00)

#define PLLE_RESETB_ADDR		(ESUB_CLK_BASE_ADDR + 0x00000C58)
#define PLLE_RESETB_I_PLL_RESETB_PLLE_MASK		0x00010000
#define PLLE_POST_RESETB_I_POST_RESETB_PLLE_MASK	0x00000001

#define PLL_LOCK_ADDR			(ESUB_CLK_BASE_ADDR + 0x00000C38)
#define PLL_LOCK_PLL_LOCK_PLLE_MASK			0x00000001

#define ESW_SYS_DIV_ADDR		(ESUB_CLK_BASE_ADDR + 0x00000A04)
#define ESW_SYS_DIV_PLL_SELECT_MASK			0x00000300
#define ESW_SYS_DIV_DIV_MASK				0x0000001C
#define ESW_SYS_DIV_PLL_VAR_208M_CLK_SELECT		0x00000100
#define ESW_SYS_DIV_DIV_SELECT				0x4
#define ESW_SYS_DIV_TRIGGER_MASK			0x00000001

#define ESUB_AXI_DIV_DEBUG_ADDR		(ESUB_CLK_BASE_ADDR + 0x00000E04)
#define ESUB_AXI_DIV_DEBUG_PLL_SELECT_MASK		0x0000001C
#define ESUB_AXI_DIV_DEBUG_PLL_SELECT_OVERRIDE_MASK	0x00000040
#define ESUB_AXI_DIV_DEBUG_PLL_VAR_208M_CLK_SELECT	0x0
#define ESUB_AXI_DIV_DEBUG_TRIGGER_MASK			0x00000001

#define PLL_MAX_RETRY	100

/* Enable appropriate clocks for Ethernet */
int clk_eth_enable(void)
{
	int rc = -1;
	int retry_count = 0;
	rc = clk_get_and_enable("esub_ccu_clk");

	/* Enable Access to CCU registers */
	writel((1 | WR_ACCESS_PASSWORD), WR_ACCESS_ADDR);

	writel(readl(PLLE_POST_RESETB_ADDR) &
	       ~PLLE_POST_RESETB_I_POST_RESETB_PLLE_MASK,
	       PLLE_POST_RESETB_ADDR);

	/* Take PLL out of reset and put into normal mode */
	writel(readl(PLLE_RESETB_ADDR) | PLLE_RESETB_I_PLL_RESETB_PLLE_MASK,
	       PLLE_RESETB_ADDR);

	/* Wait for PLL lock */
	rc = -1;
	while (retry_count < PLL_MAX_RETRY) {
		udelay(100);
		if (readl(PLL_LOCK_ADDR) & PLL_LOCK_PLL_LOCK_PLLE_MASK) {
			rc = 0;
			break;
		}
		retry_count++;
	}

	if (rc == -1) {
		printf("%s: ETH-PLL lock timeout, Ethernet is not enabled!\n",
		       __func__);
		return -1;
	}

	writel(readl(PLLE_POST_RESETB_ADDR) |
	       PLLE_POST_RESETB_I_POST_RESETB_PLLE_MASK,
	       PLLE_POST_RESETB_ADDR);

	/* Switch esw_sys_clk to use 104MHz(208MHz/2) clock */
	writel((readl(ESW_SYS_DIV_ADDR) &
		~(ESW_SYS_DIV_PLL_SELECT_MASK | ESW_SYS_DIV_DIV_MASK)) |
	       ESW_SYS_DIV_PLL_VAR_208M_CLK_SELECT | ESW_SYS_DIV_DIV_SELECT,
	       ESW_SYS_DIV_ADDR);

	writel(readl(ESW_SYS_DIV_ADDR) | ESW_SYS_DIV_TRIGGER_MASK,
	       ESW_SYS_DIV_ADDR);

	/* Wait for trigger complete */
	rc = -1;
	retry_count = 0;
	while (retry_count < PLL_MAX_RETRY) {
		udelay(100);
		if (!(readl(ESW_SYS_DIV_ADDR) & ESW_SYS_DIV_TRIGGER_MASK)) {
			rc = 0;
			break;
		}
		retry_count++;
	}

	if (rc == -1) {
		printf("%s: SYS CLK Trigger timeout, Ethernet is not enabled!\n",
		       __func__);
		return -1;
	}

	/* switch Esub AXI clock to 208MHz */
	writel((readl(ESUB_AXI_DIV_DEBUG_ADDR) &
		~(ESUB_AXI_DIV_DEBUG_PLL_SELECT_MASK |
		  ESUB_AXI_DIV_DEBUG_PLL_SELECT_OVERRIDE_MASK |
		  ESUB_AXI_DIV_DEBUG_TRIGGER_MASK)) |
	       ESUB_AXI_DIV_DEBUG_PLL_VAR_208M_CLK_SELECT |
	       ESUB_AXI_DIV_DEBUG_PLL_SELECT_OVERRIDE_MASK,
	       ESUB_AXI_DIV_DEBUG_ADDR);

	writel(readl(ESUB_AXI_DIV_DEBUG_ADDR) |
	       ESUB_AXI_DIV_DEBUG_TRIGGER_MASK,
	       ESUB_AXI_DIV_DEBUG_ADDR);

	/* Wait for trigger complete */
	rc = -1;
	retry_count = 0;
	while (retry_count < PLL_MAX_RETRY) {
		udelay(100);
		if (!(readl(ESUB_AXI_DIV_DEBUG_ADDR) &
		      ESUB_AXI_DIV_DEBUG_TRIGGER_MASK)) {
			rc = 0;
			break;
		}
		retry_count++;
	}

	if (rc == -1) {
		printf("%s: AXI CLK Trigger timeout, Ethernet is not enabled!\n",
		       __func__);
		return -1;
	}

	/* Disable Access to CCU registers */
	writel(WR_ACCESS_PASSWORD, WR_ACCESS_ADDR);

	return rc;
}
