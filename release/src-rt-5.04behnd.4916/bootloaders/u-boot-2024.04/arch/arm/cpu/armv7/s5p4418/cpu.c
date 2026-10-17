// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Nexell
 * Hyunseok, Jung <hsjung@nexell.co.kr>
 */

#include <common.h>
#include <command.h>
#include <asm/system.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <asm/io.h>
#include <asm/arch/nexell.h>
#include <asm/arch/clk.h>
#include <asm/arch/tieoff.h>
#include <cpu_func.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef	CONFIG_ARCH_CPU_INIT
#error must be define the macro "CONFIG_ARCH_CPU_INIT"
#endif

void s_init(void)
{
}

static void cpu_soc_init(void)
{
	/*
	 * NOTE> ALIVE Power Gate must enable for Alive register access.
	 *	     must be clear wfi jump address
	 */
	writel(1, ALIVEPWRGATEREG);
	writel(0xFFFFFFFF, SCR_ARM_SECOND_BOOT);

	/* write 0xf0 on alive scratchpad reg for boot success check */
	writel(readl(SCR_SIGNAGURE_READ) | 0xF0, (SCR_SIGNAGURE_SET));

	/* set l2 cache tieoff */
	nx_tieoff_set(NX_TIEOFF_CORTEXA9MP_TOP_QUADL2C_L2RET1N_0, 1);
	nx_tieoff_set(NX_TIEOFF_CORTEXA9MP_TOP_QUADL2C_L2RET1N_1, 1);
}

int arch_cpu_init(void)
{
	flush_dcache_all();
	cpu_soc_init();
	clk_init();

	return 0;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
int print_cpuinfo(void)
{
	return 0;
}
#endif

void reset_cpu(void)
{
	void *clkpwr_reg = (void *)PHY_BASEADDR_CLKPWR;
	const u32 sw_rst_enb_bitpos = 3;
	const u32 sw_rst_enb_mask = 1 << sw_rst_enb_bitpos;
	const u32 sw_rst_bitpos = 12;
	const u32 sw_rst_mask = 1 << sw_rst_bitpos;
	int pwrcont = 0x224;
	int pwrmode = 0x228;
	u32 read_value;

	read_value = readl((void *)(clkpwr_reg + pwrcont));

	read_value &= ~sw_rst_enb_mask;
	read_value |= 1 << sw_rst_enb_bitpos;

	writel(read_value, (void *)(clkpwr_reg + pwrcont));
	writel(sw_rst_mask, (void *)(clkpwr_reg + pwrmode));
}

void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
