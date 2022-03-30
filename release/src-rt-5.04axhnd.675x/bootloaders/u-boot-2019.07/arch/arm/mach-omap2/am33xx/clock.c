// SPDX-License-Identifier: GPL-2.0+
/*
 * clock.c
 *
 * Clock initialization for AM33XX boards.
 * Derived from OMAP4 boards
 *
 * Copyright (C) 2013, Texas Instruments, Incorporated - http://www.ti.com/
 */
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clock.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

static void setup_post_dividers(const struct dpll_regs *dpll_regs,
			 const struct dpll_params *params)
{
	/* Setup post-dividers */
	if (params->m2 >= 0)
		writel(params->m2, dpll_regs->cm_div_m2_dpll);
	if (params->m3 >= 0)
		writel(params->m3, dpll_regs->cm_div_m3_dpll);
	if (params->m4 >= 0)
		writel(params->m4, dpll_regs->cm_div_m4_dpll);
	if (params->m5 >= 0)
		writel(params->m5, dpll_regs->cm_div_m5_dpll);
	if (params->m6 >= 0)
		writel(params->m6, dpll_regs->cm_div_m6_dpll);
}

static inline void do_lock_dpll(const struct dpll_regs *dpll_regs)
{
	clrsetbits_le32(dpll_regs->cm_clkmode_dpll,
			CM_CLKMODE_DPLL_DPLL_EN_MASK,
			DPLL_EN_LOCK << CM_CLKMODE_DPLL_EN_SHIFT);
}

static inline void wait_for_lock(const struct dpll_regs *dpll_regs)
{
	if (!wait_on_value(ST_DPLL_CLK_MASK, ST_DPLL_CLK_MASK,
			   (void *)dpll_regs->cm_idlest_dpll, LDELAY)) {
		printf("DPLL locking failed for 0x%x\n",
		       dpll_regs->cm_clkmode_dpll);
		hang();
	}
}

static inline void do_bypass_dpll(const struct dpll_regs *dpll_regs)
{
	clrsetbits_le32(dpll_regs->cm_clkmode_dpll,
			CM_CLKMODE_DPLL_DPLL_EN_MASK,
			DPLL_EN_MN_BYPASS << CM_CLKMODE_DPLL_EN_SHIFT);
}

static inline void wait_for_bypass(const struct dpll_regs *dpll_regs)
{
	if (!wait_on_value(ST_DPLL_CLK_MASK, 0,
			   (void *)dpll_regs->cm_idlest_dpll, LDELAY)) {
		printf("Bypassing DPLL failed 0x%x\n",
		       dpll_regs->cm_clkmode_dpll);
	}
}

static void bypass_dpll(const struct dpll_regs *dpll_regs)
{
	do_bypass_dpll(dpll_regs);
	wait_for_bypass(dpll_regs);
}

void do_setup_dpll(const struct dpll_regs *dpll_regs,
		   const struct dpll_params *params)
{
	u32 temp;

	if (!params)
		return;

	temp = readl(dpll_regs->cm_clksel_dpll);

	bypass_dpll(dpll_regs);

	/* Set M & N */
	temp &= ~CM_CLKSEL_DPLL_M_MASK;
	temp |= (params->m << CM_CLKSEL_DPLL_M_SHIFT) & CM_CLKSEL_DPLL_M_MASK;

	temp &= ~CM_CLKSEL_DPLL_N_MASK;
	temp |= (params->n << CM_CLKSEL_DPLL_N_SHIFT) & CM_CLKSEL_DPLL_N_MASK;

	writel(temp, dpll_regs->cm_clksel_dpll);

	setup_post_dividers(dpll_regs, params);

	/* Wait till the DPLL locks */
	do_lock_dpll(dpll_regs);
	wait_for_lock(dpll_regs);
}

static void setup_dplls(void)
{
	const struct dpll_params *params;

	params = get_dpll_core_params();
	do_setup_dpll(&dpll_core_regs, params);

	params = get_dpll_mpu_params();
	do_setup_dpll(&dpll_mpu_regs, params);

	params = get_dpll_per_params();
	do_setup_dpll(&dpll_per_regs, params);
	writel(0x300, &cmwkup->clkdcoldodpllper);

	params = get_dpll_ddr_params();
	do_setup_dpll(&dpll_ddr_regs, params);
}

static inline void wait_for_clk_enable(u32 *clkctrl_addr)
{
	u32 clkctrl, idlest = MODULE_CLKCTRL_IDLEST_DISABLED;
	u32 bound = LDELAY;

	while ((idlest == MODULE_CLKCTRL_IDLEST_DISABLED) ||
		(idlest == MODULE_CLKCTRL_IDLEST_TRANSITIONING)) {
		clkctrl = readl(clkctrl_addr);
		idlest = (clkctrl & MODULE_CLKCTRL_IDLEST_MASK) >>
			 MODULE_CLKCTRL_IDLEST_SHIFT;
		if (--bound == 0) {
			printf("Clock enable failed for 0x%p idlest 0x%x\n",
			       clkctrl_addr, clkctrl);
			return;
		}
	}
}

