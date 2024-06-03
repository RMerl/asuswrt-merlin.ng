// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2010-2015
 * NVIDIA Corporation <www.nvidia.com>
 */

/* Tegra20 Clock control functions */

#include <common.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/timer.h>
#include <div64.h>
#include <fdtdec.h>

/*
 * Clock types that we can use as a source. The Tegra20 has muxes for the
 * peripheral clocks, and in most cases there are four options for the clock
 * source. This gives us a clock 'type' and exploits what commonality exists
 * in the device.
 *
 * Letters are obvious, except for T which means CLK_M, and S which means the
 * clock derived from 32KHz. Beware that CLK_M (also called OSC in the
 * datasheet) and PLL_M are different things. The former is the basic
 * clock supplied to the SOC from an external oscillator. The latter is the
 * memory clock PLL.
 *
 * See definitions in clock_id in the header file.
 */
enum clock_type_id {
	CLOCK_TYPE_AXPT,	/* PLL_A, PLL_X, PLL_P, CLK_M */
	CLOCK_TYPE_MCPA,	/* and so on */
	CLOCK_TYPE_MCPT,
	CLOCK_TYPE_PCM,
	CLOCK_TYPE_PCMT,
	CLOCK_TYPE_PCMT16,	/* CLOCK_TYPE_PCMT with 16-bit divider */
	CLOCK_TYPE_PCXTS,
	CLOCK_TYPE_PDCT,

	CLOCK_TYPE_COUNT,
	CLOCK_TYPE_NONE = -1,	/* invalid clock type */
};

enum {
	CLOCK_MAX_MUX	= 4	/* number of source options for each clock */
};

/*
 * Clock source mux for each clock type. This just converts our enum into
 * a list of mux sources for use by the code. Note that CLOCK_TYPE_PCXTS
 * is special as it has 5 sources. Since it also has a different number of
 * bits in its register for the source, we just handle it with a special
 * case in the code.
 */
#define CLK(x) CLOCK_ID_ ## x
static enum clock_id clock_source[CLOCK_TYPE_COUNT][CLOCK_MAX_MUX] = {
	{ CLK(AUDIO),	CLK(XCPU),	CLK(PERIPH),	CLK(OSC)	},
	{ CLK(MEMORY),	CLK(CGENERAL),	CLK(PERIPH),	CLK(AUDIO)	},
	{ CLK(MEMORY),	CLK(CGENERAL),	CLK(PERIPH),	CLK(OSC)	},
	{ CLK(PERIPH),	CLK(CGENERAL),	CLK(MEMORY),	CLK(NONE)	},
	{ CLK(PERIPH),	CLK(CGENERAL),	CLK(MEMORY),	CLK(OSC)	},
	{ CLK(PERIPH),	CLK(CGENERAL),	CLK(MEMORY),	CLK(OSC)	},
	{ CLK(PERIPH),	CLK(CGENERAL),	CLK(XCPU),	CLK(OSC)	},
	{ CLK(PERIPH),	CLK(DISPLAY),	CLK(CGENERAL),	CLK(OSC)	},
};

/*
 * Clock peripheral IDs which sadly don't match up with PERIPH_ID. This is
 * not in the header file since it is for purely internal use - we want
 * callers to use the PERIPH_ID for all access to peripheral clocks to avoid
 * confusion bewteen PERIPH_ID_... and PERIPHC_...
 *
 * We don't call this CLOCK_PERIPH_ID or PERIPH_CLOCK_ID as it would just be
 * confusing.
 *
 * Note to SOC vendors: perhaps define a unified numbering for peripherals and
 * use it for reset, clock enable, clock source/divider and even pinmuxing
 * if you can.
 */
enum periphc_internal_id {
	/* 0x00 */
	PERIPHC_I2S1,
	PERIPHC_I2S2,
	PERIPHC_SPDIF_OUT,
	PERIPHC_SPDIF_IN,
	PERIPHC_PWM,
	PERIPHC_SPI1,
	PERIPHC_SPI2,
	PERIPHC_SPI3,

