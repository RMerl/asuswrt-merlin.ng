/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <asm/arch/pmc.h>
#include <asm/arch/cpu.h>
#include <asm/arch/misc.h>
#include <spl.h>
#include "bcmbca-dtsetup.h"
#include "pmc_drv.h"
#include "pmc_a9_core.h"
#if !defined(CONFIG_TPL_ATF)
#include <asm/armv7.h>
#include <asm/pl310.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define SINGLE_CORE_63132 0x63132
int set_cpu_freq(int freqMHz);

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
void bcm_setsw(void)
{
	/* set the internal voltage regulator gain to 8. This reduces the response time and keeps
	   voltage supply to ddr stable. */
	swr_write(2, 0 ,0x800);
}

static void enable_ubus_fast_ack(void)
{
	ARMAIPCTRL->cfg |= AIP_CTRL_CFG_WR_FAST_ACK_MASK;
}
#endif

void boost_cpu_clock(void)
{
	printf("set cpu freq to 1000MHz\n");
	set_cpu_freq(1000);
}

int set_cpu_freq(int freqMHz)
{
	int mdiv = 0;
	int policy = 0;

	if (freqMHz < 200 || freqMHz > 1000) {
		printf("invalid cpu frequency %d\n", freqMHz);
		return -1;
	}

	/* enable write access the arm clk mananger */
	ARMCFG->proc_clk.wr_access |=
	    (ARM_PROC_CLK_WR_ACCESS_PASSWORD <<
	     ARM_PROC_CLK_WR_ACCESS_PASSWORD_SHIFT) |
	    ARM_PROC_CLK_WR_ACCESS_CLKMGR_ACC;
	mdiv = 2000 / freqMHz;

	if (mdiv < 10) {	/* setting frequency between 200 MHz and 1 GHz */
		/* set clk divider and enable pll */
		int ndiv = 2 * 1000 / 25;	// ndiv based on 1GHz

		ARMCFG->proc_clk.pllarma =
		    (ARMCFG->proc_clk.
		     pllarma & ~(ARM_PROC_CLK_PLLARMA_PDIV_MASK |
				 ARM_PROC_CLK_PLLARMA_NDIV_MASK |
				 ARM_PROC_CLK_PLLARMA_PWRDWN_SWOVRRIDE_MASK)) |
		    (2 << ARM_PROC_CLK_PLLARMA_PDIV_SHIFT) | (ndiv <<
							      ARM_PROC_CLK_PLLARMA_NDIV_SHIFT)
		    | ARM_PROC_CLK_PLLARMA_SOFT_RESETB_N;

		/* wait for pll to lock */
		while ((ARMCFG->proc_clk.
			pllarma & ARM_PROC_CLK_PLLARMA_PLL_LOCK_RAW) == 0) ;

		/* enable post diveder */
		ARMCFG->proc_clk.pllarma |=
		    ARM_PROC_CLK_PLLARMA_SOFT_POST_RESETB_N;
	}

	/* set the freq policy */
	policy =
	    (freqMHz ==
	     200) ? ARM_PROC_CLK_POLICY_FREQ_SYSCLK :
	    ARM_PROC_CLK_POLICY_FREQ_ARMPLL_SLOW;
	ARMCFG->proc_clk.policy_freq =
	    (ARMCFG->proc_clk.
	     policy_freq & ~ARM_PROC_CLK_POLICY_FREQ_MASK) | (policy <<
							      ARM_PROC_CLK_POLICY3_FREQ_SHIFT)
	    | (policy << ARM_PROC_CLK_POLICY2_FREQ_SHIFT) | (policy <<
							     ARM_PROC_CLK_POLICY1_FREQ_SHIFT)
	    | (policy << ARM_PROC_CLK_POLICY0_FREQ_SHIFT);

	/* setting the mdiv */
	ARMCFG->proc_clk.pllarmc &= 0xffffff00;
	ARMCFG->proc_clk.pllarmc |= mdiv;
	ARMCFG->proc_clk.pllarmc |= 0x800;

	/* enabled hardware clock gating */
	ARMCFG->proc_clk.core0_clkgate =
	    (ARMCFG->proc_clk.
	     core0_clkgate & ~ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_MASK) |
	    (ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_HW <<
	     ARM_PROC_CLK_CORE0_CLKGATE_GATING_SEL_SHIFT);
	ARMCFG->proc_clk.core1_clkgate =
	    (ARMCFG->proc_clk.
	     core1_clkgate & ~ARM_PROC_CLK_CORE1_CLKGATE_GATING_SEL_MASK) |
	    (ARM_PROC_CLK_CORE1_CLKGATE_GATING_SEL_HW <<
	     ARM_PROC_CLK_CORE1_CLKGATE_GATING_SEL_SHIFT);
	ARMCFG->proc_clk.arm_switch_clkgate =
	    (ARMCFG->proc_clk.
	     arm_switch_clkgate & ~ARM_PROC_CLK_SWITCH_CLKGATE_GATING_SEL_MASK)
	    | (ARM_PROC_CLK_SWITCH_CLKGATE_GATING_SEL_HW <<
	       ARM_PROC_CLK_SWITCH_CLKGATE_GATING_SEL_SHIFT);
	ARMCFG->proc_clk.arm_periph_clkgate =
	    (ARMCFG->proc_clk.
	     arm_periph_clkgate & ~ARM_PROC_CLK_PERIPH_CLKGATE_GATING_SEL_MASK)
	    | (ARM_PROC_CLK_PERIPH_CLKGATE_GATING_SEL_HW <<
	       ARM_PROC_CLK_PERIPH_CLKGATE_GATING_SEL_SHIFT);
	ARMCFG->proc_clk.apb0_clkgate =
	    (ARMCFG->proc_clk.
	     apb0_clkgate & ~ARM_PROC_CLK_APB0_CLKGATE_GATING_SEL_MASK) |
	    (ARM_PROC_CLK_APB0_CLKGATE_GATING_SEL_HW <<
	     ARM_PROC_CLK_APB0_CLKGATE_GATING_SEL_SHIFT);

	/* enable the new freq policy */
	ARMCFG->proc_clk.policy_ctl |=
	    (ARM_PROC_CLK_POLICY_CTL_GO_AC | ARM_PROC_CLK_POLICY_CTL_GO);

	/* wait for policy to be activated */
	while (ARMCFG->proc_clk.policy_ctl & ARM_PROC_CLK_POLICY_CTL_GO) ;

	return 2000/mdiv;
}

