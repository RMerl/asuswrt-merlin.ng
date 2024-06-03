// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Broadcom
 */

/*
 * pmc_xport.c
 *
 *  Created on  May 2022
 *      Author: Yuval Raviv
 */

#include "pmc_drv.h"
#include "asm/arch/BPCM.h"

int pmc_xport_power_on(int xport_id)
{
#if IS_BCMCHIP(4912) || IS_BCMCHIP(6813) || IS_BCMCHIP(6888) || IS_BCMCHIP(68880) || IS_BCMCHIP(6837)
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
#if IS_BCMCHIP(6888) || IS_BCMCHIP(68880) || IS_BCMCHIP(6837)
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
