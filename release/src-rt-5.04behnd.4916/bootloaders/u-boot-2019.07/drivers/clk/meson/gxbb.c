// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 - Beniamino Galvani <b.galvani@gmail.com>
 * (C) Copyright 2018 - BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <asm/arch/clock-gx.h>
#include <asm/io.h>
#include <clk-uclass.h>
#include <div64.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <dt-bindings/clock/gxbb-clkc.h>
#include "clk_meson.h"

/* This driver support only basic clock tree operations :
 * - Can calculate clock frequency on a limited tree
 * - Can Read muxes and basic dividers (0-based only)
 * - Can enable/disable gates with limited propagation
 * - Can reparent without propagation, only on muxes
 * - Can set rates without reparenting
 * This driver is adapted to what is actually supported by U-Boot
 */

/* Only the clocks ids we don't want to expose, such as the internal muxes
 * and dividers of composite clocks, will remain defined here.
 */
#define CLKID_MPEG_SEL		  10
#define CLKID_MPEG_DIV		  11
#define CLKID_SAR_ADC_DIV	  99
#define CLKID_MALI_0_DIV	  101
#define CLKID_MALI_1_DIV	  104
#define CLKID_CTS_AMCLK_SEL	  108
#define CLKID_CTS_AMCLK_DIV	  109
#define CLKID_CTS_MCLK_I958_SEL	  111
#define CLKID_CTS_MCLK_I958_DIV	  112
#define CLKID_32K_CLK_SEL	  115
#define CLKID_32K_CLK_DIV	  116
#define CLKID_SD_EMMC_A_CLK0_SEL  117
#define CLKID_SD_EMMC_A_CLK0_DIV  118
#define CLKID_SD_EMMC_B_CLK0_SEL  120
#define CLKID_SD_EMMC_B_CLK0_DIV  121
#define CLKID_SD_EMMC_C_CLK0_SEL  123
#define CLKID_SD_EMMC_C_CLK0_DIV  124
#define CLKID_VPU_0_DIV		  127
#define CLKID_VPU_1_DIV		  130
#define CLKID_VAPB_0_DIV	  134
#define CLKID_VAPB_1_DIV	  137
#define CLKID_HDMI_PLL_PRE_MULT	  141
#define CLKID_MPLL0_DIV		  142
#define CLKID_MPLL1_DIV		  143
#define CLKID_MPLL2_DIV		  144
#define CLKID_MPLL_PREDIV	  145
#define CLKID_FCLK_DIV2_DIV	  146
#define CLKID_FCLK_DIV3_DIV	  147
#define CLKID_FCLK_DIV4_DIV	  148
#define CLKID_FCLK_DIV5_DIV	  149
#define CLKID_FCLK_DIV7_DIV	  150
#define CLKID_VDEC_1_SEL	  151
#define CLKID_VDEC_1_DIV	  152
#define CLKID_VDEC_HEVC_SEL	  154
#define CLKID_VDEC_HEVC_DIV	  155

#define XTAL_RATE 24000000

struct meson_clk {
	struct regmap *map;
};

static ulong meson_div_get_rate(struct clk *clk, unsigned long id);
static ulong meson_div_set_rate(struct clk *clk, unsigned long id, ulong rate,
				ulong current_rate);
static ulong meson_mux_set_parent(struct clk *clk, unsigned long id,
				  unsigned long parent_id);
static ulong meson_mux_get_rate(struct clk *clk, unsigned long id);
static ulong meson_clk_set_rate_by_id(struct clk *clk, unsigned long id,
				      ulong rate, ulong current_rate);
static ulong meson_mux_get_parent(struct clk *clk, unsigned long id);
static ulong meson_clk_get_rate_by_id(struct clk *clk, unsigned long id);

