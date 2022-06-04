/*
 * HWA library exported routines.
 *
 * Copyright (C) 2021, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * <<Broadcom-WL-IPTag/Proprietary:>>
 *
 * $Id: $
 */

#ifndef _HWA_EXPORT_H
#define _HWA_EXPORT_H

/**
 * -----------------------------------------------------------------------------
 * HWA Core 0x851
 *
 * Family   Revision     Chips
 * ------   --------     ----------------------------
 * HWA2.0   128          43684Ax (Deprecated/Deleted)
 * HWA2.1   129          43684Bx
 * HWA2.1   130          43684Cx
 * HWA2.2   131           6715Ax
 *
 * vim: set ts=4 noet sw=4 tw=80:
 * -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * -----------------------------------------------------------------------------
 */

// List all revisions first. -DBCMHWA = ### specified in #chip#.mk file.
#define HWA_REVISION_ID         (BCMHWA)

#define HWA_FAMILY(corerev) (HWAREV_GE((corerev), 131) ? "2.2" : "2.1")

#include <hwa_defs.h>

// Forward declarations
struct hwa_dev;

#ifdef DONGLEBUILD
// Global HWA device pointer
extern struct hwa_dev *hwa_dev;
#endif // endif

// Aggregation Compact WI format. 0 means no aggregation
#define HWA_PCIEIPC_WI_AGGR_CNT     0 // 1a, 2b, 4b aggregation

// Processing HWA DPC functions with bound or not.
#define HWA_PROCESS_BOUND           TRUE
#define HWA_PROCESS_NOBOUND         FALSE

// Access to hwa_dev pointer in WL
#ifdef DONGLEBUILD
#define WL_HWA_DEVP(wlc)	hwa_dev
#else
#define WL_HWA_DEVP(wlc)	wlc->hwa_dev
#endif /* DONGLEBUILD */

// HWA capabilities
void    hwa_caps(struct hwa_dev *dev, struct bcmstrbuf *b);
// HWA BM pool use
void    hwa_print_pooluse(struct hwa_dev *dev);

#if defined(HWA_RXPOST_BUILD)

// Fixed length of Rx buffers posted
uint16  hwa_rxpost_data_buf_len(void);

// When HWA1a and HWA1b are built, SW can allocate an RPH from HWA1a
int     hwa_rph_allocate(uint32 *bufid, uint16 *len, dma64addr_t *haddr64,
            bool pre_req);

#endif /* HWA_RXPOST_BUILD */

#if defined(HWA_RXFILL_BUILD)

// Exported to WL BMAC which allocates the DMA rings and passes address to HWA1b
void    BCMUCODEFN(hwa_rxfill_fifo_attach)(void *wlc, uint32 core, uint32 fifo,
            uint32 depth, dma64addr_t fifo_addr, uint32 dmarcv_ptr, bool hme_macifs);
// Free a simple or paired RxBuffer into S2H RxFree interface
int     hwa_rxfill_rxbuffer_free(struct hwa_dev *dev, uint32 core,
            hwa_rxbuffer_t *rx_buffer, bool has_rph);
#ifdef HWA_RXFILL_RXFREE_AUDIT_ENABLED
void    hwa_rxfill_rxfree_audit(struct hwa_dev *dev, uint32 core,
            hwa_rxbuffer_t *rx_buffer, const bool alloc);
#endif // endif

#endif /* HWA_RXFILL_BUILD */

#if defined(HWA_RXPATH_BUILD)

// Handle MAC DMA reset
int     hwa_rxpath_dma_reset(struct hwa_dev *dev, uint32 core);
// Reclaim MAC Rx DMA posted buffer
void    hwa_rxpath_dma_reclaim(struct hwa_dev *dev);

#endif /* HWA_RXPATH_BUILD */

#if defined(HWA_RXDATA_BUILD)

// Query the type of filter match in a rxstatus, if any
uint32  hwa_rxdata_fhr_is_pktfetch(uint32 fhr_filter_match);
uint32  hwa_rxdata_fhr_is_l2filter(uint32 fhr_filter_match);
uint32  hwa_rxdata_fhr_is_llc_snap_da(uint32 fhr_filter_match);
uint32  hwa_rxdata_fhr_is_udpv6(uint32 fhr_filter_match);
uint32  hwa_rxdata_fhr_is_udpv4(uint32 fhr_filter_match);
uint32  hwa_rxdata_fhr_is_tcp(uint32 fhr_filter_match);
uint32  hwa_rxdata_fhr_unchainable(uint32 fhr_filter_match);

