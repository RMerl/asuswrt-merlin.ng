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

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

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

typedef struct
{
    bdmf_boolean bn_valid;
    uint16_t next_bn;
    bdmf_boolean bn_null;
    uint8_t mcnt_val;
    bdmf_boolean busy;
    bdmf_boolean rdy;
} sbpm_regs_get_next_rply;

typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intervl;
    uint8_t keep_alive_cyc;
} sbpm_regs_sbpm_clk_gate_cntrl;

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

typedef struct
{
    uint8_t alloc_sm;
    bdmf_boolean cnnct_sm;
    uint8_t mcint_sm;
    uint8_t free_w_cnxt_sm;
    uint8_t free_wo_cnxt_sm;
    uint8_t gn_sm;
    uint8_t multi_gn_sm;
    uint16_t free_lst_hd;
} sbpm_regs_sbpm_dbg_vec0;

typedef struct
{
    bdmf_boolean in2e_valid;
    uint8_t multi_gn_valid;
    uint8_t ug_active;
    bdmf_boolean tx_cmd_full;
    bdmf_boolean rx_fifo_pop;
    bdmf_boolean ram_init_start;
    bdmf_boolean ram_init_done;
    uint16_t rx_fifo_data;
    bdmf_boolean free_decode;
    bdmf_boolean in2e_decode;
    bdmf_boolean free_wo_decode;
    bdmf_boolean get_nxt_decode;
    bdmf_boolean multi_get_nxt_decode;
    bdmf_boolean cnct_decode;
    bdmf_boolean free_w_decode;
    bdmf_boolean mcin_decode;
    bdmf_boolean alloc_decode;
} sbpm_regs_sbpm_dbg_vec1;

typedef struct
{
    bdmf_boolean tx_data_full;
    bdmf_boolean tx_fifo_empty;
    bdmf_boolean lcl_stts_full;
    bdmf_boolean lcl_stts_empty;
    bdmf_boolean tx_cmd_full;
    bdmf_boolean tx_cmd_fifo_empty;
    uint8_t bb_decoder_dest_id;
    bdmf_boolean tx_bbh_send_in_progress;
    uint8_t sp_2send;
    uint8_t tx2data_fifo_taddr;
    bdmf_boolean cpu_access;
    bdmf_boolean bbh_access;
    bdmf_boolean rnr_access;
} sbpm_regs_sbpm_dbg_vec2;

typedef struct
{
    bdmf_boolean alloc_rply;
    uint16_t bn_rply;
    bdmf_boolean txfifo_alloc_ack;
    bdmf_boolean tx_fifo_mcinc_ack;
    bdmf_boolean txfifo_cnct_ack;
    bdmf_boolean txfifo_gt_nxt_rply;
    bdmf_boolean txfifo_mlti_gt_nxt_rply;
    uint8_t tx_msg_pipe_sm;
    bdmf_boolean send_stt_sm;
    bdmf_boolean txfifo_in2estts_chng;
} sbpm_regs_sbpm_dbg_vec3;

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
    bdmf_boolean invalid_in2e_overflow;
    bdmf_boolean invalid_in2e_underflow;
    bdmf_boolean bac_underrun_ug0;
    bdmf_boolean bac_underrun_ug1;
} sbpm_intr_ctrl_isr;


