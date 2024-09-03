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

#ifndef _DRV_EPON_CLK_PRG_SWCH_ADDR_AG_H_
#define _DRV_EPON_CLK_PRG_SWCH_ADDR_AG_H_

#include "access_macros.h"
#if !defined(_CFE_)
#include "bdmf_interface.h"
#else
#include "bdmf_data_types.h"
#include "bdmf_errno.h"
#endif
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
typedef struct
{
    uint16_t cfgppsoffset;
    uint8_t cfgppstrackgain;
    bdmf_boolean cfgoutclkinv;
    bdmf_boolean cfgppstracken;
    bdmf_boolean cfgppsalignen;
    bdmf_boolean cfgppsselect;
} clk_prg_swch_addr_clk_prg_config3;

bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config_set(uint8_t cfgprgclksel, uint32_t cfgprgclkdivide);
bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config_get(uint8_t *cfgprgclksel, uint32_t *cfgprgclkdivide);
bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config2_set(uint16_t cfgprgclkdenom);
bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config2_get(uint16_t *cfgprgclkdenom);
bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config_1_set(uint8_t cfgprgclksel_1, uint32_t cfgprgclkdivide_1);
bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config_1_get(uint8_t *cfgprgclksel_1, uint32_t *cfgprgclkdivide_1);
bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config2_1_set(uint16_t cfgprgclkdenom_1);
bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config2_1_get(uint16_t *cfgprgclkdenom_1);
bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config3_set(const clk_prg_swch_addr_clk_prg_config3 *clk_prg_config3);
bdmf_error_t ag_drv_clk_prg_swch_addr_clk_prg_config3_get(clk_prg_swch_addr_clk_prg_config3 *clk_prg_config3);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_clk_prg_swch_addr_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