static struct meson_gate gates[] = {
	/* Everything Else (EE) domain gates */
	MESON_GATE(CLKID_DDR, HHI_GCLK_MPEG0, 0),
	MESON_GATE(CLKID_DOS, HHI_GCLK_MPEG0, 1),
	MESON_GATE(CLKID_ISA, HHI_GCLK_MPEG0, 5),
	MESON_GATE(CLKID_PL301, HHI_GCLK_MPEG0, 6),
	MESON_GATE(CLKID_PERIPHS, HHI_GCLK_MPEG0, 7),
	MESON_GATE(CLKID_SPICC, HHI_GCLK_MPEG0, 8),
	MESON_GATE(CLKID_I2C, HHI_GCLK_MPEG0, 9),
	MESON_GATE(CLKID_SAR_ADC, HHI_GCLK_MPEG0, 10),
	MESON_GATE(CLKID_SMART_CARD, HHI_GCLK_MPEG0, 11),
	MESON_GATE(CLKID_RNG0, HHI_GCLK_MPEG0, 12),
	MESON_GATE(CLKID_UART0, HHI_GCLK_MPEG0, 13),
	MESON_GATE(CLKID_SDHC, HHI_GCLK_MPEG0, 14),
	MESON_GATE(CLKID_STREAM, HHI_GCLK_MPEG0, 15),
	MESON_GATE(CLKID_ASYNC_FIFO, HHI_GCLK_MPEG0, 16),
	MESON_GATE(CLKID_SDIO, HHI_GCLK_MPEG0, 17),
	MESON_GATE(CLKID_ABUF, HHI_GCLK_MPEG0, 18),
	MESON_GATE(CLKID_HIU_IFACE, HHI_GCLK_MPEG0, 19),
	MESON_GATE(CLKID_ASSIST_MISC, HHI_GCLK_MPEG0, 23),
	MESON_GATE(CLKID_SD_EMMC_A, HHI_GCLK_MPEG0, 24),
	MESON_GATE(CLKID_SD_EMMC_B, HHI_GCLK_MPEG0, 25),
	MESON_GATE(CLKID_SD_EMMC_C, HHI_GCLK_MPEG0, 26),
	MESON_GATE(CLKID_SPI, HHI_GCLK_MPEG0, 30),

	MESON_GATE(CLKID_I2S_SPDIF, HHI_GCLK_MPEG1, 2),
	MESON_GATE(CLKID_ETH, HHI_GCLK_MPEG1, 3),
	MESON_GATE(CLKID_DEMUX, HHI_GCLK_MPEG1, 4),
	MESON_GATE(CLKID_AIU_GLUE, HHI_GCLK_MPEG1, 6),
	MESON_GATE(CLKID_IEC958, HHI_GCLK_MPEG1, 7),
	MESON_GATE(CLKID_I2S_OUT, HHI_GCLK_MPEG1, 8),
	MESON_GATE(CLKID_AMCLK, HHI_GCLK_MPEG1, 9),
	MESON_GATE(CLKID_AIFIFO2, HHI_GCLK_MPEG1, 10),
	MESON_GATE(CLKID_MIXER, HHI_GCLK_MPEG1, 11),
	MESON_GATE(CLKID_MIXER_IFACE, HHI_GCLK_MPEG1, 12),
	MESON_GATE(CLKID_ADC, HHI_GCLK_MPEG1, 13),
	MESON_GATE(CLKID_BLKMV, HHI_GCLK_MPEG1, 14),
	MESON_GATE(CLKID_AIU, HHI_GCLK_MPEG1, 15),
	MESON_GATE(CLKID_UART1, HHI_GCLK_MPEG1, 16),
	MESON_GATE(CLKID_G2D, HHI_GCLK_MPEG1, 20),
	MESON_GATE(CLKID_USB0, HHI_GCLK_MPEG1, 21),
	MESON_GATE(CLKID_USB1, HHI_GCLK_MPEG1, 22),
	MESON_GATE(CLKID_RESET, HHI_GCLK_MPEG1, 23),
	MESON_GATE(CLKID_NAND, HHI_GCLK_MPEG1, 24),
	MESON_GATE(CLKID_DOS_PARSER, HHI_GCLK_MPEG1, 25),
	MESON_GATE(CLKID_USB, HHI_GCLK_MPEG1, 26),
	MESON_GATE(CLKID_VDIN1, HHI_GCLK_MPEG1, 28),
	MESON_GATE(CLKID_AHB_ARB0, HHI_GCLK_MPEG1, 29),
	MESON_GATE(CLKID_EFUSE, HHI_GCLK_MPEG1, 30),
	MESON_GATE(CLKID_BOOT_ROM, HHI_GCLK_MPEG1, 31),

	MESON_GATE(CLKID_AHB_DATA_BUS, HHI_GCLK_MPEG2, 1),
	MESON_GATE(CLKID_AHB_CTRL_BUS, HHI_GCLK_MPEG2, 2),
	MESON_GATE(CLKID_HDMI_INTR_SYNC, HHI_GCLK_MPEG2, 3),
	MESON_GATE(CLKID_HDMI_PCLK, HHI_GCLK_MPEG2, 4),
	MESON_GATE(CLKID_USB1_DDR_BRIDGE, HHI_GCLK_MPEG2, 8),
	MESON_GATE(CLKID_USB0_DDR_BRIDGE, HHI_GCLK_MPEG2, 9),
	MESON_GATE(CLKID_MMC_PCLK, HHI_GCLK_MPEG2, 11),
	MESON_GATE(CLKID_DVIN, HHI_GCLK_MPEG2, 12),
	MESON_GATE(CLKID_UART2, HHI_GCLK_MPEG2, 15),
	MESON_GATE(CLKID_SANA, HHI_GCLK_MPEG2, 22),
	MESON_GATE(CLKID_VPU_INTR, HHI_GCLK_MPEG2, 25),
	MESON_GATE(CLKID_SEC_AHB_AHB3_BRIDGE, HHI_GCLK_MPEG2, 26),
	MESON_GATE(CLKID_CLK81_A53, HHI_GCLK_MPEG2, 29),

