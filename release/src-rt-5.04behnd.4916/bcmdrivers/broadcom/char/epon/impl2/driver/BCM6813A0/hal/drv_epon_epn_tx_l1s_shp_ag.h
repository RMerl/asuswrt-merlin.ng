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

#ifndef _DRV_EPON_EPN_TX_L1S_SHP_AG_H_
#define _DRV_EPON_EPN_TX_L1S_SHP_AG_H_

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
bdmf_error_t ag_drv_epn_tx_l1s_shp_config_set(uint8_t shaper_idx, uint32_t cfgshprate, uint8_t cfgshpbstsize);
bdmf_error_t ag_drv_epn_tx_l1s_shp_config_get(uint8_t shaper_idx, uint32_t *cfgshprate, uint8_t *cfgshpbstsize);
bdmf_error_t ag_drv_epn_tx_l1s_shp_que_en_set(uint8_t shaper_idx, uint16_t cfgshpen);
bdmf_error_t ag_drv_epn_tx_l1s_shp_que_en_get(uint8_t shaper_idx, uint16_t *cfgshpen);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_epn_tx_l1s_shp_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

