/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/ddr.h>
#include <asm/arch/ubus4.h>
#include <spl.h>
#include "bcm_otp.h"
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#include "asm/arch/BPCM.h"
#include "clk_rst.h"
#endif
#include "bca_common.h"
#if defined(CONFIG_BCMBCA_UBUS4_DCM)
#include "bcm_ubus4.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_TPL_BUILD)
static void cci500_enable(void)
{
    /*Enable access from E2 and below */
    CCI500->secr_acc |= SECURE_ACCESS_UNSECURE_ENABLE;
}
#endif

int arch_cpu_init(void)
{
#if defined(CONFIG_BCMBCA_IKOS)
	icache_enable();
#endif  
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
    u32 frq = COUNTER_FREQUENCY;

    // set arch timer frequency
    asm volatile("mcr p15, 0, %0, c14, c0, 0" : : "r" (frq));

    // enable system timer
    BIUCFG->TSO_CNTCR |= 1;

    /* enable unalgined access */    
    set_cr(get_cr() & ~CR_A);
#endif

#if defined(CONFIG_TPL_BUILD)
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

void boost_cpu_clock(void)
{
    int stat = 0;
    uint32_t chipid;
    PLL_CTRL_REG ctrl_reg;

    chipid = bcmbca_get_chipid();
    if ((chipid==0x68252) || (chipid==0x68252c) || (chipid==0x68252d) || (chipid==0x68252e))
    {
        stat = ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
        ctrl_reg.Bits.byp_wait = 1;
        stat |= WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);

        stat |= pll_vco_config(PMB_ADDR_BIU_PLL ,60, 1);
        stat |= pll_ch_freq_set(PMB_ADDR_BIU_PLL, 0, 2);

        stat = ReadBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), &ctrl_reg.Reg32);
        ctrl_reg.Bits.byp_wait = 0;
        stat |= WriteBPCMRegister(PMB_ADDR_BIU_PLL, PLLBPCMRegOffset(resets), ctrl_reg.Reg32);
    }
    else if ((chipid==0x68552) || (chipid==0x68552c) || (chipid==0x6753))
    {
        stat = pll_ch_freq_set(PMB_ADDR_BIU_PLL, 0, 2);
    }
    else
    {
        printf("ERROR: Wrong CHIP ID!!!\n");
        while(1);
    }

    if (stat)
        printf("Error: failed to set cpu clock\n");

#if defined(CONFIG_BCMBCA_UBUS4_DCM)
    //configure ubus clock
    bcm_ubus4_dcm_clk_bypass(1);
#endif

    pll_ch_freq_set(PMB_ADDR_SYSPLL, 3, 14);
}

u32 bcmbca_get_chipid(void)
{
	u32 chipId = (PERF->RevID & CHIP_ID_MASK) >> CHIP_ID_SHIFT;
	u32 chipIdLC = (TOP->OtpChipidLC & CHIP_ID_LC_MASK);

	if (chipIdLC)
		chipId = (chipId << CHIP_ID_LC_SIZE) | chipIdLC;
	
	return chipId;
}

#if !defined(CONFIG_SPL_BUILD)
void print_chipinfo(void)
{
	unsigned int cpu_speed, rdp_speed;
	unsigned int chipId = bcmbca_get_chipid();
	unsigned int revId = bcmbca_get_chiprev();

	if (chipId == 0x68252d)
		printf("Chip ID: BCM%s_%X\n", "68252R", revId);
	else if (chipId == 0x68252e)
		printf("Chip ID: BCM%s_%X\n", "68252CR", revId);
	else
		printf("Chip ID: BCM%X_%X\n",chipId,revId);

	pll_ch_freq_get(PMB_ADDR_BIU_PLL, 0, &cpu_speed);
	if (chipId==0x68552 || chipId==0x06753)
		printf("ARM Cortex A7 Triple Core: %dMHz\n", cpu_speed);
	else
		printf("ARM Cortex A7 Dual Core: %dMHz\n", cpu_speed);

	get_rdp_freq(& rdp_speed);
	printf("RDP Freq: %dMHz\n", rdp_speed);
}
#endif

#if !defined(CONFIG_TPL_ATF)
void boot_secondary_cpu(unsigned long vector)
{
    uint32_t cpu = 1;
    uint32_t nr_cpus;
    ARM_CONTROL_REG ctrl_reg;
    uint64_t rvbar = vector;
    int stat;
    unsigned int chipId = bcmbca_get_chipid();

    printf("boot secondary cpu from 0x%lx\n", vector);

    *(volatile uint32_t*)(BOOTLUT_BASE+0x20) = vector;

    if ((chipId==0x68552) || (chipId==0x6753))
        nr_cpus = 3;
    else
        nr_cpus = 2;

    while (cpu < nr_cpus)
    {
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