	MESON_GATE(CLKID_VCLK2_VENCI0, HHI_GCLK_OTHER, 1),
	MESON_GATE(CLKID_VCLK2_VENCI1, HHI_GCLK_OTHER, 2),
	MESON_GATE(CLKID_VCLK2_VENCP0, HHI_GCLK_OTHER, 3),
	MESON_GATE(CLKID_VCLK2_VENCP1, HHI_GCLK_OTHER, 4),
	MESON_GATE(CLKID_GCLK_VENCI_INT0, HHI_GCLK_OTHER, 8),
	MESON_GATE(CLKID_DAC_CLK, HHI_GCLK_OTHER, 10),
	MESON_GATE(CLKID_AOCLK_GATE, HHI_GCLK_OTHER, 14),
	MESON_GATE(CLKID_IEC958_GATE, HHI_GCLK_OTHER, 16),
	MESON_GATE(CLKID_ENC480P, HHI_GCLK_OTHER, 20),
	MESON_GATE(CLKID_RNG1, HHI_GCLK_OTHER, 21),
	MESON_GATE(CLKID_GCLK_VENCI_INT1, HHI_GCLK_OTHER, 22),
	MESON_GATE(CLKID_VCLK2_VENCLMCC, HHI_GCLK_OTHER, 24),
	MESON_GATE(CLKID_VCLK2_VENCL, HHI_GCLK_OTHER, 25),
	MESON_GATE(CLKID_VCLK_OTHER, HHI_GCLK_OTHER, 26),
	MESON_GATE(CLKID_EDP, HHI_GCLK_OTHER, 31),

	/* Always On (AO) domain gates */
	MESON_GATE(CLKID_AO_MEDIA_CPU, HHI_GCLK_AO, 0),
	MESON_GATE(CLKID_AO_AHB_SRAM, HHI_GCLK_AO, 1),
	MESON_GATE(CLKID_AO_AHB_BUS, HHI_GCLK_AO, 2),
	MESON_GATE(CLKID_AO_IFACE, HHI_GCLK_AO, 3),
	MESON_GATE(CLKID_AO_I2C, HHI_GCLK_AO, 4),

	/* PLL Gates */
	/* CLKID_FCLK_DIV2 is critical for the SCPI Processor */
	MESON_GATE(CLKID_FCLK_DIV3, HHI_MPLL_CNTL6, 28),
	MESON_GATE(CLKID_FCLK_DIV4, HHI_MPLL_CNTL6, 29),
	MESON_GATE(CLKID_FCLK_DIV5, HHI_MPLL_CNTL6, 30),
	MESON_GATE(CLKID_FCLK_DIV7, HHI_MPLL_CNTL6, 31),
	MESON_GATE(CLKID_MPLL0, HHI_MPLL_CNTL7, 14),
	MESON_GATE(CLKID_MPLL1, HHI_MPLL_CNTL8, 14),
	MESON_GATE(CLKID_MPLL2, HHI_MPLL_CNTL9, 14),
	/* CLKID_CLK81 is critical for the system */

	/* Peripheral Gates */
	MESON_GATE(CLKID_SAR_ADC_CLK, HHI_SAR_CLK_CNTL, 8),
	MESON_GATE(CLKID_SD_EMMC_A_CLK0, HHI_SD_EMMC_CLK_CNTL, 7),
	MESON_GATE(CLKID_SD_EMMC_B_CLK0, HHI_SD_EMMC_CLK_CNTL, 23),
	MESON_GATE(CLKID_SD_EMMC_C_CLK0, HHI_NAND_CLK_CNTL, 7),
	MESON_GATE(CLKID_VPU_0, HHI_VPU_CLK_CNTL, 8),
	MESON_GATE(CLKID_VPU_1, HHI_VPU_CLK_CNTL, 24),
	MESON_GATE(CLKID_VAPB_0, HHI_VAPBCLK_CNTL, 8),
	MESON_GATE(CLKID_VAPB_1, HHI_VAPBCLK_CNTL, 24),
	MESON_GATE(CLKID_VAPB, HHI_VAPBCLK_CNTL, 30),
};

