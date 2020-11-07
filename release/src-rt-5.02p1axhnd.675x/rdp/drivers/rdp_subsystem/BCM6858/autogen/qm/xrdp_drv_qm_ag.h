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

#ifndef _XRDP_DRV_QM_AG_H_
#define _XRDP_DRV_QM_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* ddr_byte_congestion_drop_enabl: DDR_BYTE_CONGESTION_DROP_ENABLE - This field indicates whether */
/*                                  crossing the DDR bytes thresholds (the number of bytes waitin */
/*                                 g to be copied to DDR) will result in dropping packets or in a */
/*                                 pplying back pressure to the re-order.0 - apply back pressure1 */
/*                                  - drop packets                                                */
/* ddr_bytes_lower_thr: DDR_BYTES_LOWER_THR - DDR copy bytes Lower Threshold.When working in pack */
/*                      et drop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:* If (DDR copy by */
/*                      tes counter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped.* If  */
/*                      (DDR_BYTES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_THR), */
/*                       then packets in low/high priority are dropped (only exclusive packets ar */
/*                      e not dropped).* If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counter) <= ( */
/*                      DDR_BYTES_MID_THR), then packets in low priority are dropped.* If (DDR co */
/*                      py bytes counter) <=  (DDR_BYTES_LOWER_THR), then no packets are dropped. */
/*                      When working in backpressure mode (DDR_BYTES_CONGESTION_DROP_ENABLE=0), T */
/*                      hen if (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then backpressu */
/*                      re is applied to re-order (in this case DDR_BYTES_LOWER_THR and DDR_BYTES */
/*                      _MID_THR are dont care).                                                  */
/* ddr_bytes_mid_thr: DDR_BYTES_MID_THR - DDR copy bytes Lower Threshold.When working in packet d */
/*                    rop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:* If (DDR copy bytes co */
/*                    unter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped.* If (DDR_BYT */
/*                    ES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_THR), then pack */
/*                    ets in low/high priority are dropped (only exclusive packets are not droppe */
/*                    d).* If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counter) <= (DDR_BYTES_MID_ */
/*                    THR), then packets in low priority are dropped.* If (DDR copy bytes counter */
/*                    ) <=  (DDR_BYTES_LOWER_THR), then no packets are dropped.When working in ba */
/*                    ckpressure mode (DDR_BYTES_CONGESTION_DROP_ENABLE=0), Then if (DDR copy byt */
/*                    es counter) > (DDR_BYTES_HIGHER_THR), then backpressure is applied to re-or */
/*                    der (in this case DDR_BYTES_LOWER_THR and DDR_BYTES_MID_THR are dont care). */
/* ddr_bytes_higher_thr: DDR_BYTES_HIGHER_THR - DDR copy bytes Lower Threshold.When working in pa */
/*                       cket drop mode (DDR_BYTES_CONGESTION_DROP_ENABLE=1), Then:* If (DDR copy */
/*                        bytes counter) > (DDR_BYTES_HIGHER_THR), then all packets are dropped.* */
/*                        If (DDR_BYTES_MID_THR) < (DDR copy bytes counter) <= (DDR_BYTES_HIGHER_ */
/*                       THR), then packets in low/high priority are dropped (only exclusive pack */
/*                       ets are not dropped).* If (DDR_BYTES_LOWER_THR) < (DDR copy bytes counte */
/*                       r) <= (DDR_BYTES_MID_THR), then packets in low priority are dropped.* If */
/*                        (DDR copy bytes counter) <=  (DDR_BYTES_LOWER_THR), then no packets are */
/*                        dropped.When working in backpressure mode (DDR_BYTES_CONGESTION_DROP_EN */
/*                       ABLE=0), Then if (DDR copy bytes counter) > (DDR_BYTES_HIGHER_THR), then */
/*                        backpressure is applied to re-order (in this case DDR_BYTES_LOWER_THR a */
/*                       nd DDR_BYTES_MID_THR are dont care).                                     */
/* ddr_pd_congestion_drop_enable: DDR_PD_CONGESTION_DROP_ENABLE - This field indicates whether cr */
/*                                ossing the DDR Pipe thresholds will result in dropping packets  */
/*                                or in applying back pressure to the re-order.0 - apply back pre */
/*                                ssure1 - drop packets                                           */
/* ddr_pipe_lower_thr: DDR_PIPE_LOWER_THR - DDR copy Pipe Lower Threshold.When working in packet  */
/*                     drop mode (DDR_PD_CONGESTION_DROP_ENABLE=1), Then:* If (DDR copy pipe occu */
/*                     pancy) > (DDR_PIPE_HIGHER_THR), then packets in low/high priority are drop */
/*                     ped (only exclusive packets are not dropped).* If (DDR_PIPE_LOWER_THR) < ( */
/*                     DDR copy pipe occupancy) <= (DDR_PIPE_HIGHER_THR), then packets in low pri */
/*                     ority are dropped.* If (DDR copy pipe occupancy) <=  (DDR_PIPE_LOWER_THR), */
/*                      then no packets are dropped.When working in backpressure mode (DDR_PD_CON */
/*                     GESTION_DROP_ENABLE=0), Then if (DDR copy pipe occupancy) > (DDR_PIPE_HIGH */
/*                     ER_THR), then backpressure is applied to re-order (in this case DDR_PIPE_L */
/*                     OWER_THR is dont care).                                                    */
/* ddr_pipe_higher_thr: DDR_PIPE_HIGHER_THR - DDR copy Pipe Lower Threshold.When working in packe */
/*                      t drop mode (DDR_PD_CONGESTION_DROP_ENABLE=1), Then:* If (DDR copy pipe o */
/*                      ccupancy) > (DDR_PIPE_HIGHER_THR), then packets in low/high priority are  */
/*                      dropped (only exclusive packets are not dropped).* If (DDR_PIPE_LOWER_THR */
/*                      ) < (DDR copy pipe occupancy) <= (DDR_PIPE_HIGHER_THR), then packets in l */
/*                      ow priority are dropped.* If (DDR copy pipe occupancy) <= (DDR_PIPE_LOWER */
/*                      _THR), then no packets are dropped.When working in backpressure mode (DDR */
/*                      _PD_CONGESTION_DROP_ENABLE=0), Then if (DDR copy pipe occupancy) > (DDR_P */
/*                      IPE_HIGHER_THR), then backpressure is applied to re-order (in this case D */
/*                      DR_PIPE_LOWER_THR is dont care).IMPORTANT: recommended maximum value is 0 */
/*                      x7B in order to avoid performance degradation when working with aggregati */
/*                      on timeout enable                                                         */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean ddr_byte_congestion_drop_enabl;
    uint32_t ddr_bytes_lower_thr;
    uint32_t ddr_bytes_mid_thr;
    uint32_t ddr_bytes_higher_thr;
    bdmf_boolean ddr_pd_congestion_drop_enable;
    uint8_t ddr_pipe_lower_thr;
    uint8_t ddr_pipe_higher_thr;
} qm_ddr_cong_ctrl;


/**************************************************************************************************/
/* lower_thr: FPM_GRP_LOWER_THR - FPM group Lower Threshold.* If (FPM User Group Counter) > (FPM_ */
/*            GRP_HIGHER_THR), all packets in this user group are dropped.* If (FPM_GRP_MID_THR)  */
/*            < (FPM User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high priori */
/*            ty are dropped (only exclusive packets are not dropped).* If (FPM_GRP_LOWER_THR) <  */
/*            (FPM User Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority are dro */
/*            pped.* If (FPM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets are drop */
/*            ped.                                                                                */
/* mid_thr: FPM_GRP_MID_THR - FPM group Lower Threshold.* If (FPM User Group Counter) > (FPM_GRP_ */
/*          HIGHER_THR), all packets in this user group are dropped.* If (FPM_GRP_MID_THR) < (FPM */
/*           User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high priority are d */
/*          ropped (only exclusive packets are not dropped).* If (FPM_GRP_LOWER_THR) < (FPM User  */
/*          Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority are dropped.* If (F */
/*          PM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets are dropped.           */
/* higher_thr: FPM_GRP_HIGHER_THR - FPM group Lower Threshold.* If (FPM User Group Counter) > (FP */
/*             M_GRP_HIGHER_THR), all packets in this user group are dropped.* If (FPM_GRP_MID_TH */
/*             R) < (FPM User Group Counter) <= (FPM_GRP_HIGHER_THR), then packets in low/high pr */
/*             iority are dropped (only exclusive packets are not dropped).* If (FPM_GRP_LOWER_TH */
/*             R) < (FPM User Group Counter) <= (FPM_GRP_MID_THR), then packets in low priority a */
/*             re dropped.* If (FPM User Group Counter) <= (FPM_GRP_LOWER_THR), then no packets a */
/*             re dropped.                                                                        */
/**************************************************************************************************/
typedef struct
{
    uint16_t lower_thr;
    uint16_t mid_thr;
    uint16_t higher_thr;
} qm_fpm_ug_thr;


/**************************************************************************************************/
/* start_queue: START_QUEUE - Indicates the Queue that starts this runner group. Queues belonging */
/*               to the runner group are defined by the following equation:START_QUEUE <= runner_ */
/*              queues <= END_QUEUE                                                               */
/* end_queue: END_QUEUE - Indicates the Queue that ends this runner group.Queues belonging to the */
/*             runner group are defined by the following equation:START_QUEUE <= runner_queues <= */
/*             END_QUEUE                                                                          */
/* pd_fifo_base: BASE_ADDR - PD FIFO Base Address. This is an 8-byte address (Byte_addr = BASE_AD */
/*               DR*8).                                                                           */
/* pd_fifo_size: SIZE - PD FIFO Size0 - 2 entries1 - 4 entries2 - 8 entries                       */
/* upd_fifo_base: BASE_ADDR - PD FIFO Base Address. This is an 8-byte address (Byte_addr = BASE_A */
/*                DDR*8).                                                                         */
/* upd_fifo_size: SIZE - PD FIFO Size0 - 8 entries1 - 16 entries2 - 32 entries3 - 64 entries4 - 1 */
/*                28 entries5 - 256 entries                                                       */
/* rnr_bb_id: RNR_BB_ID - Runner BB ID associated with this configuration.                        */
/* rnr_task: RNR_TASK - Runner Task number to be woken up when the update FIFO is written to.     */
/* rnr_enable: RNR_ENABLE - Enable this runner interface                                          */
/**************************************************************************************************/
typedef struct
{
    uint16_t start_queue;
    uint16_t end_queue;
    uint16_t pd_fifo_base;
    uint8_t pd_fifo_size;
    uint16_t upd_fifo_base;
    uint8_t upd_fifo_size;
    uint8_t rnr_bb_id;
    uint8_t rnr_task;
    bdmf_boolean rnr_enable;
} qm_rnr_group_cfg;


