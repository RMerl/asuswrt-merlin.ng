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

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif

typedef struct
{
    uint8_t fpmsrc;
    uint8_t sbpmsrc;
    uint8_t stsrnrsrc;
    uint8_t msgrnrsrc;
} bbh_tx_cfg_src_id;

typedef struct
{
    uint8_t dmasrc;
    uint8_t descbase;
    uint8_t descsize;
} bbh_tx_bbh_dma_cfg;

typedef struct
{
    uint8_t sdmasrc;
    uint8_t descbase;
    uint8_t descsize;
} bbh_tx_bbh_sdma_cfg;

typedef struct
{
    uint8_t bufsize;
    bdmf_boolean byteresul;
    uint16_t ddrtxoffset;
    uint8_t hnsize0;
    uint8_t hnsize1;
} bbh_tx_bbh_ddr_cfg;

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

typedef struct
{
    bdmf_boolean freenocntxt;
    bdmf_boolean specialfree;
    bdmf_boolean second_bn_dis;
    bdmf_boolean second_bn_len_mis_dis;
    bdmf_boolean use_second_bn_from_pd_in_free;
    bdmf_boolean use_second_bn_from_pd_in_free_len_mis;
    bdmf_boolean use_free_without_cntxt_len_mis;
    uint8_t maxgn;
} bbh_tx_common_configurations_sbpmcfg;

typedef struct
{
    uint32_t addr[2];
} bbh_tx_common_configurations_ddrtmbasel;

typedef struct
{
    uint32_t addr[2];
} bbh_tx_common_configurations_ddrtmbaseh;

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

typedef struct
{
    bdmf_boolean bypass_clk_gate;
    uint8_t timer_val;
    bdmf_boolean keep_alive_en;
    uint8_t keep_alive_intrvl;
    uint8_t keep_alive_cyc;
} bbh_tx_common_configurations_clk_gate_cntrl;

typedef struct
{
    bdmf_boolean dsdma;
    bdmf_boolean agg640;
    bdmf_boolean pair256;
    bdmf_boolean macdrop;
    bdmf_boolean embhdr;
    bdmf_boolean ecnrmrk;
    bdmf_boolean cntraddcrc;
    bdmf_boolean dblsop;
    bdmf_boolean fpmini;
    bdmf_boolean crdtfix;
    bdmf_boolean discopycrdt;
} bbh_tx_common_configurations_general_cfg;

typedef struct
{
    uint8_t ecnbyteipv4;
    uint8_t ecnbitipv4;
    uint8_t ecnbyteipv6;
    uint8_t ecnbitipv6;
    uint8_t chksumbyteipv4;
} bbh_tx_common_configurations_ecncfg;

typedef struct
{
    bdmf_boolean stplenerr;
    bdmf_boolean cmp_width;
    bdmf_boolean considerfull;
    bdmf_boolean addcrc;
    uint8_t req_full;
    bdmf_boolean sendrdptr;
} bbh_tx_wan_configurations_epncfg;

typedef struct
{
    uint16_t maxwlen;
    uint16_t min_credit;
    uint8_t spare;
    bdmf_boolean prio_en;
    bdmf_boolean srst_n;
} bbh_tx_wan_configurations_dsl_cfg;

typedef struct
{
    uint8_t flush_wait_cycles;
    bdmf_boolean sw_flush_done;
    uint8_t flush_wait_en;
    bdmf_boolean sw_flush_req;
    bdmf_boolean flush_ignore_rd;
    uint16_t sw_crdts_val;
    bdmf_boolean sw_crdts_init;
} bbh_tx_wan_configurations_dsl_cfg2;

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

typedef struct
{
    uint8_t oflw_q;
    bdmf_boolean oflw;
    bdmf_boolean flush_req;
    bdmf_boolean flush;
    bdmf_boolean flush_done;
    bdmf_boolean init_req;
    bdmf_boolean init_done;
    uint16_t credit_init_val;
} bbh_tx_debug_counters_dsl_sts;

typedef struct
{
    uint32_t debug_counters_dsl_credits[8];
} bbh_tx_debug_counters_dsl_credits;

typedef struct
{
    uint32_t debug_out_reg[8];
} bbh_tx_debug_counters_dbgoutreg;

typedef struct
{
    bdmf_boolean ddr_fifo;
    bdmf_boolean sram_fifo;
    bdmf_boolean ddr_reorder_fifo;
    bdmf_boolean sram_reorder_fifo;
    bdmf_boolean bpm_fifo;
    bdmf_boolean bpm_flush_fifo;
    bdmf_boolean bpm_eop_fifo;
    bdmf_boolean sbpm_fifo;
    bdmf_boolean sbpm_flush_fifo;
    bdmf_boolean sbpm_eop_fifo;
    bdmf_boolean dbr_sync_fifo;
} bbh_tx_debug_counters_fifos_overrun;


