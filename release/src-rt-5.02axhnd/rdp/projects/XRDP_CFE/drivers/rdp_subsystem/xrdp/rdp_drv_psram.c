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
