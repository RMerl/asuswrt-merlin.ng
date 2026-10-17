/*
 * Synopsys HSDK SDP CGU clock driver
 *
 * Copyright (C) 2017 Synopsys
 * Author: Eugeniy Paltsev <Eugeniy.Paltsev@synopsys.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <common.h>
#include <clk-uclass.h>
#include <div64.h>
#include <dm.h>
#include <log.h>
#include <linux/bitops.h>
#include <linux/bug.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <asm/arcregs.h>
#include <linux/printk.h>

#include <dt-bindings/clock/snps,hsdk-cgu.h>

/*
 * Synopsys ARC HSDK clock tree.
 *
 *   ------------------
 *   | 33.33 MHz xtal |
 *   ------------------
 *            |
 *            |   -----------
 *            |-->| ARC PLL |
 *            |   -----------
 *            |        |
 *            |        |-->|CGU_ARC_IDIV|----------->
 *            |        |-->|CREG_CORE_IF_DIV|------->
 *            |
 *            |   --------------
 *            |-->| SYSTEM PLL |
 *            |   --------------
 *            |        |
 *            |        |-->|CGU_SYS_IDIV_APB|------->
 *            |        |-->|CGU_SYS_IDIV_AXI|------->
 *            |        |-->|CGU_SYS_IDIV_*|--------->
 *            |        |-->|CGU_SYS_IDIV_EBI_REF|--->
 *            |
 *            |   --------------
 *            |-->| TUNNEL PLL |
 *            |   --------------
 *            |        |
 *            |        |-->|CGU_TUN_IDIV_TUN|----------->
 *            |        |-->|CGU_TUN_IDIV_ROM|----------->
 *            |        |-->|CGU_TUN_IDIV_PWM|----------->
 *            |
 *            |   -----------
 *            |-->| DDR PLL |
 *                -----------
 *                     |
 *                     |---------------------------->
 *
 *   ------------------
 *   | 27.00 MHz xtal |
 *   ------------------
 *            |
 *            |   ------------
 *            |-->| HDMI PLL |
 *                ------------
 *                     |
 *                     |-->|CGU_HDMI_IDIV_APB|------>
 */

#define CGU_ARC_IDIV		0x080
#define CGU_TUN_IDIV_TUN	0x380
#define CGU_TUN_IDIV_ROM	0x390
#define CGU_TUN_IDIV_PWM	0x3A0
#define CGU_TUN_IDIV_TIMER	0x3B0
#define CGU_HDMI_IDIV_APB	0x480
#define CGU_SYS_IDIV_APB	0x180
#define CGU_SYS_IDIV_AXI	0x190
#define CGU_SYS_IDIV_ETH	0x1A0
#define CGU_SYS_IDIV_USB	0x1B0
#define CGU_SYS_IDIV_SDIO	0x1C0
#define CGU_SYS_IDIV_HDMI	0x1D0
#define CGU_SYS_IDIV_GFX_CORE	0x1E0
#define CGU_SYS_IDIV_GFX_DMA	0x1F0
#define CGU_SYS_IDIV_GFX_CFG	0x200
#define CGU_SYS_IDIV_DMAC_CORE	0x210
#define CGU_SYS_IDIV_DMAC_CFG	0x220
#define CGU_SYS_IDIV_SDIO_REF	0x230
#define CGU_SYS_IDIV_SPI_REF	0x240
#define CGU_SYS_IDIV_I2C_REF	0x250
#define CGU_SYS_IDIV_UART_REF	0x260
#define CGU_SYS_IDIV_EBI_REF	0x270

#define CGU_IDIV_MASK		0xFF /* All idiv have 8 significant bits */

#define CGU_ARC_PLL		0x0
#define CGU_SYS_PLL		0x10
#define CGU_DDR_PLL		0x20
#define CGU_TUN_PLL		0x30
#define CGU_HDMI_PLL		0x40

#define CGU_PLL_CTRL		0x000 /* ARC PLL control register */
#define CGU_PLL_STATUS		0x004 /* ARC PLL status register */
#define CGU_PLL_FMEAS		0x008 /* ARC PLL frequency measurement register */
#define CGU_PLL_MON		0x00C /* ARC PLL monitor register */

#define CGU_PLL_CTRL_ODIV_SHIFT		2
#define CGU_PLL_CTRL_IDIV_SHIFT		4
#define CGU_PLL_CTRL_FBDIV_SHIFT	9
#define CGU_PLL_CTRL_BAND_SHIFT		20

