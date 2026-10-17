// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Claudiu Beznea <claudiu.beznea@microchip.com>
 *
 * Based on sam9x60.c on Linux.
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dt-bindings/clk/at91.h>
#include <linux/clk-provider.h>

#include "pmc.h"

/**
 * Clock identifiers to be used in conjunction with macros like
 * AT91_TO_CLK_ID()
 *
 * @ID_MD_SLCK:			TD slow clock identifier
 * @ID_TD_SLCK:			MD slow clock identifier
 * @ID_MAIN_XTAL:		Main Xtal clock identifier
 * @ID_MAIN_RC:			Main RC clock identifier
 * @ID_MAIN_RC_OSC:		Main RC Oscillator clock identifier
 * @ID_MAIN_OSC:		Main Oscillator clock identifier
 * @ID_MAINCK:			MAINCK clock identifier
 * @ID_PLL_U_FRAC:		UPLL fractional clock identifier
 * @ID_PLL_U_DIV:		UPLL divider clock identifier
 * @ID_PLL_A_FRAC:		APLL fractional clock identifier
 * @ID_PLL_A_DIV:		APLL divider clock identifier

 * @ID_MCK_DIV:			MCK DIV clock identifier

 * @ID_UTMI:			UTMI clock identifier

 * @ID_PROG0:			Programmable 0 clock identifier
 * @ID_PROG1:			Programmable 1 clock identifier

 * @ID_PCK0:			PCK0 system clock identifier
 * @ID_PCK1:			PCK1 system clock identifier
 * @ID_DDR:			DDR system clock identifier
 * @ID_QSPI:			QSPI system clock identifier
 *
 * @ID_MCK_PRES:		MCK PRES clock identifier
 *
 * Note: if changing the values of this enums please sync them with
 *       device tree
 */
enum pmc_clk_ids {
	ID_MD_SLCK		= 0,
	ID_TD_SLCK		= 1,
	ID_MAIN_XTAL		= 2,
	ID_MAIN_RC		= 3,
	ID_MAIN_RC_OSC		= 4,
	ID_MAIN_OSC		= 5,
	ID_MAINCK		= 6,

	ID_PLL_U_FRAC		= 7,
	ID_PLL_U_DIV		= 8,
	ID_PLL_A_FRAC		= 9,
	ID_PLL_A_DIV		= 10,

	ID_MCK_DIV		= 11,

	ID_UTMI			= 12,

	ID_PROG0		= 13,
	ID_PROG1		= 14,

	ID_PCK0			= 15,
	ID_PCK1			= 16,

	ID_DDR			= 17,
	ID_QSPI			= 18,

	ID_MCK_PRES		= 19,
	ID_USBCK		= 20,
	ID_UHPCK		= 21,

	ID_MAX,
};

/**
 * PLL type identifiers
 * @PLL_TYPE_FRAC:	fractional PLL identifier
 * @PLL_TYPE_DIV:	divider PLL identifier
 */
enum pll_type {
	PLL_TYPE_FRAC,
	PLL_TYPE_DIV,
};

/* Clock names used as parents for multiple clocks. */
static const char *clk_names[] = {
	[ID_MAIN_RC_OSC]	= "main_rc_osc",
	[ID_MAIN_OSC]		= "main_osc",
	[ID_MAINCK]		= "mainck",
	[ID_PLL_U_DIV]		= "upll_divpmcck",
	[ID_PLL_A_DIV]		= "plla_divpmcck",
	[ID_MCK_PRES]		= "mck_pres",
	[ID_MCK_DIV]		= "mck_div",
	[ID_USBCK]		= "usbck",
};

/* Fractional PLL output range. */
static const struct clk_range plla_outputs[] = {
	{ .min = 2343750, .max = 1200000000 },
};

static const struct clk_range upll_outputs[] = {
	{ .min = 300000000, .max = 500000000 },
};

/* PLL characteristics. */
static const struct clk_pll_characteristics apll_characteristics = {
	.input = { .min = 12000000, .max = 48000000 },
	.num_output = ARRAY_SIZE(plla_outputs),
	.output = plla_outputs,
};

static const struct clk_pll_characteristics upll_characteristics = {
	.input = { .min = 12000000, .max = 48000000 },
	.num_output = ARRAY_SIZE(upll_outputs),
	.output = upll_outputs,
	.upll = true,
};

