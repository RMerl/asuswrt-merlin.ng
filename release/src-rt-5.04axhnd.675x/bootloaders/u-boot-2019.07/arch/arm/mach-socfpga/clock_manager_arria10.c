// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2016-2017 Intel Corporation
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <dm.h>
#include <clk.h>
#include <dm/device-internal.h>
#include <asm/arch/clock_manager.h>

#ifdef CONFIG_SPL_BUILD

static u32 eosc1_hz;
static u32 cb_intosc_hz;
static u32 f2s_free_hz;

struct mainpll_cfg {
	u32 vco0_psrc;
	u32 vco1_denom;
	u32 vco1_numer;
	u32 mpuclk;
	u32 mpuclk_cnt;
	u32 mpuclk_src;
	u32 nocclk;
	u32 nocclk_cnt;
	u32 nocclk_src;
	u32 cntr2clk_cnt;
	u32 cntr3clk_cnt;
	u32 cntr4clk_cnt;
	u32 cntr5clk_cnt;
	u32 cntr6clk_cnt;
	u32 cntr7clk_cnt;
	u32 cntr7clk_src;
	u32 cntr8clk_cnt;
	u32 cntr9clk_cnt;
	u32 cntr9clk_src;
	u32 cntr15clk_cnt;
	u32 nocdiv_l4mainclk;
	u32 nocdiv_l4mpclk;
	u32 nocdiv_l4spclk;
	u32 nocdiv_csatclk;
	u32 nocdiv_cstraceclk;
	u32 nocdiv_cspdbclk;
};

struct perpll_cfg {
	u32 vco0_psrc;
	u32 vco1_denom;
	u32 vco1_numer;
	u32 cntr2clk_cnt;
	u32 cntr2clk_src;
	u32 cntr3clk_cnt;
	u32 cntr3clk_src;
	u32 cntr4clk_cnt;
	u32 cntr4clk_src;
	u32 cntr5clk_cnt;
	u32 cntr5clk_src;
	u32 cntr6clk_cnt;
	u32 cntr6clk_src;
	u32 cntr7clk_cnt;
	u32 cntr8clk_cnt;
	u32 cntr8clk_src;
	u32 cntr9clk_cnt;
	u32 cntr9clk_src;
	u32 emacctl_emac0sel;
	u32 emacctl_emac1sel;
	u32 emacctl_emac2sel;
	u32 gpiodiv_gpiodbclk;
};

struct strtou32 {
	const char *str;
	const u32 val;
};

static const struct strtou32 mainpll_cfg_tab[] = {
	{ "vco0-psrc", offsetof(struct mainpll_cfg, vco0_psrc) },
	{ "vco1-denom", offsetof(struct mainpll_cfg, vco1_denom) },
	{ "vco1-numer", offsetof(struct mainpll_cfg, vco1_numer) },
	{ "mpuclk-cnt", offsetof(struct mainpll_cfg, mpuclk_cnt) },
	{ "mpuclk-src", offsetof(struct mainpll_cfg, mpuclk_src) },
	{ "nocclk-cnt", offsetof(struct mainpll_cfg, nocclk_cnt) },
	{ "nocclk-src", offsetof(struct mainpll_cfg, nocclk_src) },
	{ "cntr2clk-cnt", offsetof(struct mainpll_cfg, cntr2clk_cnt) },
	{ "cntr3clk-cnt", offsetof(struct mainpll_cfg, cntr3clk_cnt) },
	{ "cntr4clk-cnt", offsetof(struct mainpll_cfg, cntr4clk_cnt) },
	{ "cntr5clk-cnt", offsetof(struct mainpll_cfg, cntr5clk_cnt) },
	{ "cntr6clk-cnt", offsetof(struct mainpll_cfg, cntr6clk_cnt) },
	{ "cntr7clk-cnt", offsetof(struct mainpll_cfg, cntr7clk_cnt) },
	{ "cntr7clk-src", offsetof(struct mainpll_cfg, cntr7clk_src) },
	{ "cntr8clk-cnt", offsetof(struct mainpll_cfg, cntr8clk_cnt) },
	{ "cntr9clk-cnt", offsetof(struct mainpll_cfg, cntr9clk_cnt) },
	{ "cntr9clk-src", offsetof(struct mainpll_cfg, cntr9clk_src) },
	{ "cntr15clk-cnt", offsetof(struct mainpll_cfg, cntr15clk_cnt) },
	{ "nocdiv-l4mainclk", offsetof(struct mainpll_cfg, nocdiv_l4mainclk) },
	{ "nocdiv-l4mpclk", offsetof(struct mainpll_cfg, nocdiv_l4mpclk) },
	{ "nocdiv-l4spclk", offsetof(struct mainpll_cfg, nocdiv_l4spclk) },
	{ "nocdiv-csatclk", offsetof(struct mainpll_cfg, nocdiv_csatclk) },
	{ "nocdiv-cstraceclk", offsetof(struct mainpll_cfg, nocdiv_cstraceclk) },
	{ "nocdiv-cspdbgclk", offsetof(struct mainpll_cfg, nocdiv_cspdbclk) },
};

