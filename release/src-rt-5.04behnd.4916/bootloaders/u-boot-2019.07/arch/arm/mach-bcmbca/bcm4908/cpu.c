/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <asm/sections.h>
#include <asm/arch/cpu.h>
#include <spl.h>
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#include "asm/arch/BPCM.h"
#endif
#if defined(CONFIG_BCMBCA_DDRC)
#include "spl_ddrinit.h"
#endif
#include "bcmbca-dtsetup.h"
#include "bcm_strap_drv.h"

DECLARE_GLOBAL_DATA_PTR;

int set_cpu_freq(int freqMHz);
uint32_t cpu_speed = 0;
#define DUAL_CORE_4906 0x4906

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)

static void enable_ubus_fast_write_ack(void)
{
	BIUCTRL->ubus_cfg |= 0x70;
}

#endif

#if defined(CONFIG_BCMBCA_PMC)

void boost_cpu_clock(void)
{
	cpu_speed = bcm_strap_parse_and_test(ofnode_null(), "strap-cpu-slow-freq") ? 400: 1800;
	printf("set cpu freq to 1800MHz from %dMHz\n", cpu_speed);
	set_cpu_freq(1800);
}

int set_cpu_freq(int freqMHz)
{
	PLL_CTRL_REG ctrl_reg;
	uint32_t clkcfg;

	/* only support 1800MHz and 400MHz for now */
	if( freqMHz != 1800 && freqMHz != 400)
	{
		printf("%dMHz is not supported, stay with %dMHz\n", freqMHz, cpu_speed);
		return -1;
	}

	/* this code is used to switch from default slow cpu 400MHz to 1.8GHz */
	if( freqMHz == 1800 )
	{
		if( cpu_speed == 400 )
		{
			ReadBPCMRegister(PMB_ADDR_B53PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
			ctrl_reg.Bits.byp_wait = 0;
			WriteBPCMRegister(PMB_ADDR_B53PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);
		}
		/* adjust clock divider and safe mode setting for better perfomance */
		clkcfg = BIUCTRL->clock_cfg;
		clkcfg &= ~(BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_MASK|BIU_CPU_CTRL_CLK_CFG_SAFE_CLOCK_MODE_MASK|BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_MASK);
		clkcfg |= (BIU_CPU_CTRL_CLK_CFG_CCI_CLK_RATIO_DIV2|BIU_CPU_CTRL_CLK_CFG_CLK_RATIO_DIV1);
		BIUCTRL->clock_cfg = clkcfg;
	}

	if( freqMHz == 400 )
	{
		/* turn on safe mode setting */
		clkcfg = BIUCTRL->clock_cfg;
		clkcfg &= ~(BIU_CPU_CTRL_CLK_CFG_SAFE_CLOCK_MODE_MASK);
		clkcfg |= (BIU_CPU_CTRL_CLK_CFG_SAFE_CLOCK_MODE_MASK);
		BIUCTRL->clock_cfg = clkcfg;

		if( cpu_speed == 1800)
		{
			ReadBPCMRegister(PMB_ADDR_B53PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
			ctrl_reg.Bits.byp_wait = 1;
			WriteBPCMRegister(PMB_ADDR_B53PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);
		}
	}

	cpu_speed = freqMHz;
	return freqMHz;
}
#endif

u32 bcmbca_get_chipid(void)
{
	u32 chipId = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
	/*
	 * 62118 and 62116 use 20 bits to represent the chip id 
	 * as compared to 16 in case of 4908/4906
	 */
	if((chipId & 0x49000) == 0x49000) {
		chipId = chipId >> 4;
	}
	return chipId;
}

int arch_cpu_init(void)
{
#if defined(CONFIG_BCMBCA_IKOS)
	icache_enable();
#endif  
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	enable_ubus_fast_write_ack();
#if defined(CONFIG_BCMBCA_DDRC)
	spl_ddrinit_prepare();
#endif
	/* enable unalgined access */
	set_sctlr(get_sctlr() & ~CR_A);
#endif

#ifdef CONFIG_DISABLE_CONSOLE
        gd->flags |= GD_FLG_DISABLE_CONSOLE;
#endif
#ifdef CONFIG_SILENT_CONSOLE
	gd->flags |= GD_FLG_SILENT;
#endif

	return 0;
}
int get_nr_cpus(void)
{
	unsigned int chipId = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
	int nr_cpus=QUAD_CPUS;

	if(chipId == DUAL_CORE_4906)
		nr_cpus=DUAL_CPUS;

	return nr_cpus;
}

#if !defined(CONFIG_TPL_ATF)
void boot_secondary_cpu(unsigned long vector)
{
	uint32_t cpu, nr_cpus = QUAD_CPUS;

	printf("boot secondary cpu from 0x%lx\n", vector);

	nr_cpus = get_nr_cpus();
	cpu = 1;
	while (cpu < nr_cpus) {
		BOOT_LUT->bootLutRst = (uint32_t)vector;
		BIUCTRL->power_cfg |= (0x1 << (cpu+BIU_CPU_CTRL_PWR_CFG_CPU0_BPCM_INIT_ON_SHIFT));
		BIUCTRL->reset_cfg &= ~(0x1 << cpu);
		cpu++;
	}

	return;
}
#endif