#endif /* HWA_RXDATA_BUILD */

#if defined(HWA_TXPOST_BUILD)

#define HWA_TXPOST_HISTOGRAM_MAX        64
#define HWA_TXPOST_HISTOGRAM_UNIT_SHIFT 1

// Issue a TxPost schedule command request to HWA3a
int     hwa_txpost_schedcmd_request(struct hwa_dev *dev,
            uint32 flowring_id, uint16 rd_idx, uint32 transfer_count);
// HWA3a FRC Management
int     hwa_txpost_frc_table_config(struct hwa_dev *dev, uint32 ring_id,
            uint8 ifid, uint16 ring_size, dma64addr_t *haddr64, uint8 ether_ip_enab);
int     hwa_txpost_frc_table_reset(struct hwa_dev *dev, uint32 ring_id);
// Handle SW allocate tx buffer from TxBM.
void    *hwa_txpost_txbuffer_get(struct hwa_dev *dev);
// Handle a Free TxBuffer request from WLAN driver
int     hwa_txpost_txbuffer_free(struct hwa_dev *dev, void *buf);
// Schedule command histogram
uint32  *hwa_txpost_histogram(struct hwa_dev *dev, bool clear);
// Dump TxPost HWA packet
void    hwa_txpost_dump_pkt(void *pkt, struct bcmstrbuf *b,
	const char *title, uint32 pkt_index, bool one_shot);
void    _hwa_txpost_dump_pkt(void *pkt, struct bcmstrbuf *b,
	const char *title, uint32 pkt_index, bool one_shot, bool raw);
// Update schedule command flags for last request
int	hwa_txpost_schedcmd_flags_update(struct hwa_dev *dev,
	uint8 schedule_flags);
#endif /* HWA_TXPOST_BUILD */

#if defined(HWA_TXFIFO_BUILD)

/* XXX Need better description in HWA-2.0 reg spec as to what these do
 * See frameid mismatch (txs->frameid 0xa5c1 txh->TxFrameID 0x65c1)
 * when set HWA_TXFIFO_LIMIT_THRESHOLD to 1024
 */
#define HWA_TXFIFO_LIMIT_THRESHOLD      63
#define HWA_TXFIFO_EMPTY_THRESHOLD      73

// Configure a TxFIFO's pkt and aqm descriptor ring context in HWA AXI memory
int     hwa_txfifo_config(struct hwa_dev *dev, uint32 core, uint32 fifo_idx,
            dma64addr_t fifo_base, uint32 fifo_depth,
            dma64addr_t aqm_fifo_base, uint32 aqm_fifo_depth, bool hme_macifs);
// Prepare to disable 3b block before MAC suspend.
void    hwa_txfifo_disable_prep(struct hwa_dev *dev, uint32 core);
// Enable or Disable 3b block
void    hwa_txfifo_enable(struct hwa_dev *dev, uint32 core, bool enable);
// Reclaim MAC Tx DMA posted packets
void    hwa_txfifo_dma_reclaim(struct hwa_dev *dev, uint32 core);
// Reset TxFIFO's curr_ptr and last_ptr of pkt and aqm ring context in HWA AXI memory
void    hwa_txfifo_dma_init(struct hwa_dev *dev, uint32 core, uint32 fifo_idx);
// Get TxFIFO's active descriptor count
uint    hwa_txfifo_dma_active(struct hwa_dev *dev, uint32 core, uint32 fifo_idx);
// Clear OvflowQ pkt_count, mpdu_count to avoid 3b process reclaimed packets
void    hwa_txfifo_clear_ovfq(struct hwa_dev *dev, uint32 core, uint32 fifo_idx);
// Handle a request from WLAN driver for transmission of a packet chain
bool    hwa_txfifo_pktchain_xmit_request(struct hwa_dev *dev, uint32 core,
            uint32 fifo_idx, void *pktchain_head, void *pktchain_tail,
            uint16 pkt_count, uint16 mpdu_count);