static const struct strtou32 perpll_cfg_tab[] = {
	{ "vco0-psrc", offsetof(struct perpll_cfg, vco0_psrc) },
	{ "vco1-denom", offsetof(struct perpll_cfg, vco1_denom) },
	{ "vco1-numer", offsetof(struct perpll_cfg, vco1_numer) },
	{ "cntr2clk-cnt", offsetof(struct perpll_cfg, cntr2clk_cnt) },
	{ "cntr2clk-src", offsetof(struct perpll_cfg, cntr2clk_src) },
	{ "cntr3clk-cnt", offsetof(struct perpll_cfg, cntr3clk_cnt) },
	{ "cntr3clk-src", offsetof(struct perpll_cfg, cntr3clk_src) },
	{ "cntr4clk-cnt", offsetof(struct perpll_cfg, cntr4clk_cnt) },
	{ "cntr4clk-src", offsetof(struct perpll_cfg, cntr4clk_src) },
	{ "cntr5clk-cnt", offsetof(struct perpll_cfg, cntr5clk_cnt) },
	{ "cntr5clk-src", offsetof(struct perpll_cfg, cntr5clk_src) },
	{ "cntr6clk-cnt", offsetof(struct perpll_cfg, cntr6clk_cnt) },
	{ "cntr6clk-src", offsetof(struct perpll_cfg, cntr6clk_src) },
	{ "cntr7clk-cnt", offsetof(struct perpll_cfg, cntr7clk_cnt) },
	{ "cntr8clk-cnt", offsetof(struct perpll_cfg, cntr8clk_cnt) },
	{ "cntr8clk-src", offsetof(struct perpll_cfg, cntr8clk_src) },
	{ "cntr9clk-cnt", offsetof(struct perpll_cfg, cntr9clk_cnt) },
	{ "emacctl-emac0sel", offsetof(struct perpll_cfg, emacctl_emac0sel) },
	{ "emacctl-emac1sel", offsetof(struct perpll_cfg, emacctl_emac1sel) },
	{ "emacctl-emac2sel", offsetof(struct perpll_cfg, emacctl_emac2sel) },
	{ "gpiodiv-gpiodbclk", offsetof(struct perpll_cfg, gpiodiv_gpiodbclk) },
};

static const struct strtou32 alteragrp_cfg_tab[] = {
	{ "nocclk", offsetof(struct mainpll_cfg, nocclk) },
	{ "mpuclk", offsetof(struct mainpll_cfg, mpuclk) },
};

struct strtopu32 {
	const char *str;
	u32 *p;
};

const struct strtopu32 dt_to_val[] = {
	{ "altera_arria10_hps_eosc1", &eosc1_hz },
	{ "altera_arria10_hps_cb_intosc_ls", &cb_intosc_hz },
	{ "altera_arria10_hps_f2h_free", &f2s_free_hz },
};

static int of_to_struct(const void *blob, int node, const struct strtou32 *cfg_tab,
			int cfg_tab_len, void *cfg)
{
	int i;
	u32 val;

	for (i = 0; i < cfg_tab_len; i++) {
		if (fdtdec_get_int_array(blob, node, cfg_tab[i].str, &val, 1)) {
			/* could not find required property */
			return -EINVAL;
		}
		*(u32 *)(cfg + cfg_tab[i].val) = val;
	}

	return 0;
}

static int of_get_input_clks(const void *blob)
{
	struct udevice *dev;
	struct clk clk;
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(dt_to_val); i++) {
		memset(&clk, 0, sizeof(clk));

		ret = uclass_get_device_by_name(UCLASS_CLK, dt_to_val[i].str,
						&dev);
		if (ret)
			return ret;

		ret = clk_request(dev, &clk);
		if (ret)
			return ret;

		*dt_to_val[i].p = clk_get_rate(&clk);
	}

	return 0;
}

static int of_get_clk_cfg(const void *blob, struct mainpll_cfg *main_cfg,
			  struct perpll_cfg *per_cfg)
{
	int ret, node, child, len;
	const char *node_name;

	ret = of_get_input_clks(blob);
	if (ret)
		return ret;

	node = fdtdec_next_compatible(blob, 0, COMPAT_ALTERA_SOCFPGA_CLK_INIT);

	if (node < 0)
		return -EINVAL;

	child = fdt_first_subnode(blob, node);

	if (child < 0)
		return -EINVAL;

	node_name = fdt_get_name(blob, child, &len);

	while (node_name) {
		if (!strcmp(node_name, "mainpll")) {
			if (of_to_struct(blob, child, mainpll_cfg_tab,
					 ARRAY_SIZE(mainpll_cfg_tab), main_cfg))
				return -EINVAL;
		} else if (!strcmp(node_name, "perpll")) {
			if (of_to_struct(blob, child, perpll_cfg_tab,
					 ARRAY_SIZE(perpll_cfg_tab), per_cfg))
				return -EINVAL;
		} else if (!strcmp(node_name, "alteragrp")) {
			if (of_to_struct(blob, child, alteragrp_cfg_tab,
					 ARRAY_SIZE(alteragrp_cfg_tab), main_cfg))
				return -EINVAL;
		}
		child = fdt_next_subnode(blob, child);

		if (child < 0)
			break;

		node_name = fdt_get_name(blob, child, &len);
	}

	return 0;
}

static const struct socfpga_clock_manager *clock_manager_base =
	(struct socfpga_clock_manager *)SOCFPGA_CLKMGR_ADDRESS;

/* calculate the intended main VCO frequency based on handoff */
static unsigned int cm_calc_handoff_main_vco_clk_hz
					(struct mainpll_cfg *main_cfg)
{
	unsigned int clk_hz;

	/* Check main VCO clock source: eosc, intosc or f2s? */
	switch (main_cfg->vco0_psrc) {
	case CLKMGR_MAINPLL_VCO0_PSRC_EOSC:
		clk_hz = eosc1_hz;
		break;
	case CLKMGR_MAINPLL_VCO0_PSRC_E_INTOSC:
		clk_hz = cb_intosc_hz;
		break;
	case CLKMGR_MAINPLL_VCO0_PSRC_F2S:
		clk_hz = f2s_free_hz;
		break;
	default:
		return 0;
	}

	/* calculate the VCO frequency */
	clk_hz /= 1 + main_cfg->vco1_denom;
	clk_hz *= 1 + main_cfg->vco1_numer;

	return clk_hz;
}

/* calculate the intended periph VCO frequency based on handoff */
static unsigned int cm_calc_handoff_periph_vco_clk_hz(
		struct mainpll_cfg *main_cfg, struct perpll_cfg *per_cfg)
{
	unsigned int clk_hz;

