// SPDX-License-Identifier: GPL-2.0+
/*
 * sun4i, sun5i and sun7i specific clock code
 *
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * (C) Copyright 2013 Luke Kenneth Casson Leighton <lkcl@lkcl.net>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/gpio.h>
#include <asm/arch/sys_proto.h>

#ifdef CONFIG_SPL_BUILD
void clock_init_safe(void)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* Set safe defaults until PMU is configured */
	writel(AXI_DIV_1 << AXI_DIV_SHIFT |
	       AHB_DIV_2 << AHB_DIV_SHIFT |
	       APB0_DIV_1 << APB0_DIV_SHIFT |
	       CPU_CLK_SRC_OSC24M << CPU_CLK_SRC_SHIFT,
	       &ccm->cpu_ahb_apb0_cfg);
	writel(PLL1_CFG_DEFAULT, &ccm->pll1_cfg);
	sdelay(200);
	writel(AXI_DIV_1 << AXI_DIV_SHIFT |
	       AHB_DIV_2 << AHB_DIV_SHIFT |
	       APB0_DIV_1 << APB0_DIV_SHIFT |
	       CPU_CLK_SRC_PLL1 << CPU_CLK_SRC_SHIFT,
	       &ccm->cpu_ahb_apb0_cfg);
#ifdef CONFIG_MACH_SUN7I
	setbits_le32(&ccm->ahb_gate0, 0x1 << AHB_GATE_OFFSET_DMA);
#endif
	writel(PLL6_CFG_DEFAULT, &ccm->pll6_cfg);
#ifdef CONFIG_SUNXI_AHCI
	setbits_le32(&ccm->ahb_gate0, 0x1 << AHB_GATE_OFFSET_SATA);
	setbits_le32(&ccm->pll6_cfg, 0x1 << CCM_PLL6_CTRL_SATA_EN_SHIFT);
#endif
}
#endif

void clock_init_uart(void)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* uart clock source is apb1 */
	writel(APB1_CLK_SRC_OSC24M|
	       APB1_CLK_RATE_N_1|
	       APB1_CLK_RATE_M(1),
	       &ccm->apb1_clk_div_cfg);

	/* open the clock for uart */
	setbits_le32(&ccm->apb1_gate,
		CLK_GATE_OPEN << (APB1_GATE_UART_SHIFT+CONFIG_CONS_INDEX - 1));
}

int clock_twi_onoff(int port, int state)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* set the apb clock gate for twi */
	if (state)
		setbits_le32(&ccm->apb1_gate,
			     CLK_GATE_OPEN << (APB1_GATE_TWI_SHIFT + port));
	else
		clrbits_le32(&ccm->apb1_gate,
			     CLK_GATE_OPEN << (APB1_GATE_TWI_SHIFT + port));

	return 0;
}

#ifdef CONFIG_SPL_BUILD
#define PLL1_CFG(N, K, M, P)	( 1 << CCM_PLL1_CFG_ENABLE_SHIFT | \
				  0 << CCM_PLL1_CFG_VCO_RST_SHIFT |  \
				  8 << CCM_PLL1_CFG_VCO_BIAS_SHIFT | \
				  0 << CCM_PLL1_CFG_PLL4_EXCH_SHIFT | \
				 16 << CCM_PLL1_CFG_BIAS_CUR_SHIFT | \
				 (P)<< CCM_PLL1_CFG_DIVP_SHIFT | \
				  2 << CCM_PLL1_CFG_LCK_TMR_SHIFT | \
				 (N)<< CCM_PLL1_CFG_FACTOR_N_SHIFT | \
				 (K)<< CCM_PLL1_CFG_FACTOR_K_SHIFT | \
				  0 << CCM_PLL1_CFG_SIG_DELT_PAT_IN_SHIFT | \
				  0 << CCM_PLL1_CFG_SIG_DELT_PAT_EN_SHIFT | \
				 (M)<< CCM_PLL1_CFG_FACTOR_M_SHIFT)

static struct {
	u32 pll1_cfg;
	unsigned int freq;
} pll1_para[] = {
	/* This array must be ordered by frequency. */
	{ PLL1_CFG(31, 1, 0, 0), 1488000000},
	{ PLL1_CFG(30, 1, 0, 0), 1440000000},
	{ PLL1_CFG(29, 1, 0, 0), 1392000000},
	{ PLL1_CFG(28, 1, 0, 0), 1344000000},
	{ PLL1_CFG(27, 1, 0, 0), 1296000000},
	{ PLL1_CFG(26, 1, 0, 0), 1248000000},
	{ PLL1_CFG(25, 1, 0, 0), 1200000000},
	{ PLL1_CFG(24, 1, 0, 0), 1152000000},
	{ PLL1_CFG(23, 1, 0, 0), 1104000000},
	{ PLL1_CFG(22, 1, 0, 0), 1056000000},
	{ PLL1_CFG(21, 1, 0, 0), 1008000000},
	{ PLL1_CFG(20, 1, 0, 0), 960000000 },
	{ PLL1_CFG(19, 1, 0, 0), 912000000 },
	{ PLL1_CFG(16, 1, 0, 0), 768000000 },
	/* Final catchall entry 384MHz*/
	{ PLL1_CFG(16, 0, 0, 0), 0 },

};

