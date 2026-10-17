// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 - Beniamino Galvani <b.galvani@gmail.com>
 * (C) Copyright 2018 - BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <log.h>
#include <asm/arch/clock-g12a.h>
#include <asm/io.h>
#include <clk-uclass.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <div64.h>
#include <dt-bindings/clock/g12a-clkc.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kernel.h>
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
#define CLKID_MPEG_SEL				8
#define CLKID_MPEG_DIV				9
#define CLKID_SD_EMMC_A_CLK0_SEL		63
#define CLKID_SD_EMMC_A_CLK0_DIV		64
#define CLKID_SD_EMMC_B_CLK0_SEL		65
#define CLKID_SD_EMMC_B_CLK0_DIV		66
#define CLKID_SD_EMMC_C_CLK0_SEL		67
#define CLKID_SD_EMMC_C_CLK0_DIV		68
#define CLKID_MPLL0_DIV				69
#define CLKID_MPLL1_DIV				70
#define CLKID_MPLL2_DIV				71
#define CLKID_MPLL3_DIV				72
#define CLKID_MPLL_PREDIV			73
#define CLKID_FCLK_DIV2_DIV			75
#define CLKID_FCLK_DIV3_DIV			76
#define CLKID_FCLK_DIV4_DIV			77
#define CLKID_FCLK_DIV5_DIV			78
#define CLKID_FCLK_DIV7_DIV			79
#define CLKID_FCLK_DIV2P5_DIV			100
#define CLKID_FIXED_PLL_DCO			101
#define CLKID_SYS_PLL_DCO			102
#define CLKID_GP0_PLL_DCO			103
#define CLKID_HIFI_PLL_DCO			104
#define CLKID_VPU_0_DIV				111
#define CLKID_VPU_1_DIV				114
#define CLKID_VAPB_0_DIV			118
#define CLKID_VAPB_1_DIV			121
#define CLKID_HDMI_PLL_DCO			125
#define CLKID_HDMI_PLL_OD			126
#define CLKID_HDMI_PLL_OD2			127
#define CLKID_VID_PLL_SEL			130
#define CLKID_VID_PLL_DIV			131
#define CLKID_VCLK_SEL				132
#define CLKID_VCLK2_SEL				133
#define CLKID_VCLK_INPUT			134
#define CLKID_VCLK2_INPUT			135
#define CLKID_VCLK_DIV				136
#define CLKID_VCLK2_DIV				137
#define CLKID_VCLK_DIV2_EN			140
#define CLKID_VCLK_DIV4_EN			141
#define CLKID_VCLK_DIV6_EN			142
#define CLKID_VCLK_DIV12_EN			143
#define CLKID_VCLK2_DIV2_EN			144
#define CLKID_VCLK2_DIV4_EN			145
#define CLKID_VCLK2_DIV6_EN			146
#define CLKID_VCLK2_DIV12_EN			147
#define CLKID_CTS_ENCI_SEL			158
#define CLKID_CTS_ENCP_SEL			159
#define CLKID_CTS_VDAC_SEL			160
#define CLKID_HDMI_TX_SEL			161
#define CLKID_HDMI_SEL				166
#define CLKID_HDMI_DIV				167
#define CLKID_MALI_0_DIV			170
#define CLKID_MALI_1_DIV			173

#define CLKID_XTAL				0x10000000

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

#define NUM_CLKS 178

static struct meson_gate gates[NUM_CLKS] = {
	/* Everything Else (EE) domain gates */
	MESON_GATE(CLKID_SPICC0, HHI_GCLK_MPEG0, 8),
	MESON_GATE(CLKID_I2C, HHI_GCLK_MPEG0, 9),
	MESON_GATE(CLKID_UART0, HHI_GCLK_MPEG0, 13),
	MESON_GATE(CLKID_SPICC1, HHI_GCLK_MPEG0, 14),
	MESON_GATE(CLKID_SD_EMMC_A, HHI_GCLK_MPEG0, 4),
	MESON_GATE(CLKID_SD_EMMC_B, HHI_GCLK_MPEG0, 25),
	MESON_GATE(CLKID_SD_EMMC_C, HHI_GCLK_MPEG0, 26),
	MESON_GATE(CLKID_ETH, HHI_GCLK_MPEG1, 3),
	MESON_GATE(CLKID_UART1, HHI_GCLK_MPEG1, 16),
	MESON_GATE(CLKID_PCIE_COMB, HHI_GCLK_MPEG1, 24),
	MESON_GATE(CLKID_USB, HHI_GCLK_MPEG1, 25),
	MESON_GATE(CLKID_PCIE_PHY, HHI_GCLK_MPEG1, 27),
	MESON_GATE(CLKID_HTX_PCLK, HHI_GCLK_MPEG2, 4),
	MESON_GATE(CLKID_USB1_DDR_BRIDGE, HHI_GCLK_MPEG2, 8),
	MESON_GATE(CLKID_VPU_INTR, HHI_GCLK_MPEG2, 25),

