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


#ifndef _XRDP_DRV_BUFMNG_AG_H_
#define _XRDP_DRV_BUFMNG_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif


/**********************************************************************************************************************
 * nxtlvl: 
 *     the address is counter
 *     bits[ 5:0] are next-level after this counter.
 *     If msb=1, then nxt_level is not valid
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_ordr_set(uint16_t ctr_idx, uint8_t nxtlvl);
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_ordr_get(uint16_t ctr_idx, uint8_t *nxtlvl);

/**********************************************************************************************************************
 * thr: 
 *     threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_rsrv_thr_set(uint16_t ctr_idx, uint32_t thr);
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_rsrv_thr_get(uint16_t ctr_idx, uint32_t *thr);

/**********************************************************************************************************************
 * thr: 
 *     threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_hipri_thr_set(uint16_t ctr_idx, uint32_t thr);
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_hipri_thr_get(uint16_t ctr_idx, uint32_t *thr);

/**********************************************************************************************************************
 * thr: 
 *     threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_max_thr_set(uint16_t ctr_idx, uint32_t thr);
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_max_thr_get(uint16_t ctr_idx, uint32_t *thr);

/**********************************************************************************************************************
 * ctr0: 
 *     counter to get its high-watermark
 *     
 * ctr1: 
 *     counter to get its high-watermark
 *     
 * ctr2: 
 *     counter to get its high-watermark
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_hi_wmrk_cfg_set(uint8_t ctr0, uint8_t ctr1, uint8_t ctr2);
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_hi_wmrk_cfg_get(uint8_t *ctr0, uint8_t *ctr1, uint8_t *ctr2);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_hi_wmrk_val_get(uint16_t hi_wmrk_idx, uint32_t *val);

/**********************************************************************************************************************
 * idx: 
 *     counter index
 * val: 
 *     counter value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_cntr_init_set(uint8_t idx, uint32_t val);

/**********************************************************************************************************************
 * val: 
 *     If bit=1, the corresponding counter got a wrong decrement command that was supposed to turn it negative.
 *     Sticky, until cleared with register cntr_neg_stat_clr.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_cntr_neg_st_get(uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     if bit=1, it clears corresponding bit in cntr_neg_st register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_cntr_neg_st_clr_set(uint32_t val);

/**********************************************************************************************************************
 * mod: 
 *     mode
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_capt_cfg_set(bdmf_boolean mod);
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_capt_cfg_get(bdmf_boolean *mod);

/**********************************************************************************************************************
 * idx: 
 *     counter index
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_cnt_neg_cap_cnt_get(uint16_t neg_cap_cnt, uint8_t *idx, uint32_t *val);

/**********************************************************************************************************************
 * idx: 
 *     command index inside cnpl (pointer to bacif array)
 * cmd: 
 *     copy of command as seen on bb_data[31:9].
 *     This can be compare to bacif debug info.
 *     bit 63:60 are always 4h4 for this command, other params[59:41] should reflect bb_data[27:9] (bit 8 is reserved).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_cnt_neg_cap_cmd_get(uint8_t *idx, uint32_t *cmd);

/**********************************************************************************************************************
 * pool0_size: 
 *     size of pool0 in tokens.
 *     
 * pool1_size: 
 *     size of pool1 in tokens.
 *     
 * pool2_size: 
 *     size of pool2 in tokens.
 *     
 * pool3_size: 
 *     size of pool3 in tokens.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_pools_size_set(uint8_t pool0_size, uint8_t pool1_size, uint8_t pool2_size, uint8_t pool3_size);
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_pools_size_get(uint8_t *pool0_size, uint8_t *pool1_size, uint8_t *pool2_size, uint8_t *pool3_size);

/**********************************************************************************************************************
 * neg_en: 
 *     negative value enable:
 *     1: decrement can reach negative values of counter(bit 19 in counter[19:0])
 *     0: decrement not allowed to reach negative values of counter
 * fc_hyst: 
 *     fc hysteresis (resolution of 16).
 *     used as configuration for following feature:
 *     fc vector of 32 bits (1 per counter) with the following definition:
 *     1.	If  0:
 *     For each INC or DEC, the HW checks if the counter >= high_prio_thr (unique for each counter).
 *     If yes, asserts the relevant bit in the vector
 *     2.	If 1:
 *     For each INC or DEC, the HW checks if the counter <= high_prio_thr - 16*fc_hyst.
 *     If yes, de-asserts the relevant bit
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_misc_set(bdmf_boolean neg_en, uint8_t fc_hyst);
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_misc_get(bdmf_boolean *neg_en, uint8_t *fc_hyst);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_fc_st_vec_get(uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bufmng_counters_cfg_stat_ctrs_val_get(uint16_t ctr_idx, uint32_t *val);

#ifdef USE_BDMF_SHELL
enum
{
    cli_bufmng_counters_cfg_stat_ordr,
    cli_bufmng_counters_cfg_stat_rsrv_thr,
    cli_bufmng_counters_cfg_stat_hipri_thr,
    cli_bufmng_counters_cfg_stat_max_thr,
    cli_bufmng_counters_cfg_stat_hi_wmrk_cfg,
    cli_bufmng_counters_cfg_stat_hi_wmrk_val,
    cli_bufmng_counters_cfg_stat_cntr_init,
    cli_bufmng_counters_cfg_stat_cntr_neg_st,
    cli_bufmng_counters_cfg_stat_cntr_neg_st_clr,
    cli_bufmng_counters_cfg_stat_capt_cfg,
    cli_bufmng_counters_cfg_stat_cnt_neg_cap_cnt,
    cli_bufmng_counters_cfg_stat_cnt_neg_cap_cmd,
    cli_bufmng_counters_cfg_stat_pools_size,
    cli_bufmng_counters_cfg_stat_misc,
    cli_bufmng_counters_cfg_stat_fc_st_vec,
    cli_bufmng_counters_cfg_stat_ctrs_val,
};

int bcm_bufmng_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_bufmng_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
