// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) ASPEED Technology Inc.
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/io.h>
#include <dm/lists.h>
#include <linux/delay.h>
#include <asm/arch/scu_ast2600.h>
#include <asm/global_data.h>
#include <dt-bindings/clock/ast2600-clock.h>
#include <dt-bindings/reset/ast2600-reset.h>

DECLARE_GLOBAL_DATA_PTR;

#define CLKIN_25M 25000000UL

/* MAC Clock Delay settings */
#define MAC12_DEF_DELAY_1G		0x0028a410
#define MAC12_DEF_DELAY_100M		0x00410410
#define MAC12_DEF_DELAY_10M		0x00410410
#define MAC34_DEF_DELAY_1G		0x00104208
#define MAC34_DEF_DELAY_100M		0x00104208
#define MAC34_DEF_DELAY_10M		0x00104208

/*
 * 3-bit encode of CPU freqeucy
 * Some code is duplicated
 */
enum ast2600_cpu_freq {
	CPU_FREQ_1200M_1,
	CPU_FREQ_1600M_1,
	CPU_FREQ_1200M_2,
	CPU_FREQ_1600M_2,
	CPU_FREQ_800M_1,
	CPU_FREQ_800M_2,
	CPU_FREQ_800M_3,
	CPU_FREQ_800M_4,
};

struct ast2600_clk_priv {
	struct ast2600_scu *scu;
};

/*
 * Clock divider/multiplier configuration struct.
 * For H-PLL and M-PLL the formula is
 * (Output Frequency) = CLKIN * ((M + 1) / (N + 1)) / (P + 1)
 * M - Numerator
 * N - Denumerator
 * P - Post Divider
 * They have the same layout in their control register.
 *
 * D-PLL and D2-PLL have extra divider (OD + 1), which is not
 * yet needed and ignored by clock configurations.
 */
union ast2600_pll_reg {
	uint32_t w;
	struct {
		unsigned int m : 13;
		unsigned int n : 6;
		unsigned int p : 4;
		unsigned int off : 1;
		unsigned int bypass : 1;
		unsigned int reset : 1;
		unsigned int reserved : 6;
	} b;
};

struct ast2600_pll_cfg {
	union ast2600_pll_reg reg;
	unsigned int ext_reg;
};

struct ast2600_pll_desc {
	uint32_t in;
	uint32_t out;
	struct ast2600_pll_cfg cfg;
};

static const struct ast2600_pll_desc ast2600_pll_lookup[] = {
	{
		.in = CLKIN_25M,
		.out = 400000000,
		.cfg.reg.b.m = 95,
		.cfg.reg.b.n = 2,
		.cfg.reg.b.p = 1,
		.cfg.ext_reg = 0x31,
	},
	{
		.in = CLKIN_25M,
		.out = 200000000,
		.cfg.reg.b.m = 127,
		.cfg.reg.b.n = 0,
		.cfg.reg.b.p = 15,
		.cfg.ext_reg = 0x3f,
	},
	{
		.in = CLKIN_25M,
		.out = 334000000,
		.cfg.reg.b.m = 667,
		.cfg.reg.b.n = 4,
		.cfg.reg.b.p = 9,
		.cfg.ext_reg = 0x14d,
	},
	{
		.in = CLKIN_25M,
		.out = 1000000000,
		.cfg.reg.b.m = 119,
		.cfg.reg.b.n = 2,
		.cfg.reg.b.p = 0,
		.cfg.ext_reg = 0x3d,
	},
	{
		.in = CLKIN_25M,
		.out = 50000000,
		.cfg.reg.b.m = 95,
		.cfg.reg.b.n = 2,
		.cfg.reg.b.p = 15,
		.cfg.ext_reg = 0x31,
	},
};

/* divisor tables */
static uint32_t axi_ahb_div0_table[] = {
	3, 2, 3, 4,
};

static uint32_t axi_ahb_div1_table[] = {
	3, 4, 6, 8,
};

static uint32_t axi_ahb_default_table[] = {
	3, 4, 3, 4, 2, 2, 2, 2,
};

extern uint32_t ast2600_get_pll_rate(struct ast2600_scu *scu, int pll_idx)
{
	union ast2600_pll_reg pll_reg;
	uint32_t hwstrap1;
	uint32_t cpu_freq;
	uint32_t mul = 1, div = 1;

	switch (pll_idx) {
	case ASPEED_CLK_APLL:
		pll_reg.w = readl(&scu->apll);
		break;
	case ASPEED_CLK_DPLL:
		pll_reg.w = readl(&scu->dpll);
		break;
	case ASPEED_CLK_EPLL:
		pll_reg.w = readl(&scu->epll);
		break;
	case ASPEED_CLK_HPLL:
		pll_reg.w = readl(&scu->hpll);
		break;
	case ASPEED_CLK_MPLL:
		pll_reg.w = readl(&scu->mpll);
		break;
	}

	if (!pll_reg.b.bypass) {
		/* F = 25Mhz * [(M + 2) / (n + 1)] / (p + 1)
		 * HPLL Numerator (M) = fix 0x5F when SCU500[10]=1
		 * Fixed 0xBF when SCU500[10]=0 and SCU500[8]=1
		 * SCU200[12:0] (default 0x8F) when SCU510[10]=0 and SCU510[8]=0
		 * HPLL Denumerator (N) =	SCU200[18:13] (default 0x2)
		 * HPLL Divider (P)	 =	SCU200[22:19] (default 0x0)
		 * HPLL Bandwidth Adj (NB) =  fix 0x2F when SCU500[10]=1
		 * Fixed 0x5F when SCU500[10]=0 and SCU500[8]=1
		 * SCU204[11:0] (default 0x31) when SCU500[10]=0 and SCU500[8]=0
		 */
		if (pll_idx == ASPEED_CLK_HPLL) {
			hwstrap1 = readl(&scu->hwstrap1);
			cpu_freq = (hwstrap1 & SCU_HWSTRAP1_CPU_FREQ_MASK) >>
				    SCU_HWSTRAP1_CPU_FREQ_SHIFT;

			switch (cpu_freq) {
			case CPU_FREQ_800M_1:
			case CPU_FREQ_800M_2:
			case CPU_FREQ_800M_3:
			case CPU_FREQ_800M_4:
				pll_reg.b.m = 0x5f;
				break;
			case CPU_FREQ_1600M_1:
			case CPU_FREQ_1600M_2:
				pll_reg.b.m = 0xbf;
				break;
			default:
				pll_reg.b.m = 0x8f;
				break;
			}
		}

		mul = (pll_reg.b.m + 1) / (pll_reg.b.n + 1);
		div = (pll_reg.b.p + 1);
	}

	return ((CLKIN_25M * mul) / div);
}

