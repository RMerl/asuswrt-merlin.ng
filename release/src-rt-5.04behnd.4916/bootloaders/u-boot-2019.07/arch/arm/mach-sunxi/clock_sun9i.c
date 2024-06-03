// SPDX-License-Identifier: GPL-2.0+

/*
 * sun9i specific clock code
 *
 * (C) Copyright 2015 Hans de Goede <hdegoede@redhat.com>
 *
 * (C) Copyright 2016 Theobroma Systems Design und Consulting GmbH
 *                    Philipp Tomsich <philipp.tomsich@theobroma-systems.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/prcm.h>
#include <asm/arch/sys_proto.h>


#ifdef CONFIG_SPL_BUILD

void clock_init_safe(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* Set up PLL12 (peripheral 1) */
	clock_set_pll12(1200000000);

	/* Set up PLL1 (cluster 0) and PLL2 (cluster 1) */
	clock_set_pll1(408000000);
	clock_set_pll2(408000000);

	/* Set up PLL4 (peripheral 0) */
	clock_set_pll4(960000000);

	/* Set up dividers for AXI0 and APB0 on cluster 0: PLL1 / 2 = 204MHz */
	writel(C0_CFG_AXI0_CLK_DIV_RATIO(2) |
	       C0_CFG_APB0_CLK_DIV_RATIO(2), &ccm->c0_cfg);

	/* AHB0: 120 MHz (PLL_PERIPH0 / 8) */
	writel(AHBx_SRC_PLL_PERIPH0 | AHBx_CLK_DIV_RATIO(8),
	       &ccm->ahb0_cfg);
	/* AHB1: 240 MHz (PLL_PERIPH0 / 4) */
	writel(AHBx_SRC_PLL_PERIPH0 | AHBx_CLK_DIV_RATIO(4),
	       &ccm->ahb1_cfg);
	/* AHB2: 120 MHz (PLL_PERIPH0 / 8) */
	writel(AHBx_SRC_PLL_PERIPH0 | AHBx_CLK_DIV_RATIO(8),
	       &ccm->ahb2_cfg);
	/* APB0: 120 MHz (PLL_PERIPH0 / 8) */
	writel(APB0_SRC_PLL_PERIPH0 | APB0_CLK_DIV_RATIO(8),
	       &ccm->apb0_cfg);

	/* GTBUS: 400MHz (PERIPH0 div 3) */
	writel(GTBUS_SRC_PLL_PERIPH1 | GTBUS_CLK_DIV_RATIO(3),
	       &ccm->gtbus_cfg);
	/* CCI400: 480MHz (PERIPH1 div 2) */
	writel(CCI400_SRC_PLL_PERIPH0 | CCI400_CLK_DIV_RATIO(2),
	       &ccm->cci400_cfg);

	/* Deassert DMA reset and open clock gating for DMA */
	setbits_le32(&ccm->ahb_reset1_cfg, (1 << 24));
	setbits_le32(&ccm->apb1_gate, (1 << 24));

	/* set enable-bit in TSTAMP_CTRL_REG */
	writel(1, 0x01720000);
}
#endif

void clock_init_uart(void)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* open the clock for uart */
	setbits_le32(&ccm->apb1_gate,
		     CLK_GATE_OPEN << (APB1_GATE_UART_SHIFT +
				       CONFIG_CONS_INDEX - 1));
	/* deassert uart reset */
	setbits_le32(&ccm->apb1_reset_cfg,
		     1 << (APB1_RESET_UART_SHIFT +
			   CONFIG_CONS_INDEX - 1));
}

