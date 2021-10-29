/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
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
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_bac_if_ag.h"

int drv_bac_if_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val)
{
     bac_if_bacif_block_bacif_configurations_clk_gate_cntrl bacif_ctrl;
     uint8_t block_id = 0;

     for (block_id = 0; block_id < RU_BLK(BAC_IF).addr_count; block_id++){
#if defined(BCM6836) || defined(BCM63158)
         /* DO NOT auto clock gate BACIF #3 which is connected to the NATC (Nat Cache).
            Currently NATC is not supporting auto clock gating mode.
         */
         if (block_id == 3) continue;
#endif
         ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_get(block_id, &bacif_ctrl);
         bacif_ctrl.bypass_clk_gate = auto_gate ? 0 : 1;
         bacif_ctrl.timer_val = timer_val;
         ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_set(block_id, &bacif_ctrl);
     }
     return 0;
}
