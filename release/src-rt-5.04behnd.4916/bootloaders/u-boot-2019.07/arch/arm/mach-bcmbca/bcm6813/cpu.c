/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <spl.h>
#include <asm/arch/ddr.h>
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#include "clk_rst.h"
#include "asm/arch/BPCM.h"
#include "clk_rst.h"
#endif
#include "tpl_params.h"
#include "spl_ddrinit.h"
#include "bcm_strap_drv.h"

int pll_ch_freq_set(unsigned int pll_addr, unsigned int ch, unsigned int mdiv);
int pll_ch_reset(unsigned int pll_addr, unsigned int ch, unsigned int pll_reg_offset);
int set_cpu_freq(int freqMHz);

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SPL_BUILD)

void disable_memc_sram(void)
{
	uint32_t *reg = (uint32_t *)(MEMC_BASE + mc2_afx_sram_match_cfg_sram_start_addr_hi);
	
	writel(readl(reg)&~0x80000000, reg);
}

void enable_memc_sram(void)
{
	uint32_t *reg;
	uint64_t addr;

	reg  = (uint32_t *)(MEMC_BASE + mc2_afx_sram_match_cfg_sram_end_addr_lo);
	addr = CONFIG_SYS_PAGETBL_BASE + CONFIG_SYS_PAGETBL_SIZE - 1;
	writel((uint32_t)addr, reg);
	reg  = (uint32_t *)(MEMC_BASE + mc2_afx_sram_match_cfg_sram_end_addr_hi);
	writel((uint32_t)(addr>>32), reg);

	reg  = (uint32_t *)(MEMC_BASE + mc2_afx_sram_match_cfg_sram_start_addr_lo);
	addr = CONFIG_SYS_PAGETBL_BASE;
	writel((uint32_t)addr, reg);
	reg  = (uint32_t *)(MEMC_BASE + mc2_afx_sram_match_cfg_sram_start_addr_hi);
	writel(((uint32_t)(addr>>32))|0x80000000, reg);
}

#if !defined(CONFIG_TPL_BUILD)
static void enable_ts0_couner(void)
{
	BIUCFG->ts0_ctrl.CNTCR |= 0x1;
}
#else
static void cci500_enable(void)
{
	/*Enable access from E2 and below */
	CCI500->secr_acc |= SECURE_ACCESS_UNSECURE_ENABLE;
}

static void enable_ns_access(void)
{
	BIUCFG->bac.bac_permission |= 0x33; // Linux access to BAC_CPU_THERM_TEMP
}

static void setup_ubus_rangechk(void)
{
	/* Size in MB. First 2GB is set up by default */  
	int size_left = tplparams->ddr_size - 2048;
	int size, size_bit, i = 1;
#ifdef PHYS_SDRAM_2
	uint64_t addr = PHYS_SDRAM_2;
#else
	uint64_t addr = 0x100000000UL;
#endif
	
	/* Fix the default of RC0 to only enable lower 2G memory for ubus master */
	UBUS4_RANGE_CHK_SETUP->cfg[0].base = 0x13;
	
	/* setup the second range check for the top DDR region */
	while (size_left > 0 && i < 16) {
		/* each range checker can support up to 4GB size */  
		if (size_left > 4096 )
			size = 4096;
		else
			size = size_left;
		size_left -= size;
		size_bit = 0;
		size = (size << 8);  /* MB to # of 4KB */
		while (size) {
			size = (size >> 1);
			if (size)
				size_bit++;
		}

		UBUS4_RANGE_CHK_SETUP->cfg[i].control = 0x1f0;
		UBUS4_RANGE_CHK_SETUP->cfg[i].srcpid[0] = 0xffffffff;
		UBUS4_RANGE_CHK_SETUP->cfg[i].seclev = 0xffffffff;
		UBUS4_RANGE_CHK_SETUP->cfg[i].base = (addr&0xffffffe0) | size_bit;
		UBUS4_RANGE_CHK_SETUP->cfg[i].base_up = addr >> 32;

		addr += 4096UL << size_bit;
		i++;
	}
}
#endif
#endif