static uint32_t ast2600_get_hclk_rate(struct ast2600_scu *scu)
{
	uint32_t rate = ast2600_get_pll_rate(scu, ASPEED_CLK_HPLL);
	uint32_t axi_div, ahb_div;
	uint32_t hwstrap1 = readl(&scu->hwstrap1);
	uint32_t cpu_freq = (hwstrap1 & SCU_HWSTRAP1_CPU_FREQ_MASK) >>
			     SCU_HWSTRAP1_CPU_FREQ_SHIFT;
	uint32_t axi_ahb_ratio = (hwstrap1 & SCU_HWSTRAP1_AXI_AHB_CLK_RATIO_MASK) >>
				  SCU_HWSTRAP1_AXI_AHB_CLK_RATIO_SHIFT;

	if (hwstrap1 & SCU_HWSTRAP1_CPU_AXI_CLK_RATIO) {
		axi_ahb_div1_table[0] = axi_ahb_default_table[cpu_freq] * 2;
		axi_div = 1;
		ahb_div = axi_ahb_div1_table[axi_ahb_ratio];
	} else {
		axi_ahb_div0_table[0] = axi_ahb_default_table[cpu_freq];
		axi_div = 2;
		ahb_div = axi_ahb_div0_table[axi_ahb_ratio];
	}

	return (rate / axi_div / ahb_div);
}

static uint32_t ast2600_get_bclk_rate(struct ast2600_scu *scu)
{
	uint32_t rate = ast2600_get_pll_rate(scu, ASPEED_CLK_HPLL);
	uint32_t clksrc1 = readl(&scu->clksrc1);
	uint32_t bclk_div = (clksrc1 & SCU_CLKSRC1_BCLK_DIV_MASK) >>
			     SCU_CLKSRC1_BCLK_DIV_SHIFT;

	return (rate / ((bclk_div + 1) * 4));
}

static uint32_t ast2600_get_pclk1_rate(struct ast2600_scu *scu)
{
	uint32_t rate = ast2600_get_pll_rate(scu, ASPEED_CLK_HPLL);
	uint32_t clksrc1 = readl(&scu->clksrc1);
	uint32_t pclk_div = (clksrc1 & SCU_CLKSRC1_PCLK_DIV_MASK) >>
			     SCU_CLKSRC1_PCLK_DIV_SHIFT;

	return (rate / ((pclk_div + 1) * 4));
}

static uint32_t ast2600_get_pclk2_rate(struct ast2600_scu *scu)
{
	uint32_t rate = ast2600_get_hclk_rate(scu);
	uint32_t clksrc4 = readl(&scu->clksrc4);
	uint32_t pclk_div = (clksrc4 & SCU_CLKSRC4_PCLK_DIV_MASK) >>
			     SCU_CLKSRC4_PCLK_DIV_SHIFT;

	return (rate / ((pclk_div + 1) * 2));
}

static uint32_t ast2600_get_uxclk_in_rate(struct ast2600_scu *scu)
{
	uint32_t rate = 0;
	uint32_t clksrc5 = readl(&scu->clksrc5);
	uint32_t uxclk = (clksrc5 & SCU_CLKSRC5_UXCLK_MASK) >>
			  SCU_CLKSRC5_UXCLK_SHIFT;

	switch (uxclk) {
	case 0:
		rate = ast2600_get_pll_rate(scu, ASPEED_CLK_APLL) / 4;
		break;
	case 1:
		rate = ast2600_get_pll_rate(scu, ASPEED_CLK_APLL) / 2;
		break;
	case 2:
		rate = ast2600_get_pll_rate(scu, ASPEED_CLK_APLL);
		break;
	case 3:
		rate = ast2600_get_hclk_rate(scu);
		break;
	}

	return rate;
}

static uint32_t ast2600_get_huxclk_in_rate(struct ast2600_scu *scu)
{
	uint32_t rate = 0;
	uint32_t clksrc5 = readl(&scu->clksrc5);
	uint32_t huxclk = (clksrc5 & SCU_CLKSRC5_HUXCLK_MASK) >>
			   SCU_CLKSRC5_HUXCLK_SHIFT;

	switch (huxclk) {
	case 0:
		rate = ast2600_get_pll_rate(scu, ASPEED_CLK_APLL) / 4;
		break;
	case 1:
		rate = ast2600_get_pll_rate(scu, ASPEED_CLK_APLL) / 2;
		break;
	case 2:
		rate = ast2600_get_pll_rate(scu, ASPEED_CLK_APLL);
		break;
	case 3:
		rate = ast2600_get_hclk_rate(scu);
		break;
	}

	return rate;
}