	/* Check periph VCO clock source: eosc, intosc, f2s or mainpll? */
	switch (per_cfg->vco0_psrc) {
	case CLKMGR_PERPLL_VCO0_PSRC_EOSC:
		clk_hz = eosc1_hz;
		break;
	case CLKMGR_PERPLL_VCO0_PSRC_E_INTOSC:
		clk_hz = cb_intosc_hz;
		break;
	case CLKMGR_PERPLL_VCO0_PSRC_F2S:
		clk_hz = f2s_free_hz;
		break;
	case CLKMGR_PERPLL_VCO0_PSRC_MAIN:
		clk_hz = cm_calc_handoff_main_vco_clk_hz(main_cfg);
		clk_hz /= main_cfg->cntr15clk_cnt;
		break;
	default:
		return 0;
	}

	/* calculate the VCO frequency */
	clk_hz /= 1 + per_cfg->vco1_denom;
	clk_hz *= 1 + per_cfg->vco1_numer;

	return clk_hz;
}

/* calculate the intended MPU clock frequency based on handoff */
static unsigned int cm_calc_handoff_mpu_clk_hz(struct mainpll_cfg *main_cfg,
					       struct perpll_cfg *per_cfg)
{
	unsigned int clk_hz;

	/* Check MPU clock source: main, periph, osc1, intosc or f2s? */
	switch (main_cfg->mpuclk_src) {
	case CLKMGR_MAINPLL_MPUCLK_SRC_MAIN:
		clk_hz = cm_calc_handoff_main_vco_clk_hz(main_cfg);
		clk_hz /= (main_cfg->mpuclk & CLKMGR_MAINPLL_MPUCLK_CNT_MSK)
			   + 1;
		break;
	case CLKMGR_MAINPLL_MPUCLK_SRC_PERI:
		clk_hz = cm_calc_handoff_periph_vco_clk_hz(main_cfg, per_cfg);
		clk_hz /= ((main_cfg->mpuclk >>
			   CLKMGR_MAINPLL_MPUCLK_PERICNT_LSB) &
			   CLKMGR_MAINPLL_MPUCLK_CNT_MSK) + 1;
		break;
	case CLKMGR_MAINPLL_MPUCLK_SRC_OSC1:
		clk_hz = eosc1_hz;
		break;
	case CLKMGR_MAINPLL_MPUCLK_SRC_INTOSC:
		clk_hz = cb_intosc_hz;
		break;
	case CLKMGR_MAINPLL_MPUCLK_SRC_FPGA:
		clk_hz = f2s_free_hz;
		break;
	default:
		return 0;
	}

	clk_hz /= main_cfg->mpuclk_cnt + 1;
	return clk_hz;
}

/* calculate the intended NOC clock frequency based on handoff */
static unsigned int cm_calc_handoff_noc_clk_hz(struct mainpll_cfg *main_cfg,
					       struct perpll_cfg *per_cfg)
{
	unsigned int clk_hz;

	/* Check MPU clock source: main, periph, osc1, intosc or f2s? */
	switch (main_cfg->nocclk_src) {
	case CLKMGR_MAINPLL_NOCCLK_SRC_MAIN:
		clk_hz = cm_calc_handoff_main_vco_clk_hz(main_cfg);
		clk_hz /= (main_cfg->nocclk & CLKMGR_MAINPLL_NOCCLK_CNT_MSK)
			 + 1;
		break;
	case CLKMGR_MAINPLL_NOCCLK_SRC_PERI:
		clk_hz = cm_calc_handoff_periph_vco_clk_hz(main_cfg, per_cfg);
		clk_hz /= ((main_cfg->nocclk >>
			   CLKMGR_MAINPLL_NOCCLK_PERICNT_LSB) &
			   CLKMGR_MAINPLL_NOCCLK_CNT_MSK) + 1;
		break;
	case CLKMGR_MAINPLL_NOCCLK_SRC_OSC1:
		clk_hz = eosc1_hz;
		break;
	case CLKMGR_MAINPLL_NOCCLK_SRC_INTOSC:
		clk_hz = cb_intosc_hz;
		break;
	case CLKMGR_MAINPLL_NOCCLK_SRC_FPGA:
		clk_hz = f2s_free_hz;
		break;
	default:
		return 0;
	}

	clk_hz /= main_cfg->nocclk_cnt + 1;
	return clk_hz;
}

/* return 1 if PLL ramp is required */
static int cm_is_pll_ramp_required(int main0periph1,
				   struct mainpll_cfg *main_cfg,
				   struct perpll_cfg *per_cfg)
{
	/* Check for main PLL */
	if (main0periph1 == 0) {
		/*
		 * PLL ramp is not required if both MPU clock and NOC clock are
		 * not sourced from main PLL
		 */
		if (main_cfg->mpuclk_src != CLKMGR_MAINPLL_MPUCLK_SRC_MAIN &&
		    main_cfg->nocclk_src != CLKMGR_MAINPLL_NOCCLK_SRC_MAIN)
			return 0;

		/*
		 * PLL ramp is required if MPU clock is sourced from main PLL
		 * and MPU clock is over 900MHz (as advised by HW team)
		 */
		if (main_cfg->mpuclk_src == CLKMGR_MAINPLL_MPUCLK_SRC_MAIN &&
		    (cm_calc_handoff_mpu_clk_hz(main_cfg, per_cfg) >
		     CLKMGR_PLL_RAMP_MPUCLK_THRESHOLD_HZ))
			return 1;

		/*
		 * PLL ramp is required if NOC clock is sourced from main PLL
		 * and NOC clock is over 300MHz (as advised by HW team)
		 */
		if (main_cfg->nocclk_src == CLKMGR_MAINPLL_NOCCLK_SRC_MAIN &&
		    (cm_calc_handoff_noc_clk_hz(main_cfg, per_cfg) >
		     CLKMGR_PLL_RAMP_NOCCLK_THRESHOLD_HZ))
			return 2;

	} else if (main0periph1 == 1) {
		/*
		 * PLL ramp is not required if both MPU clock and NOC clock are
		 * not sourced from periph PLL
		 */
		if (main_cfg->mpuclk_src != CLKMGR_MAINPLL_MPUCLK_SRC_PERI &&
		    main_cfg->nocclk_src != CLKMGR_MAINPLL_NOCCLK_SRC_PERI)
			return 0;

		/*
		 * PLL ramp is required if MPU clock are source from periph PLL
		 * and MPU clock is over 900MHz (as advised by HW team)
		 */
		if (main_cfg->mpuclk_src == CLKMGR_MAINPLL_MPUCLK_SRC_PERI &&
		    (cm_calc_handoff_mpu_clk_hz(main_cfg, per_cfg) >
		     CLKMGR_PLL_RAMP_MPUCLK_THRESHOLD_HZ))
			return 1;

		/*
		 * PLL ramp is required if NOC clock are source from periph PLL
		 * and NOC clock is over 300MHz (as advised by HW team)
		 */
		if (main_cfg->nocclk_src == CLKMGR_MAINPLL_NOCCLK_SRC_PERI &&
		    (cm_calc_handoff_noc_clk_hz(main_cfg, per_cfg) >
		     CLKMGR_PLL_RAMP_NOCCLK_THRESHOLD_HZ))
			return 2;
	}

