/*
 * wlc_tx.h
 *
 * Common headers for transmit datapath components
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: wlc_tx.h 725045 2017-10-05 02:39:38Z $
 *
 */
#ifndef _wlc_tx_c
#define _wlc_tx_c

/* Place holder for tx datapath functions
 * Refer to RB http://wlan-rb.sj.broadcom.com/r/18439/ and
 * TWIKI http://hwnbu-twiki.broadcom.com/bin/view/Mwgroup/WlDriverTxQueuingUpdate2013
 * for details on datapath restructuring work
 * These functions will be moved in stages from wlc.[ch]
 */

#define WLPKTTIME(p) (WLPKTTAG(p)->pktinfo.atf.pkt_time)

/* scb+prec based txq pktq filter */
extern void wlc_txq_pktq_scb_pfilter(wlc_info_t *wlc, int prec, struct pktq *pq, struct scb *scb);
/* scb based txq pktq filter */
extern void wlc_txq_pktq_scb_filter(wlc_info_t *wlc, uint prec_bmp, struct pktq *pq,
	struct scb *scb);

extern uint16 wlc_get_txmaxpkts(wlc_info_t *wlc);
extern void wlc_set_txmaxpkts(wlc_info_t *wlc, uint16 txmaxpkts);
extern void wlc_set_default_txmaxpkts(wlc_info_t *wlc);

#ifdef  NEW_TXQ
extern struct swpktq* wlc_low_txq(txq_t *txq);
#define WLC_TXQ_OCCUPIED(w) \
	(!pktq_empty(WLC_GET_TXQ((w)->active_queue)) || \
	!pktq_empty(wlc_low_txq((w)->active_queue->low_txq)))
#else
#define WLC_TXQ_OCCUPIED(w) (!pktq_empty(WLC_GET_TXQ((w)->active_queue)))
#endif /* NEW_TXQ */

#ifdef NEW_TXQ
typedef uint (*txq_supply_fn_t)(void *ctx, uint ac,
	int requested_time, struct spktq *output_q, uint *fifo_idx);

extern uint wlc_pull_q(void *ctx, uint ac, int requested_time,
	struct spktq *output_q, uint *fifo_idx);
void wlc_low_txq_enq(txq_info_t *txqi,
	txq_t *txq, uint fifo_idx, void *pkt, int pkt_time);

/* used in bmac_fifo: acct for pkts that don't go through normal tx path
 * best example is pkteng
*/
extern int wlc_low_txq_buffered_inc(txq_t *txq, uint fifo_idx, int inc_time);

extern int wlc_low_txq_buffered_dec(txq_t *txq, uint fifo_idx, int dec_time);

extern uint wlc_txq_nfifo(txq_t *txq);

#ifdef TXQ_MUX

/* initial Tx MUX service quanta in usec */
#define WLC_TX_MUX_QUANTA 2000

extern ratespec_t wlc_tx_current_ucast_rate(wlc_info_t *wlc,
	struct scb *scb, uint ac);
extern ratespec_t wlc_pdu_txparams_rspec(osl_t *osh, void *p);
extern uint wlc_tx_mpdu_time(wlc_info_t *wlc, struct scb* scb,
	ratespec_t rspec, uint ac, uint mpdu_len);
extern uint wlc_tx_mpdu_frame_seq_overhead(ratespec_t rspec,
	wlc_bsscfg_t *bsscfg, wlcband_t *band, uint ac);
extern int wlc_txq_immediate_enqueue(wlc_info_t *wlc,
	wlc_txq_info_t *qi, void *pkt, uint prec);
extern ratespec_t wlc_lowest_scb_rspec(struct scb *scb);
extern ratespec_t wlc_lowest_band_rspec(wlcband_t *band);

#endif /* TXQ_MUX */

extern txq_info_t * wlc_txq_attach(wlc_info_t *wlc);
extern void wlc_txq_detach(txq_info_t *txq);
extern txq_t* wlc_low_txq_alloc(txq_info_t *txqi,
                                txq_supply_fn_t supply_fn, void *supply_ctx,
                                uint nswq, int high, int low);
extern void wlc_low_txq_free(txq_info_t *txqi, txq_t* txq);
extern void wlc_low_txq_flush(txq_info_t *txqi, txq_t* txq);
extern void wlc_low_txq_scb_flush(wlc_info_t *wlc, wlc_txq_info_t *qi, struct scb *remove);
#if defined(WL_MULTIQUEUE)
extern void wlc_tx_fifo_scb_flush(wlc_info_t *wlc, struct scb *remove);
#endif /* WL_MULTIQUEUE */
extern void wlc_txq_fill(txq_info_t *txqi, txq_t *txq);
extern void wlc_txq_complete(txq_info_t *txqi, txq_t *txq, uint fifo_idx,
                             int complete_pkts, int complete_time);
