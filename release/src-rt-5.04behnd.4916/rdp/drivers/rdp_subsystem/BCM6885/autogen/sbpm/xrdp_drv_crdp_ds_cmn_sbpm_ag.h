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


#ifndef _XRDP_DRV_CRDP_DS_CMN_SBPM_AG_H_
#define _XRDP_DRV_CRDP_DS_CMN_SBPM_AG_H_

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
    bdmf_boolean excl_med;
    bdmf_boolean excl_low;
    bdmf_boolean busy;
    bdmf_boolean rdy;
} crdp_ds_cmn_sbpm_regs_bn_alloc_rply;

typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intervl;
    uint8_t keep_alive_cyc;
} crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl;

typedef struct
{
    bdmf_boolean free_ack;
    bdmf_boolean ack_stat;
    bdmf_boolean nack_stat;
    bdmf_boolean excl_high_stat;
    bdmf_boolean excl_med_stat;
    bdmf_boolean excl_low_stat;
    bdmf_boolean bsy;
    bdmf_boolean rdy;
} crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_rply;

typedef struct
{
    bdmf_boolean free_list_non_alloc_bn;
    bdmf_boolean get2free_non_alloc_bn;
    bdmf_boolean mc2free_non_alloc_bn;
    bdmf_boolean alloc_alrdy_alloc_bn;
    bdmf_boolean get_non_alloc_bn;
    bdmf_boolean get_res_non_alloc_bn;
    bdmf_boolean connect_non_alloc_bn;
    bdmf_boolean connect_to_non_alloc_pointed_bn;
    bdmf_boolean mc_inc_non_alloc_bn;
    uint8_t free_wo_context_comb;
    bdmf_boolean bac_underrun;
    bdmf_boolean mcst_overflow;
    bdmf_boolean empty_free_list;
    bdmf_boolean error_bb_cmd_msg;
} crdp_ds_cmn_sbpm_intr_ctrl_isr;