static int meson_set_gate_by_id(struct clk *clk, unsigned long id, bool on)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	struct meson_gate *gate;

	debug("%s: %sabling %ld\n", __func__, on ? "en" : "dis", id);

	/* Propagate through muxes */
	switch (id) {
	case CLKID_VPU:
		return meson_set_gate_by_id(clk,
				meson_mux_get_parent(clk, CLKID_VPU), on);
	case CLKID_VAPB_SEL:
		return meson_set_gate_by_id(clk,
				meson_mux_get_parent(clk, CLKID_VAPB_SEL), on);
	}

	if (id >= ARRAY_SIZE(gates))
		return -ENOENT;

	gate = &gates[id];

	if (gate->reg == 0)
		return 0;

	debug("%s: really %sabling %ld\n", __func__, on ? "en" : "dis", id);

	regmap_update_bits(priv->map, gate->reg,
			   BIT(gate->bit), on ? BIT(gate->bit) : 0);

	/* Propagate to next gate(s) */
	switch (id) {
	case CLKID_VAPB:
		return meson_set_gate_by_id(clk, CLKID_VAPB_SEL, on);
	}

	return 0;
}

static int meson_clk_enable(struct clk *clk)
{
	return meson_set_gate_by_id(clk, clk->id, true);
}

static int meson_clk_disable(struct clk *clk)
{
	return meson_set_gate_by_id(clk, clk->id, false);
}

static struct parm meson_vpu_0_div_parm = {
	HHI_VPU_CLK_CNTL, 0, 7,
};

int meson_vpu_0_div_parent = CLKID_VPU_0_SEL;

static struct parm meson_vpu_1_div_parm = {
	HHI_VPU_CLK_CNTL, 16, 7,
};

int meson_vpu_1_div_parent = CLKID_VPU_1_SEL;

static struct parm meson_vapb_0_div_parm = {
	HHI_VAPBCLK_CNTL, 0, 7,
};

int meson_vapb_0_div_parent = CLKID_VAPB_0_SEL;

static struct parm meson_vapb_1_div_parm = {
	HHI_VAPBCLK_CNTL, 16, 7,
};

int meson_vapb_1_div_parent = CLKID_VAPB_1_SEL;

static ulong meson_div_get_rate(struct clk *clk, unsigned long id)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	unsigned int rate, parent_rate;
	struct parm *parm;
	int parent;
	uint reg;

	switch (id) {
	case CLKID_VPU_0_DIV:
		parm = &meson_vpu_0_div_parm;
		parent = meson_vpu_0_div_parent;
		break;
	case CLKID_VPU_1_DIV:
		parm = &meson_vpu_1_div_parm;
		parent = meson_vpu_1_div_parent;
		break;
	case CLKID_VAPB_0_DIV:
		parm = &meson_vapb_0_div_parm;
		parent = meson_vapb_0_div_parent;
		break;
	case CLKID_VAPB_1_DIV:
		parm = &meson_vapb_1_div_parm;
		parent = meson_vapb_1_div_parent;
		break;
	default:
		return -ENOENT;
	}

	regmap_read(priv->map, parm->reg_off, &reg);
	reg = PARM_GET(parm->width, parm->shift, reg);

	debug("%s: div of %ld is %d\n", __func__, id, reg + 1);

	parent_rate = meson_clk_get_rate_by_id(clk, parent);
	if (IS_ERR_VALUE(parent_rate))
		return parent_rate;

	debug("%s: parent rate of %ld is %d\n", __func__, id, parent_rate);

	rate = parent_rate / (reg + 1);

	debug("%s: rate of %ld is %d\n", __func__, id, rate);

	return rate;
}

