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

uint32_t cpu_speed = 0;
#define DUAL_CORE_4906 0x4906

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)

static void enable_ubus_fast_write_ack(void)
{
	BIUCTRL->ubus_cfg |= 0x70;
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
	if ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_NAND_MASK) == MISC_STRAP_BUS_BOOT_NAND)
		return BOOT_DEVICE_NAND;

	if ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK) == MISC_STRAP_BUS_BOOT_SPI_NAND)
		return BOOT_DEVICE_SPI;

	if ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK) == MISC_STRAP_BUS_BOOT_EMMC)
		return BOOT_DEVICE_MMC1;

	if ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK) == MISC_STRAP_BUS_BOOT_SPI_NOR)
		return BOOT_DEVICE_NOR;

	printf("Error: boot_sel straps are not set correctly\n");

	return BOOT_DEVICE_NONE;
}

#if defined(CONFIG_BCMBCA_PMC)

void boost_cpu_clock(void)
{
	cpu_speed = (MISC->miscStrapBus&MISC_STRAP_BUS_CPU_SLOW_FREQ) ? 400: 1800;
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

int arch_cpu_init(void)
{
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	enable_ubus_fast_write_ack();
#if defined(CONFIG_BCMBCA_DDRC)
	spl_ddrinit_prepare();
#endif
	/* enable unalgined access */
	set_sctlr(get_sctlr() & ~CR_A);
#endif
	return 0;
}
int get_nr_cpus()
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