	return 0;
}

static u32 cm_calculate_numer(struct mainpll_cfg *main_cfg,
			      struct perpll_cfg *per_cfg,
			      u32 safe_hz, u32 clk_hz)
{
	u32 cnt;
	u32 clk;
	u32 shift;
	u32 mask;
	u32 denom;

	if (main_cfg->mpuclk_src == CLKMGR_MAINPLL_MPUCLK_SRC_MAIN) {
		cnt = main_cfg->mpuclk_cnt;
		clk = main_cfg->mpuclk;
		shift = 0;
		mask = CLKMGR_MAINPLL_MPUCLK_CNT_MSK;
		denom = main_cfg->vco1_denom;
	} else if (main_cfg->nocclk_src == CLKMGR_MAINPLL_NOCCLK_SRC_MAIN) {
		cnt = main_cfg->nocclk_cnt;
		clk = main_cfg->nocclk;
		shift = 0;
		mask = CLKMGR_MAINPLL_NOCCLK_CNT_MSK;
		denom = main_cfg->vco1_denom;
	} else if (main_cfg->mpuclk_src == CLKMGR_MAINPLL_MPUCLK_SRC_PERI) {
		cnt = main_cfg->mpuclk_cnt;
		clk = main_cfg->mpuclk;
		shift = CLKMGR_MAINPLL_MPUCLK_PERICNT_LSB;
		mask = CLKMGR_MAINPLL_MPUCLK_CNT_MSK;
		denom = per_cfg->vco1_denom;
	} else if (main_cfg->nocclk_src == CLKMGR_MAINPLL_NOCCLK_SRC_PERI) {
		cnt = main_cfg->nocclk_cnt;
		clk = main_cfg->nocclk;
		shift = CLKMGR_MAINPLL_NOCCLK_PERICNT_LSB;
		mask = CLKMGR_MAINPLL_NOCCLK_CNT_MSK;
		denom = per_cfg->vco1_denom;
	} else {
		return 0;
	}

	return (safe_hz / clk_hz) * (cnt + 1) * (((clk >> shift) & mask) + 1) *
		(1 + denom) - 1;
}

/*
 * Calculate the new PLL numerator which is based on existing DTS hand off and
 * intended safe frequency (safe_hz). Note that PLL ramp is only modifying the
 * numerator while maintaining denominator as denominator will influence the
 * jitter condition. Please refer A10 HPS TRM for the jitter guide. Note final
 * value for numerator is minus with 1 to cater our register value
 * representation.
 */
static unsigned int cm_calc_safe_pll_numer(int main0periph1,
					   struct mainpll_cfg *main_cfg,
					   struct perpll_cfg *per_cfg,
					   unsigned int safe_hz)
{
	unsigned int clk_hz = 0;

	/* Check for main PLL */
	if (main0periph1 == 0) {
		/* Check main VCO clock source: eosc, intosc or f2s? */
		switch (main_cfg->vco0_psrc) {
		case CLKMGR_MAINPLL_VCO0_PSRC_EOSC:
			clk_hz = eosc1_hz;
			break;
		case CLKMGR_MAINPLL_VCO0_PSRC_E_INTOSC:
			clk_hz = cb_intosc_hz;
			break;
		case CLKMGR_MAINPLL_VCO0_PSRC_F2S:
			clk_hz = f2s_free_hz;
			break;
		default:
			return 0;
		}
	} else if (main0periph1 == 1) {
		/* Check periph VCO clock source: eosc, intosc, f2s, mainpll */
		switch (per_cfg->vco0_psrc) {
		case CLKMGR_PERPLL_VCO0_PSRC_EOSC:
			clk_hz = eosc1_hz;
			break;
		case CLKMGR_PERPLL_VCO0_PSRC_E_INTOSC:
			clk_hz = cb_intosc_hz;
			break;
		case CLKMGR_PERPLL_VCO0_PSRC_F2S:
			clk_hz = f2s_free_hz;
			break;
		case CLKMGR_PERPLL_VCO0_PSRC_MAIN:
			clk_hz = cm_calc_handoff_main_vco_clk_hz(main_cfg);
			clk_hz /= main_cfg->cntr15clk_cnt;
			break;
		default:
			return 0;
		}
	} else {
		return 0;
	}

	return cm_calculate_numer(main_cfg, per_cfg, safe_hz, clk_hz);
}