/**********************************************************************************************************************
 * type: 
 *     MAC type
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_mac_type_set(uint8_t bbh_id, uint8_t type);
bdmf_error_t ag_drv_bbh_tx_mac_type_get(uint8_t bbh_id, uint8_t *type);

/**********************************************************************************************************************
 * fpmsrc: 
 *     source id. This id is used to determine the route to the module.
 * sbpmsrc: 
 *     source id. This id is used to determine the route to the module.
 * stsrnrsrc: 
 *     source id. This id is used to determine the route to the Runner that is responsible for sending status messages
 *     (WAN only).
 * msgrnrsrc: 
 *     source id. This id is used to determine the route to the Runner which is responsible for sending DBR/Ghost
 *     messages (WAN only).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_cfg_src_id_set(uint8_t bbh_id, const bbh_tx_cfg_src_id *cfg_src_id);
bdmf_error_t ag_drv_bbh_tx_cfg_src_id_get(uint8_t bbh_id, bbh_tx_cfg_src_id *cfg_src_id);

/**********************************************************************************************************************
 * pdrnr0src: 
 *     source id. This id is used to determine the route to the 1st (out of possible 2 runners) which are responsible
 *     for sending PDs.
 * pdrnr1src: 
 *     source id. This id is used to determine the route to the 2nd (out of possible 2 runners) which are responsible
 *     for sending PDs.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_rnr_src_id_set(uint8_t bbh_id, uint8_t pdrnr0src, uint8_t pdrnr1src);
bdmf_error_t ag_drv_bbh_tx_rnr_src_id_get(uint8_t bbh_id, uint8_t *pdrnr0src, uint8_t *pdrnr1src);

/**********************************************************************************************************************
 * dmasrc: 
 *     source id. This id is used to determine the route to the module.
 * descbase: 
 *     Defines the base address of the read request FIFO within the DMA address space.
 *     The value should be identical to the relevant configuration in the DMA.
 * descsize: 
 *     The size of the BBH read requests FIFO inside the DMA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_bbh_dma_cfg_set(uint8_t bbh_id, const bbh_tx_bbh_dma_cfg *bbh_dma_cfg);
bdmf_error_t ag_drv_bbh_tx_bbh_dma_cfg_get(uint8_t bbh_id, bbh_tx_bbh_dma_cfg *bbh_dma_cfg);

/**********************************************************************************************************************
 * maxreq: 
 *     Defines the maximum allowed number of on-the-fly read requests.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_dma_max_otf_read_request_set(uint8_t bbh_id, uint8_t maxreq);
bdmf_error_t ag_drv_bbh_tx_dma_max_otf_read_request_get(uint8_t bbh_id, uint8_t *maxreq);

/**********************************************************************************************************************
 * epnurgnt: 
 *     When asserted, this bit forces urgent priority on the EPON read requests towards the DMA (relevant only for
 *     EPON BBH)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_dma_epon_urgent_set(uint8_t bbh_id, bdmf_boolean epnurgnt);
bdmf_error_t ag_drv_bbh_tx_dma_epon_urgent_get(uint8_t bbh_id, bdmf_boolean *epnurgnt);

/**********************************************************************************************************************
 * sdmasrc: 
 *     source id. This id is used to determine the route to the module.
 * descbase: 
 *     Defines the base address of the read request FIFO within the DMA address space.
 *     The value should be identical to the relevant configuration in the DMA.
 * descsize: 
 *     The size of the BBH read requests FIFO inside the DMA
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_bbh_sdma_cfg_set(uint8_t bbh_id, const bbh_tx_bbh_sdma_cfg *bbh_sdma_cfg);
bdmf_error_t ag_drv_bbh_tx_bbh_sdma_cfg_get(uint8_t bbh_id, bbh_tx_bbh_sdma_cfg *bbh_sdma_cfg);

/**********************************************************************************************************************
 * maxreq: 
 *     Defines the maximum allowed number of on-the-fly read requests.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_sdma_max_otf_read_request_set(uint8_t bbh_id, uint8_t maxreq);
bdmf_error_t ag_drv_bbh_tx_sdma_max_otf_read_request_get(uint8_t bbh_id, uint8_t *maxreq);

/**********************************************************************************************************************
 * epnurgnt: 
 *     When asserted, this bit forces urgent priority on the EPON read requests towards the DMA (relevant only for
 *     EPON BBH)
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_sdma_epon_urgent_set(uint8_t bbh_id, bdmf_boolean epnurgnt);
bdmf_error_t ag_drv_bbh_tx_sdma_epon_urgent_get(uint8_t bbh_id, bdmf_boolean *epnurgnt);

/**********************************************************************************************************************
 * bufsize: 
 *     The data is arranged in the DDR in a fixed size buffers.
 * byteresul: 
 *     The packet offset byte resulotion.
 * ddrtxoffset: 
 *     Static offset in 8-bytes resolution for non aggregated packets in DDR
 * hnsize0: 
 *     The size of the HN (Header number) in bytes. The BBH decides between size 0 and size 1 according to a bit in
 *     the PD.
 *     In multicast, several headers are located in the same DDR buffer. the offset of header N within a buffer is
 *     (N-1)*64bytes, regardless of the HN actual size.
 * hnsize1: 
 *     The size of the HN (Header number) in bytes. The BBH decides between size 0 and size 1 according to a bit in
 *     the PD.
 *     In multicast, several headers are located in the same DDR buffer. the offset of header N within a buffer is
 *     (N-1)*64bytes, regardless of the HN actual size.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_bbh_ddr_cfg_set(uint8_t bbh_id, const bbh_tx_bbh_ddr_cfg *bbh_ddr_cfg);
bdmf_error_t ag_drv_bbh_tx_bbh_ddr_cfg_get(uint8_t bbh_id, bbh_tx_bbh_ddr_cfg *bbh_ddr_cfg);

/**********************************************************************************************************************
 * ddrbyte: 
 *     This counter counts the number of transmitted bytes from the DDr.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_ddrbyte_get(uint8_t bbh_id, uint32_t *ddrbyte);

/**********************************************************************************************************************
 * srambyte: 
 *     This counter counts the number of transmitted bytes from the SRAM.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_srambyte_get(uint8_t bbh_id, uint32_t *srambyte);

/**********************************************************************************************************************
 * srampd: 
 *     This counter counts the number of packets which were transmitted from the SRAM.
 * ddrpd: 
 *     This counter counts the number of packets which were transmitted from the DDR.
 * pddrop: 
 *     This counter counts the number of PDs which were dropped due to PD FIFO full.
 * stscnt: 
 *     This counter counts the number of received status messages.
 * stsdrop: 
 *     This counter counts the number of STS which were dropped due to PD FIFO full.
 * msgcnt: 
 *     This counter counts the number of received DBR/ghost messages.
 * msgdrop: 
 *     This counter counts the number of MSG which were dropped due to PD FIFO full.
 * getnextnull: 
 *     This counter counts the number Get next responses with a null BN.
 * lenerr: 
 *     This counter counts the number of times a length error occuered
 * aggrlenerr: 
 *     This counter counts the number of times an aggregation length error occuered
 * srampkt: 
 *     This counter counts the number of packets which were transmitted from the SRAM.
 * ddrpkt: 
 *     This counter counts the number of packets which were transmitted from the DDR.
 * flshpkts: 
 *     This counter counts the number of flushed packets
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_get(uint8_t bbh_id, bbh_tx_debug_counters *debug_counters);

/**********************************************************************************************************************
 * freenocntxt: 
 *     When this bit is enabled, the BBH will use free without context command.
 * specialfree: 
 *     When this bit is enabled, the BBH will use special free with context command.
 *     This bit is relevant only if free without context_en is configured to 0.
 * second_bn_dis: 
 *     The bbh tx uses the second BN from the PD if the packet is 2 buffers long. This bit disables this optimization,
 *     so the BBH tx will always use get next command to understand what is the next BN
 * second_bn_len_mis_dis: 
 *     The bbh tx uses the second BN from the PD if the packet is 2 buffers long. This bit disables this optimization
 *     in case there is a mismatch between the PLEN and the number of SBNs in the PD, so the BBH tx will use get next
 *     command to understand what is the next BN in this case
 * use_second_bn_from_pd_in_free: 
 *     When freeing a packet that is located in SRAM, the BBH uses the last SBN he got from issuing get next commands
 *     for the free command.
 *     When this bit is asserted, the BBH will not use the last SBN from the get-next, but the one from the PD.
 * use_second_bn_from_pd_in_free_len_mis: 
 *     When freeing a packet that is located in SRAM, the BBH uses the last SBN he got from issuing get next commands
 *     for the free command.
 *     When this bit is asserted and there is a mismatch between the number of SBNs and the PLEN in the PD, and the
 *     BBH is not configured to use free without context, the BBH will not use the last SBN from the get-next, but the
 *     one from the PD.
 * use_free_without_cntxt_len_mis: 
 *     When freeing a packet that is located in SRAM, the BBH uses the last SBN he got from issuing get next commands
 *     for the free command.
 *     When this bit is asserted and there is a length mismatch between the PLEN and the number of SBNs in the PD, the
 *     BBH will not use the last SBN, but will send free without context command to the SBPM
 * maxgn: 
 *     maximum number of pending on the fly get next commands
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_sbpmcfg_set(uint8_t bbh_id, const bbh_tx_common_configurations_sbpmcfg *common_configurations_sbpmcfg);
bdmf_error_t ag_drv_bbh_tx_common_configurations_sbpmcfg_get(uint8_t bbh_id, bbh_tx_common_configurations_sbpmcfg *common_configurations_sbpmcfg);

/**********************************************************************************************************************
 * ddrtmbase: 
 *     DDR TM base.
 *     The address is in bytes resolution.
 *     The address should be aligned to 128 bytes.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrtmbasel_set(uint8_t bbh_id, uint8_t zero, const bbh_tx_common_configurations_ddrtmbasel *common_configurations_ddrtmbasel);
bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrtmbasel_get(uint8_t bbh_id, uint8_t zero, bbh_tx_common_configurations_ddrtmbasel *common_configurations_ddrtmbasel);

/**********************************************************************************************************************
 * ddrtmbase: 
 *     MSB of DDR TM base.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrtmbaseh_set(uint8_t bbh_id, uint8_t zero, const bbh_tx_common_configurations_ddrtmbaseh *common_configurations_ddrtmbaseh);
bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrtmbaseh_get(uint8_t bbh_id, uint8_t zero, bbh_tx_common_configurations_ddrtmbaseh *common_configurations_ddrtmbaseh);

/**********************************************************************************************************************
 * psramsize: 
 *     The size of the PSRAM data FIFO in 8 bytes resolution. The BBH uses this information for determining the amount
 *     of data that can be ordered from the PSRAM.
 * ddrsize: 
 *     The size of the DDR data FIFO in 8 bytes resolution. The BBH uses this information for determining the amount
 *     of data that can be ordered from the DDR.
 * psrambase: 
 *     the base address of the PSRAM data FIFO in 8 bytes resolution. The DDR data FIFO base address is always 0.
 *     In case the whole RAM is to be dedicated to PSRAM data, the base should be 0 as well, and the DDR FIFO size
 *     should be configured to 0.
 * reorder_per_q_en: 
 *     When asserted, the BBH TX will do reorder per q, meaning order between SRAM and DDR  pds willl be kept only per
 *     q.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_dfifoctrl_set(uint8_t bbh_id, uint16_t psramsize, uint16_t ddrsize, uint16_t psrambase, bdmf_boolean reorder_per_q_en);
bdmf_error_t ag_drv_bbh_tx_common_configurations_dfifoctrl_get(uint8_t bbh_id, uint16_t *psramsize, uint16_t *ddrsize, uint16_t *psrambase, bdmf_boolean *reorder_per_q_en);

/**********************************************************************************************************************
 * hightrxq: 
 *     this configuration determines whether to give high priority to a current transmitting queue or not.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_arb_cfg_set(uint8_t bbh_id, bdmf_boolean hightrxq);
bdmf_error_t ag_drv_bbh_tx_common_configurations_arb_cfg_get(uint8_t bbh_id, bdmf_boolean *hightrxq);

/**********************************************************************************************************************
 * route: 
 *     route address
 * dest: 
 *     destination source id
 * en: 
 *     enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_bbroute_set(uint8_t bbh_id, uint16_t route, uint8_t dest, bdmf_boolean en);
bdmf_error_t ag_drv_bbh_tx_common_configurations_bbroute_get(uint8_t bbh_id, uint16_t *route, uint8_t *dest, bdmf_boolean *en);

/**********************************************************************************************************************
 * agg_poolid: 
 *     the pool-d is needed by the FPM in free commands. The pool-id for regular packets is determined in the PD. For
 *     aggregated PD it is determined here. This register should be aligned with the same value that is configured in
 *     the QM.
 * mcst_poolid: 
 *     the pool-id is needed by the FPM in free commands. The pool-id for regular packets is determined in the PD.
 *     multicast headers BN, the pool-id will be taken from here
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrcfg_tx2_set(uint8_t bbh_id, uint8_t agg_poolid, uint8_t mcst_poolid);
bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrcfg_tx2_get(uint8_t bbh_id, uint8_t *agg_poolid, uint8_t *mcst_poolid);

/**********************************************************************************************************************
 * tcontaddr: 
 *     Defines the TCONT address within the Runner address space.
 *     The address is in 8 bytes resolution.
 *     
 * skbaddr: 
 *     Defines the SKB free address within the Runner address space.
 *     The address is in 8-bytes resolution.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_rnrcfg_1_set(uint8_t bbh_id, uint8_t rnr_cfg_index_1, uint16_t tcontaddr, uint16_t skbaddr);
bdmf_error_t ag_drv_bbh_tx_common_configurations_rnrcfg_1_get(uint8_t bbh_id, uint8_t rnr_cfg_index_1, uint16_t *tcontaddr, uint16_t *skbaddr);

/**********************************************************************************************************************
 * ptraddr: 
 *     This field defins the address in the Runner memory space to which the read pointer is written.
 *     The address is in 8-bytes resolution.
 * task: 
 *     The number of the task that is responsible for sending PDs to the BBH
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_rnrcfg_2_set(uint8_t bbh_id, uint16_t rnr_cfg_index_2, uint16_t ptraddr, uint8_t task);
bdmf_error_t ag_drv_bbh_tx_common_configurations_rnrcfg_2_get(uint8_t bbh_id, uint16_t rnr_cfg_index_2, uint16_t *ptraddr, uint8_t *task);

/**********************************************************************************************************************
 * task0: 
 *     task number for queue 0
 * task1: 
 *     task number for queue 1
 * task2: 
 *     task number for queue 2
 * task3: 
 *     task number for queue 3
 * task4: 
 *     task number for queue 4
 * task5: 
 *     task number for queue 5
 * task6: 
 *     task number for queue 6
 * task7: 
 *     task number for queue 7
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_perqtask_set(uint8_t bbh_id, const bbh_tx_common_configurations_perqtask *common_configurations_perqtask);
bdmf_error_t ag_drv_bbh_tx_common_configurations_perqtask_get(uint8_t bbh_id, bbh_tx_common_configurations_perqtask *common_configurations_perqtask);

/**********************************************************************************************************************
 * cntxtrst: 
 *     Writing 1 to this register will reset the segmentation context table.
 *     The reset is done immediately. Reading this register will always return 0.
 * pdfiforst: 
 *     Writing 1 to this register will reset the PDs FIFOs.
 *     The reset is done immediately. Reading this register will always return 0.
 * dmaptrrst: 
 *     Writing 1 to this register will reset the DMA write pointer.
 *     The reset is done immediately. Reading this register will always return 0.
 * sdmaptrrst: 
 *     Writing 1 to this register will reset the SDMA write pointer.
 *     The reset is done immediately. Reading this register will always return 0.
 *     This register is relevalt only for Ethernet.
 * bpmfiforst: 
 *     Writing 1 to this register will reset the BPM FIFO.
 *     The reset is done immediately. Reading this register will always return 0.
 * sbpmfiforst: 
 *     Writing 1 to this register will reset the SBPM FIFO.
 *     The reset is done immediately. Reading this register will always return 0.
 *     This register is relevalt only for Ethernet.
 * okfiforst: 
 *     Writing 1 to this register will reset the order keeper FIFO.
 *     The reset is done immediately. Reading this register will always return 0.
 *     This register is relevalt only for Ethernet.
 * ddrfiforst: 
 *     Writing 1 to this register will reset the DDR data FIFO.
 *     The reset is done immediately. Reading this register will always return 0.
 *     This register is relevalt only for Ethernet.
 * sramfiforst: 
 *     Writing 1 to this register will reset the SRAM data FIFO.
 *     The reset is done immediately. Reading this register will always return 0.
 *     This register is relevalt only for Ethernet.
 * skbptrrst: 
 *     Writing 1 to this register will reset the SKB pointers.
 *     The reset is done immediately. Reading this register will always return 0.
 *     
 * stsfiforst: 
 *     Writing 1 to this register will reset the EPON status FIFOs (per queue 32 fifos).
 *     The reset is done immediately. Reading this register will always return 0.
 * reqfiforst: 
 *     Writing 1 to this register will reset the EPON request FIFO (8 entries FIFO that holds the packet requests from
 *     the EPON MAC).
 *     The reset is done immediately. Reading this register will always return 0.
 * msgfiforst: 
 *     Writing 1 to this register will reset the EPON/GPON MSG FIFO
 *     The reset is done immediately. Reading this register will always return 0.
 * gnxtfiforst: 
 *     Writing 1 to this register will reset the GET NEXT FIFOs
 *     The reset is done immediately. Reading this register will always return 0.
 * fbnfiforst: 
 *     Writing 1 to this register will reset the FIRST BN FIFOs
 *     The reset is done immediately. Reading this register will always return 0.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_txrstcmd_set(uint8_t bbh_id, const bbh_tx_common_configurations_txrstcmd *common_configurations_txrstcmd);
bdmf_error_t ag_drv_bbh_tx_common_configurations_txrstcmd_get(uint8_t bbh_id, bbh_tx_common_configurations_txrstcmd *common_configurations_txrstcmd);

/**********************************************************************************************************************
 * dbgsel: 
 *     This register selects 1 of 8 debug vectors.
 *     The selected vector is reflected to DBGOUTREG.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_dbgsel_set(uint8_t bbh_id, uint8_t dbgsel);
bdmf_error_t ag_drv_bbh_tx_common_configurations_dbgsel_get(uint8_t bbh_id, uint8_t *dbgsel);

/**********************************************************************************************************************
 * bypass_clk_gate: 
 *     If set to 1b1 will disable the clock gate logic such to always enable the clock
 * timer_val: 
 *     For how long should the clock stay active once all conditions for clock disable are met.
 *     
 *     
 * keep_alive_en: 
 *     Enables the keep alive logic which will periodically enable the clock to assure that no deadlock of clock being
 *     removed completely will occur
 * keep_alive_intrvl: 
 *     If the KEEP alive option is enabled the field will determine for how many cycles should the clock be active
 * keep_alive_cyc: 
 *     If the KEEP alive option is enabled this field will determine for how many cycles should the clock be disabled
 *     (minus the KEEP_ALIVE_INTERVAL)
 *     
 *     So KEEP_ALIVE_CYCLE must be larger than KEEP_ALIVE_INTERVAL.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_set(uint8_t bbh_id, const bbh_tx_common_configurations_clk_gate_cntrl *common_configurations_clk_gate_cntrl);
bdmf_error_t ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_get(uint8_t bbh_id, bbh_tx_common_configurations_clk_gate_cntrl *common_configurations_clk_gate_cntrl);

/**********************************************************************************************************************
 * gpr: 
 *     general purpose register
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_gpr_set(uint8_t bbh_id, uint32_t gpr);
bdmf_error_t ag_drv_bbh_tx_common_configurations_gpr_get(uint8_t bbh_id, uint32_t *gpr);

/**********************************************************************************************************************
 * dsdma: 
 *     support dual slave DMA
 * agg640: 
 *     support aggregation of 640 bytes
 * pair256: 
 *     when asserted, the segmentation SM will try to group read commands of the same PD to pairs in order to create
 *     longer reads from the DDR
 * macdrop: 
 *     when asserted, the BBH TX will drop the packet data of an invalid MAC flow
 * embhdr: 
 *     when asserted, the BBH TX will read extra 4 bytes for each packet. these 4 bytes will reside before the packet
 *     SOP and will contain extra PD info.
 * ecnrmrk: 
 *     when asserted, the BBH TX will change the ecn bit according to the information in the PD + embedded header
 * cntraddcrc: 
 *     when asserted, the BBH TX will add 4 bytes per packet to all the byte counters, compensating the crc bytes
 * dblsop: 
 *     when asserted, the BBH TX will double the SOP offset of g9991 PDs, to allow 2K sop offset with 10 bits SOP
 *     field in the PD
 * fpmini: 
 *     When asserted, the BBH TX will use FPMini free commands format
 * crdtfix: 
 *     allow unified credits to reach fifos full and not just almost full
 * discopycrdt: 
 *     For the copy BBH TX, there is no need to check for credits in the FE buffer. When this bit is asserted, the
 *     segmentation SM will not check for credits, and the unified IF will not read from the data FIFO is the FE
 *     buffers are full.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_general_cfg_set(uint8_t bbh_id, const bbh_tx_common_configurations_general_cfg *common_configurations_general_cfg);
bdmf_error_t ag_drv_bbh_tx_common_configurations_general_cfg_get(uint8_t bbh_id, bbh_tx_common_configurations_general_cfg *common_configurations_general_cfg);

/**********************************************************************************************************************
 * ecnbyteipv4: 
 *     the number of byte in which the ecn bits are located in ipv4 packets, relative to the ip header offset
 * ecnbitipv4: 
 *     the bit offset of the ECN bits within the byte. 0 means the ecn bits are [1:0], 6 means the ecn bits are [7:6]
 * ecnbyteipv6: 
 *     the number of byte in which the ecn bits are located in ipv6 packets, relative to the ip header offset
 * ecnbitipv6: 
 *     the bit offset of the ECN bits within the byte. 0 means the ecn bits are [1:0], 6 means the ecn bits are [7:6]
 * chksumbyteipv4: 
 *     the offset of the checksum field related to ip header offset in ipv4 packets. The offset must be multiplication
 *     of 2
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_ecncfg_set(uint8_t bbh_id, const bbh_tx_common_configurations_ecncfg *common_configurations_ecncfg);
bdmf_error_t ag_drv_bbh_tx_common_configurations_ecncfg_get(uint8_t bbh_id, bbh_tx_common_configurations_ecncfg *common_configurations_ecncfg);

/**********************************************************************************************************************
 * ddrfpminibase: 
 *     DDR FPMINI base.
 *     The address is in bytes resolution.
 *     The address should be aligned to 128 bytes.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrfpminibasel_set(uint8_t bbh_id, uint32_t ddrfpminibase);
bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrfpminibasel_get(uint8_t bbh_id, uint32_t *ddrfpminibase);

/**********************************************************************************************************************
 * ddrfpminibase: 
 *     DDR FPMINI base.
 *     The address is in bytes resolution.
 *     The address should be aligned to 128 bytes.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrfpminibaseh_set(uint8_t bbh_id, uint8_t ddrfpminibase);
bdmf_error_t ag_drv_bbh_tx_common_configurations_ddrfpminibaseh_get(uint8_t bbh_id, uint8_t *ddrfpminibase);

/**********************************************************************************************************************
 * q0: 
 *     Q0 configuration
 * q1: 
 *     Q1 configuration
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_q2rnr_set(uint8_t bbh_id, uint8_t q_2_rnr_index, uint8_t q0, uint8_t q1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_q2rnr_get(uint8_t bbh_id, uint8_t q_2_rnr_index, uint8_t *q0, uint8_t *q1);

/**********************************************************************************************************************
 * q0: 
 *     Q0 configuration
 * q1: 
 *     Q1 configuration
 * dis0: 
 *     disable q 0
 * dis1: 
 *     disable q 1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_qprof_set(uint8_t bbh_id, uint8_t wan_q_profile_index, bdmf_boolean q0, bdmf_boolean q1, bdmf_boolean dis0, bdmf_boolean dis1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_qprof_get(uint8_t bbh_id, uint8_t wan_q_profile_index, bdmf_boolean *q0, bdmf_boolean *q1, bdmf_boolean *dis0, bdmf_boolean *dis1);

/**********************************************************************************************************************
 * fifosize0: 
 *     The size of PD FIFO for queue 0.
 *     A value of n refers to n+1.
 *     For GPON, the max value is 0x7
 *     For EPON, the max value is 0xf
 * fifosize1: 
 *     The size of PD FIFO for queue 1.
 *     A value of n refers to n+1.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdsize_set(uint8_t bbh_id, uint16_t fifosize0, uint16_t fifosize1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdsize_get(uint8_t bbh_id, uint16_t *fifosize0, uint16_t *fifosize1);

/**********************************************************************************************************************
 * wkupthresh0: 
 *     The wakeup threshold of the PD FIFO for queue 0.
 *     A value of n refers to n+1.
 *     Relevant only for EPON BBH.
 * wkupthresh1: 
 *     The wakeup threshold of the PD FIFO for queue 1.
 *     A value of n refers to n+1.
 *     Relevant only for EPON BBH.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdwkuph_set(uint8_t bbh_id, uint8_t wkupthresh0, uint8_t wkupthresh1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdwkuph_get(uint8_t bbh_id, uint8_t *wkupthresh0, uint8_t *wkupthresh1);

/**********************************************************************************************************************
 * pdlimit0: 
 *     Defines the number of bytes for PDs pre fetch limited according to the total number of bytes.
 *     The value is in 8-bytes resolution.
 *     
 * pdlimit1: 
 *     Defines the number of bytes for PDs pre fetch limited according to the total number of bytes.
 *     The value is in 8-bytes resolution.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pd_byte_th_set(uint8_t bbh_id, uint16_t pdlimit0, uint16_t pdlimit1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pd_byte_th_get(uint8_t bbh_id, uint16_t *pdlimit0, uint16_t *pdlimit1);

/**********************************************************************************************************************
 * q0: 
 *     Q0 configuration
 * q1: 
 *     Q1 configuration
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_qmq_set(uint8_t bbh_id, uint8_t wan_qm_q_index, bdmf_boolean q0, bdmf_boolean q1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_qmq_get(uint8_t bbh_id, uint8_t wan_qm_q_index, bdmf_boolean *q0, bdmf_boolean *q1);

/**********************************************************************************************************************
 * fifosize0: 
 *     The size of PD FIFO for queue 0.
 *     A value of n refers to n+1.
 *     For GPON, the max value is 0x7
 *     For EPON, the max value is 0xf
 * fifosize1: 
 *     The size of PD FIFO for queue 1.
 *     A value of n refers to n+1.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_stssize_set(uint8_t bbh_id, uint16_t fifosize0, uint16_t fifosize1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_stssize_get(uint8_t bbh_id, uint16_t *fifosize0, uint16_t *fifosize1);

/**********************************************************************************************************************
 * wkupthresh0: 
 *     The wakeup threshold of the PD FIFO for queue 0.
 *     A value of n refers to n+1.
 *     Relevant only for EPON BBH.
 * wkupthresh1: 
 *     The wakeup threshold of the PD FIFO for queue 1.
 *     A value of n refers to n+1.
 *     Relevant only for EPON BBH.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_stswkuph_set(uint8_t bbh_id, uint8_t wkupthresh0, uint8_t wkupthresh1);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_stswkuph_get(uint8_t bbh_id, uint8_t *wkupthresh0, uint8_t *wkupthresh1);

/**********************************************************************************************************************
 * pdlimiten: 
 *     This bit enables the above feature (PDs pre fetch limited according to the total number of bytes).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pd_byte_th_en_set(uint8_t bbh_id, bdmf_boolean pdlimiten);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pd_byte_th_en_get(uint8_t bbh_id, bdmf_boolean *pdlimiten);

/**********************************************************************************************************************
 * empty: 
 *     EPON PD FIFO empty threshold.
 *     A queue which its PD FIFO occupancy is below this threshold will have high priority in PD ordering arbitration.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdempty_set(uint8_t bbh_id, uint8_t empty);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_pdempty_get(uint8_t bbh_id, uint8_t *empty);

/**********************************************************************************************************************
 * empty: 
 *     EPON PD FIFO empty threshold.
 *     A queue which its PD FIFO occupancy is below this threshold will have high priority in PD ordering arbitration.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_stsempty_set(uint8_t bbh_id, uint8_t empty);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_stsempty_get(uint8_t bbh_id, uint8_t *empty);

/**********************************************************************************************************************
 * tcontaddr: 
 *     Defines the TCONT address within the Runner address space.
 *     The address is in 8 bytes resolution.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_set(uint8_t bbh_id, uint8_t wan_sts_rnr_cfg_1_index, uint16_t tcontaddr);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_stsrnrcfg_1_get(uint8_t bbh_id, uint8_t wan_sts_rnr_cfg_1_index, uint16_t *tcontaddr);

/**********************************************************************************************************************
 * ptraddr: 
 *     This field defins the address in the Runner memory space to which the read pointer is written.
 *     The address is in 8-bytes resolution.
 * task: 
 *     The number of the task that is responsible for sending PDs to the BBH
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_set(uint8_t bbh_id, uint8_t wan_sts_rnr_cfg_2_index, uint16_t ptraddr, uint8_t task);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_stsrnrcfg_2_get(uint8_t bbh_id, uint8_t wan_sts_rnr_cfg_2_index, uint16_t *ptraddr, uint8_t *task);

/**********************************************************************************************************************
 * tcontaddr: 
 *     Defines the TCONT address within the Runner address space.
 *     The address is in 8 bytes resolution.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_msgrnrcfg_1_set(uint8_t bbh_id, uint8_t wan_msg_rnr_cfg_1_index, uint16_t tcontaddr);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_msgrnrcfg_1_get(uint8_t bbh_id, uint8_t wan_msg_rnr_cfg_1_index, uint16_t *tcontaddr);

/**********************************************************************************************************************
 * ptraddr: 
 *     This field defins the address in the Runner memory space to which the read pointer is written.
 *     The address is in 8-bytes resolution.
 * task: 
 *     The number of the task that is responsible for sending PDs to the BBH
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_msgrnrcfg_2_set(uint8_t bbh_id, uint8_t wan_msg_rnr_cfg_2_index, uint16_t ptraddr, uint8_t task);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_msgrnrcfg_2_get(uint8_t bbh_id, uint8_t wan_msg_rnr_cfg_2_index, uint16_t *ptraddr, uint8_t *task);

/**********************************************************************************************************************
 * stplenerr: 
 *     In case of fatal length error - a mismatch between the request message from MAC and its relevant PD from Runner
 *     - the BBH can stop performing or continue regardless of the error.
 *     The error is also reflected to the SW in a counter.
 * cmp_width: 
 *     configures the width of the comparison of the packet ength.
 *     The length field in the EPON request interface is 11 bit, while it is 14 bit in the pd.
 *     If this bit is 0, then the comparison of the length will be between the 11 bit of the interface and the 11 lsb
 *     bits of the pd.
 *     If this ibt is 1, the comparison will be done between the 11 bits of the interface, concatenated with 3 zeros
 *     and the 14 bits of the pd
 * considerfull: 
 *     determines whether the BBH will consider the sts_full vector state when pushing STS messages to the MAC or not.
 *     
 *     The status fifos inside the MAC should never go full as they are mirror of the BBH PD FIFOs, but in cases where
 *     the MAC design behaves different than expected, we want the BBH to be able to operate as in 1G EPON mode
 * addcrc: 
 *     configuration whether to add 4 bytes per packet to the length received in the status message from the Runner so
 *     the MAC would know the actual length to be transmitted.
 * req_full: 
 *     defines the depth of the req fifo.
 *     Physically there are 8 entries in the FIFO, but this configuration changes the full indication towards the EPON
 *     MAC so the FIFO depth is reduced effectively.
 * sendrdptr: 
 *     in xepon, the BBH generates the STS message from the PD so no need to send rd pointer to the runner, but we
 *     keep this option in case the runner would like to check that the STS fifo is not full before sending a PD. In
 *     epon, this bit should be set anyway.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_epncfg_set(uint8_t bbh_id, const bbh_tx_wan_configurations_epncfg *wan_configurations_epncfg);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_epncfg_get(uint8_t bbh_id, bbh_tx_wan_configurations_epncfg *wan_configurations_epncfg);

/**********************************************************************************************************************
 * wdata: 
 *     write data.
 *     
 *     15:0 - port-id - default is 0x0000
 *     16 - regenerate CRC - enabled by default
 *     17 - enc enable - disabled by default
 *     18 - enable - enabled by default
 *     
 *     
 * a: 
 *     address
 * cmd: 
 *     rd/wr cmd
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_flow2port_set(uint8_t bbh_id, uint32_t wdata, uint8_t a, bdmf_boolean cmd);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_flow2port_get(uint8_t bbh_id, uint32_t *wdata, uint8_t *a, bdmf_boolean *cmd);

/**********************************************************************************************************************
 * en: 
 *     1588 enable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_ts_set(uint8_t bbh_id, bdmf_boolean en);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_ts_get(uint8_t bbh_id, bdmf_boolean *en);

/**********************************************************************************************************************
 * maxwlen: 
 *     max_word_len
 * min_credit: 
 *     min credits that allow the bbh to order new chunk of data. Default is 18 as the max size of a chunk is 16 (128
 *     bytes) + 2 PDs (in case of aggregation in DDR).
 * spare: 
 *     spare
 * prio_en: 
 *     enables the priority mechanism (q_in_burst)
 * srst_n: 
 *     soft reset
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_dsl_cfg_set(uint8_t bbh_id, const bbh_tx_wan_configurations_dsl_cfg *wan_configurations_dsl_cfg);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_dsl_cfg_get(uint8_t bbh_id, bbh_tx_wan_configurations_dsl_cfg *wan_configurations_dsl_cfg);

/**********************************************************************************************************************
 * flush_wait_cycles: 
 *     option to delay the flush done indication towards the VDSL MAC.
 *     The field indicates the number of cycles to wait before flush done, in 1024 cycles resolution.
 * sw_flush_done: 
 *     indication from the SW to end the flush sequence.
 *     
 *     The HW will send flush done indication to the VDSL MAC once the HW flush done condition is met and this bit is
 *     set to 1.
 *     It is SW responsibility to de-assert this bit once the flush sequence is ended and before a new flush sequence.
 * flush_wait_en: 
 *     enables the options to wait after flush done condition is met.
 *     one option is to wait for the delay counter to end. The second option is to wait for SW to end the flush
 *     sequence.
 *     The SW has visibility of the HW flush state.
 * sw_flush_req: 
 *     request from the SW to enter flush state
 * flush_ignore_rd: 
 *     ignore DSL MAC reads during flush, meaning the BBH TX will push data to the MAC regardless of the read state
 * sw_crdts_val: 
 *     The number of credits per DSL channel in case the SW initializes the credits counters.
 * sw_crdts_init: 
 *     indication for the HW to initialize the credits counters. The HW will identify assertion of this bit, so the SW
 *     is responsible for de-assertion before the next init.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_dsl_cfg2_set(uint8_t bbh_id, const bbh_tx_wan_configurations_dsl_cfg2 *wan_configurations_dsl_cfg2);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_dsl_cfg2_get(uint8_t bbh_id, bbh_tx_wan_configurations_dsl_cfg2 *wan_configurations_dsl_cfg2);

/**********************************************************************************************************************
 * init: 
 *     when asserted, the HW will start initializing the counters with value of 0. Should not be done during traffic.
 * initdone: 
 *     Asserted by the HW, this bit indicates the HW finished initializing the counters with value of 0.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_gemctrinit_set(uint8_t bbh_id, bdmf_boolean init, bdmf_boolean initdone);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_gemctrinit_get(uint8_t bbh_id, bdmf_boolean *init, bdmf_boolean *initdone);

/**********************************************************************************************************************
 * rdaddress: 
 *     the counter to be read
 * rd: 
 *     rd
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_gemctrrd_set(uint8_t bbh_id, uint8_t rdaddress, bdmf_boolean rd);
bdmf_error_t ag_drv_bbh_tx_wan_configurations_gemctrrd_get(uint8_t bbh_id, uint8_t *rdaddress, bdmf_boolean *rd);

/**********************************************************************************************************************
 * rddata: 
 *     read data:
 *     pkts count 27:0, byte cnt 35:32
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_gemctrrd0_get(uint8_t bbh_id, uint32_t *rddata);

/**********************************************************************************************************************
 * rddata: 
 *     read data:
 *     byte cnt 31:0
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_wan_configurations_gemctrrd1_get(uint8_t bbh_id, uint32_t *rddata);

/**********************************************************************************************************************
 * q0: 
 *     Q0 configuration
 * q1: 
 *     Q1 configuration
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_q2rnr_set(uint8_t bbh_id, uint8_t q_2_rnr_index, uint8_t q0, uint8_t q1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_q2rnr_get(uint8_t bbh_id, uint8_t q_2_rnr_index, uint8_t *q0, uint8_t *q1);

/**********************************************************************************************************************
 * q0: 
 *     Q0 configuration
 * q1: 
 *     Q1 configuration
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_chksumq_set(uint8_t bbh_id, uint8_t unified_q_profile_index, bdmf_boolean q0, bdmf_boolean q1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_chksumq_get(uint8_t bbh_id, uint8_t unified_q_profile_index, bdmf_boolean *q0, bdmf_boolean *q1);

/**********************************************************************************************************************
 * q0: 
 *     Q0 configuration
 * q1: 
 *     Q1 configuration
 * dis0: 
 *     disable q 0
 * dis1: 
 *     disable q 1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_qprof_set(uint8_t bbh_id, uint8_t unified_q_profile_index, bdmf_boolean q0, bdmf_boolean q1, bdmf_boolean dis0, bdmf_boolean dis1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_qprof_get(uint8_t bbh_id, uint8_t unified_q_profile_index, bdmf_boolean *q0, bdmf_boolean *q1, bdmf_boolean *dis0, bdmf_boolean *dis1);

/**********************************************************************************************************************
 * fifosize0: 
 *     The size of PD FIFO for queue 0.
 *     A value of n refers to n+1.
 *     For GPON, the max value is 0x7
 *     For EPON, the max value is 0xf
 * fifosize1: 
 *     The size of PD FIFO for queue 1.
 *     A value of n refers to n+1.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdsize_set(uint8_t bbh_id, uint16_t fifosize0, uint16_t fifosize1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdsize_get(uint8_t bbh_id, uint16_t *fifosize0, uint16_t *fifosize1);

/**********************************************************************************************************************
 * wkupthresh0: 
 *     The wakeup threshold of the PD FIFO for queue 0.
 *     A value of n refers to n+1.
 *     Relevant only for EPON BBH.
 * wkupthresh1: 
 *     The wakeup threshold of the PD FIFO for queue 1.
 *     A value of n refers to n+1.
 *     Relevant only for EPON BBH.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdwkuph_set(uint8_t bbh_id, uint8_t wkupthresh0, uint8_t wkupthresh1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdwkuph_get(uint8_t bbh_id, uint8_t *wkupthresh0, uint8_t *wkupthresh1);

/**********************************************************************************************************************
 * pdlimit0: 
 *     Defines the number of bytes for PDs pre fetch limited according to the total number of bytes.
 *     The value is in 8-bytes resolution.
 *     
 * pdlimit1: 
 *     Defines the number of bytes for PDs pre fetch limited according to the total number of bytes.
 *     The value is in 8-bytes resolution.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pd_byte_th_set(uint8_t bbh_id, uint16_t pdlimit0, uint16_t pdlimit1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pd_byte_th_get(uint8_t bbh_id, uint16_t *pdlimit0, uint16_t *pdlimit1);

/**********************************************************************************************************************
 * q0: 
 *     Q0 configuration
 * q1: 
 *     Q1 configuration
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_qmq_set(uint8_t bbh_id, uint8_t unified_qm_q_index, bdmf_boolean q0, bdmf_boolean q1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_qmq_get(uint8_t bbh_id, uint8_t unified_qm_q_index, bdmf_boolean *q0, bdmf_boolean *q1);

/**********************************************************************************************************************
 * q0: 
 *     Q0 configuration
 * q1: 
 *     Q1 configuration
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_motf_set(uint8_t bbh_id, uint8_t unified_motf_index, uint8_t q0, uint8_t q1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_motf_get(uint8_t bbh_id, uint8_t unified_motf_index, uint8_t *q0, uint8_t *q1);

/**********************************************************************************************************************
 * pdlimiten: 
 *     This bit enables the above feature (PDs pre fetch limited according to the total number of bytes).
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pd_byte_th_en_set(uint8_t bbh_id, bdmf_boolean pdlimiten);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pd_byte_th_en_get(uint8_t bbh_id, bdmf_boolean *pdlimiten);

/**********************************************************************************************************************
 * empty: 
 *     EPON PD FIFO empty threshold.
 *     A queue which its PD FIFO occupancy is below this threshold will have high priority in PD ordering arbitration.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdempty_set(uint8_t bbh_id, uint8_t empty);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_pdempty_get(uint8_t bbh_id, uint8_t *empty);

/**********************************************************************************************************************
 * ddrthresh: 
 *     DDR Transmit threshold in 8 bytes resoltion
 * sramthresh: 
 *     SRAM Transmit threshold in 8 bytes resoltion
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_gtxthresh_set(uint8_t bbh_id, uint16_t ddrthresh, uint16_t sramthresh);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_gtxthresh_get(uint8_t bbh_id, uint16_t *ddrthresh, uint16_t *sramthresh);

/**********************************************************************************************************************
 * en: 
 *     enable bit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_eee_set(uint8_t bbh_id, uint8_t en);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_eee_get(uint8_t bbh_id, uint8_t *en);

/**********************************************************************************************************************
 * en: 
 *     enable bit
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_ts_set(uint8_t bbh_id, uint8_t en);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_ts_get(uint8_t bbh_id, uint8_t *en);

/**********************************************************************************************************************
 * init: 
 *     initialization of the credits counter with the size of the FE FIFOS (pd and data)
 * min_pd: 
 *     minimum number of credits for the FE PD FIFO in order to perform an ordering of chuck of data from the SRAM/DDR
 * min_data: 
 *     minimum number of credits for the FE TX FIFO in order to perform an ordering of chuck of data from the SRAM/DDR
 * use_buf_rdy: 
 *     determine whether to use credits mechanism or buf_rdy mechanism
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_fe_credits_set(uint8_t bbh_id, uint8_t init, uint16_t min_pd, uint16_t min_data, bdmf_boolean use_buf_rdy);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_fe_credits_get(uint8_t bbh_id, uint8_t *init, uint16_t *min_pd, uint16_t *min_data, bdmf_boolean *use_buf_rdy);

/**********************************************************************************************************************
 * fifobase0: 
 *     The base of FE FIFO for queue 0.
 * fifobase1: 
 *     The base of FE FIFO for queue 1.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_febase_set(uint8_t bbh_id, uint8_t unified_fe_base_index, uint16_t fifobase0, uint16_t fifobase1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_febase_get(uint8_t bbh_id, uint8_t unified_fe_base_index, uint16_t *fifobase0, uint16_t *fifobase1);

/**********************************************************************************************************************
 * fifosize0: 
 *     The size of PD FIFO for queue 0.
 *     A value of n refers to n+1.
 *     
 * fifosize1: 
 *     The size of FE FIFO for queue 1.
 *     A value of n refers to n+1.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_fesize_set(uint8_t bbh_id, uint8_t unified_fe_size_index, uint16_t fifosize0, uint16_t fifosize1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_fesize_get(uint8_t bbh_id, uint8_t unified_fe_size_index, uint16_t *fifosize0, uint16_t *fifosize1);

/**********************************************************************************************************************
 * fifobase0: 
 *     The base of FE PD FIFO for queue 0.
 * fifobase1: 
 *     The base of FE PD FIFO for queue 1.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_fepdbase_set(uint8_t bbh_id, uint8_t unified_fe_pd_base_index, uint16_t fifobase0, uint16_t fifobase1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_fepdbase_get(uint8_t bbh_id, uint8_t unified_fe_pd_base_index, uint16_t *fifobase0, uint16_t *fifobase1);

/**********************************************************************************************************************
 * fifosize0: 
 *     The size of FE PD FIFO for queue 0.
 *     A value of n refers to n+1.
 *     
 * fifosize1: 
 *     The size of FE PD FIFO for queue 1.
 *     A value of n refers to n+1.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_fepdsize_set(uint8_t bbh_id, uint8_t unified_fe_pd_size_index, uint16_t fifosize0, uint16_t fifosize1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_fepdsize_get(uint8_t bbh_id, uint8_t unified_fe_pd_size_index, uint16_t *fifosize0, uint16_t *fifosize1);

/**********************************************************************************************************************
 * w0: 
 *     weight of MAC0
 * w1: 
 *     weight of MAC1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_txwrr_set(uint8_t bbh_id, uint8_t unified_tx_wrr_index, uint8_t w0, uint8_t w1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_txwrr_get(uint8_t bbh_id, uint8_t unified_tx_wrr_index, uint8_t *w0, uint8_t *w1);

/**********************************************************************************************************************
 * w0: 
 *     weight of MAC0
 * w1: 
 *     weight of MAC1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_sgmtwrr_set(uint8_t bbh_id, uint8_t unified_q_profile_index, uint8_t w0, uint8_t w1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_sgmtwrr_get(uint8_t bbh_id, uint8_t unified_q_profile_index, uint8_t *w0, uint8_t *w1);

/**********************************************************************************************************************
 * thresh0: 
 *     Transmit threshold in 8 bytes resoltion for mac 0
 * thresh1: 
 *     Transmit threshold in 8 bytes resolution for MAC1
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_unified_configurations_txthresh_set(uint8_t bbh_id, uint8_t unified_tx_thr_index, uint16_t thresh0, uint16_t thresh1);
bdmf_error_t ag_drv_bbh_tx_unified_configurations_txthresh_get(uint8_t bbh_id, uint8_t unified_tx_thr_index, uint16_t *thresh0, uint16_t *thresh1);

/**********************************************************************************************************************
 * pdsel: 
 *     rd from the PD FIFO
 * pdvsel: 
 *     rd from the PD valid array
 * pdemptysel: 
 *     rd from the PD empty array
 * pdfullsel: 
 *     rd from the PD Full array
 * pdbemptysel: 
 *     rd from the PD beliow empty array
 * pdffwkpsel: 
 *     rd from the PD full for wakeup empty array
 * fbnsel: 
 *     rd from the first BN array
 * fbnvsel: 
 *     rd from the first BN valid array
 * fbnemptysel: 
 *     rd from the first BN empty array
 * fbnfullsel: 
 *     rd from the first BN full array
 * getnextsel: 
 *     rd from the first Get Next array
 * getnextvsel: 
 *     rd from the get_next valid array
 * getnextemptysel: 
 *     rd from the get next empty array
 * getnextfullsel: 
 *     rd from the get next full array
 * gpncntxtsel: 
 *     rd from the gpon context array
 * bpmsel: 
 *     rd from the BPM FIFO
 * bpmfsel: 
 *     rd from the BPM FLUSH FIFO
 * sbpmsel: 
 *     rd from the SBPM FIFO
 * sbpmfsel: 
 *     rd from the SBPM FLUSH FIFO
 * stssel: 
 *     rd from the STS FIFO
 * stsvsel: 
 *     rd from the STS valid array
 * stsemptysel: 
 *     rd from the STS empty array
 * stsfullsel: 
 *     rd from the STS Full array
 * stsbemptysel: 
 *     rd from the STS beliow empty array
 * stsffwkpsel: 
 *     rd from the STS full for wakeup empty array
 * msgsel: 
 *     rd from the MSG FIFO
 * msgvsel: 
 *     rd from the msg valid array
 * epnreqsel: 
 *     rd from the epon request FIFO
 * datasel: 
 *     rd from the DATA FIFO (SRAM and DDR)
 * reordersel: 
 *     rd from the reorder FIFO
 * tsinfosel: 
 *     rd from the Timestamp Info FIFO
 * mactxsel: 
 *     rd from the MAC TX FIFO.
 *     
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_swrden_set(uint8_t bbh_id, const bbh_tx_debug_counters_swrden *debug_counters_swrden);
bdmf_error_t ag_drv_bbh_tx_debug_counters_swrden_get(uint8_t bbh_id, bbh_tx_debug_counters_swrden *debug_counters_swrden);

/**********************************************************************************************************************
 * rdaddr: 
 *     The address inside the array the sw wants to read
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_swrdaddr_set(uint8_t bbh_id, uint16_t rdaddr);
bdmf_error_t ag_drv_bbh_tx_debug_counters_swrdaddr_get(uint8_t bbh_id, uint16_t *rdaddr);

/**********************************************************************************************************************
 * data: 
 *     data
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_swrddata_get(uint8_t bbh_id, uint32_t *data);

/**********************************************************************************************************************
 * unified_pkt: 
 *     This counter counts the number of packets transmitted per unified port.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_unifiedpkt_get(uint8_t bbh_id, uint8_t debug_unified_pkt_ctr_idx, uint32_t *unified_pkt);

/**********************************************************************************************************************
 * ddrbyte: 
 *     This counter counts the number of transmitted bytes from the DDr.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_unifiedbyte_get(uint8_t bbh_id, uint8_t debug_unified_byte_ctr_idx, uint32_t *ddrbyte);

/**********************************************************************************************************************
 * oflw_q: 
 *     The queue in which an overflow occurred. valid only when credits_overflow is asserted.
 * oflw: 
 *     Indication that one of the queues credits counter has reached a value that is larger than the Queues FIFO.
 * flush_req: 
 *     flush request from the MAC
 * flush: 
 *     indication that the BBH is in flush state
 * flush_done: 
 *     Indication that the flush process is done
 * init_req: 
 *     initialization request from the DSL. At init state, the BBH sets its credit counters to the amount indicated by
 *     the DSL
 * init_done: 
 *     Init process is done
 * credit_init_val: 
 *     the size of the FIFO of each queue in 8 bytes resolution.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_dsl_sts_get(uint8_t bbh_id, bbh_tx_debug_counters_dsl_sts *debug_counters_dsl_sts);

/**********************************************************************************************************************
 * credits: 
 *     credits
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_dsl_credits_get(uint8_t bbh_id, uint8_t zero, bbh_tx_debug_counters_dsl_credits *debug_counters_dsl_credits);

/**********************************************************************************************************************
 * dbgvec: 
 *     Selected debug vector.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_dbgoutreg_get(uint8_t bbh_id, uint8_t zero, bbh_tx_debug_counters_dbgoutreg *debug_counters_dbgoutreg);

/**********************************************************************************************************************
 * in_segmentation: 
 *     in_segmentation indication
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_in_segmentation_get(uint8_t bbh_id, uint8_t index, uint32_t *in_segmentation);

/**********************************************************************************************************************
 * credits: 
 *     credits
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_unified_data_credits_get(uint8_t bbh_id, uint8_t bbh_tx_debug_counters_unified_data_credits_byte_ctr_idx, uint16_t *credits);

/**********************************************************************************************************************
 * credits: 
 *     credits
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_unified_pd_credits_get(uint8_t bbh_id, uint8_t bbh_tx_debug_counters_unified_pd_credits_byte_ctr_idx, uint16_t *credits);

/**********************************************************************************************************************
 * ddr_fifo: 
 *     ddr_fifo_overrun
 * sram_fifo: 
 *     sram_fifo_overrun
 * ddr_reorder_fifo: 
 *     ddr_reorder_fifo_overrun
 * sram_reorder_fifo: 
 *     sram_reorder_fifo_overrun
 * bpm_fifo: 
 *     bpm_fifo_overrun
 * bpm_flush_fifo: 
 *     bpm_flush_fifo_overrun
 * bpm_eop_fifo: 
 *     bpm_eop_fifo_overrun
 * sbpm_fifo: 
 *     sbpm_fifo_overrun
 * sbpm_flush_fifo: 
 *     sbpm_flush_fifo_overrun
 * sbpm_eop_fifo: 
 *     sbpm_eop_fifo_overrun
 * dbr_sync_fifo: 
 *     dbr_sync_fifo_overrun
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_fifos_overrun_get(uint8_t bbh_id, bbh_tx_debug_counters_fifos_overrun *debug_counters_fifos_overrun);

/**********************************************************************************************************************
 * srampd: 
 *     This counter counts the number of packets which were calculated checksum from the SRAM.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_chksumsrampd_get(uint8_t bbh_id, uint32_t *srampd);

/**********************************************************************************************************************
 * ddrpd: 
 *     This counter counts the number of packets which were calculated checksum from the DDR.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_chksumddrpd_get(uint8_t bbh_id, uint32_t *ddrpd);

/**********************************************************************************************************************
 * srambyte: 
 *     This counter counts the number of calculated checksum bytes from the SRAM.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_chksumsrambyte_get(uint8_t bbh_id, uint32_t *srambyte);

/**********************************************************************************************************************
 * ddrbyte: 
 *     This counter counts the number of calculated checksum bytes from the DDr.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_chksumddrbyte_get(uint8_t bbh_id, uint32_t *ddrbyte);

/**********************************************************************************************************************
 * srampd: 
 *     This counter counts the number of packets which were bypassing the checksum from the SRAM.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_chksumbypsrampd_get(uint8_t bbh_id, uint32_t *srampd);

/**********************************************************************************************************************
 * ddrpd: 
 *     This counter counts the number of packets which were bypassing checksum from the DDR.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_chksumbypddrpd_get(uint8_t bbh_id, uint32_t *ddrpd);

/**********************************************************************************************************************
 * srambyte: 
 *     This counter counts the number of bypass checksum bytes from the SRAM.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_chksumbypsrambyte_get(uint8_t bbh_id, uint32_t *srambyte);

/**********************************************************************************************************************
 * ddrbyte: 
 *     This counter counts the number of bypass checksum bytes from the DDr.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_chksumbypddrbyte_get(uint8_t bbh_id, uint32_t *ddrbyte);

/**********************************************************************************************************************
 * droppkt: 
 *     This counter counts the number of dropped pkts due to GEM disable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_disgemdroppkt_get(uint8_t bbh_id, uint32_t *droppkt);

/**********************************************************************************************************************
 * dropbyte: 
 *     This counter counts the number of dropped bytes due to GEM disable
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_disgemdropbyte_get(uint8_t bbh_id, uint32_t *dropbyte);

/**********************************************************************************************************************
 * ecnpkt: 
 *     This counter counts the number of packets that were ecn remarked
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_ecnpkt_get(uint8_t bbh_id, uint32_t *ecnpkt);

/**********************************************************************************************************************
 * ecnbytes: 
 *     This counter counts the number of bytes of ecn marked packets.
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_ecnbytes_get(uint8_t bbh_id, uint32_t *ecnbytes);

/**********************************************************************************************************************
 * v1: 
 *     value1
 * v2: 
 *     value2
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_ecnmarkerr_get(uint8_t bbh_id, uint16_t *v1, uint16_t *v2);

/**********************************************************************************************************************
 * v1: 
 *     value1
 * v2: 
 *     value2
 **********************************************************************************************************************/
