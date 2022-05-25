/*
<:copyright-BRCM:2020:DUAL/GPL:standard 

   Copyright (c) 2020 Broadcom 
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

#include "pmc_drv.h"
#include "BPCM.h"

int pmc_xport_power_on(int xport_id)
{
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
        default:
            return kPMC_INVALID_DEVICE;
    }
    // tsclk_clk_en(b6)=1 data_path_cclk_clk_en(b5)=1 cclk_clk_en(b4)=1 tsc_clk_gated_clk_en(b3)=1 tsc_clk_en(b2)=1 sys_clk_en(b1)=1 sw_init(b0)=0 

    data = 0x7e;  
    WriteBPCMRegister(PMB_ADDR_ETH, port_offset, data);
    return kPMC_NO_ERROR;
}
