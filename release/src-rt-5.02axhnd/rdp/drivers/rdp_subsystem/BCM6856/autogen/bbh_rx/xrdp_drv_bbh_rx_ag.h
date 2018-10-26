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

#ifndef _XRDP_DRV_BBH_RX_AG_H_
#define _XRDP_DRV_BBH_RX_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* patterndatalsb: Pattern_Data_LSB - Pattern Data[31:0]                                          */
/* patterndatamsb: Pattern_Data_MSB - Pattern Data[63:32]                                         */
/* patternmasklsb: Pattern_Mask_LSB - Pattern mask[31:0]                                          */
/* patternmaskmsb: Pattern_Mask_MSB - Pattern Mask[63:32]                                         */
/* pattenoffset: Pattern_recognition_offset - Defines the pattern recognition offset within the p */
/*               acket. Offset is 8 bytes resolution                                              */
/**************************************************************************************************/
typedef struct
{
    uint32_t patterndatalsb;
    uint32_t patterndatamsb;
    uint32_t patternmasklsb;
    uint32_t patternmaskmsb;
    uint8_t pattenoffset;
} bbh_rx_pattern_recog;


/**************************************************************************************************/
/* dispdropdis: Dispatcher_drop_disable - Disable dropping packets due to no space in the Dispatc */
/*              her.                                                                              */
/* sdmadropdis: SMDA_drop_disable - Disable dropping packets due to no space in the SDMA.         */
/* sbpmdropdis: SBPM_drop_disable - Disable dropping packets due to no space in the SBPM.         */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean dispdropdis;
    bdmf_boolean sdmadropdis;
    bdmf_boolean sbpmdropdis;
} bbh_rx_flow_ctrl_drops_config;


/**************************************************************************************************/
/* numofcd: Number_of_Chunk-Descriptors - Defines the size of the Chunk descripors FIFO in the DM */
/*          A.                                                                                    */
/* exclth: Exclusive_threshold - This field defines the number of occupied write chunks for dropp */
/*         ing normal or high priority packets.                                                   */
/* database: Data_base_address - The Data FIFO base address within the SDMA address space.The add */
/*           ress is in chunk resolution (128 bytes).The value should be identical to the relevan */
/*           t configuration in the SDMA.                                                         */
/* descbase: Descriptor_base_address - The Descriptor FIFO base address within the SDMA address s */
/*           pace.The address is in chunk descriptor resolution (8 bytes).The value  should be id */
/*           entical to the relevant configuration in the SDMA.                                   */
/**************************************************************************************************/
typedef struct
{
    uint8_t numofcd;
    uint8_t exclth;
    uint8_t database;
    uint8_t descbase;
} bbh_rx_sdma_config;


/**************************************************************************************************/
/* crc_err_ploam: PM_counter_value - PM counter value.                                            */
/* third_flow: PM_counter_value - PM counter value.                                               */
/* sop_after_sop: PM_counter_value - PM counter value.                                            */
/* no_sbpm_bn_ploam: PM_counter_value - PM counter value.                                         */
/**************************************************************************************************/
typedef struct
{
    uint32_t crc_err_ploam;
    uint32_t third_flow;
    uint32_t sop_after_sop;
    uint32_t no_sbpm_bn_ploam;
} bbh_rx_error_pm_counters;


/**************************************************************************************************/
/* inpkt: Incoming_packets - This counter counts the number of incoming good packets.             */
/* crc_err: PM_counter_value - PM counter value.                                                  */
/* too_short: PM_counter_value - PM counter value.                                                */
/* too_long: PM_counter_value - PM counter value.                                                 */
/* no_sbpm_sbn: PM_counter_value - PM counter value.                                              */
/* disp_cong: PM_counter_value - PM counter value.                                                */
/* no_sdma_cd: PM_counter_value - PM counter value.                                               */
/* ploam_no_sdma_cd: PM_counter_value - PM counter value.                                         */
/* ploam_disp_cong: PM_counter_value - PM counter value.                                          */
/**************************************************************************************************/
typedef struct
{
    uint32_t inpkt;
    uint32_t crc_err;
    uint32_t too_short;
    uint32_t too_long;
    uint32_t no_sbpm_sbn;
    uint32_t disp_cong;
    uint32_t no_sdma_cd;
    uint32_t ploam_no_sdma_cd;
    uint32_t ploam_disp_cong;
} bbh_rx_pm_counters;


