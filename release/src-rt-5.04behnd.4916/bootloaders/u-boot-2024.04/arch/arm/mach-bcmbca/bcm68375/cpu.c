/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 Broadcom Ltd.
 */

#include <linux/types.h>
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
#endif
#include "tpl_params.h"
#include "bca_common.h"
#include "ba_svc.h"

DECLARE_GLOBAL_DATA_PTR;

#if !defined(CONFIG_SPL_BUILD)
void print_chipinfo(void)
{
	unsigned int chipId = bcmbca_get_chipid();
	unsigned int revId = bcmbca_get_chiprev();
	unsigned int rdp_speed, cpu_speed;

	printf("Chip ID: BCM%X_%X\n",chipId,revId);
	biu_ch_freq_get(0, &cpu_speed);
	printf("ARM Cortex A53 Dual Core: %dMHz\n", cpu_speed);

	get_rdp_freq(&rdp_speed);
	printf("RDP Freq: %dMHz\n", rdp_speed);
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

#ifdef CONFIG_DISABLE_CONSOLE
        gd->flags |= GD_FLG_DISABLE_CONSOLE;
#endif
#ifdef CONFIG_SILENT_CONSOLE
	gd->flags |= GD_FLG_SILENT;
#endif

    return 0;
}

int get_nr_cpus(void)
{
    uint32_t cores_num = 1;
    ba_get_cores_number(&cores_num);

    printf("Number of supports cores are:[%u]\n",cores_num);

    return cores_num;
}

#if !defined(CONFIG_TPL_ATF) 
#if !defined(CONFIG_BCMBCA_IKOS)

static uint8_t find_cpu_mask(uint32_t cpu_num)
{
    uint8_t idx;
    uint32_t cpu_mask = 0;

    for (idx = (cpu_num - 1); idx > 0; idx--)
        cpu_mask |= 1 << idx;

    printf("Core Mask is:[%u]\n",cpu_mask);
    return cpu_mask;
}

void boot_secondary_cpu(unsigned long vector)
{
    int stat;
    uint32_t nr_cpus = get_nr_cpus();

    printf("boot secondary cpu from 0x%lx\n", vector);
    stat = ba_svc_boot_secondary(find_cpu_mask(nr_cpus), vector);
    
    if(stat)
        printf("failed to boot secondary cpu - sts %d\n", stat);

    return;
}
#endif
#endif