// Is the TXFIFO Pktchain Ring full.
bool    hwa_txfifo_pktchain_ring_isfull(struct hwa_dev *dev);
// Provide the TXed packet
void    *hwa_txfifo_getnexttxp32(struct hwa_dev *dev, uint32 fifo_idx, uint32 range);
// Like hwa_txfifo_getnexttxp32 but no reclaim
void    *hwa_txfifo_peeknexttxp(struct hwa_dev *dev, uint32 fifo_idx);
// HWA txfifo map function.
int     hwa_txfifo_map_pkts(struct hwa_dev *dev, uint32 fifo_idx, void *cb, void *ctx);
// HWA txfifo dump packet function.
void    hwa_txfifo_dump_pkt(void *pkt, struct bcmstrbuf *b, const char *title, bool one_shot);

#if defined(BCMDBG) || defined(HWA_DUMP)
void    hwa_txfifo_dump_shadow(struct hwa_dev *dev, struct bcmstrbuf *b, uint32 fifo_idx);
void    hwa_txfifo_dump_ovfq(struct hwa_dev *dev, struct bcmstrbuf *b, uint32 fifo_idx);
void    hwa_txfifo_dump_fifoctx(struct hwa_dev *dev, struct bcmstrbuf *b, uint32 fifo_idx);
#endif /* BCMDBG */

#endif /* HWA_TXFIFO_BUILD */

/* Macros to access HWA HW header */
#if defined(HWA_PKTPGR_BUILD)

#define HWA_PGI_RX_RECV_HISTOGRAM_MAX        64
#define HWA_PGI_RX_RECV_HISTOGRAM_UNIT_SHIFT 2

// Intended to not have HWAPKTSETNEXT in HWA_PKTPGR_BUILD
//#define HWAPKTSETNEXT(p, n)             (LBP(p)->control.next = LBP(n))
#define HWAPKTSETLINK(p, n)             (LBP(p)->control.link = LBP(n))
#define HWAPKTFLOWID(p)                 (LBFP(p)->fraginfo.tx.flowring_id)
#define HWAPKTTXPOSTETHTYPE(p) \
	(*(uint16 *)&(PPLBUF(p)->data_buffer[HWA_TXPOST_ETHTYPE_OFFSET_BYTES]))
#define HWAPKTPRIO(p)                   (LBP(p)->control.prio)
#define HWAPKTTXPOSTDATA(p) \
	(&(PPLBUF(p)->data_buffer[HWA_TXPOST_DATA_OFFSET_BYTES]))
#define HWAPKTLEN(p)                    (LBP(p)->control.len)
#define HWAPKTHDATA(p, ix)              (LBFP(p)->fraginfo.data_buf_haddr64[ix])
#define HWAPKTHLEN(p, ix)               (LBFP(p)->fraginfo.host_datalen[ix])
#define HWAPKTNDESC(p)                  (LBP(p)->control.txfifo.num_desc)
#define HWAPKTAMSDUTLEN(p)              (LBP(p)->control.txfifo.amsdu_total_len)
#define HWAPKTTXDMAFLAGS(p)             (LBP(p)->control.txfifo.txdma_flags)
#define HWAPKTSETDATA(p, data)          do {} while (0) // use PKTSETBUF if need
#define HWAPKTSETLEN(p, len)            do {} while (0) // use PKTSETLEN if need
#define HWAPKTSETHLEN(p, ix, len)       (LBFP(p)->fraginfo.host_datalen[ix] = (len))
#define HWAPKTSETNDESC(p, num)          (LBP(p)->control.txfifo.num_desc = (num))
#define HWAPKTSETAMSDUTLEN(p, len)      (LBP(p)->control.txfifo.amsdu_total_len = (len))
#define HWAPKTSETTXDMAFLAGS(p, flags)   (LBP(p)->control.txfifo.txdma_flags = (flags))

#else

