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
#ifndef _RDP_DRV_SBPM_H_
#define _RDP_DRV_SBPM_H_

#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_sbpm_ag.h"

// hw init values
#define SBPM_SP_RNR_LOW_INIT_VAL                       0xffff
#define SBPM_SP_RNR_HIGH_INIT_VAL                      0
#define SBPM_UG_MAP_LOW_INIT_VAL                       0x7ffb0000
#define SBPM_UG_MAP_HIGH_INIT_VAL                      0xffffd555

typedef struct
{
    uint16_t bn_hyst;
    uint16_t bn_thr;
    uint16_t excl_high_hyst;
    uint16_t excl_high_thr;
    uint16_t excl_low_hyst;
    uint16_t excl_low_thr;
} sbpm_thr_ug;

bdmf_error_t drv_sbpm_thr_ug0_set(const sbpm_thr_ug *thr_ug);
bdmf_error_t drv_sbpm_thr_ug1_set(const sbpm_thr_ug *thr_ug);
bdmf_error_t drv_sbpm_thr_ug0_get(sbpm_thr_ug *thr_ug);
bdmf_error_t drv_sbpm_thr_ug1_get(sbpm_thr_ug *thr_ug);
void drv_sbpm_default_val_init(void);

#ifdef USE_BDMF_SHELL
int drv_sbpm_cli_debug_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_sbpm_cli_sanity_check(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_sbpm_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void drv_sbpm_cli_init(bdmfmon_handle_t driver_dir);
void drv_sbpm_cli_exit(bdmfmon_handle_t driver_dir);
#endif

#endif
