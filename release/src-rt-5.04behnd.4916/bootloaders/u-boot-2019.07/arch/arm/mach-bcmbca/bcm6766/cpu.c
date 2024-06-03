/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr.h>
#include <asm/system.h>
#include <spl.h>
#include <asm/system.h>
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#include "asm/arch/BPCM.h"
#include "bcm_otp.h"
#endif
#include "tpl_params.h"
#if defined(CONFIG_BCMBCA_UBUS4_DCM)
#include "bcm_ubus4.h"
#endif
#include "spl_ddrinit.h"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SPL_BUILD)
#if !defined(CONFIG_TPL_BUILD)
int i=0;
static void enable_ts0_couner(void)
{
	BIUCFG->ctmr_ctrl.CTRL = (CTMR_CTRL_ENABLE_ACCESS|CTMR_CTRL_ENABLE_TMR);
	
}
#else
static void enable_ns_access(void)
{
	BIUCFG->bac.bac_permission |= 0x33; // Linux access to BAC_CPU_THERM_TEMP
}

static void setup_ubus_rangechk(void)
{
#ifdef PHYS_SDRAM_2
	uint64_t addr = PHYS_SDRAM_2;
#else
	uint64_t addr = 0x100000000ULL;
#endif
	
	/* RC0 for the lower 2G memory */
	UBUS4CCB_RANGE_CHK_SETUP->cfg[0].control = 0xff0;
	UBUS4CCB_RANGE_CHK_SETUP->cfg[0].seclev = 0xffffffff;
	UBUS4CCB_RANGE_CHK_SETUP->cfg[0].start = 0x0;
	if (tplparams->ddr_size > 2048) {
		UBUS4CCB_RANGE_CHK_SETUP->cfg[0].end = 0x7ffff;  /* bit 39:12 */

		/* RC1 for the upper 2G memory */
		UBUS4CCB_RANGE_CHK_SETUP->cfg[1].control = 0xff0;
		UBUS4CCB_RANGE_CHK_SETUP->cfg[1].seclev = 0xffffffff;
		UBUS4CCB_RANGE_CHK_SETUP->cfg[1].start = addr >> 12;
		addr += (tplparams->ddr_size - 2048) << 20;
		UBUS4CCB_RANGE_CHK_SETUP->cfg[1].end = (addr - 1) >> 12;
	}
	else {
		addr = tplparams->ddr_size << 20;
		UBUS4CCB_RANGE_CHK_SETUP->cfg[0].end = (addr -1) >> 12;

		/* clear default value for rc1 */
		UBUS4CCB_RANGE_CHK_SETUP->cfg[1].control = 0x0;
		UBUS4CCB_RANGE_CHK_SETUP->cfg[1].start = 0x0;
		UBUS4CCB_RANGE_CHK_SETUP->cfg[1].end = 0x0;
	}
}
#endif
#endif

#if defined(CONFIG_BCMBCA_PMC)
int set_cpu_freq(int freqMHz);
int pll_ch_freq_set(unsigned int pll_addr, unsigned int ch, unsigned int mdiv);

void boost_cpu_clock(void)
{
#if defined(CONFIG_BCMBCA_IKOS)
	/* 6765 IKOS db already setup cpu clock to 2GHz and AXI to 800MHz
	 * Just need to set dcm bypass mode so ubus runs at 625MHz
	 */
#if defined(CONFIG_BCMBCA_UBUS4_DCM)
	bcm_ubus4_dcm_clk_bypass(1);
#endif
#else
	printf("set cpu freq to 2000MHz\n");
	set_cpu_freq(2000);
	pll_ch_freq_set(PMB_ADDR_BIU_PLL, 1, 4000/800); // raise AXI/ACEBIU clock rate to 800 MHz
#endif
}

int set_cpu_freq(int freqMHz)
{
	int mdiv;

	if( freqMHz > 2000 || freqMHz < 400 )
	{
		printf("%dMHz is not supported\n", freqMHz);
		return -1;
	}

	/* VCO at 4GHz, mdiv = Fvco/target frequency */
	mdiv = 4000/freqMHz;

#if defined(CONFIG_BCMBCA_IKOS)
#else
	pll_ch_freq_set(PMB_ADDR_BIU_PLL, 0, mdiv);
#endif

	return 4000/mdiv;	
}
#endif

int arch_cpu_init(void)
{
#if defined(CONFIG_BCMBCA_IKOS)
	icache_enable();
#endif

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	enable_ts0_couner();
	u32 frq = COUNTER_FREQUENCY;
	// set arch timer frequency
	asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (frq));
#if defined(CONFIG_BCMBCA_DDRC)
	spl_ddrinit_prepare();
#endif
	/* enable unalgined access */
	set_cr(get_cr() & ~CR_A);
#endif

#if defined(CONFIG_TPL_BUILD)
	enable_ns_access();
	setup_ubus_rangechk();
#endif

#ifdef CONFIG_DISABLE_CONSOLE
	gd->flags |= GD_FLG_DISABLE_CONSOLE;
#endif
#ifdef CONFIG_SILENT_CONSOLE
	gd->flags |= GD_FLG_SILENT;
#endif

    return 0;
}

void arch_cpu_deinit(void)
{
}

int get_nr_cpus(void)
{
	uint32_t nr_cpus;

#if defined(CONFIG_BCMBCA_IKOS)
	nr_cpus=4;
#else
	if (bcm_otp_get_nr_cpus(&nr_cpus)) {
		printf("Error: failed to read cpu core from OTP\n");
		nr_cpus = 4;
	} else {
			nr_cpus = 3 - nr_cpus;
	}
#endif
	return nr_cpus;
}

#if !defined(CONFIG_TPL_ATF) && defined(CONFIG_BCMBCA_PMC)
void boot_secondary_cpu(unsigned long vector)
{
	uint32_t cpu = 1; 
	uint32_t nr_cpus = get_nr_cpus();
	ARM_CONTROL_REG ctrl_reg;

	printf("boot secondary cpu from 0x%lx\n", vector);

	*(volatile uint32_t*)(BOOTLUT_BASE+0x20) = vector;
	
	while (cpu < nr_cpus) {
		int stat;

		stat = PowerOnDevice(PMB_ADDR_ORION_CPU0 + cpu);
		if (stat != kPMC_NO_ERROR)
			printf("failed to power on secondary cpu %d - sts %d\n", cpu, stat);
		
		stat = ReadBPCMRegister(PMB_ADDR_BIU_BPCM, ARMBPCMRegOffset(arm_control), &ctrl_reg.Reg32);
		ctrl_reg.Bits.cpu_reset_n &= ~(0x1 << cpu);
		stat |= WriteBPCMRegister(PMB_ADDR_BIU_BPCM, ARMBPCMRegOffset(arm_control), ctrl_reg.Reg32);
		if (stat != kPMC_NO_ERROR)
			printf("failed to boot secondary cpu %d - sts %d\n", cpu, stat);
		
		cpu++;
	}

	return;
}
#endif