static uint32_t ast2600_get_uart_uxclk_rate(struct ast2600_scu *scu)
{
	uint32_t rate = ast2600_get_uxclk_in_rate(scu);
	uint32_t uart_clkgen = readl(&scu->uart_clkgen);
	uint32_t n = (uart_clkgen & SCU_UART_CLKGEN_N_MASK) >>
		      SCU_UART_CLKGEN_N_SHIFT;
	uint32_t r = (uart_clkgen & SCU_UART_CLKGEN_R_MASK) >>
		      SCU_UART_CLKGEN_R_SHIFT;

	return ((rate * r) / (n * 2));
}

static uint32_t ast2600_get_uart_huxclk_rate(struct ast2600_scu *scu)
{
	uint32_t rate = ast2600_get_huxclk_in_rate(scu);
	uint32_t huart_clkgen = readl(&scu->huart_clkgen);
	uint32_t n = (huart_clkgen & SCU_HUART_CLKGEN_N_MASK) >>
		      SCU_HUART_CLKGEN_N_SHIFT;
	uint32_t r = (huart_clkgen & SCU_HUART_CLKGEN_R_MASK) >>
		      SCU_HUART_CLKGEN_R_SHIFT;

	return ((rate * r) / (n * 2));
}

static uint32_t ast2600_get_sdio_clk_rate(struct ast2600_scu *scu)
{
	uint32_t rate = 0;
	uint32_t clksrc4 = readl(&scu->clksrc4);
	uint32_t sdio_div = (clksrc4 & SCU_CLKSRC4_SDIO_DIV_MASK) >>
			     SCU_CLKSRC4_SDIO_DIV_SHIFT;

	if (clksrc4 & SCU_CLKSRC4_SDIO)
		rate = ast2600_get_pll_rate(scu, ASPEED_CLK_APLL);
	else
		rate = ast2600_get_hclk_rate(scu);

	return (rate / ((sdio_div + 1) * 2));
}

static uint32_t ast2600_get_emmc_clk_rate(struct ast2600_scu *scu)
{
	uint32_t rate = ast2600_get_pll_rate(scu, ASPEED_CLK_HPLL);
	uint32_t clksrc1 = readl(&scu->clksrc1);
	uint32_t emmc_div = (clksrc1 & SCU_CLKSRC1_EMMC_DIV_MASK) >>
			     SCU_CLKSRC1_EMMC_DIV_SHIFT;

	return (rate / ((emmc_div + 1) * 4));
}

static uint32_t ast2600_get_uart_clk_rate(struct ast2600_scu *scu, int uart_idx)
{
	uint32_t rate = 0;
	uint32_t uart5_clk = 0;
	uint32_t clksrc2 = readl(&scu->clksrc2);
	uint32_t clksrc4 = readl(&scu->clksrc4);
	uint32_t clksrc5 = readl(&scu->clksrc5);
	uint32_t misc_ctrl1 = readl(&scu->misc_ctrl1);

	switch (uart_idx) {
	case 1:
	case 2:
	case 3:
	case 4:
	case 6:
		if (clksrc4 & BIT(uart_idx - 1))
			rate = ast2600_get_uart_huxclk_rate(scu);
		else
			rate = ast2600_get_uart_uxclk_rate(scu);
		break;
	case 5:
		/*
		 * SCU0C[12] and SCU304[14] together decide
		 * the UART5 clock generation
		 */
		if (misc_ctrl1 & SCU_MISC_CTRL1_UART5_DIV)
			uart5_clk = 0x1 << 1;

		if (clksrc2 & SCU_CLKSRC2_UART5)
			uart5_clk |= 0x1;

		switch (uart5_clk) {
		case 0:
			rate = 24000000;
			break;
		case 1:
			rate = 192000000;
			break;
		case 2:
			rate = 24000000 / 13;
			break;
		case 3:
			rate = 192000000 / 13;
			break;
		}

		break;
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
		if (clksrc5 & BIT(uart_idx - 1))
			rate = ast2600_get_uart_huxclk_rate(scu);
		else
			rate = ast2600_get_uart_uxclk_rate(scu);
		break;
	}

	return rate;
}

static ulong ast2600_clk_get_rate(struct clk *clk)
{
	struct ast2600_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate = 0;

	switch (clk->id) {
	case ASPEED_CLK_HPLL:
	case ASPEED_CLK_EPLL:
	case ASPEED_CLK_DPLL:
	case ASPEED_CLK_MPLL:
	case ASPEED_CLK_APLL:
		rate = ast2600_get_pll_rate(priv->scu, clk->id);
		break;
	case ASPEED_CLK_AHB:
		rate = ast2600_get_hclk_rate(priv->scu);
		break;
	case ASPEED_CLK_APB1:
		rate = ast2600_get_pclk1_rate(priv->scu);
		break;
	case ASPEED_CLK_APB2:
		rate = ast2600_get_pclk2_rate(priv->scu);
		break;
	case ASPEED_CLK_GATE_UART1CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 1);
		break;
	case ASPEED_CLK_GATE_UART2CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 2);
		break;
	case ASPEED_CLK_GATE_UART3CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 3);
		break;
	case ASPEED_CLK_GATE_UART4CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 4);
		break;
	case ASPEED_CLK_GATE_UART5CLK:
		rate = ast2600_get_uart_clk_rate(priv->scu, 5);
		break;
	case ASPEED_CLK_BCLK:
		rate = ast2600_get_bclk_rate(priv->scu);
		break;
	case ASPEED_CLK_SDIO:
		rate = ast2600_get_sdio_clk_rate(priv->scu);
		break;
	case ASPEED_CLK_EMMC:
		rate = ast2600_get_emmc_clk_rate(priv->scu);
		break;
	case ASPEED_CLK_UARTX:
		rate = ast2600_get_uart_uxclk_rate(priv->scu);
		break;
	case ASPEED_CLK_HUARTX:
		rate = ast2600_get_uart_huxclk_rate(priv->scu);
		break;
	default:
		debug("%s: unknown clk %ld\n", __func__, clk->id);
		return -ENOENT;
	}

	return rate;
}