extern uint8 txq_stopped_map(txq_t *txq);
extern int wlc_txq_buffered_time(txq_t *txq, uint fifo_idx);
extern void wlc_tx_fifo_attach_complete(wlc_info_t *wlc);
/**
 * @brief Sanity check on the Flow Control state of the TxQ
 */
extern int wlc_txq_fc_verify(txq_info_t *txqi, txq_t *txq);

#endif /* NEW_TXQ */
extern wlc_txq_info_t* wlc_txq_alloc(wlc_info_t *wlc, osl_t *osh);
extern void wlc_txq_free(wlc_info_t *wlc, osl_t *osh, wlc_txq_info_t *qi);
extern void wlc_send_q(wlc_info_t *wlc, wlc_txq_info_t *qi);
extern void wlc_send_active_q(wlc_info_t *wlc);
extern int wlc_prep_pdu(wlc_info_t *wlc, struct scb *scb, void *pdu, uint *fifo);
extern int wlc_prep_sdu(wlc_info_t *wlc, struct scb *scb, void **sdu, int *nsdu, uint *fifo);
extern int wlc_prep_sdu_fast(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, struct scb *scb,
	void *sdu, uint *fifop, wlc_key_t *key, wlc_key_info_t* key_info, uint8* txc_hit);
extern void* wlc_hdr_proc(wlc_info_t *wlc, void *sdu, struct scb *scb);
extern uint16 wlc_d11hdrs(wlc_info_t *wlc, void *p, struct scb *scb, uint txparams_flags,
	uint frag, uint nfrags, uint queue, uint next_frag_len,
	const wlc_key_info_t *key_info, ratespec_t rspec_override, uint16 *txh_off);
#ifndef NEW_TXQ
extern void wlc_txfifo(wlc_info_t *wlc, uint fifo, void *p, wlc_txh_info_t* tx_info,
	bool commit, int8 txpktpend);
#endif /* NEW_TXQ */
extern void wlc_txprep_pkt_get_hdrs(wlc_info_t* wlc, void* p,
	uint8** txd, d11actxh_t** vhtHdr, struct dot11_header** d11_hdr);
extern bool wlc_txh_get_isSGI(const wlc_txh_info_t* txh_info);
extern bool wlc_txh_get_isSTBC(const wlc_txh_info_t* txh_info);
extern uint8 wlc_txh_info_get_mcs(wlc_info_t* wlc, wlc_txh_info_t* txh_info);
extern bool wlc_txh_info_is5GHz(wlc_info_t* wlc, wlc_txh_info_t* txh_info);
extern bool wlc_txh_has_rts(wlc_info_t* wlc, wlc_txh_info_t* txh_info);
extern bool wlc_txh_get_isAMPDU(wlc_info_t* wlc, const wlc_txh_info_t* txh_info);
extern void wlc_txh_set_epoch(wlc_info_t* wlc, uint8* pdata, uint8 epoch);
extern uint8 wlc_txh_get_epoch(wlc_info_t* wlc, wlc_txh_info_t* txh_info);
extern void wlc_tx_open_datafifo(wlc_info_t *wlc);
#if defined(TXQ_MUX)
#define WLC_TXFIFO_COMPLETE(w, f, p, t) wlc_txfifo_complete((w), (f), (p), (t))
extern void wlc_txfifo_complete(wlc_info_t *wlc, uint fifo, uint16 txpktpend, int txpkttime);
#define WLC_LOWEST_SCB_RSPEC(scb) wlc_lowest_scb_rspec((scb))
#define WLC_LOWEST_BAND_RSPEC(band) wlc_lowest_band_rspec((band))
#else
#define WLC_TXFIFO_COMPLETE(w, f, p, t) wlc_txfifo_complete((w), (f), (p))
extern void wlc_txfifo_complete(wlc_info_t *wlc, uint fifo, uint16 txpktpend);
#define WLC_LOWEST_SCB_RSPEC(scb) 0
#define WLC_LOWEST_BAND_RSPEC(band) 0
#endif /* TXQ_MUX */
#ifdef AP
extern void wlc_tx_fifo_sync_bcmc_reset(wlc_info_t *wlc);
#endif /* AP */
extern uint16 wlc_get_txh_frameid(wlc_info_t* wlc, void* pkt);
extern void wlc_get_txh_info(wlc_info_t* wlc, void* pkt, wlc_txh_info_t* tx_info);
extern void wlc_set_txh_info(wlc_info_t* wlc, void* pkt, wlc_txh_info_t* tx_info);
extern void wlc_block_datafifo(wlc_info_t *wlc, uint32 mask, uint32 val);
extern void wlc_txflowcontrol(wlc_info_t *wlc, wlc_txq_info_t *qi, bool on, int prio);
extern void wlc_txflowcontrol_override(wlc_info_t *wlc, wlc_txq_info_t *qi, bool on, uint override);
extern bool wlc_txflowcontrol_prio_isset(wlc_info_t *wlc, wlc_txq_info_t *qi, int prio);
extern bool wlc_txflowcontrol_override_isset(wlc_info_t *wlc, wlc_txq_info_t *qi, uint override);
extern void wlc_txflowcontrol_reset(wlc_info_t *wlc);
extern void wlc_txq_enq(void *ctx, struct scb *scb, void *sdu, uint prec);
extern uint wlc_txq_txpktcnt(void *ctx);
extern void wlc_txflowcontrol_reset_qi(wlc_info_t *wlc, wlc_txq_info_t *qi);
extern uint16 bcmc_fid_generate(wlc_info_t *wlc, wlc_bsscfg_t *bsscfg, uint16 txFrameID);