/* ramping the main PLL to final value */
static void cm_pll_ramp_main(struct mainpll_cfg *main_cfg,
			     struct perpll_cfg *per_cfg,
			     unsigned int pll_ramp_main_hz)
{
	unsigned int clk_hz = 0, clk_incr_hz = 0, clk_final_hz = 0;

	/* find out the increment value */
	if (main_cfg->mpuclk_src == CLKMGR_MAINPLL_MPUCLK_SRC_MAIN) {
		clk_incr_hz = CLKMGR_PLL_RAMP_MPUCLK_INCREMENT_HZ;
		clk_final_hz = cm_calc_handoff_mpu_clk_hz(main_cfg, per_cfg);
	} else if (main_cfg->nocclk_src == CLKMGR_MAINPLL_NOCCLK_SRC_MAIN) {
		clk_incr_hz = CLKMGR_PLL_RAMP_NOCCLK_INCREMENT_HZ;
		clk_final_hz = cm_calc_handoff_noc_clk_hz(main_cfg, per_cfg);
	}

	/* execute the ramping here */
	for (clk_hz = pll_ramp_main_hz + clk_incr_hz;
	     clk_hz < clk_final_hz; clk_hz += clk_incr_hz) {
		writel((main_cfg->vco1_denom <<
			CLKMGR_MAINPLL_VCO1_DENOM_LSB) |
			cm_calc_safe_pll_numer(0, main_cfg, per_cfg, clk_hz),
			&clock_manager_base->main_pll.vco1);
		mdelay(1);
		cm_wait_for_lock(LOCKED_MASK);
	}
	writel((main_cfg->vco1_denom << CLKMGR_MAINPLL_VCO1_DENOM_LSB) |
		main_cfg->vco1_numer, &clock_manager_base->main_pll.vco1);
	mdelay(1);
	cm_wait_for_lock(LOCKED_MASK);
}

/* ramping the periph PLL to final value */
static void cm_pll_ramp_periph(struct mainpll_cfg *main_cfg,
			       struct perpll_cfg *per_cfg,
			       unsigned int pll_ramp_periph_hz)
{
	unsigned int clk_hz = 0, clk_incr_hz = 0, clk_final_hz = 0;

	/* find out the increment value */
	if (main_cfg->mpuclk_src == CLKMGR_MAINPLL_MPUCLK_SRC_PERI) {
		clk_incr_hz = CLKMGR_PLL_RAMP_MPUCLK_INCREMENT_HZ;
		clk_final_hz = cm_calc_handoff_mpu_clk_hz(main_cfg, per_cfg);
	} else if (main_cfg->nocclk_src == CLKMGR_MAINPLL_NOCCLK_SRC_PERI) {
		clk_incr_hz = CLKMGR_PLL_RAMP_NOCCLK_INCREMENT_HZ;
		clk_final_hz = cm_calc_handoff_noc_clk_hz(main_cfg, per_cfg);
	}
	/* execute the ramping here */
	for (clk_hz = pll_ramp_periph_hz + clk_incr_hz;
	     clk_hz < clk_final_hz; clk_hz += clk_incr_hz) {
		writel((per_cfg->vco1_denom << CLKMGR_PERPLL_VCO1_DENOM_LSB) |
			cm_calc_safe_pll_numer(1, main_cfg, per_cfg, clk_hz),
			&clock_manager_base->per_pll.vco1);
		mdelay(1);
		cm_wait_for_lock(LOCKED_MASK);
	}
	writel((per_cfg->vco1_denom << CLKMGR_PERPLL_VCO1_DENOM_LSB) |
		per_cfg->vco1_numer, &clock_manager_base->per_pll.vco1);
	mdelay(1);
	cm_wait_for_lock(LOCKED_MASK);
}

/*
 * Setup clocks while making no assumptions of the
 * previous state of the clocks.
 *
 * Start by being paranoid and gate all sw managed clocks
 *
 * Put all plls in bypass
 *
 * Put all plls VCO registers back to reset value (bgpwr dwn).
 *
 * Put peripheral and main pll src to reset value to avoid glitch.
 *
 * Delay 5 us.
 *
 * Deassert bg pwr dn and set numerator and denominator
 *
 * Start 7 us timer.
 *
 * set internal dividers
 *
 * Wait for 7 us timer.
 *
 * Enable plls
 *
 * Set external dividers while plls are locking
 *
 * Wait for pll lock
 *
 * Assert/deassert outreset all.
 *
 * Take all pll's out of bypass
 *
 * Clear safe mode
 *
 * set source main and peripheral clocks
 *
 * Ungate clocks
 */

