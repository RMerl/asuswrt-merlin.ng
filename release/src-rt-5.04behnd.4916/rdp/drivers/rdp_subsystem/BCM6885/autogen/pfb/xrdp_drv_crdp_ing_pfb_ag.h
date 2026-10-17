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


#ifndef _XRDP_DRV_CRDP_ING_PFB_AG_H_
#define _XRDP_DRV_CRDP_ING_PFB_AG_H_

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    bdmf_boolean fifo_en;
    uint8_t to_val;
    uint8_t my_ch_addr;
    uint8_t egr_ch_addr;
    uint8_t egr_bb_id;
    uint8_t wrr_val;
    bdmf_boolean init_addr_en;
    uint8_t if_width;
    bdmf_boolean eop_en;
    bdmf_boolean iso_input;
} crdp_ing_pfb_cfg_ing;

typedef struct
{
    bdmf_boolean fifo_en;
    uint8_t cr_rep_gran;
    uint8_t my_ch_addr;
    uint8_t ing_ch_addr;
    uint8_t init_cr;
    uint8_t ing_bb_id;
    bdmf_boolean init_addr_en;
    uint8_t if_width;
    bdmf_boolean flush_en;
    bdmf_boolean iso_input;
} crdp_ing_pfb_cfg_egr;

typedef struct
{
    bdmf_boolean arb_scheme;
    bdmf_boolean route_addr_overwr_en;
    uint8_t src_bb_id;
    bdmf_boolean src_bb_id_ovr_en;
    uint8_t flush_pace_timeout;
    uint16_t route_addr_overwr_val;
} crdp_ing_pfb_cfg_gbl;