#ifdef WL_MULTIQUEUE
extern void wlc_tx_fifo_sync_complete(wlc_info_t *wlc, uint fifo_bitmap, uint8 flag);
extern void wlc_excursion_start(wlc_info_t *wlc);
extern void wlc_excursion_end(wlc_info_t *wlc);
extern void wlc_active_queue_set(wlc_info_t *wlc, wlc_txq_info_t *new_active_queue);
extern void wlc_primary_queue_set(wlc_info_t *wlc, wlc_txq_info_t *new_primary);
#ifdef WL_MU_TX
extern int wlc_tx_fifo_hold_set(wlc_info_t *wlc, uint fifo_bitmap);
extern void wlc_tx_fifo_hold_clr(wlc_info_t *wlc, uint fifo_bitmap);
#endif // endif
#else
#define wlc_excursion_start(wlc)	((void)(wlc))
#define wlc_excursion_end(wlc)		((void)(wlc))
#define wlc_primary_queue_set(wlc, qi)  ((void)(wlc), (void)(qi))
#define wlc_active_queue_set(wlc, qi)   ((void)(wlc), (void)(qi))
#ifdef WL_MU_TX
#define wlc_tx_fifo_hold_set(wlc, fifo_bitmap)	(0)
#define wlc_tx_fifo_hold_clr(wlc, fifo_bitmap)	((void)(wlc), (void)(fifo_bitmap))
#endif // endif
#endif /* WL_MULTIQUEUE */

#ifdef STA
extern void *wlc_sdu_to_pdu(wlc_info_t *wlc, void *sdu, struct scb *scb, bool is_8021x);
#endif /* STA */

#ifdef _wlc_wpa_h_
extern bool wlc_is_4way_msg(wlc_info_t *wlc, void *pkt, int offset, wpa_msg_t msg);
#endif /* _wlc_wpa_h_ */
extern void wlc_low_txq_map_pkts(wlc_info_t *wlc, wlc_txq_info_t *qi, map_pkts_cb_fn cb, void *ctx);
extern void wlc_txq_map_pkts(wlc_info_t *wlc, wlc_txq_info_t *qi, map_pkts_cb_fn cb, void *ctx);
extern void wlc_bsscfg_psq_map_pkts(wlc_info_t *wlc, struct pktq *q, map_pkts_cb_fn cb, void *ctx);
#ifdef AP
extern void wlc_scb_psq_map_pkts(wlc_info_t *wlc, struct pktq *q, map_pkts_cb_fn cb, void *ctx);
#endif // endif
extern void wlc_tx_map_pkts(wlc_info_t *wlc, struct pktq *q, int prec,
            map_pkts_cb_fn cb, void *ctx);
#ifdef PROP_TXSTATUS
extern void wlc_txq_flush_flowid_pkts(wlc_info_t *wlc, uint16 flowid);
#endif // endif

#if defined(WLAMPDU_MAC)
extern void wlc_epoch_upd(wlc_info_t *wlc, void *pkt, uint8 *flipEpoch, uint8 *lastEpoch);
#endif /* WLAMPDU_MAC */
#ifdef HOST_HDR_FETCH
extern void wlc_bmac_dma_submit(wlc_info_t *wlc, void *p,  uint queue,
	bool commit, bool firstentry);
extern void wlc_bmac_dma_commit(wlc_info_t *wlc, uint queue);
struct scb* wlc_recover_pkt_scb_hdr_inhost(wlc_info_t *wlc, void *pkt);
#endif /* HOST_HDR_FETCH */

extern void wlc_txq_enq_spq(wlc_info_t *wlc, struct scb *scb, struct spktq *spq, uint prec);
bool wlc_is_packet_fragmented(wlc_info_t *wlc, struct scb *scb,
		wlc_bsscfg_t *bsscfg, void *lb);
#endif /* _wlc_tx_c */