/**************************************************************************************************/
/* inbufrst: Input_buf_reset_command - Writing 1 to this register will reset the input buffer.For */
/*            a reset operation the SW should assert and then de-assert this bit.                 */
/* burstbufrst: Burst_buf_reset_command - Writing 1 to this register will reset the Burst buffer. */
/*              For a reset operation the SW should assert and then de-assert this bit.           */
/* ingresscntxt: Ingress_context_reset_command - Writing 1 to this register will reset the ingres */
/*               s context.For a reset operation the SW should assert and then de-assert this bit */
/*               .                                                                                */
/* cmdfiforst: CMD_FIFO_reset_command - Writing 1 to this register will reset the IH buffer enabl */
/*             e.For a reset operation the SW should assert and then de-assert this bit.          */
/* sbpmfiforst: SBPM_FIFO_reset_command - Writing 1 to this register will reset the SBPM FIFO.The */
/*               reset is done immediately. Reading this register will always return 0.           */
/* coherencyfiforst: Coherency_FIFO_reset_command - Writing 1 to this register will reset the coh */
/*                   erency FIFO.For a reset operation the SW should assert and then de-assert th */
/*                   is bit.                                                                      */
/* cntxtrst: Context_reset_command - Writing 1 to this register will reset the reassembly context */
/*            table.The reset is done immediately. Reading this register will always return 0.    */
/* sdmarst: SDMA_write_pointer_reset_command - Writing 1 to this register will reset the SDMA wri */
/*          te pointer.For a reset operation the SW should assert and then de-assert this bit.    */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean inbufrst;
    bdmf_boolean burstbufrst;
    bdmf_boolean ingresscntxt;
    bdmf_boolean cmdfiforst;
    bdmf_boolean sbpmfiforst;
    bdmf_boolean coherencyfiforst;
    bdmf_boolean cntxtrst;
    bdmf_boolean sdmarst;
} bbh_rx_general_configuration_rxrstrst;


/**************************************************************************************************/
/* bypass_clk_gate: BYPASS_CLOCK_GATE - If set to 1b1 will disable the clock gate logic such to a */
/*                  lways enable the clock                                                        */
/* timer_val: TIMER_VALUE - For how long should the clock stay active once all conditions for clo */
/*            ck disable are met.                                                                 */
/* keep_alive_en: KEEP_ALIVE_ENABLE - Enables the keep alive logic which will periodically enable */
/*                 the clock to assure that no deadlock of clock being removed completely will oc */
/*                cur                                                                             */
/* keep_alive_intrvl: KEEP_ALIVE_INTERVAL - If the KEEP alive option is enabled the field will de */
/*                    termine for how many cycles should the clock be active                      */
/* keep_alive_cyc: KEEP_ALIVE_CYCLE - If the KEEP alive option is enabled this field will determi */
/*                 ne for how many cycles should the clock be disabled (minus the KEEP_ALIVE_INTE */
/*                 RVAL)So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.              */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} bbh_rx_general_configuration_clk_gate_cntrl;


/**************************************************************************************************/
/* inreass: In_reassembly - In reassembly.Not relevant for Ethernet.                              */
/* sop: SOP - SOP                                                                                 */
/* priority: Priority - Priority                                                                  */
/* flowid: Flow_ID - Flow ID                                                                      */
/* curoffset: Current_offset - Current offset                                                     */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean inreass;
    bdmf_boolean sop;
    uint8_t priority;
    uint8_t flowid;
    uint16_t curoffset;
} bbh_rx_debug_cntxtx0ingress;


/**************************************************************************************************/
/* inreass: In_reassembly - In reassembly.Not relevant for Ethernet.                              */
/* sop: SOP - SOP                                                                                 */
/* priority: Priority - Priority                                                                  */
/* flowid: Flow_ID - Flow ID                                                                      */
/* curoffset: Current_offset - Current offset                                                     */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean inreass;
    bdmf_boolean sop;
    uint8_t priority;
    uint8_t flowid;
    uint16_t curoffset;
} bbh_rx_debug_cntxtx1ingress;


/**************************************************************************************************/
/* bnentry: BN_entry - BN                                                                         */
/* valid: VALID - SBN is Valid                                                                    */
/**************************************************************************************************/
typedef struct
{
    uint32_t sbn_fifo[8];
} bbh_rx_debug_sbnfifo;


