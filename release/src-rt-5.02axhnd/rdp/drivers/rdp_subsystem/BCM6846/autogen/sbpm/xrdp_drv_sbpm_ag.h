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

#ifndef _XRDP_DRV_SBPM_AG_H_
#define _XRDP_DRV_SBPM_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* search_depth: search_depth - Depth (or maximal threshold) for search during Free without conte */
/*               xt                                                                               */
/* max_search_en: max_search_en - Enable for max search  during Free without context              */
/* chck_last_en: chck_last_en - Enable for Last BN checking  during Free with context             */
/* freeze_in_error: freeze_in_error - Freeze Ug/Global counters + mask access to SBPM RAM while i */
/*                  n ERROR state                                                                 */
/**************************************************************************************************/
typedef struct
{
    uint8_t search_depth;
    bdmf_boolean max_search_en;
    bdmf_boolean chck_last_en;
    bdmf_boolean freeze_in_error;
} sbpm_error_handle_parm;


/**************************************************************************************************/
/* alloc_bn_valid: alloc_bn_valid - alloc_bn_valid                                                */
/* alloc_bn: alloc_bn - alloc_bn                                                                  */
/* ack: ack - ack                                                                                 */
/* nack: nack - nack                                                                              */
/* excl_high: excl_high - Exclusive bit is indication of Exclusive_high status of client with rel */
/*            ated Alloc request                                                                  */
/* excl_low: excl_low - Exclusive bit is indication of Exclusive_low status of client with relate */
/*           d Alloc request                                                                      */
/* busy: busy - busy                                                                              */
/* rdy: rdy - rdy                                                                                 */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean alloc_bn_valid;
    uint16_t alloc_bn;
    bdmf_boolean ack;
    bdmf_boolean nack;
    bdmf_boolean excl_high;
    bdmf_boolean excl_low;
    bdmf_boolean busy;
    bdmf_boolean rdy;
} sbpm_regs_bn_alloc_rply;


/**************************************************************************************************/
/* bn_valid: bn_valid - Used for validation of Next BN reply                                      */
/* next_bn: next_bn - Next BN - reply of Get_next command                                         */
/* bn_null: bn_null - Next BN is null indication                                                  */
/* mcnt_val: mcnt_val - mcst cnt val                                                              */
/* busy: busy - Get Next command is busy                                                          */
/* rdy: rdy - Get Next command is ready                                                           */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean bn_valid;
    uint16_t next_bn;
    bdmf_boolean bn_null;
    uint8_t mcnt_val;
    bdmf_boolean busy;
    bdmf_boolean rdy;
} sbpm_regs_get_next_rply;


/**************************************************************************************************/
/* bypass_clk_gate: BYPASS_CLOCK_GATE - If set to 1b1 will disable the clock gate logic such to a */
/*                  lways enable the clock                                                        */
/* timer_val: TIMER_VALUE - For how long should the clock stay active once all conditions for clo */
/*            ck disable are met.                                                                 */
/* keep_alive_en: KEEP_ALIVE_ENABLE - Enables the keep alive logic which will periodically enable */
/*                 the clock to assure that no deadlock of clock being removed completely will oc */
/*                cur                                                                             */
/* keep_alive_intervl: KEEP_ALIVE_INTERVAL - If the KEEP alive option is enabled the field will d */
/*                     etermine for how many cycles should the clock be active                    */
/* keep_alive_cyc: KEEP_ALIVE_CYCLE - If the KEEP alive option is enabled this field will determi */
/*                 ne for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTE */
/*                 RVAL)So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.              */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intervl;
    uint8_t keep_alive_cyc;
} sbpm_regs_sbpm_clk_gate_cntrl;


/**************************************************************************************************/
/* free_ack: free_ack - Acknowledge on Free command                                               */
/* ack_stat: ack_stat - ACK status of CPU                                                         */
/* nack_stat: nack_stat - NACK status of CPU                                                      */
/* excl_high_stat: excl_high_stat - Exclusive_high status of CPU                                  */
/* excl_low_stat: excl_low_stat - Exclusive_low status of CPU                                     */
/* bsy: bsy - Busy bit of command (command is currently in execution)                             */
/* rdy: rdy - Ready bit of command (ready for new command execution)                              */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean free_ack;
    bdmf_boolean ack_stat;
    bdmf_boolean nack_stat;
    bdmf_boolean excl_high_stat;
    bdmf_boolean excl_low_stat;
    bdmf_boolean bsy;
    bdmf_boolean rdy;
} sbpm_regs_bn_free_without_contxt_rply;


