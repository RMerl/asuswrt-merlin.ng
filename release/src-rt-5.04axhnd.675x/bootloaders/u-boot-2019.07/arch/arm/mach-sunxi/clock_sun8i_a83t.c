// SPDX-License-Identifier: GPL-2.0+
/*
 * A83 specific clock code
 *
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * (C) Copyright 2015 Vishnu Patekar <vishnupatekar0510@gmail.com>
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

	clock_set_pll1(408000000);
	/* enable pll_hsic, default is 480M */
	writel(PLL8_CFG_DEFAULT, &ccm->pll8_cfg);
	writel(readl(&ccm->pll8_cfg) | (0x1 << 31), &ccm->pll8_cfg);
	while (!(readl(&ccm->pll_stable_status) & (1 << 8))) {}

	/* switch to default 24MHz before changing to hsic */
	writel(0x0, &ccm->cci400_cfg);
	sdelay(50);
	writel(CCM_CCI400_CLK_SEL_HSIC, &ccm->cci400_cfg);
	sdelay(100);

	/* switch before changing pll6 */
	clrsetbits_le32(&ccm->ahb1_apb1_div, AHB1_CLK_SRC_MASK,
			AHB1_CLK_SRC_OSC24M);
	writel(PLL6_CFG_DEFAULT, &ccm->pll6_cfg);
	while (!(readl(&ccm->pll_stable_status) & (1 << 6))) {}

	writel(AHB1_ABP1_DIV_DEFAULT, &ccm->ahb1_apb1_div);
	writel(CCM_MBUS_RESET_RESET, &ccm->mbus_reset);
	writel(MBUS_CLK_DEFAULT, &ccm->mbus_clk_cfg);

	/* timestamp */
	writel(1, 0x01720000);
}
#endif

void clock_init_uart(void)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* uart clock source is apb2 */
	writel(APB2_CLK_SRC_OSC24M|
	       APB2_CLK_RATE_N_1|
	       APB2_CLK_RATE_M(1),
	       &ccm->apb2_div);

	/* open the clock for uart */
	setbits_le32(&ccm->apb2_gate,
		     CLK_GATE_OPEN << (APB2_GATE_UART_SHIFT +
				       CONFIG_CONS_INDEX - 1));

	/* deassert uart reset */
	setbits_le32(&ccm->apb2_reset_cfg,
		     1 << (APB2_RESET_UART_SHIFT +
			   CONFIG_CONS_INDEX - 1));
}

#ifdef CONFIG_SPL_BUILD
void clock_set_pll1(unsigned int clk)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	const int p = 0;

	/* Switch to 24MHz clock while changing PLL1 */
	writel(AXI_DIV_2 << AXI0_DIV_SHIFT |
		AXI_DIV_2 << AXI1_DIV_SHIFT |
		CPU_CLK_SRC_OSC24M << C0_CPUX_CLK_SRC_SHIFT |
		CPU_CLK_SRC_OSC24M << C1_CPUX_CLK_SRC_SHIFT,
	       &ccm->cpu_axi_cfg);

	/* clk = 24*n/p, p is ignored if clock is >288MHz */
	writel(CCM_PLL1_CTRL_EN | CCM_PLL1_CTRL_P(p) | CMM_PLL1_CLOCK_TIME_2 |
		CCM_PLL1_CTRL_N(clk / 24000000),
		&ccm->pll1_c0_cfg);
	while (!(readl(&ccm->pll_stable_status) & 0x01)) {}

	writel(CCM_PLL1_CTRL_EN | CCM_PLL1_CTRL_P(p) | CMM_PLL1_CLOCK_TIME_2 |
		CCM_PLL1_CTRL_N(clk / (24000000)),
		&ccm->pll1_c1_cfg);
	while (!(readl(&ccm->pll_stable_status) & 0x02)) {}

	/* Switch CPU to PLL1 */
	writel(AXI_DIV_2 << AXI0_DIV_SHIFT |
		AXI_DIV_2 << AXI1_DIV_SHIFT |
		CPU_CLK_SRC_PLL1 << C0_CPUX_CLK_SRC_SHIFT |
		CPU_CLK_SRC_PLL1 << C1_CPUX_CLK_SRC_SHIFT,
	       &ccm->cpu_axi_cfg);
}
#endif

void clock_set_pll5(unsigned int clk)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	unsigned int div1 = 0, div2 = 0;

	/* A83T PLL5 DDR rate = 24000000 * (n+1)/(div1+1)/(div2+1) */
	writel(CCM_PLL5_CTRL_EN | CCM_PLL5_CTRL_UPD |
			CCM_PLL5_CTRL_N(clk / (24000000)) |
			div2 << CCM_PLL5_DIV2_SHIFT |
			div1 << CCM_PLL5_DIV1_SHIFT, &ccm->pll5_cfg);

	udelay(5500);
}


unsigned int clock_get_pll6(void)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	uint32_t rval = readl(&ccm->pll6_cfg);
	int n = ((rval & CCM_PLL6_CTRL_N_MASK) >> CCM_PLL6_CTRL_N_SHIFT);
	int div1 = ((rval & CCM_PLL6_CTRL_DIV1_MASK) >>
			CCM_PLL6_CTRL_DIV1_SHIFT) + 1;
	int div2 = ((rval & CCM_PLL6_CTRL_DIV2_MASK) >>
			CCM_PLL6_CTRL_DIV2_SHIFT) + 1;
	return 24000000 * n / div1 / div2;
}
