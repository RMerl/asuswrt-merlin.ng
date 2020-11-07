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

#ifndef _XRDP_DRV_BBH_TX_AG_H_
#define _XRDP_DRV_BBH_TX_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* fpmsrc: FPM_source_id - source id. This id is used to determine the route to the module.       */
/* sbpmsrc: SBPM_source_id - source id. This id is used to determine the route to the module.     */
/* stsrnrsrc: Status_Runner_source_id - source id. This id is used to determine the route to the  */
/*            Runner that is responsible for sending status messages (WAN only).                  */
/* msgrnrsrc: Message_Runner_source_id - source id. This id is used to determine the route to the */
/*             Runner which is responsible for sending DBR/Ghost messages (WAN only).             */
/**************************************************************************************************/
typedef struct
{
    uint8_t fpmsrc;
    uint8_t sbpmsrc;
    uint8_t stsrnrsrc;
    uint8_t msgrnrsrc;
} bbh_tx_cfg_src_id;


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
} bbh_tx_bbh_dma_cfg;


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
} bbh_tx_bbh_sdma_cfg;


/**************************************************************************************************/
/* bufsize: DDR_buffer_size - The data is arranged in the DDR in a fixed size buffers.            */
/* byteresul: PO_bytes_resulotion - The packet offset byte resulotion.                            */
/* ddrtxoffset: DDR_tx_offset - Static offset in 8-bytes resolution for non aggregated packets in */
/*               DDR                                                                              */
/* hnsize0: HN_size_0 - The size of the HN (Header number) in bytes. The BBH decides between size */
/*           0 and size 1 according to a bit in the PD.In multicast, several headers are located  */
/*          in the same DDR buffer. the offset of header N within a buffer is (N-1)*64bytes, rega */
/*          rdless of the HN actual size.                                                         */
/* hnsize1: HN_size_1 - The size of the HN (Header number) in bytes. The BBH decides between size */
/*           0 and size 1 according to a bit in the PD.In multicast, several headers are located  */
/*          in the same DDR buffer. the offset of header N within a buffer is (N-1)*64bytes, rega */
/*          rdless of the HN actual size.                                                         */
/**************************************************************************************************/
typedef struct
{
    uint8_t bufsize;
    bdmf_boolean byteresul;
    uint16_t ddrtxoffset;
    uint8_t hnsize0;
    uint8_t hnsize1;
} bbh_tx_bbh_ddr_cfg;


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
} bbh_tx_debug_counters;


/**************************************************************************************************/
/* ddrtmbase: DDR_TM_BASE - DDR TM base.The address is in bytes resolution.The address should be  */
/*            aligned to 128 bytes.                                                               */
/**************************************************************************************************/
typedef struct
{
    uint32_t addr[2];
} bbh_tx_common_configurations_ddrtmbasel;


/**************************************************************************************************/
/* ddrtmbase: DDR_TM_BASE - MSB of DDR TM base.                                                   */
/**************************************************************************************************/
typedef struct
{
    uint32_t addr[2];
} bbh_tx_common_configurations_ddrtmbaseh;


/**************************************************************************************************/
/* task0: task_0 - task number for queue 0                                                        */
/* task1: task_1 - task number for queue 1                                                        */
/* task2: task_2 - task number for queue 2                                                        */
/* task3: task_3 - task number for queue 3                                                        */
/* task4: task_4 - task number for queue 4                                                        */
/* task5: task_5 - task number for queue 5                                                        */
/* task6: task_6 - task number for queue 6                                                        */
/* task7: task_7 - task number for queue 7                                                        */
/**************************************************************************************************/
typedef struct
{
    uint8_t task0;
    uint8_t task1;
    uint8_t task2;
    uint8_t task3;
    uint8_t task4;
    uint8_t task5;
    uint8_t task6;
    uint8_t task7;
} bbh_tx_common_configurations_perqtask;


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
} bbh_tx_common_configurations_txrstcmd;


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
} bbh_tx_common_configurations_clk_gate_cntrl;