/* Layout for fractional PLLs. */
static const struct clk_pll_layout pll_layout_frac = {
	.mul_mask = GENMASK(31, 24),
	.frac_mask = GENMASK(21, 0),
	.mul_shift = 24,
	.frac_shift = 0,
};

/* Layout for DIV PLLs. */
static const struct clk_pll_layout pll_layout_div = {
	.div_mask = GENMASK(7, 0),
	.endiv_mask = BIT(29),
	.div_shift = 0,
	.endiv_shift = 29,
};

/* MCK characteristics. */
static const struct clk_master_characteristics mck_characteristics = {
	.output = { .min = 140000000, .max = 200000000 },
	.divisors = { 1, 2, 4, 3 },
	.have_div3_pres = 1,
};

/* MCK layout. */
static const struct clk_master_layout mck_layout = {
	.mask = 0x373,
	.pres_shift = 4,
	.offset = 0x28,
};

/* Programmable clock layout. */
static const struct clk_programmable_layout programmable_layout = {
	.pres_mask = 0xff,
	.pres_shift = 8,
	.css_mask = 0x1f,
	.have_slck_mck = 0,
	.is_pres_direct = 1,
};

/* Peripheral clock layout. */
static const struct clk_pcr_layout pcr_layout = {
	.offset = 0x88,
	.cmd = BIT(31),
	.gckcss_mask = GENMASK(12, 8),
	.pid_mask = GENMASK(6, 0),
};

/* USB clock layout */
static const struct clk_usbck_layout usbck_layout = {
	.offset = 0x38,
	.usbs_mask = GENMASK(1, 0),
	.usbdiv_mask = GENMASK(11, 8),
};

/**
 * PLL clocks description
 * @n:		clock name
 * @p:		clock parent
 * @l:		clock layout
 * @t:		clock type
 * @f:		true if clock is fixed and not changeable by driver
 * @id:		clock id corresponding to PLL driver
 * @cid:	clock id corresponding to clock subsystem
 */
static const struct {
	const char *n;
	const char *p;
	const struct clk_pll_layout *l;
	const struct clk_pll_characteristics *c;
	u8 t;
	u8 f;
	u8 id;
	u8 cid;
} sam9x60_plls[] = {
	{
		.n = "plla_fracck",
		.p = "mainck",
		.l = &pll_layout_frac,
		.c = &apll_characteristics,
		.t = PLL_TYPE_FRAC,
		.f = 1,
		.id = 0,
		.cid = ID_PLL_A_FRAC,
	},

	{
		.n = "plla_divpmcck",
		.p = "plla_fracck",
		.l = &pll_layout_div,
		.c = &apll_characteristics,
		.t = PLL_TYPE_DIV,
		.f = 1,
		.id = 0,
		.cid = ID_PLL_A_DIV,
	},

	{
		.n = "upll_fracck",
		.p = "main_osc",
		.l = &pll_layout_frac,
		.c = &upll_characteristics,
		.t = PLL_TYPE_FRAC,
		.f = 1,
		.id = 1,
		.cid = ID_PLL_U_FRAC,
	},

	{
		.n = "upll_divpmcck",
		.p = "upll_fracck",
		.l = &pll_layout_div,
		.c = &upll_characteristics,
		.t = PLL_TYPE_DIV,
		.f = 1,
		.id = 1,
		.cid = ID_PLL_U_DIV,
	},
};

/**
 * Programmable clock description
 * @n:			clock name
 * @cid:		clock id corresponding to clock subsystem
 */
static const struct {
	const char *n;
	u8 cid;
} sam9x60_prog[] = {
	{ .n = "prog0", .cid = ID_PROG0, },
	{ .n = "prog1", .cid = ID_PROG1, },
};

/* Mux table for programmable clocks. */
static u32 sam9x60_prog_mux_table[] = { 0, 1, 2, 3, 4, 5, };

/**
 * System clock description
 * @n:			clock name
 * @p:			parent clock name
 * @id:			clock id corresponding to system clock driver
 * @cid:		clock id corresponding to clock subsystem
 */
static const struct {
	const char *n;
	const char *p;
	u8 id;
	u8 cid;
} sam9x60_systemck[] = {
	{ .n = "ddrck",		.p = "mck_div",  .id = 2, .cid = ID_DDR, },
	{ .n = "uhpck",		.p = "usbck",    .id = 6, .cid = ID_UHPCK },
	{ .n = "pck0",		.p = "prog0",    .id = 8, .cid = ID_PCK0, },
	{ .n = "pck1",		.p = "prog1",    .id = 9, .cid = ID_PCK1, },
	{ .n = "qspick",	.p = "mck_div",  .id = 19, .cid = ID_QSPI, },
};