static inline void enable_clock_module(u32 *const clkctrl_addr, u32 enable_mode,
				       u32 wait_for_enable)
{
	clrsetbits_le32(clkctrl_addr, MODULE_CLKCTRL_MODULEMODE_MASK,
			enable_mode << MODULE_CLKCTRL_MODULEMODE_SHIFT);
	debug("Enable clock module - %p\n", clkctrl_addr);
	if (wait_for_enable)
		wait_for_clk_enable(clkctrl_addr);
}

static inline void wait_for_clk_disable(u32 *clkctrl_addr)
{
	u32 clkctrl, idlest = MODULE_CLKCTRL_IDLEST_FULLY_FUNCTIONAL;
	u32 bound = LDELAY;

	while ((idlest != MODULE_CLKCTRL_IDLEST_DISABLED)) {
		clkctrl = readl(clkctrl_addr);
		idlest = (clkctrl & MODULE_CLKCTRL_IDLEST_MASK) >>
			  MODULE_CLKCTRL_IDLEST_SHIFT;
		if (--bound == 0) {
			printf("Clock disable failed for 0x%p idlest 0x%x\n",
			       clkctrl_addr, clkctrl);
			 return;
		}
	}
}
static inline void disable_clock_module(u32 *const clkctrl_addr,
					u32 wait_for_disable)
{
	clrsetbits_le32(clkctrl_addr, MODULE_CLKCTRL_MODULEMODE_MASK,
			MODULE_CLKCTRL_MODULEMODE_SW_DISABLE <<
			MODULE_CLKCTRL_MODULEMODE_SHIFT);
	debug("Disable clock module - %p\n", clkctrl_addr);
	if (wait_for_disable)
		wait_for_clk_disable(clkctrl_addr);
}

static inline void enable_clock_domain(u32 *const clkctrl_reg, u32 enable_mode)
{
	clrsetbits_le32(clkctrl_reg, CD_CLKCTRL_CLKTRCTRL_MASK,
			enable_mode << CD_CLKCTRL_CLKTRCTRL_SHIFT);
	debug("Enable clock domain - %p\n", clkctrl_reg);
}

static inline void disable_clock_domain(u32 *const clkctrl_reg)
{
	clrsetbits_le32(clkctrl_reg, CD_CLKCTRL_CLKTRCTRL_MASK,
			CD_CLKCTRL_CLKTRCTRL_SW_SLEEP <<
			CD_CLKCTRL_CLKTRCTRL_SHIFT);
	debug("Disable clock domain - %p\n", clkctrl_reg);
}

void do_enable_clocks(u32 *const *clk_domains,
		      u32 *const *clk_modules_explicit_en, u8 wait_for_enable)
{
	u32 i, max = 100;

	/* Put the clock domains in SW_WKUP mode */
	for (i = 0; (i < max) && clk_domains[i]; i++) {
		enable_clock_domain(clk_domains[i],
				    CD_CLKCTRL_CLKTRCTRL_SW_WKUP);
	}

	/* Clock modules that need to be put in SW_EXPLICIT_EN mode */
	for (i = 0; (i < max) && clk_modules_explicit_en[i]; i++) {
		enable_clock_module(clk_modules_explicit_en[i],
				    MODULE_CLKCTRL_MODULEMODE_SW_EXPLICIT_EN,
				    wait_for_enable);
	};
}

void do_disable_clocks(u32 *const *clk_domains,
			u32 *const *clk_modules_disable,
			u8 wait_for_disable)
{
	u32 i, max = 100;


	/* Clock modules that need to be put in SW_DISABLE */
	for (i = 0; (i < max) && clk_modules_disable[i]; i++)
		disable_clock_module(clk_modules_disable[i],
				     wait_for_disable);

	/* Put the clock domains in SW_SLEEP mode */
	for (i = 0; (i < max) && clk_domains[i]; i++)
		disable_clock_domain(clk_domains[i]);
}

/*
 * Before scaling up the clocks we need to have the PMIC scale up the
 * voltages first.  This will be dependent on which PMIC is in use
 * and in some cases we may not be scaling things up at all and thus not
 * need to do anything here.
 */
__weak void scale_vcores(void)
{
}

void setup_early_clocks(void)
{
	setup_clocks_for_console();
	enable_basic_clocks();
	timer_init();
}

void prcm_init(void)
{
	scale_vcores();
	setup_dplls();
}

void rtc_only_prcm_init(void)
{
	const struct dpll_params *params;

	rtc_only_enable_basic_clocks();

	params = get_dpll_ddr_params();
	do_setup_dpll(&dpll_ddr_regs, params);
}