/**************************************************************************************************/
/* free_ack: free_ack - Free command acknowledge                                                  */
/* ack_state: ack_state - ACK status of CPU                                                       */
/* nack_state: nack_state - NACK status of CPU                                                    */
/* excl_high_state: excl_high_state - Exclusive high status of CPU                                */
/* excl_low_state: excl_low_state - Exclusive low status of CPU                                   */
/* busy: busy - Busy bit of command                                                               */
/* rdy: rdy - Ready bit of command                                                                */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean free_ack;
    bdmf_boolean ack_state;
    bdmf_boolean nack_state;
    bdmf_boolean excl_high_state;
    bdmf_boolean excl_low_state;
    bdmf_boolean busy;
    bdmf_boolean rdy;
} sbpm_regs_bn_free_with_contxt_rply;


/**************************************************************************************************/
/* bac_underrun: bac_underrun - This error bit indicates underrun state of SBPM Buffer Allocated  */
/*               Counter (one of User Groups). SW can clear this bit by writing 1 to this field   */
/* mcst_overflow: mcst_overflow - This error bit indicates if the Multi Cast value of a buffer is */
/*                 in overflow as a result of erroneous MCINC command                             */
/* check_last_err: check_last_err - This bit indicates error state on Last BN checking during Fre */
/*                 e with context request. SW can clear this bit by writing 1 to this field.      */
/* max_search_err: max_search_err - This bit indicates error state on maximal search checking dur */
/*                 ing Free without context request. SW can clear this bit by writing 1 to this f */
/*                 ield.                                                                          */
/* invalid_in2e: invalid_in2e - This bit indicates invalid ingress2egress command (caused BAC und */
/*               er/overrun). SW can clear this bit by writing 1 to this field.                   */
/* multi_get_next_null: multi_get_next_null - This bit indicates Null encounter during one of the */
/*                       next BNs. SW can clear this bit by writing 0 to this field.              */
/* cnct_null: cnct_null - This bit indicates connection of the NULL buffer to another buufer. SW  */
/*            can clear this bit by writing 0 to this field.                                      */
/* alloc_null: alloc_null - This bit indicates allocation of the NULL buffer. SW can clear this b */
/*             it by writing 0 to this field.                                                     */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean bac_underrun;
    bdmf_boolean mcst_overflow;
    bdmf_boolean check_last_err;
    bdmf_boolean max_search_err;
    bdmf_boolean invalid_in2e;
    bdmf_boolean multi_get_next_null;
    bdmf_boolean cnct_null;
    bdmf_boolean alloc_null;
} sbpm_intr_ctrl_isr;