/**************************************************************************************************/
/* min_thr0: MIN_THR - WRED Color Min Threshold.This field represents the higher 24-bits of the q */
/*           ueue occupancy byte threshold.byte_threshold = THR*64.                               */
/* flw_ctrl_en0: FLW_CTRL_EN - 0 - flow control disable. regular WRED profile1 - flow control ena */
/*               ble. no WRED drop, wake up appropriate runner task when crossed.                 */
/* min_thr1: MIN_THR - WRED Color Min Threshold.This field represents the higher 24-bits of the q */
/*           ueue occupancy byte threshold.byte_threshold = THR*64.                               */
/* flw_ctrl_en1: FLW_CTRL_EN - 0 - flow control disable. regular WRED profile1 - flow control ena */
/*               ble. no WRED drop, wake up appropriate runner task when crossed.                 */
/* max_thr0: MAX_THR - WRED Color Max Threshold.This field represents the higher 24-bits of the q */
/*           ueue occupancy byte threshold.byte_threshold = THR*64.                               */
/* max_thr1: MAX_THR - WRED Color Max Threshold.This field represents the higher 24-bits of the q */
/*           ueue occupancy byte threshold.byte_threshold = THR*64.                               */
/* slope_mantissa0: SLOPE_MANTISSA - WRED Color slope mantissa.                                   */
/* slope_exp0: SLOPE_EXP - WRED Color slope exponent.                                             */
/* slope_mantissa1: SLOPE_MANTISSA - WRED Color slope mantissa.                                   */
/* slope_exp1: SLOPE_EXP - WRED Color slope exponent.                                             */
/**************************************************************************************************/
typedef struct
{
    uint32_t min_thr0;
    bdmf_boolean flw_ctrl_en0;
    uint32_t min_thr1;
    bdmf_boolean flw_ctrl_en1;
    uint32_t max_thr0;
    uint32_t max_thr1;
    uint8_t slope_mantissa0;
    uint8_t slope_exp0;
    uint8_t slope_mantissa1;
    uint8_t slope_exp1;
} qm_wred_profile_cfg;


/**************************************************************************************************/
/* vpb_base: Base - base                                                                          */
/* vpb_mask: Mask - mask                                                                          */
/* apb_base: Base - base                                                                          */
/* apb_mask: Mask - mask                                                                          */
/* dqm_base: Base - base                                                                          */
/* dqm_mask: Mask - mask                                                                          */
/**************************************************************************************************/
typedef struct
{
    uint32_t vpb_base;
    uint32_t vpb_mask;
    uint32_t apb_base;
    uint32_t apb_mask;
    uint32_t dqm_base;
    uint32_t dqm_mask;
} qm_ubus_slave;


/**************************************************************************************************/
/* fpmsrc: FPM_source_id - source id. This id is used to determine the route to the module.       */
/* sbpmsrc: SBPM_source_id - source id. This id is used to determine the route to the module.     */
/* stsrnrsrc: Status_Runner_source_id - source id. This id is used to determine the route to the  */
/*            Runner that is responsible for sending status messages (WAN only).                  */
/**************************************************************************************************/
typedef struct
{
    uint8_t fpmsrc;
    uint8_t sbpmsrc;
    uint8_t stsrnrsrc;
} qm_cfg_src_id;


/**************************************************************************************************/
/* dmasrc: DMA_source_id - source id. This id is used to determine the route to the module.       */
/* descbase: Descriptor_FIFO_base - Defines the base address of the read request FIFO within the  */
/*           DMA address space.The value should be identical to the relevant configuration in the */
/*            DMA.                                                                                */
/* descsize: Descriptor_FIFO_size - The size of the BBH read requests FIFO inside the DMA         */
/**************************************************************************************************/
typedef struct
{
    uint8_t dmasrc;
    uint8_t descbase;
    uint8_t descsize;
} qm_bbh_dma_cfg;


/**************************************************************************************************/
/* sdmasrc: SDMA_source_id - source id. This id is used to determine the route to the module.     */
/* descbase: Descriptor_FIFO_base - Defines the base address of the read request FIFO within the  */
/*           DMA address space.The value should be identical to the relevant configuration in the */
/*            DMA.                                                                                */
/* descsize: Descriptor_FIFO_size - The size of the BBH read requests FIFO inside the DMA         */
/**************************************************************************************************/
typedef struct
{
    uint8_t sdmasrc;
    uint8_t descbase;
    uint8_t descsize;
} qm_bbh_sdma_cfg;


/**************************************************************************************************/
/* bufsize: DDR_buffer_size - The data is arranged in the DDR in a fixed size buffers.            */
/* byteresul: PO_bytes_resulotion - The packet offset byte resulotion.                            */
/* ddrtxoffset: DDR_tx_offset - Static offset in 8-bytes resolution for non aggregated packets in */
/*               DDR                                                                              */
/* hnsize0: HN_size_0 - The size of the HN (Header number) in bytes. The BBH decides between size */
/*           0 and size 1 according to a bit in the PD                                            */
/* hnsize1: HN_size_1 - The size of the HN (Header number) in bytes. The BBH decides between size */
/*           0 and size 1 according to a bit in the PD                                            */
/**************************************************************************************************/
typedef struct
{
    uint8_t bufsize;
    bdmf_boolean byteresul;
    uint16_t ddrtxoffset;
    uint8_t hnsize0;
    uint8_t hnsize1;
} qm_bbh_ddr_cfg;


/**************************************************************************************************/
/* srampd: SRAM_PD - This counter counts the number of packets which were transmitted from the SR */
/*         AM.                                                                                    */
/* ddrpd: DDR_PD - This counter counts the number of packets which were transmitted from the DDR. */
/* pddrop: PD_DROP - This counter counts the number of PDs which were dropped due to PD FIFO full */
/*         .                                                                                      */
/* stscnt: STS_CNT - This counter counts the number of received status messages.                  */
/* stsdrop: STS_DROP - This counter counts the number of STS which were dropped due to PD FIFO fu */
/*          ll.                                                                                   */
/* msgcnt: MSG_CNT - This counter counts the number of received DBR/ghost messages.               */
/* msgdrop: MSG_DROP - This counter counts the number of MSG which were dropped due to PD FIFO fu */
/*          ll.                                                                                   */
/* getnextnull: Get_next_is_null - This counter counts the number Get next responses with a null  */
/*              BN.                                                                               */
/* lenerr: LEN_ERR - This counter counts the number of times a length error occuered              */
/* aggrlenerr: AGGR_LEN_ERR - This counter counts the number of times an aggregation length error */
/*              occuered                                                                          */
/* srampkt: SRAM_PKT - This counter counts the number of packets which were transmitted from the  */
/*          SRAM.                                                                                 */
/* ddrpkt: DDR_PKT - This counter counts the number of packets which were transmitted from the DD */
/*         R.                                                                                     */
/* flshpkts: FLSH_PKTS - This counter counts the number of flushed packets                        */
/**************************************************************************************************/
typedef struct
{
    uint32_t srampd;
    uint32_t ddrpd;
    uint16_t pddrop;
    uint32_t stscnt;
    uint16_t stsdrop;
    uint32_t msgcnt;
    uint16_t msgdrop;
    uint16_t getnextnull;
    uint16_t lenerr;
    uint16_t aggrlenerr;
    uint32_t srampkt;
    uint32_t ddrpkt;
    uint16_t flshpkts;
} qm_debug_counters;


/**************************************************************************************************/
/* nempty: not_empty_indications - indication of the queue state                                  */
/* urgnt: urgent - indication whether the queue is in urgent state or not                         */
/* sel_src: selected_source - the next peripheral to be served by the dma                         */
/* address: address - address within the ram                                                      */
/* datacs: data_ram_cs - chip select for write data ram                                           */
/* cdcs: cd_ram_cs - chip select for chunk descriptors ram                                        */
/* rrcs: rr_ram_cd - chip select for read requests ram                                            */
/* valid: valid - indirect read request is valid                                                  */
/* ready: ready - read data ready                                                                 */
/**************************************************************************************************/
typedef struct
{
    uint16_t nempty;
    uint16_t urgnt;
    uint8_t sel_src;
    uint16_t address;
    bdmf_boolean datacs;
    bdmf_boolean cdcs;
    bdmf_boolean rrcs;
    bdmf_boolean valid;
    bdmf_boolean ready;
} qm_debug_info;


/**************************************************************************************************/
/* fpm_prefetch_enable: FPM_PREFETCH_ENABLE - FPM Prefetch Enable. Setting this bit to 1 will sta */
/*                      rt filling up the FPM pool prefetch FIFOs.Seeting this bit to 0, will sto */
/*                      p FPM prefetches.                                                         */
/* reorder_credit_enable: REORDER_CREDIT_ENABLE - When this bit is set the QM will send credits t */
/*                        o the REORDER block.Disabling this bit will stop sending credits to the */
/*                         reorder.                                                               */
/* dqm_pop_enable: DQM_POP_ENABLE - When this bit is set the QM will pop PDs from the DQM and pla */
/*                 ce them in the runner SRAM                                                     */
/* rmt_fixed_arb_enable: RMT_FIXED_ARB_ENABLE - When this bit is set Fixed arbitration will be do */
/*                       ne in pops from the remote FIFOs (Non delayed highest priority). If this */
/*                        bit is cleared RR arbitration is done                                   */
/* dqm_push_fixed_arb_enable: DQM_PUSH_FIXED_ARB_ENABLE - When this bit is set Fixed arbitration  */
/*                            will be done in DQM pushes (CPU highest priority, then non-delayed  */
/*                            queues and then normal queues. If this bit is cleared RR arbitratio */
/*                            n is done.                                                          */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean fpm_prefetch_enable;
    bdmf_boolean reorder_credit_enable;
    bdmf_boolean dqm_pop_enable;
    bdmf_boolean rmt_fixed_arb_enable;
    bdmf_boolean dqm_push_fixed_arb_enable;
} qm_enable_ctrl;