/**************************************************************************************************/
/* cmdentry: CMD_entry - CMD                                                                      */
/**************************************************************************************************/
typedef struct
{
    uint32_t cmd_fifo[4];
} bbh_rx_debug_cmdfifo;


/**************************************************************************************************/
/* bnentry: BN_entry - BN                                                                         */
/* valid: VALID - SBN is Valid                                                                    */
/**************************************************************************************************/
typedef struct
{
    uint32_t sbn_recycle_fifo[2];
} bbh_rx_debug_sbnrecyclefifo;

bdmf_error_t ag_drv_bbh_rx_ploam_en_set(uint8_t bbh_id, bdmf_boolean ploamen);
bdmf_error_t ag_drv_bbh_rx_ploam_en_get(uint8_t bbh_id, bdmf_boolean *ploamen);
bdmf_error_t ag_drv_bbh_rx_user_priority3_en_set(uint8_t bbh_id, bdmf_boolean pri3en);
bdmf_error_t ag_drv_bbh_rx_user_priority3_en_get(uint8_t bbh_id, bdmf_boolean *pri3en);
bdmf_error_t ag_drv_bbh_rx_pause_en_set(uint8_t bbh_id, bdmf_boolean pauseen);
bdmf_error_t ag_drv_bbh_rx_pause_en_get(uint8_t bbh_id, bdmf_boolean *pauseen);
bdmf_error_t ag_drv_bbh_rx_pfc_en_set(uint8_t bbh_id, bdmf_boolean pfcen);
bdmf_error_t ag_drv_bbh_rx_pfc_en_get(uint8_t bbh_id, bdmf_boolean *pfcen);
bdmf_error_t ag_drv_bbh_rx_ctrl_en_set(uint8_t bbh_id, bdmf_boolean ctrlen);
bdmf_error_t ag_drv_bbh_rx_ctrl_en_get(uint8_t bbh_id, bdmf_boolean *ctrlen);
bdmf_error_t ag_drv_bbh_rx_multicast_en_set(uint8_t bbh_id, bdmf_boolean multen);
bdmf_error_t ag_drv_bbh_rx_multicast_en_get(uint8_t bbh_id, bdmf_boolean *multen);
bdmf_error_t ag_drv_bbh_rx_oam_en_set(uint8_t bbh_id, bdmf_boolean oamen);
bdmf_error_t ag_drv_bbh_rx_oam_en_get(uint8_t bbh_id, bdmf_boolean *oamen);
bdmf_error_t ag_drv_bbh_rx_pattern_en_set(uint8_t bbh_id, bdmf_boolean patternen);
bdmf_error_t ag_drv_bbh_rx_pattern_en_get(uint8_t bbh_id, bdmf_boolean *patternen);
bdmf_error_t ag_drv_bbh_rx_exc_en_set(uint8_t bbh_id, bdmf_boolean excen);
bdmf_error_t ag_drv_bbh_rx_exc_en_get(uint8_t bbh_id, bdmf_boolean *excen);
bdmf_error_t ag_drv_bbh_rx_disable_normal_check_set(uint8_t bbh_id, bdmf_boolean disnormalcheck);
bdmf_error_t ag_drv_bbh_rx_disable_normal_check_get(uint8_t bbh_id, bdmf_boolean *disnormalcheck);
bdmf_error_t ag_drv_bbh_rx_pattern_recog_set(uint8_t bbh_id, const bbh_rx_pattern_recog *pattern_recog);
bdmf_error_t ag_drv_bbh_rx_pattern_recog_get(uint8_t bbh_id, bbh_rx_pattern_recog *pattern_recog);
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_timer_set(uint8_t bbh_id, uint32_t timer);
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_timer_get(uint8_t bbh_id, uint32_t *timer);
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_force_set(uint8_t bbh_id, bdmf_boolean fcforce);
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_force_get(uint8_t bbh_id, bdmf_boolean *fcforce);
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_drops_config_set(uint8_t bbh_id, const bbh_rx_flow_ctrl_drops_config *flow_ctrl_drops_config);
bdmf_error_t ag_drv_bbh_rx_flow_ctrl_drops_config_get(uint8_t bbh_id, bbh_rx_flow_ctrl_drops_config *flow_ctrl_drops_config);
bdmf_error_t ag_drv_bbh_rx_sdma_bb_id_set(uint8_t bbh_id, uint8_t sdmabbid);
bdmf_error_t ag_drv_bbh_rx_sdma_bb_id_get(uint8_t bbh_id, uint8_t *sdmabbid);
bdmf_error_t ag_drv_bbh_rx_dispatcher_sbpm_bb_id_set(uint8_t bbh_id, uint8_t dispbbid, uint8_t sbpmbbid);
bdmf_error_t ag_drv_bbh_rx_dispatcher_sbpm_bb_id_get(uint8_t bbh_id, uint8_t *dispbbid, uint8_t *sbpmbbid);
bdmf_error_t ag_drv_bbh_rx_dispatcher_virtual_queues_set(uint8_t bbh_id, uint8_t normalviq, uint8_t exclviq);
bdmf_error_t ag_drv_bbh_rx_dispatcher_virtual_queues_get(uint8_t bbh_id, uint8_t *normalviq, uint8_t *exclviq);
bdmf_error_t ag_drv_bbh_rx_sdma_config_set(uint8_t bbh_id, const bbh_rx_sdma_config *sdma_config);
bdmf_error_t ag_drv_bbh_rx_sdma_config_get(uint8_t bbh_id, bbh_rx_sdma_config *sdma_config);
bdmf_error_t ag_drv_bbh_rx_pkt_size0_set(uint8_t bbh_id, uint8_t minpkt0, uint16_t maxpkt0);
bdmf_error_t ag_drv_bbh_rx_pkt_size0_get(uint8_t bbh_id, uint8_t *minpkt0, uint16_t *maxpkt0);
bdmf_error_t ag_drv_bbh_rx_pkt_size1_set(uint8_t bbh_id, uint8_t minpkt1, uint16_t maxpkt1);
bdmf_error_t ag_drv_bbh_rx_pkt_size1_get(uint8_t bbh_id, uint8_t *minpkt1, uint16_t *maxpkt1);
bdmf_error_t ag_drv_bbh_rx_pkt_size2_set(uint8_t bbh_id, uint8_t minpkt2, uint16_t maxpkt2);
bdmf_error_t ag_drv_bbh_rx_pkt_size2_get(uint8_t bbh_id, uint8_t *minpkt2, uint16_t *maxpkt2);
bdmf_error_t ag_drv_bbh_rx_pkt_size3_set(uint8_t bbh_id, uint8_t minpkt3, uint16_t maxpkt3);
bdmf_error_t ag_drv_bbh_rx_pkt_size3_get(uint8_t bbh_id, uint8_t *minpkt3, uint16_t *maxpkt3);
bdmf_error_t ag_drv_bbh_rx_pkt_sel_group_0_set(uint8_t bbh_id, uint8_t minpktsel0, uint8_t maxpktsel0);
bdmf_error_t ag_drv_bbh_rx_pkt_sel_group_0_get(uint8_t bbh_id, uint8_t *minpktsel0, uint8_t *maxpktsel0);
bdmf_error_t ag_drv_bbh_rx_pkt_sel_group_1_set(uint8_t bbh_id, uint8_t minpktsel1, uint8_t maxpktsel1);
bdmf_error_t ag_drv_bbh_rx_pkt_sel_group_1_get(uint8_t bbh_id, uint8_t *minpktsel1, uint8_t *maxpktsel1);
bdmf_error_t ag_drv_bbh_rx_error_pm_counters_get(uint8_t bbh_id, bbh_rx_error_pm_counters *error_pm_counters);
bdmf_error_t ag_drv_bbh_rx_pm_counters_get(uint8_t bbh_id, bbh_rx_pm_counters *pm_counters);
bdmf_error_t ag_drv_bbh_rx_mac_mode_set(uint8_t bbh_id, bdmf_boolean macmode, bdmf_boolean gponmode);
bdmf_error_t ag_drv_bbh_rx_mac_mode_get(uint8_t bbh_id, bdmf_boolean *macmode, bdmf_boolean *gponmode);
bdmf_error_t ag_drv_bbh_rx_general_configuration_sopoffset_set(uint8_t bbh_id, uint8_t sopoffset);
bdmf_error_t ag_drv_bbh_rx_general_configuration_sopoffset_get(uint8_t bbh_id, uint8_t *sopoffset);
bdmf_error_t ag_drv_bbh_rx_general_configuration_crcomitdis_set(uint8_t bbh_id, bdmf_boolean crcomitdis);
bdmf_error_t ag_drv_bbh_rx_general_configuration_crcomitdis_get(uint8_t bbh_id, bdmf_boolean *crcomitdis);
bdmf_error_t ag_drv_bbh_rx_general_configuration_enable_set(uint8_t bbh_id, bdmf_boolean pkten, bdmf_boolean sbpmen);
bdmf_error_t ag_drv_bbh_rx_general_configuration_enable_get(uint8_t bbh_id, bdmf_boolean *pkten, bdmf_boolean *sbpmen);
bdmf_error_t ag_drv_bbh_rx_general_configuration_g9991en_set(uint8_t bbh_id, bdmf_boolean enable, bdmf_boolean bytes4_7enable);
bdmf_error_t ag_drv_bbh_rx_general_configuration_g9991en_get(uint8_t bbh_id, bdmf_boolean *enable, bdmf_boolean *bytes4_7enable);
bdmf_error_t ag_drv_bbh_rx_general_configuration_perflowth_set(uint8_t bbh_id, uint8_t flowth);
bdmf_error_t ag_drv_bbh_rx_general_configuration_perflowth_get(uint8_t bbh_id, uint8_t *flowth);
bdmf_error_t ag_drv_bbh_rx_min_pkt_sel_flows_0_15_set(uint8_t bbh_id, uint8_t flow_id, uint8_t sel_id);
bdmf_error_t ag_drv_bbh_rx_min_pkt_sel_flows_0_15_get(uint8_t bbh_id, uint8_t flow_id, uint8_t *sel_id);
bdmf_error_t ag_drv_bbh_rx_min_pkt_sel_flows_16_31_set(uint8_t bbh_id, uint8_t flow_id, uint8_t sel_id);
bdmf_error_t ag_drv_bbh_rx_min_pkt_sel_flows_16_31_get(uint8_t bbh_id, uint8_t flow_id, uint8_t *sel_id);
bdmf_error_t ag_drv_bbh_rx_max_pkt_sel_flows_0_15_set(uint8_t bbh_id, uint8_t flow_id, uint8_t sel_id);
bdmf_error_t ag_drv_bbh_rx_max_pkt_sel_flows_0_15_get(uint8_t bbh_id, uint8_t flow_id, uint8_t *sel_id);
bdmf_error_t ag_drv_bbh_rx_max_pkt_sel_flows_16_31_set(uint8_t bbh_id, uint8_t flow_id, uint8_t sel_id);
bdmf_error_t ag_drv_bbh_rx_max_pkt_sel_flows_16_31_get(uint8_t bbh_id, uint8_t flow_id, uint8_t *sel_id);
bdmf_error_t ag_drv_bbh_rx_general_configuration_sbpmcfg_set(uint8_t bbh_id, uint8_t max_otf_sbpm_req);
bdmf_error_t ag_drv_bbh_rx_general_configuration_sbpmcfg_get(uint8_t bbh_id, uint8_t *max_otf_sbpm_req);
bdmf_error_t ag_drv_bbh_rx_general_configuration_rxrstrst_set(uint8_t bbh_id, const bbh_rx_general_configuration_rxrstrst *general_configuration_rxrstrst);
bdmf_error_t ag_drv_bbh_rx_general_configuration_rxrstrst_get(uint8_t bbh_id, bbh_rx_general_configuration_rxrstrst *general_configuration_rxrstrst);
bdmf_error_t ag_drv_bbh_rx_general_configuration_rxdbgsel_set(uint8_t bbh_id, uint8_t rxdbgsel);
bdmf_error_t ag_drv_bbh_rx_general_configuration_rxdbgsel_get(uint8_t bbh_id, uint8_t *rxdbgsel);
bdmf_error_t ag_drv_bbh_rx_general_configuration_bbhrx_raddr_decoder_set(uint8_t bbh_id, uint8_t id_2overwr, uint16_t overwr_ra, bdmf_boolean overwr_en);
bdmf_error_t ag_drv_bbh_rx_general_configuration_bbhrx_raddr_decoder_get(uint8_t bbh_id, uint8_t *id_2overwr, uint16_t *overwr_ra, bdmf_boolean *overwr_en);
bdmf_error_t ag_drv_bbh_rx_general_configuration_noneth_set(uint8_t bbh_id, uint8_t flowid, bdmf_boolean enable);
bdmf_error_t ag_drv_bbh_rx_general_configuration_noneth_get(uint8_t bbh_id, uint8_t *flowid, bdmf_boolean *enable);
bdmf_error_t ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_set(uint8_t bbh_id, const bbh_rx_general_configuration_clk_gate_cntrl *general_configuration_clk_gate_cntrl);
bdmf_error_t ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_get(uint8_t bbh_id, bbh_rx_general_configuration_clk_gate_cntrl *general_configuration_clk_gate_cntrl);
bdmf_error_t ag_drv_bbh_rx_pm_counters_encrypterror_get(uint8_t bbh_id, uint32_t *encry_type_err);
bdmf_error_t ag_drv_bbh_rx_pm_counters_inploam_get(uint8_t bbh_id, uint32_t *inploam);
bdmf_error_t ag_drv_bbh_rx_pm_counters_epontyperror_get(uint8_t bbh_id, uint32_t *pmvalue);
bdmf_error_t ag_drv_bbh_rx_pm_counters_runterror_get(uint8_t bbh_id, uint16_t *pmvalue);
bdmf_error_t ag_drv_bbh_rx_debug_cntxtx0lsb_get(uint8_t bbh_id, bdmf_boolean *inreass, uint8_t *flowid, uint16_t *curoffset);
bdmf_error_t ag_drv_bbh_rx_debug_cntxtx0msb_get(uint8_t bbh_id, uint16_t *curbn, uint16_t *firstbn);
bdmf_error_t ag_drv_bbh_rx_debug_cntxtx1lsb_get(uint8_t bbh_id, bdmf_boolean *inreass, uint8_t *flowid, uint16_t *curoffset);
bdmf_error_t ag_drv_bbh_rx_debug_cntxtx1msb_get(uint8_t bbh_id, uint16_t *curbn, uint16_t *firstbn);
bdmf_error_t ag_drv_bbh_rx_debug_cntxtx0ingress_get(uint8_t bbh_id, bbh_rx_debug_cntxtx0ingress *debug_cntxtx0ingress);
bdmf_error_t ag_drv_bbh_rx_debug_cntxtx1ingress_get(uint8_t bbh_id, bbh_rx_debug_cntxtx1ingress *debug_cntxtx1ingress);
bdmf_error_t ag_drv_bbh_rx_debug_ibuw_get(uint8_t bbh_id, uint8_t *uw);
bdmf_error_t ag_drv_bbh_rx_debug_bbuw_get(uint8_t bbh_id, uint8_t *uw);
bdmf_error_t ag_drv_bbh_rx_debug_cfuw_get(uint8_t bbh_id, uint8_t *uw);
bdmf_error_t ag_drv_bbh_rx_debug_ackcnt_get(uint8_t bbh_id, uint8_t *sdma, uint8_t *connect);
bdmf_error_t ag_drv_bbh_rx_debug_coherencycnt_get(uint8_t bbh_id, uint8_t *normal, uint8_t *exclusive);
bdmf_error_t ag_drv_bbh_rx_debug_dbgvec_get(uint8_t bbh_id, uint32_t *dbgvec);
bdmf_error_t ag_drv_bbh_rx_debug_ufuw_get(uint8_t bbh_id, uint8_t *uw);
bdmf_error_t ag_drv_bbh_rx_debug_creditcnt_get(uint8_t bbh_id, uint8_t *normal, uint8_t *exclusive);
bdmf_error_t ag_drv_bbh_rx_debug_sdmacnt_get(uint8_t bbh_id, uint8_t *ucd);
bdmf_error_t ag_drv_bbh_rx_debug_cmfuw_get(uint8_t bbh_id, uint8_t *uw);
bdmf_error_t ag_drv_bbh_rx_debug_sbnfifo_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_sbnfifo *debug_sbnfifo);
bdmf_error_t ag_drv_bbh_rx_debug_cmdfifo_get(uint8_t bbh_id, uint32_t zero, bbh_rx_debug_cmdfifo *debug_cmdfifo);
bdmf_error_t ag_drv_bbh_rx_debug_sbnrecyclefifo_get(uint8_t bbh_id, uint8_t zero, bbh_rx_debug_sbnrecyclefifo *debug_sbnrecyclefifo);
bdmf_error_t ag_drv_bbh_rx_debug_coherencycnt2_get(uint8_t bbh_id, uint8_t *cdsent, uint8_t *ackreceived);
bdmf_error_t ag_drv_bbh_rx_debug_dropstatus_set(uint8_t bbh_id, bdmf_boolean dispstatus, bdmf_boolean sdmastatus);
bdmf_error_t ag_drv_bbh_rx_debug_dropstatus_get(uint8_t bbh_id, bdmf_boolean *dispstatus, bdmf_boolean *sdmastatus);