#define CGU_PLL_CTRL_ODIV_MASK		GENMASK(3, CGU_PLL_CTRL_ODIV_SHIFT)
#define CGU_PLL_CTRL_IDIV_MASK		GENMASK(8, CGU_PLL_CTRL_IDIV_SHIFT)
#define CGU_PLL_CTRL_FBDIV_MASK		GENMASK(15, CGU_PLL_CTRL_FBDIV_SHIFT)

#define CGU_PLL_CTRL_PD			BIT(0)
#define CGU_PLL_CTRL_BYPASS		BIT(1)

#define CGU_PLL_STATUS_LOCK		BIT(0)
#define CGU_PLL_STATUS_ERR		BIT(1)

#define HSDK_PLL_MAX_LOCK_TIME		100 /* 100 us */

#define CREG_CORE_IF_DIV		0x000 /* ARC CORE interface divider */
#define CORE_IF_CLK_THRESHOLD_HZ	500000000
#define CREG_CORE_IF_CLK_DIV_1		0x0
#define CREG_CORE_IF_CLK_DIV_2		0x1

#define MIN_PLL_RATE			100000000 /* 100 MHz */
#define PARENT_RATE_33			33333333 /* fixed clock - xtal */
#define PARENT_RATE_27			27000000 /* fixed clock - xtal */
#define CGU_MAX_CLOCKS			27

#define MAX_FREQ_VARIATIONS		6

struct hsdk_idiv_cfg {
	const u32 oft;
	const u8  val[MAX_FREQ_VARIATIONS];
};

struct hsdk_div_full_cfg {
	const u32 clk_rate[MAX_FREQ_VARIATIONS];
	const u32 pll_rate[MAX_FREQ_VARIATIONS];
	const struct hsdk_idiv_cfg idiv[];
};

static const struct hsdk_div_full_cfg hsdk_4xd_tun_clk_cfg = {
	{ 25000000,  50000000,  75000000,  100000000, 125000000, 150000000 },
	{ 600000000, 600000000, 600000000, 600000000, 750000000, 600000000 }, {
	{ CGU_TUN_IDIV_TUN,	{ 24,	12,	8,	6,	6,	4 } },
	{ CGU_TUN_IDIV_ROM,	{ 4,	4,	4,	4,	5,	4 } },
	{ CGU_TUN_IDIV_PWM,	{ 8,	8,	8,	8,	10,	8 } },
	{ CGU_TUN_IDIV_TIMER,	{ 12,	12,	12,	12,	15,	12 } },
	{ /* last one */ }
	}
};

static const struct hsdk_div_full_cfg hsdk_tun_clk_cfg = {
	{ 25000000,  50000000,  75000000,  100000000, 125000000, 150000000 },
	{ 600000000, 600000000, 600000000, 600000000, 750000000, 600000000 }, {
	{ CGU_TUN_IDIV_TUN,	{ 24,	12,	8,	6,	6,	4 } },
	{ CGU_TUN_IDIV_ROM,	{ 4,	4,	4,	4,	5,	4 } },
	{ CGU_TUN_IDIV_PWM,	{ 8,	8,	8,	8,	10,	8 } },
	{ /* last one */ }
	}
};

static const struct hsdk_div_full_cfg axi_clk_cfg = {
	{ 200000000,	400000000,	600000000,	800000000 },
	{ 800000000,	800000000,	600000000,	800000000 }, {
	{ CGU_SYS_IDIV_APB,	 { 4,	4,	3,	4 } },	/* APB */
	{ CGU_SYS_IDIV_AXI,	 { 4,	2,	1,	1 } },	/* AXI */
	{ CGU_SYS_IDIV_ETH,	 { 2,	2,	2,	2 } },	/* ETH */
	{ CGU_SYS_IDIV_USB,	 { 2,	2,	2,	2 } },	/* USB */
	{ CGU_SYS_IDIV_SDIO,	 { 2,	2,	2,	2 } },	/* SDIO */
	{ CGU_SYS_IDIV_HDMI,	 { 2,	2,	2,	2 } },	/* HDMI */
	{ CGU_SYS_IDIV_GFX_CORE, { 1,	1,	1,	1 } },	/* GPU-CORE */
	{ CGU_SYS_IDIV_GFX_DMA,	 { 2,	2,	2,	2 } },	/* GPU-DMA */
	{ CGU_SYS_IDIV_GFX_CFG,	 { 4,	4,	3,	4 } },	/* GPU-CFG */
	{ CGU_SYS_IDIV_DMAC_CORE,{ 2,	2,	2,	2 } },	/* DMAC-CORE */
	{ CGU_SYS_IDIV_DMAC_CFG, { 4,	4,	3,	4 } },	/* DMAC-CFG */
	{ CGU_SYS_IDIV_SDIO_REF, { 8,	8,	6,	8 } },	/* SDIO-REF */
	{ CGU_SYS_IDIV_SPI_REF,	 { 24,	24,	18,	24 } },	/* SPI-REF */
	{ CGU_SYS_IDIV_I2C_REF,	 { 4,	4,	3,	4 } },	/* I2C-REF */
	{ CGU_SYS_IDIV_UART_REF, { 24,	24,	18,	24 } },	/* UART-REF */
	{ CGU_SYS_IDIV_EBI_REF,	 { 16,	16,	12,	16 } },	/* EBI-REF */
	{ /* last one */ }
	}
};

