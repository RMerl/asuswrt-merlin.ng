/*
 * Required functions exported by the port-specific (os-dependent) driver
 * to common (os-independent) driver code.
 *
 * Copyright (C) 2017, Broadcom. All Rights Reserved.
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
 * $Id: wl_export.h 627697 2016-03-28 06:15:02Z $
 */

#ifndef _wl_export_h_
#define _wl_export_h_

/* misc callbacks */
struct wl_info;
struct wl_if;
struct wlc_if;
struct wlc_key_info;

/** wl_init() is called upon fault ('big hammer') conditions and as part of a 'wlc up' */
extern void wl_init(struct wl_info *wl);
extern uint wl_reset(struct wl_info *wl);
extern void wl_intrson(struct wl_info *wl);
extern uint32 wl_intrsoff(struct wl_info *wl);
extern void wl_intrsrestore(struct wl_info *wl, uint32 macintmask);
extern void wl_event(struct wl_info *wl, char *ifname, wlc_event_t *e);
extern void wl_event_sync(struct wl_info *wl, char *ifname, wlc_event_t *e);
extern void wl_event_sendup(struct wl_info *wl, const wlc_event_t *e, uint8 *data, uint32 len);
extern int wl_up(struct wl_info *wl);
extern void wl_down(struct wl_info *wl);
extern void wl_dump_ver(struct wl_info *wl, struct bcmstrbuf *b);
extern void wl_txflowcontrol(struct wl_info *wl, struct wl_if *wlif, bool state, int prio);
extern void wl_set_copycount_bytes(struct wl_info *wl, uint16 copycount,
	uint16 d11rxoffset);
extern int wl_busioctl(struct wl_info *wl, uint32 cmd, void *buf, int len, int *used,
	int *needed, int set);
extern bool wl_alloc_dma_resources(struct wl_info *wl, uint dmaddrwidth);
extern void wl_isucodereclaimed(uint8 *value);
extern void wl_reclaim(void);
extern void wl_reclaim_postattach(void);
extern void wl_reclaim_ucode(void);
extern void wl_nocard_timer(void *arg);
extern void wl_recover_nocard(struct wl_info *wl);

#if defined(WLOFFLD) || defined(VASIP_HW_SUPPORT)
extern uint32 wl_pcie_bar1(struct wl_info *wl, uchar** addr);
#endif
#if defined(VASIP_HW_SUPPORT)
extern uint32 wl_pcie_bar2(struct wl_info *wl, uchar** addr);
#endif
#ifdef BCM_OL_DEV
extern void wl_watchdog(struct wl_info *wl);
extern void wl_msgup(struct wl_info *wl, osl_t *osh, void* resp);
extern void * wl_get_arpi(struct wl_info *wl, struct wl_if *wlif);
extern void * wl_get_icmpi(struct wl_info *wl, struct wl_if *wlif);
extern void * wl_get_tcpkeepi(struct wl_info *wl, struct wl_if *wlif);
extern void * wl_get_ndi(struct wl_info *wl, struct wl_if *wlif);
#endif
/* timer functions */
struct wl_timer;
extern struct wl_timer *wl_init_timer(struct wl_info *wl, void (*fn)(void* arg), void *arg,
                                      const char *name);
extern void wl_free_timer(struct wl_info *wl, struct wl_timer *timer);
/* Add timer guarantees the callback fn will not be called until AT LEAST ms later.  In the
 *  case of a periodic timer, this guarantee is true of consecutive callback fn invocations.
 *  As a result, the period may not average ms duration and the callbacks may "drift".
 *
 * A periodic timer must have a non-zero ms delay.
 */
extern void wl_add_timer(struct wl_info *wl, struct wl_timer *timer, uint ms, int periodic);
extern bool wl_del_timer(struct wl_info *wl, struct wl_timer *timer);

/* proptxtstatus functions */
#ifdef PROP_TXSTATUS
struct wlfc_mac_desc_handle_map;
extern uint8 wlfc_allocate_MAC_descriptor_handle(struct wlfc_mac_desc_handle_map* map);
extern int wlfc_MAC_table_update(struct wl_info *wl, uint8* ea,
	uint8 add_del, uint8 mac_handle, uint8 ifidx);
extern void wlfc_release_MAC_descriptor_handle(struct wlfc_mac_desc_handle_map* map, uint8 handle);
extern int wlfc_psmode_request(struct wl_info *wl, uint8 mac_handle, uint8 credit,
	uint8 precedence_bitmap, uint8 request_type);
extern void wlfc_process_wlhdr_complete_txstatus(struct wl_info *wl, uint8 status_flag, void* p,
	void *ptxs, bool hold);
extern int wlfc_push_credit_data(struct wl_info *wl, void* p);
extern int wlfc_push_signal_data(struct wl_info *wl, void* data, uint8 len, bool hold);
extern int wlfc_queue_signal_data(struct wl_info *wl, void* data, uint8 len);
extern int wlfc_queue_avail(struct wl_info *wl);
extern int wlfc_send_signal_data(struct wl_info *wl, bool hold);
extern int wlfc_sendup_ctl_info_now(struct wl_info *wl);
extern struct wlfc_info_state* wlfc_state_get(struct wl_info *wl);
extern uint32 wlfc_query_mode(struct wl_info *wl);
#ifdef BCMPCIEDEV
extern void wlfc_push_pkt_txstatus(struct wl_info *wl, void* p, void *txs, uint32 sz);
#endif

extern void wl_reset_credittohost(struct wl_info *wl);
#endif /* PROP_TXSTATUS */