/**********************************************************************************************************************
 * sbpm_nack_mask_high: 
 *     bit i value determine if SP number i got nack or not
 * sbpm_nack_mask_low: 
 *     bit i value determine if SP number i got nack or not
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_nack_mask_get(uint32_t *sbpm_nack_mask_high, uint32_t *sbpm_nack_mask_low);

/**********************************************************************************************************************
 * bac: 
 *     Global BN counter
 * ug1bac: 
 *     Baffer Allocated Counter
 * ug0bac: 
 *     UG0 counter for allocated BNs
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_bac_get(uint16_t *bac, uint16_t *ug1bac, uint16_t *ug0bac);

/**********************************************************************************************************************
 * init_base_addr: 
 *     init_base_addr
 * init_offset: 
 *     init_offset
 * bsy: 
 *     The bit is used  as busy  indication of buffer allocation request status (busy status)  by CPU.
 *     BPM asserts this bit on each valid request and de-asserts when request is treated.
 * rdy: 
 *     The bit is used  as ready indication of buffer allocation request status (ready status)  by CPU.
 *     BPM asserts this bit  when request is treated and de-asserts when new valid request is accepted, thus this is
 *     READY indication
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_init_free_list_set(uint16_t init_base_addr, uint16_t init_offset, bdmf_boolean bsy, bdmf_boolean rdy);
bdmf_error_t ag_drv_sbpm_regs_init_free_list_get(uint16_t *init_base_addr, uint16_t *init_offset, bdmf_boolean *bsy, bdmf_boolean *rdy);

/**********************************************************************************************************************
 * sa: 
 *     Source address used by Alloc BN command (may be used for alloc on behalf another user)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_bn_alloc_set(uint8_t sa);
bdmf_error_t ag_drv_sbpm_regs_bn_alloc_get(uint8_t *sa);

/**********************************************************************************************************************
 * alloc_bn_valid: 
 *     alloc_bn_valid
 * alloc_bn: 
 *     alloc_bn
 * ack: 
 *     ack
 * nack: 
 *     nack
 * excl_high: 
 *     Exclusive bit is indication of Exclusive_high status of client with related Alloc request
 * excl_low: 
 *     Exclusive bit is indication of Exclusive_low status of client with related Alloc request
 * busy: 
 *     busy
 * rdy: 
 *     rdy
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_bn_alloc_rply_get(sbpm_regs_bn_alloc_rply *regs_bn_alloc_rply);

/**********************************************************************************************************************
 * head_bn: 
 *     head_bn
 * sa: 
 *     Source addres used for free comand (may be used for freeing BN on behalf another port)
 * offset: 
 *     Offset (or length) = number of BNs in packet that is going to be freed
 * ack: 
 *     Ack request
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_low_set(uint16_t head_bn, uint8_t sa, uint8_t offset, bdmf_boolean ack);
bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_low_get(uint16_t *head_bn, uint8_t *sa, uint8_t *offset, bdmf_boolean *ack);

/**********************************************************************************************************************
 * last_bn: 
 *     Last BN in packet that is going to be freed
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_high_set(uint16_t last_bn);
bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_high_get(uint16_t *last_bn);

/**********************************************************************************************************************
 * bn: 
 *     bufer number
 * mcst_val: 
 *     MCST value that should be added to current mulicast counter
 * ack_req: 
 *     Acknowledge request
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_mcst_inc_set(uint16_t bn, uint8_t mcst_val, bdmf_boolean ack_req);
bdmf_error_t ag_drv_sbpm_regs_mcst_inc_get(uint16_t *bn, uint8_t *mcst_val, bdmf_boolean *ack_req);

/**********************************************************************************************************************
 * mcst_ack: 
 *     Acknowledge reply of MCST command
 * bsy: 
 *     The bit is used  as busy  indication of MCST request status (busy status)  by CPU
 *     SBPM asserts this bit on each valid request and de-asserts when request is treated:
 *     1 - request is busy,
 *     0- request is not busy (ready)
 * rdy: 
 *     The bit is used  as ready indication of MCST request status (ready status)  by CPU.
 *     SBPM asserts this bit  when request is treated and de-asserts when new valid request is accepted, thus this is
 *     READY indication:
 *     1 - request is ready,
 *     0- request is not ready (busy)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_mcst_inc_rply_get(bdmf_boolean *mcst_ack, bdmf_boolean *bsy, bdmf_boolean *rdy);

/**********************************************************************************************************************
 * bn: 
 *     bn
 * ack_req: 
 *     ack_req for Connect command (should be always set)
 * wr_req: 
 *     Used for Direct Write (for work arround)
 * pointed_bn: 
 *     pointed_bn
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_bn_connect_set(uint16_t bn, bdmf_boolean ack_req, bdmf_boolean wr_req, uint16_t pointed_bn);
bdmf_error_t ag_drv_sbpm_regs_bn_connect_get(uint16_t *bn, bdmf_boolean *ack_req, bdmf_boolean *wr_req, uint16_t *pointed_bn);

/**********************************************************************************************************************
 * connect_ack: 
 *     Acknowledge reply on Connect request
 * busy: 
 *     busy bit
 * rdy: 
 *     ready bit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_bn_connect_rply_get(bdmf_boolean *connect_ack, bdmf_boolean *busy, bdmf_boolean *rdy);

/**********************************************************************************************************************
 * bn: 
 *     Get Next Buffer of current BN (used in this field)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_get_next_set(uint16_t bn);
bdmf_error_t ag_drv_sbpm_regs_get_next_get(uint16_t *bn);

/**********************************************************************************************************************
 * bn_valid: 
 *     Used for validation of Next BN reply
 * next_bn: 
 *     Next BN - reply of Get_next command
 * bn_null: 
 *     Next BN is null indication
 * mcnt_val: 
 *     mcst cnt val
 * busy: 
 *     Get Next command is busy
 * rdy: 
 *     Get Next command is ready
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_get_next_rply_get(sbpm_regs_get_next_rply *regs_get_next_rply);

/**********************************************************************************************************************
 * bypass_clk_gate: 
 *     If set to 1b1 will disable the clock gate logic such to always enable the clock
 * timer_val: 
 *     For how long should the clock stay active once all conditions for clock disable are met.
 *     
 * keep_alive_en: 
 *     Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being
 *     removed completely will occur
 * keep_alive_intervl: 
 *     If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active
 * keep_alive_cyc: 
 *     If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled
 *     (minus the KEEP_ALIVE_INTERVAL)
 *     
 *     So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_clk_gate_cntrl_set(const sbpm_regs_sbpm_clk_gate_cntrl *regs_sbpm_clk_gate_cntrl);
bdmf_error_t ag_drv_sbpm_regs_sbpm_clk_gate_cntrl_get(sbpm_regs_sbpm_clk_gate_cntrl *regs_sbpm_clk_gate_cntrl);

/**********************************************************************************************************************
 * head_bn: 
 *     Head BN = First BN in packet that is going to be freed
 * sa: 
 *     source address used for command (may be used for performing command on behalf another port)
 * ack_req: 
 *     ACK request - should be always set
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_bn_free_without_contxt_set(uint16_t head_bn, uint8_t sa, bdmf_boolean ack_req);
bdmf_error_t ag_drv_sbpm_regs_bn_free_without_contxt_get(uint16_t *head_bn, uint8_t *sa, bdmf_boolean *ack_req);

/**********************************************************************************************************************
 * free_ack: 
 *     Acknowledge on Free command
 * ack_stat: 
 *     ACK status of CPU
 * nack_stat: 
 *     NACK status of CPU
 * excl_high_stat: 
 *     Exclusive_high status of CPU
 * excl_low_stat: 
 *     Exclusive_low status of CPU
 * bsy: 
 *     Busy bit of command (command is currently in execution)
 * rdy: 
 *     Ready bit of command (ready for new command execution)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_bn_free_without_contxt_rply_get(sbpm_regs_bn_free_without_contxt_rply *regs_bn_free_without_contxt_rply);

/**********************************************************************************************************************
 * free_ack: 
 *     Free command acknowledge
 * ack_state: 
 *     ACK status of CPU
 * nack_state: 
 *     NACK status of CPU
 * excl_high_state: 
 *     Exclusive high status of CPU
 * excl_low_state: 
 *     Exclusive low status of CPU
 * busy: 
 *     Busy bit of command
 * rdy: 
 *     Ready bit of command
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_bn_free_with_contxt_rply_get(sbpm_regs_bn_free_with_contxt_rply *regs_bn_free_with_contxt_rply);

/**********************************************************************************************************************
 * gl_bat: 
 *     Global Threshold for Allocated BN = maximal total number of BNs in SBPM
 * gl_bah: 
 *     Global Hysteresis for Allocated BN = hysteresis value related to maximal total threshold of SRAM BNs
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_gl_trsh_set(uint16_t gl_bat, uint16_t gl_bah);
bdmf_error_t ag_drv_sbpm_regs_sbpm_gl_trsh_get(uint16_t *gl_bat, uint16_t *gl_bah);

/**********************************************************************************************************************
 * ug_bat: 
 *     Current UG Threshold for Allocated BN
 * ug_bah: 
 *     Current UG hysteresis Threshold for Allocated BN
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_trsh_set(uint16_t ug_bat, uint16_t ug_bah);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_trsh_get(uint16_t *ug_bat, uint16_t *ug_bah);

/**********************************************************************************************************************
 * ug_bat: 
 *     Current UG Threshold for Allocated BN
 * ug_bah: 
 *     Current UG hysteresis delta Threshold for Allocated BN
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_trsh_set(uint16_t ug_bat, uint16_t ug_bah);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_trsh_get(uint16_t *ug_bat, uint16_t *ug_bah);

/**********************************************************************************************************************
 * select_bus: 
 *     select bus. the bus index should be mentioned in onehot writting:
 *     bus0 = 0001
 *     bus1 = 0010
 *     bus2 = 0100
 *     bus3 = 1000
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_dbg_set(uint8_t select_bus);
bdmf_error_t ag_drv_sbpm_regs_sbpm_dbg_get(uint8_t *select_bus);

/**********************************************************************************************************************
 * exclt: 
 *     exclusive high threshold
 * exclh: 
 *     exclusive histeresis threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_excl_high_trsh_set(uint16_t exclt, uint16_t exclh);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_excl_high_trsh_get(uint16_t *exclt, uint16_t *exclh);

/**********************************************************************************************************************
 * exclt: 
 *     exclusive high threshold
 * exclh: 
 *     exclusive histeresis threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_excl_high_trsh_set(uint16_t exclt, uint16_t exclh);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_excl_high_trsh_get(uint16_t *exclt, uint16_t *exclh);

/**********************************************************************************************************************
 * exclt: 
 *     exclusive low threshold
 * exclh: 
 *     exclusive histeresis threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_excl_low_trsh_set(uint16_t exclt, uint16_t exclh);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug0_excl_low_trsh_get(uint16_t *exclt, uint16_t *exclh);

/**********************************************************************************************************************
 * exclt: 
 *     exclusive low threshold
 * exclh: 
 *     exclusive histeresis threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_excl_low_trsh_set(uint16_t exclt, uint16_t exclh);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug1_excl_low_trsh_get(uint16_t *exclt, uint16_t *exclh);

/**********************************************************************************************************************
 * ug_ack_stts: 
 *     Ack/Nack status per UG.
 *     0 - NACK
 *     1 - ACK
 *     
 *     bit [0] in field matches UG0 ACK status,
 *     bit [1] in field matches UG1 ACK status,
 *     bit [2] in field matches UG2 ACK status,
 *     bit [3] in field matches UG3 ACK status,
 *     bit [4] in field matches UG4 ACK status,
 *     bit [5] in field matches UG5 ACK status,
 *     bit [6] in field matches UG6 ACK status,
 *     bit [7] in field matches UG7 ACK status,
 * ug_excl_high_stts: 
 *     High EXCL/Non-Excl status per UG.
 *     0 - non_exclusive
 *     1 - exclusive
 *     
 * ug_excl_low_stts: 
 *     Low EXCL/Non-Excl status per UG.
 *     0 - non_exclusive
 *     1 - exclusive
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_status_get(uint8_t *ug_ack_stts, uint8_t *ug_excl_high_stts, uint8_t *ug_excl_low_stts);

/**********************************************************************************************************************
 * search_depth: 
 *     Depth (or maximal threshold) for search during Free without context
 * max_search_en: 
 *     Enable for max search  during Free without context
 * chck_last_en: 
 *     Enable for Last BN checking  during Free with context
 * freeze_in_error: 
 *     Freeze Ug/Global counters + mask access to SBPM RAM while in ERROR state
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_error_handling_params_set(uint8_t search_depth, bdmf_boolean max_search_en, bdmf_boolean chck_last_en, bdmf_boolean freeze_in_error);
bdmf_error_t ag_drv_sbpm_regs_error_handling_params_get(uint8_t *search_depth, bdmf_boolean *max_search_en, bdmf_boolean *chck_last_en, bdmf_boolean *freeze_in_error);

/**********************************************************************************************************************
 * cmd_sa: 
 *     Source addres of command that caused Interrupt
 * cmd_ta: 
 *     Target address of command that caused Interrupt
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_iir_addr_get(uint8_t *cmd_sa, uint8_t *cmd_ta);

/**********************************************************************************************************************
 * cmd_data_0to31: 
 *     Interrupt command data lowest 32-bit  (latched from BB data[31:0] or CPU request data)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_iir_low_get(uint32_t *cmd_data_0to31);

/**********************************************************************************************************************
 * cmd_data_32to63: 
 *     Data (bits [63:32], with reserved bits) of the command that caused interrupt
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_iir_high_get(uint32_t *cmd_data_32to63);

/**********************************************************************************************************************
 * alloc_sm: 
 *     Alloc State Machine
 *     {update, rd_head_cnxt}
 * cnnct_sm: 
 *     Connect State Machine
 *     {update}
 * mcint_sm: 
 *     Multicast incr State Machine
 *     {read,check,error,update}
 * free_w_cnxt_sm: 
 *     Free w cnxt State Machine
 *     {read,check,update,error}
 * free_wo_cnxt_sm: 
 *     Free w/o cnxt State Machine
 *     {read,check,update,error}
 * gn_sm: 
 *     Get next State Machine:
 *     {read,reply}
 * multi_gn_sm: 
 *     Those are the 4 Multi get next states:
 *     {rd_next,error,rd_last,wait}
 * free_lst_hd: 
 *     the value of the head of FREE list
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_dbg_vec0_get(sbpm_regs_sbpm_dbg_vec0 *regs_sbpm_dbg_vec0);

/**********************************************************************************************************************
 * in2e_valid: 
 *     sbpm_ingress2egress_valid bit
 * multi_gn_valid: 
 *     multi_get_next_valid bits
 * ug_active: 
 *     sbpm_ug_active 2 bits
 * tx_cmd_full: 
 *     sbpm_tx_cmd_fifo_full bit
 * rx_fifo_pop: 
 *     sbpm_rx_fifo_pop bit
 * ram_init_start: 
 *     sbpm_ram_init_start bit
 * ram_init_done: 
 *     sbpm_ram_init_done bit
 * rx_fifo_data: 
 *     RX FIFO Data in pipe
 * free_decode: 
 *     sbpm_free_rqst_dec
 * in2e_decode: 
 *     sbpm_in2e_rqst_dec
 * free_wo_decode: 
 *     sbpm_free_wo_cnxt_rqst_dec
 * get_nxt_decode: 
 *     sbpm_get_next_rqst_dec
 * multi_get_nxt_decode: 
 *     sbpm_multi_get_next_rqst_dec
 * cnct_decode: 
 *     sbpm_cnct_rqst_dec
 * free_w_decode: 
 *     sbpm_free_w_cnxt_rqst_dec
 * mcin_decode: 
 *     sbpm_mcinc_rqst_dec
 * alloc_decode: 
 *     sbpm_alloc_rqst_dec
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_dbg_vec1_get(sbpm_regs_sbpm_dbg_vec1 *regs_sbpm_dbg_vec1);

/**********************************************************************************************************************
 * tx_data_full: 
 *     sbpm_tx_data_fifo_full
 * tx_fifo_empty: 
 *     sbpm_tx_fifo_empty
 * lcl_stts_full: 
 *     sbpm_tx_cmd_local_stts_fifo_full
 * lcl_stts_empty: 
 *     sbpm_tx_cmd_local_stts_fifo_empty
 * tx_cmd_full: 
 *     sbpm_tx_cmd_fifo_full
 * tx_cmd_fifo_empty: 
 *     sbpm_tx_cmd_fifo_empty
 * bb_decoder_dest_id: 
 *     bb_decoder_dest_id
 *     This is the ID of the user that will recieve a message from SBPM
 * tx_bbh_send_in_progress: 
 *     sbpm_tx_bbh_send_in_progress bit
 * sp_2send: 
 *     sbpm_sp_2send - this is the user ID that is about to get stts msg
 * tx2data_fifo_taddr: 
 *     sbpm_tx2data_fifo_taddr[2:0] this is the opcode that describe the type of the reply
 * cpu_access: 
 *     sbpm_cpu_access bit
 * bbh_access: 
 *     sbpm_bbh_access bit
 * rnr_access: 
 *     sbpm_rnr_access bit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_dbg_vec2_get(sbpm_regs_sbpm_dbg_vec2 *regs_sbpm_dbg_vec2);

/**********************************************************************************************************************
 * alloc_rply: 
 *     ALLOC_RPLY bit
 * bn_rply: 
 *     BN_RPLY value
 * txfifo_alloc_ack: 
 *     sbpm_txfifo_alloc_ack
 * tx_fifo_mcinc_ack: 
 *     sbpm_txfifo_mcinc_ack
 * txfifo_cnct_ack: 
 *     sbpm_txfifo_cnct_ack
 * txfifo_gt_nxt_rply: 
 *     sbpm_txfifo_get_next_reply
 * txfifo_mlti_gt_nxt_rply: 
 *     sbpm_txfifo_multi_get_next_reply
 * tx_msg_pipe_sm: 
 *     sbpm_tx_msg_pipe_cur_sm
 * send_stt_sm: 
 *     sbpm_send_stat_sm_ps
 * txfifo_in2estts_chng: 
 *     sbpm_txfifo_ingress2egress_stts_change
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_dbg_vec3_get(sbpm_regs_sbpm_dbg_vec3 *regs_sbpm_dbg_vec3);

/**********************************************************************************************************************
 * sbpm_sp_bbh_low: 
 *     sbpm_sp_bbh_low bit i tells us if SP #i is a BBH (1) or not (0)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_bbh_low_set(uint32_t sbpm_sp_bbh_low);
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_bbh_low_get(uint32_t *sbpm_sp_bbh_low);

/**********************************************************************************************************************
 * sbpm_sp_bbh_high: 
 *     Not in use in 68360!
 *     sbpm_sp_bbh_high bit i tells us if SP #i is a BBH (1) or not (0)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_bbh_high_set(uint32_t sbpm_sp_bbh_high);
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_bbh_high_get(uint32_t *sbpm_sp_bbh_high);

/**********************************************************************************************************************
 * sbpm_sp_rnr_low: 
 *     sbpm_sp_rnr_low bit i tells us if SP #i is a runner (1) or not (0)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_rnr_low_set(uint32_t sbpm_sp_rnr_low);
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_rnr_low_get(uint32_t *sbpm_sp_rnr_low);

/**********************************************************************************************************************
 * sbpm_sp_rnr_high: 
 *     Not in use in 68360!
 *     sbpm_sp_rnr_high bit i tells us if SP #i is a runner (1) or not (0)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_rnr_high_set(uint32_t sbpm_sp_rnr_high);
bdmf_error_t ag_drv_sbpm_regs_sbpm_sp_rnr_high_get(uint32_t *sbpm_sp_rnr_high);

/**********************************************************************************************************************
 * sbpm_ug_map_low: 
 *     bit i value determine if SP number i belongs to UG0 (ingress) or UG1 (egress)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_map_low_set(uint32_t sbpm_ug_map_low);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_map_low_get(uint32_t *sbpm_ug_map_low);

/**********************************************************************************************************************
 * sbpm_ug_map_high: 
 *     Not in use in 68360!
 *     bit i value determine if SP number i belongs to UG0 (ingress) or UG1 (egress)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_map_high_set(uint32_t sbpm_ug_map_high);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_map_high_get(uint32_t *sbpm_ug_map_high);

/**********************************************************************************************************************
 * sbpm_excl_mask_low: 
 *     This register mark all the SPs that should get exclusive messages
 *     yes no
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_excl_mask_low_set(uint32_t sbpm_excl_mask_low);
bdmf_error_t ag_drv_sbpm_regs_sbpm_excl_mask_low_get(uint32_t *sbpm_excl_mask_low);

/**********************************************************************************************************************
 * sbpm_excl_mask_high: 
 *     Not in use in 68360!
 *     This register mark all the SPs that should get exclusive messages
 *     yes no
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_excl_mask_high_set(uint32_t sbpm_excl_mask_high);
bdmf_error_t ag_drv_sbpm_regs_sbpm_excl_mask_high_get(uint32_t *sbpm_excl_mask_high);

/**********************************************************************************************************************
 * id_2overwr: 
 *     this field contains the users id that you want to override its default RA
 * overwr_ra: 
 *     The new RA
 * overwr_valid: 
 *     the overwr mechanism will be used only if this bit is active (1).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_raddr_decoder_set(uint8_t id_2overwr, uint16_t overwr_ra, bdmf_boolean overwr_valid);
bdmf_error_t ag_drv_sbpm_regs_sbpm_raddr_decoder_get(uint8_t *id_2overwr, uint16_t *overwr_ra, bdmf_boolean *overwr_valid);

/**********************************************************************************************************************
 * sbpm_wr_data: 
 *     If SW want to write a whole word into the SBPMs RAM, it needs first to write the data to this register and
 *     then, send connect request with the wr_req bit asserted, with the address (BN field).
 *     
 *     In 68360 the only the 15 LSB are used
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_wr_data_set(uint32_t sbpm_wr_data);
bdmf_error_t ag_drv_sbpm_regs_sbpm_wr_data_get(uint32_t *sbpm_wr_data);

/**********************************************************************************************************************
 * ug0bacmax: 
 *     This is the maximum value that have been recorded on the UG0 counter.
 *     SW can write to this field in order to change the max record (for example write 0 to reset it)
 * ug1bacmax: 
 *     This is the maximum value that have been recorded on the UG1 counter.
 *     SW can write to this field in order to change the max record (for example write 0 to reset it)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_bac_max_set(uint16_t ug0bacmax, uint16_t ug1bacmax);
bdmf_error_t ag_drv_sbpm_regs_sbpm_ug_bac_max_get(uint16_t *ug0bacmax, uint16_t *ug1bacmax);

/**********************************************************************************************************************
 * gl_bac_clear_en: 
 *     sbpm_gl_bac_clear_en
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_spare_set(bdmf_boolean gl_bac_clear_en);
bdmf_error_t ag_drv_sbpm_regs_sbpm_spare_get(bdmf_boolean *gl_bac_clear_en);

/**********************************************************************************************************************
 * sbpm_cfg_bac_underrun_en: 
 *     When=1, enable UG1 BAC Underrun, to overcome scenario of Free is send after in2eg, but free is received at SBPM
 *     before In2Eg due to In2Eg Broad-bus leaf stuck
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_regs_sbpm_bac_underrun_set(bdmf_boolean sbpm_cfg_bac_underrun_en);
bdmf_error_t ag_drv_sbpm_regs_sbpm_bac_underrun_get(bdmf_boolean *sbpm_cfg_bac_underrun_en);

/**********************************************************************************************************************
 * bac_underrun: 
 *     This error bit indicates underrun state of SBPM Buffer Allocated Counter (one of User Groups) due to free
 *     operation (w/wo context). Note that ug0 bac is NOT updated/affected, whereas UG1 may be updated, according to
 *     config bit bac_underrun_en. SW can clear this bit by writing 1 to this field
 * mcst_overflow: 
 *     This error bit indicates if the Multi Cast value of a buffer is in overflow as a result of erroneous MCINC
 *     command
 * check_last_err: 
 *     This bit indicates error state on Last BN checking during Free with context request. SW can clear this bit by
 *     writing 1 to this field.
 * max_search_err: 
 *     This bit indicates error state on maximal search checking during Free without context request. SW can clear
 *     this bit by writing 1 to this field.
 * invalid_in2e: 
 *     This bit indicates invalid ingress2egress command (caused BAC under/overrun). SW can clear this bit by writing
 *     1 to this field.
 * multi_get_next_null: 
 *     This bit indicates Null encounter during one of the next BNs. SW can clear this bit by writing 0 to this field.
 * cnct_null: 
 *     This bit indicates connection of the NULL buffer to another buufer. SW can clear this bit by writing 0 to this
 *     field.
 * alloc_null: 
 *     This bit indicates allocation of the NULL buffer. SW can clear this bit by writing 0 to this field.
 * invalid_in2e_overflow: 
 *     This bit indicates invalid ingress2egress overflow command (caused BAC over-run). SW can clear this bit by
 *     writing 1 to this field.
 * invalid_in2e_underflow: 
 *     This bit indicates invalid ingress2egress underflow command (caused BAC under-run). SW can clear this bit by
 *     writing 1 to this field.
 * bac_underrun_ug0: 
 *     This error bit indicates underrun state of SBPM Buffer Allocated Counter UG0 due to free operation (w/wo
 *     context). Note that ug0 bac is NOT updated/affected. SW can clear this bit by writing 1 to this field
 * bac_underrun_ug1: 
 *     This error bit indicates underrun state of SBPM Buffer Allocated Counter UG1 due to free operation (w/wo
 *     context). Note that ug1 bac May be updated/affected, depending on bac_underrun_en config bit. SW can clear this
 *     bit by writing 1 to this field
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_intr_ctrl_isr_set(const sbpm_intr_ctrl_isr *intr_ctrl_isr);
bdmf_error_t ag_drv_sbpm_intr_ctrl_isr_get(sbpm_intr_ctrl_isr *intr_ctrl_isr);

/**********************************************************************************************************************
 * ism: 
 *     Status Masked of corresponding interrupt source in the ISR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_intr_ctrl_ism_get(uint32_t *ism);

/**********************************************************************************************************************
 * iem: 
 *     Each bit in the mask controls the corresponding interrupt source in the IER
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_intr_ctrl_ier_set(uint32_t iem);
bdmf_error_t ag_drv_sbpm_intr_ctrl_ier_get(uint32_t *iem);

/**********************************************************************************************************************
 * ist: 
 *     Each bit in the mask tests the corresponding interrupt source in the ISR
 **********************************************************************************************************************/