	/* 0x08 */
	PERIPHC_XIO,
	PERIPHC_I2C1,
	PERIPHC_DVC_I2C,
	PERIPHC_TWC,
	PERIPHC_0c,
	PERIPHC_10,	/* PERIPHC_SPI1, what is this really? */
	PERIPHC_DISP1,
	PERIPHC_DISP2,

	/* 0x10 */
	PERIPHC_CVE,
	PERIPHC_IDE0,
	PERIPHC_VI,
	PERIPHC_1c,
	PERIPHC_SDMMC1,
	PERIPHC_SDMMC2,
	PERIPHC_G3D,
	PERIPHC_G2D,

	/* 0x18 */
	PERIPHC_NDFLASH,
	PERIPHC_SDMMC4,
	PERIPHC_VFIR,
	PERIPHC_EPP,
	PERIPHC_MPE,
	PERIPHC_MIPI,
	PERIPHC_UART1,
	PERIPHC_UART2,

	/* 0x20 */
	PERIPHC_HOST1X,
	PERIPHC_21,
	PERIPHC_TVO,
	PERIPHC_HDMI,
	PERIPHC_24,
	PERIPHC_TVDAC,
	PERIPHC_I2C2,
	PERIPHC_EMC,

	/* 0x28 */
	PERIPHC_UART3,
	PERIPHC_29,
	PERIPHC_VI_SENSOR,
	PERIPHC_2b,
	PERIPHC_2c,
	PERIPHC_SPI4,
	PERIPHC_I2C3,
	PERIPHC_SDMMC3,

	/* 0x30 */
	PERIPHC_UART4,
	PERIPHC_UART5,
	PERIPHC_VDE,
	PERIPHC_OWR,
	PERIPHC_NOR,
	PERIPHC_CSITE,

	PERIPHC_COUNT,

	PERIPHC_NONE = -1,
};

/*
 * Clock type for each peripheral clock source. We put the name in each
 * record just so it is easy to match things up
 */
#define TYPE(name, type) type
static enum clock_type_id clock_periph_type[PERIPHC_COUNT] = {
	/* 0x00 */
	TYPE(PERIPHC_I2S1,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_I2S2,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_SPDIF_OUT,	CLOCK_TYPE_AXPT),
	TYPE(PERIPHC_SPDIF_IN,	CLOCK_TYPE_PCM),
	TYPE(PERIPHC_PWM,	CLOCK_TYPE_PCXTS),
	TYPE(PERIPHC_SPI1,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_SPI22,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_SPI3,	CLOCK_TYPE_PCMT),

	/* 0x08 */
	TYPE(PERIPHC_XIO,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_I2C1,	CLOCK_TYPE_PCMT16),
	TYPE(PERIPHC_DVC_I2C,	CLOCK_TYPE_PCMT16),
	TYPE(PERIPHC_TWC,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SPI1,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_DISP1,	CLOCK_TYPE_PDCT),
	TYPE(PERIPHC_DISP2,	CLOCK_TYPE_PDCT),

	/* 0x10 */
	TYPE(PERIPHC_CVE,	CLOCK_TYPE_PDCT),
	TYPE(PERIPHC_IDE0,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_VI,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SDMMC1,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_SDMMC2,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_G3D,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_G2D,	CLOCK_TYPE_MCPA),

	/* 0x18 */
	TYPE(PERIPHC_NDFLASH,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_SDMMC4,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_VFIR,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_EPP,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_MPE,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_MIPI,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_UART1,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_UART2,	CLOCK_TYPE_PCMT),

