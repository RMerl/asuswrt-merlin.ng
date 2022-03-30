/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr.h>
#include <asm/arch/ubus4.h>
#include <spl.h>
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#include "asm/arch/BPCM.h"
#endif

#if defined(CONFIG_TPL_BUILD)
static void enable_ns_access(void)
{
	BIUCFG->bac.bac_permission |= 0x33;     // Linux access to BAC_CPU_THERM_TEMP
}

/*this function is used to turn on CCI from secure mode 
*  *   it also turns snooping enable for S5 CPU interface*/
static void cci500_enable(void)
{
	/*Enable access from E2 and below */
	CCI500->secr_acc |= SECURE_ACCESS_UNSECURE_ENABLE;
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
	printf("set cpu freq to 1500MHz\n");
	set_cpu_freq(1500);
	pll_ch_freq_set(PMB_ADDR_BIU_PLL, 1, 3000/600); // raise ACEBIU clock rate to 600 MHz
}

int set_cpu_freq(int freqMHz)
{
	int mdiv;

	if( freqMHz > 1500 || freqMHz < 300 )
	{
		printf("%dMHz is not supported\n", freqMHz);
		return -1;
	}

	/* VCO at 3GHz, mdiv = Fvco/target frequency */
	mdiv = 3000/freqMHz;

	pll_ch_freq_set(PMB_ADDR_BIU_PLL, 0, mdiv);

	return 3000/mdiv;	
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

	printf("Error: boot_sel straps are not set correctly\n");

	return BOOT_DEVICE_NONE;
}

int arch_cpu_init(void)
{
#if defined(CONFIG_SPL_BUILD)
#if !defined(CONFIG_TPL_BUILD) /* SPL */
	u32 frq = COUNTER_FREQUENCY;

	spl_ddrinit_prepare();

	/* set arch timer frequency */
	asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (frq));
	/* enable system timer */
	BIUCFG->TSO_CNTCR |= 1;

	/* force axi write reply to workaround wifi memory write ordering issue */
	ubus_master_cpu_enable_axi_write_cache(0);
	/* enable unalgined access */
	set_cr(get_cr() & ~CR_A);	
#else /* TPL */
	enable_ns_access();
	cci500_enable();
#endif
#else /* u-boot proper */
	/* u-boot proper to fix up arbiter setting in case SPL of lockdown */
	MEMC->ARB_CFG |= MC_ARB_CFG_ARB_MODE1_MASK;
#endif

	return 0;
}

void arch_cpu_deinit()
{
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	/* restore axi write cache when we spl does not need to use wifi memory */
	ubus_master_cpu_enable_axi_write_cache(1);
#endif
}

int get_nr_cpus()
{
	uint32_t nr_cpus;

	if (bcm_otp_get_nr_cpus(&nr_cpus)) {
		printf("Error: failed to read cpu core from OTP\n");
		nr_cpus = 4;
	} else {
		if (nr_cpus >= 0 && nr_cpus <= 4)
			nr_cpus = 4;
		else
			nr_cpus = 8 - nr_cpus;
	}

	return nr_cpus;
}
