// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016  Nexell Co., Ltd.
 *
 * Author: junghyun, kim <jhkim@nexell.co.kr>
 */

#include <config.h>
#include <common.h>
#include <errno.h>

#include <asm/arch/nexell.h>
#include <asm/arch/reset.h>
#include <asm/arch/display.h>

#include "soc/s5pxx18_soc_lvds.h"
#include "soc/s5pxx18_soc_disptop.h"
#include "soc/s5pxx18_soc_disptop_clk.h"

#define	__io_address(a)	(void *)(uintptr_t)(a)

static void lvds_phy_reset(void)
{
	nx_rstcon_setrst(RESET_ID_LVDS, RSTCON_ASSERT);
	nx_rstcon_setrst(RESET_ID_LVDS, RSTCON_NEGATE);
}

static void lvds_init(void)
{
	int clkid = DP_CLOCK_LVDS;
	int index = 0;
	void *base;

	base = __io_address(nx_disp_top_clkgen_get_physical_address(clkid));
	nx_disp_top_clkgen_set_base_address(clkid, base);

	nx_lvds_initialize();

	for (index = 0; nx_lvds_get_number_of_module() > index; index++)
		nx_lvds_set_base_address(index,
		  (void *)__io_address(nx_lvds_get_physical_address(index)));

	nx_disp_top_clkgen_set_clock_pclk_mode(clkid, nx_pclkmode_always);
}

static void lvds_enable(int enable)
{
	int clkid = DP_CLOCK_LVDS;
	int on = (enable ? 1 : 0);

	nx_disp_top_clkgen_set_clock_divisor_enable(clkid, on);
}

static int lvds_setup(int module, int input,
		      struct dp_sync_info *sync, struct dp_ctrl_info *ctrl,
		      struct dp_lvds_dev *dev)
{
	unsigned int val;
	int clkid = DP_CLOCK_LVDS;
	enum dp_lvds_format format = DP_LVDS_FORMAT_JEIDA;
	u32 voltage = DEF_VOLTAGE_LEVEL;

	if (dev) {
		format = dev->lvds_format;
		voltage = dev->voltage_level;
	}

	printf("LVDS:  ");
	printf("%s, ", format == DP_LVDS_FORMAT_VESA ? "VESA" :
		format == DP_LVDS_FORMAT_JEIDA ? "JEIDA" : "LOC");
	printf("voltage LV:0x%x\n", voltage);

	/*
	 *-------- predefined type.
	 * only change iTA to iTE in VESA mode
	 * wire [34:0] loc_VideoIn =
	 * {4'hf, 4'h0, i_VDEN, i_VSYNC, i_HSYNC, i_VD[23:0] };
	 */
	u32 VSYNC = 25;
	u32 HSYNC = 24;
	u32 VDEN  = 26; /* bit position */
	u32 ONE   = 34;
	u32 ZERO  = 27;

	/*====================================================
	 * current not use location mode
	 *====================================================
	 */
	u32 LOC_A[7] = {ONE, ONE, ONE, ONE, ONE, ONE, ONE};
	u32 LOC_B[7] = {ONE, ONE, ONE, ONE, ONE, ONE, ONE};
	u32 LOC_C[7] = {VDEN, VSYNC, HSYNC, ONE, HSYNC, VSYNC, VDEN};
	u32 LOC_D[7] = {ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO};
	u32 LOC_E[7] = {ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO};

	switch (input) {
	case DP_DEVICE_DP0:
		input = 0;
		break;
	case DP_DEVICE_DP1:
		input = 1;
		break;
	case DP_DEVICE_RESCONV:
		input = 2;
		break;
	default:
		return -EINVAL;
	}

	/*
	 * select TOP MUX
	 */
	nx_disp_top_clkgen_set_clock_divisor_enable(clkid, 0);
	nx_disp_top_clkgen_set_clock_source(clkid, 0, ctrl->clk_src_lv0);
	nx_disp_top_clkgen_set_clock_divisor(clkid, 0, ctrl->clk_div_lv0);
	nx_disp_top_clkgen_set_clock_source(clkid, 1, ctrl->clk_src_lv1);
	nx_disp_top_clkgen_set_clock_divisor(clkid, 1, ctrl->clk_div_lv1);

