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