int get_nr_cpus(void)
{
	unsigned int chipId = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
	int nr_cpus=DUAL_CPUS;

	if(chipId == SINGLE_CORE_63132)
		nr_cpus=ONE_CPU;

	return nr_cpus;
}

#if !defined(CONFIG_TPL_ATF)
void prepare_ns_boot(void)
{
	u32 value;
	struct pl310_regs *const pl310 = (struct pl310_regs *)CONFIG_SYS_PL310_BASE;

#define L2C_AUX_CONTROL_VAL (L310_AUX_CTRL_NS_LOCK_ENABLE |\
							 L310_AUX_CTRL_NS_INT_ENABLE |\
	                      	 L310_SHARED_ATT_OVERRIDE_ENABLE |\
							 L310_AUX_CTRL_EARLY_BRESP_ENABLE |\
							 L310_AUX_CTRL_WAY_SIZE_32KB |\
							 PL310_AUX_CTRL_ASSOCIATIVITY_MASK | 0x1)
#define L2C_AUX_CONTROL_MASK ~(L2C_AUX_CONTROL_VAL|L310_AUX_CTRL_WAY_SIZE_MASK)
	
	/* Update L2C AUX when in secure mode */
	value = readl(&pl310->pl310_aux_ctrl);
	value &= L2C_AUX_CONTROL_MASK;
	value |= L2C_AUX_CONTROL_VAL;
	writel(value, &pl310->pl310_aux_ctrl);

	/* invalidate l2 cache */
	v7_outer_cache_inval_all();

	/* enable l2c cache while in secure mode */
	setbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);
	
	/* set non-secure ACR. Allow SMP, L2ERR, CP10 and CP11 and 
	   Enable Neon/VFP bit for non-secure mode */
	asm volatile ("movw	r0, #0x0c00");
	asm volatile ("movt	r0, #0x0006");
	asm volatile ("mcr	p15, 0, r0, c1, c1, 2");
		  
	/* set FW bit in ACTRL for 63138 */
	asm volatile ("mrc p15, 0, r1, c1, c0, 1");
	asm volatile ("orr r1, r1, #0x1");
	asm volatile ("mcr p15, 0, r1, c1, c0, 1");
}

void enable_ns_access(void)
{
	SCU->secure_acc = 0xfff;	//enable linux access to SCU control reg
	SCU->invalid_all = 0xffff;
}

void boot_secondary_cpu(unsigned long vector)
{
	unsigned int nr_cpus=0;

	prepare_ns_boot();

	printf("boot secondary cpu from 0x%lx\n", vector);

	*(volatile uint32_t*)(BOOTLUT_BASE+0x20) = vector;

	nr_cpus = get_nr_cpus();
	if(nr_cpus > 1) {
		if (pmc_a9_core_power_up(1))
			printf("failed to power up secondary cpu\n");
	}

	return;
}
#endif

int arch_cpu_init(void)
{
#if defined(CONFIG_BCMBCA_IKOS)
	icache_enable();
#endif  
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	enable_ubus_fast_ack();
	/* enable unalgined access */
	set_cr(get_cr() & ~CR_A);
#if !defined(CONFIG_TPL_ATF)
	enable_ns_access();
#endif
#endif

#ifdef CONFIG_DISABLE_CONSOLE
        gd->flags |= GD_FLG_DISABLE_CONSOLE;
#endif
#ifdef CONFIG_SILENT_CONSOLE
	gd->flags |= GD_FLG_SILENT;
#endif

	return 0;
}