	/* 0x20 */
	TYPE(PERIPHC_HOST1X,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_TVO,	CLOCK_TYPE_PDCT),
	TYPE(PERIPHC_HDMI,	CLOCK_TYPE_PDCT),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_TVDAC,	CLOCK_TYPE_PDCT),
	TYPE(PERIPHC_I2C2,	CLOCK_TYPE_PCMT16),
	TYPE(PERIPHC_EMC,	CLOCK_TYPE_MCPT),

	/* 0x28 */
	TYPE(PERIPHC_UART3,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_VI,	CLOCK_TYPE_MCPA),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_NONE,	CLOCK_TYPE_NONE),
	TYPE(PERIPHC_SPI4,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_I2C3,	CLOCK_TYPE_PCMT16),
	TYPE(PERIPHC_SDMMC3,	CLOCK_TYPE_PCMT),

	/* 0x30 */
	TYPE(PERIPHC_UART4,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_UART5,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_VDE,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_OWR,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_NOR,	CLOCK_TYPE_PCMT),
	TYPE(PERIPHC_CSITE,	CLOCK_TYPE_PCMT),
};

/*
 * This array translates a periph_id to a periphc_internal_id
 *
 * Not present/matched up:
 *	uint vi_sensor;	 _VI_SENSOR_0,		0x1A8
 *	SPDIF - which is both 0x08 and 0x0c
 *
 */
#define NONE(name) (-1)
#define OFFSET(name, value) PERIPHC_ ## name
static s8 periph_id_to_internal_id[PERIPH_ID_COUNT] = {
	/* Low word: 31:0 */
	NONE(CPU),
	NONE(RESERVED1),
	NONE(RESERVED2),
	NONE(AC97),
	NONE(RTC),
	NONE(TMR),
	PERIPHC_UART1,
	PERIPHC_UART2,	/* and vfir 0x68 */

	/* 0x08 */
	NONE(GPIO),
	PERIPHC_SDMMC2,
	NONE(SPDIF),		/* 0x08 and 0x0c, unclear which to use */
	PERIPHC_I2S1,
	PERIPHC_I2C1,
	PERIPHC_NDFLASH,
	PERIPHC_SDMMC1,
	PERIPHC_SDMMC4,

	/* 0x10 */
	PERIPHC_TWC,
	PERIPHC_PWM,
	PERIPHC_I2S2,
	PERIPHC_EPP,
	PERIPHC_VI,
	PERIPHC_G2D,
	NONE(USBD),
	NONE(ISP),

	/* 0x18 */
	PERIPHC_G3D,
	PERIPHC_IDE0,
	PERIPHC_DISP2,
	PERIPHC_DISP1,
	PERIPHC_HOST1X,
	NONE(VCP),
	NONE(RESERVED30),
	NONE(CACHE2),

	/* Middle word: 63:32 */
	NONE(MEM),
	NONE(AHBDMA),
	NONE(APBDMA),
	NONE(RESERVED35),
	NONE(KBC),
	NONE(STAT_MON),
	NONE(PMC),
	NONE(FUSE),

	/* 0x28 */
	NONE(KFUSE),
	NONE(SBC1),	/* SBC1, 0x34, is this SPI1? */
	PERIPHC_NOR,
	PERIPHC_SPI1,
	PERIPHC_SPI2,
	PERIPHC_XIO,
	PERIPHC_SPI3,
	PERIPHC_DVC_I2C,

	/* 0x30 */
	NONE(DSI),
	PERIPHC_TVO,	/* also CVE 0x40 */
	PERIPHC_MIPI,
	PERIPHC_HDMI,
	PERIPHC_CSITE,
	PERIPHC_TVDAC,
	PERIPHC_I2C2,
	PERIPHC_UART3,

	/* 0x38 */
	NONE(RESERVED56),
	PERIPHC_EMC,
	NONE(USB2),
	NONE(USB3),
	PERIPHC_MPE,
	PERIPHC_VDE,
	NONE(BSEA),
	NONE(BSEV),

	/* Upper word 95:64 */
	NONE(SPEEDO),
	PERIPHC_UART4,
	PERIPHC_UART5,
	PERIPHC_I2C3,
	PERIPHC_SPI4,
	PERIPHC_SDMMC3,
	NONE(PCIE),
	PERIPHC_OWR,

	/* 0x48 */
	NONE(AFI),
	NONE(CORESIGHT),
	NONE(PCIEXCLK),
	NONE(AVPUCQ),
	NONE(RESERVED76),
	NONE(RESERVED77),
	NONE(RESERVED78),
	NONE(RESERVED79),

	/* 0x50 */
	NONE(RESERVED80),
	NONE(RESERVED81),
	NONE(RESERVED82),
	NONE(RESERVED83),
	NONE(IRAMA),
	NONE(IRAMB),
	NONE(IRAMC),
	NONE(IRAMD),

	/* 0x58 */
	NONE(CRAM2),
};