#ifdef CONFIG_SPL_BUILD
void clock_set_pll1(unsigned int clk)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	const int p = 0;

	/* Switch cluster 0 to 24MHz clock while changing PLL1 */
	clrsetbits_le32(&ccm->cpu_clk_source, C0_CPUX_CLK_SRC_MASK,
			C0_CPUX_CLK_SRC_OSC24M);

	writel(CCM_PLL1_CTRL_EN | CCM_PLL1_CTRL_P(p) |
	       CCM_PLL1_CLOCK_TIME_2 |
	       CCM_PLL1_CTRL_N(clk / 24000000),
	       &ccm->pll1_c0_cfg);
	/*
	 * Don't bother with the stable-time registers, as it doesn't
	 * wait until the PLL is stable.  Note, that even Allwinner
	 * just uses a delay loop (or rather the AVS timer) for this
	 * instead of the PLL_STABLE_STATUS register.
	 */
	sdelay(2000);

	/* Switch cluster 0 back to PLL1 */
	clrsetbits_le32(&ccm->cpu_clk_source, C0_CPUX_CLK_SRC_MASK,
			C0_CPUX_CLK_SRC_PLL1);
}

void clock_set_pll2(unsigned int clk)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	const int p = 0;

	/* Switch cluster 1 to 24MHz clock while changing PLL2 */
	clrsetbits_le32(&ccm->cpu_clk_source, C1_CPUX_CLK_SRC_MASK,
			C1_CPUX_CLK_SRC_OSC24M);

	writel(CCM_PLL2_CTRL_EN | CCM_PLL2_CTRL_P(p) |
	       CCM_PLL2_CLOCK_TIME_2 | CCM_PLL2_CTRL_N(clk / 24000000),
	       &ccm->pll2_c1_cfg);

	sdelay(2000);

	/* Switch cluster 1 back to PLL2 */
	clrsetbits_le32(&ccm->cpu_clk_source, C1_CPUX_CLK_SRC_MASK,
			C1_CPUX_CLK_SRC_PLL2);
}

void clock_set_pll6(unsigned int clk)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	const int p = 0;

	writel(CCM_PLL6_CTRL_EN | CCM_PLL6_CFG_UPDATE | CCM_PLL6_CTRL_P(p)
	       | CCM_PLL6_CTRL_N(clk / 24000000),
	       &ccm->pll6_ddr_cfg);
	do { } while (!(readl(&ccm->pll_stable_status) & PLL_DDR_STATUS));

	sdelay(2000);
}

void clock_set_pll12(unsigned int clk)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	if (readl(&ccm->pll12_periph1_cfg) & CCM_PLL12_CTRL_EN)
		return;

	writel(CCM_PLL12_CTRL_EN | CCM_PLL12_CTRL_N(clk / 24000000),
	       &ccm->pll12_periph1_cfg);

	sdelay(2000);
}


void clock_set_pll4(unsigned int clk)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	writel(CCM_PLL4_CTRL_EN | CCM_PLL4_CTRL_N(clk / 24000000),
	       &ccm->pll4_periph0_cfg);

	sdelay(2000);
}
#endif

int clock_twi_onoff(int port, int state)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	if (port > 4)
		return -1;

	/* set the apb reset and clock gate for twi */
	if (state) {
		setbits_le32(&ccm->apb1_gate,
			     CLK_GATE_OPEN << (APB1_GATE_TWI_SHIFT + port));
		setbits_le32(&ccm->apb1_reset_cfg,
			     1 << (APB1_RESET_TWI_SHIFT + port));
	} else {
		clrbits_le32(&ccm->apb1_reset_cfg,
			     1 << (APB1_RESET_TWI_SHIFT + port));
		clrbits_le32(&ccm->apb1_gate,
			     CLK_GATE_OPEN << (APB1_GATE_TWI_SHIFT + port));
	}

	return 0;
}

unsigned int clock_get_pll4_periph0(void)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	uint32_t rval = readl(&ccm->pll4_periph0_cfg);
	int n = ((rval & CCM_PLL4_CTRL_N_MASK) >> CCM_PLL4_CTRL_N_SHIFT);
	int p = ((rval & CCM_PLL4_CTRL_P_MASK) >> CCM_PLL4_CTRL_P_SHIFT);
	int m = ((rval & CCM_PLL4_CTRL_M_MASK) >> CCM_PLL4_CTRL_M_SHIFT) + 1;
	const int k = 1;

	return ((24000000 * n * k) >> p) / m;
}