bdmf_error_t ag_drv_sbpm_nack_mask_get(uint32_t *sbpm_nack_mask_high, uint32_t *sbpm_nack_mask_low);
bdmf_error_t ag_drv_sbpm_bac_get(uint16_t *bac, uint16_t *ug1bac, uint16_t *ug0bac);
bdmf_error_t ag_drv_sbpm_error_handle_parm_set(const sbpm_error_handle_parm *error_handle_parm);
bdmf_error_t ag_drv_sbpm_error_handle_parm_get(sbpm_error_handle_parm *error_handle_parm);
bdmf_error_t ag_drv_sbpm_regs_init_free_list_set(uint16_t init_base_addr, uint16_t init_offset, bdmf_boolean bsy, bdmf_boolean rdy);
bdmf_error_t ag_drv_sbpm_regs_init_free_list_get(uint16_t *init_base_addr, uint16_t *init_offset, bdmf_boolean *bsy, bdmf_boolean *rdy);
bdmf_error_t ag_drv_sbpm_regs_bn_alloc_set(uint8_t sa);
bdmf_error_t ag_drv_sbpm_regs_bn_alloc_get(uint8_t *sa);
bdmf_error_t ag_drv_sbpm_regs_bn_alloc_rply_get(sbpm_regs_bn_alloc_rply *regs_bn_alloc_rply);
bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_low_set(uint16_t head_bn, uint8_t sa, uint8_t offset, bdmf_boolean ack);
bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_low_get(uint16_t *head_bn, uint8_t *sa, uint8_t *offset, bdmf_boolean *ack);
bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_high_set(uint16_t last_bn);
bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_high_get(uint16_t *last_bn);
bdmf_error_t ag_drv_sbpm_regs_mcst_inc_set(uint16_t bn, uint8_t mcst_val, bdmf_boolean ack_req);
bdmf_error_t ag_drv_sbpm_regs_mcst_inc_get(uint16_t *bn, uint8_t *mcst_val, bdmf_boolean *ack_req);
bdmf_error_t ag_drv_sbpm_regs_mcst_inc_rply_get(bdmf_boolean *mcst_ack, bdmf_boolean *bsy, bdmf_boolean *rdy);
bdmf_error_t ag_drv_sbpm_regs_bn_connect_set(uint16_t bn, bdmf_boolean ack_req, bdmf_boolean wr_req, uint16_t pointed_bn);
bdmf_error_t ag_drv_sbpm_regs_bn_connect_get(uint16_t *bn, bdmf_boolean *ack_req, bdmf_boolean *wr_req, uint16_t *pointed_bn);
bdmf_error_t ag_drv_sbpm_regs_bn_connect_rply_get(bdmf_boolean *connect_ack, bdmf_boolean *busy, bdmf_boolean *rdy);
bdmf_error_t ag_drv_sbpm_regs_get_next_set(uint16_t bn);
bdmf_error_t ag_drv_sbpm_regs_get_next_get(uint16_t *bn);
bdmf_error_t ag_drv_sbpm_regs_get_next_rply_get(sbpm_regs_get_next_rply *regs_get_next_rply);
bdmf_error_t ag_drv_sbpm_regs_sbpm_clk_gate_cntrl_set(const sbpm_regs_sbpm_clk_gate_cntrl *regs_sbpm_clk_gate_cntrl);
bdmf_error_t ag_drv_sbpm_regs_sbpm_clk_gate_cntrl_get(sbpm_regs_sbpm_clk_gate_cntrl *regs_sbpm_clk_gate_cntrl);
bdmf_error_t ag_drv_sbpm_regs_bn_free_without_contxt_set(uint16_t head_bn, uint8_t sa, bdmf_boolean ack_req);
bdmf_error_t ag_drv_sbpm_regs_bn_free_without_contxt_get(uint16_t *head_bn, uint8_t *sa, bdmf_boolean *ack_req);
bdmf_error_t ag_drv_sbpm_regs_bn_free_without_contxt_rply_get(sbpm_regs_bn_free_without_contxt_rply *regs_bn_free_without_contxt_rply);
bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_rply_get(sbpm_regs_bn_free_with_contxt_rply *regs_bn_free_with_contxt_rply);
bdmf_error_t ag_drv_sbpm_regs_sbpm_gl_trsh_set(uint16_t gl_bat, uint16_t gl_bah);
bdmf_error_t ag_drv_sbpm_regs_sbpm_gl_trsh_get(uint16_t *gl_bat, uint16_t *gl_bah);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_trsh_set(uint16_t ug_bat, uint16_t ug_bah);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_trsh_get(uint16_t *ug_bat, uint16_t *ug_bah);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_trsh_set(uint16_t ug_bat, uint16_t ug_bah);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_trsh_get(uint16_t *ug_bat, uint16_t *ug_bah);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_excl_high_trsh_set(uint16_t exclt, uint16_t exclh);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_excl_high_trsh_get(uint16_t *exclt, uint16_t *exclh);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_excl_high_trsh_set(uint16_t exclt, uint16_t exclh);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_excl_high_trsh_get(uint16_t *exclt, uint16_t *exclh);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_excl_low_trsh_set(uint16_t exclt, uint16_t exclh);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_excl_low_trsh_get(uint16_t *exclt, uint16_t *exclh);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_excl_low_trsh_set(uint16_t exclt, uint16_t exclh);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_excl_low_trsh_get(uint16_t *exclt, uint16_t *exclh);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_status_get(uint8_t *ug_ack_stts, uint8_t *ug_excl_high_stts, uint8_t *ug_excl_low_stts);
bdmf_error_t ag_drv_sbpm_regs_sbpm_iir_low_get(uint8_t *cmd_sa, uint8_t *cmd_ta, uint32_t *cmd_data_22to0);
bdmf_error_t ag_drv_sbpm_regs_sbpm_iir_high_get(uint32_t *cmd_data_23to63);
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_bbh_low_set(uint32_t sbpm_sp_bbh_low);
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_bbh_low_get(uint32_t *sbpm_sp_bbh_low);
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_bbh_high_set(uint32_t sbpm_sp_bbh_high);
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_bbh_high_get(uint32_t *sbpm_sp_bbh_high);
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_rnr_low_set(uint32_t sbpm_sp_rnr_low);
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_rnr_low_get(uint32_t *sbpm_sp_rnr_low);
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_rnr_high_set(uint32_t sbpm_sp_rnr_high);
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_rnr_high_get(uint32_t *sbpm_sp_rnr_high);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_map_low_set(uint32_t sbpm_ug_map_low);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_map_low_get(uint32_t *sbpm_ug_map_low);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_map_high_set(uint32_t sbpm_ug_map_high);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_map_high_get(uint32_t *sbpm_ug_map_high);
bdmf_error_t ag_drv_sbpm_regs_sbpm_excl_mask_low_set(uint32_t sbpm_excl_mask_low);
bdmf_error_t ag_drv_sbpm_regs_sbpm_excl_mask_low_get(uint32_t *sbpm_excl_mask_low);
bdmf_error_t ag_drv_sbpm_regs_sbpm_excl_mask_high_set(uint32_t sbpm_excl_mask_high);
bdmf_error_t ag_drv_sbpm_regs_sbpm_excl_mask_high_get(uint32_t *sbpm_excl_mask_high);
bdmf_error_t ag_drv_sbpm_regs_sbpm_raddr_decoder_set(uint8_t id_2overwr, uint16_t overwr_ra, bdmf_boolean overwr_valid);
bdmf_error_t ag_drv_sbpm_regs_sbpm_raddr_decoder_get(uint8_t *id_2overwr, uint16_t *overwr_ra, bdmf_boolean *overwr_valid);
bdmf_error_t ag_drv_sbpm_regs_sbpm_wr_data_set(uint32_t sbpm_wr_data);
bdmf_error_t ag_drv_sbpm_regs_sbpm_wr_data_get(uint32_t *sbpm_wr_data);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_bac_max_set(uint16_t ug0bacmax, uint16_t ug1bacmax);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_bac_max_get(uint16_t *ug0bacmax, uint16_t *ug1bacmax);
bdmf_error_t ag_drv_sbpm_intr_ctrl_isr_set(const sbpm_intr_ctrl_isr *intr_ctrl_isr);
bdmf_error_t ag_drv_sbpm_intr_ctrl_isr_get(sbpm_intr_ctrl_isr *intr_ctrl_isr);
bdmf_error_t ag_drv_sbpm_intr_ctrl_ism_get(uint32_t *ism);
bdmf_error_t ag_drv_sbpm_intr_ctrl_ier_set(uint32_t iem);
bdmf_error_t ag_drv_sbpm_intr_ctrl_ier_get(uint32_t *iem);
bdmf_error_t ag_drv_sbpm_intr_ctrl_itr_set(uint32_t ist);
bdmf_error_t ag_drv_sbpm_intr_ctrl_itr_get(uint32_t *ist);

