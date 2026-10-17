/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _CLK_NPCM_H_
#define _CLK_NPCM_H_

#include <clk-uclass.h>

/* Register offsets */
#define CLKSEL		0x04	/* clock source selection */
#define CLKDIV1		0x08	/* clock divider 1 */
#define CLKDIV2		0x2C	/* clock divider 2 */
#define CLKDIV3		0x58	/* clock divider 3 */
#define PLLCON0		0x0C	/* pll0 control */
#define PLLCON1		0x10	/* pll1 control */
#define PLLCON2		0x54	/* pll2 control */

/* CLKSEL bit filed */
#define NPCM7XX_CPUCKSEL	GENMASK(1, 0)
#define NPCM8XX_CPUCKSEL	GENMASK(2, 0)
#define SDCKSEL		GENMASK(7, 6)
#define UARTCKSEL	GENMASK(9, 8)
#define TIMCKSEL	GENMASK(15, 14)

/* CLKDIV1 bit filed */
#define SPI3CKDIV	GENMASK(10, 6)
#define MMCCKDIV	GENMASK(15, 11)
#define UARTDIV1	GENMASK(20, 16)
#define TIMCKDIV	GENMASK(25, 21)
#define CLK4DIV		GENMASK(27, 26)

/* CLKDIV2 bit filed */
#define APB5CKDIV	GENMASK(23, 22)
#define APB2CKDIV	GENMASK(27, 26)

/* CLKDIV3 bit filed */
#define SPIXCKDIV	GENMASK(5, 1)
#define SPI0CKDIV	GENMASK(10, 6)
#define UARTDIV2	GENMASK(15, 11)
#define SPI1CKDIV	GENMASK(23, 16)

/* PLLCON bit filed */
#define PLLCON_INDV	GENMASK(5, 0)
#define PLLCON_OTDV1	GENMASK(10, 8)
#define PLLCON_OTDV2	GENMASK(15, 13)
#define PLLCON_FBDV	GENMASK(27, 16)

/* Flags */
#define DIV_TYPE1	BIT(0)	/* div = clkdiv + 1 */
#define DIV_TYPE2	BIT(1)	/* div = 1 << clkdiv */
#define PRE_DIV2	BIT(2)	/* Pre divisor = 2 */
#define POST_DIV2	BIT(3)	/* Post divisor = 2 */
#define FIXED_PARENT	BIT(4)	/* clock source is fixed */
#define DIV_RO		BIT(5)	/* divider is read-only */

/* Parameters of PLL configuration */
struct npcm_clk_pll {
	const int id;
	const int parent_id;
	u32 reg;
	u32 flags;
};

/* Parent clock id to clksel mapping */
struct parent_data {
	int id;
	int clksel;
};

/* Parameters of parent selection */
struct npcm_clk_select {
	const int id;
	const struct parent_data *parents;
	u32 reg;
	u32 mask;
	u8 num_parents;
	u32 flags;
};

/* Parameters of clock divider */
struct npcm_clk_div {
	const int id;
	u32 reg;
	u32 mask;
	u32 flags;
};

struct npcm_clk_data {
	struct npcm_clk_pll *clk_plls;
	int num_plls;
	struct npcm_clk_select *clk_selectors;
	int num_selectors;
	struct npcm_clk_div *clk_dividers;
	int num_dividers;
	int refclk_id;
	int pll0_id;
};

struct npcm_clk_priv {
	void __iomem *base;
	struct npcm_clk_data *clk_data;
	int num_clks;
};

extern const struct clk_ops npcm_clk_ops;

#endif
