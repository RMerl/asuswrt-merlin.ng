/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010-2019
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _PMC_H_
#define _PMC_H_

#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

/* Power Management Controller (APBDEV_PMC_) registers */
struct pmc_ctlr {
	uint pmc_cntrl;			/* _CNTRL_0, offset 00 */
	uint pmc_sec_disable;		/* _SEC_DISABLE_0, offset 04 */
	uint pmc_pmc_swrst;		/* _PMC_SWRST_0, offset 08 */
	uint pmc_wake_mask;		/* _WAKE_MASK_0, offset 0C */
	uint pmc_wake_lvl;		/* _WAKE_LVL_0, offset 10 */
	uint pmc_wake_status;		/* _WAKE_STATUS_0, offset 14 */
	uint pmc_sw_wake_status;	/* _SW_WAKE_STATUS_0, offset 18 */
	uint pmc_dpd_pads_oride;	/* _DPD_PADS_ORIDE_0, offset 1C */
	uint pmc_dpd_sample;		/* _DPD_PADS_SAMPLE_0, offset 20 */
	uint pmc_dpd_enable;		/* _DPD_PADS_ENABLE_0, offset 24 */
	uint pmc_pwrgate_timer_off;	/* _PWRGATE_TIMER_OFF_0, offset 28 */
#if defined(CONFIG_TEGRA20) || defined(CONFIG_TEGRA30)
	uint pmc_pwrgate_timer_on;	/* _PWRGATE_TIMER_ON_0, offset 2C */
#else
	uint pmc_clamp_status;		/* _CLAMP_STATUS_0, offset 2C */
#endif
	uint pmc_pwrgate_toggle;	/* _PWRGATE_TOGGLE_0, offset 30 */
	uint pmc_remove_clamping;	/* _REMOVE_CLAMPING_CMD_0, offset 34 */
	uint pmc_pwrgate_status;	/* _PWRGATE_STATUS_0, offset 38 */
	uint pmc_pwrgood_timer;		/* _PWRGOOD_TIMER_0, offset 3C */
	uint pmc_blink_timer;		/* _BLINK_TIMER_0, offset 40 */
	uint pmc_no_iopower;		/* _NO_IOPOWER_0, offset 44 */
	uint pmc_pwr_det;		/* _PWR_DET_0, offset 48 */
	uint pmc_pwr_det_latch;		/* _PWR_DET_LATCH_0, offset 4C */

	uint pmc_scratch0;		/* _SCRATCH0_0, offset 50 */
	uint pmc_scratch1;		/* _SCRATCH1_0, offset 54 */
	uint pmc_scratch2;		/* _SCRATCH2_0, offset 58 */
	uint pmc_scratch3;		/* _SCRATCH3_0, offset 5C */
	uint pmc_scratch4;		/* _SCRATCH4_0, offset 60 */
	uint pmc_scratch5;		/* _SCRATCH5_0, offset 64 */
	uint pmc_scratch6;		/* _SCRATCH6_0, offset 68 */
	uint pmc_scratch7;		/* _SCRATCH7_0, offset 6C */
	uint pmc_scratch8;		/* _SCRATCH8_0, offset 70 */
	uint pmc_scratch9;		/* _SCRATCH9_0, offset 74 */
	uint pmc_scratch10;		/* _SCRATCH10_0, offset 78 */
	uint pmc_scratch11;		/* _SCRATCH11_0, offset 7C */
	uint pmc_scratch12;		/* _SCRATCH12_0, offset 80 */
	uint pmc_scratch13;		/* _SCRATCH13_0, offset 84 */
	uint pmc_scratch14;		/* _SCRATCH14_0, offset 88 */
	uint pmc_scratch15;		/* _SCRATCH15_0, offset 8C */
	uint pmc_scratch16;		/* _SCRATCH16_0, offset 90 */
	uint pmc_scratch17;		/* _SCRATCH17_0, offset 94 */
	uint pmc_scratch18;		/* _SCRATCH18_0, offset 98 */
	uint pmc_scratch19;		/* _SCRATCH19_0, offset 9C */
	uint pmc_scratch20;		/* _SCRATCH20_0, offset A0 */
	uint pmc_scratch21;		/* _SCRATCH21_0, offset A4 */
	uint pmc_scratch22;		/* _SCRATCH22_0, offset A8 */
	uint pmc_scratch23;		/* _SCRATCH23_0, offset AC */

