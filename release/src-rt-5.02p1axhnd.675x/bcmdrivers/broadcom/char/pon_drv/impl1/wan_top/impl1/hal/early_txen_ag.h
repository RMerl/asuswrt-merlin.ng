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

#ifndef _BCM6858_EARLY_TXEN_AG_H_
#define _BCM6858_EARLY_TXEN_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif

/**************************************************************************************************/
/* cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass:  - Early TXEN Enable Logic Bypass. 0 = */
/*                                                          NO_BYPASS                             */
/* cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity:  - Mac TXEN input polarity. 0 = ACTI */
/*                                                           VE_LOW                               */
/* cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity:  - Mac TXEN output polarity. 0 = AC */
/*                                                            TIVE_LOW                            */
/* cr_xgwan_top_wan_misc_early_txen_cfg_toff_time:  - Early TXEN Toff Time                        */
/* cr_xgwan_top_wan_misc_early_txen_cfg_setup_time:  - Early TXEN Setup Time                      */
/* cr_xgwan_top_wan_misc_early_txen_cfg_hold_time:  - Early TXEN Hold Time                        */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean cr_xgwan_top_wan_misc_early_txen_cfg_early_txen_bypass;
    bdmf_boolean cr_xgwan_top_wan_misc_early_txen_cfg_input_txen_polarity;
    bdmf_boolean cr_xgwan_top_wan_misc_early_txen_cfg_output_txen_polarity;
    uint8_t cr_xgwan_top_wan_misc_early_txen_cfg_toff_time;
    uint8_t cr_xgwan_top_wan_misc_early_txen_cfg_setup_time;
    uint8_t cr_xgwan_top_wan_misc_early_txen_cfg_hold_time;
} early_txen_txen;

bdmf_error_t ag_drv_early_txen_txen_set(const early_txen_txen *txen);
bdmf_error_t ag_drv_early_txen_txen_get(early_txen_txen *txen);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_early_txen_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

