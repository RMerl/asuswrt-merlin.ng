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
#include "xrdp_drv_psram_ag.h"

int drv_psram_config_clock_autogate(bdmf_boolean auto_gate, uint8_t timer_val)
{
     psram_configurations_clk_gate_cntrl psram_ctrl;

     ag_drv_psram_configurations_clk_gate_cntrl_get(&psram_ctrl);
     psram_ctrl.bypass_clk_gate = auto_gate ? 0 : 1;
     psram_ctrl.timer_val = timer_val;
     ag_drv_psram_configurations_clk_gate_cntrl_set(&psram_ctrl);

     return 0;
}
