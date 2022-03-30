/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010-2014
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TEGRA_CLK_RST_H_
#define _TEGRA_CLK_RST_H_

/* PLL registers - there are several PLLs in the clock controller */
struct clk_pll {
	uint pll_base;		/* the control register */
	/* pll_out[0] is output A control, pll_out[1] is output B control */
	uint pll_out[2];
	uint pll_misc;		/* other misc things */
};

/* PLL registers - there are several PLLs in the clock controller */
struct clk_pll_simple {
	uint pll_base;		/* the control register */
	uint pll_misc;		/* other misc things */
};

struct clk_pllm {
	uint pllm_base;		/* the control register */
	uint pllm_out;		/* output control */
	uint pllm_misc1;	/* misc1 */
	uint pllm_misc2;	/* misc2 */
};

/* RST_DEV_(L,H,U,V,W)_(SET,CLR) and CLK_ENB_(L,H,U,V,W)_(SET,CLR) */
struct clk_set_clr {
	uint set;
	uint clr;
};

/*
 * Most PLLs use the clk_pll structure, but some have a simpler two-member
 * structure for which we use clk_pll_simple. The reason for this non-
 * othogonal setup is not stated.
 */
enum {
	TEGRA_CLK_PLLS		= 6,	/* Number of normal PLLs */
	TEGRA_CLK_SIMPLE_PLLS	= 3,	/* Number of simple PLLs */
	TEGRA_CLK_REGS		= 3,	/* Number of clock enable regs L/H/U */
	TEGRA_CLK_SOURCES	= 64,	/* Number of ppl clock sources L/H/U */
	TEGRA_CLK_REGS_VW	= 2,	/* Number of clock enable regs V/W */
	TEGRA_CLK_SOURCES_VW	= 32,	/* Number of ppl clock sources V/W */
	TEGRA_CLK_SOURCES_X	= 32,	/* Number of ppl clock sources X */
	TEGRA_CLK_SOURCES_Y	= 18,	/* Number of ppl clock sources Y */
};

/* Clock/Reset Controller (CLK_RST_CONTROLLER_) regs */
struct clk_rst_ctlr {
	uint crc_rst_src;			/* _RST_SOURCE_0,0x00 */
	uint crc_rst_dev[TEGRA_CLK_REGS];	/* _RST_DEVICES_L/H/U_0 */
	uint crc_clk_out_enb[TEGRA_CLK_REGS];	/* _CLK_OUT_ENB_L/H/U_0 */
	uint crc_reserved0;		/* reserved_0,		0x1C */
	uint crc_cclk_brst_pol;		/* _CCLK_BURST_POLICY_0, 0x20 */
	uint crc_super_cclk_div;	/* _SUPER_CCLK_DIVIDER_0,0x24 */
	uint crc_sclk_brst_pol;		/* _SCLK_BURST_POLICY_0, 0x28 */
	uint crc_super_sclk_div;	/* _SUPER_SCLK_DIVIDER_0,0x2C */
	uint crc_clk_sys_rate;		/* _CLK_SYSTEM_RATE_0,	0x30 */
	uint crc_prog_dly_clk;		/* _PROG_DLY_CLK_0,	0x34 */
	uint crc_aud_sync_clk_rate;	/* _AUDIO_SYNC_CLK_RATE_0,0x38 */
	uint crc_reserved1;		/* reserved_1,		0x3C */
	uint crc_cop_clk_skip_plcy;	/* _COP_CLK_SKIP_POLICY_0,0x40 */
	uint crc_clk_mask_arm;		/* _CLK_MASK_ARM_0,	0x44 */
	uint crc_misc_clk_enb;		/* _MISC_CLK_ENB_0,	0x48 */
	uint crc_clk_cpu_cmplx;		/* _CLK_CPU_CMPLX_0,	0x4C */
	uint crc_osc_ctrl;		/* _OSC_CTRL_0,		0x50 */
	uint crc_pll_lfsr;		/* _PLL_LFSR_0,		0x54 */
	uint crc_osc_freq_det;		/* _OSC_FREQ_DET_0,	0x58 */
	uint crc_osc_freq_det_stat;	/* _OSC_FREQ_DET_STATUS_0,0x5C */
	uint crc_reserved2[8];		/* reserved_2[8],	0x60-7C */

