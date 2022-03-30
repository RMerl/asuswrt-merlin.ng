/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <asm/sections.h>
#include <asm/arch/ddr.h>
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

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)

static void enable_ts0_counter(void)
{
	BIUCFG->TSO_CNTCR |= 1;
}

#elif defined(CONFIG_TPL_BUILD)
static void cci500_enable(void)
{
	/*Enable access from E2 and below */
	CCI500->secr_acc |= SECURE_ACCESS_UNSECURE_ENABLE;
}

static void enable_ns_access(void)
{
	BIUCFG->bac.bac_permission |= 0x33;	// Linux access to BAC_CPU_THERM_TEMP
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
static void wait_biu_pll_lock(int bpcmaddr)
{
	PLL_STAT_REG stat_reg;

	do {
		ReadBPCMRegister(bpcmaddr, PLLBPCMRegOffset(stat),
				 &stat_reg.Reg32);
	} while (stat_reg.Bits.lock == 0);

	return;
}

#define PLL_GET_CHANNEL_OFFSET(channel)  (PLLBPCMRegOffset(ch01_cfg) + ((channel/2)*sizeof(PLL_CHCFG_REG)>>2))
void set_biu_pll_post_divider(int bpcmaddr, int channel, int mdiv)
{
	PLL_CHCFG_REG pll_ch_cfg;
	int offset, mdiv_rb;

	if (channel < 0 || channel > 5)
		return;

	offset = PLL_GET_CHANNEL_OFFSET(channel);

	ReadBPCMRegister(bpcmaddr, offset, &pll_ch_cfg.Reg32);
	mdiv_rb = channel & 1 ? pll_ch_cfg.Bits.mdiv1 : pll_ch_cfg.Bits.mdiv0;
	if (mdiv_rb != mdiv) {
		if (channel & 1)
			pll_ch_cfg.Bits.mdiv1 = mdiv;
		else
			pll_ch_cfg.Bits.mdiv0 = mdiv;
		WriteBPCMRegister(bpcmaddr, offset, pll_ch_cfg.Reg32);
		udelay(1000);
		if (channel & 1)
			pll_ch_cfg.Bits.mdiv_override1 = 1;
		else
			pll_ch_cfg.Bits.mdiv_override0 = 1;
		WriteBPCMRegister(bpcmaddr, offset, pll_ch_cfg.Reg32);
		udelay(10000);
	}

	return;
}

#if defined(CONFIG_TPL_BUILD)
#define CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV (17)
static void disable_xtal_clk(void)
{
	uint32_t data;
	int ret;

	ret = ReadBPCMRegister(PMB_ADDR_CHIP_CLKRST,
		CLKRSTBPCMRegOffset(xtal_control), &data);

	data |= (0x1 << CLOCK_RESET_XTAL_CONTROL_BIT_PD_DRV);

	ret |= WriteBPCMRegister(PMB_ADDR_CHIP_CLKRST,
		CLKRSTBPCMRegOffset(xtal_control), data);

	if (ret)
		printf("Failed to disable xtal clk\n");
}
#endif

void boost_cpu_clock(void)
{
	printf("set cpu freq to 1675MHz\n");
	set_cpu_freq(1675);

	disable_xtal_clk();
}

int set_cpu_freq(int freqMHz)
{
	PLL_CTRL_REG ctrl_reg;
	PLL_NDIV_REG ndiv_reg;
	PLL_PDIV_REG pdiv_reg;
	int mdiv = 2;
	int ndiv;

	/* cpufreq = Fvco/mdiv = ndiv*50MHz/mdiv */
	/* For clock below the nominal frequency, we use post divider mdiv = 2  
	   cpufreq = ndiv*50MHz/2 where ndiv = 22..67 for 550MHz, 425MHz.. 1675
	   For overclock, we use post divider mdiv=1 in order to keep vco frequency low
	   cpufreq = ndiv*50MHz/1 where ndiv = 34 .. 44  for 1700MHz, 1750MHz.. 2200MHz */

	if (freqMHz < 550 || freqMHz > 1675) {
		printf("%dMHz is not supported\n", freqMHz);
		return -1;
	}

	if (freqMHz > 1675)
		mdiv = 1;
	ndiv = (mdiv * freqMHz) / 50;

	/* round freqMHz to the factor of 50/25MHz */
	freqMHz = (50 * ndiv) / mdiv;

	/* switch to bypass clock for cpu clock change first */
	ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets),
			 &ctrl_reg.Reg32);
	ctrl_reg.Bits.byp_wait = 1;
	WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets),
			  ctrl_reg.Reg32);

	/* assert pll reset */
	ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets),
			 &ctrl_reg.Reg32);
	ctrl_reg.Bits.master_reset = 1;
	WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets),
			  ctrl_reg.Reg32);

	/* change vco pll frequency */
	ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(pdiv),
			 &pdiv_reg.Reg32);
	pdiv_reg.Bits.ndiv_pdiv_override = 1;
	WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(pdiv),
			  pdiv_reg.Reg32);

	ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ndiv),
			 &ndiv_reg.Reg32);
	ndiv_reg.Bits.ndiv_int = ndiv;
	WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(ndiv),
			  ndiv_reg.Reg32);

	/* de-assert pll reset */
	ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets),
			 &ctrl_reg.Reg32);
	ctrl_reg.Bits.master_reset = 0;
	WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets),
			  ctrl_reg.Reg32);

	/* wait for pll to lock */
	wait_biu_pll_lock(PMB_ADDR_BIU_PLL);

	/* set the post divider */
	set_biu_pll_post_divider(PMB_ADDR_BIU_PLL, 0, mdiv);

	/* switch back to VCO PLL clock */
	ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets),
			 &ctrl_reg.Reg32);
	ctrl_reg.Bits.byp_wait = 0;
	WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets),
			  ctrl_reg.Reg32);

	return freqMHz;
}
#endif

int arch_cpu_init(void)
{
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	enable_ts0_counter();
#if defined(CONFIG_BCMBCA_DDRC)
	spl_ddrinit_prepare();
	/* enable unalgined access */
	set_sctlr(get_sctlr() & ~CR_A);
#endif
#endif
#if defined(CONFIG_TPL_BUILD)
	enable_ns_access();
	setup_ubus_rangechk();	
	cci500_enable();
#endif
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

int get_nr_cpus()
{
	unsigned int chipId = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
	int nr_cpus=QUAD_CPUS;

	if(chipId == DUAL_CORE_63152)
		nr_cpus=DUAL_CPUS;

	return nr_cpus;
}

#if !defined(CONFIG_TPL_ATF)
void boot_secondary_cpu(unsigned long vector)
{
	uint32_t cpu, nr_cpus = QUAD_CPUS;
	ARM_CONTROL_REG ctrl_reg;
	uint64_t rvbar = vector;

	nr_cpus = get_nr_cpus();

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