/**********************************************************************************************************************
 * fifo_en: 
 *     channel enabled is used as qualifier for the configuration. should be set to 0 before writing any
 *     configurations.
 *     set to 1 enables the channel.
 *     ORed with INIT_ADDR_EN. see INIT_ADDR_EN for further description
 *     
 * to_val: 
 *     timeout value (power of 2)
 *     Note: value of 0x0 is used to disable timer feature
 *     
 * my_ch_addr: 
 *     INGRESS FIFO ID
 * egr_ch_addr: 
 *     destination egress BB target address
 * egr_bb_id: 
 *     destination PFB BB ID
 * wrr_val: 
 *     if arb_scheme == 0x0 then the arbitration is done according the value here.
 * init_addr_en: 
 *     Enable initialization according to incoming credit messages from egress side.
 *     if set to 0, then channel parameters are taken from MY_TARGET_ADDR, DEST_TARGET_ADDR, TRGT_BB_ID and WIDTH_IF
 *     (new in Lotus PFB)
 *     if set to 1, then the values of the above are dismissed. Configured by egress
 *     FIFO_EN is ORed with INIT_ADDR_EN. if set to 1 then FIFO_EN is set too.
 *     if any configuration of ingress FIFO should be modified then set INIT_ADDR_EN and FIFO_EN to 0 before config.
 *     When config is done set on consecutive write FIFO_EN =1 and INIT_ADDR_EN = 0/1
 *     
 *     if ING_INIT_ADDR_EN is set 1,  two following fields are hardwiried:
 *     1) cfg_egr_my_target_addr is set to channel index
 *     2) cfg_ing_my_target_addr is set to channel index
 *     
 *     and three other following fields are updated via CREDIT message from egress to ingress:
 *     3) cfg_ing_trgt_bb_id = target BB_ID programmmed by EGR
 *     4) cr_pfb_ing_cr_if_width = client width  programmed by EGR
 *     5)  cfg_ing_dest_target_addr = egress FIFO ID programmed by EGR
 * if_width: 
 *     Enable different interface widths on ingress client side. Options:
 *     0x0: 32 bit
 *     0x1: 128 bit
 *     0x2: 256 bit
 *     
 * eop_en: 
 *     Enable EOP detection in ingress channel
 *     
 * iso_input: 
 *     Isolate (=ignore) client input (in case of ingress channel: ignore push input signal)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ing_pfb_cfg_ing_set(uint8_t pfb_id, uint8_t idx, const crdp_ing_pfb_cfg_ing *cfg_ing);
bdmf_error_t ag_drv_crdp_ing_pfb_cfg_ing_get(uint8_t pfb_id, uint8_t idx, crdp_ing_pfb_cfg_ing *cfg_ing);

/**********************************************************************************************************************
 * fifo_en: 
 *     fifo enable
 * cr_rep_gran: 
 *     credits reply granularity 8B granulariy (power of 2)
 * my_ch_addr: 
 *     egress BB target address
 * ing_ch_addr: 
 *     source ingress BB target address
 * init_cr: 
 *     init credits override 8B granulariy (power of 2). if 0 (default), then the initial credits are derived from
 *     FIFO depth:
 *     init_credits_to_ingress = FIFO_size_in_bytes/8B
 * ing_bb_id: 
 *     source PFB BB ID
 * init_addr_en: 
 *     if set to 1, then my_target_addr is hardwired to egress fifo sequential value: [0..7]
 *     
 *     if set to 0, then my_target_addr is set according to my_target_addr reg value
 * if_width: 
 *     Enable different interface widths on egress client side. Options:
 *     0x0: 32 bit
 *     0x1: 128 bit
 *     0x2: 256 bit
 * flush_en: 
 *     Flush egress FIFO (the assertion of this bit will cause Egress PFB channel to empty its FIFO, while the emptied
 *     data will be not pushed to egress client side)
 * iso_input: 
 *     Isolate (=ignore) client input (in case of egress channel: ignore pop input signal)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ing_pfb_cfg_egr_set(uint8_t pfb_id, uint8_t idx, const crdp_ing_pfb_cfg_egr *cfg_egr);
bdmf_error_t ag_drv_crdp_ing_pfb_cfg_egr_get(uint8_t pfb_id, uint8_t idx, crdp_ing_pfb_cfg_egr *cfg_egr);

/**********************************************************************************************************************
 * arb_scheme: 
 *     Configure the arbitration scheme:
 *     0. (Weighted) Round robin (default)
 *     1. Strict priority. FIFO with the lower interface number gets the highest priority
 *     
 * route_addr_overwr_en: 
 *     Enable of override of PFB BB256 route address for any outcoming transaction (data/credit/pwire) from PFB unit
 *     on broadbus. In case , the route addr will applied constantly from ROUTE_ADDR_OVERWR_VAL
 * src_bb_id: 
 *     Configure source BB_ID of pfb_top, which can be used alternately (upon overiide_en) to wired input of BB_ID
 *     
 * src_bb_id_ovr_en: 
 *     Enable of override of source BB_ID of pfb_top, which can be used alternately to wired input of BB_ID
 *     
 * flush_pace_timeout: 
 *     timeout value in power of 2 (in clock cycles) that used for pace pop during egress flush
 *     
 *     Note: value of 0x0 is used to disable timer feature
 *     
 * route_addr_overwr_val: 
 *     If Enabled of override of PFB BB256 route address for any outcoming transaction (data/credit/pwire) from PFB
 *     unit on broadbus. In case , the route addr will applied constantly from this ROUTE_ADDR_OVERWR_VAL[15:0]. Note
 *     that formal PFB BB addr is 17-bit, but practically only 15bit are in use , so 16bit config are enough
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ing_pfb_cfg_gbl_set(uint8_t pfb_id, const crdp_ing_pfb_cfg_gbl *cfg_gbl);
bdmf_error_t ag_drv_crdp_ing_pfb_cfg_gbl_get(uint8_t pfb_id, crdp_ing_pfb_cfg_gbl *cfg_gbl);

/**********************************************************************************************************************
 * mask: 
 *     Config = Number of bits of mask that will be applied on Pseudo-Wire 16-bits input bus.
 *     Note that Mask config bit applies on the nibble of value be masked , i.e.:
 *     if desirable pseudo-wire width is 12 bits (need mask only 4 MSBits), mask config field should be =0x7
 * to_val: 
 *     timeout value (power of 2)
 * ing_spare_cfg: 
 *     spare config
 * pwire_val: 
 *     The last captured pseudo-wire on ingress channel after pseudo-wire timer expired its counting (since the last
 *     change).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ing_pfb_cfg_ing_pwire_set(uint8_t pfb_id, uint8_t idx, uint8_t mask, uint8_t to_val, uint8_t ing_spare_cfg, uint16_t pwire_val);
bdmf_error_t ag_drv_crdp_ing_pfb_cfg_ing_pwire_get(uint8_t pfb_id, uint8_t idx, uint8_t *mask, uint8_t *to_val, uint8_t *ing_spare_cfg, uint16_t *pwire_val);

/**********************************************************************************************************************
 * mask: 
 *     Config = Number of bits of mask that will be applied on Pseudo-Wire 16-bits input bus.
 *     Note that Mask config bit applies on the nibble of value be masked , i.e.:
 *     if desirable pseudo-wire width is 12 bits (need mask only 4 MSBits), mask config field should be =0x7
 * to_val: 
 *     timeout value (power of 2)
 * egr_spare_cfg: 
 *     EGR_SPARE_CFG
 * pwire_val: 
 *     The last captured pseudo-wire on egress channel after pseudo-wire timer expired its counting (since the last
 *     change).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ing_pfb_cfg_egr_pwire_set(uint8_t pfb_id, uint8_t idx, uint8_t mask, uint8_t to_val, uint8_t egr_spare_cfg, uint16_t pwire_val);
bdmf_error_t ag_drv_crdp_ing_pfb_cfg_egr_pwire_get(uint8_t pfb_id, uint8_t idx, uint8_t *mask, uint8_t *to_val, uint8_t *egr_spare_cfg, uint16_t *pwire_val);

/**********************************************************************************************************************
 * mask: 
 *     EOP_mask 16-bit (all ones and all zeros) , example: 11h0, 5b11111
 *     
 * eop_msb_mask: 
 *     EOP_MSB mask (16-bit) , example:  11h0, 1b1, 4b0
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ing_pfb_cfg_ing_eop_mask_set(uint8_t pfb_id, uint8_t idx, uint16_t mask, uint16_t eop_msb_mask);
bdmf_error_t ag_drv_crdp_ing_pfb_cfg_ing_eop_mask_get(uint8_t pfb_id, uint8_t idx, uint16_t *mask, uint16_t *eop_msb_mask);

/**********************************************************************************************************************
 * ing_full: 
 *     ingress FIFO full status
 * ing_empty: 
 *     ingress FIFO empty status
 * egr_full: 
 *     egress FIFO full status
 * egr_empty: 
 *     egress FIFO empty status
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ing_pfb_dbg_fifo_status_get(uint8_t pfb_id, uint8_t *ing_full, uint8_t *ing_empty, uint8_t *egr_full, uint8_t *egr_empty);

/**********************************************************************************************************************
 * used_words: 
 *     USED_WORDS
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ing_pfb_dbg_ingress_used_words_get(uint8_t pfb_id, uint8_t idx, uint32_t *used_words);

/**********************************************************************************************************************
 * used_words: 
 *     USED_WORDS
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ing_pfb_dbg_egress_used_words_get(uint8_t pfb_id, uint8_t idx, uint32_t *used_words);

/**********************************************************************************************************************
 * bb_msg_trans: 
 *     BB_MSG_TRANS
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ing_pfb_dbg_ingress_bb_msg_trans_get(uint8_t pfb_id, uint8_t idx, uint32_t *bb_msg_trans);

/**********************************************************************************************************************
 * bb_msg_trans: 
 *     BB_MSG_TRANS
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ing_pfb_dbg_egress_bb_msg_trans_get(uint8_t pfb_id, uint8_t idx, uint32_t *bb_msg_trans);

/**********************************************************************************************************************
 * ingress_credit_cnt: 
 *     INGRESS_CREDIT_CNT
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ing_pfb_dbg_ingress_credit_cnt_get(uint8_t pfb_id, uint8_t idx, uint32_t *ingress_credit_cnt);

#ifdef USE_BDMF_SHELL
enum
{
    cli_crdp_ing_pfb_cfg_ing,
    cli_crdp_ing_pfb_cfg_egr,
    cli_crdp_ing_pfb_cfg_gbl,
    cli_crdp_ing_pfb_cfg_ing_pwire,
    cli_crdp_ing_pfb_cfg_egr_pwire,
    cli_crdp_ing_pfb_cfg_ing_eop_mask,
    cli_crdp_ing_pfb_dbg_fifo_status,
    cli_crdp_ing_pfb_dbg_ingress_used_words,
    cli_crdp_ing_pfb_dbg_egress_used_words,
    cli_crdp_ing_pfb_dbg_ingress_bb_msg_trans,
    cli_crdp_ing_pfb_dbg_egress_bb_msg_trans,
    cli_crdp_ing_pfb_dbg_ingress_credit_cnt,
};

int bcm_crdp_ing_pfb_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_crdp_ing_pfb_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