void clock_set_pll1(unsigned int hz)
{
	int i = 0;
	int axi, ahb, apb0;
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	/* Find target frequency */
	while (pll1_para[i].freq > hz)
		i++;

	hz = pll1_para[i].freq;
	if (! hz)
		hz = 384000000;

	/* Calculate system clock divisors */
	axi = DIV_ROUND_UP(hz, 432000000);	/* Max 450MHz */
	ahb = DIV_ROUND_UP(hz/axi, 204000000);	/* Max 250MHz */
	apb0 = 2;				/* Max 150MHz */

	printf("CPU: %uHz, AXI/AHB/APB: %d/%d/%d\n", hz, axi, ahb, apb0);

	/* Map divisors to register values */
	axi = axi - 1;
	if (ahb > 4)
		ahb = 3;
	else if (ahb > 2)
		ahb = 2;
	else if (ahb > 1)
		ahb = 1;
	else
		ahb = 0;

	apb0 = apb0 - 1;

	/* Switch to 24MHz clock while changing PLL1 */
	writel(AXI_DIV_1 << AXI_DIV_SHIFT |
	       AHB_DIV_2 << AHB_DIV_SHIFT |
	       APB0_DIV_1 << APB0_DIV_SHIFT |
	       CPU_CLK_SRC_OSC24M << CPU_CLK_SRC_SHIFT,
	       &ccm->cpu_ahb_apb0_cfg);
	sdelay(20);

	/* Configure sys clock divisors */
	writel(axi << AXI_DIV_SHIFT |
	       ahb << AHB_DIV_SHIFT |
	       apb0 << APB0_DIV_SHIFT |
	       CPU_CLK_SRC_OSC24M << CPU_CLK_SRC_SHIFT,
	       &ccm->cpu_ahb_apb0_cfg);

	/* Configure PLL1 at the desired frequency */
	writel(pll1_para[i].pll1_cfg, &ccm->pll1_cfg);
	sdelay(200);

	/* Switch CPU to PLL1 */
	writel(axi << AXI_DIV_SHIFT |
	       ahb << AHB_DIV_SHIFT |
	       apb0 << APB0_DIV_SHIFT |
	       CPU_CLK_SRC_PLL1 << CPU_CLK_SRC_SHIFT,
	       &ccm->cpu_ahb_apb0_cfg);
	sdelay(20);
}
#endif

void clock_set_pll3(unsigned int clk)
{
	struct sunxi_ccm_reg * const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;

	if (clk == 0) {
		clrbits_le32(&ccm->pll3_cfg, CCM_PLL3_CTRL_EN);
		return;
	}

	/* PLL3 rate = 3000000 * m */
	writel(CCM_PLL3_CTRL_EN | CCM_PLL3_CTRL_INTEGER_MODE |
	       CCM_PLL3_CTRL_M(clk / 3000000), &ccm->pll3_cfg);
}

unsigned int clock_get_pll3(void)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	uint32_t rval = readl(&ccm->pll3_cfg);
	int m = ((rval & CCM_PLL3_CTRL_M_MASK) >> CCM_PLL3_CTRL_M_SHIFT);
	return 3000000 * m;
}

unsigned int clock_get_pll5p(void)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	uint32_t rval = readl(&ccm->pll5_cfg);
	int n = ((rval & CCM_PLL5_CTRL_N_MASK) >> CCM_PLL5_CTRL_N_SHIFT);
	int k = ((rval & CCM_PLL5_CTRL_K_MASK) >> CCM_PLL5_CTRL_K_SHIFT) + 1;
	int p = ((rval & CCM_PLL5_CTRL_P_MASK) >> CCM_PLL5_CTRL_P_SHIFT);
	return (24000000 * n * k) >> p;
}

unsigned int clock_get_pll6(void)
{
	struct sunxi_ccm_reg *const ccm =
		(struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	uint32_t rval = readl(&ccm->pll6_cfg);
	int n = ((rval & CCM_PLL6_CTRL_N_MASK) >> CCM_PLL6_CTRL_N_SHIFT);
	int k = ((rval & CCM_PLL6_CTRL_K_MASK) >> CCM_PLL6_CTRL_K_SHIFT) + 1;
	return 24000000 * n * k / 2;
}

void clock_set_de_mod_clock(u32 *clk_cfg, unsigned int hz)
{
	int pll = clock_get_pll5p();
	int div = 1;

	while ((pll / div) > hz)
		div++;

	writel(CCM_DE_CTRL_GATE | CCM_DE_CTRL_RST | CCM_DE_CTRL_PLL5P |
	       CCM_DE_CTRL_M(div), clk_cfg);
}
