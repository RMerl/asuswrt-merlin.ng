/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/io.h>
#include <spl.h>
#if defined(CONFIG_BCMBCA_DDRC)
#include <asm/arch/ddr.h>
#endif
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#include "asm/arch/BPCM.h"
#endif
#include "tpl_params.h"

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)

static void enable_ts0_couner(void)
{
	BIUCFG->ts0_ctrl.CNTCR |= 0x1;
}
#elif defined(CONFIG_TPL_BUILD)
static void cci500_enable(void)
{
	/*Enable access from E2 and below */
	CCI500->secr_acc |= SECURE_ACCESS_UNSECURE_ENABLE;
}

static void enable_ns_access(void)
{
	BIUCFG->bac.bac_permission |= 0x33; // Linux access to BAC_CPU_THERM_TEMP
}

static void disable_memc_sram(void)
{
	uint32_t addr = MEMC_BASE + mc2_afx_sram_match_cfg_sram_start_addr_hi;
	
	writel(readl(addr)&~0x80000000, addr);
}

static void setup_ubus_rangechk(void)
{
	/* Fix the default of RC0 to only enable lower 2G memory for ubus master */		
	UBUS4_RANGE_CHK_SETUP->cfg[0].base = 0x13;
	/* setup the second range check for the top DDR region */
	if (tplparams->ddr_size > 2048) {
		UBUS4_RANGE_CHK_SETUP->cfg[1].control = 0x1f0;
		UBUS4_RANGE_CHK_SETUP->cfg[1].srcpid[0] = 0xffffffff;
		UBUS4_RANGE_CHK_SETUP->cfg[1].seclev = 0xffffffff;
		/* enable ubus maser to access the upper 2GB */	
		UBUS4_RANGE_CHK_SETUP->cfg[1].base = 0x13;
		UBUS4_RANGE_CHK_SETUP->cfg[1].base_up = 0x1;
	}
}

#define CLKRST_XTAL_CNTLXTAL_DRV_EN_BIT (20)  
static void disable_xtal_clk(void)
{
	uint32_t data;
	int ret;

	ret = ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST,
		CLKRSTBPCMRegOffset(xtal_control), &data);

	data &= ~(0x1 << CLKRST_XTAL_CNTLXTAL_DRV_EN_BIT);

	ret |= WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST,
		CLKRSTBPCMRegOffset(xtal_control), data);

	if (ret)
		printf("Failed to disable xtal clk\n");
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

#if defined(CONFIG_BCMBCA_PMC)
void boost_cpu_clock(void)
{
	printf("set cpu freq to 2000MHz\n");
	set_cpu_freq(2000);
	pll_ch_freq_set(PMB_ADDR_BIU_PLL, 1, 4000/800); // raise ACEBIU clock rate to 800 MHz

	disable_xtal_clk();
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

static int reset_plls(void)
{
	/* Force Reset eMMC PLL */
	pll_ch_reset(PMB_ADDR_RDPPLL, 1, PLLBPCMRegOffset(ch01_cfg));
	return 0;
}

int arch_cpu_init(void)
{
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
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

    reset_plls();
    return 0;
}

int bcmbca_get_boot_device(void)
{
	if ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_NAND_MASK) == MISC_STRAP_BUS_BOOT_NAND)
		return BOOT_DEVICE_NAND;

	if ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK) == MISC_STRAP_BUS_BOOT_SPI_NAND)
		return BOOT_DEVICE_SPI;

	if ((MISC->miscStrapBus&MISC_STRAP_BUS_BOOT_SEL_MASK) == MISC_STRAP_BUS_BOOT_EMMC)
		return BOOT_DEVICE_MMC1;

	printf("Error: boot_sel straps are not set correctly\n");

	return BOOT_DEVICE_NONE;
}

#if !defined(CONFIG_TPL_ATF)
void boot_secondary_cpu(unsigned long vector)
{
	uint32_t cpu, nr_cpus = 2;
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