/**************************************************************************************************/
/* fpm_prefetch0_sw_rst: FPM_PREFETCH0_SW_RST - FPM Prefetch FIFO0 SW reset.                      */
/* fpm_prefetch1_sw_rst: FPM_PREFETCH1_SW_RST - FPM Prefetch FIFO1 SW reset.                      */
/* fpm_prefetch2_sw_rst: FPM_PREFETCH2_SW_RST - FPM Prefetch FIFO2 SW reset.                      */
/* fpm_prefetch3_sw_rst: FPM_PREFETCH3_SW_RST - FPM Prefetch FIFO3 SW reset.                      */
/* normal_rmt_sw_rst: NORMAL_RMT_SW_RST - Normal Remote FIFO SW reset.                            */
/* non_delayed_rmt_sw_rst: NON_DELAYED_RMT_SW_RST - Non-delayed Remote FIFO SW reset.             */
/* pre_cm_fifo_sw_rst: PRE_CM_FIFO_SW_RST - Pre Copy Machine FIFO SW reset.                       */
/* cm_rd_pd_fifo_sw_rst: CM_RD_PD_FIFO_SW_RST - Copy Machine RD PD FIFO SW reset.                 */
/* cm_wr_pd_fifo_sw_rst: CM_WR_PD_FIFO_SW_RST - Pre Copy Machine FIFO SW reset.                   */
/* bb0_output_fifo_sw_rst: BB0_OUTPUT_FIFO_SW_RST - BB0 OUTPUT FIFO SW reset.                     */
/* bb1_output_fifo_sw_rst: BB1_OUTPUT_FIFO_SW_RST - BB1 Output FIFO SW reset.                     */
/* bb1_input_fifo_sw_rst: BB1_INPUT_FIFO_SW_RST - BB1 Input FIFO SW reset.                        */
/* tm_fifo_ptr_sw_rst: TM_FIFO_PTR_SW_RST - TM FIFOs Pointers SW reset.                           */
/* non_delayed_out_fifo_sw_rst: NON_DELAYED_OUT_FIFO_SW_RST - Non delayed output FIFO Pointers SW */
/*                               reset.                                                           */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean fpm_prefetch0_sw_rst;
    bdmf_boolean fpm_prefetch1_sw_rst;
    bdmf_boolean fpm_prefetch2_sw_rst;
    bdmf_boolean fpm_prefetch3_sw_rst;
    bdmf_boolean normal_rmt_sw_rst;
    bdmf_boolean non_delayed_rmt_sw_rst;
    bdmf_boolean pre_cm_fifo_sw_rst;
    bdmf_boolean cm_rd_pd_fifo_sw_rst;
    bdmf_boolean cm_wr_pd_fifo_sw_rst;
    bdmf_boolean bb0_output_fifo_sw_rst;
    bdmf_boolean bb1_output_fifo_sw_rst;
    bdmf_boolean bb1_input_fifo_sw_rst;
    bdmf_boolean tm_fifo_ptr_sw_rst;
    bdmf_boolean non_delayed_out_fifo_sw_rst;
} qm_reset_ctrl;


/**************************************************************************************************/
/* read_clear_pkts: DROP_CNT_PKTS_READ_CLEAR_ENABLE - Indicates whether the Drop/max_occupancy pa */
/*                  ckets counter is read clear.                                                  */
/* read_clear_bytes: DROP_CNT_BYTES_READ_CLEAR_ENABLE - Indicates whether the Drop/max_occupancy  */
/*                   bytes counter is read clear.                                                 */
/* disable_wrap_around_pkts: DROP_CNT_MAX_OCCUPANCY_PKTS_SELECT - This bit defines the functional */
/*                           ity of the drop packets counter.0 - Functions as the drop packets co */
/*                           unter1 - Functions as the max packets occupancy holder               */
/* disable_wrap_around_bytes: DROP_CNT_MAX_OCCUPANCY_BYTES_SELECT - This bit defines the function */
/*                            ality of the drop bytes counter.0 - Functions as the drop bytes cou */
/*                            nter1 - Functions as the max bytes occupancy holder                 */
/* free_with_context_last_search: FREE_WITH_CONTEXT_LAST_SEARCH - Indicates The value to put in t */
/*                                he last_search field of the SBPM free with context message      */
/* wred_disable: WRED_DISABLE - Disables WRED influence on drop condition                         */
/* ddr_pd_congestion_disable: DDR_PD_CONGESTION_DISABLE - Disables DDR_PD_CONGESTION influence on */
/*                             drop/bpcondition                                                   */
/* ddr_byte_congestion_disable: DDR_BYTE_CONGESTION_DISABLE - Disables DDR_BYTE_CONGESTION influe */
/*                              nce on drop/bp condition                                          */
/* ddr_occupancy_disable: DDR_OCCUPANCY_DISABLE - Disables DDR_OCCUPANCY influence on drop/bp con */
/*                        dition                                                                  */
/* ddr_fpm_congestion_disable: DDR_FPM_CONGESTION_DISABLE - Disables DDR_FPM_CONGESTION influence */
/*                              on drop/bp condition                                              */
/* fpm_ug_disable: FPM_UG_DISABLE - Disables FPM_UG influence on drop condition                   */
/* queue_occupancy_ddr_copy_decis: QUEUE_OCCUPANCY_DDR_COPY_DECISION_DISABLE - Disables QUEUE_OCC */
/*                                 UPANCY_DDR_COPY_DECISION influence on copy condition           */
/* psram_occupancy_ddr_copy_decis: PSRAM_OCCUPANCY_DDR_COPY_DECISION_DISABLE - Disables PSRAM_OCC */
/*                                 UPANCY_DDR_COPY_DECISION influence on copy condition           */
/* dont_send_mc_bit_to_bbh: DONT_SEND_MC_BIT_TO_BBH - When set, the multicast bit of the PD will  */
/*                          not be sent to BBH TX                                                 */
/* close_aggregation_on_timeout_d: CLOSE_AGGREGATION_ON_TIMEOUT_DISABLE - When set, aggregations  */
/*                                 are not closed automatically when queue open aggregation time  */
/*                                 expired.                                                       */
/* fpm_congestion_buf_release_mec: FPM_CONGESTION_BUF_RELEASE_MECHANISM_DISABLE - When cleared, g */
/*                                 iven that there is an FPM congestion situation and all prefetc */
/*                                 h FPM buffers are full then a min pool size buffer will be fre */
/*                                 ed each 1us. This is done due to the fact that exclusive indic */
/*                                 ation is received only togeter with buffer allocation reply an */
/*                                 d if this will not be done then a deadlock could occur.Setting */
/*                                  this bit will disable this mechanism.                         */
/* fpm_buffer_global_res_enable: FPM_BUFFER_GLOBAL_RES_ENABLE - FPM over subscription mechanism.E */
/*                               ach queue will have one out of 8 reserved byte threshold profile */
/*                               s. Each profile defines 8 bit threshold with 512byte resolution. */
/*                               Once the global FPM counter pass configurable threshold the syst */
/*                               em goes to buffer reservation congestion state. In this state an */
/*                               y PD entering a queue which passes the reserved byte threshold w */
/*                               ill be dropped.                                                  */
/* qm_preserve_pd_with_fpm: QM_PRESERVE_PD_WITH_FPM - Dont drop pd with fpm allocation.           */
/* qm_residue_per_queue: QM_RESIDUE_PER_QUEUE - 0 for 64B/Queue1 for 128B/Queue                   */
/* ghost_rpt_update_after_close_a: GHOST_RPT_UPDATE_AFTER_CLOSE_AGG_EN - Controls the timing of u */
/*                                 pdating the overhead counters with packets which goes through  */
/*                                 aggregation.0 - updates when the packets enters QM1 - updates  */
/*                                 when aggregation is done.                                      */
/* fpm_ug_flow_ctrl_disable: FPM_UG_FLOW_CTRL_DISABLE - Disables FPM_UG influence on flow control */
/*                            wake up messages to FW.                                             */
/* ddr_write_multi_slave_en: DDR_WRITE_MULTI_SLAVE_EN - Enables to write packet transaction to mu */
/*                           ltiple slave (unlimited), if disable only one ubus slave allowed.    */
/* ddr_pd_congestion_agg_priority: DDR_PD_CONGESTION_AGG_PRIORITY - global priority bit to aggreg */
/*                                 ated PDs which go through reprocessing.                        */
/* psram_occupancy_drop_disable: PSRAM_OCCUPANCY_DROP_DISABLE - Disables PSRAM_OCCUPANCY_DROP inf */
/*                               luence on drop condition                                         */
/* qm_ddr_write_alignment: QM_DDR_WRITE_ALIGNMENT - 0 According to length1 8-byte aligned         */
/* exclusive_dont_drop: EXCLUSIVE_DONT_DROP - Controls if the exclusive indication in PD marks th */
/*                      e PD as dont drop or as dont drop if the fpm in exclusive state1 - global */
/*                       dont drop0 - FPM exclusive state dont drop                               */
/* exclusive_dont_drop_bp_en: EXCLUSIVE_DONT_DROP_BP_EN - when set 1 backpressure will be applied */
/*                             when the DONT_DROP pd should be dropped.for example, 0 fpm buffers */
/*                             available and the PD should be copied to DDR.                      */
/* gpon_dbr_ceil: GPON_DBR_CEIL - If the bit enable, QM round up to 4 every packet length added g */
/*                host counters.                                                                  */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean read_clear_pkts;
    bdmf_boolean read_clear_bytes;
    bdmf_boolean disable_wrap_around_pkts;
    bdmf_boolean disable_wrap_around_bytes;
    bdmf_boolean free_with_context_last_search;
    bdmf_boolean wred_disable;
    bdmf_boolean ddr_pd_congestion_disable;
    bdmf_boolean ddr_byte_congestion_disable;
    bdmf_boolean ddr_occupancy_disable;
    bdmf_boolean ddr_fpm_congestion_disable;
    bdmf_boolean fpm_ug_disable;
    bdmf_boolean queue_occupancy_ddr_copy_decis;
    bdmf_boolean psram_occupancy_ddr_copy_decis;
    bdmf_boolean dont_send_mc_bit_to_bbh;
    bdmf_boolean close_aggregation_on_timeout_d;
    bdmf_boolean fpm_congestion_buf_release_mec;
    bdmf_boolean fpm_buffer_global_res_enable;
    bdmf_boolean qm_preserve_pd_with_fpm;
    bdmf_boolean qm_residue_per_queue;
    bdmf_boolean ghost_rpt_update_after_close_a;
    bdmf_boolean fpm_ug_flow_ctrl_disable;
    bdmf_boolean ddr_write_multi_slave_en;
    bdmf_boolean ddr_pd_congestion_agg_priority;
    bdmf_boolean psram_occupancy_drop_disable;
    bdmf_boolean qm_ddr_write_alignment;
    bdmf_boolean exclusive_dont_drop;
    bdmf_boolean exclusive_dont_drop_bp_en;
    bdmf_boolean gpon_dbr_ceil;
} qm_drop_counters_ctrl;


/**************************************************************************************************/
/* epon_line_rate: EPON_LINE_RATE - EPON Line Rate0 - 1G1 - 10G                                   */
/* epon_crc_add_disable: EPON_CRC_ADD_DISABLE - If this bit is not set then 4-bytes will be added */
/*                        to the ghost reporting accumulated bytes and to the byte overhead calcu */
/*                       lation input                                                             */
/* mac_flow_overwrite_crc_en: MAC_FLOW_OVERWRITE_CRC_EN - Enables to overwrite CRC addition speci */
/*                            fied MAC FLOW in the field below.                                   */
/* mac_flow_overwrite_crc: MAC_FLOW_OVERWRITE_CRC - MAC flow ID to force disable CRC addition     */
/* fec_ipg_length: FEC_IPG_LENGTH - FEC IPG Length                                                */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean epon_line_rate;
    bdmf_boolean epon_crc_add_disable;
    bdmf_boolean mac_flow_overwrite_crc_en;
    uint8_t mac_flow_overwrite_crc;
    uint16_t fec_ipg_length;
} qm_epon_overhead_ctrl;


