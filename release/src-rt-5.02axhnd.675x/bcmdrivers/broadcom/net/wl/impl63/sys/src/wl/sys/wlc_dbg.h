/*
 * driver debug and print functions
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
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
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: wlc_dbg.h 778878 2019-09-12 11:54:06Z $
 */

#ifndef _wlc_dbg_h_
#define _wlc_dbg_h_

#include <typedefs.h>
#include <bcmutils.h>

#if defined(EVENT_LOG_COMPILE) && defined(WLMSG_TRACECH)
#define _WL_CHANSW(fmt, ...)	EVENT_LOG(EVENT_LOG_TAG_TRACE_CHANSW, fmt, ##__VA_ARGS__)
#define WL_CHANSW(args)		_WL_CHANSW args
#else
#define WL_CHANSW(args)
#endif // endif

#if defined(BCMDBG) || defined(WLMSG_PRPKT) || defined(WLMSG_ASSOC)
extern void
wlc_print_rxbcn_prb(wlc_info_t *wlc, uint8 *frame, int len);
#endif // endif

#if defined(BCMDBG) || defined(WLMSG_PRHDRS) || defined(WLMSG_PRPKT) || \
	defined(WLMSG_ASSOC) || defined(CFP_DEBUG)
void
wlc_print_dot11_mac_hdr(uint8* buf, int len);
#endif // endif

#if defined(BCMDBG) || defined(WLMSG_PRHDRS) || defined(ENABLE_CORECAPTURE) || \
	defined(CFP_DEBUG)
/* 'txd' points to the TxH in front of the TxD in core revid >= 40 */
void
wlc_print_txdesc(wlc_info_t *wlc, uint8 *txd);
void
wlc_recv_print_rxh(wlc_info_t *wlc, wlc_d11rxhdr_t *wrxh);
void
/* 'txd' points to the TxH in front of the TxD in core revid >= 40 */
wlc_print_hdrs(wlc_info_t *wlc, const char *prefix, uint8 *frame,
               uint8 *txd, wlc_d11rxhdr_t *wrxh, uint len);

#endif // endif

#if defined(WLTINYDUMP) || defined(BCMDBG) || defined(WLMSG_ASSOC) || \
	defined(WLMSG_PRPKT) || defined(WLMSG_OID) || defined(WLMSG_INFORM) || \
	defined(WLMSG_WSEC) || defined(WLTEST) || defined(BCMDBG_ERR) || defined(DNG_DBGDUMP) \
	|| defined(BCMDBG_RSDB) || defined(WLMSG_MESH) || defined(BCMDBG_MU)
int
wlc_format_ssid(char* buf, const uchar ssid[], uint ssid_len);
#endif /* defined(WLTINYDUMP) || defined(BCMDBG) || defined(WLMSG_ASSOC) ... */

#if defined(BCMDBG) || defined(WLMSG_PRPKT)
void wlc_print_auth(wlc_info_t *wlc, struct dot11_management_header *mng, int len);
void wlc_print_assoc_req(wlc_info_t *wlc, struct dot11_management_header *mng, int len);
void wlc_print_assoc_resp(wlc_info_t *wlc, struct dot11_management_header *mng, int len);
#endif // endif

#if defined(BCMDBG)
void wlc_dump_ie(wlc_info_t *wlc, bcm_tlv_t *ie, struct bcmstrbuf *b);
#endif // endif

#if defined(BCMDBG) || defined(WLMSG_PRPKT)
void wlc_print_ies(wlc_info_t *wlc, uint8 *ies, uint ies_len);
#endif // endif

/* This approach avoids overflow of a 32 bits integer */
#define PERCENT(total, part) (((total) > 10000) ? \
	((part) / ((total) / 100)) : \
	(((part) * 100) / (total)))

#if defined(BCMDBG) || defined(WLTEST) || defined(DUMP_MUTX) || defined(BCMDBG_AMPDU) \
	|| defined(DL_RU_STATS_DUMP) || defined(UL_RU_STATS_DUMP)
typedef enum {
	TABLE_NONAME,
	TABLE_MCS,
	TABLE_RU
} table_type_t;

extern void wlc_print_dump_table(struct bcmstrbuf *b, char* title,
	uint32 *table_val, char* title_per, uint32 *tsuccess,
	int max_idx, int nbr_colums, table_type_t table_type);
#endif /* BCMDBG || BCMDBG_DUMP || WLTEST || DUMP_MUTX ||
		* BCMDBG_AMPDU || DL_RU_STATS_DUMP || UL_RU_STATS_DUMP
		*/
#endif /* !_wlc_dbg_h_ */