bdmf_error_t ag_drv_bbh_tx_debug_counters_ecnlenerr_get(uint8_t bbh_id, uint16_t *v1, uint16_t *v2);

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
    cli_bbh_tx_debug_counters_ddrbyte,
    cli_bbh_tx_debug_counters_srambyte,
    cli_bbh_tx_debug_counters,
    cli_bbh_tx_common_configurations_sbpmcfg,
    cli_bbh_tx_common_configurations_ddrtmbasel,
    cli_bbh_tx_common_configurations_ddrtmbaseh,
    cli_bbh_tx_common_configurations_dfifoctrl,
    cli_bbh_tx_common_configurations_arb_cfg,
    cli_bbh_tx_common_configurations_bbroute,
    cli_bbh_tx_common_configurations_ddrcfg_tx2,
    cli_bbh_tx_common_configurations_rnrcfg_1,
    cli_bbh_tx_common_configurations_rnrcfg_2,
    cli_bbh_tx_common_configurations_perqtask,
    cli_bbh_tx_common_configurations_txrstcmd,
    cli_bbh_tx_common_configurations_dbgsel,
    cli_bbh_tx_common_configurations_clk_gate_cntrl,
    cli_bbh_tx_common_configurations_gpr,
    cli_bbh_tx_common_configurations_general_cfg,
    cli_bbh_tx_common_configurations_ecncfg,
    cli_bbh_tx_common_configurations_ddrfpminibasel,
    cli_bbh_tx_common_configurations_ddrfpminibaseh,
    cli_bbh_tx_wan_configurations_q2rnr,
    cli_bbh_tx_wan_configurations_qprof,
    cli_bbh_tx_wan_configurations_pdsize,
    cli_bbh_tx_wan_configurations_pdwkuph,
    cli_bbh_tx_wan_configurations_pd_byte_th,
    cli_bbh_tx_wan_configurations_qmq,
    cli_bbh_tx_wan_configurations_stssize,
    cli_bbh_tx_wan_configurations_stswkuph,
    cli_bbh_tx_wan_configurations_pd_byte_th_en,
    cli_bbh_tx_wan_configurations_pdempty,
    cli_bbh_tx_wan_configurations_stsempty,
    cli_bbh_tx_wan_configurations_stsrnrcfg_1,
    cli_bbh_tx_wan_configurations_stsrnrcfg_2,
    cli_bbh_tx_wan_configurations_msgrnrcfg_1,
    cli_bbh_tx_wan_configurations_msgrnrcfg_2,
    cli_bbh_tx_wan_configurations_epncfg,
    cli_bbh_tx_wan_configurations_flow2port,
    cli_bbh_tx_wan_configurations_ts,
    cli_bbh_tx_wan_configurations_dsl_cfg,
    cli_bbh_tx_wan_configurations_dsl_cfg2,
    cli_bbh_tx_wan_configurations_gemctrinit,
    cli_bbh_tx_wan_configurations_gemctrrd,
    cli_bbh_tx_wan_configurations_gemctrrd0,
    cli_bbh_tx_wan_configurations_gemctrrd1,
    cli_bbh_tx_unified_configurations_q2rnr,
    cli_bbh_tx_unified_configurations_chksumq,
    cli_bbh_tx_unified_configurations_qprof,
    cli_bbh_tx_unified_configurations_pdsize,
    cli_bbh_tx_unified_configurations_pdwkuph,
    cli_bbh_tx_unified_configurations_pd_byte_th,
    cli_bbh_tx_unified_configurations_qmq,
    cli_bbh_tx_unified_configurations_motf,
    cli_bbh_tx_unified_configurations_pd_byte_th_en,
    cli_bbh_tx_unified_configurations_pdempty,
    cli_bbh_tx_unified_configurations_gtxthresh,
    cli_bbh_tx_unified_configurations_eee,
    cli_bbh_tx_unified_configurations_ts,
    cli_bbh_tx_unified_configurations_fe_credits,
    cli_bbh_tx_unified_configurations_febase,
    cli_bbh_tx_unified_configurations_fesize,
    cli_bbh_tx_unified_configurations_fepdbase,
    cli_bbh_tx_unified_configurations_fepdsize,
    cli_bbh_tx_unified_configurations_txwrr,
    cli_bbh_tx_unified_configurations_sgmtwrr,
    cli_bbh_tx_unified_configurations_txthresh,
    cli_bbh_tx_debug_counters_swrden,
    cli_bbh_tx_debug_counters_swrdaddr,
    cli_bbh_tx_debug_counters_swrddata,
    cli_bbh_tx_debug_counters_unifiedpkt,
    cli_bbh_tx_debug_counters_unifiedbyte,
    cli_bbh_tx_debug_counters_dsl_sts,
    cli_bbh_tx_debug_counters_dsl_credits,
    cli_bbh_tx_debug_counters_dbgoutreg,
    cli_bbh_tx_debug_counters_in_segmentation,
    cli_bbh_tx_debug_counters_unified_data_credits,
    cli_bbh_tx_debug_counters_unified_pd_credits,
    cli_bbh_tx_debug_counters_fifos_overrun,
    cli_bbh_tx_debug_counters_chksumsrampd,
    cli_bbh_tx_debug_counters_chksumddrpd,
    cli_bbh_tx_debug_counters_chksumsrambyte,
    cli_bbh_tx_debug_counters_chksumddrbyte,
    cli_bbh_tx_debug_counters_chksumbypsrampd,
    cli_bbh_tx_debug_counters_chksumbypddrpd,
    cli_bbh_tx_debug_counters_chksumbypsrambyte,
    cli_bbh_tx_debug_counters_chksumbypddrbyte,
    cli_bbh_tx_debug_counters_disgemdroppkt,
    cli_bbh_tx_debug_counters_disgemdropbyte,
    cli_bbh_tx_debug_counters_ecnpkt,
    cli_bbh_tx_debug_counters_ecnbytes,
    cli_bbh_tx_debug_counters_ecnmarkerr,
    cli_bbh_tx_debug_counters_ecnlenerr,
};

int bcm_bbh_tx_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_bbh_tx_cli_init(bdmfmon_handle_t root_dir);

#endif
#endif