/**********************************************************************************************************************
 * init_max_bn: 
 *     Maximum BN value.
 * ram_init_start: 
 *     Start SBPM initialization for free BNs memory and MC memory (if it has MC).
 *     
 * bsy: 
 *     The bit is used  as busy  indication of buffer allocation request status (busy status)  by CPU.
 *     BPM asserts this bit on each valid request and de-asserts when request is treated.
 *     TRIGGERS_HW
 *     
 * rdy: 
 *     The bit is used  as ready indication of buffer allocation request status (ready status)  by CPU.
 *     BPM asserts this bit  when request is treated and de-asserts when new valid request is accepted, thus this is
 *     READY indication
 *     TRIGGERS_HW
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_init_free_list_set(uint8_t sbpm_id, uint16_t init_max_bn, bdmf_boolean ram_init_start, bdmf_boolean bsy, bdmf_boolean rdy);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_init_free_list_get(uint8_t sbpm_id, uint16_t *init_max_bn, bdmf_boolean *ram_init_start, bdmf_boolean *bsy, bdmf_boolean *rdy);

/**********************************************************************************************************************
 * req: 
 *     rsv
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_set(uint8_t sbpm_id, bdmf_boolean req);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_get(uint8_t sbpm_id, bdmf_boolean *req);

/**********************************************************************************************************************
 * alloc_bn_valid: 
 *     alloc_bn_valid
 *     CHANGED_BY_HW
 * alloc_bn: 
 *     alloc_bn
 *     CHANGED_BY_HW
 *     
 * ack: 
 *     ack
 *     CHANGED_BY_HW
 *     
 * nack: 
 *     nack
 *     CHANGED_BY_HW
 *     
 * excl_high: 
 *     Exclusive bit is indication of Exclusive_high status of client with related Alloc request
 *     CHANGED_BY_HW
 *     
 * excl_med: 
 *     Exclusive bit is indication of Exclusive_med status of client with related Alloc request
 *     CHANGED_BY_HW
 *     
 * excl_low: 
 *     exclusive low
 * busy: 
 *     busy
 *     CHANGED_BY_HW
 *     
 * rdy: 
 *     rdy
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_alloc_rply_get(uint8_t sbpm_id, crdp_ds_cmn_sbpm_regs_bn_alloc_rply *regs_bn_alloc_rply);

/**********************************************************************************************************************
 * bn: 
 *     bufer number
 *     TRIGGERS_HW
 *     
 * mcst_val: 
 *     MCST value that should be added to current mulicast counter
 *     TRIGGERS_HW
 *     
 * ack_req: 
 *     Acknowledge request
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_set(uint8_t sbpm_id, uint16_t bn, uint8_t mcst_val, bdmf_boolean ack_req);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_get(uint8_t sbpm_id, uint16_t *bn, uint8_t *mcst_val, bdmf_boolean *ack_req);

/**********************************************************************************************************************
 * mcst_ack: 
 *     Acknowledge reply of MCST command
 *     CHANGED_BY_HW
 *     
 * mc_res: 
 *     MC increment result
 * bsy: 
 *     The bit is used  as busy  indication of MCST request status (busy status)  by CPU
 *     SBPM asserts this bit on each valid request and de-asserts when request is treated:
 *     1 - request is busy,
 *     0- request is not busy (ready)
 *     CHANGED_BY_HW
 *     
 * rdy: 
 *     The bit is used  as ready indication of MCST request status (ready status)  by CPU.
 *     SBPM asserts this bit  when request is treated and de-asserts when new valid request is accepted, thus this is
 *     READY indication:
 *     1 - request is ready,
 *     0- request is not ready (busy)
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_mcst_inc_rply_get(uint8_t sbpm_id, bdmf_boolean *mcst_ack, uint8_t *mc_res, bdmf_boolean *bsy, bdmf_boolean *rdy);

/**********************************************************************************************************************
 * bn: 
 *     bn
 *     TRIGGERS_HW
 *     
 * ack_req: 
 *     ack_req for Connect command (should be always set)
 *     TRIGGERS_HW
 *     
 * pointed_bn: 
 *     pointed_bn
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_set(uint8_t sbpm_id, uint16_t bn, bdmf_boolean ack_req, uint16_t pointed_bn);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_get(uint8_t sbpm_id, uint16_t *bn, bdmf_boolean *ack_req, uint16_t *pointed_bn);

/**********************************************************************************************************************
 * connect_ack: 
 *     Acknowledge reply on Connect request
 *     CHANGED_BY_HW
 *     
 * busy: 
 *     busy bit
 *     CHANGED_BY_HW
 *     
 * rdy: 
 *     ready bit
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_connect_rply_get(uint8_t sbpm_id, bdmf_boolean *connect_ack, bdmf_boolean *busy, bdmf_boolean *rdy);

/**********************************************************************************************************************
 * bn: 
 *     Get Next Buffer of current BN (used in this field)
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_get_next_set(uint8_t sbpm_id, uint16_t bn);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_get_next_get(uint8_t sbpm_id, uint16_t *bn);

/**********************************************************************************************************************
 * bn_valid: 
 *     Used for validation of Next BN reply
 *     CHANGED_BY_HW
 *     
 * next_bn: 
 *     Next BN - reply of Get_next command
 *     CHANGED_BY_HW
 *     
 * busy: 
 *     Get Next command is busy
 *     CHANGED_BY_HW
 *     
 * rdy: 
 *     Get Next command is ready
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_get_next_rply_get(uint8_t sbpm_id, bdmf_boolean *bn_valid, uint16_t *next_bn, bdmf_boolean *busy, bdmf_boolean *rdy);

/**********************************************************************************************************************
 * bypass_clk_gate: 
 *     If set to 1b1 will disable the clock gate logic such to always enable the clock
 *     TRIGGERS_HW
 *     
 * timer_val: 
 *     For how long should the clock stay active once all conditions for clock disable are met.
 *     TRIGGERS_HW
 *     
 * keep_alive_en: 
 *     Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being
 *     removed completely will occur
 *     TRIGGERS_HW
 *     
 * keep_alive_intervl: 
 *     If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active
 *     TRIGGERS_HW
 *     
 * keep_alive_cyc: 
 *     If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled
 *     (minus the KEEP_ALIVE_INTERVAL)
 *     
 *     So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.
 *     TRIGGERS_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl_set(uint8_t sbpm_id, const crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl *regs_sbpm_clk_gate_cntrl);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl_get(uint8_t sbpm_id, crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl *regs_sbpm_clk_gate_cntrl);

/**********************************************************************************************************************
 * plen: 
 *     plen
 * sop_offset: 
 *     number of packets
 * calc: 
 *     rsv
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1_set(uint8_t sbpm_id, uint16_t plen, uint16_t sop_offset, bdmf_boolean calc);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1_get(uint8_t sbpm_id, uint16_t *plen, uint16_t *sop_offset, bdmf_boolean *calc);

/**********************************************************************************************************************
 * first_bn: 
 *     First BN
 * num_packet: 
 *     number of packets
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2_set(uint8_t sbpm_id, uint16_t first_bn, uint8_t num_packet);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2_get(uint8_t sbpm_id, uint16_t *first_bn, uint8_t *num_packet);

/**********************************************************************************************************************
 * free_ack: 
 *     Acknowledge on Free command
 *     CHANGED_BY_HW
 *     
 * ack_stat: 
 *     ACK status of CPU
 *     CHANGED_BY_HW
 *     
 * nack_stat: 
 *     NACK status of CPU
 *     CHANGED_BY_HW
 *     
 * excl_high_stat: 
 *     Exclusive_high status of CPU
 *     CHANGED_BY_HW
 *     
 * excl_med_stat: 
 *     Exclusive_med status of CPU
 *     CHANGED_BY_HW
 *     
 * excl_low_stat: 
 *     Exclusive_low status of CPU
 *     CHANGED_BY_HW
 *     
 * bsy: 
 *     Busy bit of command (command is currently in execution)
 *     CHANGED_BY_HW
 *     
 * rdy: 
 *     Ready bit of command (ready for new command execution)
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_rply_get(uint8_t sbpm_id, crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_rply *regs_bn_free_without_contxt_rply);

/**********************************************************************************************************************
 * gl_bat: 
 *     Global Threshold for Allocated BN = maximal total number of BNs in SBPM
 * gl_bah: 
 *     Global Hysteresis for Allocated BN = hysteresis value related to maximal total threshold of SRAM BNs
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_trsh_set(uint8_t sbpm_id, uint16_t gl_bat, uint16_t gl_bah);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_trsh_get(uint8_t sbpm_id, uint16_t *gl_bat, uint16_t *gl_bah);

/**********************************************************************************************************************
 * select_bus: 
 *     select bus. the bus index should be mentioned in onehot writting:
 *     bus0 = 0001
 *     bus1 = 0010
 *     bus2 = 0100
 *     bus3 = 1000
 *     
 * window_start: 
 *     This field set the start of the bn allocation range.
 *     
 *     Vector bit register dbg_bn_alloc[MAX_BN-1:0]. Each bit in the register is corresponding to a BN allocation.
 *     For simulation the design will support 2K (ingress) and 16K (egress) bits vector. For synthesis the design will
 *     support only 2K bits vector with start BN window allocation and. It also affects the interrupts.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_set(uint8_t sbpm_id, uint8_t select_bus, uint16_t window_start);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_get(uint8_t sbpm_id, uint8_t *select_bus, uint16_t *window_start);

/**********************************************************************************************************************
 * val: 
 *     Global BN counter
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_bac_get(uint8_t sbpm_id, uint16_t *val);

/**********************************************************************************************************************
 * exclt: 
 *     exclusive high threshold
 * exclh: 
 *     exclusive histeresis threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh_set(uint8_t sbpm_id, uint16_t exclt, uint16_t exclh);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh_get(uint8_t sbpm_id, uint16_t *exclt, uint16_t *exclh);

/**********************************************************************************************************************
 * exclt: 
 *     exclusive low threshold
 * exclh: 
 *     exclusive histeresis threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh_set(uint8_t sbpm_id, uint16_t exclt, uint16_t exclh);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh_get(uint8_t sbpm_id, uint16_t *exclt, uint16_t *exclh);

/**********************************************************************************************************************
 * exclt: 
 *     exclusive low threshold
 * exclh: 
 *     exclusive histeresis threshold
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh_set(uint8_t sbpm_id, uint16_t exclt, uint16_t exclh);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh_get(uint8_t sbpm_id, uint16_t *exclt, uint16_t *exclh);

/**********************************************************************************************************************
 * ack_stts: 
 *     Ack/Nack status
 *     0 - NACK
 *     1 - ACK
 * excl_high_stts: 
 *     High EXCL/Non-Excl
 *     0 - non_exclusive
 *     1 - exclusive
 *     CHANGED_BY_HW
 *     
 * excl_med_stts: 
 *     Med EXCL/Non-Excl status.
 *     0 - non_exclusive
 *     1 - exclusive
 *     CHANGED_BY_HW
 *     
 * excl_low_stts: 
 *     Low EXCL/Non-Excl status
 *     0 - non_exclusive
 *     1 - exclusive
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_gl_status_get(uint8_t sbpm_id, bdmf_boolean *ack_stts, bdmf_boolean *excl_high_stts, bdmf_boolean *excl_med_stts, bdmf_boolean *excl_low_stts);

/**********************************************************************************************************************
 * cmd_sa: 
 *     Source addres of command that caused Interrupt
 * cmd_ta: 
 *     Target address of command that caused Interrupt
 * bn_src: 
 *     The non-allocated BN that caused the interrupt (non_alloc interrupt).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_iir_addr_get(uint8_t sbpm_id, uint16_t *cmd_sa, uint8_t *cmd_ta, uint16_t *bn_src);

/**********************************************************************************************************************
 * cmd_data_0to31: 
 *     Interrupt command data lowest 32-bit  (latched from BB data[31:0] or CPU request data)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_iir_low_get(uint8_t sbpm_id, uint32_t *cmd_data_0to31);

/**********************************************************************************************************************
 * cmd_data_32to63: 
 *     Data (bits [63:32], with reserved bits) of the command that caused interrupt
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_iir_high_get(uint8_t sbpm_id, uint32_t *cmd_data_32to63);

/**********************************************************************************************************************
 * sbpm_fifo_empty: 
 *     SBPM FIFO empty status
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec0_get(uint8_t sbpm_id, uint32_t *sbpm_fifo_empty);

/**********************************************************************************************************************
 * sbpm_fifo_full: 
 *     SBPM FIFO full status
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec1_get(uint8_t sbpm_id, uint32_t *sbpm_fifo_full);

/**********************************************************************************************************************
 * overwr_ra: 
 *     The new RA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1_set(uint8_t sbpm_id, uint32_t overwr_ra);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1_get(uint8_t sbpm_id, uint32_t *overwr_ra);

/**********************************************************************************************************************
 * id_2overwr: 
 *     this field contains the users id that you want to override its default RA
 * overwr_valid: 
 *     the overwr mechanism will be used only if this bit is active (1).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2_set(uint8_t sbpm_id, uint16_t id_2overwr, bdmf_boolean overwr_valid);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2_get(uint8_t sbpm_id, uint16_t *id_2overwr, bdmf_boolean *overwr_valid);

/**********************************************************************************************************************
 * val: 
 *     This is the maximum BN value that have been recorded.
 *     
 * clr: 
 *     max value clear
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bac_max_set(uint8_t sbpm_id, uint16_t val, bdmf_boolean clr);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_bac_max_get(uint8_t sbpm_id, uint16_t *val, bdmf_boolean *clr);

/**********************************************************************************************************************
 * spr: 
 *     reserved bits
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_spare_set(uint8_t sbpm_id, uint32_t spr);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sbpm_spare_get(uint8_t sbpm_id, uint32_t *spr);

/**********************************************************************************************************************
 * size: 
 *     Configure the buffer number (BN) size for BN free without context calculation.
 *     
 *     0: BN is 128 bytes
 *     1: BN is 256 bytes
 *     2: BN is 512 bytes
 *     3: reserved
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_cfg_bn_size_set(uint8_t sbpm_id, uint8_t size);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_cfg_bn_size_get(uint8_t sbpm_id, uint8_t *size);

/**********************************************************************************************************************
 * val: 
 *     bits 0-2  : weights for connect (wr access)
 *     bits 3-5  : get fifo a (rd access)
 *     bits 6-8  : get fifo b (rd access)
 *     bits 9-11 : mc2get fifo (rd access)
 *     bits 12-14: rsv
 *     bits 15-17: rsv
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_ll_weight_arb_set(uint8_t sbpm_id, uint32_t val);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_ll_weight_arb_get(uint8_t sbpm_id, uint32_t *val);

/**********************************************************************************************************************
 * start: 
 *     runner start number
 * end: 
 *     runner end number
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g0_set(uint8_t sbpm_id, uint16_t start, uint16_t end);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g0_get(uint8_t sbpm_id, uint16_t *start, uint16_t *end);

/**********************************************************************************************************************
 * start: 
 *     runner start number
 * end: 
 *     runner end number
 * valid: 
 *     rsv
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g1_set(uint8_t sbpm_id, uint16_t start, uint16_t end, bdmf_boolean valid);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_sp_rnr_g1_get(uint8_t sbpm_id, uint16_t *start, uint16_t *end, bdmf_boolean *valid);

/**********************************************************************************************************************
 * nack_mask: 
 *     non-runner client number/ID
 * valid: 
 *     valid
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_nack_mask_get(uint8_t sbpm_id, uint16_t *nack_mask, bdmf_boolean *valid);

/**********************************************************************************************************************
 * excl_mask: 
 *     This register holds the number of the client that should receive congestion message.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_excl_mask_set(uint8_t sbpm_id, uint16_t excl_mask);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_regs_excl_mask_get(uint8_t sbpm_id, uint16_t *excl_mask);

/**********************************************************************************************************************
 * free_list_non_alloc_bn: 
 *     This error bit is set when a non allocated BN is free by free list command.
 *     
 *     Only the lower part of the free list command is captured (the first 4 BNs).
 * get2free_non_alloc_bn: 
 *     This error bit is set when a non allocated BN is free by free w/o context command.
 *     
 *     The command source is not captured.
 * mc2free_non_alloc_bn: 
 *     This error bit is set when a non allocated BN is free by free w/o context command. This BN is the head BN or
 *     the third BN when mcst_srch == 4.
 *     
 *     The command source is not captured.
 * alloc_alrdy_alloc_bn: 
 *     This bit indicates that an already allocated BN was allocated again.
 *     
 * get_non_alloc_bn: 
 *     This bit error indicates that a get request is executed on non allocated BN
 * get_res_non_alloc_bn: 
 *     This bit error indicates that a get result is not allocated.
 * connect_non_alloc_bn: 
 *     This bit indicates a connect request was set for non allocated BN
 *     
 * connect_to_non_alloc_pointed_bn: 
 *     This bit indicates a connect request is set to non allocated pointed BN.
 *     
 * mc_inc_non_alloc_bn: 
 *     This bit error indicates that a mc increment request is for non allocated BN or If the command is free w/o
 *     context 1st/2nd level BNs are not allocated.
 * free_wo_context_comb: 
 *     This bits indicate that the free w/o context command was sent with illegal combinations.
 *     
 *     bit [0]: Number of BNs == 1 and 2nd BN valid == 1.
 *     
 *     bit [1]: Number of BNs is lower than decrements value (which could be 1, 2 or 3).
 *     
 *     bit [2]: mcst_srch == 0 and 2nd BN valid == 1.
 *     
 *     bit [3]: Number of BNs == 0.
 *     
 *     bit [4]: mcst_srch != 0 and 2nd BN valid == 0 and Number of BNs > 1.
 *     
 *     bit [5]: mcst_srch == 4 and Number of BNs < 3.
 *     
 * bac_underrun: 
 *     This error bit indicates under-run state of SBPM Buffer Allocated Counter.
 *     
 *     The source command is not captured.
 *     
 * mcst_overflow: 
 *     This error bit indicates if the Multi Cast result is overflow.
 * empty_free_list: 
 *     This error bit indicates the bn free list command is empty. No valid bit was set.
 * error_bb_cmd_msg: 
 *     This error bit indicates undefined BB command message was sent.
 *     The cmd == 101b or cmd == 111b are undefined.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_isr_set(uint8_t sbpm_id, const crdp_ds_cmn_sbpm_intr_ctrl_isr *intr_ctrl_isr);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_isr_get(uint8_t sbpm_id, crdp_ds_cmn_sbpm_intr_ctrl_isr *intr_ctrl_isr);

/**********************************************************************************************************************
 * ism: 
 *     Status Masked of corresponding interrupt source in the ISR
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ism_get(uint8_t sbpm_id, uint32_t *ism);

/**********************************************************************************************************************
 * iem: 
 *     Each bit in the mask controls the corresponding interrupt source in the IER
 *     TRIGGERED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ier_set(uint8_t sbpm_id, uint32_t iem);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_ier_get(uint8_t sbpm_id, uint32_t *iem);

/**********************************************************************************************************************
 * ist: 
 *     Each bit in the mask tests the corresponding interrupt source in the ISR
 *     CHANGED_BY_HW
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_itr_set(uint8_t sbpm_id, uint32_t ist);
bdmf_error_t ag_drv_crdp_ds_cmn_sbpm_intr_ctrl_itr_get(uint8_t sbpm_id, uint32_t *ist);

#ifdef USE_BDMF_SHELL
enum
{
    cli_crdp_ds_cmn_sbpm_regs_init_free_list,
    cli_crdp_ds_cmn_sbpm_regs_bn_alloc,
    cli_crdp_ds_cmn_sbpm_regs_bn_alloc_rply,
    cli_crdp_ds_cmn_sbpm_regs_mcst_inc,
    cli_crdp_ds_cmn_sbpm_regs_mcst_inc_rply,
    cli_crdp_ds_cmn_sbpm_regs_bn_connect,
    cli_crdp_ds_cmn_sbpm_regs_bn_connect_rply,
    cli_crdp_ds_cmn_sbpm_regs_get_next,
    cli_crdp_ds_cmn_sbpm_regs_get_next_rply,
    cli_crdp_ds_cmn_sbpm_regs_sbpm_clk_gate_cntrl,
    cli_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg1,
    cli_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_reg2,
    cli_crdp_ds_cmn_sbpm_regs_bn_free_without_contxt_rply,
    cli_crdp_ds_cmn_sbpm_regs_gl_trsh,
    cli_crdp_ds_cmn_sbpm_regs_sbpm_dbg,
    cli_crdp_ds_cmn_sbpm_regs_gl_bac,
    cli_crdp_ds_cmn_sbpm_regs_gl_excl_high_trsh,
    cli_crdp_ds_cmn_sbpm_regs_gl_excl_low_trsh,
    cli_crdp_ds_cmn_sbpm_regs_gl_excl_med_trsh,
    cli_crdp_ds_cmn_sbpm_regs_sbpm_gl_status,
    cli_crdp_ds_cmn_sbpm_regs_iir_addr,
    cli_crdp_ds_cmn_sbpm_regs_sbpm_iir_low,
    cli_crdp_ds_cmn_sbpm_regs_sbpm_iir_high,
    cli_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec0,
    cli_crdp_ds_cmn_sbpm_regs_sbpm_dbg_vec1,
    cli_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg1,
    cli_crdp_ds_cmn_sbpm_regs_sbpm_raddr_decoder_reg2,
    cli_crdp_ds_cmn_sbpm_regs_bac_max,
    cli_crdp_ds_cmn_sbpm_regs_sbpm_spare,
    cli_crdp_ds_cmn_sbpm_regs_cfg_bn_size,
    cli_crdp_ds_cmn_sbpm_regs_ll_weight_arb,
    cli_crdp_ds_cmn_sbpm_regs_sp_rnr_g0,
    cli_crdp_ds_cmn_sbpm_regs_sp_rnr_g1,
    cli_crdp_ds_cmn_sbpm_regs_nack_mask,
    cli_crdp_ds_cmn_sbpm_regs_excl_mask,
    cli_crdp_ds_cmn_sbpm_intr_ctrl_isr,
    cli_crdp_ds_cmn_sbpm_intr_ctrl_ism,
    cli_crdp_ds_cmn_sbpm_intr_ctrl_ier,
    cli_crdp_ds_cmn_sbpm_intr_ctrl_itr,
};

int bcm_crdp_ds_cmn_sbpm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_crdp_ds_cmn_sbpm_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