	/* Peripheral Gates */
	MESON_GATE(CLKID_FCLK_DIV2, HHI_FIX_PLL_CNTL1, 24),
	MESON_GATE(CLKID_FCLK_DIV3, HHI_FIX_PLL_CNTL1, 20),
	MESON_GATE(CLKID_FCLK_DIV4, HHI_FIX_PLL_CNTL1, 21),
	MESON_GATE(CLKID_FCLK_DIV5, HHI_FIX_PLL_CNTL1, 22),
	MESON_GATE(CLKID_FCLK_DIV7, HHI_FIX_PLL_CNTL1, 23),
	MESON_GATE(CLKID_SD_EMMC_A_CLK0, HHI_SD_EMMC_CLK_CNTL, 7),
	MESON_GATE(CLKID_SD_EMMC_B_CLK0, HHI_SD_EMMC_CLK_CNTL, 23),
	MESON_GATE(CLKID_SD_EMMC_C_CLK0, HHI_NAND_CLK_CNTL, 7),
	MESON_GATE(CLKID_VPU_0, HHI_VPU_CLK_CNTL, 8),
	MESON_GATE(CLKID_VPU_1, HHI_VPU_CLK_CNTL, 24),
	MESON_GATE(CLKID_VAPB_0, HHI_VAPBCLK_CNTL, 8),
	MESON_GATE(CLKID_VAPB_1, HHI_VAPBCLK_CNTL, 24),
	MESON_GATE(CLKID_VAPB, HHI_VAPBCLK_CNTL, 30),
	MESON_GATE(CLKID_HDMI, HHI_HDMI_CLK_CNTL, 8),
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
	case CLKID_VAPB_0:
		return meson_set_gate_by_id(clk,
			meson_mux_get_parent(clk, CLKID_VAPB_0_SEL), on);
	case CLKID_VAPB_1:
		return meson_set_gate_by_id(clk,
			meson_mux_get_parent(clk, CLKID_VAPB_0_SEL), on);
	case CLKID_VPU_0:
		return meson_set_gate_by_id(clk,
			meson_mux_get_parent(clk, CLKID_VPU_0_SEL), on);
	case CLKID_VPU_1:
		return meson_set_gate_by_id(clk,
			meson_mux_get_parent(clk, CLKID_VPU_1_SEL), on);
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

static struct parm meson_hdmi_div_parm = {
	HHI_HDMI_CLK_CNTL, 0, 7,
};

int meson_hdmi_div_parent = CLKID_HDMI_SEL;

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
	case CLKID_HDMI_DIV:
		parm = &meson_hdmi_div_parm;
		parent = meson_hdmi_div_parent;
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
	case CLKID_HDMI_DIV:
		parm = &meson_hdmi_div_parm;
		parent = meson_hdmi_div_parent;
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