	struct clk_pll crc_pll[TEGRA_CLK_PLLS];	/* PLLs from 0x80 to 0xdc */

	/* PLLs from 0xe0 to 0xf4    */
	struct clk_pll_simple crc_pll_simple[TEGRA_CLK_SIMPLE_PLLS];

	uint crc_reserved10;		/* _reserved_10,	0xF8 */
	uint crc_reserved11;		/* _reserved_11,	0xFC */

	uint crc_clk_src[TEGRA_CLK_SOURCES]; /*_I2S1_0...	0x100-1fc */

	uint crc_reserved20[32];	/* _reserved_20,	0x200-27c */

	uint crc_clk_out_enb_x;		/* _CLK_OUT_ENB_X_0,	0x280 */
	uint crc_clk_enb_x_set;		/* _CLK_ENB_X_SET_0,	0x284 */
	uint crc_clk_enb_x_clr;		/* _CLK_ENB_X_CLR_0,	0x288 */

	uint crc_rst_devices_x;		/* _RST_DEVICES_X_0,	0x28c */
	uint crc_rst_dev_x_set;		/* _RST_DEV_X_SET_0,	0x290 */
	uint crc_rst_dev_x_clr;		/* _RST_DEV_X_CLR_0,	0x294 */

	uint crc_clk_out_enb_y;		/* _CLK_OUT_ENB_Y_0,	0x298 */
	uint crc_clk_enb_y_set;		/* _CLK_ENB_Y_SET_0,	0x29c */
	uint crc_clk_enb_y_clr;		/* _CLK_ENB_Y_CLR_0,	0x2a0 */

	uint crc_rst_devices_y;		/* _RST_DEVICES_Y_0,	0x2a4 */
	uint crc_rst_dev_y_set;		/* _RST_DEV_Y_SET_0,	0x2a8 */
	uint crc_rst_dev_y_clr;		/* _RST_DEV_Y_CLR_0,	0x2ac */

	uint crc_reserved21[17];	/* _reserved_21,	0x2b0-2f0 */

	uint crc_dfll_base;		/* _DFLL_BASE_0,	0x2f4 */

	uint crc_reserved22[2];		/* _reserved_22,	0x2f8-2fc */

	/* _RST_DEV_L/H/U_SET_0 0x300 ~ 0x314 */
	struct clk_set_clr crc_rst_dev_ex[TEGRA_CLK_REGS];

	uint crc_reserved30[2];		/* _reserved_30,	0x318, 0x31c */

	/* _CLK_ENB_L/H/U_CLR_0 0x320 ~ 0x334 */
	struct clk_set_clr crc_clk_enb_ex[TEGRA_CLK_REGS];

	uint crc_reserved31[2];		/* _reserved_31,	0x338, 0x33c */

	uint crc_cpu_cmplx_set;		/* _RST_CPU_CMPLX_SET_0,    0x340 */
	uint crc_cpu_cmplx_clr;		/* _RST_CPU_CMPLX_CLR_0,    0x344 */

	/* Additional (T30) registers */
	uint crc_clk_cpu_cmplx_set;	/* _CLK_CPU_CMPLX_SET_0,    0x348 */
	uint crc_clk_cpu_cmplx_clr;	/* _CLK_CPU_CMPLX_SET_0,    0x34c */

	uint crc_reserved32[2];		/* _reserved_32,      0x350,0x354 */