/* Terminate 3a/3b swptk next point */
#define HWAPKTSETNEXT(p, n)             (((hwa_tx_pkt_t *)p)->next = ((hwa_tx_pkt_t *)n))
#define HWAPKTFLOWID(p)                 (((hwa_tx_pkt_t *)p)->flowid_override)
#define HWAPKTTXPOSTETHTYPE(p)          (((hwa_tx_pkt_t *)p)->eth_type)
#define HWAPKTPRIO(p)                   (((hwa_tx_pkt_t *)p)->prio)
#define HWAPKTLEN(p)                    (((hwa_tx_pkt_t *)p)->hdr_buf_dlen)
#define HWAPKTHDATA(p)                  (((hwa_tx_pkt_t *)p)->data_buf_haddr)
#define HWAPKTHLEN(p)                   (((hwa_tx_pkt_t *)p)->data_buf_hlen)
#define HWAPKTNDESC(p)                  (((hwa_tx_pkt_t *)p)->num_desc)
#define HWAPKTSETDATA(p, data)          (((hwa_tx_pkt_t *)p)->hdr_buf = (data))
#define HWAPKTSETLEN(p, len)            (((hwa_tx_pkt_t *)p)->hdr_buf_dlen = (len))
#define HWAPKTSETHLEN(p, len)           (((hwa_tx_pkt_t *)p)->data_buf_hlen = (len))
#define HWAPKTSETNDESC(p, num)          (((hwa_tx_pkt_t *)p)->num_desc = (num))
#define HWAPKTSETAMSDUTLEN(p, len)      (((hwa_tx_pkt_t *)p)->amsdu_total_len = (len))
#define HWAPKTSETTXDMAFLAGS(p, flags)   (((hwa_tx_pkt_t *)p)->txdma_flags = (flags))

#endif /* HWA_PKTPGR_BUILD */

#if defined(HWA_TXSTAT_BUILD)

// HWA4a TxStatus block reclaim
void    hwa_txstat_reclaim(struct hwa_dev *dev, uint32 core, bool reinit);

#endif /* HWA_TXSTAT_BUILD */

#if defined(HWA_RXCPLE_BUILD)

// Check if there is space to add workitem for the D2H RxCompletion common ring
bool    hwa_rxcple_resource_avail_check(struct hwa_dev *dev);
// Commit all added WI to the D2H RxCompletion common ring
void    hwa_rxcple_commit(struct hwa_dev *dev);
// Current added WIs for the D2H RxCompletion common ring
uint16  hwa_rxcple_pend_item_cnt(struct hwa_dev *dev);
// Add a RxCompletion WI to the circular WI array
int     hwa_rxcple_wi_add(struct hwa_dev *dev, uint8 ifid, uint8 flags,
                          uint8 data_offset, uint16 data_len, uint32 pktid);

#endif /* HWA_RXCPLE_BUILD */

#if defined(HWA_TXCPLE_BUILD)

// Check if there is space to add workitem for the D2H TxCompletion common ring
bool    hwa_txcple_resource_avail_check(struct hwa_dev *dev);

// Commit all added WI to the D2H TxCompletion common ring
void    hwa_txcple_commit(struct hwa_dev *dev);

// Current added WIs for the D2H TxCompletion common ring
uint16  hwa_txcple_pend_item_cnt(struct hwa_dev *dev);

// Add a TxCompletion WI to the curcular WI array
int     hwa_txcple_wi_add(struct hwa_dev *dev, uint32 pktid, uint16 ringid, uint8 ifindx);

#endif /* HWA_TXCPLE_BUILD */

#if defined(HWA_PKTPGR_BUILD)

// Free Simple-RxLfrag. No RSP
void    hwa_pktpgr_free_rx(struct hwa_dev *dev, hwa_pp_lbuf_t *pktlist_head,
            hwa_pp_lbuf_t * pktlist_tail, int pkt_count);

// Free RxPostHostInfo. No RSP
void    hwa_pktpgr_free_rph(struct hwa_dev *dev, uint32 host_pktid,
            uint16 host_datalen, dma64addr_t data_buf_haddr64);

// Free TxLfrag. No RSP
void    hwa_pktpgr_free_tx(struct hwa_dev *dev, hwa_pp_lbuf_t *pktlist_head,
            hwa_pp_lbuf_t * pktlist_tail, int pkt_count);

// Flag to enable HW capability of one MPDU has more than one TxLfrag.
// Mandatory for more thean 4-in-1 AMSDU.
uint8   hwa_pktpgr_multi_txlfrag(struct hwa_dev *dev);