	/*
	 * LVDS Control Pin Setting
	 */
	val = (0 << 30) |      /* CPU_I_VBLK_FLAG_SEL */
	      (0 << 29) |      /* CPU_I_BVLK_FLAG */
	      (1 << 28) |      /* SKINI_BST  */
	      (1 << 27) |      /* DLYS_BST  */
	      (0 << 26) |      /* I_AUTO_SEL */
	      (format << 19) | /* JEiDA data packing */
	      (0x1B << 13)   | /* I_LOCK_PPM_SET, PPM setting for PLL lock */
	      (0x638 << 1);    /* I_DESKEW_CNT_SEL, period of de-skew region */
	nx_lvds_set_lvdsctrl0(0, val);

	val = (0 << 28) |   /* I_ATE_MODE, function mode */
	      (0 << 27) |   /* I_TEST_CON_MODE, DA (test ctrl mode) */
	      (0 << 24) |   /* I_TX4010X_DUMMY */
	      (0 << 15) |   /* SKCCK 0 */
	      (0 << 12) |   /* SKC4 (TX output skew control pin at ODD ch4) */
	      (0 << 9)  |   /* SKC3 (TX output skew control pin at ODD ch3) */
	      (0 << 6)  |   /* SKC2 (TX output skew control pin at ODD ch2) */
	      (0 << 3)  |   /* SKC1 (TX output skew control pin at ODD ch1) */
	      (0 << 0);     /* SKC0 (TX output skew control pin at ODD ch0) */
	nx_lvds_set_lvdsctrl1(0, val);

	val = (0 << 15)   | /* CK_POL_SEL, Input clock, bypass */
	      (0 << 14)   | /* VSEL, VCO Freq. range. 0: Low(40MHz~90MHz),
			     *                        1: High(90MHz~160MHz) */
	      (0x1 << 12) | /* S (Post-scaler) */
	      (0xA << 6)  | /* M (Main divider) */
	      (0xA << 0);   /* P (Pre-divider) */

	nx_lvds_set_lvdsctrl2(0, val);
	val = (0x03 << 6) | /* SK_BIAS, Bias current ctrl pin */
	      (0 << 5)    | /* SKEWINI, skew selection pin, 0: bypass,
			     *                              1: skew enable */
	      (0 << 4)    | /* SKEW_EN_H, skew block power down, 0: power down,
			     *                                   1: operating */
	      (1 << 3)    | /* CNTB_TDLY, delay control pin */
	      (0 << 2)    | /* SEL_DATABF, input clock 1/2 division cont. pin */
	      (0x3 << 0);   /* SKEW_REG_CUR, regulator bias current selection
			     *               in SKEW block */

	nx_lvds_set_lvdsctrl3(0, val);
	val = (0 << 28)   | /* FLT_CNT, filter control pin for PLL */
	      (0 << 27)   | /* VOD_ONLY_CNT, the pre-emphasis's pre-diriver
			     *               control pin (VOD only) */
	      (0 << 26)   | /* CNNCT_MODE_SEL, connectivity mode selection,
			     *                 0:TX operating, 1:con check */
	      (0 << 24)   | /* CNNCT_CNT, connectivity ctrl pin,
			     *            0: tx operating, 1: con check */
	      (0 << 23)   | /* VOD_HIGH_S, VOD control pin, 1: Vod only */
	      (0 << 22)   | /* SRC_TRH, source termination resistor sel. pin */
	      (voltage << 14) |
	      (0x01 << 6) | /* CNT_PEN_H, TX driver pre-emphasis level cont. */
	      (0x4 << 3)  | /* FC_CODE, vos control pin */
	      (0 << 2)    | /* OUTCON, TX Driver state selectioin pin, 0:Hi-z,
			     *                                         1:Low */
	      (0 << 1)    | /* LOCK_CNT, Lock signal selection pin, enable */
	      (0 << 0);     /* AUTO_DSK_SEL, auto deskew sel. pin, normal */
	nx_lvds_set_lvdsctrl4(0, val);