	uint pmc_secure_scratch0;	/* _SECURE_SCRATCH0_0, offset B0 */
	uint pmc_secure_scratch1;	/* _SECURE_SCRATCH1_0, offset B4 */
	uint pmc_secure_scratch2;	/* _SECURE_SCRATCH2_0, offset B8 */
	uint pmc_secure_scratch3;	/* _SECURE_SCRATCH3_0, offset BC */
	uint pmc_secure_scratch4;	/* _SECURE_SCRATCH4_0, offset C0 */
	uint pmc_secure_scratch5;	/* _SECURE_SCRATCH5_0, offset C4 */

	uint pmc_cpupwrgood_timer;	/* _CPUPWRGOOD_TIMER_0, offset C8 */
	uint pmc_cpupwroff_timer;	/* _CPUPWROFF_TIMER_0, offset CC */
	uint pmc_pg_mask;		/* _PG_MASK_0, offset D0 */
	uint pmc_pg_mask_1;		/* _PG_MASK_1_0, offset D4 */
	uint pmc_auto_wake_lvl;		/* _AUTO_WAKE_LVL_0, offset D8 */
	uint pmc_auto_wake_lvl_mask;	/* _AUTO_WAKE_LVL_MASK_0, offset DC */
	uint pmc_wake_delay;		/* _WAKE_DELAY_0, offset E0 */
	uint pmc_pwr_det_val;		/* _PWR_DET_VAL_0, offset E4 */
	uint pmc_ddr_pwr;		/* _DDR_PWR_0, offset E8 */
	uint pmc_usb_debounce_del;	/* _USB_DEBOUNCE_DEL_0, offset EC */
	uint pmc_usb_ao;		/* _USB_AO_0, offset F0 */
	uint pmc_crypto_op;		/* _CRYPTO_OP__0, offset F4 */
	uint pmc_pllp_wb0_override;	/* _PLLP_WB0_OVERRIDE_0, offset F8 */

	uint pmc_scratch24;		/* _SCRATCH24_0, offset FC */
	uint pmc_scratch25;		/* _SCRATCH24_0, offset 100 */
	uint pmc_scratch26;		/* _SCRATCH24_0, offset 104 */
	uint pmc_scratch27;		/* _SCRATCH24_0, offset 108 */
	uint pmc_scratch28;		/* _SCRATCH24_0, offset 10C */
	uint pmc_scratch29;		/* _SCRATCH24_0, offset 110 */
	uint pmc_scratch30;		/* _SCRATCH24_0, offset 114 */
	uint pmc_scratch31;		/* _SCRATCH24_0, offset 118 */
	uint pmc_scratch32;		/* _SCRATCH24_0, offset 11C */
	uint pmc_scratch33;		/* _SCRATCH24_0, offset 120 */
	uint pmc_scratch34;		/* _SCRATCH24_0, offset 124 */
	uint pmc_scratch35;		/* _SCRATCH24_0, offset 128 */
	uint pmc_scratch36;		/* _SCRATCH24_0, offset 12C */
	uint pmc_scratch37;		/* _SCRATCH24_0, offset 130 */
	uint pmc_scratch38;		/* _SCRATCH24_0, offset 134 */
	uint pmc_scratch39;		/* _SCRATCH24_0, offset 138 */
	uint pmc_scratch40;		/* _SCRATCH24_0, offset 13C */
	uint pmc_scratch41;		/* _SCRATCH24_0, offset 140 */
	uint pmc_scratch42;		/* _SCRATCH24_0, offset 144 */

