/*
<:copyright-BRCM:2018:DUAL/GPL:standard 

   Copyright (c) 2018 Broadcom 
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
#ifndef _CFE_
//#include <linux/init.h>
#include <linux/kernel.h>
//#include <linux/module.h>
//#include <linux/delay.h>
#else
#include "lib_printf.h"
#include "lib_types.h"
#endif

#ifndef _CFE_
#define PRINTK	printk
#else
#define PRINTK	xprintf
#endif

#include "pmc_drv.h"
#include "pmc_sysport.h"
#include "BPCM.h"
#include "boardparms.h"

int pmc_sysport_power_up(void)
{
    PowerOnDevice(PMB_ADDR_SYSP);

    // reset system port through BPCM
    pmc_sysport_reset_system_port(0);
#if defined(_BCM947622_) || defined(CONFIG_BCM947622)
    pmc_sysport_reset_system_port(1);
#endif

    return 0;
}

int pmc_sysport_power_down(void)
{
    return PowerOffDevice(PMB_ADDR_SYSP, 0);
}

void pmc_sysport_reset_system_port (int port)
{
    int status;
    uint32_t reg;
    int offset;

    // only sysport 0 and 1 is expect to be used in CFE, reject non port reset request
    if (port == 0)
    {
        offset=SYSPRegOffset(z1_pm_cntl);
    }
#if defined(_BCM947622_) || defined(CONFIG_BCM947622)
    else if (port == 1)
    {
        offset=SYSPRegOffset(z2_pm_cntl);
    }
#endif
    else
    {
        PRINTK("unexpected system port %d reset\n", port);
        return;
    }

    status = ReadBPCMRegister(PMB_ADDR_SYSP, offset, &reg);
    if (status == kPMC_NO_ERROR)
    {
        // toggle sw_init field (bit 0)
        reg |= 0x1;
        status = WriteBPCMRegister(PMB_ADDR_SYSP, offset, reg);

        if (status == kPMC_NO_ERROR)
        {
            reg &= ~0x1;
            status = WriteBPCMRegister(PMB_ADDR_SYSP, offset, reg);
        }
    }

		
}

#ifndef _CFE_
EXPORT_SYMBOL(pmc_sysport_power_up);
EXPORT_SYMBOL(pmc_sysport_power_down);
#endif