static int cm_full_cfg(struct mainpll_cfg *main_cfg, struct perpll_cfg *per_cfg)
{
	unsigned int pll_ramp_main_hz = 0, pll_ramp_periph_hz = 0,
		ramp_required;

	/* gate off all mainpll clock excpet HW managed clock */
	writel(CLKMGR_MAINPLL_EN_S2FUSER0CLKEN_SET_MSK |
		CLKMGR_MAINPLL_EN_HMCPLLREFCLKEN_SET_MSK,
		&clock_manager_base->main_pll.enr);

	/* now we can gate off the rest of the peripheral clocks */
	writel(0, &clock_manager_base->per_pll.en);

	/* Put all plls in external bypass */
	writel(CLKMGR_MAINPLL_BYPASS_RESET,
	       &clock_manager_base->main_pll.bypasss);
	writel(CLKMGR_PERPLL_BYPASS_RESET,
	       &clock_manager_base->per_pll.bypasss);

	/*
	 * Put all plls VCO registers back to reset value.
	 * Some code might have messed with them. At same time set the
	 * desired clock source
	 */
	writel(CLKMGR_MAINPLL_VCO0_RESET |
	       CLKMGR_MAINPLL_VCO0_REGEXTSEL_SET_MSK |
	       (main_cfg->vco0_psrc << CLKMGR_MAINPLL_VCO0_PSRC_LSB),
	       &clock_manager_base->main_pll.vco0);

	writel(CLKMGR_PERPLL_VCO0_RESET |
	       CLKMGR_PERPLL_VCO0_REGEXTSEL_SET_MSK |
	       (per_cfg->vco0_psrc << CLKMGR_PERPLL_VCO0_PSRC_LSB),
	       &clock_manager_base->per_pll.vco0);

	writel(CLKMGR_MAINPLL_VCO1_RESET, &clock_manager_base->main_pll.vco1);
	writel(CLKMGR_PERPLL_VCO1_RESET, &clock_manager_base->per_pll.vco1);

	/* clear the interrupt register status register */
	writel(CLKMGR_CLKMGR_INTR_MAINPLLLOST_SET_MSK |
		CLKMGR_CLKMGR_INTR_PERPLLLOST_SET_MSK |
		CLKMGR_CLKMGR_INTR_MAINPLLRFSLIP_SET_MSK |
		CLKMGR_CLKMGR_INTR_PERPLLRFSLIP_SET_MSK |
		CLKMGR_CLKMGR_INTR_MAINPLLFBSLIP_SET_MSK |
		CLKMGR_CLKMGR_INTR_PERPLLFBSLIP_SET_MSK |
		CLKMGR_CLKMGR_INTR_MAINPLLACHIEVED_SET_MSK |
		CLKMGR_CLKMGR_INTR_PERPLLACHIEVED_SET_MSK,
		&clock_manager_base->intr);

	/* Program VCO Numerator and Denominator for main PLL */
	ramp_required = cm_is_pll_ramp_required(0, main_cfg, per_cfg);
	if (ramp_required) {
		/* set main PLL to safe starting threshold frequency */
		if (ramp_required == 1)
			pll_ramp_main_hz = CLKMGR_PLL_RAMP_MPUCLK_THRESHOLD_HZ;
		else if (ramp_required == 2)
			pll_ramp_main_hz = CLKMGR_PLL_RAMP_NOCCLK_THRESHOLD_HZ;

		writel((main_cfg->vco1_denom << CLKMGR_MAINPLL_VCO1_DENOM_LSB) |
			cm_calc_safe_pll_numer(0, main_cfg, per_cfg,
					       pll_ramp_main_hz),
			&clock_manager_base->main_pll.vco1);
	} else
		writel((main_cfg->vco1_denom << CLKMGR_MAINPLL_VCO1_DENOM_LSB) |
			main_cfg->vco1_numer,
			&clock_manager_base->main_pll.vco1);

	/* Program VCO Numerator and Denominator for periph PLL */
	ramp_required = cm_is_pll_ramp_required(1, main_cfg, per_cfg);
	if (ramp_required) {
		/* set periph PLL to safe starting threshold frequency */
		if (ramp_required == 1)
			pll_ramp_periph_hz =
				CLKMGR_PLL_RAMP_MPUCLK_THRESHOLD_HZ;
		else if (ramp_required == 2)
			pll_ramp_periph_hz =
				CLKMGR_PLL_RAMP_NOCCLK_THRESHOLD_HZ;

		writel((per_cfg->vco1_denom << CLKMGR_PERPLL_VCO1_DENOM_LSB) |
			cm_calc_safe_pll_numer(1, main_cfg, per_cfg,
					       pll_ramp_periph_hz),
			&clock_manager_base->per_pll.vco1);
	} else
		writel((per_cfg->vco1_denom << CLKMGR_PERPLL_VCO1_DENOM_LSB) |
			per_cfg->vco1_numer,
			&clock_manager_base->per_pll.vco1);

	/* Wait for at least 5 us */
	udelay(5);

	/* Now deassert BGPWRDN and PWRDN */
	clrbits_le32(&clock_manager_base->main_pll.vco0,
		     CLKMGR_MAINPLL_VCO0_BGPWRDN_SET_MSK |
		     CLKMGR_MAINPLL_VCO0_PWRDN_SET_MSK);
	clrbits_le32(&clock_manager_base->per_pll.vco0,
		     CLKMGR_PERPLL_VCO0_BGPWRDN_SET_MSK |
		     CLKMGR_PERPLL_VCO0_PWRDN_SET_MSK);

	/* Wait for at least 7 us */
	udelay(7);

	/* enable the VCO and disable the external regulator to PLL */
	writel((readl(&clock_manager_base->main_pll.vco0) &
		~CLKMGR_MAINPLL_VCO0_REGEXTSEL_SET_MSK) |
		CLKMGR_MAINPLL_VCO0_EN_SET_MSK,
		&clock_manager_base->main_pll.vco0);
	writel((readl(&clock_manager_base->per_pll.vco0) &
		~CLKMGR_PERPLL_VCO0_REGEXTSEL_SET_MSK) |
		CLKMGR_PERPLL_VCO0_EN_SET_MSK,
		&clock_manager_base->per_pll.vco0);

	/* setup all the main PLL counter and clock source */
	writel(main_cfg->nocclk,
	       SOCFPGA_CLKMGR_ADDRESS + CLKMGR_MAINPLL_NOC_CLK_OFFSET);
	writel(main_cfg->mpuclk,
	       SOCFPGA_CLKMGR_ADDRESS + CLKMGR_ALTERAGRP_MPU_CLK_OFFSET);

	/* main_emaca_clk divider */
	writel(main_cfg->cntr2clk_cnt, &clock_manager_base->main_pll.cntr2clk);
	/* main_emacb_clk divider */
	writel(main_cfg->cntr3clk_cnt, &clock_manager_base->main_pll.cntr3clk);
	/* main_emac_ptp_clk divider */
	writel(main_cfg->cntr4clk_cnt, &clock_manager_base->main_pll.cntr4clk);
	/* main_gpio_db_clk divider */
	writel(main_cfg->cntr5clk_cnt, &clock_manager_base->main_pll.cntr5clk);
	/* main_sdmmc_clk divider */
	writel(main_cfg->cntr6clk_cnt, &clock_manager_base->main_pll.cntr6clk);
	/* main_s2f_user0_clk divider */
	writel(main_cfg->cntr7clk_cnt |
	       (main_cfg->cntr7clk_src << CLKMGR_MAINPLL_CNTR7CLK_SRC_LSB),
	       &clock_manager_base->main_pll.cntr7clk);
	/* main_s2f_user1_clk divider */
	writel(main_cfg->cntr8clk_cnt, &clock_manager_base->main_pll.cntr8clk);
	/* main_hmc_pll_clk divider */
	writel(main_cfg->cntr9clk_cnt |
	       (main_cfg->cntr9clk_src << CLKMGR_MAINPLL_CNTR9CLK_SRC_LSB),
	       &clock_manager_base->main_pll.cntr9clk);
	/* main_periph_ref_clk divider */
	writel(main_cfg->cntr15clk_cnt,
	       &clock_manager_base->main_pll.cntr15clk);

	/* setup all the peripheral PLL counter and clock source */
	/* peri_emaca_clk divider */
	writel(per_cfg->cntr2clk_cnt |
	       (per_cfg->cntr2clk_src << CLKMGR_PERPLL_CNTR2CLK_SRC_LSB),
	       &clock_manager_base->per_pll.cntr2clk);
	/* peri_emacb_clk divider */
	writel(per_cfg->cntr3clk_cnt |
	       (per_cfg->cntr3clk_src << CLKMGR_PERPLL_CNTR3CLK_SRC_LSB),
	       &clock_manager_base->per_pll.cntr3clk);
	/* peri_emac_ptp_clk divider */
	writel(per_cfg->cntr4clk_cnt |
	       (per_cfg->cntr4clk_src << CLKMGR_PERPLL_CNTR4CLK_SRC_LSB),
	       &clock_manager_base->per_pll.cntr4clk);
	/* peri_gpio_db_clk divider */
	writel(per_cfg->cntr5clk_cnt |
	       (per_cfg->cntr5clk_src << CLKMGR_PERPLL_CNTR5CLK_SRC_LSB),
	       &clock_manager_base->per_pll.cntr5clk);
	/* peri_sdmmc_clk divider */
	writel(per_cfg->cntr6clk_cnt |
	       (per_cfg->cntr6clk_src << CLKMGR_PERPLL_CNTR6CLK_SRC_LSB),
	       &clock_manager_base->per_pll.cntr6clk);
	/* peri_s2f_user0_clk divider */
	writel(per_cfg->cntr7clk_cnt, &clock_manager_base->per_pll.cntr7clk);
	/* peri_s2f_user1_clk divider */
	writel(per_cfg->cntr8clk_cnt |
	       (per_cfg->cntr8clk_src << CLKMGR_PERPLL_CNTR8CLK_SRC_LSB),
	       &clock_manager_base->per_pll.cntr8clk);
	/* peri_hmc_pll_clk divider */
	writel(per_cfg->cntr9clk_cnt, &clock_manager_base->per_pll.cntr9clk);

	/* setup all the external PLL counter */
	/* mpu wrapper / external divider */
	writel(main_cfg->mpuclk_cnt |
	       (main_cfg->mpuclk_src << CLKMGR_MAINPLL_MPUCLK_SRC_LSB),
	       &clock_manager_base->main_pll.mpuclk);
	/* NOC wrapper / external divider */
	writel(main_cfg->nocclk_cnt |
	       (main_cfg->nocclk_src << CLKMGR_MAINPLL_NOCCLK_SRC_LSB),
	       &clock_manager_base->main_pll.nocclk);
	/* NOC subclock divider such as l4 */
	writel(main_cfg->nocdiv_l4mainclk |
	       (main_cfg->nocdiv_l4mpclk <<
		CLKMGR_MAINPLL_NOCDIV_L4MPCLK_LSB) |
	       (main_cfg->nocdiv_l4spclk <<
		CLKMGR_MAINPLL_NOCDIV_L4SPCLK_LSB) |
	       (main_cfg->nocdiv_csatclk <<
		CLKMGR_MAINPLL_NOCDIV_CSATCLK_LSB) |
	       (main_cfg->nocdiv_cstraceclk <<
		CLKMGR_MAINPLL_NOCDIV_CSTRACECLK_LSB) |
	       (main_cfg->nocdiv_cspdbclk <<
		CLKMGR_MAINPLL_NOCDIV_CSPDBGCLK_LSB),
		&clock_manager_base->main_pll.nocdiv);
	/* gpio_db external divider */
	writel(per_cfg->gpiodiv_gpiodbclk,
	       &clock_manager_base->per_pll.gpiodiv);

	/* setup the EMAC clock mux select */
	writel((per_cfg->emacctl_emac0sel <<
		CLKMGR_PERPLL_EMACCTL_EMAC0SEL_LSB) |
	       (per_cfg->emacctl_emac1sel <<
		CLKMGR_PERPLL_EMACCTL_EMAC1SEL_LSB) |
	       (per_cfg->emacctl_emac2sel <<
		CLKMGR_PERPLL_EMACCTL_EMAC2SEL_LSB),
	       &clock_manager_base->per_pll.emacctl);

	/* at this stage, check for PLL lock status */
	cm_wait_for_lock(LOCKED_MASK);

	/*
	 * after locking, but before taking out of bypass,
	 * assert/deassert outresetall
	 */
	/* assert mainpll outresetall */
	setbits_le32(&clock_manager_base->main_pll.vco0,
		     CLKMGR_MAINPLL_VCO0_OUTRSTALL_SET_MSK);
	/* assert perpll outresetall */
	setbits_le32(&clock_manager_base->per_pll.vco0,
		     CLKMGR_PERPLL_VCO0_OUTRSTALL_SET_MSK);
	/* de-assert mainpll outresetall */
	clrbits_le32(&clock_manager_base->main_pll.vco0,
		     CLKMGR_MAINPLL_VCO0_OUTRSTALL_SET_MSK);
	/* de-assert perpll outresetall */
	clrbits_le32(&clock_manager_base->per_pll.vco0,
		     CLKMGR_PERPLL_VCO0_OUTRSTALL_SET_MSK);

	/* Take all PLLs out of bypass when boot mode is cleared. */
	/* release mainpll from bypass */
	writel(CLKMGR_MAINPLL_BYPASS_RESET,
	       &clock_manager_base->main_pll.bypassr);
	/* wait till Clock Manager is not busy */
	cm_wait_for_fsm();

	/* release perpll from bypass */
	writel(CLKMGR_PERPLL_BYPASS_RESET,
	       &clock_manager_base->per_pll.bypassr);
	/* wait till Clock Manager is not busy */
	cm_wait_for_fsm();

	/* clear boot mode */
	clrbits_le32(&clock_manager_base->ctrl,
		     CLKMGR_CLKMGR_CTL_BOOTMOD_SET_MSK);
	/* wait till Clock Manager is not busy */
	cm_wait_for_fsm();

	/* At here, we need to ramp to final value if needed */
	if (pll_ramp_main_hz != 0)
		cm_pll_ramp_main(main_cfg, per_cfg, pll_ramp_main_hz);
	if (pll_ramp_periph_hz != 0)
		cm_pll_ramp_periph(main_cfg, per_cfg, pll_ramp_periph_hz);

	/* Now ungate non-hw-managed clocks */
	writel(CLKMGR_MAINPLL_EN_S2FUSER0CLKEN_SET_MSK |
		CLKMGR_MAINPLL_EN_HMCPLLREFCLKEN_SET_MSK,
		&clock_manager_base->main_pll.ens);
	writel(CLKMGR_PERPLL_EN_RESET, &clock_manager_base->per_pll.ens);

	/* Clear the loss lock and slip bits as they might set during
	clock reconfiguration */
	writel(CLKMGR_CLKMGR_INTR_MAINPLLLOST_SET_MSK |
	       CLKMGR_CLKMGR_INTR_PERPLLLOST_SET_MSK |
	       CLKMGR_CLKMGR_INTR_MAINPLLRFSLIP_SET_MSK |
	       CLKMGR_CLKMGR_INTR_PERPLLRFSLIP_SET_MSK |
	       CLKMGR_CLKMGR_INTR_MAINPLLFBSLIP_SET_MSK |
	       CLKMGR_CLKMGR_INTR_PERPLLFBSLIP_SET_MSK,
	       &clock_manager_base->intr);

	return 0;
}