struct hsdk_pll_cfg {
	const u32 rate;
	const u8  idiv;
	const u8  fbdiv;
	const u8  odiv;
	const u8  band;
};

static const struct hsdk_pll_cfg asdt_pll_cfg[] = {
	{ 100000000,  0, 11, 3, 0 },
	{ 125000000,  0, 14, 3, 0 },
	{ 133000000,  0, 15, 3, 0 },
	{ 150000000,  0, 17, 3, 0 },
	{ 200000000,  1, 47, 3, 0 },
	{ 233000000,  1, 27, 2, 0 },
	{ 300000000,  1, 35, 2, 0 },
	{ 333000000,  1, 39, 2, 0 },
	{ 400000000,  1, 47, 2, 0 },
	{ 500000000,  0, 14, 1, 0 },
	{ 600000000,  0, 17, 1, 0 },
	{ 700000000,  0, 20, 1, 0 },
	{ 750000000,  1, 44, 1, 0 },
	{ 800000000,  0, 23, 1, 0 },
	{ 900000000,  1, 26, 0, 0 },
	{ 1000000000, 1, 29, 0, 0 },
	{ 1100000000, 1, 32, 0, 0 },
	{ 1200000000, 1, 35, 0, 0 },
	{ 1300000000, 1, 38, 0, 0 },
	{ 1400000000, 1, 41, 0, 0 },
	{ 1500000000, 1, 44, 0, 0 },
	{ 1600000000, 1, 47, 0, 0 },
	{}
};

static const struct hsdk_pll_cfg hdmi_pll_cfg[] = {
	{ 297000000,  0, 21, 2, 0 },
	{ 540000000,  0, 19, 1, 0 },
	{ 594000000,  0, 21, 1, 0 },
	{}
};

struct hsdk_cgu_domain {
	/* PLLs registers */
	void __iomem *pll_regs;
	/* PLLs special registers */
	void __iomem *spec_regs;
	/* PLLs devdata */
	const struct hsdk_pll_devdata *pll;

	/* Dividers registers */
	void __iomem *idiv_regs;
};

struct hsdk_cgu_clk {
	const struct cgu_clk_map *map;
	/* CGU block register */
	void __iomem *cgu_regs;
	/* CREG block register */
	void __iomem *creg_regs;

	/* The domain we are working with */
	struct hsdk_cgu_domain curr_domain;
};

struct hsdk_pll_devdata {
	const u32 parent_rate;
	const struct hsdk_pll_cfg *const pll_cfg;
	const int (*const update_rate)(struct hsdk_cgu_clk *clk,
				       unsigned long rate,
				       const struct hsdk_pll_cfg *cfg);
};

static int hsdk_pll_core_update_rate(struct hsdk_cgu_clk *, unsigned long,
				     const struct hsdk_pll_cfg *);
static int hsdk_pll_comm_update_rate(struct hsdk_cgu_clk *, unsigned long,
				     const struct hsdk_pll_cfg *);

static const struct hsdk_pll_devdata core_pll_dat = {
	.parent_rate = PARENT_RATE_33,
	.pll_cfg = asdt_pll_cfg,
	.update_rate = hsdk_pll_core_update_rate,
};

static const struct hsdk_pll_devdata sdt_pll_dat = {
	.parent_rate = PARENT_RATE_33,
	.pll_cfg = asdt_pll_cfg,
	.update_rate = hsdk_pll_comm_update_rate,
};

static const struct hsdk_pll_devdata hdmi_pll_dat = {
	.parent_rate = PARENT_RATE_27,
	.pll_cfg = hdmi_pll_cfg,
	.update_rate = hsdk_pll_comm_update_rate,
};

static ulong idiv_set(struct clk *, ulong);
static ulong cpu_clk_set(struct clk *, ulong);
static ulong axi_clk_set(struct clk *, ulong);
static ulong tun_hsdk_set(struct clk *, ulong);
static ulong tun_h4xd_set(struct clk *, ulong);
static ulong idiv_get(struct clk *);
static int idiv_off(struct clk *);
static ulong pll_set(struct clk *, ulong);
static ulong pll_get(struct clk *);

