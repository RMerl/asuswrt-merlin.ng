/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * keystone2: common clock header file
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef __ASM_ARCH_CLOCK_H
#define __ASM_ARCH_CLOCK_H

#ifndef __ASSEMBLY__

#ifdef CONFIG_SOC_K2HK
#include <asm/arch/clock-k2hk.h>
#endif

#ifdef CONFIG_SOC_K2E
#include <asm/arch/clock-k2e.h>
#endif

#ifdef CONFIG_SOC_K2L
#include <asm/arch/clock-k2l.h>
#endif

#ifdef CONFIG_SOC_K2G
#include <asm/arch/clock-k2g.h>
#endif

#define CORE_PLL MAIN_PLL
#define DDR3_PLL DDR3A_PLL
#define NSS_PLL PASS_PLL

#define CLK_LIST(CLK)\
	CLK(0, core_pll_clk)\
	CLK(1, pass_pll_clk)\
	CLK(2, tetris_pll_clk)\
	CLK(3, ddr3a_pll_clk)\
	CLK(4, ddr3b_pll_clk)\
	CLK(5, sys_clk0_clk)\
	CLK(6, sys_clk0_1_clk)\
	CLK(7, sys_clk0_2_clk)\
	CLK(8, sys_clk0_3_clk)\
	CLK(9, sys_clk0_4_clk)\
	CLK(10, sys_clk0_6_clk)\
	CLK(11, sys_clk0_8_clk)\
	CLK(12, sys_clk0_12_clk)\
	CLK(13, sys_clk0_24_clk)\
	CLK(14, sys_clk1_clk)\
	CLK(15, sys_clk1_3_clk)\
	CLK(16, sys_clk1_4_clk)\
	CLK(17, sys_clk1_6_clk)\
	CLK(18, sys_clk1_12_clk)\
	CLK(19, sys_clk2_clk)\
	CLK(20, sys_clk3_clk)\
	CLK(21, uart_pll_clk)

#include <asm/types.h>

#define GENERATE_ENUM(NUM, ENUM) ENUM = NUM,
#define GENERATE_INDX_STR(NUM, STRING) #NUM"\t- "#STRING"\n"
#define CLOCK_INDEXES_LIST	CLK_LIST(GENERATE_INDX_STR)

enum {
	SPD200,
	SPD400,
	SPD600,
	SPD800,
	SPD850,
	SPD900,
	SPD1000,
	SPD1200,
	SPD1250,
	SPD1350,
	SPD1400,
	SPD1500,
	NUM_SPDS,
};

/* PLL identifiers */
enum {
	MAIN_PLL,
	TETRIS_PLL,
	PASS_PLL,
	DDR3A_PLL,
	DDR3B_PLL,
	UART_PLL,
	MAX_PLL_COUNT,
};

enum ext_clk_e {
	sys_clk,
	alt_core_clk,
	pa_clk,
	tetris_clk,
	ddr3a_clk,
	ddr3b_clk,
	uart_clk,
	ext_clk_count /* number of external clocks */
};

enum clk_e {
	CLK_LIST(GENERATE_ENUM)
};

struct keystone_pll_regs {
	u32 reg0;
	u32 reg1;
};

/* PLL configuration data */
struct pll_init_data {
	int pll;
	int pll_m;		/* PLL Multiplier */
	int pll_d;		/* PLL divider */
	int pll_od;		/* PLL output divider */
};

extern const struct keystone_pll_regs keystone_pll_regs[];
extern s16 divn_val[];
extern int speeds[];

void init_plls(void);
void init_pll(const struct pll_init_data *data);
struct pll_init_data *get_pll_init_data(int pll);
unsigned long ks_clk_get_rate(unsigned int clk);
int get_max_dev_speed(int *spds);
int get_max_arm_speed(int *spds);
void pll_pa_clk_sel(void);
unsigned int get_external_clk(u32 clk);

#endif
#endif