/**
 * Peripheral clock description
 * @n:		clock name
 * @id:		clock id
 */
static const struct {
	const char *n;
	u8 id;
} sam9x60_periphck[] = {
	{ .n = "pioA_clk",   .id = 2, },
	{ .n = "pioB_clk",   .id = 3, },
	{ .n = "pioC_clk",   .id = 4, },
	{ .n = "flex0_clk",  .id = 5, },
	{ .n = "flex1_clk",  .id = 6, },
	{ .n = "flex2_clk",  .id = 7, },
	{ .n = "flex3_clk",  .id = 8, },
	{ .n = "flex6_clk",  .id = 9, },
	{ .n = "flex7_clk",  .id = 10, },
	{ .n = "flex8_clk",  .id = 11, },
	{ .n = "sdmmc0_clk", .id = 12, },
	{ .n = "flex4_clk",  .id = 13, },
	{ .n = "flex5_clk",  .id = 14, },
	{ .n = "flex9_clk",  .id = 15, },
	{ .n = "flex10_clk", .id = 16, },
	{ .n = "tcb0_clk",   .id = 17, },
	{ .n = "pwm_clk",    .id = 18, },
	{ .n = "adc_clk",    .id = 19, },
	{ .n = "dma0_clk",   .id = 20, },
	{ .n = "matrix_clk", .id = 21, },
	{ .n = "uhphs_clk",  .id = 22, },
	{ .n = "udphs_clk",  .id = 23, },
	{ .n = "macb0_clk",  .id = 24, },
	{ .n = "lcd_clk",    .id = 25, },
	{ .n = "sdmmc1_clk", .id = 26, },
	{ .n = "macb1_clk",  .id = 27, },
	{ .n = "ssc_clk",    .id = 28, },
	{ .n = "can0_clk",   .id = 29, },
	{ .n = "can1_clk",   .id = 30, },
	{ .n = "flex11_clk", .id = 32, },
	{ .n = "flex12_clk", .id = 33, },
	{ .n = "i2s_clk",    .id = 34, },
	{ .n = "qspi_clk",   .id = 35, },
	{ .n = "gfx2d_clk",  .id = 36, },
	{ .n = "pit64b_clk", .id = 37, },
	{ .n = "trng_clk",   .id = 38, },
	{ .n = "aes_clk",    .id = 39, },
	{ .n = "tdes_clk",   .id = 40, },
	{ .n = "sha_clk",    .id = 41, },
	{ .n = "classd_clk", .id = 42, },
	{ .n = "isi_clk",    .id = 43, },
	{ .n = "pioD_clk",   .id = 44, },
	{ .n = "tcb1_clk",   .id = 45, },
	{ .n = "dbgu_clk",   .id = 47, },
	{ .n = "mpddr_clk",  .id = 49, },
};

/**
 * Generic clock description
 * @n:			clock name
 * @ep:			extra parents parents names
 * @ep_mux_table:	extra parents mux table
 * @ep_clk_mux_table:	extra parents clock mux table (for CCF)
 * @r:			clock output range
 * @ep_count:		extra parents count
 * @id:			clock id
 */
static const struct {
	const char *n;
	struct clk_range r;
	u8 id;
} sam9x60_gck[] = {
	{ .n = "flex0_gclk",  .id = 5, },
	{ .n = "flex1_gclk",  .id = 6, },
	{ .n = "flex2_gclk",  .id = 7, },
	{ .n = "flex3_gclk",  .id = 8, },
	{ .n = "flex6_gclk",  .id = 9, },
	{ .n = "flex7_gclk",  .id = 10, },
	{ .n = "flex8_gclk",  .id = 11, },
	{ .n = "sdmmc0_gclk", .id = 12, .r = { .min = 0, .max = 105000000 }, },
	{ .n = "flex4_gclk",  .id = 13, },
	{ .n = "flex5_gclk",  .id = 14, },
	{ .n = "flex9_gclk",  .id = 15, },
	{ .n = "flex10_gclk", .id = 16, },
	{ .n = "tcb0_gclk",   .id = 17, },
	{ .n = "adc_gclk",    .id = 19, },
	{ .n = "lcd_gclk",    .id = 25, .r = { .min = 0, .max = 140000000 }, },
	{ .n = "sdmmc1_gclk", .id = 26, .r = { .min = 0, .max = 105000000 }, },
	{ .n = "flex11_gclk", .id = 32, },
	{ .n = "flex12_gclk", .id = 33, },
	{ .n = "i2s_gclk",    .id = 34, .r = { .min = 0, .max = 105000000 }, },
	{ .n = "pit64b_gclk", .id = 37, },
	{ .n = "classd_gclk", .id = 42, .r = { .min = 0, .max = 100000000 }, },
	{ .n = "tcb1_gclk",   .id = 45, },
	{ .n = "dbgu_gclk",   .id = 47, },
};

