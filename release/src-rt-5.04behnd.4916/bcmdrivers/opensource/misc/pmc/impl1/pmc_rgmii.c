/*
<:copyright-BRCM:2013:DUAL/GPL:standard

   Copyright (c) 2013 Broadcom 
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

void pmc_rgmii_clk_en(void)
{
#if IS_BCMCHIP(63146)
    // turn on RGMII clk 250 en
    uint32_t data;
    ReadBPCMRegister(PMB_ADDR_EGPHY, BPCMETHRegOffset(rgmii_cntrl), &data);
    data |= (1 << 1);  // bpcm_clk_en
    WriteBPCMRegister(PMB_ADDR_EGPHY, BPCMETHRegOffset(rgmii_cntrl), data);
#endif
#if IS_BCMCHIP(4912) || defined(CONFIG_BCM96888)
    uint32_t data;
    ReadBPCMRegister(PMB_ADDR_ETH, BPCMETHRegOffset(rgmii_cntrl), &data);
    // bpcm_pwrdown_n(b4)=1 bpcm_reset_n(b3)=1 bpcm_rgmii_en(b2)=0 bpcm_clk_en(b1)=1 bpcm_mux_sel(b0)=0
    data |= (1 << 1);  // bpcm_clk_en
    WriteBPCMRegister(PMB_ADDR_ETH, BPCMETHRegOffset(rgmii_cntrl), data);
#endif
}

EXPORT_SYMBOL(pmc_rgmii_clk_en);