static ulong meson_div_set_rate(struct clk *clk, unsigned long id, ulong rate,
				ulong current_rate)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	unsigned int new_div = -EINVAL;
	unsigned long parent_rate;
	struct parm *parm;
	int parent;
	int ret;

	if (current_rate == rate)
		return 0;

	debug("%s: setting rate of %ld from %ld to %ld\n",
	      __func__, id, current_rate, rate);

	switch (id) {
	case CLKID_VPU_0_DIV:
		parm = &meson_vpu_0_div_parm;
		parent = meson_vpu_0_div_parent;
		break;
	case CLKID_VPU_1_DIV:
		parm = &meson_vpu_1_div_parm;
		parent = meson_vpu_1_div_parent;
		break;
	case CLKID_VAPB_0_DIV:
		parm = &meson_vapb_0_div_parm;
		parent = meson_vapb_0_div_parent;
		break;
	case CLKID_VAPB_1_DIV:
		parm = &meson_vapb_1_div_parm;
		parent = meson_vapb_1_div_parent;
		break;
	default:
		return -ENOENT;
	}

	parent_rate = meson_clk_get_rate_by_id(clk, parent);
	if (IS_ERR_VALUE(parent_rate))
		return parent_rate;

	debug("%s: parent rate of %ld is %ld\n", __func__, id, parent_rate);

	/* If can't divide, set parent instead */
	if (!parent_rate || rate > parent_rate)
		return meson_clk_set_rate_by_id(clk, parent, rate,
						current_rate);

	new_div = DIV_ROUND_CLOSEST(parent_rate, rate);

	debug("%s: new div of %ld is %d\n", __func__, id, new_div);

	/* If overflow, try to set parent rate and retry */
	if (!new_div || new_div > (1 << parm->width)) {
		ret = meson_clk_set_rate_by_id(clk, parent, rate, current_rate);
		if (IS_ERR_VALUE(ret))
			return ret;

		parent_rate = meson_clk_get_rate_by_id(clk, parent);
		if (IS_ERR_VALUE(parent_rate))
			return parent_rate;

		new_div = DIV_ROUND_CLOSEST(parent_rate, rate);

		debug("%s: new new div of %ld is %d\n", __func__, id, new_div);

		if (!new_div || new_div > (1 << parm->width))
			return -EINVAL;
	}

	debug("%s: setting div of %ld to %d\n", __func__, id, new_div);

	regmap_update_bits(priv->map, parm->reg_off, SETPMASK(parm->width, parm->shift),
			   (new_div - 1) << parm->shift);

	debug("%s: new rate of %ld is %ld\n",
	      __func__, id, meson_div_get_rate(clk, id));

	return 0;
}

static struct parm meson_vpu_mux_parm = {
	HHI_VPU_CLK_CNTL, 31, 1,
};

int meson_vpu_mux_parents[] = {
	CLKID_VPU_0,
	CLKID_VPU_1,
};

static struct parm meson_vpu_0_mux_parm = {
	HHI_VPU_CLK_CNTL, 9, 2,
};

static struct parm meson_vpu_1_mux_parm = {
	HHI_VPU_CLK_CNTL, 25, 2,
};

static int meson_vpu_0_1_mux_parents[] = {
	CLKID_FCLK_DIV4,
	CLKID_FCLK_DIV3,
	CLKID_FCLK_DIV5,
	CLKID_FCLK_DIV7,
};

static struct parm meson_vapb_sel_mux_parm = {
	HHI_VAPBCLK_CNTL, 31, 1,
};

int meson_vapb_sel_mux_parents[] = {
	CLKID_VAPB_0,
	CLKID_VAPB_1,
};

static struct parm meson_vapb_0_mux_parm = {
	HHI_VAPBCLK_CNTL, 9, 2,
};

static struct parm meson_vapb_1_mux_parm = {
	HHI_VAPBCLK_CNTL, 25, 2,
};

static int meson_vapb_0_1_mux_parents[] = {
	CLKID_FCLK_DIV4,
	CLKID_FCLK_DIV3,
	CLKID_FCLK_DIV5,
	CLKID_FCLK_DIV7,
};

static ulong meson_mux_get_parent(struct clk *clk, unsigned long id)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	struct parm *parm;
	int *parents;
	uint reg;

	switch (id) {
	case CLKID_VPU:
		parm = &meson_vpu_mux_parm;
		parents = meson_vpu_mux_parents;
		break;
	case CLKID_VPU_0_SEL:
		parm = &meson_vpu_0_mux_parm;
		parents = meson_vpu_0_1_mux_parents;
		break;
	case CLKID_VPU_1_SEL:
		parm = &meson_vpu_1_mux_parm;
		parents = meson_vpu_0_1_mux_parents;
		break;
	case CLKID_VAPB_SEL:
		parm = &meson_vapb_sel_mux_parm;
		parents = meson_vapb_sel_mux_parents;
		break;
	case CLKID_VAPB_0_SEL:
		parm = &meson_vapb_0_mux_parm;
		parents = meson_vapb_0_1_mux_parents;
		break;
	case CLKID_VAPB_1_SEL:
		parm = &meson_vapb_1_mux_parm;
		parents = meson_vapb_0_1_mux_parents;
		break;
	default:
		return -ENOENT;
	}

	regmap_read(priv->map, parm->reg_off, &reg);
	reg = PARM_GET(parm->width, parm->shift, reg);

	debug("%s: parent of %ld is %d (%d)\n",
	      __func__, id, parents[reg], reg);

	return parents[reg];
}