	val = (0 << 24) |   /* I_BIST_RESETB */
	      (0 << 23) |   /* I_BIST_EN */
	      (0 << 21) |   /* I_BIST_PAT_SEL */
	      (0 << 14) |   /* I_BIST_USER_PATTERN */
	      (0 << 13) |   /* I_BIST_FORCE_ERROR */
	      (0 << 7)  |   /* I_BIST_SKEW_CTRL */
	      (0 << 5)  |   /* I_BIST_CLK_INV */
	      (0 << 3)  |   /* I_BIST_DATA_INV */
	      (0 << 0);     /* I_BIST_CH_SEL */
	nx_lvds_set_lvdstmode0(0, val);

	/* user do not need to modify this codes. */
	val = (LOC_A[4] << 24) | (LOC_A[3] << 18) | (LOC_A[2] << 12) |
	      (LOC_A[1] << 6)  | (LOC_A[0] << 0);
	nx_lvds_set_lvdsloc0(0, val);

	val = (LOC_B[2] << 24) | (LOC_B[1] << 18) | (LOC_B[0] << 12) |
	      (LOC_A[6] << 6)  | (LOC_A[5] << 0);
	nx_lvds_set_lvdsloc1(0, val);

	val = (LOC_C[0] << 24) | (LOC_B[6] << 18) | (LOC_B[5] << 12) |
	      (LOC_B[4] << 6)  | (LOC_B[3] << 0);
	nx_lvds_set_lvdsloc2(0, val);

	val = (LOC_C[5] << 24) | (LOC_C[4] << 18) | (LOC_C[3] << 12) |
	      (LOC_C[2] << 6)  | (LOC_C[1] << 0);
	nx_lvds_set_lvdsloc3(0, val);

	val = (LOC_D[3] << 24) | (LOC_D[2] << 18) | (LOC_D[1] << 12) |
	      (LOC_D[0] << 6)  | (LOC_C[6] << 0);
	nx_lvds_set_lvdsloc4(0, val);

	val = (LOC_E[1] << 24) | (LOC_E[0] << 18) | (LOC_D[6] << 12) |
	      (LOC_D[5] << 6)  | (LOC_D[4] << 0);
	nx_lvds_set_lvdsloc5(0, val);

	val = (LOC_E[6] << 24) | (LOC_E[5] << 18) | (LOC_E[4] << 12) |
	      (LOC_E[3] << 6)  | (LOC_E[2] << 0);
	nx_lvds_set_lvdsloc6(0, val);

	nx_lvds_set_lvdslocmask0(0, 0xffffffff);
	nx_lvds_set_lvdslocmask1(0, 0xffffffff);

	nx_lvds_set_lvdslocpol0(0, (0 << 19) | (0 << 18));

	/*
	 * select TOP MUX
	 */
	nx_disp_top_set_lvdsmux(1, input);

	/*
	 * LVDS PHY Reset, make sure last.
	 */
	lvds_phy_reset();

	return 0;
}

void nx_lvds_display(int module,
		     struct dp_sync_info *sync, struct dp_ctrl_info *ctrl,
		     struct dp_plane_top *top, struct dp_plane_info *planes,
		     struct dp_lvds_dev *dev)
{
	struct dp_plane_info *plane = planes;
	int input = module == 0 ? DP_DEVICE_DP0 : DP_DEVICE_DP1;
	int count = top->plane_num;
	int i = 0;

	printf("LVDS:  dp.%d\n", module);

	dp_control_init(module);
	dp_plane_init(module);

	lvds_init();

	/* set plane */
	dp_plane_screen_setup(module, top);

	for (i = 0; count > i; i++, plane++) {
		if (!plane->enable)
			continue;
		dp_plane_layer_setup(module, plane);
		dp_plane_layer_enable(module, plane, 1);
	}

	dp_plane_screen_enable(module, 1);

	/* set lvds */
	lvds_setup(module, input, sync, ctrl, dev);

	lvds_enable(1);

	/* set dp control */
	dp_control_setup(module, sync, ctrl);
	dp_control_enable(module, 1);
}