struct cgu_clk_map {
	const u32 cgu_pll_oft;
	const u32 cgu_div_oft;
	const struct hsdk_pll_devdata *const pll_devdata;
	const ulong (*const get_rate)(struct clk *clk);
	const ulong (*const set_rate)(struct clk *clk, ulong rate);
	const int (*const disable)(struct clk *clk);
};

static const struct cgu_clk_map hsdk_clk_map[] = {
	[CLK_ARC_PLL]        = { CGU_ARC_PLL,  0,                      &core_pll_dat, pll_get,  pll_set,      NULL     },
	[CLK_ARC]            = { CGU_ARC_PLL,  CGU_ARC_IDIV,           &core_pll_dat, idiv_get, cpu_clk_set,  idiv_off },
	[CLK_DDR_PLL]        = { CGU_DDR_PLL,  0,                      &sdt_pll_dat,  pll_get,  pll_set,      NULL     },
	[CLK_SYS_PLL]        = { CGU_SYS_PLL,  0,                      &sdt_pll_dat,  pll_get,  pll_set,      NULL     },
	[CLK_SYS_APB]        = { CGU_SYS_PLL,  CGU_SYS_IDIV_APB,       &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_AXI]        = { CGU_SYS_PLL,  CGU_SYS_IDIV_AXI,       &sdt_pll_dat,  idiv_get, axi_clk_set,  idiv_off },
	[CLK_SYS_ETH]        = { CGU_SYS_PLL,  CGU_SYS_IDIV_ETH,       &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_USB]        = { CGU_SYS_PLL,  CGU_SYS_IDIV_USB,       &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_SDIO]       = { CGU_SYS_PLL,  CGU_SYS_IDIV_SDIO,      &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_HDMI]       = { CGU_SYS_PLL,  CGU_SYS_IDIV_HDMI,      &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_GFX_CORE]   = { CGU_SYS_PLL,  CGU_SYS_IDIV_GFX_CORE,  &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_GFX_DMA]    = { CGU_SYS_PLL,  CGU_SYS_IDIV_GFX_DMA,   &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_GFX_CFG]    = { CGU_SYS_PLL,  CGU_SYS_IDIV_GFX_CFG,   &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_DMAC_CORE]  = { CGU_SYS_PLL,  CGU_SYS_IDIV_DMAC_CORE, &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_DMAC_CFG]   = { CGU_SYS_PLL,  CGU_SYS_IDIV_DMAC_CFG,  &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_SDIO_REF]   = { CGU_SYS_PLL,  CGU_SYS_IDIV_SDIO_REF,  &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_SPI_REF]    = { CGU_SYS_PLL,  CGU_SYS_IDIV_SPI_REF,   &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_I2C_REF]    = { CGU_SYS_PLL,  CGU_SYS_IDIV_I2C_REF,   &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_UART_REF]   = { CGU_SYS_PLL,  CGU_SYS_IDIV_UART_REF,  &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_EBI_REF]    = { CGU_SYS_PLL,  CGU_SYS_IDIV_EBI_REF,   &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_TUN_PLL]        = { CGU_TUN_PLL,  0,                      &sdt_pll_dat,  pll_get,  pll_set,      NULL     },
	[CLK_TUN_TUN]        = { CGU_TUN_PLL,  CGU_TUN_IDIV_TUN,       &sdt_pll_dat,  idiv_get, tun_hsdk_set, idiv_off },
	[CLK_TUN_ROM]        = { CGU_TUN_PLL,  CGU_TUN_IDIV_ROM,       &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_TUN_PWM]        = { CGU_TUN_PLL,  CGU_TUN_IDIV_PWM,       &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_TUN_TIMER]      = { /* missing in HSDK */ },
	[CLK_HDMI_PLL]       = { CGU_HDMI_PLL, 0,                      &hdmi_pll_dat, pll_get,  pll_set,      NULL     },
	[CLK_HDMI]           = { CGU_HDMI_PLL, CGU_HDMI_IDIV_APB,      &hdmi_pll_dat, idiv_get, idiv_set,     idiv_off }
};

