/*
<:copyright-BRCM:2015:DUAL/GPL:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#include "cfe.h"
#include "lib_types.h"
#include "lib_string.h"
#include "lib_printf.h"
#include "bsp_config.h"
#include "bcm_map.h"
#include "cfe_iocb.h"
#include "pmc_drv.h"
#include "BPCM.h"
#include "bcm_otp.h"
#if defined(_BCM963138_)
#include "pmc_cpu_core.h"
#endif
#include "bcm63xx_util.h"

extern unsigned char sec_entry_begin;
extern unsigned char sec_entry_end;

extern void _cfe_flushcache(int, uint8_t *, uint8_t *);

static void powerup_secondary_cores(unsigned long vector)
{
#if defined(_BCM96846_) || defined(_BCM963138_) || defined(_BCM963148_)
    *(volatile uint32_t*)(BOOTLUT_BASE+0x20) = vector;
#endif
#if defined(_BCM963138_)
    unsigned int nr_cpus=0;
    get_nr_cpus(&nr_cpus);
    if(nr_cpus > 1)
        if (pmc_cpu_core_power_up(1))
            xprintf("failed to power up secondary cpu\n");
#elif defined(_BCM963148_)
    B15CTRL->cpu_ctrl.cpu1_pwr_zone_ctrl |= 0x400;
    B15CTRL->cpu_ctrl.reset_cfg &= 0xfffffffd;
#else
    uint32_t cpu, nr_cpus;
#if defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96856_) || defined(_BCM96846_)
    ARM_CONTROL_REG ctrl_reg;
#endif
#if defined(_BCM96858_)
    uint32_t rvbar = (uint32_t)vector;
#elif  defined(_BCM963158_) || defined(_BCM96856_)
    uint64_t rvbar = vector;
#endif

    if ( bcm_otp_get_nr_cpus(&nr_cpus) )
        return;

    nr_cpus = MAX_NUM_OF_CPU-nr_cpus;

    cpu = 1;
    while (cpu < nr_cpus)
    {
#if defined (_BCM96858_) || defined (_BCM963158_) || defined(_BCM96856_)
        // get cpu out of reset
        int stat;

#if defined (_BCM96858_)
        BIUCFG->cluster[0].rvbar_addr[cpu] = rvbar >> 8;
#else
        BIUCFG->cluster[0].rvbar_addr[cpu] = rvbar;
#endif
        stat = PowerOnDevice(PMB_ADDR_ORION_CPU0 + cpu);
        if (stat != kPMC_NO_ERROR)
            xprintf("failed to power on secondary cpu %d - sts %d\n", cpu, stat);
#elif defined(_BCM96846_)
        int stat;
#endif
#if defined(_BCM96858_) || defined(_BCM963158_) || defined(_BCM96856_) || defined(_BCM96846_)
        stat = ReadBPCMRegister(PMB_ADDR_BIU_BPCM, ARMBPCMRegOffset(arm_control), &ctrl_reg.Reg32);
        ctrl_reg.Bits.cpu_reset &= ~(0x1 << cpu);
        stat |= WriteBPCMRegister(PMB_ADDR_BIU_BPCM, ARMBPCMRegOffset(arm_control), ctrl_reg.Reg32);
        if (stat != kPMC_NO_ERROR)
            xprintf("failed to boot secondary cpu %d - sts $d\n", cpu, stat);
#endif

#if defined(_BCM94908_)
        BOOT_LUT->bootLutRst = (uint32_t)vector;

        BIUCTRL->power_cfg |= (0x1 << (cpu+BIU_CPU_CTRL_PWR_CFG_CPU0_BPCM_INIT_ON_SHIFT));
        BIUCTRL->reset_cfg &= ~(0x1 << cpu);
#endif
        cpu++;
    }
#endif
    return;
}


void cfe_boot_second_cpu (unsigned long vector)
{
    /* copy the secondary cpu boot strap code to the target memory */
    uint32_t copy_size = (uint32_t)(&sec_entry_end - &sec_entry_begin);
#if defined(CONFIG_ARM64)
    unsigned long vector_space;
    vector_space = vector + copy_size;
    /* Need to make sure, boot code does not step into PMC boot area */
    if ((vector_space > CFG_BOOT_PMC_ADDR) && (vector_space < CFG_BOOT_PMC_ADDR + CFG_BOOT_PMC_SIZE)) {
       xprintf("Error: %s Secondary cpu boot code overwriting PMC boot area\n", __func__);
       return;
    }
#endif
    memcpy((uint8_t*)vector, (uint8_t*)&sec_entry_begin, copy_size);

    _cfe_flushcache(CFE_CACHE_FLUSH_RANGE, (uint8_t *)vector,(uint8_t *)(vector+copy_size));

    powerup_secondary_cores(vector);

    return;
}