static ulong meson_mux_set_parent(struct clk *clk, unsigned long id,
				  unsigned long parent_id)
{
	unsigned long cur_parent = meson_mux_get_parent(clk, id);
	struct meson_clk *priv = dev_get_priv(clk->dev);
	unsigned int new_index = -EINVAL;
	struct parm *parm;
	int *parents;
	int i;

	if (IS_ERR_VALUE(cur_parent))
		return cur_parent;

	debug("%s: setting parent of %ld from %ld to %ld\n",
	      __func__, id, cur_parent, parent_id);

	if (cur_parent == parent_id)
		return 0;

	switch (id) {
	case CLKID_VPU:
		parm = &meson_vpu_mux_parm;
		parents = meson_vpu_mux_parents;
		break;
	case CLKID_VPU_0_SEL:
		parm = &meson_vpu_0_mux_parm;
		parents = meson_vpu_0_1_mux_parents;
		break;
	case CLKID_VPU_1_SEL:
		parm = &meson_vpu_1_mux_parm;
		parents = meson_vpu_0_1_mux_parents;
		break;
	case CLKID_VAPB_SEL:
		parm = &meson_vapb_sel_mux_parm;
		parents = meson_vapb_sel_mux_parents;
		break;
	case CLKID_VAPB_0_SEL:
		parm = &meson_vapb_0_mux_parm;
		parents = meson_vapb_0_1_mux_parents;
		break;
	case CLKID_VAPB_1_SEL:
		parm = &meson_vapb_1_mux_parm;
		parents = meson_vapb_0_1_mux_parents;
		break;
	default:
		/* Not a mux */
		return -ENOENT;
	}

	for (i = 0 ; i < (1 << parm->width) ; ++i) {
		if (parents[i] == parent_id)
			new_index = i;
	}

	if (IS_ERR_VALUE(new_index))
		return new_index;

	debug("%s: new index of %ld is %d\n", __func__, id, new_index);

	regmap_update_bits(priv->map, parm->reg_off, SETPMASK(parm->width, parm->shift),
			   new_index << parm->shift);

	debug("%s: new parent of %ld is %ld\n",
	      __func__, id, meson_mux_get_parent(clk, id));

	return 0;
}

static ulong meson_mux_get_rate(struct clk *clk, unsigned long id)
{
	int parent = meson_mux_get_parent(clk, id);

	if (IS_ERR_VALUE(parent))
		return parent;

	return meson_clk_get_rate_by_id(clk, parent);
}

static unsigned long meson_clk81_get_rate(struct clk *clk)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	unsigned long parent_rate;
	uint reg;
	int parents[] = {
		-1,
		-1,
		CLKID_FCLK_DIV7,
		CLKID_MPLL1,
		CLKID_MPLL2,
		CLKID_FCLK_DIV4,
		CLKID_FCLK_DIV3,
		CLKID_FCLK_DIV5
	};

	/* mux */
	regmap_read(priv->map, HHI_MPEG_CLK_CNTL, &reg);
	reg = (reg >> 12) & 7;

	switch (reg) {
	case 0:
		parent_rate = XTAL_RATE;
		break;
	case 1:
		return -ENOENT;
	default:
		parent_rate = meson_clk_get_rate_by_id(clk, parents[reg]);
	}

	/* divider */
	regmap_read(priv->map, HHI_MPEG_CLK_CNTL, &reg);
	reg = reg & ((1 << 7) - 1);

	/* clk81 divider is zero based */
	return parent_rate / (reg + 1);
}

static long mpll_rate_from_params(unsigned long parent_rate,
				  unsigned long sdm,
				  unsigned long n2)
{
	unsigned long divisor = (SDM_DEN * n2) + sdm;

	if (n2 < N2_MIN)
		return -EINVAL;

	return DIV_ROUND_UP_ULL((u64)parent_rate * SDM_DEN, divisor);
}

static struct parm meson_mpll0_parm[3] = {
	{HHI_MPLL_CNTL7, 0, 14}, /* psdm */
	{HHI_MPLL_CNTL7, 16, 9}, /* pn2 */
};

static struct parm meson_mpll1_parm[3] = {
	{HHI_MPLL_CNTL8, 0, 14}, /* psdm */
	{HHI_MPLL_CNTL8, 16, 9}, /* pn2 */
};

static struct parm meson_mpll2_parm[3] = {
	{HHI_MPLL_CNTL9, 0, 14}, /* psdm */
	{HHI_MPLL_CNTL9, 16, 9}, /* pn2 */
};

/*
 * MultiPhase Locked Loops are outputs from a PLL with additional frequency
 * scaling capabilities. MPLL rates are calculated as:
 *
 * f(N2_integer, SDM_IN ) = 2.0G/(N2_integer + SDM_IN/16384)
 */
