/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * clock.h
 *
 * clock header
 *
 * Copyright (C) 2011, Texas Instruments Incorporated - http://www.ti.com/
 */

#ifndef _CLOCKS_H_
#define _CLOCKS_H_

#include <asm/arch/clocks_am33xx.h>
#include <asm/arch/hardware.h>

#if defined(CONFIG_TI816X) || defined(CONFIG_TI814X)
#include <asm/arch/clock_ti81xx.h>
#endif

#define LDELAY 1000000

/*CM_<clock_domain>__CLKCTRL */
#define CD_CLKCTRL_CLKTRCTRL_SHIFT		0
#define CD_CLKCTRL_CLKTRCTRL_MASK		3

#define CD_CLKCTRL_CLKTRCTRL_NO_SLEEP		0
#define CD_CLKCTRL_CLKTRCTRL_SW_SLEEP		1
#define CD_CLKCTRL_CLKTRCTRL_SW_WKUP		2

/* CM_<clock_domain>_<module>_CLKCTRL */
#define MODULE_CLKCTRL_MODULEMODE_SHIFT		0
#define MODULE_CLKCTRL_MODULEMODE_MASK		3
#define MODULE_CLKCTRL_IDLEST_SHIFT		16
#define MODULE_CLKCTRL_IDLEST_MASK		(3 << 16)

#define MODULE_CLKCTRL_MODULEMODE_SW_DISABLE		0
#define MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN	2

#define MODULE_CLKCTRL_IDLEST_FULLY_FUNCTIONAL	0
#define MODULE_CLKCTRL_IDLEST_TRANSITIONING	1
#define MODULE_CLKCTRL_IDLEST_IDLE		2
#define MODULE_CLKCTRL_IDLEST_DISABLED		3

/* CM_CLKMODE_DPLL */
#define CM_CLKMODE_DPLL_SSC_EN_SHIFT		12
#define CM_CLKMODE_DPLL_SSC_EN_MASK		(1 << 12)
#define CM_CLKMODE_DPLL_SSC_ACK_MASK		(1 << 13)
#define CM_CLKMODE_DPLL_SSC_DOWNSPREAD_MASK	(1 << 14)
#define CM_CLKMODE_DPLL_SSC_TYPE_MASK		(1 << 15)
#define CM_CLKMODE_DPLL_REGM4XEN_SHIFT		11
#define CM_CLKMODE_DPLL_REGM4XEN_MASK		(1 << 11)
#define CM_CLKMODE_DPLL_LPMODE_EN_SHIFT		10
#define CM_CLKMODE_DPLL_LPMODE_EN_MASK		(1 << 10)
#define CM_CLKMODE_DPLL_RELOCK_RAMP_EN_SHIFT	9
#define CM_CLKMODE_DPLL_RELOCK_RAMP_EN_MASK	(1 << 9)
#define CM_CLKMODE_DPLL_DRIFTGUARD_EN_SHIFT	8
#define CM_CLKMODE_DPLL_DRIFTGUARD_EN_MASK	(1 << 8)
#define CM_CLKMODE_DPLL_RAMP_RATE_SHIFT		5
#define CM_CLKMODE_DPLL_RAMP_RATE_MASK		(0x7 << 5)
#define CM_CLKMODE_DPLL_EN_SHIFT		0
#define CM_CLKMODE_DPLL_EN_MASK			(0x7 << 0)

#define CM_CLKMODE_DPLL_DPLL_EN_SHIFT		0
#define CM_CLKMODE_DPLL_DPLL_EN_MASK		7

#define DPLL_EN_STOP			1
#define DPLL_EN_MN_BYPASS		4
#define DPLL_EN_LOW_POWER_BYPASS	5
#define DPLL_EN_LOCK			7

/* CM_IDLEST_DPLL fields */
#define ST_DPLL_CLK_MASK		1

/* CM_CLKSEL_DPLL */
#define CM_CLKSEL_DPLL_M_SHIFT			8
#define CM_CLKSEL_DPLL_M_MASK			(0x7FF << 8)
#define CM_CLKSEL_DPLL_N_SHIFT			0
#define CM_CLKSEL_DPLL_N_MASK			0x7F

struct dpll_params {
	u32 m;
	u32 n;
	s8 m2;
	s8 m3;
	s8 m4;
	s8 m5;
	s8 m6;
};

struct dpll_regs {
	u32 cm_clkmode_dpll;
	u32 cm_idlest_dpll;
	u32 cm_autoidle_dpll;
	u32 cm_clksel_dpll;
	u32 cm_div_m2_dpll;
	u32 cm_div_m3_dpll;
	u32 cm_div_m4_dpll;
	u32 cm_div_m5_dpll;
	u32 cm_div_m6_dpll;
};

extern const struct dpll_regs dpll_mpu_regs;
extern const struct dpll_regs dpll_core_regs;
extern const struct dpll_regs dpll_per_regs;
extern const struct dpll_regs dpll_ddr_regs;
extern const struct dpll_regs dpll_disp_regs;
extern const struct dpll_params dpll_mpu_opp[NUM_CRYSTAL_FREQ][NUM_OPPS];
extern const struct dpll_params dpll_core_1000MHz[NUM_CRYSTAL_FREQ];
extern const struct dpll_params dpll_per_192MHz[NUM_CRYSTAL_FREQ];
extern const struct dpll_params dpll_ddr2_266MHz[NUM_CRYSTAL_FREQ];
extern const struct dpll_params dpll_ddr3_303MHz[NUM_CRYSTAL_FREQ];
extern const struct dpll_params dpll_ddr3_400MHz[NUM_CRYSTAL_FREQ];

extern struct cm_wkuppll *const cmwkup;

const struct dpll_params *get_dpll_mpu_params(void);
const struct dpll_params *get_dpll_core_params(void);
const struct dpll_params *get_dpll_per_params(void);
const struct dpll_params *get_dpll_ddr_params(void);
void scale_vcores(void);
void do_setup_dpll(const struct dpll_regs *, const struct dpll_params *);
void prcm_init(void);
void enable_basic_clocks(void);

void rtc_only_update_board_type(u32 btype);
u32 rtc_only_get_board_type(void);
void rtc_only_prcm_init(void);
void rtc_only_enable_basic_clocks(void);

void do_enable_clocks(u32 *const *, u32 *const *, u8);
void do_disable_clocks(u32 *const *, u32 *const *, u8);

void set_mpu_spreadspectrum(int permille);
#endif
