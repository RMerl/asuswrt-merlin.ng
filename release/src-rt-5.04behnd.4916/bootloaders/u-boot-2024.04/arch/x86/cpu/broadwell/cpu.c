// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016 Google, Inc
 *
 * Based on code from coreboot src/soc/intel/broadwell/cpu.c
 */

#include <common.h>
#include <dm.h>
#include <cpu.h>
#include <event.h>
#include <init.h>
#include <log.h>
#include <spl.h>
#include <asm/cpu.h>
#include <asm/cpu_x86.h>
#include <asm/cpu_common.h>
#include <asm/global_data.h>
#include <asm/intel_regs.h>
#include <asm/lpc_common.h>
#include <asm/msr.h>
#include <asm/pci.h>
#include <asm/post.h>
#include <asm/turbo.h>
#include <asm/arch/cpu.h>
#include <asm/arch/pch.h>
#include <asm/arch/rcb.h>

static int broadwell_init_cpu(void)
{
	struct udevice *dev;
	int ret;

	/* Start up the LPC so we have serial */
	ret = uclass_first_device_err(UCLASS_LPC, &dev);
	if (ret)
		return ret;
	ret = cpu_set_flex_ratio_to_tdp_nominal();
	if (ret)
		return ret;

	return 0;
}
EVENT_SPY_SIMPLE(EVT_DM_POST_INIT_F, broadwell_init_cpu);

void set_max_freq(void)
{
	msr_t msr, perf_ctl;

	if (cpu_config_tdp_levels()) {
		/* Set to nominal TDP ratio */
		msr = msr_read(MSR_CONFIG_TDP_NOMINAL);
		perf_ctl.lo = (msr.lo & 0xff) << 8;
	} else {
		/* Platform Info bits 15:8 give max ratio */
		msr = msr_read(MSR_PLATFORM_INFO);
		perf_ctl.lo = msr.lo & 0xff00;
	}

	perf_ctl.hi = 0;
	msr_write(MSR_IA32_PERF_CTL, perf_ctl);

	debug("CPU: frequency set to %d MHz\n",
	      ((perf_ctl.lo >> 8) & 0xff) * INTEL_BCLK_MHZ);
}

int arch_cpu_init(void)
{
	post_code(POST_CPU_INIT);

	/* Do a mini-init if TPL has already done the full init */
	if (IS_ENABLED(CONFIG_TPL) && spl_phase() != PHASE_TPL)
		return x86_cpu_reinit_f();
	else
		return x86_cpu_init_f();
}

int checkcpu(void)
{
	int ret;

	set_max_freq();

	ret = cpu_common_init();
	if (ret)
		return ret;
	gd->arch.pei_boot_mode = PEI_BOOT_NONE;

	return 0;
}

int print_cpuinfo(void)
{
	char processor_name[CPU_MAX_NAME_LEN];
	const char *name;

	/* Print processor name */
	name = cpu_get_name(processor_name);
	printf("CPU:   %s\n", name);

	return 0;
}

void board_debug_uart_init(void)
{
	/* com1 / com2 decode range */
	pci_x86_write_config(PCH_DEV_LPC, LPC_IO_DEC, 1 << 4, PCI_SIZE_16);

	pci_x86_write_config(PCH_DEV_LPC, LPC_EN, COMA_LPC_EN, PCI_SIZE_16);
}