/**
 * @brief	lookup PLL divider config by input/output rate
 * @param[in]	*pll - PLL descriptor
 * Return:	true - if PLL divider config is found, false - else
 * The function caller shall fill "pll->in" and "pll->out",
 * then this function will search the lookup table
 * to find a valid PLL divider configuration.
 */
static bool ast2600_search_clock_config(struct ast2600_pll_desc *pll)
{
	uint32_t i;
	const struct ast2600_pll_desc *def_desc;
	bool is_found = false;

	for (i = 0; i < ARRAY_SIZE(ast2600_pll_lookup); i++) {
		def_desc = &ast2600_pll_lookup[i];

		if (def_desc->in == pll->in && def_desc->out == pll->out) {
			is_found = true;
			pll->cfg.reg.w = def_desc->cfg.reg.w;
			pll->cfg.ext_reg = def_desc->cfg.ext_reg;
			break;
		}
	}
	return is_found;
}

static uint32_t ast2600_configure_pll(struct ast2600_scu *scu,
				 struct ast2600_pll_cfg *p_cfg, int pll_idx)
{
	uint32_t addr, addr_ext;
	uint32_t reg;

	switch (pll_idx) {
	case ASPEED_CLK_HPLL:
		addr = (uint32_t)(&scu->hpll);
		addr_ext = (uint32_t)(&scu->hpll_ext);
		break;
	case ASPEED_CLK_MPLL:
		addr = (uint32_t)(&scu->mpll);
		addr_ext = (uint32_t)(&scu->mpll_ext);
		break;
	case ASPEED_CLK_DPLL:
		addr = (uint32_t)(&scu->dpll);
		addr_ext = (uint32_t)(&scu->dpll_ext);
		break;
	case ASPEED_CLK_EPLL:
		addr = (uint32_t)(&scu->epll);
		addr_ext = (uint32_t)(&scu->epll_ext);
		break;
	case ASPEED_CLK_APLL:
		addr = (uint32_t)(&scu->apll);
		addr_ext = (uint32_t)(&scu->apll_ext);
		break;
	default:
		debug("unknown PLL index\n");
		return 1;
	}

	p_cfg->reg.b.bypass = 0;
	p_cfg->reg.b.off = 0;
	p_cfg->reg.b.reset = 1;

	reg = readl(addr);
	reg &= ~GENMASK(25, 0);
	reg |= p_cfg->reg.w;
	writel(reg, addr);

	/* write extend parameter */
	writel(p_cfg->ext_reg, addr_ext);
	udelay(100);
	p_cfg->reg.b.reset = 0;
	reg &= ~GENMASK(25, 0);
	reg |= p_cfg->reg.w;
	writel(reg, addr);
	while (!(readl(addr_ext) & BIT(31)))
		;

	return 0;
}

static uint32_t ast2600_configure_ddr(struct ast2600_scu *scu, ulong rate)
{
	struct ast2600_pll_desc mpll;

	mpll.in = CLKIN_25M;
	mpll.out = rate;
	if (ast2600_search_clock_config(&mpll) == false) {
		printf("error!! unable to find valid DDR clock setting\n");
		return 0;
	}
	ast2600_configure_pll(scu, &mpll.cfg, ASPEED_CLK_MPLL);

	return ast2600_get_pll_rate(scu, ASPEED_CLK_MPLL);
}

static ulong ast2600_clk_set_rate(struct clk *clk, ulong rate)
{
	struct ast2600_clk_priv *priv = dev_get_priv(clk->dev);
	ulong new_rate;

	switch (clk->id) {
	case ASPEED_CLK_MPLL:
		new_rate = ast2600_configure_ddr(priv->scu, rate);
		break;
	default:
		return -ENOENT;
	}

	return new_rate;
}

static uint32_t ast2600_configure_mac12_clk(struct ast2600_scu *scu)
{
	/* scu340[25:0]: 1G default delay */
	clrsetbits_le32(&scu->mac12_clk_delay, GENMASK(25, 0),
			MAC12_DEF_DELAY_1G);

	/* set 100M/10M default delay */
	writel(MAC12_DEF_DELAY_100M, &scu->mac12_clk_delay_100M);
	writel(MAC12_DEF_DELAY_10M, &scu->mac12_clk_delay_10M);

	/* MAC AHB = HPLL / 6 */
	clrsetbits_le32(&scu->clksrc1, SCU_CLKSRC1_MAC_DIV_MASK,
			(0x2 << SCU_CLKSRC1_MAC_DIV_SHIFT));

	return 0;
}