/*
 * PLL divider shift/mask tables for all PLL IDs.
 */
struct clk_pll_info tegra_pll_info_table[CLOCK_ID_PLL_COUNT] = {
	/*
	 * T20 and T25
	 * NOTE: If kcp_mask/kvco_mask == 0, they're not used in that PLL (PLLX, etc.)
	 *       If lock_ena or lock_det are >31, they're not used in that PLL.
	 */

	{ .m_shift = 0, .m_mask = 0xFF, .n_shift = 8, .n_mask = 0x3FF,  .p_shift = 20, .p_mask = 0x0F,
	  .lock_ena = 24, .lock_det = 27, .kcp_shift = 28, .kcp_mask = 3, .kvco_shift = 27, .kvco_mask = 1 },	/* PLLC */
	{ .m_shift = 0, .m_mask = 0xFF, .n_shift = 8, .n_mask = 0x3FF,  .p_shift = 0,  .p_mask = 0,
	  .lock_ena = 0,  .lock_det = 27, .kcp_shift = 1, .kcp_mask = 3, .kvco_shift = 0, .kvco_mask = 1 },	/* PLLM */
	{ .m_shift = 0, .m_mask = 0x1F, .n_shift = 8, .n_mask = 0x3FF, .p_shift = 20, .p_mask = 0x07,
	  .lock_ena = 18, .lock_det = 27, .kcp_shift = 8, .kcp_mask = 0xF, .kvco_shift = 4, .kvco_mask = 0xF },	/* PLLP */
	{ .m_shift = 0, .m_mask = 0x1F, .n_shift = 8, .n_mask = 0x3FF, .p_shift = 20, .p_mask = 0x07,
	  .lock_ena = 18, .lock_det = 27, .kcp_shift = 8, .kcp_mask = 0xF, .kvco_shift = 4, .kvco_mask = 0xF },	/* PLLA */
	{ .m_shift = 0, .m_mask = 0x1F, .n_shift = 8, .n_mask = 0x3FF, .p_shift = 20, .p_mask = 0x01,
	  .lock_ena = 22, .lock_det = 27, .kcp_shift = 8, .kcp_mask = 0xF, .kvco_shift = 4, .kvco_mask = 0xF },	/* PLLU */
	{ .m_shift = 0, .m_mask = 0x1F, .n_shift = 8, .n_mask = 0x3FF, .p_shift = 20, .p_mask = 0x07,
	  .lock_ena = 22, .lock_det = 27, .kcp_shift = 8, .kcp_mask = 0xF, .kvco_shift = 4, .kvco_mask = 0xF },	/* PLLD */
	{ .m_shift = 0, .m_mask = 0x1F, .n_shift = 8, .n_mask = 0x3FF,  .p_shift = 20, .p_mask = 0x0F,
	  .lock_ena = 18, .lock_det = 27, .kcp_shift = 8, .kcp_mask = 0xF, .kvco_shift = 0, .kvco_mask = 0 },	/* PLLX */
	{ .m_shift = 0, .m_mask = 0xFF, .n_shift = 8, .n_mask = 0xFF,  .p_shift = 0,  .p_mask = 0,
	  .lock_ena = 9,  .lock_det = 11, .kcp_shift = 6, .kcp_mask = 3, .kvco_shift = 0, .kvco_mask = 1 },	/* PLLE */
	{ .m_shift = 0, .m_mask = 0x0F, .n_shift = 8, .n_mask = 0x3FF, .p_shift = 20, .p_mask = 0x07,
	  .lock_ena = 18, .lock_det = 0, .kcp_shift = 8, .kcp_mask = 0xF, .kvco_shift = 4, .kvco_mask = 0xF },	/* PLLS */
};