	uint crc_rst_dev_vw[TEGRA_CLK_REGS_VW]; /* _RST_DEVICES_V/W_0 */
	uint crc_clk_out_enb_vw[TEGRA_CLK_REGS_VW]; /* _CLK_OUT_ENB_V/W_0 */
	uint crc_cclkg_brst_pol;	/* _CCLKG_BURST_POLICY_0,   0x368 */
	uint crc_super_cclkg_div;	/* _SUPER_CCLKG_DIVIDER_0,  0x36C */
	uint crc_cclklp_brst_pol;	/* _CCLKLP_BURST_POLICY_0,  0x370 */
	uint crc_super_cclkp_div;	/* _SUPER_CCLKLP_DIVIDER_0, 0x374 */
	uint crc_clk_cpug_cmplx;	/* _CLK_CPUG_CMPLX_0,       0x378 */
	uint crc_clk_cpulp_cmplx;	/* _CLK_CPULP_CMPLX_0,      0x37C */
	uint crc_cpu_softrst_ctrl;	/* _CPU_SOFTRST_CTRL_0,     0x380 */
	uint crc_cpu_softrst_ctrl1;	/* _CPU_SOFTRST_CTRL1_0,    0x384 */
	uint crc_cpu_softrst_ctrl2;	/* _CPU_SOFTRST_CTRL2_0,    0x388 */
	uint crc_reserved33[9];		/* _reserved_33,        0x38c-3ac */
	uint crc_clk_src_vw[TEGRA_CLK_SOURCES_VW];	/* 0x3B0-0x42C */
	/* _RST_DEV_V/W_SET_0 0x430 ~ 0x43c */
	struct clk_set_clr crc_rst_dev_ex_vw[TEGRA_CLK_REGS_VW];
	/* _CLK_ENB_V/W_CLR_0 0x440 ~ 0x44c */
	struct clk_set_clr crc_clk_enb_ex_vw[TEGRA_CLK_REGS_VW];
	/* Additional (T114+) registers */
	uint crc_rst_cpug_cmplx_set;	/* _RST_CPUG_CMPLX_SET_0,  0x450 */
	uint crc_rst_cpug_cmplx_clr;	/* _RST_CPUG_CMPLX_CLR_0,  0x454 */
	uint crc_rst_cpulp_cmplx_set;	/* _RST_CPULP_CMPLX_SET_0, 0x458 */
	uint crc_rst_cpulp_cmplx_clr;	/* _RST_CPULP_CMPLX_CLR_0, 0x45C */
	uint crc_clk_cpug_cmplx_set;	/* _CLK_CPUG_CMPLX_SET_0,  0x460 */
	uint crc_clk_cpug_cmplx_clr;	/* _CLK_CPUG_CMPLX_CLR_0,  0x464 */
	uint crc_clk_cpulp_cmplx_set;	/* _CLK_CPULP_CMPLX_SET_0, 0x468 */
	uint crc_clk_cpulp_cmplx_clr;	/* _CLK_CPULP_CMPLX_CLR_0, 0x46C */
	uint crc_cpu_cmplx_status;	/* _CPU_CMPLX_STATUS_0,    0x470 */
	uint crc_reserved40[1];		/* _reserved_40,        0x474 */
	uint crc_intstatus;		/* __INTSTATUS_0,       0x478 */
	uint crc_intmask;		/* __INTMASK_0,         0x47C */
	uint crc_utmip_pll_cfg0;	/* _UTMIP_PLL_CFG0_0,	0x480 */
	uint crc_utmip_pll_cfg1;	/* _UTMIP_PLL_CFG1_0,	0x484 */
	uint crc_utmip_pll_cfg2;	/* _UTMIP_PLL_CFG2_0,	0x488 */

	uint crc_plle_aux;		/* _PLLE_AUX_0,		0x48C */
	uint crc_sata_pll_cfg0;		/* _SATA_PLL_CFG0_0,	0x490 */
	uint crc_sata_pll_cfg1;		/* _SATA_PLL_CFG1_0,	0x494 */
	uint crc_pcie_pll_cfg0;		/* _PCIE_PLL_CFG0_0,	0x498 */