// Free HME LCLPKT
void    hwa_pktpgr_hmedata_free(struct hwa_dev *dev, void *pkt);

// Prepare map pkts
void    hwa_pktpgr_map_pkts_prep(struct hwa_dev *dev);

// Done of map pkts process
void    hwa_pktpgr_map_pkts_done(struct hwa_dev *dev);

// Map pkts in pager rings
void    hwa_pktpgr_ring_map_pkts(struct hwa_dev *dev, void *cb, void *ctx);

// Pagein packets in HW shadow to SW shadow.
void    hwa_pktpgr_txfifo_shadow_reclaim(struct hwa_dev *dev, uint32 fifo_idx);

// Remove specific packets in PageOut Req Q to SW shadow.
void    hwa_pktpgr_pageout_ring_shadow_reclaim(struct hwa_dev *dev, uint32 fifo_idx);

#if defined(HWA_TXCPLE_BUILD)
// Check if there are spaces to add workitem for the D2H TxCompletion common ring
bool    hwa_txcple_resources_avail_check(struct hwa_dev *dev, void *p);
#endif /* HWA_TXCPLE_BUILD */

// PageIn Rx recv histogram
uint32  *hwa_rxfill_pagein_rx_recv_histogram(struct hwa_dev *dev, bool clear);

#if (HWA_REVISION_ID == 131)
// XXX: WAR for TXs framid mismatch for 6715A0
bool    hwa_txstat_mismatch_bypass(struct hwa_dev *dev);
void    hwa_txstat_mismatch_reset(struct hwa_dev *dev);
#endif /* HWA_REVISION_ID == 131 */

#endif /* HWA_PKTPGR_BUILD */

/*
 * -----------------------------------------------------------------------------
 * Section: MAC Configuration
 * -----------------------------------------------------------------------------
 */
// HWA Dev and Common register configuration
typedef enum hwa_mac_config
{
	HWA_HC_HIN_REGS       = 0, // HWA dev: Setup the hc_hin register pointer
	HWA_MAC_AXI_BASE      = 1, // Direct access to MAC HWA 2a register file
	                           // HWA Common register configuration
	HWA_MAC_BASE_ADDR     = 2, // MAC Base address
	HWA_MAC_FRMTXSTATUS   = 3, // MAC FrmTxStatus
	HWA_MAC_DMA_PTR       = 4, // MAC DMA Pointer
	HWA_MAC_IND_XMTPTR    = 5, // MAC IND XMT Pointer
	HWA_MAC_IND_QSEL      = 6, // MAC IND QSel
	HWA_MAC_CONFIG_MAX    = 7
} hwa_mac_config_t;

// Invoked by MAC to configure the HWA Common block registers
void    hwa_mac_config(struct hwa_dev *dev, hwa_mac_config_t config, uint32 core,
            volatile void *ptr, uint32 val);

/*
 * -----------------------------------------------------------------------------
 * Section: Software Doorbell - Wake host network processor thread on core
 * -----------------------------------------------------------------------------
 */
typedef enum hwa_sw_doorbell
{
	HWA_TX_DOORBELL        = 0, // TxPost RD Index update
	HWA_RX_DOORBELL        = 1, // RxPost RD Index update
	HWA_TXCPL_DOORBELL     = 2, // TxCPL WR Index update
	HWA_RXCPL_DOORBELL     = 3, // RxCPL WR Index update
} hwa_sw_doorbell_t;

// Register a software doorbell - write configure value to a Runner register
void    hwa_sw_doorbell_request(struct hwa_dev *dev, hwa_sw_doorbell_t request,
            uint32 index, dma64addr_t haddr64, uint32 value);

// HWA 3a packet chain will be splited into two types of PTKC by SW
typedef enum hwa_pktc_type
{
	HWA_PKTC_TYPE_UNICAST        = 0,
	HWA_PKTC_TYPE_UNICAST_LEGACY = 1,
	HWA_PKTC_TYPE_BCMC           = 2,
	HWA_PKTC_TYPE_FLOW_MISMATCH  = 3,
	HWA_PKTC_TYPE_MAX            = 4
} hwa_pktc_type_t;