/*
 * Get the oscillator frequency, from the corresponding hardware configuration
 * field. T20 has 4 frequencies that it supports.
 */
enum clock_osc_freq clock_get_osc_freq(void)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 reg;

	reg = readl(&clkrst->crc_osc_ctrl);
	return (reg & OSC_FREQ_MASK) >> OSC_FREQ_SHIFT;
}

/* Returns a pointer to the clock source register for a peripheral */
u32 *get_periph_source_reg(enum periph_id periph_id)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	enum periphc_internal_id internal_id;

	assert(clock_periph_id_isvalid(periph_id));
	internal_id = periph_id_to_internal_id[periph_id];
	assert(internal_id != -1);
	return &clkrst->crc_clk_src[internal_id];
}

int get_periph_clock_info(enum periph_id periph_id, int *mux_bits,
			  int *divider_bits, int *type)
{
	enum periphc_internal_id internal_id;

	if (!clock_periph_id_isvalid(periph_id))
		return -1;

	internal_id = periph_id_to_internal_id[periph_id];
	if (!periphc_internal_id_isvalid(internal_id))
		return -1;

	*type = clock_periph_type[internal_id];
	if (!clock_type_id_isvalid(*type))
		return -1;

	/*
	 * Special cases here for the clock with a 4-bit source mux and I2C
	 * with its 16-bit divisor
	 */
	if (*type == CLOCK_TYPE_PCXTS)
		*mux_bits = MASK_BITS_31_28;
	else
		*mux_bits = MASK_BITS_31_30;
	if (*type == CLOCK_TYPE_PCMT16)
		*divider_bits = 16;
	else
		*divider_bits = 8;

	return 0;
}

enum clock_id get_periph_clock_id(enum periph_id periph_id, int source)
{
	enum periphc_internal_id internal_id;
	int type;

	if (!clock_periph_id_isvalid(periph_id))
		return CLOCK_ID_NONE;

	internal_id = periph_id_to_internal_id[periph_id];
	if (!periphc_internal_id_isvalid(internal_id))
		return CLOCK_ID_NONE;

	type = clock_periph_type[internal_id];
	if (!clock_type_id_isvalid(type))
		return CLOCK_ID_NONE;

	return clock_source[type][source];
}

/**
 * Given a peripheral ID and the required source clock, this returns which
 * value should be programmed into the source mux for that peripheral.
 *
 * There is special code here to handle the one source type with 5 sources.
 *
 * @param periph_id	peripheral to start
 * @param source	PLL id of required parent clock
 * @param mux_bits	Set to number of bits in mux register: 2 or 4
 * @param divider_bits	Set to number of divider bits (8 or 16)
 * @return mux value (0-4, or -1 if not found)
 */
int get_periph_clock_source(enum periph_id periph_id,
		enum clock_id parent, int *mux_bits, int *divider_bits)
{
	enum clock_type_id type;
	int mux, err;

	err = get_periph_clock_info(periph_id, mux_bits, divider_bits, &type);
	assert(!err);

	for (mux = 0; mux < CLOCK_MAX_MUX; mux++)
		if (clock_source[type][mux] == parent)
			return mux;

	/*
	 * Not found: it might be looking for the 'S' in CLOCK_TYPE_PCXTS
	 * which is not in our table. If not, then they are asking for a
	 * source which this peripheral can't access through its mux.
	 */
	assert(type == CLOCK_TYPE_PCXTS);
	assert(parent == CLOCK_ID_SFROM32KHZ);
	if (type == CLOCK_TYPE_PCXTS && parent == CLOCK_ID_SFROM32KHZ)
		return 4;	/* mux value for this clock */