	uint pmc_bo_mirror0;		/* _BOUNDOUT_MIRROR0_0, offset 148 */
	uint pmc_bo_mirror1;		/* _BOUNDOUT_MIRROR1_0, offset 14C */
	uint pmc_bo_mirror2;		/* _BOUNDOUT_MIRROR2_0, offset 150 */
	uint pmc_sys_33v_en;		/* _SYS_33V_EN_0, offset 154 */
	uint pmc_bo_mirror_access;	/* _BOUNDOUT_MIRROR_ACCESS_0, off158 */
	uint pmc_gate;			/* _GATE_0, offset 15C */
	/* The following fields are in Tegra124 and later only */
	uint pmc_wake2_mask;		/* _WAKE2_MASK_0, offset 160 */
	uint pmc_wake2_lvl;		/* _WAKE2_LVL_0,  offset 164 */
	uint pmc_wake2_stat;		/* _WAKE2_STATUS_0, offset 168 */
	uint pmc_sw_wake2_stat;		/* _SW_WAKE2_STATUS_0, offset 16C */
	uint pmc_auto_wake2_lvl_mask;	/* _AUTO_WAKE2_LVL_MASK_0, offset 170 */
	uint pmc_pg_mask2;		/* _PG_MASK_2_0, offset 174 */
	uint pmc_pg_mask_ce1;		/* _PG_MASK_CE1_0, offset 178 */
	uint pmc_pg_mask_ce2;		/* _PG_MASK_CE2_0, offset 17C */
	uint pmc_pg_mask_ce3;		/* _PG_MASK_CE3_0, offset 180 */
	uint pmc_pwrgate_timer_ce0;	/* _PWRGATE_TIMER_CE_0_0, offset 184 */
	uint pmc_pwrgate_timer_ce1;	/* _PWRGATE_TIMER_CE_1_0, offset 188 */
	uint pmc_pwrgate_timer_ce2;	/* _PWRGATE_TIMER_CE_2_0, offset 18C */
	uint pmc_pwrgate_timer_ce3;	/* _PWRGATE_TIMER_CE_3_0, offset 190 */
	uint pmc_pwrgate_timer_ce4;	/* _PWRGATE_TIMER_CE_4_0, offset 194 */
	uint pmc_pwrgate_timer_ce5;	/* _PWRGATE_TIMER_CE_5_0, offset 198 */
	uint pmc_pwrgate_timer_ce6;	/* _PWRGATE_TIMER_CE_6_0, offset 19C */
	uint pmc_pcx_edpd_cntrl;	/* _PCX_EDPD_CNTRL_0, offset 1A0 */
	uint pmc_osc_edpd_over;		/* _OSC_EDPD_OVER_0, offset 1A4 */
	uint pmc_clk_out_cntrl;		/* _CLK_OUT_CNTRL_0, offset 1A8 */
	uint pmc_sata_pwrgate;		/* _SATA_PWRGT_0, offset 1AC */
	uint pmc_sensor_ctrl;		/* _SENSOR_CTRL_0, offset 1B0 */
	uint pmc_reset_status;		/* _RTS_STATUS_0, offset 1B4 */
	uint pmc_io_dpd_req;		/* _IO_DPD_REQ_0, offset 1B8 */
	uint pmc_io_dpd_stat;		/* _IO_DPD_STATUS_0, offset 1BC */
	uint pmc_io_dpd2_req;		/* _IO_DPD2_REQ_0, offset 1C0 */
	uint pmc_io_dpd2_stat;		/* _IO_DPD2_STATUS_0, offset 1C4 */
	uint pmc_sel_dpd_tim;		/* _SEL_DPD_TIM_0, offset 1C8 */
	uint pmc_vddp_sel;		/* _VDDP_SEL_0, offset 1CC */

	uint pmc_ddr_cfg;		/* _DDR_CFG_0, offset 1D0 */
	uint pmc_e_no_vttgen;		/* _E_NO_VTTGEN_0, offset 1D4 */
	uint pmc_reserved0;		/* _RESERVED, offset 1D8 */
	uint pmc_pllm_wb0_ovrride_frq;	/* _PLLM_WB0_OVERRIDE_FREQ_0, off 1DC */
	uint pmc_test_pwrgate;		/* _TEST_PWRGATE_0, offset 1E0 */
	uint pmc_pwrgate_timer_mult;	/* _PWRGATE_TIMER_MULT_0, offset 1E4 */
	uint pmc_dsi_sel_dpd;		/* _DSI_SEL_DPD_0, offset 1E8 */
	uint pmc_utmip_uhsic_triggers;	/* _UTMIP_UHSIC_TRIGGERS_0, off 1EC */
	uint pmc_utmip_uhsic_saved_st;  /* _UTMIP_UHSIC_SAVED_STATE_0, off1F0 */
	uint pmc_utmip_pad_cfg;		/* _UTMIP_PAD_CFG_0, offset 1F4 */
	uint pmc_utmip_term_pad_cfg;	/* _UTMIP_TERM_PAD_CFG_0, offset 1F8 */
	uint pmc_utmip_uhsic_sleep_cfg;	/* _UTMIP_UHSIC_SLEEP_CFG_0, off 1FC */

