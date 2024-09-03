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
#include "pmc_wan.h"
#include "BPCM.h"

int sgmii_bpcm_init(void)
{
    uint32_t data;

    pmc_wan_interface_power_control(WAN_INTF_MAIN, 1);

    ReadBPCMRegister(PMB_ADDR_WAN, 0x10, &data);
    data |= (1 << 5); // sgmii_z2_serdes_reset_n
    WriteBPCMRegister(PMB_ADDR_WAN, 0x10, data);

    return 0;
}