bdmf_error_t ag_drv_sbpm_intr_ctrl_itr_set(uint32_t ist);
bdmf_error_t ag_drv_sbpm_intr_ctrl_itr_get(uint32_t *ist);

#ifdef USE_BDMF_SHELL
enum
{
    cli_sbpm_nack_mask,
    cli_sbpm_bac,
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
    cli_sbpm_regs_sbpm_dbg,
    cli_sbpm_regs_sbpm_ug0_excl_high_trsh,
    cli_sbpm_regs_sbpm_ug1_excl_high_trsh,
    cli_sbpm_regs_sbpm_ug0_excl_low_trsh,
    cli_sbpm_regs_sbpm_ug1_excl_low_trsh,
    cli_sbpm_regs_sbpm_ug_status,
    cli_sbpm_regs_error_handling_params,
    cli_sbpm_regs_sbpm_iir_addr,
    cli_sbpm_regs_sbpm_iir_low,
    cli_sbpm_regs_sbpm_iir_high,
    cli_sbpm_regs_sbpm_dbg_vec0,
    cli_sbpm_regs_sbpm_dbg_vec1,
    cli_sbpm_regs_sbpm_dbg_vec2,
    cli_sbpm_regs_sbpm_dbg_vec3,
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
    cli_sbpm_regs_sbpm_spare,
    cli_sbpm_regs_sbpm_bac_underrun,
    cli_sbpm_intr_ctrl_isr,
    cli_sbpm_intr_ctrl_ism,
    cli_sbpm_intr_ctrl_ier,
    cli_sbpm_intr_ctrl_itr,
};

int bcm_sbpm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_sbpm_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