static const struct cgu_clk_map hsdk_4xd_clk_map[] = {
	[CLK_ARC_PLL]        = { CGU_ARC_PLL,  0,                      &core_pll_dat, pll_get,  pll_set,      NULL     },
	[CLK_ARC]            = { CGU_ARC_PLL,  CGU_ARC_IDIV,           &core_pll_dat, idiv_get, cpu_clk_set,  idiv_off },
	[CLK_DDR_PLL]        = { CGU_DDR_PLL,  0,                      &sdt_pll_dat,  pll_get,  pll_set,      NULL     },
	[CLK_SYS_PLL]        = { CGU_SYS_PLL,  0,                      &sdt_pll_dat,  pll_get,  pll_set,      NULL     },
	[CLK_SYS_APB]        = { CGU_SYS_PLL,  CGU_SYS_IDIV_APB,       &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_AXI]        = { CGU_SYS_PLL,  CGU_SYS_IDIV_AXI,       &sdt_pll_dat,  idiv_get, axi_clk_set,  idiv_off },
	[CLK_SYS_ETH]        = { CGU_SYS_PLL,  CGU_SYS_IDIV_ETH,       &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_USB]        = { CGU_SYS_PLL,  CGU_SYS_IDIV_USB,       &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_SDIO]       = { CGU_SYS_PLL,  CGU_SYS_IDIV_SDIO,      &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_HDMI]       = { CGU_SYS_PLL,  CGU_SYS_IDIV_HDMI,      &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_GFX_CORE]   = { CGU_SYS_PLL,  CGU_SYS_IDIV_GFX_CORE,  &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_GFX_DMA]    = { /* missing in HSDK-4xD */ },
	[CLK_SYS_GFX_CFG]    = { /* missing in HSDK-4xD */ },
	[CLK_SYS_DMAC_CORE]  = { CGU_SYS_PLL,  CGU_SYS_IDIV_DMAC_CORE, &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_DMAC_CFG]   = { CGU_SYS_PLL,  CGU_SYS_IDIV_DMAC_CFG,  &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_SDIO_REF]   = { CGU_SYS_PLL,  CGU_SYS_IDIV_SDIO_REF,  &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_SPI_REF]    = { CGU_SYS_PLL,  CGU_SYS_IDIV_SPI_REF,   &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_I2C_REF]    = { CGU_SYS_PLL,  CGU_SYS_IDIV_I2C_REF,   &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_UART_REF]   = { CGU_SYS_PLL,  CGU_SYS_IDIV_UART_REF,  &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_SYS_EBI_REF]    = { CGU_SYS_PLL,  CGU_SYS_IDIV_EBI_REF,   &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_TUN_PLL]        = { CGU_TUN_PLL,  0,                      &sdt_pll_dat,  pll_get,  pll_set,      NULL     },
	[CLK_TUN_TUN]        = { CGU_TUN_PLL,  CGU_TUN_IDIV_TUN,       &sdt_pll_dat,  idiv_get, tun_h4xd_set, idiv_off },
	[CLK_TUN_ROM]        = { CGU_TUN_PLL,  CGU_TUN_IDIV_ROM,       &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_TUN_PWM]        = { CGU_TUN_PLL,  CGU_TUN_IDIV_PWM,       &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_TUN_TIMER]      = { CGU_TUN_PLL,  CGU_TUN_IDIV_TIMER,     &sdt_pll_dat,  idiv_get, idiv_set,     idiv_off },
	[CLK_HDMI_PLL]       = { CGU_HDMI_PLL, 0,                      &hdmi_pll_dat, pll_get,  pll_set,      NULL     },
	[CLK_HDMI]           = { CGU_HDMI_PLL, CGU_HDMI_IDIV_APB,      &hdmi_pll_dat, idiv_get, idiv_set,     idiv_off }
};

static inline void hsdk_idiv_write(struct hsdk_cgu_clk *clk, u32 val)
{
	iowrite32(val, clk->curr_domain.idiv_regs);
}

static inline u32 hsdk_idiv_read(struct hsdk_cgu_clk *clk)
{
	return ioread32(clk->curr_domain.idiv_regs);
}

static inline void hsdk_pll_write(struct hsdk_cgu_clk *clk, u32 reg, u32 val)
{
	iowrite32(val, clk->curr_domain.pll_regs + reg);
}

static inline u32 hsdk_pll_read(struct hsdk_cgu_clk *clk, u32 reg)
{
	return ioread32(clk->curr_domain.pll_regs + reg);
}

static inline void hsdk_pll_spcwrite(struct hsdk_cgu_clk *clk, u32 reg, u32 val)
{
	iowrite32(val, clk->curr_domain.spec_regs + reg);
}

static inline u32 hsdk_pll_spcread(struct hsdk_cgu_clk *clk, u32 reg)
{
	return ioread32(clk->curr_domain.spec_regs + reg);
}

