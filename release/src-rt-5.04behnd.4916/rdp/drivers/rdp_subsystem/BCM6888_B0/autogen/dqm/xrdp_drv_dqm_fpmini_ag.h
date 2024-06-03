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


#ifndef _XRDP_DRV_DQM_FPMINI_AG_H_
#define _XRDP_DRV_DQM_FPMINI_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    bdmf_boolean rd_clr;
    bdmf_boolean wrap;
    bdmf_boolean msk_free_err;
    bdmf_boolean msk_mc_inc_err;
    bdmf_boolean msk_mc_dec_err;
} dqm_fpmini_fpmini_block_fpmini_prfm_cntrs_gen_cfg;


/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_lvl_0_reg_l0_set(uint32_t data);
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_lvl_0_reg_l0_get(uint32_t *data);

/**********************************************************************************************************************
 * init: 
 *     init
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_cfg0_l2_init_set(bdmf_boolean init);
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_cfg0_l2_init_get(bdmf_boolean *init);

/**********************************************************************************************************************
 * en: 
 *     0: dis
 *     1: en
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_cfg0_allc_fast_ack_set(bdmf_boolean en);
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_cfg0_allc_fast_ack_get(bdmf_boolean *en);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_prfm_cntrs_num_avail_tkns_get(uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_prfm_cntrs_num_avail_tkns_low_wm_set(uint32_t val);
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_prfm_cntrs_num_avail_tkns_low_wm_get(uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_prfm_cntrs_free_err_get(uint32_t *val);

/**********************************************************************************************************************
 * rd_clr: 
 *     read clear bit
 * wrap: 
 *     read clear bit
 * msk_free_err: 
 *     mask_fpmini_free error
 * msk_mc_inc_err: 
 *     mask_fpmcast_mc_inc error
 * msk_mc_dec_err: 
 *     mask_fpmcast_mc_dec error
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_prfm_cntrs_gen_cfg_set(const dqm_fpmini_fpmini_block_fpmini_prfm_cntrs_gen_cfg *fpmini_block_fpmini_prfm_cntrs_gen_cfg);
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_prfm_cntrs_gen_cfg_get(dqm_fpmini_fpmini_block_fpmini_prfm_cntrs_gen_cfg *fpmini_block_fpmini_prfm_cntrs_gen_cfg);

/**********************************************************************************************************************
 * vs: 
 *     selects th debug vector
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_debug_dbgsel_set(uint8_t vs);
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_debug_dbgsel_get(uint8_t *vs);

/**********************************************************************************************************************
 * vb: 
 *     debug vector
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmini_debug_dbgbus_get(uint32_t *vb);

/**********************************************************************************************************************
 * init: 
 *     init
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmcast_cfg0_mc_init_set(bdmf_boolean init);
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmcast_cfg0_mc_init_get(bdmf_boolean *init);

/**********************************************************************************************************************
 * en: 
 *     0: dis
 *     1: en
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmcast_cfg0_free_bp_mc_set(bdmf_boolean en);
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmcast_cfg0_free_bp_mc_get(bdmf_boolean *en);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmcast_prfm_cntrs_mc_inc_err_get(uint32_t *val);

/**********************************************************************************************************************
 * val: 
 *     value
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmcast_prfm_cntrs_mc_dec_err_get(uint32_t *val);

/**********************************************************************************************************************
 * vs: 
 *     selects th debug vector
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmcast_debug_dbgsel_set(uint8_t vs);
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmcast_debug_dbgsel_get(uint8_t *vs);

/**********************************************************************************************************************
 * vb: 
 *     debug vector
 **********************************************************************************************************************/
bdmf_error_t ag_drv_dqm_fpmini_fpmini_block_fpmcast_debug_dbgbus_get(uint32_t *vb);

#ifdef USE_BDMF_SHELL
enum
{
    cli_dqm_fpmini_fpmini_block_fpmini_lvl_0_reg_l0,
    cli_dqm_fpmini_fpmini_block_fpmini_cfg0_l2_init,
    cli_dqm_fpmini_fpmini_block_fpmini_cfg0_allc_fast_ack,
    cli_dqm_fpmini_fpmini_block_fpmini_prfm_cntrs_num_avail_tkns,
    cli_dqm_fpmini_fpmini_block_fpmini_prfm_cntrs_num_avail_tkns_low_wm,
    cli_dqm_fpmini_fpmini_block_fpmini_prfm_cntrs_free_err,
    cli_dqm_fpmini_fpmini_block_fpmini_prfm_cntrs_gen_cfg,
    cli_dqm_fpmini_fpmini_block_fpmini_debug_dbgsel,
    cli_dqm_fpmini_fpmini_block_fpmini_debug_dbgbus,
    cli_dqm_fpmini_fpmini_block_fpmcast_cfg0_mc_init,
    cli_dqm_fpmini_fpmini_block_fpmcast_cfg0_free_bp_mc,
    cli_dqm_fpmini_fpmini_block_fpmcast_prfm_cntrs_mc_inc_err,
    cli_dqm_fpmini_fpmini_block_fpmcast_prfm_cntrs_mc_dec_err,
    cli_dqm_fpmini_fpmini_block_fpmcast_debug_dbgsel,
    cli_dqm_fpmini_fpmini_block_fpmcast_debug_dbgbus,
};

int bcm_dqm_fpmini_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_dqm_fpmini_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