static void cm_use_intosc(void)
{
	setbits_le32(&clock_manager_base->ctrl,
		     CLKMGR_CLKMGR_CTL_BOOTCLK_INTOSC_SET_MSK);
}

int cm_basic_init(const void *blob)
{
	struct mainpll_cfg main_cfg;
	struct perpll_cfg per_cfg;
	int rval;

	/* initialize to zero for use case of optional node */
	memset(&main_cfg, 0, sizeof(main_cfg));
	memset(&per_cfg, 0, sizeof(per_cfg));

	rval = of_get_clk_cfg(blob, &main_cfg, &per_cfg);
	if (rval)
		return rval;

	cm_use_intosc();

	return cm_full_cfg(&main_cfg, &per_cfg);
}
#endif

static u32 cm_get_rate_dm(char *name)
{
	struct uclass *uc;
	struct udevice *dev = NULL;
	struct clk clk = { 0 };
	ulong rate;
	int ret;

	/* Device addresses start at 1 */
	ret = uclass_get(UCLASS_CLK, &uc);
	if (ret)
		return 0;

	ret = uclass_get_device_by_name(UCLASS_CLK, name, &dev);
	if (ret)
		return 0;

	ret = device_probe(dev);
	if (ret)
		return 0;

	ret = clk_request(dev, &clk);
	if (ret)
		return 0;

	rate = clk_get_rate(&clk);

	clk_free(&clk);

	return rate;
}