static inline void hsdk_pll_set_cfg(struct hsdk_cgu_clk *clk,
				    const struct hsdk_pll_cfg *cfg)
{
	u32 val = 0;

	/* Powerdown and Bypass bits should be cleared */
	val |= (u32)cfg->idiv << CGU_PLL_CTRL_IDIV_SHIFT;
	val |= (u32)cfg->fbdiv << CGU_PLL_CTRL_FBDIV_SHIFT;
	val |= (u32)cfg->odiv << CGU_PLL_CTRL_ODIV_SHIFT;
	val |= (u32)cfg->band << CGU_PLL_CTRL_BAND_SHIFT;

	pr_debug("write configurarion: %#x\n", val);

	hsdk_pll_write(clk, CGU_PLL_CTRL, val);
}

static inline bool hsdk_pll_is_locked(struct hsdk_cgu_clk *clk)
{
	return !!(hsdk_pll_read(clk, CGU_PLL_STATUS) & CGU_PLL_STATUS_LOCK);
}

static inline bool hsdk_pll_is_err(struct hsdk_cgu_clk *clk)
{
	return !!(hsdk_pll_read(clk, CGU_PLL_STATUS) & CGU_PLL_STATUS_ERR);
}

static ulong pll_get(struct clk *sclk)
{
	u32 val;
	u64 rate;
	u32 idiv, fbdiv, odiv;
	struct hsdk_cgu_clk *clk = dev_get_priv(sclk->dev);
	u32 parent_rate = clk->curr_domain.pll->parent_rate;

	val = hsdk_pll_read(clk, CGU_PLL_CTRL);

	pr_debug("current configurarion: %#x\n", val);

	/* Check if PLL is bypassed */
	if (val & CGU_PLL_CTRL_BYPASS)
		return parent_rate;

	/* Check if PLL is disabled */
	if (val & CGU_PLL_CTRL_PD)
		return 0;

	/* input divider = reg.idiv + 1 */
	idiv = 1 + ((val & CGU_PLL_CTRL_IDIV_MASK) >> CGU_PLL_CTRL_IDIV_SHIFT);
	/* fb divider = 2*(reg.fbdiv + 1) */
	fbdiv = 2 * (1 + ((val & CGU_PLL_CTRL_FBDIV_MASK) >> CGU_PLL_CTRL_FBDIV_SHIFT));
	/* output divider = 2^(reg.odiv) */
	odiv = 1 << ((val & CGU_PLL_CTRL_ODIV_MASK) >> CGU_PLL_CTRL_ODIV_SHIFT);

	rate = (u64)parent_rate * fbdiv;
	do_div(rate, idiv * odiv);

	return rate;
}

static unsigned long hsdk_pll_round_rate(struct clk *sclk, unsigned long rate)
{
	int i;
	unsigned long best_rate;
	struct hsdk_cgu_clk *clk = dev_get_priv(sclk->dev);
	const struct hsdk_pll_cfg *pll_cfg = clk->curr_domain.pll->pll_cfg;

	if (pll_cfg[0].rate == 0)
		return -EINVAL;

	best_rate = pll_cfg[0].rate;

	for (i = 1; pll_cfg[i].rate != 0; i++) {
		if (abs(rate - pll_cfg[i].rate) < abs(rate - best_rate))
			best_rate = pll_cfg[i].rate;
	}

	pr_debug("chosen best rate: %lu\n", best_rate);

	return best_rate;
}

static int hsdk_pll_comm_update_rate(struct hsdk_cgu_clk *clk,
				     unsigned long rate,
				     const struct hsdk_pll_cfg *cfg)
{
	hsdk_pll_set_cfg(clk, cfg);

	/*
	 * Wait until CGU relocks and check error status.
	 * If after timeout CGU is unlocked yet return error.
	 */
	udelay(HSDK_PLL_MAX_LOCK_TIME);
	if (!hsdk_pll_is_locked(clk))
		return -ETIMEDOUT;

	if (hsdk_pll_is_err(clk))
		return -EINVAL;

	return 0;
}

static int hsdk_pll_core_update_rate(struct hsdk_cgu_clk *clk,
				     unsigned long rate,
				     const struct hsdk_pll_cfg *cfg)
{
	/*
	 * When core clock exceeds 500MHz, the divider for the interface
	 * clock must be programmed to div-by-2.
	 */
	if (rate > CORE_IF_CLK_THRESHOLD_HZ)
		hsdk_pll_spcwrite(clk, CREG_CORE_IF_DIV, CREG_CORE_IF_CLK_DIV_2);

	hsdk_pll_set_cfg(clk, cfg);

	/*
	 * Wait until CGU relocks and check error status.
	 * If after timeout CGU is unlocked yet return error.
	 */
	udelay(HSDK_PLL_MAX_LOCK_TIME);
	if (!hsdk_pll_is_locked(clk))
		return -ETIMEDOUT;

	if (hsdk_pll_is_err(clk))
		return -EINVAL;

	/*
	 * Program divider to div-by-1 if we succesfuly set core clock below
	 * 500MHz threshold.
	 */
	if (rate <= CORE_IF_CLK_THRESHOLD_HZ)
		hsdk_pll_spcwrite(clk, CREG_CORE_IF_DIV, CREG_CORE_IF_CLK_DIV_1);

	return 0;
}