/**************************************************************************************************/
/* lower_thr: FPM_LOWER_THR - FPM Lower Threshold.When working in packet drop mode (FPM_BP_ENABLE */
/*            =0), Then:* If (FPM pool occupancy) <= (FPM_LOWER_THR), then packets in low/high pr */
/*            iority are dropped (only exclusive packets are not dropped).* If (FPM_LOWER_THR) <  */
/*            (FPM pool occupancy) <= (FPM_HIGHER_THR), then packets in low priority are dropped. */
/*            * If (FPM pool occupancy) > (FPM_HIGHER_THR), then no packets are dropped.When work */
/*            ing in backpressure mode (FPM_BP_ENABLE=1), Then if (FPM pool occupancy) < (FPM_LOW */
/*            ER_THR), then backpressure is applied to re-order (in this case FPM_HIGHER_THR is d */
/*            ont care).                                                                          */
/* higher_thr: FPM_HIGHER_THR - FPM Higher Threshold.When working in packet drop mode (FPM_BP_ENA */
/*             BLE=0), Then:* If (FPM pool occupancy) <= (FPM_LOWER_THR), then packets in low/hig */
/*             h priority are dropped (only exclusive packets are not dropped).* If (FPM_LOWER_TH */
/*             R) < (FPM pool occupancy) <= (FPM_HIGHER_THR), then packets in low priority are dr */
/*             opped.* If (FPM pool occupancy) > (FPM_HIGHER_THR), then no packets are dropped.Wh */
/*             en working in backpressure mode (FPM_BP_ENABLE=1), Then if (FPM pool occupancy) <  */
/*             (FPM_LOWER_THR), then backpressure is applied to re-order (in this case FPM_HIGHER */
/*             _THR is dont care).                                                                */
/**************************************************************************************************/
typedef struct
{
    uint8_t lower_thr;
    uint8_t higher_thr;
} qm_fpm_pool_thr;


/**************************************************************************************************/
/* qm_dqm_pop_on_empty: QM_DQM_POP_ON_EMPTY - HW tried to pop a PD from the DQM of an empty queue */
/*                      .                                                                         */
/* qm_dqm_push_on_full: QM_DQM_PUSH_ON_FULL - HW tried to pop a PD into the DQM of a full queue.  */
/* qm_cpu_pop_on_empty: QM_CPU_POP_ON_EMPTY - CPU tried to pop a PD from the DQM of an empty queu */
/*                      e.                                                                        */
/* qm_cpu_push_on_full: QM_CPU_PUSH_ON_FULL - CPU tried to push a PD into the DQM of a full queue */
/*                      .                                                                         */
/* qm_normal_queue_pd_no_credit: QM_NORMAL_QUEUE_PD_NO_CREDIT - A PD arrived to the Normal queue  */
/*                               without having any credits                                       */
/* qm_non_delayed_queue_pd_no_cre: QM_NON_DELAYED_QUEUE_PD_NO_CREDIT - A PD arrived to the NON-de */
/*                                 layed queue without having any credits                         */
/* qm_non_valid_queue: QM_NON_VALID_QUEUE - A PD arrived with a non valid queue number (>287)     */
/* qm_agg_coherent_inconsistency: QM_AGG_COHERENT_INCONSISTENCY - An aggregation of PDs was done  */
/*                                in which the coherent bit of the PD differs between them (The c */
/*                                oherent bit of the first aggregated PD was used)                */
/* qm_force_copy_on_non_delayed: QM_FORCE_COPY_ON_NON_DELAYED - A PD with force copy bit set was  */
/*                               received on the non-delayed queue (in this queue the copy machin */
/*                               e is bypassed)                                                   */
/* qm_fpm_pool_size_nonexistent: QM_FPM_POOL_SIZE_NONEXISTENT - A PD was marked to be copied, but */
/*                                there does not exist an FPM pool buffer large enough to hold it */
/*                               .                                                                */
/* qm_target_mem_abs_contradictio: QM_TARGET_MEM_ABS_CONTRADICTION - A PD was marked with a targe */
/*                                 t_mem=1 (located in PSRAM) and on the other hand, the absolute */
/*                                  address indication was set.                                   */
/* qm_target_mem_force_copy_contr: QM_TARGET_MEM_FORCE_COPY_CONTRADICTION - A PD was marked with  */
/*                                 a target_mem=0 (located in DDR) and on the other hand, the for */
/*                                 ce copy indication was set.                                    */
/* qm_1588_multicast_contradictio: QM_1588_MULTICAST_CONTRADICTION - A PD was marked as a 1588 an */
/*                                 d multicast together.                                          */
/* qm_byte_drop_cnt_overrun: QM_BYTE_DROP_CNT_OVERRUN - The byte drop counter of one of the queue */
/*                           s reached its maximum value and a new value was pushed.              */
/* qm_pkt_drop_cnt_overrun: QM_PKT_DROP_CNT_OVERRUN - The Packet drop counter of one of the queue */
/*                          s reached its maximum value and a new value was pushed.               */
/* qm_total_byte_cnt_underrun: QM_TOTAL_BYTE_CNT_UNDERRUN - The Total byte counter was decremente */
/*                             d to a negative value.                                             */
/* qm_total_pkt_cnt_underrun: QM_TOTAL_PKT_CNT_UNDERRUN - The Total PD counter was decremented to */
/*                             a negative value.                                                  */
/* qm_fpm_ug0_underrun: QM_FPM_UG0_UNDERRUN - The UG0 counter was decremented to a negative value */
/*                      .                                                                         */
/* qm_fpm_ug1_underrun: QM_FPM_UG1_UNDERRUN - The UG1 counter was decremented to a negative value */
/*                      .                                                                         */
/* qm_fpm_ug2_underrun: QM_FPM_UG2_UNDERRUN - The UG2 counter was decremented to a negative value */
/*                      .                                                                         */
/* qm_fpm_ug3_underrun: QM_FPM_UG3_UNDERRUN - The UG3 counter was decremented to a negative value */
/*                      .                                                                         */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean qm_dqm_pop_on_empty;
    bdmf_boolean qm_dqm_push_on_full;
    bdmf_boolean qm_cpu_pop_on_empty;
    bdmf_boolean qm_cpu_push_on_full;
    bdmf_boolean qm_normal_queue_pd_no_credit;
    bdmf_boolean qm_non_delayed_queue_pd_no_cre;
    bdmf_boolean qm_non_valid_queue;
    bdmf_boolean qm_agg_coherent_inconsistency;
    bdmf_boolean qm_force_copy_on_non_delayed;
    bdmf_boolean qm_fpm_pool_size_nonexistent;
    bdmf_boolean qm_target_mem_abs_contradictio;
    bdmf_boolean qm_target_mem_force_copy_contr;
    bdmf_boolean qm_1588_multicast_contradictio;
    bdmf_boolean qm_byte_drop_cnt_overrun;
    bdmf_boolean qm_pkt_drop_cnt_overrun;
    bdmf_boolean qm_total_byte_cnt_underrun;
    bdmf_boolean qm_total_pkt_cnt_underrun;
    bdmf_boolean qm_fpm_ug0_underrun;
    bdmf_boolean qm_fpm_ug1_underrun;
    bdmf_boolean qm_fpm_ug2_underrun;
    bdmf_boolean qm_fpm_ug3_underrun;
} qm_intr_ctrl_isr;


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
} qm_clk_gate_clk_gate_cntrl;


/**************************************************************************************************/
/* wred_profile: WRED_PROFILE - Defines to which WRED Profile this queue belongs to.              */
/* copy_dec_profile: COPY_DEC_PROFILE - Defines to which Copy Decision Profile this queue belongs */
/*                    to.                                                                         */
/* copy_to_ddr: COPY_TO_DDR - Defines this queue to always copy to DDR.                           */
/* ddr_copy_disable: DDR_COPY_DISABLE - Defines this queue never to copy to DDR.                  */
/* aggregation_disable: AGGREGATION_DISABLE - Defines this queue never to aggregated PDs.         */
/* fpm_ug: FPM_UG - Defines to which FPM UG this queue belongs to.                                */
/* exclusive_priority: EXCLUSIVE_PRIORITY - Defines this queue with exclusive priority.           */
/* q_802_1ae: Q_802_1AE - Defines this queue as 802.1AE for EPON packet overhead calculations.    */
/* sci: SCI - Configures SCI for EPON packet overhead calculations.                               */
/* fec_enable: FEC_ENABLE - FEC enable configuration for EPON packet overhead calculations.       */
/* res_profile: RES_PROFILE - FPM reservation profile.Once the QM goes over global FPM reservatio */
/*              n threshold.Queue with more bytes the defined in the profile will be dropped.Prof */
/*              ile 0 means no drop due to FPM reservation for the queues with this profile.      */
/**************************************************************************************************/
typedef struct
{
    uint8_t wred_profile;
    uint8_t copy_dec_profile;
    bdmf_boolean copy_to_ddr;
    bdmf_boolean ddr_copy_disable;
    bdmf_boolean aggregation_disable;
    uint8_t fpm_ug;
    bdmf_boolean exclusive_priority;
    bdmf_boolean q_802_1ae;
    bdmf_boolean sci;
    bdmf_boolean fec_enable;
    uint8_t res_profile;
} qm_q_context;


/**************************************************************************************************/
/* ddrtmbase: DDR_TM_BASE - DDR TM base.The address is in bytes resolution.The address should be  */
/*            aligned to 128 bytes.                                                               */
/**************************************************************************************************/
typedef struct
{
    uint32_t addr[2];
} qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel;


/**************************************************************************************************/
/* ddrtmbase: DDR_TM_BASE - MSB of DDR TM base.                                                   */
/**************************************************************************************************/
typedef struct
{
    uint32_t addr[2];
} qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh;