/**
 * Clock setup description
 * @cid:	clock id corresponding to clock subsystem
 * @pid:	parent clock id corresponding to clock subsystem
 * @rate:	clock rate
 * @prate:	parent rate
 */
static const struct pmc_clk_setup sam9x60_clk_setup[] = {
	{
		.cid = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_U_FRAC),
		.rate = 960000000,
	},

	{
		.cid = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_U_DIV),
		.rate = 480000000,
	},

	{
		.cid = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_USBCK),
		.pid = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_U_DIV),
		.rate = 48000000,
	},
};

#define prepare_mux_table(_allocs, _index, _dst, _src, _num, _label)	\
	do {								\
		int _i;							\
		(_dst) = kzalloc(sizeof(*(_dst)) * (_num), GFP_KERNEL);	\
		if (!(_dst)) {						\
			ret = -ENOMEM;					\
			goto _label;					\
		}							\
		(_allocs)[(_index)++] = (_dst);				\
		for (_i = 0; _i < (_num); _i++)				\
			(_dst)[_i] = (_src)[_i];			\
	} while (0)

static int sam9x60_clk_probe(struct udevice *dev)
{
	void __iomem *base = (void *)devfdt_get_addr_ptr(dev);
	unsigned int *clkmuxallocs[64], *muxallocs[64];
	const char *p[10];
	unsigned int cm[10], m[10], *tmpclkmux, *tmpmux;
	struct clk clk, *c;
	int ret, muxallocindex = 0, clkmuxallocindex = 0, i;
	static const struct clk_range r = { 0, 0 };

	if (!base)
		return -EINVAL;

	memset(muxallocs,    0, sizeof(muxallocs));
	memset(clkmuxallocs, 0, sizeof(clkmuxallocs));

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_get_by_id(clk.id, &c);
	if (ret)
		return ret;

	clk_names[ID_TD_SLCK] = kmemdup(clk_hw_get_name(c),
					strlen(clk_hw_get_name(c)) + 1,
					GFP_KERNEL);
	if (!clk_names[ID_TD_SLCK])
		return -ENOMEM;

	ret = clk_get_by_index(dev, 1, &clk);
	if (ret)
		return ret;

	ret = clk_get_by_id(clk.id, &c);
	if (ret)
		return ret;

	clk_names[ID_MD_SLCK] = kmemdup(clk_hw_get_name(c),
					strlen(clk_hw_get_name(c)) + 1,
					GFP_KERNEL);
	if (!clk_names[ID_MD_SLCK])
		return -ENOMEM;

	ret = clk_get_by_index(dev, 2, &clk);
	if (ret)
		return ret;

	clk_names[ID_MAIN_XTAL] = kmemdup(clk_hw_get_name(&clk),
					  strlen(clk_hw_get_name(&clk)) + 1,
					  GFP_KERNEL);
	if (!clk_names[ID_MAIN_XTAL])
		return -ENOMEM;

	ret = clk_get_by_index(dev, 3, &clk);
	if (ret)
		goto fail;

	clk_names[ID_MAIN_RC] = kmemdup(clk_hw_get_name(&clk),
					strlen(clk_hw_get_name(&clk)) + 1,
					GFP_KERNEL);
	if (ret)
		goto fail;

	/* Register main rc oscillator. */
	c = at91_clk_main_rc(base, clk_names[ID_MAIN_RC_OSC],
			     clk_names[ID_MAIN_RC]);
	if (IS_ERR(c)) {
		ret = PTR_ERR(c);
		goto fail;
	}
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAIN_RC_OSC), c);

	/* Register main oscillator. */
	c = at91_clk_main_osc(base, clk_names[ID_MAIN_OSC],
			      clk_names[ID_MAIN_XTAL], false);
	if (IS_ERR(c)) {
		ret = PTR_ERR(c);
		goto fail;
	}
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAIN_OSC), c);

	/* Register mainck. */
	p[0] = clk_names[ID_MAIN_RC_OSC];
	p[1] = clk_names[ID_MAIN_OSC];
	cm[0] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAIN_RC_OSC);
	cm[1] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAIN_OSC);
	prepare_mux_table(clkmuxallocs, clkmuxallocindex, tmpclkmux, cm, 2,
			  fail);
	c = at91_clk_sam9x5_main(base, clk_names[ID_MAINCK], p,
				 2, tmpclkmux, PMC_TYPE_CORE);
	if (IS_ERR(c)) {
		ret = PTR_ERR(c);
		goto fail;
	}
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAINCK), c);

	/* Register PLL fracs clocks. */
	for (i = 0; i < ARRAY_SIZE(sam9x60_plls); i++) {
		if (sam9x60_plls[i].t != PLL_TYPE_FRAC)
			continue;

		c = sam9x60_clk_register_frac_pll(base, sam9x60_plls[i].n,
						  sam9x60_plls[i].p,
						  sam9x60_plls[i].id,
						  sam9x60_plls[i].c,
						  sam9x60_plls[i].l,
						  sam9x60_plls[i].f);
		if (IS_ERR(c)) {
			ret = PTR_ERR(c);
			goto fail;
		}
		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, sam9x60_plls[i].cid), c);
	}

	/* Register PLL div clocks. */
	for (i = 0; i < ARRAY_SIZE(sam9x60_plls); i++) {
		if (sam9x60_plls[i].t != PLL_TYPE_DIV)
			continue;

		c = sam9x60_clk_register_div_pll(base, sam9x60_plls[i].n,
						 sam9x60_plls[i].p,
						 sam9x60_plls[i].id,
						 sam9x60_plls[i].c,
						 sam9x60_plls[i].l,
						 sam9x60_plls[i].f);
		if (IS_ERR(c)) {
			ret = PTR_ERR(c);
			goto fail;
		}
		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, sam9x60_plls[i].cid), c);
	}

	/* Register MCK pres clock. */
	p[0] = clk_names[ID_MD_SLCK];
	p[1] = clk_names[ID_MAINCK];
	p[2] = clk_names[ID_PLL_A_DIV];
	p[3] = clk_names[ID_PLL_U_DIV];
	cm[0] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MD_SLCK);
	cm[1] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAINCK);
	cm[2] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_A_DIV);
	cm[3] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_U_DIV);
	prepare_mux_table(clkmuxallocs, clkmuxallocindex, tmpclkmux, cm, 4,
			  fail);
	c = at91_clk_register_master_pres(base, clk_names[ID_MCK_PRES], p, 4,
					  &mck_layout, &mck_characteristics,
					  tmpclkmux);
	if (IS_ERR(c)) {
		ret = PTR_ERR(c);
		goto fail;
	}
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MCK_PRES), c);

	/* Register MCK div clock. */
	c = at91_clk_register_master_div(base, clk_names[ID_MCK_DIV],
					 clk_names[ID_MCK_PRES],
					 &mck_layout, &mck_characteristics);
	if (IS_ERR(c)) {
		ret = PTR_ERR(c);
		goto fail;
	}
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MCK_DIV), c);

	/* Register usbck. */
	p[0] = clk_names[ID_PLL_A_DIV];
	p[1] = clk_names[ID_PLL_U_DIV];
	p[2] = clk_names[ID_MAIN_XTAL];
	m[0] = 0;
	m[1] = 1;
	m[2] = 2;
	cm[0] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_A_DIV);
	cm[1] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_U_DIV);
	cm[2] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAIN_XTAL);
	prepare_mux_table(clkmuxallocs, clkmuxallocindex, tmpclkmux, cm,
			  3, fail);
	prepare_mux_table(muxallocs, muxallocindex, tmpmux, m, 3, fail);
	c = sam9x60_clk_register_usb(base, clk_names[ID_USBCK], p, 3,
				     &usbck_layout, tmpclkmux, tmpmux,
				     ID_USBCK);
	if (IS_ERR(c)) {
		ret = PTR_ERR(c);
		goto fail;
	}
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_USBCK), c);

	/* Register programmable clocks. */
	p[0] = clk_names[ID_MD_SLCK];
	p[1] = clk_names[ID_TD_SLCK];
	p[2] = clk_names[ID_MAINCK];
	p[3] = clk_names[ID_MCK_DIV];
	p[4] = clk_names[ID_PLL_A_DIV];
	p[5] = clk_names[ID_PLL_U_DIV];
	cm[0] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MD_SLCK);
	cm[1] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_TD_SLCK);
	cm[2] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAINCK);
	cm[3] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MCK_DIV);
	cm[4] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_A_DIV);
	cm[5] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_U_DIV);
	for (i = 0; i < ARRAY_SIZE(sam9x60_prog); i++) {
		prepare_mux_table(clkmuxallocs, clkmuxallocindex, tmpclkmux, cm,
				  6, fail);

		c = at91_clk_register_programmable(base, sam9x60_prog[i].n, p,
						   10, i, &programmable_layout,
						   tmpclkmux,
						   sam9x60_prog_mux_table);
		if (IS_ERR(c)) {
			ret = PTR_ERR(c);
			goto fail;
		}
		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, sam9x60_prog[i].cid), c);
	}

	/* System clocks. */
	for (i = 0; i < ARRAY_SIZE(sam9x60_systemck); i++) {
		c = at91_clk_register_system(base, sam9x60_systemck[i].n,
					     sam9x60_systemck[i].p,
					     sam9x60_systemck[i].id);
		if (IS_ERR(c)) {
			ret = PTR_ERR(c);
			goto fail;
		}
		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_SYSTEM, sam9x60_systemck[i].cid),
		       c);
	}

	/* Peripheral clocks. */
	for (i = 0; i < ARRAY_SIZE(sam9x60_periphck); i++) {
		c = at91_clk_register_sam9x5_peripheral(base, &pcr_layout,
							sam9x60_periphck[i].n,
							clk_names[ID_MCK_DIV],
							sam9x60_periphck[i].id,
							&r);
		if (IS_ERR(c)) {
			ret = PTR_ERR(c);
			goto fail;
		}
		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_PERIPHERAL,
				      sam9x60_periphck[i].id), c);
	}

	/* Generic clocks. */
	p[0] = clk_names[ID_MD_SLCK];
	p[1] = clk_names[ID_TD_SLCK];
	p[2] = clk_names[ID_MAINCK];
	p[3] = clk_names[ID_MCK_DIV];
	p[4] = clk_names[ID_PLL_A_DIV];
	p[5] = clk_names[ID_PLL_U_DIV];
	m[0] = 0;
	m[1] = 1;
	m[2] = 2;
	m[3] = 3;
	m[4] = 4;
	m[5] = 5;
	cm[0] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MD_SLCK);
	cm[1] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_TD_SLCK);
	cm[2] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAINCK);
	cm[3] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MCK_DIV);
	cm[4] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_A_DIV);
	cm[5] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_U_DIV);
	for (i = 0; i < ARRAY_SIZE(sam9x60_gck); i++) {
		prepare_mux_table(clkmuxallocs, clkmuxallocindex, tmpclkmux, cm,
				  6, fail);
		prepare_mux_table(muxallocs, muxallocindex, tmpmux, m,
				  6, fail);

		c = at91_clk_register_generic(base, &pcr_layout,
					      sam9x60_gck[i].n, p, tmpclkmux,
					      tmpmux, 6, sam9x60_gck[i].id,
					      &sam9x60_gck[i].r);
		if (IS_ERR(c)) {
			ret = PTR_ERR(c);
			goto fail;
		}
		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_GCK, sam9x60_gck[i].id), c);
	}

	/* Setup clocks. */
	ret = at91_clk_setup(sam9x60_clk_setup, ARRAY_SIZE(sam9x60_clk_setup));
	if (ret)
		goto fail;

	return 0;

fail:
	for (i = 0; i < ARRAY_SIZE(muxallocs); i++)
		kfree(muxallocs[i]);

	for (i = 0; i < ARRAY_SIZE(clkmuxallocs); i++)
		kfree(clkmuxallocs[i]);

	return ret;
}

static const struct udevice_id sam9x60_clk_ids[] = {
	{ .compatible = "microchip,sam9x60-pmc" },
	{ /* Sentinel. */ },
};

U_BOOT_DRIVER(at91_sam9x60_pmc) = {
	.name = "at91-sam9x60-pmc",
	.id = UCLASS_CLK,
	.of_match = sam9x60_clk_ids,
	.ops = &at91_clk_ops,
	.probe = sam9x60_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