static u32 cm_get_rate_dm_khz(char *name)
{
	return cm_get_rate_dm(name) / 1000;
}

unsigned long cm_get_mpu_clk_hz(void)
{
	return cm_get_rate_dm("main_mpu_base_clk");
}

unsigned int cm_get_qspi_controller_clk_hz(void)
{
	return cm_get_rate_dm("qspi_clk");
}

unsigned int cm_get_l4_sp_clk_hz(void)
{
	return cm_get_rate_dm("l4_sp_clk");
}

void cm_print_clock_quick_summary(void)
{
	printf("MPU       %10d kHz\n", cm_get_rate_dm_khz("main_mpu_base_clk"));
	printf("MMC         %8d kHz\n", cm_get_rate_dm_khz("sdmmc_clk"));
	printf("QSPI        %8d kHz\n", cm_get_rate_dm_khz("qspi_clk"));
	printf("SPI         %8d kHz\n", cm_get_rate_dm_khz("spi_m_clk"));
	printf("EOSC1       %8d kHz\n", cm_get_rate_dm_khz("osc1"));
	printf("cb_intosc   %8d kHz\n", cm_get_rate_dm_khz("cb_intosc_ls_clk"));
	printf("f2s_free    %8d kHz\n", cm_get_rate_dm_khz("f2s_free_clk"));
	printf("Main VCO    %8d kHz\n", cm_get_rate_dm_khz("main_pll@40"));
	printf("NOC         %8d kHz\n", cm_get_rate_dm_khz("main_noc_base_clk"));
	printf("L4 Main	    %8d kHz\n", cm_get_rate_dm_khz("l4_main_clk"));
	printf("L4 MP       %8d kHz\n", cm_get_rate_dm_khz("l4_mp_clk"));
	printf("L4 SP       %8d kHz\n", cm_get_rate_dm_khz("l4_sp_clk"));
	printf("L4 sys free %8d kHz\n", cm_get_rate_dm_khz("l4_sys_free_clk"));
}