#ifdef BCMPCIEDEV
int wlfc_upd_flr_weight(struct wl_info *wl, uint8 mac_handle, uint8 tid, void* params);
int wlfc_enab_fair_fetch_scheduling(struct wl_info *wl, void* params);
int wlfc_get_fair_fetch_scheduling(struct wl_info *wl, uint32 *status);
#endif /* BCMPCIEDEV */

#if defined(DONGLEBUILD)
#define wl_init_timer(wl, fn, arg, name)	wl_init_timer(wl, fn, arg, NULL)
#endif

#if defined(D0_COALESCING)
extern void wl_sendup_no_filter(struct wl_info *wl, struct wl_if *wlif, void *p, int numpkt);
#endif

/* data receive and interface management functions */
extern void wl_sendup(struct wl_info *wl, struct wl_if *wlif, void *p, int numpkt);
extern char *wl_ifname(struct wl_info *wl, struct wl_if *wlif);
extern struct wl_if *wl_add_if(struct wl_info *wl, struct wlc_if* wlcif, uint unit,
	struct ether_addr *remote);
extern void wl_del_if(struct wl_info *wl, struct wl_if *wlif);
/* RSDB specific interface update function */
extern void wl_update_if(struct wl_info *from_wl, struct wl_info *to_wl, wl_if_t *from_wlif,
	struct wlc_if *to_wlcif);

/* pcie root complex operations
	op == 0: get link capability in configuration space
	op == 1: hot reset
*/
extern int wl_osl_pcie_rc(struct wl_info *wl, uint op, int param);

extern void wl_monitor(struct wl_info *wl, wl_rxsts_t *rxsts, void *p);
extern void wl_set_monitor(struct wl_info *wl, int val);
#ifdef WLTXMONITOR
extern void wl_tx_monitor(struct wl_info *wl, wl_txsts_t *txsts, void *p);
#endif

extern uint wl_buf_to_pktcopy(osl_t *osh, void *p, uchar *buf, int len, uint offset);
extern void * wl_get_pktbuffer(osl_t *osh, int len);
extern int wl_set_pktlen(osl_t *osh, void *p, int len);

/* contexts in wlif structure. Currently following are valid */
#define IFCTX_ARPI	(1)
#define IFCTX_NDI	(2)
#define IFCTX_NETDEV	(3)
extern void *wl_get_ifctx(struct wl_info *wl, int ctx_id, wl_if_t *wlif);


#if defined(MACOSX) && defined(WL_BSSLISTSORT)
extern bool wl_sort_bsslist(struct wl_info *wl, wlc_bss_info_t **bip);
#else
#define wl_sort_bsslist(a, b) FALSE
#endif

#ifdef LINUX_CRYPTO
extern int wl_tkip_miccheck(struct wl_info *wl, void *p, int hdr_len, bool group_key, int id);
extern int wl_tkip_micadd(struct wl_info *wl, void *p, int hdr_len);
extern int wl_tkip_encrypt(struct wl_info *wl, void *p, int hdr_len);
extern int wl_tkip_decrypt(struct wl_info *wl, void *p, int hdr_len, bool group_key);
extern void wl_tkip_printstats(struct wl_info *wl, bool group_key);
extern int wl_tkip_keyset(struct wl_info *wl, const struct wlc_key_info *key_info,
	const uint8 *key_data, size_t key_len, const uint8 *rx_seq, size_t rx_seq_len);
#endif /* LINUX_CRYPTO */
#if defined(WL_WOWL_MEDIA) || defined(WOWLPF)
extern void wl_wowl_dngldown(struct wl_info *wl);
extern void wl_down_postwowlenab(struct wl_info *wl);
#endif
#ifdef WLC_LOW_ONLY
extern bool wl_dngl_is_ss(struct wl_info *wl);
#endif
#if defined(NDIS) && (NDISVER == 0x0620)
extern int wl_fast_scan_enabled(struct wl_info *wl, void *p);
extern void wl_fast_scan_result_search(struct wl_info *wl, void *p);
#endif

#ifdef DONGLEBUILD
extern void wl_indicate_maccore_state(struct wl_info *wl, uint8 state);
extern void wl_flush_rxreorderqeue_flow(struct wl_info *wl, struct reorder_rxcpl_id_list *list);
extern uint32 wl_chain_rxcomplete_id(struct reorder_rxcpl_id_list *list, uint16 id, bool head);
extern void wl_chain_rxcompletions_amsdu(osl_t *osh, void *p, bool norxcpl);
#define wl_chain_rxcomplete_id_tail(a, b) wl_chain_rxcomplete_id(a, b, FALSE)
#define wl_chain_rxcomplete_id_head(a, b) wl_chain_rxcomplete_id(a, b, TRUE)
#else
#define wl_indicate_maccore_state(a, b) do { } while (0)
#define wl_flush_rxreorderqeue_flow(a, b) do { } while (0)
#define wl_chain_rxcomplete_id_tail(a, b) 0
#define wl_chain_rxcomplete_id_head(a, b) 0
#define wl_chain_rxcompletions_amsdu(a, b, c) do {} while (0)
#endif /* DONGLEBUILD */
extern void wl_flowring_ctl(struct wl_info *wl, uint32 op, void *opdata);

#ifdef BCM_HOST_MEM_UCODE
extern int wl_sbaddr(struct wl_info *wl, uint32 *addr, uint32 *len);
#endif /* BCM_HOST_MEM_UCODE */

#endif	/* _wl_export_h_ */
