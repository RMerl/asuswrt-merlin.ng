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

#define SINGLE_CORE_63132 0x63132

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
static void set_vr_gain(void)
{
	/* set the internal voltage regulator gain to 8. This reduces the response time and keeps
	   voltage supply to ddr stable. */
	PROCMON->SSBMaster.wr_data = 0x800;
	PROCMON->SSBMaster.control = 0x3440;
	PROCMON->SSBMaster.control = 0xb440;
	while (PROCMON->SSBMaster.control & 0x8000) ;

	PROCMON->SSBMaster.wr_data = 0x802;
	PROCMON->SSBMaster.control = 0x3440;
	PROCMON->SSBMaster.control = 0xb440;
	while (PROCMON->SSBMaster.control & 0x8000) ;

	PROCMON->SSBMaster.wr_data = 0x800;
	PROCMON->SSBMaster.control = 0x3440;
	PROCMON->SSBMaster.control = 0xb440;
	while (PROCMON->SSBMaster.control & 0x8000) ;
}

static void enable_ubus_fast_ack(void)
{
	ARMAIPCTRL->cfg |= AIP_CTRL_CFG_WR_FAST_ACK_MASK;
}
#endif

#if !defined(CONFIG_SPL_BUILD)
void print_chipinfo(void)
{
	unsigned int chipId = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
	unsigned int revId = PERF->RevID & REV_ID_MASK;

	printf("Chip ID: BCM%X_%X\n",chipId,revId);
}
#endif

int bcmbca_get_boot_device(void)
{
	if ( (MISC->miscStrapBus & MISC_STRAP_BUS_SW_BOOT_SPI_SPINAND_EMMC_MASK) &&
		((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_OPT_MASK) == MISC_STRAP_BUS_BOOT_SPI_NAND) )
		return BOOT_DEVICE_SPI;

	if ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SPI_NOR) != MISC_STRAP_BUS_BOOT_SPI_NOR)
		return BOOT_DEVICE_NAND;

	if ( (MISC->miscStrapBus & MISC_STRAP_BUS_SW_BOOT_SPI_SPINAND_EMMC_MASK) &&
		((MISC->miscStrapBus & MISC_STRAP_BUS_BOOT_OPT_MASK) == MISC_STRAP_BUS_BOOT_EMMC) )
		return BOOT_DEVICE_MMC1;

	printf("Error: boot_sel straps are not set correctly\n");

	return BOOT_DEVICE_NONE;
}

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

int get_nr_cpus()
{
	unsigned int chipId = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
	int nr_cpus=DUAL_CPUS;

	if(chipId == SINGLE_CORE_63132)
		nr_cpus=ONE_CPU;

	return nr_cpus;
}
int arch_cpu_init(void)
{
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	enable_ubus_fast_ack();
	set_vr_gain();
	/* enable unalgined access */
	set_cr(get_cr() & ~CR_A);	
#endif
	return 0;
}