	uint crc_prog_audio_dly_clk;	/* _PROG_AUDIO_DLY_CLK_0, 0x49C */
	uint crc_audio_sync_clk_i2s0;	/* _AUDIO_SYNC_CLK_I2S0_0, 0x4A0 */
	uint crc_audio_sync_clk_i2s1;	/* _AUDIO_SYNC_CLK_I2S1_0, 0x4A4 */
	uint crc_audio_sync_clk_i2s2;	/* _AUDIO_SYNC_CLK_I2S2_0, 0x4A8 */
	uint crc_audio_sync_clk_i2s3;	/* _AUDIO_SYNC_CLK_I2S3_0, 0x4AC */
	uint crc_audio_sync_clk_i2s4;	/* _AUDIO_SYNC_CLK_I2S4_0, 0x4B0 */
	uint crc_audio_sync_clk_spdif;	/* _AUDIO_SYNC_CLK_SPDIF_0, 0x4B4 */

	uint crc_plld2_base;		/* _PLLD2_BASE_0, 0x4B8 */
	uint crc_plld2_misc;		/* _PLLD2_MISC_0, 0x4BC */
	uint crc_utmip_pll_cfg3;	/* _UTMIP_PLL_CFG3_0, 0x4C0 */
	uint crc_pllrefe_base;		/* _PLLREFE_BASE_0, 0x4C4 */
	uint crc_pllrefe_misc;		/* _PLLREFE_MISC_0, 0x4C8 */
	uint crs_reserved_50[7];	/* _reserved_50, 0x4CC-0x4E4 */
	uint crc_pllc2_base;		/* _PLLC2_BASE_0, 0x4E8 */
	uint crc_pllc2_misc0;		/* _PLLC2_MISC_0_0, 0x4EC */
	uint crc_pllc2_misc1;		/* _PLLC2_MISC_1_0, 0x4F0 */
	uint crc_pllc2_misc2;		/* _PLLC2_MISC_2_0, 0x4F4 */
	uint crc_pllc2_misc3;		/* _PLLC2_MISC_3_0, 0x4F8 */
	uint crc_pllc3_base;		/* _PLLC3_BASE_0, 0x4FC */
	uint crc_pllc3_misc0;		/* _PLLC3_MISC_0_0, 0x500 */
	uint crc_pllc3_misc1;		/* _PLLC3_MISC_1_0, 0x504 */
	uint crc_pllc3_misc2;		/* _PLLC3_MISC_2_0, 0x508 */
	uint crc_pllc3_misc3;		/* _PLLC3_MISC_3_0, 0x50C */
	uint crc_pllx_misc1;		/* _PLLX_MISC_1_0, 0x510 */
	uint crc_pllx_misc2;		/* _PLLX_MISC_2_0, 0x514 */
	uint crc_pllx_misc3;		/* _PLLX_MISC_3_0, 0x518 */
	uint crc_xusbio_pll_cfg0;	/* _XUSBIO_PLL_CFG0_0, 0x51C */
	uint crc_xusbio_pll_cfg1;	/* _XUSBIO_PLL_CFG0_1, 0x520 */
	uint crc_plle_aux1;		/* _PLLE_AUX1_0, 0x524 */
	uint crc_pllp_reshift;		/* _PLLP_RESHIFT_0, 0x528 */
	uint crc_utmipll_hw_pwrdn_cfg0;	/* _UTMIPLL_HW_PWRDN_CFG0_0, 0x52C */
	uint crc_pllu_hw_pwrdn_cfg0;	/* _PLLU_HW_PWRDN_CFG0_0, 0x530 */
	uint crc_xusb_pll_cfg0;		/* _XUSB_PLL_CFG0_0, 0x534 */
	uint crc_reserved51[1];		/* _reserved_51, 0x538 */
	uint crc_clk_cpu_misc;		/* _CLK_CPU_MISC_0, 0x53C */
	uint crc_clk_cpug_misc;		/* _CLK_CPUG_MISC_0, 0x540 */
	uint crc_clk_cpulp_misc;	/* _CLK_CPULP_MISC_0, 0x544 */
	uint crc_pllx_hw_ctrl_cfg;	/* _PLLX_HW_CTRL_CFG_0, 0x548 */
	uint crc_pllx_sw_ramp_cfg;	/* _PLLX_SW_RAMP_CFG_0, 0x54C */
	uint crc_pllx_hw_ctrl_status;	/* _PLLX_HW_CTRL_STATUS_0, 0x550 */
	uint crc_reserved52[1];		/* _reserved_52, 0x554 */
	uint crc_super_gr3d_clk_div;	/* _SUPER_GR3D_CLK_DIVIDER_0, 0x558 */
	uint crc_spare_reg0;		/* _SPARE_REG0_0, 0x55C */
	u32 _rsv32[4];                  /*                    0x560-0x56c */
	u32 crc_plld2_ss_cfg;		/* _PLLD2_SS_CFG            0x570 */
	u32 _rsv32_1[7];		/*                      0x574-58c */
	struct clk_pll_simple plldp;	/* _PLLDP_BASE, 0x590 _PLLDP_MISC */
	u32 crc_plldp_ss_cfg;		/* _PLLDP_SS_CFG, 0x598 */