/**************************************************************************************************/
/* stplenerr: Stop_on_len_error - In case of fatal length error - a mismatch between the request  */
/*            message from MAC and its relevant PD from Runner - the BBH can stop performing or c */
/*            ontinue regardless of the error.The error is also reflected to the SW in a counter. */
/* cmp_width: comp_width - configures the width of the comparison of the packet ength.The length  */
/*            field in the EPON request interface is 11 bit, while it is 14 bit in the pd.If this */
/*             bit is 0, then the comparison of the length will be between the 11 bit of the inte */
/*            rface and the 11 lsb bits of the pd.If this ibt is 1, the comparison will be done b */
/*            etween the 11 bits of the interface, concatenated with 3 zeros and the 14 bits of t */
/*            he pd                                                                               */
/* considerfull: xepon_consider_sts_full - determines whether the BBH will consider the sts_full  */
/*               vector state when pushing STS messages to the MAC or not.The status fifos inside */
/*                the MAC should never go full as they are mirror of the BBH PD FIFOs, but in cas */
/*               es where the MAC design behaves different than expected, we want the BBH to be a */
/*               ble to operate as in 1G EPON mode                                                */
/* addcrc: add_crc_bytes_to_len - configuration whether to add 4 bytes per packet to the length r */
/*         eceived in the status message from the Runner so the MAC would know the actual length  */
/*         to be transmitted.                                                                     */
/* req_full: req_fifo_depth - defines the depth of the req fifo.Physically there are 8 entries in */
/*            the FIFO, but this configuration changes the full indication towards the EPON MAC s */
/*           o the FIFO depth is reduced effectively.                                             */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean stplenerr;
    bdmf_boolean cmp_width;
    bdmf_boolean considerfull;
    bdmf_boolean addcrc;
    uint8_t req_full;
} bbh_tx_wan_configurations_epncfg;


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
} bbh_tx_debug_counters_swrden;


/**************************************************************************************************/
/* dbgvec: Debug_vector - Selected debug vector.                                                  */
/**************************************************************************************************/
typedef struct
{
    uint32_t debug_out_reg[8];
} bbh_tx_debug_counters_dbgoutreg;