static uint32_t ast2600_configure_mac34_clk(struct ast2600_scu *scu)
{
	/*
	 * scu350[31]   RGMII 125M source: 0 = from IO pin
	 * scu350[25:0] MAC 1G delay
	 */
	clrsetbits_le32(&scu->mac34_clk_delay, (BIT(31) | GENMASK(25, 0)),
			MAC34_DEF_DELAY_1G);
	writel(MAC34_DEF_DELAY_100M, &scu->mac34_clk_delay_100M);
	writel(MAC34_DEF_DELAY_10M, &scu->mac34_clk_delay_10M);

	/*
	 * clock source seletion and divider
	 * scu310[26:24] : MAC AHB bus clock = HCLK / 2
	 * scu310[18:16] : RMII 50M = HCLK_200M / 4
	 */
	clrsetbits_le32(&scu->clksrc4,
			(SCU_CLKSRC4_MAC_DIV_MASK | SCU_CLKSRC4_RMII34_DIV_MASK),
			((0x0 << SCU_CLKSRC4_MAC_DIV_SHIFT)
			 | (0x3 << SCU_CLKSRC4_RMII34_DIV_SHIFT)));

	/*
	 * set driving strength
	 * scu458[3:2] : MAC4 driving strength
	 * scu458[1:0] : MAC3 driving strength
	 */
	clrsetbits_le32(&scu->pinmux16,
			SCU_PINCTRL16_MAC4_DRIVING_MASK | SCU_PINCTRL16_MAC3_DRIVING_MASK,
			(0x3 << SCU_PINCTRL16_MAC4_DRIVING_SHIFT)
			 | (0x3 << SCU_PINCTRL16_MAC3_DRIVING_SHIFT));

	return 0;
}

/**
 * ast2600 RGMII clock source tree
 * 125M from external PAD -------->|\
 * HPLL -->|\                      | |---->RGMII 125M for MAC#1 & MAC#2
 *         | |---->| divider |---->|/                             +
 * EPLL -->|/                                                     |
 *                                                                |
 * +---------<-----------|RGMIICK PAD output enable|<-------------+
 * |
 * +--------------------------->|\
 *                              | |----> RGMII 125M for MAC#3 & MAC#4
 * HCLK 200M ---->|divider|---->|/
 * To simplify the control flow:
 * 1. RGMII 1/2 always use EPLL as the internal clock source
 * 2. RGMII 3/4 always use RGMIICK pad as the RGMII 125M source
 * 125M from external PAD -------->|\
 *                                 | |---->RGMII 125M for MAC#1 & MAC#2
 *         EPLL---->| divider |--->|/                             +
 *                                                                |
 * +<--------------------|RGMIICK PAD output enable|<-------------+
 * |
 * +--------------------------->RGMII 125M for MAC#3 & MAC#4
 */
#define RGMIICK_SRC_PAD		0
#define RGMIICK_SRC_EPLL	1 /* recommended */
#define RGMIICK_SRC_HPLL	2

#define RGMIICK_DIV2	1
#define RGMIICK_DIV3	2
#define RGMIICK_DIV4	3
#define RGMIICK_DIV5	4
#define RGMIICK_DIV6	5
#define RGMIICK_DIV7	6
#define RGMIICK_DIV8	7 /* recommended */

#define RMIICK_DIV4		0
#define RMIICK_DIV8		1
#define RMIICK_DIV12	2
#define RMIICK_DIV16	3
#define RMIICK_DIV20	4 /* recommended */
#define RMIICK_DIV24	5
#define RMIICK_DIV28	6
#define RMIICK_DIV32	7

struct ast2600_mac_clk_div {
	uint32_t src; /* 0=external PAD, 1=internal PLL */
	uint32_t fin; /* divider input speed */
	uint32_t n; /* 0=div2, 1=div2, 2=div3, 3=div4,...,7=div8 */
	uint32_t fout; /* fout = fin / n */
};

struct ast2600_mac_clk_div rgmii_clk_defconfig = {
	.src = ASPEED_CLK_EPLL,
	.fin = 1000000000,
	.n = RGMIICK_DIV8,
	.fout = 125000000,
};

struct ast2600_mac_clk_div rmii_clk_defconfig = {
	.src = ASPEED_CLK_EPLL,
	.fin = 1000000000,
	.n = RMIICK_DIV20,
	.fout = 50000000,
};

static void ast2600_init_mac_pll(struct ast2600_scu *p_scu,
				 struct ast2600_mac_clk_div *p_cfg)
{
	struct ast2600_pll_desc pll;

	pll.in = CLKIN_25M;
	pll.out = p_cfg->fin;
	if (ast2600_search_clock_config(&pll) == false) {
		pr_err("unable to find valid ETHNET MAC clock setting\n");
		return;
	}
	ast2600_configure_pll(p_scu, &pll.cfg, p_cfg->src);
}

static void ast2600_init_rgmii_clk(struct ast2600_scu *p_scu,
				   struct ast2600_mac_clk_div *p_cfg)
{
	uint32_t reg_304 = readl(&p_scu->clksrc2);
	uint32_t reg_340 = readl(&p_scu->mac12_clk_delay);
	uint32_t reg_350 = readl(&p_scu->mac34_clk_delay);

	reg_340 &= ~GENMASK(31, 29);
	/* scu340[28]: RGMIICK PAD output enable (to MAC 3/4) */
	reg_340 |= BIT(28);
	if (p_cfg->src == ASPEED_CLK_EPLL || p_cfg->src == ASPEED_CLK_HPLL) {
		/*
		 * re-init PLL if the current PLL output frequency doesn't match
		 * the divider setting
		 */
		if (p_cfg->fin != ast2600_get_pll_rate(p_scu, p_cfg->src))
			ast2600_init_mac_pll(p_scu, p_cfg);
		/* scu340[31]: select RGMII 125M from internal source */
		reg_340 |= BIT(31);
	}

