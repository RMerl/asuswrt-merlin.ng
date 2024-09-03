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
#ifndef _RDP_DRV_BUFMNG_H_
#define _RDP_DRV_BUFMNG_H_

#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_cnpl_ag.h"

#if CHIP_VER >= RDP_GEN_62
#include "xrdp_drv_bufmng_ag.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif


#if CHIP_VER >= RDP_GEN_62
#define ag_drv_cnpl_cnpl_stat_order_get                              ag_drv_bufmng_counters_cfg_stat_ordr_get
#define ag_drv_cnpl_cnpl_stat_order_set                              ag_drv_bufmng_counters_cfg_stat_ordr_set
#define ag_drv_cnpl_cnpl_stat_max_thr_get                            ag_drv_bufmng_counters_cfg_stat_max_thr_get
#define ag_drv_cnpl_cnpl_stat_max_thr_set                            ag_drv_bufmng_counters_cfg_stat_max_thr_set
#define ag_drv_cnpl_cnpl_stat_rsrv_thr_get                           ag_drv_bufmng_counters_cfg_stat_rsrv_thr_get
#define ag_drv_cnpl_cnpl_stat_rsrv_thr_set                           ag_drv_bufmng_counters_cfg_stat_rsrv_thr_set
#define ag_drv_cnpl_cnpl_stat_hipri_thr_get                          ag_drv_bufmng_counters_cfg_stat_hipri_thr_get
#define ag_drv_cnpl_cnpl_stat_hipri_thr_set                          ag_drv_bufmng_counters_cfg_stat_hipri_thr_set
#define ag_drv_cnpl_cnpl_stat_ctrs_val_get                           ag_drv_bufmng_counters_cfg_stat_ctrs_val_get
#define ag_drv_cnpl_cnpl_stat_ctrs_val_set                           ag_drv_bufmng_counters_cfg_stat_ctrs_val_set
#define ag_drv_cnpl_buf_mng_counters_cfg_stat_cntr_init_set          ag_drv_bufmng_counters_cfg_stat_cntr_init_set
#define ag_drv_cnpl_buf_mng_counters_cfg_stat_hi_wmrk_cfg_set        ag_drv_bufmng_counters_cfg_stat_hi_wmrk_cfg_set
#define ag_drv_cnpl_buf_mng_counters_cfg_stat_cntr_neg_st_get        ag_drv_bufmng_counters_cfg_stat_cntr_neg_st_get
#define ag_drv_cnpl_cnpl_stat_neg_cap_cnt_get                        ag_drv_bufmng_counters_cfg_stat_cnt_neg_cap_cnt_get
#define ag_drv_cnpl_buf_mng_counters_cfg_stat_cnt_neg_cap_cmd_get    ag_drv_bufmng_counters_cfg_stat_cnt_neg_cap_cmd_get
#endif

#ifdef USE_BDMF_SHELL
int drv_cnpl_cli_bufmng_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void drv_bufmng_cli_init(bdmfmon_handle_t driver_dir);
void drv_bufmng_cli_exit(bdmfmon_handle_t driver_dir);
#endif


#ifdef __cplusplus
}
#endif

#endif