	/* Tegra124+ - skip to 0x600 here for new CLK_SOURCE_ regs */
	uint _rsrv32_2[25];			/* _0x59C - 0x5FC */
	uint crc_clk_src_x[TEGRA_CLK_SOURCES_X]; /* XUSB, etc, 0x600-0x67C */

	/* Tegra210 - skip to 0x694 here for new CLK_SOURCE_ regs */
	uint crc_reserved61[5];	/* _reserved_61, 0x680 - 0x690 */
	/*
	 * NOTE: PLLA1 regs are in the middle of this Y region. Break this in
	 * two later if PLLA1 is needed, but for now this is cleaner.
	 */
	uint crc_clk_src_y[TEGRA_CLK_SOURCES_Y]; /* SPARE1, etc, 0x694-0x6D8 */
};

/* CLK_RST_CONTROLLER_CLK_CPU_CMPLX_0 */
#define CPU3_CLK_STP_SHIFT	11
#define CPU2_CLK_STP_SHIFT	10
#define CPU1_CLK_STP_SHIFT	9
#define CPU0_CLK_STP_SHIFT	8
#define CPU0_CLK_STP_MASK	(1U << CPU0_CLK_STP_SHIFT)

/* CLK_RST_CONTROLLER_PLLx_BASE_0 */
#define PLL_BYPASS_SHIFT	31
#define PLL_BYPASS_MASK		(1U << PLL_BYPASS_SHIFT)

#define PLL_ENABLE_SHIFT	30
#define PLL_ENABLE_MASK		(1U << PLL_ENABLE_SHIFT)

#define PLL_BASE_OVRRIDE_MASK	(1U << 28)

#define PLL_LOCK_SHIFT		27
#define PLL_LOCK_MASK		(1U << PLL_LOCK_SHIFT)

/* CLK_RST_CONTROLLER_PLLx_OUTx_0 */
#define PLL_OUT_RSTN		(1 << 0)
#define PLL_OUT_CLKEN		(1 << 1)
#define PLL_OUT_OVRRIDE		(1 << 2)

#define PLL_OUT_RATIO_SHIFT	8
#define PLL_OUT_RATIO_MASK	(0xffU << PLL_OUT_RATIO_SHIFT)

/* CLK_RST_CONTROLLER_PLLx_MISC_0 */
#define PLL_DCCON_SHIFT		20
#define PLL_DCCON_MASK		(1U << PLL_DCCON_SHIFT)