	reg_304 &= ~GENMASK(23, 20);

	/* set clock divider */
	reg_304 |= (p_cfg->n & 0x7) << 20;

	/* select internal clock source */
	if (p_cfg->src == ASPEED_CLK_HPLL)
		reg_304 |= BIT(23);

	/* RGMII 3/4 clock source select */
	reg_350 &= ~BIT(31);

	writel(reg_304, &p_scu->clksrc2);
	writel(reg_340, &p_scu->mac12_clk_delay);
	writel(reg_350, &p_scu->mac34_clk_delay);
}

/**
 * ast2600 RMII/NCSI clock source tree
 * HPLL -->|\
 *         | |---->| divider |----> RMII 50M for MAC#1 & MAC#2
 * EPLL -->|/
 * HCLK(SCLICLK)---->| divider |----> RMII 50M for MAC#3 & MAC#4
 */
static void ast2600_init_rmii_clk(struct ast2600_scu *p_scu,
				  struct ast2600_mac_clk_div *p_cfg)
{
	uint32_t clksrc2 = readl(&p_scu->clksrc2);
	uint32_t clksrc4 = readl(&p_scu->clksrc4);

	if (p_cfg->src == ASPEED_CLK_EPLL || p_cfg->src == ASPEED_CLK_HPLL) {
		/*
		 * re-init PLL if the current PLL output frequency doesn't match
		 * the divider setting
		 */
		if (p_cfg->fin != ast2600_get_pll_rate(p_scu, p_cfg->src))
			ast2600_init_mac_pll(p_scu, p_cfg);
	}

	clksrc2 &= ~(SCU_CLKSRC2_RMII12 | SCU_CLKSRC2_RMII12_DIV_MASK);

	/* set RMII 1/2 clock divider */
	clksrc2 |= (p_cfg->n & 0x7) << 16;

	/* RMII clock source selection */
	if (p_cfg->src == ASPEED_CLK_HPLL)
		clksrc2 |= SCU_CLKSRC2_RMII12;

	/* set RMII 3/4 clock divider */
	clksrc4 &= ~SCU_CLKSRC4_RMII34_DIV_MASK;
	clksrc4 |= (0x3 << SCU_CLKSRC4_RMII34_DIV_SHIFT);

	writel(clksrc2, &p_scu->clksrc2);
	writel(clksrc4, &p_scu->clksrc4);
}

