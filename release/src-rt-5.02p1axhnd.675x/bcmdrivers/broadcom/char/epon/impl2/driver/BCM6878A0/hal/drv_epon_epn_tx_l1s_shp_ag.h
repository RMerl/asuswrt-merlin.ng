/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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
bdmf_error_t ag_drv_epn_tx_l1s_shp_que_en_set(uint8_t shaper_idx, uint8_t cfgshpen);
bdmf_error_t ag_drv_epn_tx_l1s_shp_que_en_get(uint8_t shaper_idx, uint8_t *cfgshpen);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_epn_tx_l1s_shp_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