	uint pmc_todo_0[9];		/* offset 200-220 */
	uint pmc_secure_scratch6;	/* _SECURE_SCRATCH6_0, offset 224 */
	uint pmc_secure_scratch7;	/* _SECURE_SCRATCH7_0, offset 228 */
	uint pmc_scratch43;		/* _SCRATCH43_0, offset 22C */
	uint pmc_scratch44;		/* _SCRATCH44_0, offset 230 */
	uint pmc_scratch45;
	uint pmc_scratch46;
	uint pmc_scratch47;
	uint pmc_scratch48;
	uint pmc_scratch49;
	uint pmc_scratch50;
	uint pmc_scratch51;
	uint pmc_scratch52;
	uint pmc_scratch53;
	uint pmc_scratch54;
	uint pmc_scratch55;		/* _SCRATCH55_0, offset 25C */
	uint pmc_scratch0_eco;		/* _SCRATCH0_ECO_0, offset 260 */
	uint pmc_por_dpd_ctrl;		/* _POR_DPD_CTRL_0, offset 264 */
	uint pmc_scratch2_eco;		/* _SCRATCH2_ECO_0, offset 268 */
	uint pmc_todo_1[17];		/* TODO: 26C ~ 2AC */
	uint pmc_pllm_wb0_override2;	/* _PLLM_WB0_OVERRIDE2, offset 2B0 */
	uint pmc_tsc_mult;		/* _TSC_MULT_0, offset 2B4 */
	uint pmc_cpu_vsense_override;	/* _CPU_VSENSE_OVERRIDE_0, offset 2B8 */
	uint pmc_glb_amap_cfg;		/* _GLB_AMAP_CFG_0, offset 2BC */
	uint pmc_sticky_bits;		/* _STICKY_BITS_0, offset 2C0 */
	uint pmc_sec_disable2;		/* _SEC_DISALBE2, offset 2C4 */
	uint pmc_weak_bias;		/* _WEAK_BIAS_0, offset 2C8 */
	uint pmc_todo_3[13];		/* TODO: 2CC ~ 2FC */
	uint pmc_secure_scratch8;	/* _SECURE_SCRATCH8_0, offset 300 */
	uint pmc_secure_scratch9;
	uint pmc_secure_scratch10;
	uint pmc_secure_scratch11;
	uint pmc_secure_scratch12;
	uint pmc_secure_scratch13;
	uint pmc_secure_scratch14;
	uint pmc_secure_scratch15;
	uint pmc_secure_scratch16;
	uint pmc_secure_scratch17;
	uint pmc_secure_scratch18;
	uint pmc_secure_scratch19;
	uint pmc_secure_scratch20;
	uint pmc_secure_scratch21;
	uint pmc_secure_scratch22;
	uint pmc_secure_scratch23;
	uint pmc_secure_scratch24;	/* _SECURE_SCRATCH24_0, offset 340 */
	uint pmc_secure_scratch25;
	uint pmc_secure_scratch26;
	uint pmc_secure_scratch27;
	uint pmc_secure_scratch28;
	uint pmc_secure_scratch29;
	uint pmc_secure_scratch30;
	uint pmc_secure_scratch31;
	uint pmc_secure_scratch32;
	uint pmc_secure_scratch33;
	uint pmc_secure_scratch34;
	uint pmc_secure_scratch35;	/* _SECURE_SCRATCH35_0, offset 36C */

	uint pmc_reserved1[52];		/* RESERVED: 370 ~ 43C */
	uint pmc_cntrl2;		/* _CNTRL2_0, offset 440 */
	uint pmc_reserved2[6];		/* RESERVED: 444 ~ 458 */
	uint pmc_io_dpd3_req;		/* _IO_DPD3_REQ_0, offset 45c */
	uint pmc_io_dpd3_stat;		/* _IO_DPD3_STATUS_0, offset 460 */
	uint pmc_strap_opt_a;		/* _STRAPPING_OPT_A_0, offset 464 */
	uint pmc_reserved3[102];	/* RESERVED: 468 ~ 5FC */