static ulong meson_mpll_get_rate(struct clk *clk, unsigned long id)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	struct parm *psdm, *pn2;
	unsigned long sdm, n2;
	unsigned long parent_rate;
	uint reg;

	switch (id) {
	case CLKID_MPLL0:
		psdm = &meson_mpll0_parm[0];
		pn2 = &meson_mpll0_parm[1];
		break;
	case CLKID_MPLL1:
		psdm = &meson_mpll1_parm[0];
		pn2 = &meson_mpll1_parm[1];
		break;
	case CLKID_MPLL2:
		psdm = &meson_mpll2_parm[0];
		pn2 = &meson_mpll2_parm[1];
		break;
	default:
		return -ENOENT;
	}

	parent_rate = meson_clk_get_rate_by_id(clk, CLKID_FIXED_PLL);
	if (IS_ERR_VALUE(parent_rate))
		return parent_rate;

	regmap_read(priv->map, psdm->reg_off, &reg);
	sdm = PARM_GET(psdm->width, psdm->shift, reg);

	regmap_read(priv->map, pn2->reg_off, &reg);
	n2 = PARM_GET(pn2->width, pn2->shift, reg);

	return mpll_rate_from_params(parent_rate, sdm, n2);
}

static struct parm meson_fixed_pll_parm[3] = {
	{HHI_MPLL_CNTL, 0, 9}, /* pm */
	{HHI_MPLL_CNTL, 9, 5}, /* pn */
	{HHI_MPLL_CNTL, 16, 2}, /* pod */
};

static struct parm meson_sys_pll_parm[3] = {
	{HHI_SYS_PLL_CNTL, 0, 9}, /* pm */
	{HHI_SYS_PLL_CNTL, 9, 5}, /* pn */
	{HHI_SYS_PLL_CNTL, 10, 2}, /* pod */
};

static ulong meson_pll_get_rate(struct clk *clk, unsigned long id)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	struct parm *pm, *pn, *pod;
	unsigned long parent_rate_mhz = XTAL_RATE / 1000000;
	u16 n, m, od;
	uint reg;

	switch (id) {
	case CLKID_FIXED_PLL:
		pm = &meson_fixed_pll_parm[0];
		pn = &meson_fixed_pll_parm[1];
		pod = &meson_fixed_pll_parm[2];
		break;
	case CLKID_SYS_PLL:
		pm = &meson_sys_pll_parm[0];
		pn = &meson_sys_pll_parm[1];
		pod = &meson_sys_pll_parm[2];
		break;
	default:
		return -ENOENT;
	}

	regmap_read(priv->map, pn->reg_off, &reg);
	n = PARM_GET(pn->width, pn->shift, reg);

	regmap_read(priv->map, pm->reg_off, &reg);
	m = PARM_GET(pm->width, pm->shift, reg);

	regmap_read(priv->map, pod->reg_off, &reg);
	od = PARM_GET(pod->width, pod->shift, reg);

	return ((parent_rate_mhz * m / n) >> od) * 1000000;
}

static ulong meson_clk_get_rate_by_id(struct clk *clk, unsigned long id)
{
	ulong rate;

	switch (id) {
	case CLKID_FIXED_PLL:
	case CLKID_SYS_PLL:
		rate = meson_pll_get_rate(clk, id);
		break;
	case CLKID_FCLK_DIV2:
		rate = meson_pll_get_rate(clk, CLKID_FIXED_PLL) / 2;
		break;
	case CLKID_FCLK_DIV3:
		rate = meson_pll_get_rate(clk, CLKID_FIXED_PLL) / 3;
		break;
	case CLKID_FCLK_DIV4:
		rate = meson_pll_get_rate(clk, CLKID_FIXED_PLL) / 4;
		break;
	case CLKID_FCLK_DIV5:
		rate = meson_pll_get_rate(clk, CLKID_FIXED_PLL) / 5;
		break;
	case CLKID_FCLK_DIV7:
		rate = meson_pll_get_rate(clk, CLKID_FIXED_PLL) / 7;
		break;
	case CLKID_MPLL0:
	case CLKID_MPLL1:
	case CLKID_MPLL2:
		rate = meson_mpll_get_rate(clk, id);
		break;
	case CLKID_CLK81:
		rate = meson_clk81_get_rate(clk);
		break;
	case CLKID_VPU_0:
		rate = meson_div_get_rate(clk, CLKID_VPU_0_DIV);
		break;
	case CLKID_VPU_1:
		rate = meson_div_get_rate(clk, CLKID_VPU_1_DIV);
		break;
	case CLKID_VAPB:
		rate = meson_mux_get_rate(clk, CLKID_VAPB_SEL);
		break;
	case CLKID_VAPB_0:
		rate = meson_div_get_rate(clk, CLKID_VAPB_0_DIV);
		break;
	case CLKID_VAPB_1:
		rate = meson_div_get_rate(clk, CLKID_VAPB_1_DIV);
		break;
	case CLKID_VPU_0_DIV:
	case CLKID_VPU_1_DIV:
	case CLKID_VAPB_0_DIV:
	case CLKID_VAPB_1_DIV:
		rate = meson_div_get_rate(clk, id);
		break;
	case CLKID_VPU:
	case CLKID_VPU_0_SEL:
	case CLKID_VPU_1_SEL:
	case CLKID_VAPB_SEL:
	case CLKID_VAPB_0_SEL:
	case CLKID_VAPB_1_SEL:
		rate = meson_mux_get_rate(clk, id);
		break;
	default:
		if (gates[id].reg != 0) {
			/* a clock gate */
			rate = meson_clk81_get_rate(clk);
			break;
		}
		return -ENOENT;
	}

	debug("clock %lu has rate %lu\n", id, rate);
	return rate;
}