#define PLLP_OUT1_OVR		(1 << 2)
#define PLLP_OUT2_OVR		(1 << 18)
#define PLLP_OUT3_OVR		(1 << 2)
#define PLLP_OUT4_OVR		(1 << 18)
#define PLLP_OUT1_RATIO		8
#define PLLP_OUT2_RATIO		24
#define PLLP_OUT3_RATIO		8
#define PLLP_OUT4_RATIO		24

enum {
	IN_408_OUT_204_DIVISOR = 2,
	IN_408_OUT_102_DIVISOR = 6,
	IN_408_OUT_48_DIVISOR = 15,
	IN_408_OUT_9_6_DIVISOR = 83,
};

#define PLLP_OUT1_RSTN_DIS	(1 << 0)
#define PLLP_OUT1_RSTN_EN	(0 << 0)
#define PLLP_OUT1_CLKEN		(1 << 1)
#define PLLP_OUT2_RSTN_DIS	(1 << 16)
#define PLLP_OUT2_RSTN_EN	(0 << 16)
#define PLLP_OUT2_CLKEN		(1 << 17)

#define PLLP_OUT3_RSTN_DIS	(1 << 0)
#define PLLP_OUT3_RSTN_EN	(0 << 0)
#define PLLP_OUT3_CLKEN		(1 << 1)
#define PLLP_OUT4_RSTN_DIS	(1 << 16)
#define PLLP_OUT4_RSTN_EN	(0 << 16)
#define PLLP_OUT4_CLKEN		(1 << 17)

/* CLK_RST_CONTROLLER_UTMIP_PLL_CFG1_0 */
#define PLLU_POWERDOWN		(1 << 16)
#define PLL_ENABLE_POWERDOWN	(1 << 14)
#define PLL_ACTIVE_POWERDOWN	(1 << 12)

/* CLK_RST_CONTROLLER_UTMIP_PLL_CFG2_0 */
#define UTMIP_FORCE_PD_SAMP_C_POWERDOWN		(1 << 4)
#define UTMIP_FORCE_PD_SAMP_B_POWERDOWN		(1 << 2)
#define UTMIP_FORCE_PD_SAMP_A_POWERDOWN		(1 << 0)

/* CLK_RST_CONTROLLER_OSC_CTRL_0 0x50 */
#define OSC_XOE_SHIFT			0
#define OSC_XOE_MASK			(1 << OSC_XOE_SHIFT)
#define OSC_XOE_ENABLE			(1 << OSC_XOE_SHIFT)
#define OSC_XOBP_SHIFT			1
#define OSC_XOBP_MASK			(1U << OSC_XOBP_SHIFT)
#define OSC_XOFS_SHIFT			4
#define OSC_XOFS_MASK			(0x3F << OSC_XOFS_SHIFT)
#define OSC_DRIVE_STRENGTH		7

/*
 * CLK_RST_CONTROLLER_CLK_SOURCE_x_OUT_0 - the mask here is normally 8 bits
 * but can be 16. We could use knowledge we have to restrict the mask in
 * the 8-bit cases (the divider_bits value returned by
 * get_periph_clock_source()) but it does not seem worth it since the code
 * already checks the ranges of values it is writing, in clk_get_divider().
 */
#define OUT_CLK_DIVISOR_SHIFT	0
#define OUT_CLK_DIVISOR_MASK	(0xffff << OUT_CLK_DIVISOR_SHIFT)

#define OUT_CLK_SOURCE_31_30_SHIFT	30
#define OUT_CLK_SOURCE_31_30_MASK	(3U << OUT_CLK_SOURCE_31_30_SHIFT)

#define OUT_CLK_SOURCE_31_29_SHIFT	29
#define OUT_CLK_SOURCE_31_29_MASK	(7U << OUT_CLK_SOURCE_31_29_SHIFT)

