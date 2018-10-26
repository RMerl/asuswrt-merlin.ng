/*
   Copyright (c) 2015 Broadcom
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

#ifndef _XRDP_DRV_NATC_CFG_AG_H_
#define _XRDP_DRV_NATC_CFG_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"

bdmf_error_t ag_drv_natc_cfg_key_addr_set(uint8_t tbl_idx, uint32_t key_lo, uint8_t key_hi);
bdmf_error_t ag_drv_natc_cfg_key_addr_get(uint8_t tbl_idx, uint32_t *key_lo, uint8_t *key_hi);
bdmf_error_t ag_drv_natc_cfg_res_addr_set(uint8_t tbl_idx, uint32_t res_lo, uint8_t res_hi);
bdmf_error_t ag_drv_natc_cfg_res_addr_get(uint8_t tbl_idx, uint32_t *res_lo, uint8_t *res_hi);

#ifdef USE_BDMF_SHELL
enum
{
    cli_natc_cfg_key_addr,
    cli_natc_cfg_res_addr,
};

int bcm_natc_cfg_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_natc_cfg_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