	uint pmc_scratch56;		/* _SCRATCH56_0, offset 600 */
	uint pmc_scratch57;
	uint pmc_scratch58;
	uint pmc_scratch59;
	uint pmc_scratch60;
	uint pmc_scratch61;
	uint pmc_scratch62;
	uint pmc_scratch63;
	uint pmc_scratch64;
	uint pmc_scratch65;
	uint pmc_scratch66;
	uint pmc_scratch67;
	uint pmc_scratch68;
	uint pmc_scratch69;
	uint pmc_scratch70;
	uint pmc_scratch71;
	uint pmc_scratch72;
	uint pmc_scratch73;
	uint pmc_scratch74;
	uint pmc_scratch75;
	uint pmc_scratch76;
	uint pmc_scratch77;
	uint pmc_scratch78;
	uint pmc_scratch79;
	uint pmc_scratch80;
	uint pmc_scratch81;
	uint pmc_scratch82;
	uint pmc_scratch83;
	uint pmc_scratch84;
	uint pmc_scratch85;
	uint pmc_scratch86;
	uint pmc_scratch87;
	uint pmc_scratch88;
	uint pmc_scratch89;
	uint pmc_scratch90;
	uint pmc_scratch91;
	uint pmc_scratch92;
	uint pmc_scratch93;
	uint pmc_scratch94;
	uint pmc_scratch95;
	uint pmc_scratch96;
	uint pmc_scratch97;
	uint pmc_scratch98;
	uint pmc_scratch99;
	uint pmc_scratch100;
	uint pmc_scratch101;
	uint pmc_scratch102;
	uint pmc_scratch103;
	uint pmc_scratch104;
	uint pmc_scratch105;
	uint pmc_scratch106;
	uint pmc_scratch107;
	uint pmc_scratch108;
	uint pmc_scratch109;
	uint pmc_scratch110;
	uint pmc_scratch111;
	uint pmc_scratch112;
	uint pmc_scratch113;
	uint pmc_scratch114;
	uint pmc_scratch115;
	uint pmc_scratch116;
	uint pmc_scratch117;
	uint pmc_scratch118;
	uint pmc_scratch119;
	uint pmc_scratch1_eco;	/* offset 700 */
};

#define CPU_PWRED	1
#define CPU_CLMP	1

#define PARTID_CP	0xFFFFFFF8
#define START_CP	(1 << 8)

#define CPUPWRREQ_OE	(1 << 16)
#define CPUPWRREQ_POL	(1 << 15)

#define CRAIL		0
#define CE0		14
#define C0NC		15
#define SOR		17

#define PMC_XOFS_SHIFT	1
#define PMC_XOFS_MASK	(0x3F << PMC_XOFS_SHIFT)

#if defined(CONFIG_TEGRA114)
#define TIMER_MULT_SHIFT	0
#define TIMER_MULT_MASK		(3 << TIMER_MULT_SHIFT)
#define TIMER_MULT_CPU_SHIFT	2
#define TIMER_MULT_CPU_MASK	(3 << TIMER_MULT_CPU_SHIFT)
#elif defined(CONFIG_TEGRA124) || defined(CONFIG_TEGRA210)
#define TIMER_MULT_SHIFT	0
#define TIMER_MULT_MASK		(7 << TIMER_MULT_SHIFT)
#define TIMER_MULT_CPU_SHIFT	3
#define TIMER_MULT_CPU_MASK	(7 << TIMER_MULT_CPU_SHIFT)
#endif

#define MULT_1			0
#define MULT_2			1
#define MULT_4			2
#define MULT_8			3
#if defined(CONFIG_TEGRA124) || defined(CONFIG_TEGRA210)
#define MULT_16			4
#endif

#define AMAP_WRITE_SHIFT	20
#define AMAP_WRITE_ON		(1 << AMAP_WRITE_SHIFT)

/* SEC_DISABLE_0, 0x04 */
#define SEC_DISABLE_WRITE0_ON			(1 << 4)
#define SEC_DISABLE_READ0_ON			(1 << 5)
#define SEC_DISABLE_WRITE1_ON			(1 << 6)
#define SEC_DISABLE_READ1_ON			(1 << 7)
#define SEC_DISABLE_WRITE2_ON			(1 << 8)
#define SEC_DISABLE_READ2_ON			(1 << 9)
#define SEC_DISABLE_WRITE3_ON			(1 << 10)
#define SEC_DISABLE_READ3_ON			(1 << 11)
#define SEC_DISABLE_AMAP_WRITE_ON		(1 << 20)

