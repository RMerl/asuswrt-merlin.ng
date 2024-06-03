/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom Ltd.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ubus4.h>
#include "bcm_otp.h"
#include <spl.h>
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#include "asm/arch/BPCM.h"
#include "clk_rst.h"
#endif
#include "spl_ddrinit.h"
#include "bca_common.h"
#if defined(CONFIG_BCMBCA_UBUS4_DCM)
#include "bcm_ubus4.h"
#endif

DECLARE_GLOBAL_DATA_PTR;


#if defined(CONFIG_TPL_BUILD)
static void cci400_enable(void)
{
	CCI400->secr_acc |= SECURE_ACCESS_UNSECURE_ENABLE;
}

static void enable_ns_access(void)
{
	BIUCFG->aux.permission |= 0xff;
}
#endif

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
void bcm_setsw(void)
{
    swr_write(0,3,0x5372);
    swr_write(0,6,0xb000);
    swr_write(0,7,0x0029);
    swr_write(1,3,0x5370);
    swr_write(1,7,0x0029);
    swr_write(2,3,0x5370);
    swr_write(2,7,0x0029);
    swr_write(3,3,0x5370);
    swr_write(3,7,0x0029);
}
#endif

#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_TPL_BUILD)
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
#else
void disable_xtal_clk(void);
#endif

void boost_cpu_clock(void)
{
    unsigned int clk_index, cpu_clock;
    int stat;
    PLL_CTRL_REG ctrl_reg;

#if defined(CONFIG_BCMBCA_UBUS4_DCM)
    //configure ubus clock
    bcm_ubus4_dcm_clk_bypass(1);
#endif

    if ( !bcm_otp_get_cpu_clk(&clk_index) )
        cpu_clock = 500 + 500*(2-clk_index);
        else
        cpu_clock = 0;

    stat = ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLCLASSICBPCMRegOffset(resets), &ctrl_reg.Reg32);
        ctrl_reg.Bits.byp_wait = 0;
        stat |= WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLCLASSICBPCMRegOffset(resets), ctrl_reg.Reg32);

    if (stat)
        printf("Error: failed to set cpu fast mode\n");

    printf("CPU Clock: %dMHz\n", cpu_clock);

    disable_xtal_clk();
}

int arch_cpu_init(void)
{
#if defined(CONFIG_BCMBCA_IKOS)
	icache_enable();
#endif  
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
	spl_ddrinit_prepare();
	/* enable unalgined access */	
	set_sctlr(get_sctlr() & ~CR_A);	
#endif

#if defined(CONFIG_TPL_BUILD)
	enable_ns_access();
	cci400_enable();
#endif

#ifdef CONFIG_DISABLE_CONSOLE
        gd->flags |= GD_FLG_DISABLE_CONSOLE;
#endif
#ifdef CONFIG_SILENT_CONSOLE
	gd->flags |= GD_FLG_SILENT;
#endif

    return 0;
}

u32 bcmbca_get_chipid(void)
{
	unsigned int chipId = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
	unsigned int chipvar;

	if ( !bcm_otp_get_chipid(&chipvar) )
	{
		if ((chipId==0x68560) && (chipvar==3))
			chipId=0x68560B;
	}
	return chipId;
}

#if !defined(CONFIG_SPL_BUILD)
void print_chipinfo(void)
{
	char *nr_cores = NULL;
	unsigned int cpu_speed, rdp_speed, otp_cores;
	unsigned int chipId = bcmbca_get_chipid();
	unsigned int revId = bcmbca_get_chiprev();

	printf("Chip ID: BCM%X_%X\n",chipId,revId);

	if ( !bcm_otp_get_nr_cpus(&otp_cores) )
	{
		if (otp_cores == 0)
			nr_cores = "Dual";
		else if (otp_cores == 1)
			nr_cores = "Single";
	}

	pll_ch_freq_get(PMB_ADDR_BIU_PLL, 0, &cpu_speed);
	get_rdp_freq(&rdp_speed);

	printf("Broadcom B53 %s Core: %dMHz\n", nr_cores, cpu_speed);
	printf("RDP: %dMHz\n",rdp_speed);
}
#endif

#if !defined(CONFIG_TPL_ATF)
void boot_secondary_cpu(unsigned long vector)
{
	uint32_t cpu = 1; 
	uint32_t nr_cpus = 2;
	ARM_CONTROL_REG ctrl_reg;

	printf("boot secondary cpu from 0x%lx\n", vector);

	while (cpu < nr_cpus) {
		int stat;

		BIUCFG->cluster[0].rvbar_addr[cpu] = vector;
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