static ulong pll_set(struct clk *sclk, ulong rate)
{
	int i;
	unsigned long best_rate;
	struct hsdk_cgu_clk *clk = dev_get_priv(sclk->dev);
	const struct hsdk_pll_devdata *pll = clk->curr_domain.pll;
	const struct hsdk_pll_cfg *pll_cfg = pll->pll_cfg;

	best_rate = hsdk_pll_round_rate(sclk, rate);

	for (i = 0; pll_cfg[i].rate != 0; i++)
		if (pll_cfg[i].rate == best_rate)
			return pll->update_rate(clk, best_rate, &pll_cfg[i]);

	pr_err("invalid rate=%ld Hz, parent_rate=%d Hz\n", best_rate,
	       pll->parent_rate);

	return -EINVAL;
}

static int idiv_off(struct clk *sclk)
{
	struct hsdk_cgu_clk *clk = dev_get_priv(sclk->dev);

	hsdk_idiv_write(clk, 0);

	return 0;
}

static ulong idiv_get(struct clk *sclk)
{
	struct hsdk_cgu_clk *clk = dev_get_priv(sclk->dev);
	ulong parent_rate = pll_get(sclk);
	u32 div_factor = hsdk_idiv_read(clk);

	div_factor &= CGU_IDIV_MASK;

	pr_debug("current configurarion: %#x (%d)\n", div_factor, div_factor);

	if (div_factor == 0)
		return 0;

	return parent_rate / div_factor;
}

/* Special behavior: wen we set this clock we set both idiv and pll */
static ulong cpu_clk_set(struct clk *sclk, ulong rate)
{
	ulong ret;

	ret = pll_set(sclk, rate);
	idiv_set(sclk, rate);

	return ret;
}

/*
 * Special behavior:
 * when we set these clocks we set both PLL and all idiv dividers related to
 * this PLL domain.
 */
static ulong common_div_clk_set(struct clk *sclk, ulong rate,
				const struct hsdk_div_full_cfg *cfg)
{
	struct hsdk_cgu_clk *clk = dev_get_priv(sclk->dev);
	ulong pll_rate;
	int i, freq_idx = -1;
	ulong ret = 0;

	pll_rate = pll_get(sclk);

	for (i = 0; i < MAX_FREQ_VARIATIONS; i++) {
		/* unused freq variations are filled with 0 */
		if (!cfg->clk_rate[i])
			break;

		if (cfg->clk_rate[i] == rate) {
			freq_idx = i;
			break;
		}
	}

	if (freq_idx < 0) {
		pr_err("clk: invalid rate=%ld Hz\n", rate);
		return -EINVAL;
	}

	/* configure PLL before dividers */
	if (cfg->pll_rate[freq_idx] < pll_rate)
		ret = pll_set(sclk, cfg->pll_rate[freq_idx]);

	/* configure SYS dividers */
	for (i = 0; cfg->idiv[i].oft != 0; i++) {
		clk->curr_domain.idiv_regs = clk->cgu_regs + cfg->idiv[i].oft;
		hsdk_idiv_write(clk, cfg->idiv[i].val[freq_idx]);
	}

	/* configure PLL after dividers */
	if (cfg->pll_rate[freq_idx] >= pll_rate)
		ret = pll_set(sclk, cfg->pll_rate[freq_idx]);

	return ret;
}

static ulong axi_clk_set(struct clk *sclk, ulong rate)
{
	return common_div_clk_set(sclk, rate, &axi_clk_cfg);
}

static ulong tun_hsdk_set(struct clk *sclk, ulong rate)
{
	return common_div_clk_set(sclk, rate, &hsdk_tun_clk_cfg);
}

static ulong tun_h4xd_set(struct clk *sclk, ulong rate)
{
	return common_div_clk_set(sclk, rate, &hsdk_4xd_tun_clk_cfg);
}