	/* if we get here, either us or the caller has made a mistake */
	printf("Caller requested bad clock: periph=%d, parent=%d\n", periph_id,
		parent);
	return -1;
}

void clock_set_enable(enum periph_id periph_id, int enable)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 *clk = &clkrst->crc_clk_out_enb[PERIPH_REG(periph_id)];
	u32 reg;

	/* Enable/disable the clock to this peripheral */
	assert(clock_periph_id_isvalid(periph_id));
	reg = readl(clk);
	if (enable)
		reg |= PERIPH_MASK(periph_id);
	else
		reg &= ~PERIPH_MASK(periph_id);
	writel(reg, clk);
}

void reset_set_enable(enum periph_id periph_id, int enable)
{
	struct clk_rst_ctlr *clkrst =
			(struct clk_rst_ctlr *)NV_PA_CLK_RST_BASE;
	u32 *reset = &clkrst->crc_rst_dev[PERIPH_REG(periph_id)];
	u32 reg;

	/* Enable/disable reset to the peripheral */
	assert(clock_periph_id_isvalid(periph_id));
	reg = readl(reset);
	if (enable)
		reg |= PERIPH_MASK(periph_id);
	else
		reg &= ~PERIPH_MASK(periph_id);
	writel(reg, reset);
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
/*
 * Convert a device tree clock ID to our peripheral ID. They are mostly
 * the same but we are very cautious so we check that a valid clock ID is
 * provided.
 *
 * @param clk_id	Clock ID according to tegra20 device tree binding
 * @return peripheral ID, or PERIPH_ID_NONE if the clock ID is invalid
 */
enum periph_id clk_id_to_periph_id(int clk_id)
{
	if (clk_id > PERIPH_ID_COUNT)
		return PERIPH_ID_NONE;

	switch (clk_id) {
	case PERIPH_ID_RESERVED1:
	case PERIPH_ID_RESERVED2:
	case PERIPH_ID_RESERVED30:
	case PERIPH_ID_RESERVED35:
	case PERIPH_ID_RESERVED56:
	case PERIPH_ID_PCIEXCLK:
	case PERIPH_ID_RESERVED76:
	case PERIPH_ID_RESERVED77:
	case PERIPH_ID_RESERVED78:
	case PERIPH_ID_RESERVED79:
	case PERIPH_ID_RESERVED80:
	case PERIPH_ID_RESERVED81:
	case PERIPH_ID_RESERVED82:
	case PERIPH_ID_RESERVED83:
	case PERIPH_ID_RESERVED91:
		return PERIPH_ID_NONE;
	default:
		return clk_id;
	}
}
#endif /* CONFIG_IS_ENABLED(OF_CONTROL) */

void clock_early_init(void)
{
	/*
	 * PLLP output frequency set to 216MHz
	 * PLLC output frequency set to 600Mhz
	 *
	 * TODO: Can we calculate these values instead of hard-coding?
	 */
	switch (clock_get_osc_freq()) {
	case CLOCK_OSC_FREQ_12_0: /* OSC is 12Mhz */
		clock_set_rate(CLOCK_ID_PERIPH, 432, 12, 1, 8);
		clock_set_rate(CLOCK_ID_CGENERAL, 600, 12, 0, 8);
		break;

	case CLOCK_OSC_FREQ_26_0: /* OSC is 26Mhz */
		clock_set_rate(CLOCK_ID_PERIPH, 432, 26, 1, 8);
		clock_set_rate(CLOCK_ID_CGENERAL, 600, 26, 0, 8);
		break;

	case CLOCK_OSC_FREQ_13_0: /* OSC is 13Mhz */
		clock_set_rate(CLOCK_ID_PERIPH, 432, 13, 1, 8);
		clock_set_rate(CLOCK_ID_CGENERAL, 600, 13, 0, 8);
		break;
	case CLOCK_OSC_FREQ_19_2:
	default:
		/*
		 * These are not supported. It is too early to print a
		 * message and the UART likely won't work anyway due to the
		 * oscillator being wrong.
		 */
		break;
	}
}

void arch_timer_init(void)
{
}

#define PMC_SATA_PWRGT 0x1ac
#define  PMC_SATA_PWRGT_PLLE_IDDQ_OVERRIDE (1 << 5)
#define  PMC_SATA_PWRGT_PLLE_IDDQ_SWCTL (1 << 4)

#define PLLE_SS_CNTL 0x68
#define  PLLE_SS_CNTL_SSCINCINTRV(x) (((x) & 0x3f) << 24)
#define  PLLE_SS_CNTL_SSCINC(x) (((x) & 0xff) << 16)
#define  PLLE_SS_CNTL_SSCBYP (1 << 12)
#define  PLLE_SS_CNTL_INTERP_RESET (1 << 11)
#define  PLLE_SS_CNTL_BYPASS_SS (1 << 10)
#define  PLLE_SS_CNTL_SSCMAX(x) (((x) & 0x1ff) << 0)

#define PLLE_BASE 0x0e8
#define  PLLE_BASE_ENABLE_CML (1 << 31)
#define  PLLE_BASE_ENABLE (1 << 30)
#define  PLLE_BASE_PLDIV_CML(x) (((x) & 0xf) << 24)
#define  PLLE_BASE_PLDIV(x) (((x) & 0x3f) << 16)
#define  PLLE_BASE_NDIV(x) (((x) & 0xff) << 8)
#define  PLLE_BASE_MDIV(x) (((x) & 0xff) << 0)

#define PLLE_MISC 0x0ec
#define  PLLE_MISC_SETUP_BASE(x) (((x) & 0xffff) << 16)
#define  PLLE_MISC_PLL_READY (1 << 15)
#define  PLLE_MISC_LOCK (1 << 11)
#define  PLLE_MISC_LOCK_ENABLE (1 << 9)
#define  PLLE_MISC_SETUP_EXT(x) (((x) & 0x3) << 2)

static int tegra_plle_train(void)
{
	unsigned int timeout = 2000;
	unsigned long value;

	value = readl(NV_PA_PMC_BASE + PMC_SATA_PWRGT);
	value |= PMC_SATA_PWRGT_PLLE_IDDQ_OVERRIDE;
	writel(value, NV_PA_PMC_BASE + PMC_SATA_PWRGT);

	value = readl(NV_PA_PMC_BASE + PMC_SATA_PWRGT);
	value |= PMC_SATA_PWRGT_PLLE_IDDQ_SWCTL;
	writel(value, NV_PA_PMC_BASE + PMC_SATA_PWRGT);

	value = readl(NV_PA_PMC_BASE + PMC_SATA_PWRGT);
	value &= ~PMC_SATA_PWRGT_PLLE_IDDQ_OVERRIDE;
	writel(value, NV_PA_PMC_BASE + PMC_SATA_PWRGT);

	do {
		value = readl(NV_PA_CLK_RST_BASE + PLLE_MISC);
		if (value & PLLE_MISC_PLL_READY)
			break;

		udelay(100);
	} while (--timeout);

	if (timeout == 0) {
		pr_err("timeout waiting for PLLE to become ready");
		return -ETIMEDOUT;
	}

	return 0;
}

int tegra_plle_enable(void)
{
	unsigned int timeout = 1000;
	u32 value;
	int err;

	/* disable PLLE clock */
	value = readl(NV_PA_CLK_RST_BASE + PLLE_BASE);
	value &= ~PLLE_BASE_ENABLE_CML;
	value &= ~PLLE_BASE_ENABLE;
	writel(value, NV_PA_CLK_RST_BASE + PLLE_BASE);

	/* clear lock enable and setup field */
	value = readl(NV_PA_CLK_RST_BASE + PLLE_MISC);
	value &= ~PLLE_MISC_LOCK_ENABLE;
	value &= ~PLLE_MISC_SETUP_BASE(0xffff);
	value &= ~PLLE_MISC_SETUP_EXT(0x3);
	writel(value, NV_PA_CLK_RST_BASE + PLLE_MISC);

	value = readl(NV_PA_CLK_RST_BASE + PLLE_MISC);
	if ((value & PLLE_MISC_PLL_READY) == 0) {
		err = tegra_plle_train();
		if (err < 0) {
			pr_err("failed to train PLLE: %d", err);
			return err;
		}
	}

	value = readl(NV_PA_CLK_RST_BASE + PLLE_MISC);
	value |= PLLE_MISC_SETUP_BASE(0x7);
	value |= PLLE_MISC_LOCK_ENABLE;
	value |= PLLE_MISC_SETUP_EXT(0);
	writel(value, NV_PA_CLK_RST_BASE + PLLE_MISC);

	value = readl(NV_PA_CLK_RST_BASE + PLLE_SS_CNTL);
	value |= PLLE_SS_CNTL_SSCBYP | PLLE_SS_CNTL_INTERP_RESET |
		 PLLE_SS_CNTL_BYPASS_SS;
	writel(value, NV_PA_CLK_RST_BASE + PLLE_SS_CNTL);

	value = readl(NV_PA_CLK_RST_BASE + PLLE_BASE);
	value |= PLLE_BASE_ENABLE_CML | PLLE_BASE_ENABLE;
	writel(value, NV_PA_CLK_RST_BASE + PLLE_BASE);

	do {
		value = readl(NV_PA_CLK_RST_BASE + PLLE_MISC);
		if (value & PLLE_MISC_LOCK)
			break;

		udelay(2);
	} while (--timeout);

	if (timeout == 0) {
		pr_err("timeout waiting for PLLE to lock");
		return -ETIMEDOUT;
	}

	udelay(50);

	value = readl(NV_PA_CLK_RST_BASE + PLLE_SS_CNTL);
	value &= ~PLLE_SS_CNTL_SSCINCINTRV(0x3f);
	value |= PLLE_SS_CNTL_SSCINCINTRV(0x18);

	value &= ~PLLE_SS_CNTL_SSCINC(0xff);
	value |= PLLE_SS_CNTL_SSCINC(0x01);

	value &= ~PLLE_SS_CNTL_SSCBYP;
	value &= ~PLLE_SS_CNTL_INTERP_RESET;
	value &= ~PLLE_SS_CNTL_BYPASS_SS;

	value &= ~PLLE_SS_CNTL_SSCMAX(0x1ff);
	value |= PLLE_SS_CNTL_SSCMAX(0x24);
	writel(value, NV_PA_CLK_RST_BASE + PLLE_SS_CNTL);

	return 0;
}

struct periph_clk_init periph_clk_init_table[] = {
	{ PERIPH_ID_SPI1, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SBC1, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SBC2, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SBC3, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SBC4, CLOCK_ID_PERIPH },
	{ PERIPH_ID_HOST1X, CLOCK_ID_PERIPH },
	{ PERIPH_ID_DISP1, CLOCK_ID_CGENERAL },
	{ PERIPH_ID_NDFLASH, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SDMMC1, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SDMMC2, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SDMMC3, CLOCK_ID_PERIPH },
	{ PERIPH_ID_SDMMC4, CLOCK_ID_PERIPH },
	{ PERIPH_ID_PWM, CLOCK_ID_SFROM32KHZ },
	{ PERIPH_ID_DVC_I2C, CLOCK_ID_PERIPH },
	{ PERIPH_ID_I2C1, CLOCK_ID_PERIPH },
	{ PERIPH_ID_I2C2, CLOCK_ID_PERIPH },
	{ PERIPH_ID_I2C3, CLOCK_ID_PERIPH },
	{ -1, },
};
