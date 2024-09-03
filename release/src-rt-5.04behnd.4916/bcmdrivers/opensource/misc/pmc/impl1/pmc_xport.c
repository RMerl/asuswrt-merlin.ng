/*
<:copyright-BRCM:2020:DUAL/GPL:standard 

   Copyright (c) 2020 Broadcom 
   All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:> 
*/

#include "pmc_drv.h"
#include "BPCM.h"

int pmc_xport_power_on(int xport_id)
{
#if defined(CONFIG_BCM94912) || defined(CONFIG_BCM96813) || defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
    uint32_t data;
    int port_offset;

    switch (xport_id)
    {
        case 0:
            port_offset = BPCMETHRegOffset(xport0_cntrl);
            break;
        case 1:
            port_offset = BPCMETHRegOffset(xport1_cntrl);
            break;
#if defined(CONFIG_BCM96888) || defined(CONFIG_BCM968880) || defined(CONFIG_BCM96837)
        case 2:
            port_offset = BPCMETHRegOffset(xport2_cntrl);
            break;
#endif
        default:
            return kPMC_INVALID_DEVICE;
    }
    // tsclk_clk_en(b6)=1 data_path_cclk_clk_en(b5)=1 cclk_clk_en(b4)=1 tsc_clk_gated_clk_en(b3)=1 tsc_clk_en(b2)=1 sys_clk_en(b1)=1 sw_init(b0)=0 

    data = 0x7e;  
    WriteBPCMRegister(PMB_ADDR_ETH, port_offset, data);
#endif
    return kPMC_NO_ERROR;
}