#if defined(BCMDBG) || defined(HWA_DUMP)
/* HWA blocks identified by a 32 bit block_bitmap, used in debug dump */
#define HWA_DUMP_TOP            (1 <<  0)
#define HWA_DUMP_CMN            (1 <<  1)
#define HWA_DUMP_DMA            (1 <<  2)
#define HWA_DUMP_1A             (1 <<  3)
#define HWA_DUMP_1B             (1 <<  4)
#define HWA_DUMP_2A             (1 <<  5)
#define HWA_DUMP_2B             (1 <<  6)
#define HWA_DUMP_3A             (1 <<  7)
#define HWA_DUMP_3B             (1 <<  8)
#define HWA_DUMP_4A             (1 <<  9)
#define HWA_DUMP_4B             (1 << 10)
#define HWA_DUMP_PP             (1 << 11)
#define HWA_DUMP_ALL            (0xFFFFFFFF)

int     hwa_wl_dump(struct hwa_dev *dev, struct bcmstrbuf *b);
int     hwa_dhd_dump(struct hwa_dev *dev, char *dump_args);
void    hwa_dump(struct hwa_dev *dev, struct bcmstrbuf *b, uint32 block_bitmap,
	bool verbose, bool dump_regs, bool dump_txfifo_shadow, uint8 *fifo_bitmap);
#endif /* BCMDBG */

#if defined(WLTEST) || defined(HWA_DUMP) || defined(BCMDBG_ERR)
int     hwa_dbg_regread(struct hwa_dev *dev, char *type, uint32 reg_offset, int32 *ret_int_ptr);
#if defined(WLTEST)
int     hwa_dbg_regwrite(struct hwa_dev *dev, char *type, uint32 reg_offset, uint32 val);
#endif // endif
#endif // endif

// OSL related API
void    hwa_osl_detach(struct hwa_dev *dev);

extern void hwa_worklet_invoke(struct hwa_dev *dev, uint32 intmask);

// Others
struct hwa_dev *BCMATTACHFN(hwa_attach)(void *wlc, uint device, osl_t *osh,
            volatile void *regs, si_t *sih, uint unit, uint bustype);
void    BCMATTACHFN(hwa_detach)(struct hwa_dev *dev);
void    hwa_config(struct hwa_dev *dev);
void    hwa_set_reinit(struct hwa_dev *dev);

// HWA HME MACIFS offload
#define HWA_HME_MACIFS_NONE        (0)
#define HWA_HME_MACIFS_RX          (1 << 0)
#define HWA_HME_MACIFS_TX          (1 << 1)
#define HWA_HME_MACIFS_RX_TX       (HWA_HME_MACIFS_RX | HWA_HME_MACIFS_TX)
#define HWA_HME_MACIFS_RX_EN(dev)  (hwa_hme_macifs_modes(dev) & HWA_HME_MACIFS_RX)
#define HWA_HME_MACIFS_TX_EN(dev)  (hwa_hme_macifs_modes(dev) & HWA_HME_MACIFS_TX)
#define HWA_HME_MACIFS_EN(dev)     (hwa_hme_macifs_modes(dev) & HWA_HME_MACIFS_RX_TX)
uint8   hwa_hme_macifs_modes(struct hwa_dev *dev);

#ifndef DONGLEBUILD
uint32  hwa_si_flag(struct hwa_dev *dev);
bool    hwa_wlc_txstat_process(struct hwa_dev *dev);
#endif /* !DONGLEBUILD */

/* Schedule flags definitions
 * 1-7 bits are reserved for future use
 */
typedef enum schedcmd_flags {
	TXPOST_SCHED_FLAGS_RESP_PEND = 0,
	TXPOST_SCHED_FLAGS_SQS_FORCE = 1
} schedcmd_flags_t;

/* schedule flags bitfields */
#define TXPOST_SCHED_FLAGS_RESP_PEND_MASK	(1 << TXPOST_SCHED_FLAGS_RESP_PEND)
#define TXPOST_SCHED_FLAGS_SQS_FORCE_MASK	(1 << TXPOST_SCHED_FLAGS_SQS_FORCE)

#endif	/* _HWA_EXPORT_H */
