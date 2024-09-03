/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr.h>
#include <asm/io.h>
#include <spl.h>
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
#include "bca_common.h"

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_BCMBCA_CCB)
static void enable_ccb(void)
{
	volatile unsigned  int axi_val;

	axi_val=readl_relaxed((uint32_t*)&UBUS4CCB_AXI_CFG_REGS->axi_config_0) | ENABLE_CCB;
	writel_relaxed(axi_val, (uint32_t*)&UBUS4CCB_AXI_CFG_REGS->axi_config_0); 
}
#endif

#if defined(CONFIG_SPL_BUILD)

#if !defined(CONFIG_TPL_BUILD)
static void enable_ts0_couner(void)
{
	BIUCFG->ctmr_ctrl.CTRL = (CTMR_CTRL_ENABLE_ACCESS|CTMR_CTRL_ENABLE_TMR);
}
#else
static void enable_ns_access(void)
{
	/* Enable Linux access to BAC_CPU_THERM_TEMP register */
	BIUCFG->bac.bac_permission |= 0x33;

	/* Enable Linux read access to TS0 counter and control register */
	BIUCFG->access.ts_access |= 0x11;
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
#if defined(CONFIG_BCMBCA_UBUS4_DCM)
	/* IKOS: IKOS db already setup cpu clock to 2GHz and AXI to 800MHz
	 * For the silicone and emulation set dcm ubus bypass mode to run at 625MHz
	 */
	bcm_ubus4_dcm_clk_bypass(1);
#endif
#if !defined(CONFIG_BCMBCA_IKOS)
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

	pll_ch_freq_set(PMB_ADDR_BIU_PLL, 0, mdiv);

	return 4000/mdiv;	
}
#endif

#if !defined(CONFIG_BCMBCA_IKOS)
void print_chipinfo(void)
{
	u32 chipId = bcmbca_get_chipid();
	u32 revId = bcmbca_get_chiprev();
	int chipVar = -1;

	bcm_otp_get_chipid((u32 *)&chipVar);
	if (chipVar == 1 && chipId == 0x6765)
		printf("Chip ID: BCM%s_%X\n", "6764L", revId);
	else
		printf("Chip ID: BCM%X_%X\n",chipId,revId);
}
#endif

void bcmbca_enable_memc_sram(u64 addr, u64 size)
{
	uint32_t *reg;
	uint64_t end_addr;

	reg  = (uint32_t *)(MEMC_BASE + mc2_afx_sram_match_cfg_sram_end_addr_lo);
	end_addr = addr + size - 1;
	writel((uint32_t)end_addr, reg);
	reg  = (uint32_t *)(MEMC_BASE + mc2_afx_sram_match_cfg_sram_end_addr_hi);
	writel((uint32_t)(end_addr>>32), reg);

	reg  = (uint32_t *)(MEMC_BASE + mc2_afx_sram_match_cfg_sram_start_addr_lo);
	writel((uint32_t)addr, reg);
	reg  = (uint32_t *)(MEMC_BASE + mc2_afx_sram_match_cfg_sram_start_addr_hi);
	writel(((uint32_t)(addr>>32))|mc2_afx_sram_match_cfg_sram_start_addr_hi_enable_MASK, reg);
}

void bcmbca_disable_memc_sram(void)
{
	uint32_t *reg = (uint32_t *)(MEMC_BASE + mc2_afx_sram_match_cfg_sram_start_addr_hi);
	
	writel(readl(reg)&~mc2_afx_sram_match_cfg_sram_start_addr_hi_enable_MASK, reg);
}

int arch_cpu_init(void)
{
#if defined(CONFIG_BCMBCA_IKOS)
	icache_enable();
#endif

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	/* always disable memc sram first in case btrm keeps it enabled */
	bcmbca_disable_memc_sram();

	enable_ts0_couner();
#if defined(CONFIG_BCMBCA_DDRC)
	spl_ddrinit_prepare();
#endif
	/* enable unalgined access */
	set_sctlr(get_sctlr() & ~CR_A);
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

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_BCMBCA_CCB)
	/* Enable CCB even before any of the caches enabled */
	enable_ccb();
#endif

    return 0;
}

void arch_cpu_deinit(void)
{
}

#if !defined(CONFIG_TPL_ATF) && defined(CONFIG_BCMBCA_PMC)
void boot_secondary_cpu(unsigned long vector)
{
	uint32_t cpu = 1; 
	uint32_t nr_cpus = 4;
	uint64_t rvbar = vector;

	printf("boot secondary cpu from 0x%lx\n", vector);

	while (cpu < nr_cpus) {
		int stat;

		BIUCFG->cluster[0].rvbar_addr[cpu] = rvbar;
		stat = PowerOnDevice(PMB_ADDR_ORION_CPU0 + cpu);
		if (stat != kPMC_NO_ERROR)
			printf("failed to power on secondary cpu %d - sts %d\n", cpu, stat);

		cpu++;
	}

	return;
}
#endif