	regmap_update_bits(priv->map, parm->reg_off,
			   SETPMASK(parm->width, parm->shift),
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
	HHI_VPU_CLK_CNTL, 9, 3,
};

static struct parm meson_vpu_1_mux_parm = {
	HHI_VPU_CLK_CNTL, 25, 3,
};

static int meson_vpu_0_1_mux_parents[] = {
	CLKID_FCLK_DIV3,
	CLKID_FCLK_DIV4,
	CLKID_FCLK_DIV5,
	CLKID_FCLK_DIV7,
	-ENOENT,
	-ENOENT,
	-ENOENT,
	-ENOENT,
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

static struct parm meson_hdmi_mux_parm = {
	HHI_HDMI_CLK_CNTL, 9, 2,
};

static int meson_hdmi_mux_parents[] = {
	CLKID_XTAL,
	CLKID_FCLK_DIV4,
	CLKID_FCLK_DIV3,
	CLKID_FCLK_DIV5,
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
	case CLKID_HDMI_SEL:
		parm = &meson_hdmi_mux_parm;
		parents = meson_hdmi_mux_parents;
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
	case CLKID_HDMI_SEL:
		parm = &meson_hdmi_mux_parm;
		parents = meson_hdmi_mux_parents;
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

	regmap_update_bits(priv->map, parm->reg_off,
			   SETPMASK(parm->width, parm->shift),
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
		CLKID_XTAL,
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
	case 1:
		return -ENOENT;
	default:
		parent_rate = meson_clk_get_rate_by_id(clk, parents[reg]);
	}

	/* divider */
	regmap_read(priv->map, HHI_MPEG_CLK_CNTL, &reg);
	reg = reg & ((1 << 7) - 1);

	return parent_rate / reg;
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

static struct parm meson_mpll0_parm[2] = {
	{HHI_MPLL_CNTL1, 0, 14}, /* psdm */
	{HHI_MPLL_CNTL1, 20, 9}, /* pn2 */
};

static struct parm meson_mpll1_parm[2] = {
	{HHI_MPLL_CNTL3, 0, 14}, /* psdm */
	{HHI_MPLL_CNTL3, 20, 9}, /* pn2 */
};

static struct parm meson_mpll2_parm[2] = {
	{HHI_MPLL_CNTL5, 0, 14}, /* psdm */
	{HHI_MPLL_CNTL5, 20, 9}, /* pn2 */
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

static struct parm meson_fixed_pll_parm[4] = {
	{HHI_FIX_PLL_CNTL0, 0, 9}, /* pm */
	{HHI_FIX_PLL_CNTL0, 10, 5}, /* pn */
	{HHI_FIX_PLL_CNTL0, 16, 2}, /* pod */
	{HHI_FIX_PLL_CNTL1, 0, 17}, /* pfrac */
};

static struct parm meson_sys_pll_parm[3] = {
	{HHI_SYS_PLL_CNTL0, 0, 9}, /* pm */
	{HHI_SYS_PLL_CNTL0, 10, 5}, /* pn */
	{HHI_SYS_PLL_CNTL0, 16, 3}, /* pod */
};

static ulong meson_pll_get_rate(struct clk *clk, unsigned long id)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	struct parm *pm, *pn, *pod, *pfrac = NULL;
	unsigned long parent_rate_mhz = XTAL_RATE / 1000000;
	u16 n, m, od, frac;
	ulong rate;
	uint reg;

	/*
	 * FIXME: Between the unit conversion and the missing frac, we know
	 * rate will be slightly off ...
	*/

	switch (id) {
	case CLKID_FIXED_PLL:
		pm = &meson_fixed_pll_parm[0];
		pn = &meson_fixed_pll_parm[1];
		pod = &meson_fixed_pll_parm[2];
		pfrac = &meson_fixed_pll_parm[3];
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

	rate = parent_rate_mhz * m;

	if (pfrac) {
		ulong frac_rate;

		regmap_read(priv->map, pfrac->reg_off, &reg);
		frac = PARM_GET(pfrac->width - 1, pfrac->shift, reg);

		frac_rate = DIV_ROUND_UP_ULL((u64)parent_rate_mhz * frac,
					     1 << (pfrac->width - 2));

		if (frac & BIT(pfrac->width - 1))
			rate -= frac_rate;
		else
			rate += frac_rate;
	}

	return (DIV_ROUND_UP_ULL(rate, n) >> od) * 1000000;
}

static struct parm meson_pcie_pll_parm[3] = {
	{HHI_PCIE_PLL_CNTL0, 0, 8}, /* pm */
	{HHI_PCIE_PLL_CNTL0, 10, 5}, /* pn */
	{HHI_PCIE_PLL_CNTL0, 16, 5}, /* pod */
};

static ulong meson_pcie_pll_get_rate(struct clk *clk)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);
	struct parm *pm, *pn, *pod;
	unsigned long parent_rate_mhz = XTAL_RATE / 1000000;
	u16 n, m, od;
	uint reg;

	pm = &meson_pcie_pll_parm[0];
	pn = &meson_pcie_pll_parm[1];
	pod = &meson_pcie_pll_parm[2];

	regmap_read(priv->map, pn->reg_off, &reg);
	n = PARM_GET(pn->width, pn->shift, reg);

	regmap_read(priv->map, pm->reg_off, &reg);
	m = PARM_GET(pm->width, pm->shift, reg);

	regmap_read(priv->map, pod->reg_off, &reg);
	od = PARM_GET(pod->width, pod->shift, reg);

	return ((parent_rate_mhz * m / n) / 2 / od / 2) * 1000000;
}

static ulong meson_clk_get_rate_by_id(struct clk *clk, unsigned long id)
{
	ulong rate;

	switch (id) {
	case CLKID_XTAL:
		rate = XTAL_RATE;
		break;
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
	case CLKID_PCIE_PLL:
		rate = meson_pcie_pll_get_rate(clk);
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
	case CLKID_HDMI:
		rate = meson_div_get_rate(clk, CLKID_HDMI_DIV);
		break;
	case CLKID_VPU_0_DIV:
	case CLKID_VPU_1_DIV:
	case CLKID_VAPB_0_DIV:
	case CLKID_VAPB_1_DIV:
	case CLKID_HDMI_DIV:
		rate = meson_div_get_rate(clk, id);
		break;
	case CLKID_VPU:
	case CLKID_VPU_0_SEL:
	case CLKID_VPU_1_SEL:
	case CLKID_VAPB_SEL:
	case CLKID_VAPB_0_SEL:
	case CLKID_VAPB_1_SEL:
	case CLKID_HDMI_SEL:
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

static ulong meson_pcie_pll_set_rate(struct clk *clk, ulong rate)
{
	struct meson_clk *priv = dev_get_priv(clk->dev);

	regmap_write(priv->map, HHI_PCIE_PLL_CNTL0, 0x20090496);
	regmap_write(priv->map, HHI_PCIE_PLL_CNTL0, 0x30090496);
	regmap_write(priv->map, HHI_PCIE_PLL_CNTL1, 0x00000000);
	regmap_write(priv->map, HHI_PCIE_PLL_CNTL2, 0x00001100);
	regmap_write(priv->map, HHI_PCIE_PLL_CNTL3, 0x10058e00);
	regmap_write(priv->map, HHI_PCIE_PLL_CNTL4, 0x000100c0);
	regmap_write(priv->map, HHI_PCIE_PLL_CNTL5, 0x68000048);
	regmap_write(priv->map, HHI_PCIE_PLL_CNTL5, 0x68000068);
	udelay(20);
	regmap_write(priv->map, HHI_PCIE_PLL_CNTL4, 0x008100c0);
	udelay(10);
	regmap_write(priv->map, HHI_PCIE_PLL_CNTL0, 0x34090496);
	regmap_write(priv->map, HHI_PCIE_PLL_CNTL0, 0x14090496);
	udelay(10);
	regmap_write(priv->map, HHI_PCIE_PLL_CNTL2, 0x00001000);
	regmap_update_bits(priv->map, HHI_PCIE_PLL_CNTL0,
				0x1f << 16, 9 << 16);

	return 100000000;
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
	case CLKID_PCIE_PLL:
		return meson_pcie_pll_set_rate(clk, rate);

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
	case CLKID_HDMI_DIV:
		return meson_div_set_rate(clk, id, rate, current_rate);
	case CLKID_HDMI:
		return meson_clk_set_rate_by_id(clk, CLKID_HDMI_DIV,
						rate, current_rate);
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

	priv->map = syscon_node_to_regmap(dev_ofnode(dev_get_parent(dev)));
	if (IS_ERR(priv->map))
		return PTR_ERR(priv->map);

	/*
	 * Depending on the boot src, the state of the MMC clock might
	 * be different. Reset it to make sure we won't get stuck
	 */
	regmap_write(priv->map, HHI_NAND_CLK_CNTL, 0);
	regmap_write(priv->map, HHI_SD_EMMC_CLK_CNTL, 0);

	debug("meson-clk-g12a: probed\n");

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
	{ .compatible = "amlogic,g12a-clkc" },
	{ .compatible = "amlogic,g12b-clkc" },
	{ .compatible = "amlogic,sm1-clkc" },
	{ }
};

U_BOOT_DRIVER(meson_clk_g12a) = {
	.name		= "meson_clk_g12a",
	.id		= UCLASS_CLK,
	.of_match	= meson_clk_ids,
	.priv_auto	= sizeof(struct meson_clk),
	.ops		= &meson_clk_ops,
	.probe		= meson_clk_probe,
};