static ulong meson_clk_get_rate(struct clk *clk)
{
	return meson_clk_get_rate_by_id(clk, clk->id);
}

static int meson_clk_set_parent(struct clk *clk, struct clk *parent)
{
	return meson_mux_set_parent(clk, clk->id, parent->id);
}

static ulong meson_clk_set_rate_by_id(struct clk *clk, unsigned long id,
				      ulong rate, ulong current_rate)
{
	if (current_rate == rate)
		return 0;

	switch (id) {
	/* Fixed clocks */
	case CLKID_FIXED_PLL:
	case CLKID_SYS_PLL:
	case CLKID_FCLK_DIV2:
	case CLKID_FCLK_DIV3:
	case CLKID_FCLK_DIV4:
	case CLKID_FCLK_DIV5:
	case CLKID_FCLK_DIV7:
	case CLKID_MPLL0:
	case CLKID_MPLL1:
	case CLKID_MPLL2:
	case CLKID_CLK81:
		if (current_rate != rate)
			return -EINVAL;

		return 0;
	case CLKID_VPU:
		return meson_clk_set_rate_by_id(clk,
				meson_mux_get_parent(clk, CLKID_VPU), rate,
						     current_rate);
	case CLKID_VAPB:
	case CLKID_VAPB_SEL:
		return meson_clk_set_rate_by_id(clk,
				meson_mux_get_parent(clk, CLKID_VAPB_SEL),
				rate, current_rate);
	case CLKID_VPU_0:
		return meson_div_set_rate(clk, CLKID_VPU_0_DIV, rate,
					  current_rate);
	case CLKID_VPU_1:
		return meson_div_set_rate(clk, CLKID_VPU_1_DIV, rate,
					  current_rate);
	case CLKID_VAPB_0:
		return meson_div_set_rate(clk, CLKID_VAPB_0_DIV, rate,
					  current_rate);
	case CLKID_VAPB_1:
		return meson_div_set_rate(clk, CLKID_VAPB_1_DIV, rate,
					  current_rate);
	case CLKID_VPU_0_DIV:
	case CLKID_VPU_1_DIV:
	case CLKID_VAPB_0_DIV:
	case CLKID_VAPB_1_DIV:
		return meson_div_set_rate(clk, id, rate, current_rate);
	default:
		return -ENOENT;
	}

	return -EINVAL;
}

static ulong meson_clk_set_rate(struct clk *clk, ulong rate)
{
	ulong current_rate = meson_clk_get_rate_by_id(clk, clk->id);
	int ret;

	if (IS_ERR_VALUE(current_rate))
		return current_rate;

	debug("%s: setting rate of %ld from %ld to %ld\n",
	      __func__, clk->id, current_rate, rate);

	ret = meson_clk_set_rate_by_id(clk, clk->id, rate, current_rate);
	if (IS_ERR_VALUE(ret))
		return ret;

	debug("clock %lu has new rate %lu\n", clk->id,
	      meson_clk_get_rate_by_id(clk, clk->id));

	return 0;
}

static int meson_clk_probe(struct udevice *dev)
{
	struct meson_clk *priv = dev_get_priv(dev);

	priv->map = syscon_node_to_regmap(dev_get_parent(dev)->node);
	if (IS_ERR(priv->map))
		return PTR_ERR(priv->map);

	debug("meson-clk: probed\n");

	return 0;
}

static struct clk_ops meson_clk_ops = {
	.disable	= meson_clk_disable,
	.enable		= meson_clk_enable,
	.get_rate	= meson_clk_get_rate,
	.set_parent	= meson_clk_set_parent,
	.set_rate	= meson_clk_set_rate,
};

static const struct udevice_id meson_clk_ids[] = {
	{ .compatible = "amlogic,gxbb-clkc" },
	{ .compatible = "amlogic,gxl-clkc" },
	{ }
};

U_BOOT_DRIVER(meson_clk) = {
	.name		= "meson_clk",
	.id		= UCLASS_CLK,
	.of_match	= meson_clk_ids,
	.priv_auto_alloc_size = sizeof(struct meson_clk),
	.ops		= &meson_clk_ops,
	.probe		= meson_clk_probe,
};