/* Note: See comment for MASK_BITS_31_28 in arch-tegra/clock.h */
#define OUT_CLK_SOURCE_31_28_SHIFT	28
#define OUT_CLK_SOURCE_31_28_MASK	(15U << OUT_CLK_SOURCE_31_28_SHIFT)

/* CLK_RST_CONTROLLER_SCLK_BURST_POLICY */
#define SCLK_SYS_STATE_SHIFT    28U
#define SCLK_SYS_STATE_MASK     (15U << SCLK_SYS_STATE_SHIFT)
enum {
	SCLK_SYS_STATE_STDBY,
	SCLK_SYS_STATE_IDLE,
	SCLK_SYS_STATE_RUN,
	SCLK_SYS_STATE_IRQ = 4U,
	SCLK_SYS_STATE_FIQ = 8U,
};
#define SCLK_COP_FIQ_MASK       (1 << 27)
#define SCLK_CPU_FIQ_MASK       (1 << 26)
#define SCLK_COP_IRQ_MASK       (1 << 25)
#define SCLK_CPU_IRQ_MASK       (1 << 24)

#define SCLK_SWAKEUP_FIQ_SOURCE_SHIFT		12
#define SCLK_SWAKEUP_FIQ_SOURCE_MASK		\
		(7 << SCLK_SWAKEUP_FIQ_SOURCE_SHIFT)
#define SCLK_SWAKEUP_IRQ_SOURCE_SHIFT		8
#define SCLK_SWAKEUP_IRQ_SOURCE_MASK		\
		(7 << SCLK_SWAKEUP_FIQ_SOURCE_SHIFT)
#define SCLK_SWAKEUP_RUN_SOURCE_SHIFT		4
#define SCLK_SWAKEUP_RUN_SOURCE_MASK		\
		(7 << SCLK_SWAKEUP_FIQ_SOURCE_SHIFT)
#define SCLK_SWAKEUP_IDLE_SOURCE_SHIFT		0

#define SCLK_SWAKEUP_IDLE_SOURCE_MASK		\
		(7 << SCLK_SWAKEUP_FIQ_SOURCE_SHIFT)
enum {
	SCLK_SOURCE_CLKM,
	SCLK_SOURCE_PLLC_OUT1,
	SCLK_SOURCE_PLLP_OUT4,
	SCLK_SOURCE_PLLP_OUT3,
	SCLK_SOURCE_PLLP_OUT2,
	SCLK_SOURCE_CLKD,
	SCLK_SOURCE_CLKS,
	SCLK_SOURCE_PLLM_OUT1,
};
#define SCLK_SWAKE_FIQ_SRC_PLLM_OUT1    (7 << 12)
#define SCLK_SWAKE_IRQ_SRC_PLLM_OUT1    (7 << 8)
#define SCLK_SWAKE_RUN_SRC_PLLM_OUT1    (7 << 4)
#define SCLK_SWAKE_IDLE_SRC_PLLM_OUT1   (7 << 0)

/* CLK_RST_CONTROLLER_SUPER_SCLK_DIVIDER */
#define SUPER_SCLK_ENB_SHIFT		31U
#define SUPER_SCLK_ENB_MASK		(1U << 31)
#define SUPER_SCLK_DIVIDEND_SHIFT	8
#define SUPER_SCLK_DIVIDEND_MASK	(0xff << SUPER_SCLK_DIVIDEND_SHIFT)
#define SUPER_SCLK_DIVISOR_SHIFT	0
#define SUPER_SCLK_DIVISOR_MASK		(0xff << SUPER_SCLK_DIVISOR_SHIFT)