static ulong idiv_set(struct clk *sclk, ulong rate)
{
	struct hsdk_cgu_clk *clk = dev_get_priv(sclk->dev);
	ulong parent_rate = pll_get(sclk);
	u32 div_factor;

	div_factor = parent_rate / rate;
	if (abs(rate - parent_rate / (div_factor + 1)) <=
	    abs(rate - parent_rate / div_factor)) {
		div_factor += 1;
	}

	if (div_factor & ~CGU_IDIV_MASK) {
		pr_err("invalid rate=%ld Hz, parent_rate=%ld Hz, div=%d: max divider valie is%d\n",
		       rate, parent_rate, div_factor, CGU_IDIV_MASK);

		div_factor = CGU_IDIV_MASK;
	}

	if (div_factor == 0) {
		pr_err("invalid rate=%ld Hz, parent_rate=%ld Hz, div=%d: min divider valie is 1\n",
		       rate, parent_rate, div_factor);

		div_factor = 1;
	}

	hsdk_idiv_write(clk, div_factor);

	return 0;
}

static int hsdk_prepare_clock_tree_branch(struct clk *sclk)
{
	struct hsdk_cgu_clk *clk = dev_get_priv(sclk->dev);

	if (sclk->id >= CGU_MAX_CLOCKS)
		return -EINVAL;

	/* clocks missing in current map have their entry zeroed */
	if (!clk->map[sclk->id].pll_devdata)
		return -EINVAL;

	clk->curr_domain.pll = clk->map[sclk->id].pll_devdata;
	clk->curr_domain.pll_regs = clk->cgu_regs + clk->map[sclk->id].cgu_pll_oft;
	clk->curr_domain.spec_regs = clk->creg_regs;
	clk->curr_domain.idiv_regs = clk->cgu_regs + clk->map[sclk->id].cgu_div_oft;

	return 0;
}

static ulong hsdk_cgu_get_rate(struct clk *sclk)
{
	struct hsdk_cgu_clk *clk = dev_get_priv(sclk->dev);

	if (hsdk_prepare_clock_tree_branch(sclk))
		return -EINVAL;

	return clk->map[sclk->id].get_rate(sclk);
}

static ulong hsdk_cgu_set_rate(struct clk *sclk, ulong rate)
{
	struct hsdk_cgu_clk *clk = dev_get_priv(sclk->dev);

	if (hsdk_prepare_clock_tree_branch(sclk))
		return -EINVAL;

	if (clk->map[sclk->id].set_rate)
		return clk->map[sclk->id].set_rate(sclk, rate);

	return -EINVAL;
}

static int hsdk_cgu_disable(struct clk *sclk)
{
	struct hsdk_cgu_clk *clk = dev_get_priv(sclk->dev);

	if (hsdk_prepare_clock_tree_branch(sclk))
		return -EINVAL;

	if (clk->map[sclk->id].disable)
		return clk->map[sclk->id].disable(sclk);

	return -EINVAL;
}

static const struct clk_ops hsdk_cgu_ops = {
	.set_rate = hsdk_cgu_set_rate,
	.get_rate = hsdk_cgu_get_rate,
	.disable = hsdk_cgu_disable,
};

static int hsdk_cgu_clk_probe(struct udevice *dev)
{
	struct hsdk_cgu_clk *hsdk_clk = dev_get_priv(dev);

	BUILD_BUG_ON(ARRAY_SIZE(hsdk_clk_map) != CGU_MAX_CLOCKS);
	BUILD_BUG_ON(ARRAY_SIZE(hsdk_4xd_clk_map) != CGU_MAX_CLOCKS);

	/* Choose which clock map to use in runtime */
	if ((read_aux_reg(ARC_AUX_IDENTITY) & 0xFF) == 0x52)
		hsdk_clk->map = hsdk_clk_map;
	else
		hsdk_clk->map = hsdk_4xd_clk_map;

	hsdk_clk->cgu_regs = devfdt_get_addr_index_ptr(dev, 0);
	if (!hsdk_clk->cgu_regs)
		return -EINVAL;

	hsdk_clk->creg_regs = devfdt_get_addr_index_ptr(dev, 1);
	if (!hsdk_clk->creg_regs)
		return -EINVAL;

	return 0;
}

static const struct udevice_id hsdk_cgu_clk_id[] = {
	{ .compatible = "snps,hsdk-cgu-clock" },
	{ }
};

U_BOOT_DRIVER(hsdk_cgu_clk) = {
	.name = "hsdk-cgu-clk",
	.id = UCLASS_CLK,
	.of_match = hsdk_cgu_clk_id,
	.probe = hsdk_cgu_clk_probe,
	.priv_auto	= sizeof(struct hsdk_cgu_clk),
	.ops = &hsdk_cgu_ops,
};