/**************************************************************************************************/
/* cntxtrst: Context_reset - Writing 1 to this register will reset the segmentation context table */
/*           .The reset is done immediately. Reading this register will always return 0.          */
/* pdfiforst: PDs_FIFOs_reset - Writing 1 to this register will reset the PDs FIFOs.The reset is  */
/*            done immediately. Reading this register will always return 0.                       */
/* dmaptrrst: DMA_write_pointer_reset - Writing 1 to this register will reset the DMA write point */
/*            er.The reset is done immediately. Reading this register will always return 0.       */
/* sdmaptrrst: SDMA_write_pointer_reset - Writing 1 to this register will reset the SDMA write po */
/*             inter.The reset is done immediately. Reading this register will always return 0.Th */
/*             is register is relevalt only for Ethernet.                                         */
/* bpmfiforst: BPM_FIFO_reset - Writing 1 to this register will reset the BPM FIFO.The reset is d */
/*             one immediately. Reading this register will always return 0.                       */
/* sbpmfiforst: SBPM_FIFO_reset - Writing 1 to this register will reset the SBPM FIFO.The reset i */
/*              s done immediately. Reading this register will always return 0.This register is r */
/*              elevalt only for Ethernet.                                                        */
/* okfiforst: Order_Keeper_FIFO_reset - Writing 1 to this register will reset the order keeper FI */
/*            FO.The reset is done immediately. Reading this register will always return 0.This r */
/*            egister is relevalt only for Ethernet.                                              */
/* ddrfiforst: DDR_FIFO_reset - Writing 1 to this register will reset the DDR data FIFO.The reset */
/*              is done immediately. Reading this register will always return 0.This register is  */
/*             relevalt only for Ethernet.                                                        */
/* sramfiforst: SRAM_FIFO_reset - Writing 1 to this register will reset the SRAM data FIFO.The re */
/*              set is done immediately. Reading this register will always return 0.This register */
/*               is relevalt only for Ethernet.                                                   */
/* skbptrrst: SKB_PTR_reset - Writing 1 to this register will reset the SKB pointers.The reset is */
/*             done immediately. Reading this register will always return 0.                      */
/* stsfiforst: STS_FIFOs_reset - Writing 1 to this register will reset the EPON status FIFOs (per */
/*              queue 32 fifos).The reset is done immediately. Reading this register will always  */
/*             return 0.                                                                          */
/* reqfiforst: REQ_FIFO_reset - Writing 1 to this register will reset the EPON request FIFO (8 en */
/*             tries FIFO that holds the packet requests from the EPON MAC).The reset is done imm */
/*             ediately. Reading this register will always return 0.                              */
/* msgfiforst: MSG_FIFO_reset - Writing 1 to this register will reset the EPON/GPON MSG FIFOThe r */
/*             eset is done immediately. Reading this register will always return 0.              */
/* gnxtfiforst: GET_NXT_FIFO_reset - Writing 1 to this register will reset the GET NEXT FIFOsThe  */
/*              reset is done immediately. Reading this register will always return 0.            */
/* fbnfiforst: FIRST_BN_FIFO_reset - Writing 1 to this register will reset the FIRST BN FIFOsThe  */
/*             reset is done immediately. Reading this register will always return 0.             */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean cntxtrst;
    bdmf_boolean pdfiforst;
    bdmf_boolean dmaptrrst;
    bdmf_boolean sdmaptrrst;
    bdmf_boolean bpmfiforst;
    bdmf_boolean sbpmfiforst;
    bdmf_boolean okfiforst;
    bdmf_boolean ddrfiforst;
    bdmf_boolean sramfiforst;
    bdmf_boolean skbptrrst;
    bdmf_boolean stsfiforst;
    bdmf_boolean reqfiforst;
    bdmf_boolean msgfiforst;
    bdmf_boolean gnxtfiforst;
    bdmf_boolean fbnfiforst;
} qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd;


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
} qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl;


/**************************************************************************************************/
/* pdsel: pd_array_sel - rd from the PD FIFO                                                      */
/* pdvsel: pd_valid_array_sel - rd from the PD valid array                                        */
/* pdemptysel: pd_empty_array_sel - rd from the PD empty array                                    */
/* pdfullsel: pd_full_array_sel - rd from the PD Full array                                       */
/* pdbemptysel: pd_below_empty_array_sel - rd from the PD beliow empty array                      */
/* pdffwkpsel: pd_full_for_wakeup_array_sel - rd from the PD full for wakeup empty array          */
/* fbnsel: first_BN_array_sel - rd from the first BN array                                        */
/* fbnvsel: first_BN_valid_array_sel - rd from the first BN valid array                           */
/* fbnemptysel: first_BN_empty_array_sel - rd from the first BN empty array                       */
/* fbnfullsel: first_BN_full_array_sel - rd from the first BN full array                          */
/* getnextsel: get_next_array_sel - rd from the first Get Next array                              */
/* getnextvsel: get_next_valid_array_sel - rd from the get_next valid array                       */
/* getnextemptysel: get_next_empty_array_sel - rd from the get next empty array                   */
/* getnextfullsel: get_next_full_array_sel - rd from the get next full array                      */
/* gpncntxtsel: gpon_context_array_sel - rd from the gpon context array                           */
/* bpmsel: BPM_FIFO_sel - rd from the BPM FIFO                                                    */
/* bpmfsel: BPM_FLUSH_FIFO_sel - rd from the BPM FLUSH FIFO                                       */
/* sbpmsel: SBPM_FIFO_sel - rd from the SBPM FIFO                                                 */
/* sbpmfsel: SBPM_FLUSH_FIFO_sel - rd from the SBPM FLUSH FIFO                                    */
/* stssel: sts_array_sel - rd from the STS FIFO                                                   */
/* stsvsel: sts_valid_array_sel - rd from the STS valid array                                     */
/* stsemptysel: sts_empty_array_sel - rd from the STS empty array                                 */
/* stsfullsel: sts_full_array_sel - rd from the STS Full array                                    */
/* stsbemptysel: sts_below_empty_array_sel - rd from the STS beliow empty array                   */
/* stsffwkpsel: sts_full_for_wakeup_array_sel - rd from the STS full for wakeup empty array       */
/* msgsel: msg_array_sel - rd from the MSG FIFO                                                   */
/* msgvsel: msg_valid_array_sel - rd from the msg valid array                                     */
/* epnreqsel: epon_request_FIFO_sel - rd from the epon request FIFO                               */
/* datasel: DATA_FIFO_sel - rd from the DATA FIFO (SRAM and DDR)                                  */
/* reordersel: reorder_FIFO_sel - rd from the reorder FIFO                                        */
/* tsinfosel: Timestamp_info_FIFO_sel - rd from the Timestamp Info FIFO                           */
/* mactxsel: MAC_TX_FIFO_sel - rd from the MAC TX FIFO.                                           */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean pdsel;
    bdmf_boolean pdvsel;
    bdmf_boolean pdemptysel;
    bdmf_boolean pdfullsel;
    bdmf_boolean pdbemptysel;
    bdmf_boolean pdffwkpsel;
    bdmf_boolean fbnsel;
    bdmf_boolean fbnvsel;
    bdmf_boolean fbnemptysel;
    bdmf_boolean fbnfullsel;
    bdmf_boolean getnextsel;
    bdmf_boolean getnextvsel;
    bdmf_boolean getnextemptysel;
    bdmf_boolean getnextfullsel;
    bdmf_boolean gpncntxtsel;
    bdmf_boolean bpmsel;
    bdmf_boolean bpmfsel;
    bdmf_boolean sbpmsel;
    bdmf_boolean sbpmfsel;
    bdmf_boolean stssel;
    bdmf_boolean stsvsel;
    bdmf_boolean stsemptysel;
    bdmf_boolean stsfullsel;
    bdmf_boolean stsbemptysel;
    bdmf_boolean stsffwkpsel;
    bdmf_boolean msgsel;
    bdmf_boolean msgvsel;
    bdmf_boolean epnreqsel;
    bdmf_boolean datasel;
    bdmf_boolean reordersel;
    bdmf_boolean tsinfosel;
    bdmf_boolean mactxsel;
} qm_bbh_tx_qm_bbhtx_debug_counters_swrden;


/**************************************************************************************************/
/* dbgvec: Debug_vector - Selected debug vector.                                                  */
/**************************************************************************************************/
typedef struct
{
    uint32_t debug_out_reg[8];
} qm_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg;


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
} qm_dma_qm_dma_config_clk_gate_cntrl;

