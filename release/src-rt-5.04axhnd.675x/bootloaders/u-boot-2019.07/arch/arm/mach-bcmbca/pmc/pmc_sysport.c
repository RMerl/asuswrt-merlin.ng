// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2018 Broadcom
 */
/* SPDX-License-Identifier: GPL-2.0+
 *
 * Copyright 2020 Broadcom Ltd.
 */
/*
 
*/
#include "pmc_drv.h"
#include "asm/arch/BPCM.h"


int pmc_sysport_power_up(void)
{
    PowerOnDevice(PMB_ADDR_SYSP);

    // reset system port through BPCM
    pmc_sysport_reset_system_port(0);
//#if defined(_BCM947622_) || defined(CONFIG_BCM947622)
    pmc_sysport_reset_system_port(1);
//#endif

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

	printf("sysport reset %d\n", port);

    // only sysport 0 and 1 is expect to be used in CFE, reject non port reset request
    if (port == 0)
    {
        offset=SYSPRegOffset(z1_pm_cntl);
    }
//#if defined(_BCM947622_) || defined(CONFIG_BCM947622)
    else if (port == 1)
    {
        offset=SYSPRegOffset(z2_pm_cntl);
    }
//#endif
    else
    {
        printf("unexpected system port %d reset\n", port);
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

