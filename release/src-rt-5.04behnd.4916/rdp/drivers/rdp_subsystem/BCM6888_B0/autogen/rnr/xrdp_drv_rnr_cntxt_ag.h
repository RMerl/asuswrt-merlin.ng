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


#ifndef _XRDP_DRV_RNR_CNTXT_AG_H_
#define _XRDP_DRV_RNR_CNTXT_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif


/**********************************************************************************************************************
 * context_entry: 
 *     Context mem entry
 **********************************************************************************************************************/
bdmf_error_t ag_drv_rnr_cntxt_mem_entry_set(uint8_t rnr_id, uint32_t word_index, uint32_t context_entry);
bdmf_error_t ag_drv_rnr_cntxt_mem_entry_get(uint8_t rnr_id, uint32_t word_index, uint32_t *context_entry);

#ifdef USE_BDMF_SHELL
enum
{
    cli_rnr_cntxt_mem_entry,
};

int bcm_rnr_cntxt_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_rnr_cntxt_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
