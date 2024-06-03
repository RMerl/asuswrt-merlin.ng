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

uint32_t bcmbca_is_ccb(void)
{
	return (BIUCFG->ubus.status & BIUCFG_UBUS_CCB_MOD_SEL);
}
DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)

static void enable_ts0_couner(void)
{
	BIUCFG->ts0_ctrl.CNTCR |= 0x1;
}
#elif defined(CONFIG_TPL_BUILD)
static void enable_ns_access(void)
{
	BIUCFG->aux.aux_permission |= 0xff;
	BIUCFG->ubus.ubus_permission |= 0xff;

	if (bcmbca_is_ccb())
		CCB->gen_perm_ctrl |= 0xff;
	else
		/*Enable access from E2 and below */
		CCI500->secr_acc |= SECURE_ACCESS_UNSECURE_ENABLE;
}

static void setup_ubus_rangechk(void)
{
	if (!bcmbca_is_ccb()) {
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
}
#endif

#if !defined(CONFIG_SPL_BUILD)
void print_chipinfo(void)
{
	unsigned int rdp_speed, cpu_speed;
	unsigned int chipId = bcmbca_get_chipid();
	unsigned int revId = bcmbca_get_chiprev();

	printf("Chip ID: BCM%XX_%X\n",chipId,revId);

	pll_ch_freq_get(PMB_ADDR_BIU_PLL, 0, &cpu_speed);
	printf("ARM Cortex A53 Quad Core: %dMHz\n", cpu_speed);

	get_rdp_freq(& rdp_speed);
	printf("RDP Freq: %dMHz\n", rdp_speed);

	if (bcmbca_is_ccb())
		printf("CCB is used\n");
	else
		printf("CCI is used\n");
}
#endif

#if defined(CONFIG_BCMBCA_PMC)
void boost_cpu_clock(void)
{
	printf("set cpu freq to 2.2GHz\n");
	pll_ch_freq_set(PMB_ADDR_BIU_PLL, 0, 2);
}
#endif

int arch_cpu_init(void)
{
#if defined(CONFIG_BCMBCA_IKOS)
	icache_enable();
#endif
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
	setup_ubus_rangechk();	
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
void boot_secondary_cpu(unsigned long vector)
{
	uint32_t cpu, nr_cpus = 4;
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