static uint32_t ast2600_configure_mac(struct ast2600_scu *scu, int index)
{
	uint32_t reset_bit;
	uint32_t clkgate_bit;

	switch (index) {
	case 1:
		reset_bit = BIT(ASPEED_RESET_MAC1);
		clkgate_bit = SCU_CLKGATE1_MAC1;
		writel(reset_bit, &scu->modrst_ctrl1);
		udelay(100);
		writel(clkgate_bit, &scu->clkgate_clr1);
		mdelay(10);
		writel(reset_bit, &scu->modrst_clr1);
		break;
	case 2:
		reset_bit = BIT(ASPEED_RESET_MAC2);
		clkgate_bit = SCU_CLKGATE1_MAC2;
		writel(reset_bit, &scu->modrst_ctrl1);
		udelay(100);
		writel(clkgate_bit, &scu->clkgate_clr1);
		mdelay(10);
		writel(reset_bit, &scu->modrst_clr1);
		break;
	case 3:
		reset_bit = BIT(ASPEED_RESET_MAC3 - 32);
		clkgate_bit = SCU_CLKGATE2_MAC3;
		writel(reset_bit, &scu->modrst_ctrl2);
		udelay(100);
		writel(clkgate_bit, &scu->clkgate_clr2);
		mdelay(10);
		writel(reset_bit, &scu->modrst_clr2);
		break;
	case 4:
		reset_bit = BIT(ASPEED_RESET_MAC4 - 32);
		clkgate_bit = SCU_CLKGATE2_MAC4;
		writel(reset_bit, &scu->modrst_ctrl2);
		udelay(100);
		writel(clkgate_bit, &scu->clkgate_clr2);
		mdelay(10);
		writel(reset_bit, &scu->modrst_clr2);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static void ast2600_configure_rsa_ecc_clk(struct ast2600_scu *scu)
{
	uint32_t clksrc1 = readl(&scu->clksrc1);

	/* Configure RSA clock = HPLL/3 */
	clksrc1 |= SCU_CLKSRC1_ECC_RSA;
	clksrc1 &= ~SCU_CLKSRC1_ECC_RSA_DIV_MASK;
	clksrc1 |= (2 << SCU_CLKSRC1_ECC_RSA_DIV_SHIFT);

	writel(clksrc1, &scu->clksrc1);
}

static ulong ast2600_enable_sdclk(struct ast2600_scu *scu)
{
	uint32_t reset_bit;
	uint32_t clkgate_bit;

	reset_bit = BIT(ASPEED_RESET_SD - 32);
	clkgate_bit = SCU_CLKGATE2_SDIO;

	writel(reset_bit, &scu->modrst_ctrl2);
	udelay(100);
	writel(clkgate_bit, &scu->clkgate_clr2);
	mdelay(10);
	writel(reset_bit, &scu->modrst_clr2);

	return 0;
}

static ulong ast2600_enable_extsdclk(struct ast2600_scu *scu)
{
	int i = 0;
	uint32_t div = 0;
	uint32_t rate = 0;
	uint32_t clksrc4 = readl(&scu->clksrc4);

	/*
	 * ast2600 SD controller max clk is 200Mhz
	 * use apll for clock source 800/4 = 200
	 * controller max is 200mhz
	 */
	rate = ast2600_get_pll_rate(scu, ASPEED_CLK_APLL);
	for (i = 0; i < 8; i++) {
		div = (i + 1) * 2;
		if ((rate / div) <= 200000000)
			break;
	}
	clksrc4 &= ~SCU_CLKSRC4_SDIO_DIV_MASK;
	clksrc4 |= (i << SCU_CLKSRC4_SDIO_DIV_SHIFT);
	clksrc4 |= SCU_CLKSRC4_SDIO;
	writel(clksrc4, &scu->clksrc4);

	setbits_le32(&scu->clksrc4, SCU_CLKSRC4_SDIO_EN);

	return 0;
}

static ulong ast2600_enable_emmcclk(struct ast2600_scu *scu)
{
	uint32_t reset_bit;
	uint32_t clkgate_bit;

	reset_bit = BIT(ASPEED_RESET_EMMC);
	clkgate_bit = SCU_CLKGATE1_EMMC;

	writel(reset_bit, &scu->modrst_ctrl1);
	udelay(100);
	writel(clkgate_bit, &scu->clkgate_clr1);
	mdelay(10);
	writel(reset_bit, &scu->modrst_clr1);

	return 0;
}

static ulong ast2600_enable_extemmcclk(struct ast2600_scu *scu)
{
	int i = 0;
	uint32_t div = 0;
	uint32_t rate = 0;
	uint32_t clksrc1 = readl(&scu->clksrc1);

	/*
	 * ast2600 eMMC controller max clk is 200Mhz
	 * HPll->1/2->|\
	 *				|->SCU300[11]->SCU300[14:12][1/N] +
	 * MPLL------>|/								  |
	 * +----------------------------------------------+
	 * |
	 * +---------> EMMC12C[15:8][1/N]-> eMMC clk
	 */
	rate = ast2600_get_pll_rate(scu, ASPEED_CLK_MPLL);
	for (i = 0; i < 8; i++) {
		div = (i + 1) * 2;
		if ((rate / div) <= 200000000)
			break;
	}

	clksrc1 &= ~SCU_CLKSRC1_EMMC_DIV_MASK;
	clksrc1 |= (i << SCU_CLKSRC1_EMMC_DIV_SHIFT);
	clksrc1 |= SCU_CLKSRC1_EMMC;
	writel(clksrc1, &scu->clksrc1);

	setbits_le32(&scu->clksrc1, SCU_CLKSRC1_EMMC_EN);

	return 0;
}

static ulong ast2600_enable_fsiclk(struct ast2600_scu *scu)
{
	uint32_t reset_bit;
	uint32_t clkgate_bit;

	reset_bit = BIT(ASPEED_RESET_FSI % 32);
	clkgate_bit = SCU_CLKGATE2_FSI;

	/* The FSI clock is shared between masters. If it's already on
	 * don't touch it, as that will reset the existing master.
	 */
	if (!(readl(&scu->clkgate_ctrl2) & clkgate_bit)) {
		debug("%s: already running, not touching it\n", __func__);
		return 0;
	}

	writel(reset_bit, &scu->modrst_ctrl2);
	udelay(100);
	writel(clkgate_bit, &scu->clkgate_clr2);
	mdelay(10);
	writel(reset_bit, &scu->modrst_clr2);

	return 0;
}

static ulong ast2600_enable_usbahclk(struct ast2600_scu *scu)
{
	uint32_t reset_bit;
	uint32_t clkgate_bit;

	reset_bit = BIT(ASPEED_RESET_EHCI_P1);
	clkgate_bit = SCU_CLKGATE1_USB_HUB;

	writel(reset_bit, &scu->modrst_ctrl1);
	udelay(100);
	writel(clkgate_bit, &scu->clkgate_ctrl1);
	mdelay(20);
	writel(reset_bit, &scu->modrst_clr1);

	return 0;
}

static ulong ast2600_enable_usbbhclk(struct ast2600_scu *scu)
{
	uint32_t reset_bit;
	uint32_t clkgate_bit;

	reset_bit = BIT(ASPEED_RESET_EHCI_P2);
	clkgate_bit = SCU_CLKGATE1_USB_HOST2;

	writel(reset_bit, &scu->modrst_ctrl1);
	udelay(100);
	writel(clkgate_bit, &scu->clkgate_clr1);
	mdelay(20);
	writel(reset_bit, &scu->modrst_clr1);

	return 0;
}

static ulong ast2600_enable_haceclk(struct ast2600_scu *scu)
{
	uint32_t reset_bit;
	uint32_t clkgate_bit;

	/* share the same reset control bit with ACRY */
	reset_bit = BIT(ASPEED_RESET_HACE);
	clkgate_bit = SCU_CLKGATE1_HACE;

	/*
	 * we don't do reset assertion here as HACE
	 * shares the same reset control with ACRY
	 */
	writel(clkgate_bit, &scu->clkgate_clr1);
	mdelay(20);
	writel(reset_bit, &scu->modrst_clr1);

	return 0;
}

static ulong ast2600_enable_rsaclk(struct ast2600_scu *scu)
{
	uint32_t reset_bit;
	uint32_t clkgate_bit;

	/* same reset control bit with HACE */
	reset_bit = BIT(ASPEED_RESET_HACE);
	clkgate_bit = SCU_CLKGATE1_ACRY;

	/*
	 * we don't do reset assertion here as HACE
	 * shares the same reset control with ACRY
	 */
	writel(clkgate_bit, &scu->clkgate_clr1);
	mdelay(20);
	writel(reset_bit, &scu->modrst_clr1);

	return 0;
}

static int ast2600_clk_enable(struct clk *clk)
{
	struct ast2600_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case ASPEED_CLK_GATE_MAC1CLK:
		ast2600_configure_mac(priv->scu, 1);
		break;
	case ASPEED_CLK_GATE_MAC2CLK:
		ast2600_configure_mac(priv->scu, 2);
		break;
	case ASPEED_CLK_GATE_MAC3CLK:
		ast2600_configure_mac(priv->scu, 3);
		break;
	case ASPEED_CLK_GATE_MAC4CLK:
		ast2600_configure_mac(priv->scu, 4);
		break;
	case ASPEED_CLK_GATE_SDCLK:
		ast2600_enable_sdclk(priv->scu);
		break;
	case ASPEED_CLK_SDIO:
		ast2600_enable_extsdclk(priv->scu);
		break;
	case ASPEED_CLK_GATE_EMMCCLK:
		ast2600_enable_emmcclk(priv->scu);
		break;
	case ASPEED_CLK_EMMC:
		ast2600_enable_extemmcclk(priv->scu);
		break;
	case ASPEED_CLK_GATE_FSICLK:
		ast2600_enable_fsiclk(priv->scu);
		break;
	case ASPEED_CLK_GATE_USBPORT1CLK:
		ast2600_enable_usbahclk(priv->scu);
		break;
	case ASPEED_CLK_GATE_USBPORT2CLK:
		ast2600_enable_usbbhclk(priv->scu);
		break;
	case ASPEED_CLK_GATE_YCLK:
		ast2600_enable_haceclk(priv->scu);
		break;
	case ASPEED_CLK_GATE_RSACLK:
		ast2600_enable_rsaclk(priv->scu);
		break;
	default:
		debug("%s: unknown clk %ld\n", __func__, clk->id);
		return -ENOENT;
	}

	return 0;
}

struct aspeed_clks {
	ulong id;
	const char *name;
};

#if IS_ENABLED(CONFIG_CMD_CLK)
static struct aspeed_clks aspeed_clk_names[] = {
	{ ASPEED_CLK_HPLL, "hpll" },
	{ ASPEED_CLK_MPLL, "mpll" },
	{ ASPEED_CLK_APLL, "apll" },
	{ ASPEED_CLK_EPLL, "epll" },
	{ ASPEED_CLK_DPLL, "dpll" },
	{ ASPEED_CLK_AHB, "hclk" },
	{ ASPEED_CLK_APB1, "pclk1" },
	{ ASPEED_CLK_APB2, "pclk2" },
	{ ASPEED_CLK_BCLK, "bclk" },
	{ ASPEED_CLK_UARTX, "uxclk" },
	{ ASPEED_CLK_HUARTX, "huxclk" },
};

static void ast2600_clk_dump(struct udevice *dev)
{
	struct clk clk;
	unsigned long rate;
	int i, ret;

	printf("Clk\t\tHz\n");

	for (i = 0; i < ARRAY_SIZE(aspeed_clk_names); i++) {
		clk.id = aspeed_clk_names[i].id;
		ret = clk_request(dev, &clk);
		if (ret < 0) {
			debug("%s clk_request() failed: %d\n", __func__, ret);
			continue;
		}

		ret = clk_get_rate(&clk);
		rate = ret;

		if (ret == -EINVAL) {
			printf("clk ID %lu not supported yet\n",
			       aspeed_clk_names[i].id);
			continue;
		}
		if (ret < 0) {
			printf("%s %lu: get_rate err: %d\n", __func__,
			       aspeed_clk_names[i].id, ret);
			continue;
		}

		printf("%s(%3lu):\t%lu\n", aspeed_clk_names[i].name,
		       aspeed_clk_names[i].id, rate);
	}

	return 0;
}
#endif

struct clk_ops ast2600_clk_ops = {
	.get_rate = ast2600_clk_get_rate,
	.set_rate = ast2600_clk_set_rate,
	.enable = ast2600_clk_enable,
#if IS_ENABLED(CONFIG_CMD_CLK)
	.dump = ast2600_clk_dump,
#endif
};

static int ast2600_clk_probe(struct udevice *dev)
{
	struct ast2600_clk_priv *priv = dev_get_priv(dev);

	priv->scu = devfdt_get_addr_ptr(dev);
	if (IS_ERR(priv->scu))
		return PTR_ERR(priv->scu);

	ast2600_init_rgmii_clk(priv->scu, &rgmii_clk_defconfig);
	ast2600_init_rmii_clk(priv->scu, &rmii_clk_defconfig);
	ast2600_configure_mac12_clk(priv->scu);
	ast2600_configure_mac34_clk(priv->scu);
	ast2600_configure_rsa_ecc_clk(priv->scu);

	return 0;
}

static int ast2600_clk_bind(struct udevice *dev)
{
	int ret;

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(gd->dm_root, "ast_sysreset", "reset", &dev);
	if (ret)
		debug("Warning: No reset driver: ret=%d\n", ret);

	return 0;
}

static const struct udevice_id ast2600_clk_ids[] = {
	{ .compatible = "aspeed,ast2600-scu", },
	{ },
};

U_BOOT_DRIVER(aspeed_ast2600_scu) = {
	.name = "aspeed_ast2600_scu",
	.id = UCLASS_CLK,
	.of_match = ast2600_clk_ids,
	.priv_auto = sizeof(struct ast2600_clk_priv),
	.ops = &ast2600_clk_ops,
	.bind = ast2600_clk_bind,
	.probe = ast2600_clk_probe,
};