#ifdef USE_BDMF_SHELL
enum
{
    cli_bbh_rx_ploam_en,
    cli_bbh_rx_user_priority3_en,
    cli_bbh_rx_pause_en,
    cli_bbh_rx_pfc_en,
    cli_bbh_rx_ctrl_en,
    cli_bbh_rx_multicast_en,
    cli_bbh_rx_oam_en,
    cli_bbh_rx_pattern_en,
    cli_bbh_rx_exc_en,
    cli_bbh_rx_disable_normal_check,
    cli_bbh_rx_pattern_recog,
    cli_bbh_rx_flow_ctrl_timer,
    cli_bbh_rx_flow_ctrl_force,
    cli_bbh_rx_flow_ctrl_drops_config,
    cli_bbh_rx_sdma_bb_id,
    cli_bbh_rx_dispatcher_sbpm_bb_id,
    cli_bbh_rx_dispatcher_virtual_queues,
    cli_bbh_rx_sdma_config,
    cli_bbh_rx_pkt_size0,
    cli_bbh_rx_pkt_size1,
    cli_bbh_rx_pkt_size2,
    cli_bbh_rx_pkt_size3,
    cli_bbh_rx_pkt_sel_group_0,
    cli_bbh_rx_pkt_sel_group_1,
    cli_bbh_rx_error_pm_counters,
    cli_bbh_rx_pm_counters,
    cli_bbh_rx_mac_mode,
    cli_bbh_rx_general_configuration_sopoffset,
    cli_bbh_rx_general_configuration_crcomitdis,
    cli_bbh_rx_general_configuration_enable,
    cli_bbh_rx_general_configuration_g9991en,
    cli_bbh_rx_general_configuration_perflowth,
    cli_bbh_rx_min_pkt_sel_flows_0_15,
    cli_bbh_rx_min_pkt_sel_flows_16_31,
    cli_bbh_rx_max_pkt_sel_flows_0_15,
    cli_bbh_rx_max_pkt_sel_flows_16_31,
    cli_bbh_rx_general_configuration_sbpmcfg,
    cli_bbh_rx_general_configuration_rxrstrst,
    cli_bbh_rx_general_configuration_rxdbgsel,
    cli_bbh_rx_general_configuration_bbhrx_raddr_decoder,
    cli_bbh_rx_general_configuration_noneth,
    cli_bbh_rx_general_configuration_clk_gate_cntrl,
    cli_bbh_rx_pm_counters_encrypterror,
    cli_bbh_rx_pm_counters_inploam,
    cli_bbh_rx_pm_counters_epontyperror,
    cli_bbh_rx_pm_counters_runterror,
    cli_bbh_rx_debug_cntxtx0lsb,
    cli_bbh_rx_debug_cntxtx0msb,
    cli_bbh_rx_debug_cntxtx1lsb,
    cli_bbh_rx_debug_cntxtx1msb,
    cli_bbh_rx_debug_cntxtx0ingress,
    cli_bbh_rx_debug_cntxtx1ingress,
    cli_bbh_rx_debug_ibuw,
    cli_bbh_rx_debug_bbuw,
    cli_bbh_rx_debug_cfuw,
    cli_bbh_rx_debug_ackcnt,
    cli_bbh_rx_debug_coherencycnt,
    cli_bbh_rx_debug_dbgvec,
    cli_bbh_rx_debug_ufuw,
    cli_bbh_rx_debug_creditcnt,
    cli_bbh_rx_debug_sdmacnt,
    cli_bbh_rx_debug_cmfuw,
    cli_bbh_rx_debug_sbnfifo,
    cli_bbh_rx_debug_cmdfifo,
    cli_bbh_rx_debug_sbnrecyclefifo,
    cli_bbh_rx_debug_coherencycnt2,
    cli_bbh_rx_debug_dropstatus,
};

int bcm_bbh_rx_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_bbh_rx_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

