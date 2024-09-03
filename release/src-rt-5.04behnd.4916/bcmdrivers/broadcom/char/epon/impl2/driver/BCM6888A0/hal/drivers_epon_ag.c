/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
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

#include "drivers_epon_ag.h"
#include "drv_epon_clk_prg_swch_addr_ag.h"
#include "drv_epon_epn_ag.h"
#include "drv_epon_epn_onu_mac_addr_ag.h"
#include "drv_epon_epn_tx_l1s_shp_ag.h"
#include "drv_epon_epon_top_ag.h"
#include "drv_epon_lif_ag.h"
#include "drv_epon_nco_addr_ag.h"
#include "drv_epon_xif_ag.h"
#include "drv_epon_xpcsrx_ag.h"
#include "drv_epon_xpcstx_ag.h"


/* get_test_method_value */
uint32_t gtmv(bdmf_test_method method, uint8_t bits)
{
    switch (method)
    {
    case bdmf_test_method_mid :
        return 1 << (bits - 1);
    case bdmf_test_method_high :
        /* todo need to fix this, the problem was that ((1<<32)-1)=0 ???, it should be 0xFFFFFFFF */
        return bits == 32 ? UINT32_MAX : (1 << bits) - 1;
    case bdmf_test_method_low :
    default :
        return 0;
    }
    return 0;
}

#ifdef USE_BDMF_SHELL
void register_epon_drv_shell_commands(void)
{
    bdmfmon_handle_t driver_dir, epon_dir;

    driver_dir = bdmfmon_dir_find(NULL, "driver");

    if (!driver_dir)
    {
        driver_dir = bdmfmon_dir_add(NULL, "driver", "Device Drivers", BDMF_ACCESS_ADMIN, NULL);

        if (!driver_dir)
            return;
    }

    epon_dir = bdmfmon_dir_add(driver_dir, "epon_drv", "Epon Device Driver", BDMF_ACCESS_ADMIN, NULL);

    if (!epon_dir)
        return;
    
    ag_drv_clk_prg_swch_addr_cli_init(epon_dir);
    ag_drv_epn_cli_init(epon_dir);
    ag_drv_epn_onu_mac_addr_cli_init(epon_dir);
    ag_drv_epn_tx_l1s_shp_cli_init(epon_dir);
    ag_drv_epon_top_cli_init(epon_dir);
    ag_drv_lif_cli_init(epon_dir);
    ag_drv_nco_addr_cli_init(epon_dir);
    ag_drv_xif_cli_init(epon_dir);
    ag_drv_xpcsrx_cli_init(epon_dir);
    ag_drv_xpcstx_cli_init(epon_dir);
}
#endif