/* APBDEV_PMC_PWRGATE_TOGGLE_0 0x30 */
#define PWRGATE_TOGGLE_PARTID_CRAIL		0
#define PWRGATE_TOGGLE_PARTID_TD		1
#define PWRGATE_TOGGLE_PARTID_VE		2
#define PWRGATE_TOGGLE_PARTID_PCX		3
#define PWRGATE_TOGGLE_PARTID_VDE		4
#define PWRGATE_TOGGLE_PARTID_L2C		5
#define PWRGATE_TOGGLE_PARTID_MPE		6
#define PWRGATE_TOGGLE_PARTID_HEG		7
#define PWRGATE_TOGGLE_PARTID_SAX		8
#define PWRGATE_TOGGLE_PARTID_CE1		9
#define PWRGATE_TOGGLE_PARTID_CE2		10
#define PWRGATE_TOGGLE_PARTID_CE3		11
#define PWRGATE_TOGGLE_PARTID_CELP		12
#define PWRGATE_TOGGLE_PARTID_CE0		14
#define PWRGATE_TOGGLE_PARTID_C0NC		15
#define PWRGATE_TOGGLE_PARTID_C1NC		16
#define PWRGATE_TOGGLE_PARTID_SOR		17
#define PWRGATE_TOGGLE_PARTID_DIS		18
#define PWRGATE_TOGGLE_PARTID_DISB		19
#define PWRGATE_TOGGLE_PARTID_XUSBA		20
#define PWRGATE_TOGGLE_PARTID_XUSBB		21
#define PWRGATE_TOGGLE_PARTID_XUSBC		22
#define PWRGATE_TOGGLE_PARTID_VIC		23
#define PWRGATE_TOGGLE_PARTID_IRAM		24
#define PWRGATE_TOGGLE_START			(1 << 8)

/* APBDEV_PMC_PWRGATE_STATUS_0 0x38 */
#define PWRGATE_STATUS_CRAIL_ENABLE		(1 << 0)
#define PWRGATE_STATUS_TD_ENABLE		(1 << 1)
#define PWRGATE_STATUS_VE_ENABLE		(1 << 2)
#define PWRGATE_STATUS_PCX_ENABLE		(1 << 3)
#define PWRGATE_STATUS_VDE_ENABLE		(1 << 4)
#define PWRGATE_STATUS_L2C_ENABLE		(1 << 5)
#define PWRGATE_STATUS_MPE_ENABLE		(1 << 6)
#define PWRGATE_STATUS_HEG_ENABLE		(1 << 7)
#define PWRGATE_STATUS_SAX_ENABLE		(1 << 8)
#define PWRGATE_STATUS_CE1_ENABLE		(1 << 9)
#define PWRGATE_STATUS_CE2_ENABLE		(1 << 10)
#define PWRGATE_STATUS_CE3_ENABLE		(1 << 11)
#define PWRGATE_STATUS_CELP_ENABLE		(1 << 12)
#define PWRGATE_STATUS_CE0_ENABLE		(1 << 14)
#define PWRGATE_STATUS_C0NC_ENABLE		(1 << 15)
#define PWRGATE_STATUS_C1NC_ENABLE		(1 << 16)
#define PWRGATE_STATUS_SOR_ENABLE		(1 << 17)
#define PWRGATE_STATUS_DIS_ENABLE		(1 << 18)
#define PWRGATE_STATUS_DISB_ENABLE		(1 << 19)
#define PWRGATE_STATUS_XUSBA_ENABLE		(1 << 20)
#define PWRGATE_STATUS_XUSBB_ENABLE		(1 << 21)
#define PWRGATE_STATUS_XUSBC_ENABLE		(1 << 22)
#define PWRGATE_STATUS_VIC_ENABLE		(1 << 23)
#define PWRGATE_STATUS_IRAM_ENABLE		(1 << 24)

/* APBDEV_PMC_CNTRL2_0 0x440 */
#define HOLD_CKE_LOW_EN				(1 << 12)

/* PMC read/write functions */
u32 tegra_pmc_readl(unsigned long offset);
void tegra_pmc_writel(u32 value, unsigned long offset);

#define PMC_CNTRL		0x0
#define  PMC_CNTRL_MAIN_RST	BIT(4)

#if IS_ENABLED(CONFIG_TEGRA186)
#  define PMC_SCRATCH0 0x32000
#else
#  define PMC_SCRATCH0 0x00050
#endif

/* for secure PMC */
#define TEGRA_SMC_PMC		0xc2fffe00
#define  TEGRA_SMC_PMC_READ	0xaa
#define  TEGRA_SMC_PMC_WRITE	0xbb

#endif	/* PMC_H */