#ifdef USE_BDMF_SHELL
enum
{
    cli_sbpm_nack_mask,
    cli_sbpm_bac,
    cli_sbpm_error_handle_parm,
    cli_sbpm_regs_init_free_list,
    cli_sbpm_regs_bn_alloc,
    cli_sbpm_regs_bn_alloc_rply,
    cli_sbpm_regs_bn_free_with_contxt_low,
    cli_sbpm_regs_bn_free_with_contxt_high,
    cli_sbpm_regs_mcst_inc,
    cli_sbpm_regs_mcst_inc_rply,
    cli_sbpm_regs_bn_connect,
    cli_sbpm_regs_bn_connect_rply,
    cli_sbpm_regs_get_next,
    cli_sbpm_regs_get_next_rply,
    cli_sbpm_regs_sbpm_clk_gate_cntrl,
    cli_sbpm_regs_bn_free_without_contxt,
    cli_sbpm_regs_bn_free_without_contxt_rply,
    cli_sbpm_regs_bn_free_with_contxt_rply,
    cli_sbpm_regs_sbpm_gl_trsh,
    cli_sbpm_regs_sbpm_ug0_trsh,
    cli_sbpm_regs_sbpm_ug1_trsh,
    cli_sbpm_regs_sbpm_ug0_excl_high_trsh,
    cli_sbpm_regs_sbpm_ug1_excl_high_trsh,
    cli_sbpm_regs_sbpm_ug0_excl_low_trsh,
    cli_sbpm_regs_sbpm_ug1_excl_low_trsh,
    cli_sbpm_regs_sbpm_ug_status,
    cli_sbpm_regs_sbpm_iir_low,
    cli_sbpm_regs_sbpm_iir_high,
    cli_sbpm_regs_sbpm_sp_bbh_low,
    cli_sbpm_regs_sbpm_sp_bbh_high,
    cli_sbpm_regs_sbpm_sp_rnr_low,
    cli_sbpm_regs_sbpm_sp_rnr_high,
    cli_sbpm_regs_sbpm_ug_map_low,
    cli_sbpm_regs_sbpm_ug_map_high,
    cli_sbpm_regs_sbpm_excl_mask_low,
    cli_sbpm_regs_sbpm_excl_mask_high,
    cli_sbpm_regs_sbpm_raddr_decoder,
    cli_sbpm_regs_sbpm_wr_data,
    cli_sbpm_regs_sbpm_ug_bac_max,
    cli_sbpm_intr_ctrl_isr,
    cli_sbpm_intr_ctrl_ism,
    cli_sbpm_intr_ctrl_ier,
    cli_sbpm_intr_ctrl_itr,
};

int bcm_sbpm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_sbpm_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

