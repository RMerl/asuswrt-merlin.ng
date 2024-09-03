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
#ifndef _RDP_DRV_SBPM_H_
#define _RDP_DRV_SBPM_H_

#include "xrdp_drv_sbpm_ag.h"
#if defined(BCM_PON_XRDP)
#include "xrdp_drv_psram_mem_ag.h"
#else
#include "xrdp_drv_psram_ag.h"
#endif

// hw init values
#define SBPM_SP_RNR_LOW_INIT_VAL                       0xffff
#define SBPM_SP_RNR_HIGH_INIT_VAL                      0
#define SBPM_UG_MAP_LOW_INIT_VAL                       0x7ffb0000
#define SBPM_UG_MAP_HIGH_INIT_VAL                      0xffffd555
#define SBPM_BUF_SIZE 128
#define SBPM_MAX_NUM_OF_MCAST_REPLICATIONS 7

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
bdmf_error_t drv_sbpm_runner_sp_set(uint16_t sp_id, uint8_t ug_id);
extern bdmf_error_t drv_sbpm_copy_list(uint16_t bn, uint8_t *dest_buffer);
extern bdmf_error_t drv_sbpm_free_list(uint16_t head_bn);
void drv_sbpm_default_val_init(void);

bdmf_error_t drv_sbpm_alloc_list(uint32_t size, uint32_t headroom, uint8_t *data,
    uint16_t *bn0, uint16_t *bn1, uint8_t *bns_num);

bdmf_error_t drv_sbpm_free_list(uint16_t head_bn);
bdmf_error_t drv_sbpm_alloc_and_connect_next_bn(uint32_t size, uint32_t headroom, uint8_t *data, uint16_t next_bn,
    bdmf_boolean inc_next_mc_counter, uint16_t *bn);
uint16_t drv_sbpm_get_next_bn(int16_t bn);
bdmf_error_t drv_sbpm_dump_list(uint16_t bn);

#ifdef USE_BDMF_SHELL
int drv_sbpm_cli_debug_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_sbpm_cli_sanity_check(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
int drv_sbpm_cli_config_get(bdmf_session_handle session, bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
void drv_sbpm_cli_init(bdmfmon_handle_t driver_dir);
void drv_sbpm_cli_exit(bdmfmon_handle_t driver_dir);
#endif

#endif
