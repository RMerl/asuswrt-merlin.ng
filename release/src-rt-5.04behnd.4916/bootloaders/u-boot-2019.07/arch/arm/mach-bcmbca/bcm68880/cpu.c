/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Broadcom Ltd.
 */

#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/misc.h>
#include <asm/io.h>
#include <spl.h>
#if defined(CONFIG_BCMBCA_DDRC)
#include <asm/arch/ddr.h>
#include "spl_ddrinit.h"
#endif
#if defined(CONFIG_BCMBCA_PMC)
#include "pmc_drv.h"
#include "clk_rst.h"
#include "asm/arch/BPCM.h"
#endif
#include "tpl_params.h"
#include "bca_common.h"
#include "ba_svc.h"

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD) && defined(CONFIG_BCMBCA_NO_SMC_BOOT) 
static void enable_ts0_couner(void)
{
	BIUCFG->ts0_ctrl.CNTCR |= 0x1;
}
#elif defined(CONFIG_TPL_BUILD) && defined(CONFIG_BCMBCA_NO_SMC_BOOT) 
static void enable_ns_access(void)
{
	BIUCFG->aux.aux_permission |= 0xff;
	BIUCFG->ubus.ubus_permission |= 0xff;

	/*Enable access from E2 and below */
	CCI500->secr_acc |= SECURE_ACCESS_UNSECURE_ENABLE;
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
	unsigned int chipId = bcmbca_get_chipid();
	unsigned int revId = bcmbca_get_chiprev();
	unsigned int rdp_speed, cpu_speed, xrdp_freq;

	printf("Chip ID: BCM%X_%X\n",chipId,revId);
    biu_ch_freq_get(0, &cpu_speed);
	printf("ARM Cortex A55 Quad Core: %dMHz\n", cpu_speed);

	get_rdp_freq(&rdp_speed);
	printf("RDP Freq: %dMHz\n", rdp_speed);

#if defined(CONFIG_BCMBCA_NO_SMC_BOOT) 
    pll_ch_freq_get(PMB_ADDR_RDPPLL, XRDPPLL_XRDP_CHANNEL, &xrdp_freq);
#else
    get_xrdp_freq(&xrdp_freq);
#endif
	printf("XRDP Freq: %dMHz\n", xrdp_freq);

}
#endif

#if defined(CONFIG_BCMBCA_PMC)
void boost_cpu_clock(void)
{
}
#endif

int arch_cpu_init(void)
{
#if defined(CONFIG_BCMBCA_IKOS)
	icache_enable();
#endif
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD) && defined(CONFIG_BCMBCA_NO_SMC_BOOT) 
	enable_ts0_couner();
	/* enable unalgined access */
	set_sctlr(get_sctlr() & ~CR_A);
#endif
#if defined(CONFIG_TPL_BUILD) && defined(CONFIG_BCMBCA_NO_SMC_BOOT) 
	enable_ns_access();
	setup_ubus_rangechk();	
	// watchdog0 enabling, temporary code only for XIP mode 
	*((volatile unsigned int*)0x842100BC) |= 0x5; // turn on UPG_WDT0_RESET_EN and SW_MASTER_RESET_EN bits 
#endif

#ifdef CONFIG_DISABLE_CONSOLE
        gd->flags |= GD_FLG_DISABLE_CONSOLE;
#endif
#ifdef CONFIG_SILENT_CONSOLE
	gd->flags |= GD_FLG_SILENT;
#endif

    return 0;
}

#if !defined(CONFIG_TPL_ATF) 
#if !defined(CONFIG_BCMBCA_IKOS)
void boot_secondary_cpu(unsigned long vector)
{
	int stat;

#ifndef CONFIG_BCMBCA_NO_SMC_BOOT
#define CPU_MASK	0xe
	printf("boot secondary cpu from 0x%lx\n", vector);
	stat = ba_svc_boot_secondary(CPU_MASK, vector);
	if(stat)
#else
	uint32_t cpu, nr_cpus = 4;
	ARM_CONTROL_REG ctrl_reg;
	uint64_t rvbar = vector;
	
	stat = ReadBPCMRegister(PMB_ADDR_BIU_BPCM, ARMBPCMRegOffset(arm_control), &ctrl_reg.Reg32);
	cpu = 1;
	while (cpu < nr_cpus) {

		BIUCFG->cluster[0].rvbar_addr[cpu] = rvbar;

		ctrl_reg.Bits.cpu_reset_n &= ~(0x1 << cpu);
		cpu++;
	}

	stat |= WriteBPCMRegister(PMB_ADDR_BIU_BPCM, ARMBPCMRegOffset(arm_control), ctrl_reg.Reg32);
	if (stat != kPMC_NO_ERROR)
#endif
		printf("failed to boot secondary cpus - sts %d\n", stat);

	return;
}
#endif
#endif