#if defined(CONFIG_BCMBCA_PMC)
int set_cpu_freq(int freqMHz);

void boost_cpu_clock(void)
{
	/* 
	 * 6813 require PMC AVS runs first in u-boot proper before 
 	 * we can increase cpu clock. Enable this function only for 
	 * u-boot proper and call it from PMC driver
	 */
#if !defined(CONFIG_SPL_BUILD)
	u32 old_freq, new_freq;
	if (pll_ch_freq_get(PMB_ADDR_BIU_PLL, 0, &old_freq))
		return;
	new_freq = old_freq;
	/* boost too max cpu clock by four times when strapped from slow clock */
	if (bcm_strap_parse_and_test(ofnode_null(), "strap-cpu-slow-freq")) {
		new_freq = old_freq*4;
		set_cpu_freq(new_freq);
		if (new_freq == 2600)
			// raise ACEBIU clock rate to 866 MHz
			pll_ch_freq_set(PMB_ADDR_BIU_PLL, 1, 6);
		else
			// raise ACEBIU clock rate to 800 or 880 MHz based on vco freq
			pll_ch_freq_set(PMB_ADDR_BIU_PLL, 1, 5);
	}
	printf("set cpu freq to %dMHz from %dMHz\n", new_freq, old_freq);
#endif
}

int set_cpu_freq(int freqMHz)
{
	int mdiv;
	u32 vco_freq, target_freq;

	if( freqMHz > 2600 || freqMHz < 500 )
	{
		printf("%dMHz is not supported\n", freqMHz);
		return -1;
	}

	if (pll_vco_freq_get(PMB_ADDR_BIU_PLL, &vco_freq) != 0) {
		printf("failed to get vco freq!\n");
		return -2;
	}
	/* mdiv = Fvco/target frequency */
	mdiv = vco_freq/freqMHz;

	pll_ch_freq_set(PMB_ADDR_BIU_PLL, 0, mdiv);
	target_freq = vco_freq/mdiv;

	return target_freq;
}
#endif

void reset_plls(void)
{
	/* Software workaround for non-resetting eMMC PLL */
	pll_ch_reset(PMB_ADDR_BIU_PLL, 5, PLLBPCMRegOffset(ch45_cfg));

	/* Soft-reset does not reset the cpu clock so force the cpu to slow
	   clock(vco/8) for AVS to work upon board reset */
	if (bcm_strap_parse_and_test(ofnode_null(), "strap-cpu-slow-freq"))
		pll_ch_freq_set(PMB_ADDR_BIU_PLL, 0, 8);

	return;
}

int arch_cpu_init(void)
{
#if defined(CONFIG_BCMBCA_IKOS)
	icache_enable();
#endif
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	disable_memc_sram();  
	enable_ts0_couner();
#if defined(CONFIG_BCMBCA_DDRC)
	spl_ddrinit_prepare();
#endif
	/* enable unalgined access */
	set_sctlr(get_sctlr() & ~CR_A);
#endif

#if defined(CONFIG_TPL_BUILD)
	enable_ns_access();
	disable_memc_sram();
	setup_ubus_rangechk();
	cci500_enable();
#endif

#ifdef CONFIG_DISABLE_CONSOLE
        gd->flags |= GD_FLG_DISABLE_CONSOLE;
#endif
#ifdef CONFIG_SILENT_CONSOLE
	gd->flags |= GD_FLG_SILENT;
#endif

    return 0;
}

#if !defined(CONFIG_TPL_ATF)
void boot_secondary_cpu(unsigned long vector)
{
	uint32_t cpu, nr_cpus = 4;
	ARM_CONTROL_REG ctrl_reg;
	uint64_t rvbar = vector;

	printf("boot secondary cpu from 0x%lx\n", vector);

	cpu = 1;
	while (cpu < nr_cpus) {
		int stat;

		BIUCFG->cluster[0].rvbar_addr[cpu] = rvbar;
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
