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

#if defined (_BCM96858_) 
#define RVBARADDR0 (volatile uint32_t *)0x81060120
#endif

extern unsigned char sec_entry_begin;
extern unsigned char sec_entry_end;

extern void _cfe_flushcache(int, uint8_t *, uint8_t *);

static void powerup_secondary_cores(unsigned long vector)
{
    uint32_t cpu, nr_cpus;
#if defined (_BCM96858_) 
    int stat;
    ARM_CONTROL_REG ctrl_reg;
#endif

    if ( bcm_otp_get_nr_cpus(&nr_cpus) )
        return;
    nr_cpus = 4-nr_cpus;

    cpu = 1;
    while (cpu < nr_cpus)
    {
#if defined (_BCM96858_) 
        // get cpu out of reset
        *(RVBARADDR0 + cpu) = (uint32_t)vector;
        stat = PowerOnDevice(PMB_ADDR_ORION_CPU0 + cpu);
        if (stat != kPMC_NO_ERROR)
            xprintf("failed to power on secondary cpu %d - sts %d\n", cpu, stat);

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

    return;
}


void cfe_boot_second_cpu (unsigned long vector)
{
    uint32_t copy_size;

    /* copy the secondary cpu boot strap code to the target memory */
    copy_size = (uint32_t)(&sec_entry_end - &sec_entry_begin);
    memcpy((uint8_t*)vector, (uint8_t*)&sec_entry_begin, copy_size);

    _cfe_flushcache(CFE_CACHE_FLUSH_RANGE, (uint8_t *)vector,(uint8_t *)(vector+copy_size));

    powerup_secondary_cores(vector);

    return;
}