bdmf_error_t ag_drv_qm_ddr_cong_ctrl_set(const qm_ddr_cong_ctrl *ddr_cong_ctrl);
bdmf_error_t ag_drv_qm_ddr_cong_ctrl_get(qm_ddr_cong_ctrl *ddr_cong_ctrl);
bdmf_error_t ag_drv_qm_is_queue_not_empty_get(uint16_t q_idx, bdmf_boolean *data);
bdmf_error_t ag_drv_qm_is_queue_pop_ready_get(uint16_t q_idx, bdmf_boolean *data);
bdmf_error_t ag_drv_qm_is_queue_full_get(uint16_t q_idx, bdmf_boolean *data);
bdmf_error_t ag_drv_qm_fpm_ug_thr_set(uint8_t ug_grp_idx, const qm_fpm_ug_thr *fpm_ug_thr);
bdmf_error_t ag_drv_qm_fpm_ug_thr_get(uint8_t ug_grp_idx, qm_fpm_ug_thr *fpm_ug_thr);
bdmf_error_t ag_drv_qm_rnr_group_cfg_set(uint8_t rnr_idx, const qm_rnr_group_cfg *rnr_group_cfg);
bdmf_error_t ag_drv_qm_rnr_group_cfg_get(uint8_t rnr_idx, qm_rnr_group_cfg *rnr_group_cfg);
bdmf_error_t ag_drv_qm_cpu_pd_indirect_wr_data_set(uint8_t indirect_grp_idx, uint32_t data0, uint32_t data1, uint32_t data2, uint32_t data3);
bdmf_error_t ag_drv_qm_cpu_pd_indirect_wr_data_get(uint8_t indirect_grp_idx, uint32_t *data0, uint32_t *data1, uint32_t *data2, uint32_t *data3);
bdmf_error_t ag_drv_qm_cpu_pd_indirect_rd_data_get(uint8_t indirect_grp_idx, uint32_t *data0, uint32_t *data1, uint32_t *data2, uint32_t *data3);
bdmf_error_t ag_drv_qm_aggr_context_get(uint16_t idx, uint32_t *context_valid);
bdmf_error_t ag_drv_qm_wred_profile_cfg_set(uint8_t profile_idx, const qm_wred_profile_cfg *wred_profile_cfg);
bdmf_error_t ag_drv_qm_wred_profile_cfg_get(uint8_t profile_idx, qm_wred_profile_cfg *wred_profile_cfg);
bdmf_error_t ag_drv_qm_ubus_slave_set(const qm_ubus_slave *ubus_slave);
bdmf_error_t ag_drv_qm_ubus_slave_get(qm_ubus_slave *ubus_slave);
bdmf_error_t ag_drv_qm_cfg_src_id_set(const qm_cfg_src_id *cfg_src_id);
bdmf_error_t ag_drv_qm_cfg_src_id_get(qm_cfg_src_id *cfg_src_id);
bdmf_error_t ag_drv_qm_rnr_src_id_set(uint8_t pdrnr0src, uint8_t pdrnr1src);
bdmf_error_t ag_drv_qm_rnr_src_id_get(uint8_t *pdrnr0src, uint8_t *pdrnr1src);
bdmf_error_t ag_drv_qm_bbh_dma_cfg_set(const qm_bbh_dma_cfg *bbh_dma_cfg);
bdmf_error_t ag_drv_qm_bbh_dma_cfg_get(qm_bbh_dma_cfg *bbh_dma_cfg);
bdmf_error_t ag_drv_qm_dma_max_otf_read_request_set(uint8_t maxreq);
bdmf_error_t ag_drv_qm_dma_max_otf_read_request_get(uint8_t *maxreq);
bdmf_error_t ag_drv_qm_dma_epon_urgent_set(bdmf_boolean epnurgnt);
bdmf_error_t ag_drv_qm_dma_epon_urgent_get(bdmf_boolean *epnurgnt);
bdmf_error_t ag_drv_qm_bbh_sdma_cfg_set(const qm_bbh_sdma_cfg *bbh_sdma_cfg);
bdmf_error_t ag_drv_qm_bbh_sdma_cfg_get(qm_bbh_sdma_cfg *bbh_sdma_cfg);
bdmf_error_t ag_drv_qm_sdma_max_otf_read_request_set(uint8_t maxreq);
bdmf_error_t ag_drv_qm_sdma_max_otf_read_request_get(uint8_t *maxreq);
bdmf_error_t ag_drv_qm_sdma_epon_urgent_set(bdmf_boolean epnurgnt);
bdmf_error_t ag_drv_qm_sdma_epon_urgent_get(bdmf_boolean *epnurgnt);
bdmf_error_t ag_drv_qm_bbh_ddr_cfg_set(const qm_bbh_ddr_cfg *bbh_ddr_cfg);
bdmf_error_t ag_drv_qm_bbh_ddr_cfg_get(qm_bbh_ddr_cfg *bbh_ddr_cfg);
bdmf_error_t ag_drv_qm_debug_counters_get(qm_debug_counters *debug_counters);
bdmf_error_t ag_drv_qm_debug_info_set(const qm_debug_info *debug_info);
bdmf_error_t ag_drv_qm_debug_info_get(qm_debug_info *debug_info);
bdmf_error_t ag_drv_qm_enable_ctrl_set(const qm_enable_ctrl *enable_ctrl);
bdmf_error_t ag_drv_qm_enable_ctrl_get(qm_enable_ctrl *enable_ctrl);
bdmf_error_t ag_drv_qm_reset_ctrl_set(const qm_reset_ctrl *reset_ctrl);
bdmf_error_t ag_drv_qm_reset_ctrl_get(qm_reset_ctrl *reset_ctrl);
bdmf_error_t ag_drv_qm_drop_counters_ctrl_set(const qm_drop_counters_ctrl *drop_counters_ctrl);
bdmf_error_t ag_drv_qm_drop_counters_ctrl_get(qm_drop_counters_ctrl *drop_counters_ctrl);
bdmf_error_t ag_drv_qm_fpm_ctrl_set(bdmf_boolean fpm_pool_bp_enable, bdmf_boolean fpm_congestion_bp_enable, uint8_t fpm_prefetch_min_pool_size, uint8_t fpm_prefetch_pending_req_limit);
bdmf_error_t ag_drv_qm_fpm_ctrl_get(bdmf_boolean *fpm_pool_bp_enable, bdmf_boolean *fpm_congestion_bp_enable, uint8_t *fpm_prefetch_min_pool_size, uint8_t *fpm_prefetch_pending_req_limit);
bdmf_error_t ag_drv_qm_qm_pd_cong_ctrl_set(uint32_t total_pd_thr);
bdmf_error_t ag_drv_qm_qm_pd_cong_ctrl_get(uint32_t *total_pd_thr);
bdmf_error_t ag_drv_qm_global_cfg_abs_drop_queue_set(uint16_t abs_drop_queue, bdmf_boolean abs_drop_queue_en);
bdmf_error_t ag_drv_qm_global_cfg_abs_drop_queue_get(uint16_t *abs_drop_queue, bdmf_boolean *abs_drop_queue_en);
bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl_set(uint16_t max_agg_bytes, uint8_t max_agg_pkts);
bdmf_error_t ag_drv_qm_global_cfg_aggregation_ctrl_get(uint16_t *max_agg_bytes, uint8_t *max_agg_pkts);
bdmf_error_t ag_drv_qm_fpm_base_addr_set(uint32_t fpm_base_addr);
bdmf_error_t ag_drv_qm_fpm_base_addr_get(uint32_t *fpm_base_addr);
bdmf_error_t ag_drv_qm_global_cfg_fpm_coherent_base_addr_set(uint32_t fpm_base_addr);
bdmf_error_t ag_drv_qm_global_cfg_fpm_coherent_base_addr_get(uint32_t *fpm_base_addr);
bdmf_error_t ag_drv_qm_ddr_sop_offset_set(uint16_t ddr_sop_offset0, uint16_t ddr_sop_offset1);
bdmf_error_t ag_drv_qm_ddr_sop_offset_get(uint16_t *ddr_sop_offset0, uint16_t *ddr_sop_offset1);
bdmf_error_t ag_drv_qm_epon_overhead_ctrl_set(const qm_epon_overhead_ctrl *epon_overhead_ctrl);
bdmf_error_t ag_drv_qm_epon_overhead_ctrl_get(qm_epon_overhead_ctrl *epon_overhead_ctrl);
bdmf_error_t ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set(uint8_t prescaler_granularity, uint8_t aggregation_timeout_value);
bdmf_error_t ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_get(uint8_t *prescaler_granularity, uint8_t *aggregation_timeout_value);
bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_buffer_grp_res_set(uint32_t idx, uint8_t res_thr_0, uint8_t res_thr_1, uint8_t res_thr_2, uint8_t res_thr_3);
bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_buffer_grp_res_get(uint32_t idx, uint8_t *res_thr_0, uint8_t *res_thr_1, uint8_t *res_thr_2, uint8_t *res_thr_3);
bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_buffer_gbl_thr_set(uint16_t res_thr_global);
bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_buffer_gbl_thr_get(uint16_t *res_thr_global);
bdmf_error_t ag_drv_qm_global_cfg_qm_flow_ctrl_rnr_cfg_set(uint8_t rnr_bb_id, uint8_t rnr_task, bdmf_boolean rnr_enable);
bdmf_error_t ag_drv_qm_global_cfg_qm_flow_ctrl_rnr_cfg_get(uint8_t *rnr_bb_id, uint8_t *rnr_task, bdmf_boolean *rnr_enable);
bdmf_error_t ag_drv_qm_global_cfg_qm_flow_ctrl_intr_set(uint8_t qm_flow_ctrl_intr);
bdmf_error_t ag_drv_qm_global_cfg_qm_flow_ctrl_intr_get(uint8_t *qm_flow_ctrl_intr);
bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_set(uint32_t fpm_gbl_cnt);
bdmf_error_t ag_drv_qm_global_cfg_qm_fpm_ug_gbl_cnt_get(uint32_t *fpm_gbl_cnt);
bdmf_error_t ag_drv_qm_global_cfg_qm_egress_flush_queue_set(uint16_t queue_num, bdmf_boolean flush_en);
bdmf_error_t ag_drv_qm_global_cfg_qm_egress_flush_queue_get(uint16_t *queue_num, bdmf_boolean *flush_en);
bdmf_error_t ag_drv_qm_fpm_pool_thr_set(uint8_t pool_idx, const qm_fpm_pool_thr *fpm_pool_thr);
bdmf_error_t ag_drv_qm_fpm_pool_thr_get(uint8_t pool_idx, qm_fpm_pool_thr *fpm_pool_thr);
bdmf_error_t ag_drv_qm_fpm_ug_cnt_set(uint8_t grp_idx, uint16_t fpm_ug_cnt);
bdmf_error_t ag_drv_qm_fpm_ug_cnt_get(uint8_t grp_idx, uint16_t *fpm_ug_cnt);
bdmf_error_t ag_drv_qm_intr_ctrl_isr_set(const qm_intr_ctrl_isr *intr_ctrl_isr);
bdmf_error_t ag_drv_qm_intr_ctrl_isr_get(qm_intr_ctrl_isr *intr_ctrl_isr);
bdmf_error_t ag_drv_qm_intr_ctrl_ism_get(uint32_t *ism);
bdmf_error_t ag_drv_qm_intr_ctrl_ier_set(uint32_t iem);
bdmf_error_t ag_drv_qm_intr_ctrl_ier_get(uint32_t *iem);
bdmf_error_t ag_drv_qm_intr_ctrl_itr_set(uint32_t ist);
bdmf_error_t ag_drv_qm_clk_gate_clk_gate_cntrl_set(const qm_clk_gate_clk_gate_cntrl *clk_gate_clk_gate_cntrl);
bdmf_error_t ag_drv_qm_clk_gate_clk_gate_cntrl_get(qm_clk_gate_clk_gate_cntrl *clk_gate_clk_gate_cntrl);
bdmf_error_t ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_set(uint8_t indirect_grp_idx, uint16_t queue_num, uint8_t cmd, bdmf_boolean done, bdmf_boolean error);
bdmf_error_t ag_drv_qm_cpu_indr_port_cpu_pd_indirect_ctrl_get(uint8_t indirect_grp_idx, uint16_t *queue_num, uint8_t *cmd, bdmf_boolean *done, bdmf_boolean *error);
bdmf_error_t ag_drv_qm_q_context_set(uint16_t q_idx, const qm_q_context *q_context);
bdmf_error_t ag_drv_qm_q_context_get(uint16_t q_idx, qm_q_context *q_context);
bdmf_error_t ag_drv_qm_copy_decision_profile_set(uint8_t profile_idx, uint32_t queue_occupancy_thr, bdmf_boolean psram_thr);
bdmf_error_t ag_drv_qm_copy_decision_profile_get(uint8_t profile_idx, uint32_t *queue_occupancy_thr, bdmf_boolean *psram_thr);
bdmf_error_t ag_drv_qm_total_valid_cnt_set(uint16_t q_idx, uint32_t data);
bdmf_error_t ag_drv_qm_total_valid_cnt_get(uint16_t q_idx, uint32_t *data);
bdmf_error_t ag_drv_qm_dqm_valid_cnt_set(uint16_t q_idx, uint32_t data);
bdmf_error_t ag_drv_qm_dqm_valid_cnt_get(uint16_t q_idx, uint32_t *data);
bdmf_error_t ag_drv_qm_drop_counter_get(uint16_t q_idx, uint32_t *data);
bdmf_error_t ag_drv_qm_epon_q_byte_cnt_set(uint16_t q_idx, uint32_t data);
bdmf_error_t ag_drv_qm_epon_q_byte_cnt_get(uint16_t q_idx, uint32_t *data);
bdmf_error_t ag_drv_qm_epon_q_status_get(uint16_t q_idx, uint32_t *status_bit_vector);
bdmf_error_t ag_drv_qm_rd_data_pool0_get(uint32_t *data);
bdmf_error_t ag_drv_qm_rd_data_pool1_get(uint32_t *data);
bdmf_error_t ag_drv_qm_rd_data_pool2_get(uint32_t *data);
bdmf_error_t ag_drv_qm_rd_data_pool3_get(uint32_t *data);
bdmf_error_t ag_drv_qm_pdfifo_ptr_get(uint16_t q_idx, uint8_t *wr_ptr, uint8_t *rd_ptr);
bdmf_error_t ag_drv_qm_update_fifo_ptr_get(uint16_t fifo_idx, uint16_t *wr_ptr, uint8_t *rd_ptr);
bdmf_error_t ag_drv_qm_debug_sel_set(uint8_t select, bdmf_boolean enable);
bdmf_error_t ag_drv_qm_debug_sel_get(uint8_t *select, bdmf_boolean *enable);
bdmf_error_t ag_drv_qm_debug_bus_lsb_get(uint32_t *data);
bdmf_error_t ag_drv_qm_debug_bus_msb_get(uint32_t *data);
bdmf_error_t ag_drv_qm_qm_spare_config_get(uint32_t *data);
bdmf_error_t ag_drv_qm_good_lvl1_pkts_cnt_get(uint32_t *good_lvl1_pkts);
bdmf_error_t ag_drv_qm_good_lvl1_bytes_cnt_get(uint32_t *good_lvl1_bytes);
bdmf_error_t ag_drv_qm_good_lvl2_pkts_cnt_get(uint32_t *good_lvl2_pkts);
bdmf_error_t ag_drv_qm_good_lvl2_bytes_cnt_get(uint32_t *good_lvl2_bytes);
bdmf_error_t ag_drv_qm_copied_pkts_cnt_get(uint32_t *copied_pkts);
bdmf_error_t ag_drv_qm_copied_bytes_cnt_get(uint32_t *copied_bytes);
bdmf_error_t ag_drv_qm_agg_pkts_cnt_get(uint32_t *agg_pkts);
bdmf_error_t ag_drv_qm_agg_bytes_cnt_get(uint32_t *agg_bytes);
bdmf_error_t ag_drv_qm_agg_1_pkts_cnt_get(uint32_t *agg1_pkts);
bdmf_error_t ag_drv_qm_agg_2_pkts_cnt_get(uint32_t *agg2_pkts);
bdmf_error_t ag_drv_qm_agg_3_pkts_cnt_get(uint32_t *agg3_pkts);
bdmf_error_t ag_drv_qm_agg_4_pkts_cnt_get(uint32_t *agg4_pkts);
bdmf_error_t ag_drv_qm_wred_drop_cnt_get(uint32_t *wred_drop);
bdmf_error_t ag_drv_qm_fpm_congestion_drop_cnt_get(uint32_t *fpm_cong);
bdmf_error_t ag_drv_qm_ddr_pd_congestion_drop_cnt_get(uint32_t *ddr_pd_cong_drop);
bdmf_error_t ag_drv_qm_ddr_byte_congestion_drop_cnt_get(uint32_t *ddr_cong_byte_drop);
bdmf_error_t ag_drv_qm_qm_pd_congestion_drop_cnt_get(uint32_t *pd_cong_drop);
bdmf_error_t ag_drv_qm_qm_abs_requeue_cnt_get(uint32_t *abs_requeue);
bdmf_error_t ag_drv_qm_fpm_prefetch_fifo0_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_fpm_prefetch_fifo1_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_fpm_prefetch_fifo2_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_fpm_prefetch_fifo3_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_normal_rmt_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_non_delayed_rmt_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_non_delayed_out_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_pre_cm_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_cm_rd_pd_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_cm_wr_pd_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_cm_common_input_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_bb0_output_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_bb1_output_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_bb1_input_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_egress_data_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_egress_rr_fifo_status_get(uint16_t *used_words, bdmf_boolean *empty, bdmf_boolean *full);
bdmf_error_t ag_drv_qm_bb_route_ovr_set(uint8_t idx, bdmf_boolean ovr_en, uint8_t dest_id, uint16_t route_addr);
bdmf_error_t ag_drv_qm_bb_route_ovr_get(uint8_t idx, bdmf_boolean *ovr_en, uint8_t *dest_id, uint16_t *route_addr);
bdmf_error_t ag_drv_qm_ingress_stat_get(uint32_t *ingress_stat);
bdmf_error_t ag_drv_qm_egress_stat_get(uint32_t *egress_stat);
bdmf_error_t ag_drv_qm_cm_stat_get(uint32_t *cm_stat);
bdmf_error_t ag_drv_qm_fpm_prefetch_stat_get(uint32_t *fpm_prefetch_stat);
bdmf_error_t ag_drv_qm_qm_connect_ack_counter_get(uint8_t *connect_ack_counter);
bdmf_error_t ag_drv_qm_qm_ddr_wr_reply_counter_get(uint8_t *ddr_wr_reply_counter);
bdmf_error_t ag_drv_qm_qm_ddr_pipe_byte_counter_get(uint32_t *ddr_pipe);
bdmf_error_t ag_drv_qm_qm_abs_requeue_valid_counter_get(uint16_t *requeue_valid);
bdmf_error_t ag_drv_qm_qm_illegal_pd_capture_get(uint32_t idx, uint32_t *pd);
bdmf_error_t ag_drv_qm_qm_ingress_processed_pd_capture_get(uint32_t idx, uint32_t *pd);
bdmf_error_t ag_drv_qm_qm_cm_processed_pd_capture_get(uint32_t idx, uint32_t *pd);
bdmf_error_t ag_drv_qm_fpm_pool_drop_cnt_get(uint32_t idx, uint32_t *fpm_drop);
bdmf_error_t ag_drv_qm_fpm_grp_drop_cnt_get(uint32_t idx, uint32_t *fpm_grp_drop);
bdmf_error_t ag_drv_qm_fpm_buffer_res_drop_cnt_get(uint32_t *counter);
bdmf_error_t ag_drv_qm_psram_egress_cong_drp_cnt_get(uint32_t *counter);
bdmf_error_t ag_drv_qm_cm_residue_data_get(uint16_t idx, uint32_t *data);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_mactype_set(uint8_t type);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_mactype_get(uint8_t *type);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1_set(uint8_t rnr_cfg_index_1, uint16_t tcontaddr, uint16_t skbaddr);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1_get(uint8_t rnr_cfg_index_1, uint16_t *tcontaddr, uint16_t *skbaddr);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2_set(uint16_t rnr_cfg_index_2, uint16_t ptraddr, uint8_t task);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2_get(uint16_t rnr_cfg_index_2, uint16_t *ptraddr, uint8_t *task);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg_set(bdmf_boolean freenocntxt, bdmf_boolean specialfree, uint8_t maxgn);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg_get(bdmf_boolean *freenocntxt, bdmf_boolean *specialfree, uint8_t *maxgn);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel_set(uint8_t zero, const qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel *bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel_get(uint8_t zero, qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel *bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh_set(uint8_t zero, const qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh *bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh_get(uint8_t zero, qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh *bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl_set(uint16_t psramsize, uint16_t ddrsize, uint16_t psrambase);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl_get(uint16_t *psramsize, uint16_t *ddrsize, uint16_t *psrambase);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg_set(bdmf_boolean hightrxq);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg_get(bdmf_boolean *hightrxq);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute_set(uint16_t route, uint8_t dest, bdmf_boolean en);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute_get(uint16_t *route, uint8_t *dest, bdmf_boolean *en);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr_set(uint8_t q_2_rnr_index, bdmf_boolean q0, bdmf_boolean q1);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr_get(uint8_t q_2_rnr_index, bdmf_boolean *q0, bdmf_boolean *q1);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd_set(const qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd *bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd_get(qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd *bbh_tx_qm_bbhtx_common_configurations_txrstcmd);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel_set(uint8_t dbgsel);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel_get(uint8_t *dbgsel);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl_set(const qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl *bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl_get(qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl *bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase_set(uint16_t fifobase0, uint16_t fifobase1);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase_get(uint16_t *fifobase0, uint16_t *fifobase1);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize_set(uint16_t fifosize0, uint16_t fifosize1);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize_get(uint16_t *fifosize0, uint16_t *fifosize1);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph_set(uint8_t wkupthresh0, uint8_t wkupthresh1);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph_get(uint8_t *wkupthresh0, uint8_t *wkupthresh1);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_set(uint16_t pdlimit0, uint16_t pdlimit1);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_get(uint16_t *pdlimit0, uint16_t *pdlimit1);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en_set(bdmf_boolean pdlimiten);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en_get(bdmf_boolean *pdlimiten);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty_set(uint8_t empty);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty_get(uint8_t *empty);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh_set(uint16_t ddrthresh, uint16_t sramthresh);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh_get(uint16_t *ddrthresh, uint16_t *sramthresh);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_eee_set(bdmf_boolean en);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_eee_get(bdmf_boolean *en);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_ts_set(bdmf_boolean en);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_lan_configurations_ts_get(bdmf_boolean *en);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_srambyte_get(uint32_t *srambyte);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_ddrbyte_get(uint32_t *ddrbyte);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrden_set(const qm_bbh_tx_qm_bbhtx_debug_counters_swrden *bbh_tx_qm_bbhtx_debug_counters_swrden);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrden_get(qm_bbh_tx_qm_bbhtx_debug_counters_swrden *bbh_tx_qm_bbhtx_debug_counters_swrden);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr_set(uint16_t rdaddr);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr_get(uint16_t *rdaddr);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_swrddata_get(uint32_t *data);
bdmf_error_t ag_drv_qm_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg_get(uint8_t zero, qm_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg *bbh_tx_qm_bbhtx_debug_counters_dbgoutreg);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_bbrouteovrd_set(uint8_t dest, uint16_t route, bdmf_boolean ovrd);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_bbrouteovrd_get(uint8_t *dest, uint16_t *route, bdmf_boolean *ovrd);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_num_of_writes_set(uint8_t emac_index, uint8_t numofbuff);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_num_of_writes_get(uint8_t emac_index, uint8_t *numofbuff);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_num_of_reads_set(uint8_t emac_index, uint8_t rr_num);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_num_of_reads_get(uint8_t emac_index, uint8_t *rr_num);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_u_thresh_set(uint8_t emac_index, uint8_t into_u, uint8_t out_of_u);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_u_thresh_get(uint8_t emac_index, uint8_t *into_u, uint8_t *out_of_u);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_pri_set(uint8_t emac_index, uint8_t rxpri, uint8_t txpri);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_pri_get(uint8_t emac_index, uint8_t *rxpri, uint8_t *txpri);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_periph_source_set(uint8_t emac_index, uint8_t rxsource, uint8_t txsource);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_periph_source_get(uint8_t emac_index, uint8_t *rxsource, uint8_t *txsource);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_weight_set(uint8_t emac_index, uint8_t rxweight, uint8_t txweight);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_weight_get(uint8_t emac_index, uint8_t *rxweight, uint8_t *txweight);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_ptrrst_set(uint16_t rstvec);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_ptrrst_get(uint16_t *rstvec);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_max_otf_set(uint8_t max);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_max_otf_get(uint8_t *max);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_clk_gate_cntrl_set(const qm_dma_qm_dma_config_clk_gate_cntrl *dma_qm_dma_config_clk_gate_cntrl);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_clk_gate_cntrl_get(qm_dma_qm_dma_config_clk_gate_cntrl *dma_qm_dma_config_clk_gate_cntrl);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_dbg_sel_set(uint8_t dbgsel);
bdmf_error_t ag_drv_qm_dma_qm_dma_config_dbg_sel_get(uint8_t *dbgsel);
bdmf_error_t ag_drv_qm_dma_qm_dma_debug_req_cnt_rx_get(uint8_t emac_index, uint8_t *req_cnt);
bdmf_error_t ag_drv_qm_dma_qm_dma_debug_req_cnt_tx_get(uint8_t emac_index, uint8_t *req_cnt);
bdmf_error_t ag_drv_qm_dma_qm_dma_debug_req_cnt_rx_acc_get(uint8_t emac_index, uint32_t *req_cnt);
bdmf_error_t ag_drv_qm_dma_qm_dma_debug_req_cnt_tx_acc_get(uint8_t emac_index, uint32_t *req_cnt);
bdmf_error_t ag_drv_qm_dma_qm_dma_debug_rddata_get(uint8_t word_index, uint32_t *data);

#ifdef USE_BDMF_SHELL
enum
{
    cli_qm_ddr_cong_ctrl,
    cli_qm_is_queue_not_empty,
    cli_qm_is_queue_pop_ready,
    cli_qm_is_queue_full,
    cli_qm_fpm_ug_thr,
    cli_qm_rnr_group_cfg,
    cli_qm_cpu_pd_indirect_wr_data,
    cli_qm_cpu_pd_indirect_rd_data,
    cli_qm_aggr_context,
    cli_qm_wred_profile_cfg,
    cli_qm_ubus_slave,
    cli_qm_cfg_src_id,
    cli_qm_rnr_src_id,
    cli_qm_bbh_dma_cfg,
    cli_qm_dma_max_otf_read_request,
    cli_qm_dma_epon_urgent,
    cli_qm_bbh_sdma_cfg,
    cli_qm_sdma_max_otf_read_request,
    cli_qm_sdma_epon_urgent,
    cli_qm_bbh_ddr_cfg,
    cli_qm_debug_counters,
    cli_qm_debug_info,
    cli_qm_enable_ctrl,
    cli_qm_reset_ctrl,
    cli_qm_drop_counters_ctrl,
    cli_qm_fpm_ctrl,
    cli_qm_qm_pd_cong_ctrl,
    cli_qm_global_cfg_abs_drop_queue,
    cli_qm_global_cfg_aggregation_ctrl,
    cli_qm_fpm_base_addr,
    cli_qm_global_cfg_fpm_coherent_base_addr,
    cli_qm_ddr_sop_offset,
    cli_qm_epon_overhead_ctrl,
    cli_qm_global_cfg_qm_aggregation_timer_ctrl,
    cli_qm_global_cfg_qm_fpm_buffer_grp_res,
    cli_qm_global_cfg_qm_fpm_buffer_gbl_thr,
    cli_qm_global_cfg_qm_flow_ctrl_rnr_cfg,
    cli_qm_global_cfg_qm_flow_ctrl_intr,
    cli_qm_global_cfg_qm_fpm_ug_gbl_cnt,
    cli_qm_global_cfg_qm_egress_flush_queue,
    cli_qm_fpm_pool_thr,
    cli_qm_fpm_ug_cnt,
    cli_qm_intr_ctrl_isr,
    cli_qm_intr_ctrl_ism,
    cli_qm_intr_ctrl_ier,
    cli_qm_intr_ctrl_itr,
    cli_qm_clk_gate_clk_gate_cntrl,
    cli_qm_cpu_indr_port_cpu_pd_indirect_ctrl,
    cli_qm_q_context,
    cli_qm_copy_decision_profile,
    cli_qm_total_valid_cnt,
    cli_qm_dqm_valid_cnt,
    cli_qm_drop_counter,
    cli_qm_epon_q_byte_cnt,
    cli_qm_epon_q_status,
    cli_qm_rd_data_pool0,
    cli_qm_rd_data_pool1,
    cli_qm_rd_data_pool2,
    cli_qm_rd_data_pool3,
    cli_qm_pdfifo_ptr,
    cli_qm_update_fifo_ptr,
    cli_qm_debug_sel,
    cli_qm_debug_bus_lsb,
    cli_qm_debug_bus_msb,
    cli_qm_qm_spare_config,
    cli_qm_good_lvl1_pkts_cnt,
    cli_qm_good_lvl1_bytes_cnt,
    cli_qm_good_lvl2_pkts_cnt,
    cli_qm_good_lvl2_bytes_cnt,
    cli_qm_copied_pkts_cnt,
    cli_qm_copied_bytes_cnt,
    cli_qm_agg_pkts_cnt,
    cli_qm_agg_bytes_cnt,
    cli_qm_agg_1_pkts_cnt,
    cli_qm_agg_2_pkts_cnt,
    cli_qm_agg_3_pkts_cnt,
    cli_qm_agg_4_pkts_cnt,
    cli_qm_wred_drop_cnt,
    cli_qm_fpm_congestion_drop_cnt,
    cli_qm_ddr_pd_congestion_drop_cnt,
    cli_qm_ddr_byte_congestion_drop_cnt,
    cli_qm_qm_pd_congestion_drop_cnt,
    cli_qm_qm_abs_requeue_cnt,
    cli_qm_fpm_prefetch_fifo0_status,
    cli_qm_fpm_prefetch_fifo1_status,
    cli_qm_fpm_prefetch_fifo2_status,
    cli_qm_fpm_prefetch_fifo3_status,
    cli_qm_normal_rmt_fifo_status,
    cli_qm_non_delayed_rmt_fifo_status,
    cli_qm_non_delayed_out_fifo_status,
    cli_qm_pre_cm_fifo_status,
    cli_qm_cm_rd_pd_fifo_status,
    cli_qm_cm_wr_pd_fifo_status,
    cli_qm_cm_common_input_fifo_status,
    cli_qm_bb0_output_fifo_status,
    cli_qm_bb1_output_fifo_status,
    cli_qm_bb1_input_fifo_status,
    cli_qm_egress_data_fifo_status,
    cli_qm_egress_rr_fifo_status,
    cli_qm_bb_route_ovr,
    cli_qm_ingress_stat,
    cli_qm_egress_stat,
    cli_qm_cm_stat,
    cli_qm_fpm_prefetch_stat,
    cli_qm_qm_connect_ack_counter,
    cli_qm_qm_ddr_wr_reply_counter,
    cli_qm_qm_ddr_pipe_byte_counter,
    cli_qm_qm_abs_requeue_valid_counter,
    cli_qm_qm_illegal_pd_capture,
    cli_qm_qm_ingress_processed_pd_capture,
    cli_qm_qm_cm_processed_pd_capture,
    cli_qm_fpm_pool_drop_cnt,
    cli_qm_fpm_grp_drop_cnt,
    cli_qm_fpm_buffer_res_drop_cnt,
    cli_qm_psram_egress_cong_drp_cnt,
    cli_qm_cm_residue_data,
    cli_qm_bbh_tx_qm_bbhtx_common_configurations_mactype,
    cli_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_1,
    cli_qm_bbh_tx_qm_bbhtx_common_configurations_rnrcfg_2,
    cli_qm_bbh_tx_qm_bbhtx_common_configurations_sbpmcfg,
    cli_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbasel,
    cli_qm_bbh_tx_qm_bbhtx_common_configurations_ddrtmbaseh,
    cli_qm_bbh_tx_qm_bbhtx_common_configurations_dfifoctrl,
    cli_qm_bbh_tx_qm_bbhtx_common_configurations_arb_cfg,
    cli_qm_bbh_tx_qm_bbhtx_common_configurations_bbroute,
    cli_qm_bbh_tx_qm_bbhtx_common_configurations_q2rnr,
    cli_qm_bbh_tx_qm_bbhtx_common_configurations_txrstcmd,
    cli_qm_bbh_tx_qm_bbhtx_common_configurations_dbgsel,
    cli_qm_bbh_tx_qm_bbhtx_common_configurations_clk_gate_cntrl,
    cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdbase,
    cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdsize,
    cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdwkuph,
    cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th,
    cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pd_byte_th_en,
    cli_qm_bbh_tx_qm_bbhtx_lan_configurations_pdempty,
    cli_qm_bbh_tx_qm_bbhtx_lan_configurations_txthresh,
    cli_qm_bbh_tx_qm_bbhtx_lan_configurations_eee,
    cli_qm_bbh_tx_qm_bbhtx_lan_configurations_ts,
    cli_qm_bbh_tx_qm_bbhtx_debug_counters_srambyte,
    cli_qm_bbh_tx_qm_bbhtx_debug_counters_ddrbyte,
    cli_qm_bbh_tx_qm_bbhtx_debug_counters_swrden,
    cli_qm_bbh_tx_qm_bbhtx_debug_counters_swrdaddr,
    cli_qm_bbh_tx_qm_bbhtx_debug_counters_swrddata,
    cli_qm_bbh_tx_qm_bbhtx_debug_counters_dbgoutreg,
    cli_qm_dma_qm_dma_config_bbrouteovrd,
    cli_qm_dma_qm_dma_config_num_of_writes,
    cli_qm_dma_qm_dma_config_num_of_reads,
    cli_qm_dma_qm_dma_config_u_thresh,
    cli_qm_dma_qm_dma_config_pri,
    cli_qm_dma_qm_dma_config_periph_source,
    cli_qm_dma_qm_dma_config_weight,
    cli_qm_dma_qm_dma_config_ptrrst,
    cli_qm_dma_qm_dma_config_max_otf,
    cli_qm_dma_qm_dma_config_clk_gate_cntrl,
    cli_qm_dma_qm_dma_config_dbg_sel,
    cli_qm_dma_qm_dma_debug_req_cnt_rx,
    cli_qm_dma_qm_dma_debug_req_cnt_tx,
    cli_qm_dma_qm_dma_debug_req_cnt_rx_acc,
    cli_qm_dma_qm_dma_debug_req_cnt_tx_acc,
    cli_qm_dma_qm_dma_debug_rddata,
};

int bcm_qm_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_qm_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