bdmf_error_t ag_drv_bbh_tx_mac_type_set(uint8_t bbh_id_tx, uint8_t type);
bdmf_error_t ag_drv_bbh_tx_mac_type_get(uint8_t bbh_id_tx, uint8_t *type);
bdmf_error_t ag_drv_bbh_tx_cfg_src_id_set(uint8_t bbh_id_tx, const bbh_tx_cfg_src_id *cfg_src_id);
bdmf_error_t ag_drv_bbh_tx_cfg_src_id_get(uint8_t bbh_id_tx, bbh_tx_cfg_src_id *cfg_src_id);
bdmf_error_t ag_drv_bbh_tx_rnr_src_id_set(uint8_t bbh_id_tx, uint8_t pdrnr0src, uint8_t pdrnr1src);
bdmf_error_t ag_drv_bbh_tx_rnr_src_id_get(uint8_t bbh_id_tx, uint8_t *pdrnr0src, uint8_t *pdrnr1src);
bdmf_error_t ag_drv_bbh_tx_bbh_dma_cfg_set(uint8_t bbh_id_tx, const bbh_tx_bbh_dma_cfg *bbh_dma_cfg);
bdmf_error_t ag_drv_bbh_tx_bbh_dma_cfg_get(uint8_t bbh_id_tx, bbh_tx_bbh_dma_cfg *bbh_dma_cfg);
bdmf_error_t ag_drv_bbh_tx_dma_max_otf_read_request_set(uint8_t bbh_id_tx, uint8_t maxreq);
bdmf_error_t ag_drv_bbh_tx_dma_max_otf_read_request_get(uint8_t bbh_id_tx, uint8_t *maxreq);
bdmf_error_t ag_drv_bbh_tx_dma_epon_urgent_set(uint8_t bbh_id_tx, bdmf_boolean epnurgnt);
bdmf_error_t ag_drv_bbh_tx_dma_epon_urgent_get(uint8_t bbh_id_tx, bdmf_boolean *epnurgnt);
bdmf_error_t ag_drv_bbh_tx_bbh_sdma_cfg_set(uint8_t bbh_id_tx, const bbh_tx_bbh_sdma_cfg *bbh_sdma_cfg);
bdmf_error_t ag_drv_bbh_tx_bbh_sdma_cfg_get(uint8_t bbh_id_tx, bbh_tx_bbh_sdma_cfg *bbh_sdma_cfg);
bdmf_error_t ag_drv_bbh_tx_sdma_max_otf_read_request_set(uint8_t bbh_id_tx, uint8_t maxreq);
bdmf_error_t ag_drv_bbh_tx_sdma_max_otf_read_request_get(uint8_t bbh_id_tx, uint8_t *maxreq);
bdmf_error_t ag_drv_bbh_tx_sdma_epon_urgent_set(uint8_t bbh_id_tx, bdmf_boolean epnurgnt);
bdmf_error_t ag_drv_bbh_tx_sdma_epon_urgent_get(uint8_t bbh_id_tx, bdmf_boolean *epnurgnt);
bdmf_error_t ag_drv_bbh_tx_bbh_ddr_cfg_set(uint8_t bbh_id_tx, const bbh_tx_bbh_ddr_cfg *bbh_ddr_cfg);
bdmf_error_t ag_drv_bbh_tx_bbh_ddr_cfg_get(uint8_t bbh_id_tx, bbh_tx_bbh_ddr_cfg *bbh_ddr_cfg);
bdmf_error_t ag_drv_bbh_tx_debug_counters_get(uint8_t bbh_id_tx, bbh_tx_debug_counters *debug_counters);
bdmf_error_t ag_drv_bbh_tx_common_configurations_rnrcfg_1_set(uint8_t bbh_id_tx, uint8_t rnr_cfg_index_1, uint16_t tcontaddr, uint16_t skbaddr);
bdmf_error_t ag_drv_bbh_tx_common_configurations_rnrcfg_1_get(uint8_t bbh_id_tx, uint8_t rnr_cfg_index_1, uint16_t *tcontaddr, uint16_t *skbaddr);
bdmf_error_t ag_drv_bbh_tx_common_configurations_rnrcfg_2_set(uint8_t bbh_id_tx, uint16_t rnr_cfg_index_2, uint16_t ptraddr, uint8_t task);
bdmf_error_t ag_drv_bbh_tx_common_configurations_rnrcfg_2_get(uint8_t bbh_id_tx, uint16_t rnr_cfg_index_2, uint16_t *ptraddr, uint8_t *task);
bdmf_error_t ag_drv_bbh_tx_common_configurations_sbpmcfg_set(uint8_t bbh_id_tx, bdmf_boolean freenocntxt, bdmf_boolean specialfree, uint8_t maxgn);
bdmf_error_t ag_drv_bbh_tx_common_configurations_sbpmcfg_get(uint8_t bbh_id_tx, bdmf_boolean *freenocntxt, bdmf_boolean *specialfree, uint8_t *maxgn);
bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrtmbasel_set(uint8_t bbh_id_tx, uint8_t zero, const bbh_tx_common_configurations_ddrtmbasel *common_configurations_ddrtmbasel);
bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrtmbasel_get(uint8_t bbh_id_tx, uint8_t zero, bbh_tx_common_configurations_ddrtmbasel *common_configurations_ddrtmbasel);
bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrtmbaseh_set(uint8_t bbh_id_tx, uint8_t zero, const bbh_tx_common_configurations_ddrtmbaseh *common_configurations_ddrtmbaseh);
bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrtmbaseh_get(uint8_t bbh_id_tx, uint8_t zero, bbh_tx_common_configurations_ddrtmbaseh *common_configurations_ddrtmbaseh);
bdmf_error_t ag_drv_bbh_tx_common_configurations_dfifoctrl_set(uint8_t bbh_id_tx, uint16_t psramsize, uint16_t ddrsize, uint16_t psrambase);
bdmf_error_t ag_drv_bbh_tx_common_configurations_dfifoctrl_get(uint8_t bbh_id_tx, uint16_t *psramsize, uint16_t *ddrsize, uint16_t *psrambase);
bdmf_error_t ag_drv_bbh_tx_common_configurations_arb_cfg_set(uint8_t bbh_id_tx, bdmf_boolean hightrxq);
bdmf_error_t ag_drv_bbh_tx_common_configurations_arb_cfg_get(uint8_t bbh_id_tx, bdmf_boolean *hightrxq);
bdmf_error_t ag_drv_bbh_tx_common_configurations_bbroute_set(uint8_t bbh_id_tx, uint16_t route, uint8_t dest, bdmf_boolean en);
bdmf_error_t ag_drv_bbh_tx_common_configurations_bbroute_get(uint8_t bbh_id_tx, uint16_t *route, uint8_t *dest, bdmf_boolean *en);
bdmf_error_t ag_drv_bbh_tx_common_configurations_perqtask_set(uint8_t bbh_id_tx, const bbh_tx_common_configurations_perqtask *common_configurations_perqtask);
bdmf_error_t ag_drv_bbh_tx_common_configurations_perqtask_get(uint8_t bbh_id_tx, bbh_tx_common_configurations_perqtask *common_configurations_perqtask);
bdmf_error_t ag_drv_bbh_tx_common_configurations_txrstcmd_set(uint8_t bbh_id_tx, const bbh_tx_common_configurations_txrstcmd *common_configurations_txrstcmd);
bdmf_error_t ag_drv_bbh_tx_common_configurations_txrstcmd_get(uint8_t bbh_id_tx, bbh_tx_common_configurations_txrstcmd *common_configurations_txrstcmd);
bdmf_error_t ag_drv_bbh_tx_common_configurations_dbgsel_set(uint8_t bbh_id_tx, uint8_t dbgsel);
bdmf_error_t ag_drv_bbh_tx_common_configurations_dbgsel_get(uint8_t bbh_id_tx, uint8_t *dbgsel);
bdmf_error_t ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_set(uint8_t bbh_id_tx, const bbh_tx_common_configurations_clk_gate_cntrl *common_configurations_clk_gate_cntrl);
bdmf_error_t ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_get(uint8_t bbh_id_tx, bbh_tx_common_configurations_clk_gate_cntrl *common_configurations_clk_gate_cntrl);
bdmf_error_t ag_drv_bbh_tx_common_configurations_gpr_set(uint8_t bbh_id_tx, uint32_t gpr);
bdmf_error_t ag_drv_bbh_tx_common_configurations_gpr_get(uint8_t bbh_id_tx, uint32_t *gpr);
bdmf_error_t ag_drv_bbh_tx_common_configurations_ds_dma_sup_set(uint8_t bbh_id_tx, bdmf_boolean dsdma);
bdmf_error_t ag_drv_bbh_tx_common_configurations_ds_dma_sup_get(uint8_t bbh_id_tx, bdmf_boolean *dsdma);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_q2rnr_set(uint8_t bbh_id_tx, uint8_t wan_q_2_rnr_index, bdmf_boolean q0, bdmf_boolean q1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_q2rnr_get(uint8_t bbh_id_tx, uint8_t wan_q_2_rnr_index, bdmf_boolean *q0, bdmf_boolean *q1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_qprof_set(uint8_t bbh_id_tx, uint8_t wan_qarof_size_index, bdmf_boolean q0, bdmf_boolean q1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_qprof_get(uint8_t bbh_id_tx, uint8_t wan_qarof_size_index, bdmf_boolean *q0, bdmf_boolean *q1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdsize_set(uint8_t bbh_id_tx, uint16_t fifosize0, uint16_t fifosize1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdsize_get(uint8_t bbh_id_tx, uint16_t *fifosize0, uint16_t *fifosize1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdwkuph_set(uint8_t bbh_id_tx, uint8_t wkupthresh0, uint8_t wkupthresh1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdwkuph_get(uint8_t bbh_id_tx, uint8_t *wkupthresh0, uint8_t *wkupthresh1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pd_byte_th_set(uint8_t bbh_id_tx, uint16_t pdlimit0, uint16_t pdlimit1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pd_byte_th_get(uint8_t bbh_id_tx, uint16_t *pdlimit0, uint16_t *pdlimit1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_qmq_set(uint8_t bbh_id_tx, uint8_t wan_qmq_size_index, bdmf_boolean q0, bdmf_boolean q1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_qmq_get(uint8_t bbh_id_tx, uint8_t wan_qmq_size_index, bdmf_boolean *q0, bdmf_boolean *q1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pd_byte_th_en_set(uint8_t bbh_id_tx, bdmf_boolean pdlimiten);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pd_byte_th_en_get(uint8_t bbh_id_tx, bdmf_boolean *pdlimiten);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdempty_set(uint8_t bbh_id_tx, uint8_t empty);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdempty_get(uint8_t bbh_id_tx, uint8_t *empty);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_set(uint8_t bbh_id_tx, uint8_t wan_sts_rnr_cfg_1_index, uint16_t tcontaddr);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_get(uint8_t bbh_id_tx, uint8_t wan_sts_rnr_cfg_1_index, uint16_t *tcontaddr);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_set(uint8_t bbh_id_tx, uint8_t wan_sts_rnr_cfg_2_index, uint16_t ptraddr, uint8_t task);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_get(uint8_t bbh_id_tx, uint8_t wan_sts_rnr_cfg_2_index, uint16_t *ptraddr, uint8_t *task);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_msgrnrcfg_1_set(uint8_t bbh_id_tx, uint8_t wan_msg_rnr_cfg_1_index, uint16_t tcontaddr);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_msgrnrcfg_1_get(uint8_t bbh_id_tx, uint8_t wan_msg_rnr_cfg_1_index, uint16_t *tcontaddr);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_msgrnrcfg_2_set(uint8_t bbh_id_tx, uint8_t wan_msg_rnr_cfg_2_index, uint16_t ptraddr, uint8_t task);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_msgrnrcfg_2_get(uint8_t bbh_id_tx, uint8_t wan_msg_rnr_cfg_2_index, uint16_t *ptraddr, uint8_t *task);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_epncfg_set(uint8_t bbh_id_tx, const bbh_tx_wan_configurations_epncfg *wan_configurations_epncfg);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_epncfg_get(uint8_t bbh_id_tx, bbh_tx_wan_configurations_epncfg *wan_configurations_epncfg);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_flow2port_set(uint8_t bbh_id_tx, uint32_t wdata, uint8_t a, bdmf_boolean cmd);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_flow2port_get(uint8_t bbh_id_tx, uint32_t *wdata, uint8_t *a, bdmf_boolean *cmd);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_ts_set(uint8_t bbh_id_tx, bdmf_boolean en);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_ts_get(uint8_t bbh_id_tx, bdmf_boolean *en);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_maxwlen_set(uint8_t bbh_id_tx, uint16_t maxwlen);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_maxwlen_get(uint8_t bbh_id_tx, uint16_t *maxwlen);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_flush_set(uint8_t bbh_id_tx, uint16_t flush, bdmf_boolean srst_n);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_flush_get(uint8_t bbh_id_tx, uint16_t *flush, bdmf_boolean *srst_n);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_q2rnr_set(uint8_t bbh_id_tx, bdmf_boolean q0, bdmf_boolean q1);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_q2rnr_get(uint8_t bbh_id_tx, bdmf_boolean *q0, bdmf_boolean *q1);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_qprof_set(uint8_t bbh_id_tx, bdmf_boolean q0, bdmf_boolean q1);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_qprof_get(uint8_t bbh_id_tx, bdmf_boolean *q0, bdmf_boolean *q1);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_pdsize_set(uint8_t bbh_id_tx, uint16_t fifosize0, uint16_t fifosize1);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_pdsize_get(uint8_t bbh_id_tx, uint16_t *fifosize0, uint16_t *fifosize1);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_pdwkuph_set(uint8_t bbh_id_tx, uint8_t wkupthresh0, uint8_t wkupthresh1);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_pdwkuph_get(uint8_t bbh_id_tx, uint8_t *wkupthresh0, uint8_t *wkupthresh1);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_pd_byte_th_set(uint8_t bbh_id_tx, uint16_t pdlimit0, uint16_t pdlimit1);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_pd_byte_th_get(uint8_t bbh_id_tx, uint16_t *pdlimit0, uint16_t *pdlimit1);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_qmq_set(uint8_t bbh_id_tx, bdmf_boolean q0, bdmf_boolean q1);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_qmq_get(uint8_t bbh_id_tx, bdmf_boolean *q0, bdmf_boolean *q1);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_pd_byte_th_en_set(uint8_t bbh_id_tx, bdmf_boolean pdlimiten);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_pd_byte_th_en_get(uint8_t bbh_id_tx, bdmf_boolean *pdlimiten);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_pdempty_set(uint8_t bbh_id_tx, uint8_t empty);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_pdempty_get(uint8_t bbh_id_tx, uint8_t *empty);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_txthresh_set(uint8_t bbh_id_tx, uint16_t ddrthresh, uint16_t sramthresh);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_txthresh_get(uint8_t bbh_id_tx, uint16_t *ddrthresh, uint16_t *sramthresh);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_eee_set(uint8_t bbh_id_tx, bdmf_boolean en);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_eee_get(uint8_t bbh_id_tx, bdmf_boolean *en);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_ts_set(uint8_t bbh_id_tx, bdmf_boolean en);
bdmf_error_t ag_drv_bbh_tx_lan_configurations_ts_get(uint8_t bbh_id_tx, bdmf_boolean *en);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_q2rnr_set(uint8_t bbh_id_tx, uint8_t unified_q2rnr_size_index, bdmf_boolean q0, bdmf_boolean q1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_q2rnr_get(uint8_t bbh_id_tx, uint8_t unified_q2rnr_size_index, bdmf_boolean *q0, bdmf_boolean *q1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_qprof_set(uint8_t bbh_id_tx, uint8_t unified_qprof_size_index, bdmf_boolean q0, bdmf_boolean q1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_qprof_get(uint8_t bbh_id_tx, uint8_t unified_qprof_size_index, bdmf_boolean *q0, bdmf_boolean *q1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdsize_set(uint8_t bbh_id_tx, uint16_t fifosize0, uint16_t fifosize1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdsize_get(uint8_t bbh_id_tx, uint16_t *fifosize0, uint16_t *fifosize1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdwkuph_set(uint8_t bbh_id_tx, uint8_t wkupthresh0, uint8_t wkupthresh1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdwkuph_get(uint8_t bbh_id_tx, uint8_t *wkupthresh0, uint8_t *wkupthresh1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pd_byte_th_set(uint8_t bbh_id_tx, uint16_t pdlimit0, uint16_t pdlimit1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pd_byte_th_get(uint8_t bbh_id_tx, uint16_t *pdlimit0, uint16_t *pdlimit1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_qmq_set(uint8_t bbh_id_tx, uint8_t unified_qmq_size_index, bdmf_boolean q0, bdmf_boolean q1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_qmq_get(uint8_t bbh_id_tx, uint8_t unified_qmq_size_index, bdmf_boolean *q0, bdmf_boolean *q1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pd_byte_th_en_set(uint8_t bbh_id_tx, bdmf_boolean pdlimiten);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pd_byte_th_en_get(uint8_t bbh_id_tx, bdmf_boolean *pdlimiten);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdempty_set(uint8_t bbh_id_tx, uint8_t empty);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdempty_get(uint8_t bbh_id_tx, uint8_t *empty);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_gtxthresh_set(uint8_t bbh_id_tx, uint16_t ddrthresh, uint16_t sramthresh);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_gtxthresh_get(uint8_t bbh_id_tx, uint16_t *ddrthresh, uint16_t *sramthresh);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_eee_set(uint8_t bbh_id_tx, uint8_t en);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_eee_get(uint8_t bbh_id_tx, uint8_t *en);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_ts_set(uint8_t bbh_id_tx, uint8_t en);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_ts_get(uint8_t bbh_id_tx, uint8_t *en);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_febase_set(uint8_t bbh_id_tx, uint8_t unified_fe_base_index, uint16_t fifobase0, uint16_t fifobase1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_febase_get(uint8_t bbh_id_tx, uint8_t unified_fe_base_index, uint16_t *fifobase0, uint16_t *fifobase1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_fesize_set(uint8_t bbh_id_tx, uint8_t unified_fe_size_index, uint16_t fifosize0, uint16_t fifosize1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_fesize_get(uint8_t bbh_id_tx, uint8_t unified_fe_size_index, uint16_t *fifosize0, uint16_t *fifosize1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_fepdbase_set(uint8_t bbh_id_tx, uint8_t unified_fe_pd_base_index, uint8_t fifobase0, uint8_t fifobase1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_fepdbase_get(uint8_t bbh_id_tx, uint8_t unified_fe_pd_base_index, uint8_t *fifobase0, uint8_t *fifobase1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_fepdsize_set(uint8_t bbh_id_tx, uint8_t unified_fe_pd_size_index, uint8_t fifosize0, uint8_t fifosize1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_fepdsize_get(uint8_t bbh_id_tx, uint8_t unified_fe_pd_size_index, uint8_t *fifosize0, uint8_t *fifosize1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_txwrr_set(uint8_t bbh_id_tx, uint8_t unified_tx_wrr_index, uint8_t w0, uint8_t w1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_txwrr_get(uint8_t bbh_id_tx, uint8_t unified_tx_wrr_index, uint8_t *w0, uint8_t *w1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_txthresh_set(uint8_t bbh_id_tx, uint8_t unified_tx_thr_index, uint16_t thresh0, uint16_t thresh1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_txthresh_get(uint8_t bbh_id_tx, uint8_t unified_tx_thr_index, uint16_t *thresh0, uint16_t *thresh1);
bdmf_error_t ag_drv_bbh_tx_debug_counters_srambyte_get(uint8_t bbh_id_tx, uint32_t *srambyte);
bdmf_error_t ag_drv_bbh_tx_debug_counters_ddrbyte_get(uint8_t bbh_id_tx, uint32_t *ddrbyte);
bdmf_error_t ag_drv_bbh_tx_debug_counters_swrden_set(uint8_t bbh_id_tx, const bbh_tx_debug_counters_swrden *debug_counters_swrden);
bdmf_error_t ag_drv_bbh_tx_debug_counters_swrden_get(uint8_t bbh_id_tx, bbh_tx_debug_counters_swrden *debug_counters_swrden);
bdmf_error_t ag_drv_bbh_tx_debug_counters_swrdaddr_set(uint8_t bbh_id_tx, uint16_t rdaddr);
bdmf_error_t ag_drv_bbh_tx_debug_counters_swrdaddr_get(uint8_t bbh_id_tx, uint16_t *rdaddr);
bdmf_error_t ag_drv_bbh_tx_debug_counters_swrddata_get(uint8_t bbh_id_tx, uint32_t *data);
bdmf_error_t ag_drv_bbh_tx_debug_counters_unifiedpkt_get(uint8_t bbh_id_tx, uint8_t debug_unified_pkt_ctr_idx, uint32_t *ddrbyte);
bdmf_error_t ag_drv_bbh_tx_debug_counters_unifiedbyte_get(uint8_t bbh_id_tx, uint8_t debug_unified_byte_ctr_idx, uint32_t *ddrbyte);
bdmf_error_t ag_drv_bbh_tx_debug_counters_dbgoutreg_get(uint8_t bbh_id_tx, uint8_t zero, bbh_tx_debug_counters_dbgoutreg *debug_counters_dbgoutreg);
bdmf_error_t ag_drv_bbh_tx_debug_counters_in_segmentation_get(uint8_t bbh_id_tx, uint8_t bbh_tx_debug_counters_in_segmentation_byte_ctr_idx, uint32_t *in_segmentation);

#ifdef USE_BDMF_SHELL
enum
{
    cli_bbh_tx_mac_type,
    cli_bbh_tx_cfg_src_id,
    cli_bbh_tx_rnr_src_id,
    cli_bbh_tx_bbh_dma_cfg,
    cli_bbh_tx_dma_max_otf_read_request,
    cli_bbh_tx_dma_epon_urgent,
    cli_bbh_tx_bbh_sdma_cfg,
    cli_bbh_tx_sdma_max_otf_read_request,
    cli_bbh_tx_sdma_epon_urgent,
    cli_bbh_tx_bbh_ddr_cfg,
    cli_bbh_tx_debug_counters,
    cli_bbh_tx_common_configurations_rnrcfg_1,
    cli_bbh_tx_common_configurations_rnrcfg_2,
    cli_bbh_tx_common_configurations_sbpmcfg,
    cli_bbh_tx_common_configurations_ddrtmbasel,
    cli_bbh_tx_common_configurations_ddrtmbaseh,
    cli_bbh_tx_common_configurations_dfifoctrl,
    cli_bbh_tx_common_configurations_arb_cfg,
    cli_bbh_tx_common_configurations_bbroute,
    cli_bbh_tx_common_configurations_perqtask,
    cli_bbh_tx_common_configurations_txrstcmd,
    cli_bbh_tx_common_configurations_dbgsel,
    cli_bbh_tx_common_configurations_clk_gate_cntrl,
    cli_bbh_tx_common_configurations_gpr,
    cli_bbh_tx_common_configurations_ds_dma_sup,
    cli_bbh_tx_wan_configurations_q2rnr,
    cli_bbh_tx_wan_configurations_qprof,
    cli_bbh_tx_wan_configurations_pdsize,
    cli_bbh_tx_wan_configurations_pdwkuph,
    cli_bbh_tx_wan_configurations_pd_byte_th,
    cli_bbh_tx_wan_configurations_qmq,
    cli_bbh_tx_wan_configurations_pd_byte_th_en,
    cli_bbh_tx_wan_configurations_pdempty,
    cli_bbh_tx_wan_configurations_stsrnrcfg_1,
    cli_bbh_tx_wan_configurations_stsrnrcfg_2,
    cli_bbh_tx_wan_configurations_msgrnrcfg_1,
    cli_bbh_tx_wan_configurations_msgrnrcfg_2,
    cli_bbh_tx_wan_configurations_epncfg,
    cli_bbh_tx_wan_configurations_flow2port,
    cli_bbh_tx_wan_configurations_ts,
    cli_bbh_tx_wan_configurations_maxwlen,
    cli_bbh_tx_wan_configurations_flush,
    cli_bbh_tx_lan_configurations_q2rnr,
    cli_bbh_tx_lan_configurations_qprof,
    cli_bbh_tx_lan_configurations_pdsize,
    cli_bbh_tx_lan_configurations_pdwkuph,
    cli_bbh_tx_lan_configurations_pd_byte_th,
    cli_bbh_tx_lan_configurations_qmq,
    cli_bbh_tx_lan_configurations_pd_byte_th_en,
    cli_bbh_tx_lan_configurations_pdempty,
    cli_bbh_tx_lan_configurations_txthresh,
    cli_bbh_tx_lan_configurations_eee,
    cli_bbh_tx_lan_configurations_ts,
    cli_bbh_tx_unified_configurations_q2rnr,
    cli_bbh_tx_unified_configurations_qprof,
    cli_bbh_tx_unified_configurations_pdsize,
    cli_bbh_tx_unified_configurations_pdwkuph,
    cli_bbh_tx_unified_configurations_pd_byte_th,
    cli_bbh_tx_unified_configurations_qmq,
    cli_bbh_tx_unified_configurations_pd_byte_th_en,
    cli_bbh_tx_unified_configurations_pdempty,
    cli_bbh_tx_unified_configurations_gtxthresh,
    cli_bbh_tx_unified_configurations_eee,
    cli_bbh_tx_unified_configurations_ts,
    cli_bbh_tx_unified_configurations_febase,
    cli_bbh_tx_unified_configurations_fesize,
    cli_bbh_tx_unified_configurations_fepdbase,
    cli_bbh_tx_unified_configurations_fepdsize,
    cli_bbh_tx_unified_configurations_txwrr,
    cli_bbh_tx_unified_configurations_txthresh,
    cli_bbh_tx_debug_counters_srambyte,
    cli_bbh_tx_debug_counters_ddrbyte,
    cli_bbh_tx_debug_counters_swrden,
    cli_bbh_tx_debug_counters_swrdaddr,
    cli_bbh_tx_debug_counters_swrddata,
    cli_bbh_tx_debug_counters_unifiedpkt,
    cli_bbh_tx_debug_counters_unifiedbyte,
    cli_bbh_tx_debug_counters_dbgoutreg,
    cli_bbh_tx_debug_counters_in_segmentation,
};

int bcm_bbh_tx_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_bbh_tx_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif

