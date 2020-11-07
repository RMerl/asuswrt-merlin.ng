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

#ifndef _BCM6858_WAN_SERDES_AG_H_
#define _BCM6858_WAN_SERDES_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif

/**************************************************************************************************/
/* cfg_pll1_lcref_sel:  -                                                                         */
/* cfg_pll1_refout_en:  -                                                                         */
/* cfg_pll1_refin_en:  -                                                                          */
/* cfg_pll0_lcref_sel:  -                                                                         */
/* cfg_pll0_refout_en:  -                                                                         */
/* cfg_pll0_refin_en:  -                                                                          */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean cfg_pll1_lcref_sel;
    bdmf_boolean cfg_pll1_refout_en;
    bdmf_boolean cfg_pll1_refin_en;
    bdmf_boolean cfg_pll0_lcref_sel;
    bdmf_boolean cfg_pll0_refout_en;
    bdmf_boolean cfg_pll0_refin_en;
} wan_serdes_pll_ctl;


/**************************************************************************************************/
/* cfg_pram_we:  -                                                                                */
/* cfg_pram_cs:  -                                                                                */
/* cfg_pram_ability:  -                                                                           */
/* cfg_pram_datain:  -                                                                            */
/* cfg_pram_addr:  -                                                                              */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean cfg_pram_we;
    bdmf_boolean cfg_pram_cs;
    bdmf_boolean cfg_pram_ability;
    uint8_t cfg_pram_datain;
    uint16_t cfg_pram_addr;
} wan_serdes_pram_ctl;

bdmf_error_t ag_drv_wan_serdes_pll_ctl_set(const wan_serdes_pll_ctl *pll_ctl);
bdmf_error_t ag_drv_wan_serdes_pll_ctl_get(wan_serdes_pll_ctl *pll_ctl);
bdmf_error_t ag_drv_wan_serdes_temp_ctl_get(uint16_t *wan_temperature_data);
bdmf_error_t ag_drv_wan_serdes_pram_ctl_set(const wan_serdes_pram_ctl *pram_ctl);
bdmf_error_t ag_drv_wan_serdes_pram_ctl_get(wan_serdes_pram_ctl *pram_ctl);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_wan_serdes_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

