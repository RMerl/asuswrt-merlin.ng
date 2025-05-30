/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <asm/arch/pmc.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr.h>
#include <spl.h>
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#include "asm/arch/BPCM.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

int set_cpu_freq(int freqMHz);

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
static void set_b15_config(void)
{
	uint32_t reg = 0x0;

	/* set mcp0 read/write credits to 8 */
	/* And workaround for HW63148-100: Disabling write pairing by clearing bits 28-31.*/
	reg = B15CTRL->cpu_ctrl.credit;
	reg &= 0x0fffff00;
	reg |= 0x88;
	B15CTRL->cpu_ctrl.credit = reg;

	/* set RAC_config0 to 0x0 to disable it */
	B15CTRL->cpu_ctrl.rac_cfg0 = 0x0;

	return;
}

void bcm_setsw(void)
{
	/* set the internal voltage regulator gain to 8. This reduces the response time and keeps
	   voltage supply to ddr stable. */
	swr_write(2, 0 ,0x800);
}

static void set_ubus_config(void)
{
	/* enable UBUS multiple read/write transaction to improve performance */
	B15CTRL->cpu_ctrl.ubus_cfg |= 0x70;
}
#endif

#if defined(CONFIG_BCMBCA_PMC)
#define PLL_GET_CHANNEL_OFFSET(channel)  (PLLBPCMRegOffset(ch01_cfg) + ((channel/2)*sizeof(PLL_CHCFG_REG)>>2))
static void set_biu_pll_post_divider(int bpcmaddr, int channel, int mdiv)
{
	PLL_CHCFG_REG pll_ch_cfg;
	int offset, mdiv_rb;

	if( channel < 0 || channel > 5 )
	   return;

	offset = PLL_GET_CHANNEL_OFFSET(channel);

	ReadBPCMRegister(bpcmaddr, offset, &pll_ch_cfg.Reg32);
	mdiv_rb = channel&1 ? pll_ch_cfg.Bits.mdiv1 : pll_ch_cfg.Bits.mdiv0;
	if (mdiv_rb != mdiv) {
		if( channel&1 )
			pll_ch_cfg.Bits.mdiv1 = mdiv;
		else
			pll_ch_cfg.Bits.mdiv0 = mdiv;
		WriteBPCMRegister(bpcmaddr, offset, pll_ch_cfg.Reg32);
		udelay(1000);
		if( channel&1 )
			pll_ch_cfg.Bits.mdiv_override1 = 1;
		else
			pll_ch_cfg.Bits.mdiv_override0 = 1;
		WriteBPCMRegister(bpcmaddr, offset, pll_ch_cfg.Reg32);
		udelay(10000);
	}

	return;
}

void boost_cpu_clock(void)
{
	printf("set cpu freq to 1500MHz\n");
	set_cpu_freq(1500);
}

int set_cpu_freq(int freqMHz)
{
	uint32_t val;
	int mdiv0 = 2;

	/* we only support the following frequency:
	 * 1) 375, 750, and 1500
	 * 2) 125, 250, 500, and 1000 */
	val = B15CTRL->cpu_ctrl.clock_cfg;
	if ((freqMHz == 375) || (freqMHz == 750) || (freqMHz == 1500)) {
		mdiv0 = 2;
		val &= ~0xf;
		val |= (1500 / freqMHz) - 1;
		if (freqMHz == 1500)
			val &= ~0x10;
	} else if ((freqMHz == 125) || (freqMHz == 250) || (freqMHz == 500)  || (freqMHz == 1000)) {
		mdiv0 = 3;
		val &= ~0xf;
		val |= (1000 / freqMHz) - 1;
		if (freqMHz == 1000)
			val &= ~0x10;
	} else if (freqMHz == 7501) {
		/* this is a special setting of 750 MHz for hw team to try */
		mdiv0 = 4;
		val &= ~0xf;
		freqMHz = 750;
	} else {
		printf("cpufreq %dMHz is not supported\n", freqMHz);
		return -1;
	}

	set_biu_pll_post_divider(PMB_ADDR_B15_PLL, 0, mdiv0);

	B15CTRL->cpu_ctrl.clock_cfg = val;

	return freqMHz;
}
#endif

void bcmbca_disable_memc_sram(void)
{
	MEMC->SRAM_REMAP_CTRL = 0;
	MEMC->SRAM_REMAP_CTRL;
}

int arch_cpu_init(void)
{
#if defined(CONFIG_BCMBCA_IKOS)
	icache_enable();
#endif  
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	u32 frq = COUNTER_FREQUENCY;

	/* always disable memc sram first in case btrm keeps it enabled */
	bcmbca_disable_memc_sram();

	/* set arch timer frequency */
	asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (frq));
	
	set_ubus_config();
	set_b15_config();
#endif
	/* enable unalgined access */
	set_cr(get_cr() & ~CR_A);	
	
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
	printf("boot secondary cpu from 0x%lx\n", vector);

	*(volatile uint32_t*)(BOOTLUT_BASE+0x20) = vector;

	B15CTRL->cpu_ctrl.cpu1_pwr_zone_ctrl |= 0x400;
	B15CTRL->cpu_ctrl.reset_cfg &= 0xfffffffd;

	return;
}
#endif