/* CLK_RST_CONTROLLER_CLK_SYSTEM_RATE 0x30 */
#define CLK_SYS_RATE_HCLK_DISABLE_SHIFT 7
#define CLK_SYS_RATE_HCLK_DISABLE_MASK  (1 << CLK_SYS_RATE_HCLK_DISABLE_SHIFT)
#define CLK_SYS_RATE_AHB_RATE_SHIFT     4
#define CLK_SYS_RATE_AHB_RATE_MASK      (3 << CLK_SYS_RATE_AHB_RATE_SHIFT)
#define CLK_SYS_RATE_PCLK_DISABLE_SHIFT 3
#define CLK_SYS_RATE_PCLK_DISABLE_MASK  (1 << CLK_SYS_RATE_PCLK_DISABLE_SHIFT)
#define CLK_SYS_RATE_APB_RATE_SHIFT     0
#define CLK_SYS_RATE_APB_RATE_MASK      (3 << CLK_SYS_RATE_AHB_RATE_SHIFT)

/* CLK_RST_CONTROLLER_RST_CPUxx_CMPLX_CLR 0x344 */
#define CLR_CPURESET0			(1 << 0)
#define CLR_CPURESET1			(1 << 1)
#define CLR_CPURESET2			(1 << 2)
#define CLR_CPURESET3			(1 << 3)
#define CLR_DBGRESET0			(1 << 12)
#define CLR_DBGRESET1			(1 << 13)
#define CLR_DBGRESET2			(1 << 14)
#define CLR_DBGRESET3			(1 << 15)
#define CLR_CORERESET0			(1 << 16)
#define CLR_CORERESET1			(1 << 17)
#define CLR_CORERESET2			(1 << 18)
#define CLR_CORERESET3			(1 << 19)
#define CLR_CXRESET0			(1 << 20)
#define CLR_CXRESET1			(1 << 21)
#define CLR_CXRESET2			(1 << 22)
#define CLR_CXRESET3			(1 << 23)
#define CLR_L2RESET			(1 << 24)
#define CLR_NONCPURESET			(1 << 29)
#define CLR_PRESETDBG			(1 << 30)

/* CLK_RST_CONTROLLER_CLK_CPU_CMPLX_CLR 0x34c */
#define CLR_CPU0_CLK_STP		(1 << 8)
#define CLR_CPU1_CLK_STP		(1 << 9)
#define CLR_CPU2_CLK_STP		(1 << 10)
#define CLR_CPU3_CLK_STP		(1 << 11)

/* CRC_CLK_SOURCE_MSELECT_0 0x3b4 */
#define MSELECT_CLK_SRC_PLLP_OUT0	(0 << 29)

/* CRC_CLK_ENB_V_SET_0 0x440 */
#define SET_CLK_ENB_CPUG_ENABLE		(1 << 0)
#define SET_CLK_ENB_CPULP_ENABLE	(1 << 1)
#define SET_CLK_ENB_MSELECT_ENABLE	(1 << 3)

/* CLK_RST_CONTROLLER_UTMIP_PLL_CFG1_0 0x484 */
#define PLL_ACTIVE_POWERDOWN		(1 << 12)
#define PLL_ENABLE_POWERDOWN		(1 << 14)
#define PLLU_POWERDOWN			(1 << 16)

/* CLK_RST_CONTROLLER_UTMIP_PLL_CFG2_0 0x488 */
#define UTMIP_FORCE_PD_SAMP_A_POWERDOWN	(1 << 0)
#define UTMIP_FORCE_PD_SAMP_B_POWERDOWN	(1 << 2)
#define UTMIP_FORCE_PD_SAMP_C_POWERDOWN	(1 << 4)

/* CLK_RST_CONTROLLER_PLLX_MISC_3 */
#define PLLX_IDDQ_SHIFT			3
#define PLLX_IDDQ_MASK			(1U << PLLX_IDDQ_SHIFT)

/* CLK_RST_PLLDP_SS_CFG */
#define PLLDP_SS_CFG_CLAMP		(1 << 22)
#define PLLDP_SS_CFG_UNDOCUMENTED	(1 << 24)
#define PLLDP_SS_CFG_DITHER		(1 << 28)

/* CLK_RST_PLLD_MISC */
#define PLLD_CLKENABLE			30

#endif	/* _TEGRA_CLK_RST_H_ */
